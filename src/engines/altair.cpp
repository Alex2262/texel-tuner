//
// Created by Alex Tian on 2/24/2023.
//

#include "altair.h"
#include "bitboard.h"
#include "evaluation_constants.h"
#include "tables.h"
#include <regex>
#include <iostream>
#include <array>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>

using namespace std;
using namespace Altair;

//
// Created by Alex Tian on 9/29/2022.
//



template <typename Out>
void split(const std::string &s, char delim, Out result) {
    std::istringstream iss(s);
    std::string item;
    while (std::getline(iss, item, delim)) {
        *result++ = item;
    }
}


std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}


Piece piece_to_num(char piece) {

    auto it = std::find(std::begin(PIECE_MATCHER), std::end(PIECE_MATCHER), piece);
    return static_cast<Piece>(it - std::begin(PIECE_MATCHER));
}

bool is_number(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(),
                                      s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}


BITBOARD Position::get_pieces(Piece piece) const {
    return pieces[piece];
}

BITBOARD Position::get_pieces(PieceType piece, Color color) const {
    return pieces[piece + color * COLOR_OFFSET];
}

BITBOARD Position::get_pieces(Color color) const {
    return get_pieces(PAWN, color) |
           get_pieces(KNIGHT, color) |
           get_pieces(BISHOP, color) |
           get_pieces(ROOK, color) |
           get_pieces(QUEEN, color) |
           get_pieces(KING, color);
}

[[nodiscard]] BITBOARD Position::get_our_pieces() {
    return get_pieces(side);
}

[[nodiscard]] BITBOARD Position::get_opp_pieces() {
    return get_pieces(~side);
}

[[nodiscard]] BITBOARD Position::get_all_pieces() const {
    return our_pieces | opp_pieces;
}

[[nodiscard]] BITBOARD Position::get_empty_squares() const {
    return ~all_pieces;
}

Square Position::get_king_pos(Color color) const {
    return lsb(get_pieces(KING, color));
}

void Position::remove_piece(Piece piece, Square square) {
    pieces[piece] &= ~(1ULL << square);
    board[square] = EMPTY;
}

void Position::place_piece(Piece piece, Square square) {
    pieces[piece] |= (1ULL << square);
    board[square] = piece;
}

PLY_TYPE Position::set_fen(const std::string& fen_string) {

    //std::string reduced_fen_string = std::regex_replace(fen_string, std::regex("^ +| +$|( ) +"), "$1");
    std::vector<std::string> fen_tokens = split(fen_string, ' ');

    //if (fen_tokens.size() < 4) {
    //    throw std::invalid_argument( "Fen is incorrect" );
    //}

    const std::string position = fen_tokens[0];
    const std::string player = fen_tokens[1];
    const std::string castling = fen_tokens[2];
    const std::string en_passant = fen_tokens[3];

    std::string half_move_clock = fen_tokens.size() >= 5 ? fen_tokens[4] : "0";
    //std::string full_move_counter = fen_tokens.size() >= 6 ? fen_tokens[5] : "1";

    if (!is_number(half_move_clock)) half_move_clock = "0";
    //if (!is_number(full_move_counter)) full_move_counter = "0";

    side = (player == "w") ? WHITE : BLACK;

    for (int piece = WHITE_PAWN; piece != EMPTY; piece++) {
        pieces[piece] = 0ULL;
    }

    auto pos = static_cast<Square>(56);

    // Parsing the main 8x8 board part while adding appropriate padding
    for (char c : position) {
        if (c == '/' ) {
            pos = static_cast<Square>(pos - 16);
        } else if (std::isdigit(c)) {

            for (int empty_amt = 0; empty_amt < c - '0'; empty_amt++) {
                board[pos] = EMPTY;
                pos = static_cast<Square>(pos + 1);
            }

        }
        else if (std::isalpha(c)) {

            Piece piece = piece_to_num(c);
            place_piece(piece, pos);

            pos = static_cast<Square>(pos + 1);

        }
    }

    castle_ability_bits = 0;
    for (char c : castling) {

        if (c == 'K') castle_ability_bits |= 1;
        else if (c == 'Q') castle_ability_bits |= 2;
        else if (c == 'k') castle_ability_bits |= 4;
        else if (c == 'q') castle_ability_bits |= 8;

    }

    if (en_passant.size() > 1) {
        auto square = static_cast<Square>((en_passant[1] - '1') * 8 + en_passant[0] - 'a');
        ep_square = square;
    }
    else {
        ep_square = NO_SQUARE;
    }

    our_pieces = get_our_pieces();
    opp_pieces = get_opp_pieces();
    all_pieces = get_all_pieces();
    empty_squares = get_empty_squares();

    return static_cast<PLY_TYPE>(std::stoi(half_move_clock));
}


// --------------------------------------------------
//                     EVALUATION
// --------------------------------------------------

struct EvaluationInformation {
    int game_phase = 0;

    int passed_pawn_count[2]{};

    int piece_counts[2][6]{};

    int total_king_ring_attacks[2]{};

    Square king_squares[2]{};

    BITBOARD pawns[2]{};
    BITBOARD pieces[2]{};
    BITBOARD pawn_attacks[2]{};

    BITBOARD piece_relative_occupancies[2][6]{};
};

void initialize_evaluation_information(Position& position, EvaluationInformation& evaluation_information) {
    evaluation_information.game_phase = 0;

    evaluation_information.total_king_ring_attacks[WHITE] = 0;
    evaluation_information.total_king_ring_attacks[BLACK] = 0;

    evaluation_information.king_squares[WHITE] = position.get_king_pos(WHITE);
    evaluation_information.king_squares[BLACK] = position.get_king_pos(BLACK);

    evaluation_information.pawns[WHITE] = position.get_pieces(PAWN, WHITE);
    evaluation_information.pawns[BLACK] = position.get_pieces(PAWN, BLACK);

    evaluation_information.pieces[WHITE] = position.get_pieces(WHITE);
    evaluation_information.pieces[BLACK] = position.get_pieces(BLACK);

    evaluation_information.pawn_attacks[WHITE] = get_pawn_bitboard_attacks(evaluation_information.pawns[WHITE], WHITE);
    evaluation_information.pawn_attacks[BLACK] = get_pawn_bitboard_attacks(evaluation_information.pawns[BLACK], BLACK);

    for (auto & color_occupancy : evaluation_information.piece_relative_occupancies) {
        for (BITBOARD& piece_relative_occupancy : color_occupancy) {
            piece_relative_occupancy = position.all_pieces;
        }
    }

    BITBOARD white_diagonal_sliders = position.get_pieces(BISHOP, WHITE) | position.get_pieces(QUEEN, WHITE);
    BITBOARD black_diagonal_sliders = position.get_pieces(BISHOP, BLACK) | position.get_pieces(QUEEN, BLACK);

    BITBOARD white_orthogonal_sliders = position.get_pieces(ROOK, WHITE) | position.get_pieces(QUEEN, WHITE);
    BITBOARD black_orthogonal_sliders = position.get_pieces(ROOK, BLACK) | position.get_pieces(QUEEN, BLACK);

    evaluation_information.piece_relative_occupancies[WHITE][BISHOP] ^= white_diagonal_sliders;
    evaluation_information.piece_relative_occupancies[BLACK][BISHOP] ^= black_diagonal_sliders;

    evaluation_information.piece_relative_occupancies[WHITE][ROOK] ^= white_orthogonal_sliders;
    evaluation_information.piece_relative_occupancies[BLACK][ROOK] ^= black_orthogonal_sliders;

    evaluation_information.piece_relative_occupancies[WHITE][QUEEN] ^=
            white_diagonal_sliders | white_orthogonal_sliders;
    evaluation_information.piece_relative_occupancies[BLACK][QUEEN] ^=
            black_diagonal_sliders | black_orthogonal_sliders;
}

Square get_white_relative_square(Square square, Color color) {
    return static_cast<Square>(square ^ (color * 56));
}

Square get_black_relative_square(Square square, Color color) {
    return static_cast<Square>(square ^ (~color * 56));
}

SCORE_TYPE evaluate_king_pawn(const Position& position, File file, Color color, EvaluationInformation& evaluation_information, Trace& trace) {
    SCORE_TYPE score = 0;


    BITBOARD our_file_pawns = evaluation_information.pawns[color] & MASK_FILE[file];
    Square square = our_file_pawns == 0 ? NO_SQUARE :
            color == WHITE ? lsb(our_file_pawns) : msb(our_file_pawns);

    Rank relative_rank = rank_of(get_white_relative_square(square, color));

    int index = square == NO_SQUARE ? 4 : std::min(static_cast<int>(relative_rank) - 1, 3);

    score += KING_PAWN_SHIELD[index][file];
    trace.king_pawn_shield[index][file][color]++;

    // PAWN STORM
    BITBOARD opp_file_pawns = evaluation_information.pawns[~color] & MASK_FILE[file];
    square = opp_file_pawns == 0 ? NO_SQUARE :
            color == WHITE ? lsb(opp_file_pawns) : msb(opp_file_pawns);

    relative_rank = rank_of(get_white_relative_square(square, color));

    // Uses the relative ranks 2-7 (ranks 6 & 7 are combined into one index)
    index = square == NO_SQUARE ? 5 : std::min(static_cast<int>(relative_rank) - 1, 4);

    score += KING_PAWN_STORM[index][file];
    trace.king_pawn_storm[index][file][color]++;

    return score;
}

SCORE_TYPE evaluate_pawns(Position& position, Color color, EvaluationInformation& evaluation_information, Trace& trace) {

    Direction up = color == WHITE ? NORTH : SOUTH;

    SCORE_TYPE score = 0;
    BITBOARD our_pawns = evaluation_information.pawns[color];
    BITBOARD opp_pawns = evaluation_information.pawns[~color];
    BITBOARD phalanx_pawns = our_pawns & shift<WEST>(our_pawns);
    BITBOARD pawn_threats = evaluation_information.pawn_attacks[color] & evaluation_information.pieces[~color];

    // Doubled Pawns
    BITBOARD doubled_pawns = our_pawns & shift(up, our_pawns);
    score += static_cast<SCORE_TYPE>(popcount(doubled_pawns)) * DOUBLED_PAWN_PENALTY;
    trace.doubled_pawn_penalty[color] += popcount(doubled_pawns);

    // KING RING ATTACKS
    BITBOARD king_ring_attacks_1 = evaluation_information.pawn_attacks[color] &
            king_ring_zone.masks[0][evaluation_information.king_squares[~color]];
    BITBOARD king_ring_attacks_2 = evaluation_information.pawn_attacks[color] &
            king_ring_zone.masks[1][evaluation_information.king_squares[~color]];

    score += static_cast<SCORE_TYPE>(popcount(king_ring_attacks_1)) * KING_RING_ATTACKS[0][PAWN];
    score += static_cast<SCORE_TYPE>(popcount(king_ring_attacks_2)) * KING_RING_ATTACKS[1][PAWN];

    trace.king_ring_attacks[0][PAWN][color] += popcount(king_ring_attacks_1);
    trace.king_ring_attacks[1][PAWN][color] += popcount(king_ring_attacks_2);

    evaluation_information.total_king_ring_attacks[color] +=
            static_cast<int>(2 * popcount(king_ring_attacks_1) + popcount(king_ring_attacks_2));

    // MAIN PAWN EVAL
    while (our_pawns) {
        Square square = poplsb(our_pawns);
        BITBOARD bb_square = from_square(square);

        Square black_relative_square = get_black_relative_square(square, color);
        Rank relative_rank = rank_of(get_white_relative_square(square, color));

        score += PIECE_VALUES[PAWN];
        trace.piece_values[PAWN][color]++;

        score += PIECE_SQUARE_TABLES[PAWN][black_relative_square];
        trace.piece_square_tables[PAWN][black_relative_square][color]++;

        // evaluation_information.game_phase += GAME_PHASE_SCORES[PAWN];
        evaluation_information.piece_counts[color][PAWN]++;

        // PASSED PAWN
        if (!(passed_pawn_masks[color][square] & opp_pawns)) {
            auto protectors = popcount(evaluation_information.pawns[color] & get_piece_attacks(get_piece(PAWN, ~color), square, 0));

            score += PASSED_PAWN_BONUSES[protectors][relative_rank];
            trace.passed_pawn_bonuses[protectors][relative_rank][color]++;
            evaluation_information.passed_pawn_count[color]++;

            // BLOCKERS
            auto blocker_square = square + up;
            if (from_square(blocker_square) & evaluation_information.pieces[~color]) {
                score += PASSED_PAWN_BLOCKERS[get_piece_type(position.board[blocker_square], ~color)][rank_of(
                        get_white_relative_square(blocker_square, color))];
                trace.passed_pawn_blockers[get_piece_type(position.board[blocker_square], ~color)][rank_of(
                        get_white_relative_square(blocker_square, color))][color]++;
            }

            auto blocker_square_2 = blocker_square + up;
            if (relative_rank <= 5 && from_square(blocker_square_2) & evaluation_information.pieces[~color]) {
                score += PASSED_PAWN_BLOCKERS_2[get_piece_type(position.board[blocker_square_2], ~color)][rank_of(
                        get_white_relative_square(blocker_square_2, color))];
                trace.passed_pawn_blockers_2[get_piece_type(position.board[blocker_square_2], ~color)][rank_of(
                        get_white_relative_square(blocker_square_2, color))][color]++;
            }

            // Square of the Pawn
            auto promotion_square = get_black_relative_square(static_cast<Square>(square % 8), color);
            int our_distance = get_chebyshev_distance(square, promotion_square);
            int king_distance = get_chebyshev_distance(evaluation_information.king_squares[~color], promotion_square);

            if (std::min(our_distance, 5) < king_distance - (position.side != color)) {
                score += SQUARE_OF_THE_PAWN;
                trace.square_of_the_pawn[color]++;
            }
        }

        // BACKWARDS PAWN (We can use the passed pawn mask for the opposite color including the two squares next to it)
        BITBOARD backwards_pawn_mask = passed_pawn_masks[~color][square]
                                       | (shift<WEST>(bb_square)) | (shift<EAST>(bb_square));
        if (!(backwards_pawn_mask & evaluation_information.pawns[color])) {
            score += BACKWARDS_PAWN_PENALTY;
            trace.backwards_pawn_penalty[color]++;
        }

        // ISOLATED PAWN
        BITBOARD isolated_pawn_mask = fill<SOUTH>(fill<NORTH>(shift<WEST>(bb_square) | shift<EAST>(bb_square)));
        if (!(isolated_pawn_mask & evaluation_information.pawns[color])) {
            score += ISOLATED_PAWN_PENALTY;
            trace.isolated_pawn_penalty[color]++;
        }
    }

    // Phalanx Pawns
    while (phalanx_pawns) {
        Square square = poplsb(phalanx_pawns);
        Rank relative_rank = rank_of(get_white_relative_square(square, color));

        score += PHALANX_PAWN_BONUSES[relative_rank];
        trace.phalanx_pawn_bonuses[relative_rank][color]++;
    }

    while (pawn_threats) {
        Square square = poplsb(pawn_threats);
        score += PIECE_THREATS[PAWN][get_piece_type(position.board[square], ~color)];
        trace.piece_threats[PAWN][get_piece_type(position.board[square], ~color)][color]++;
    }

    return score;
}

template<PieceType piece_type>
SCORE_TYPE evaluate_piece(Position& position, Color color, EvaluationInformation& evaluation_information, Trace& trace) {
    SCORE_TYPE score = 0;
    BITBOARD pieces = position.get_pieces(piece_type, color);

    if constexpr (piece_type == BISHOP) {
        if (popcount(pieces) >= 2) {
            score += BISHOP_PAIR_BONUS;
            trace.bishop_pair_bonus[color]++;
        }
    }

    while (pieces) {
        Square square = poplsb(pieces);
        score += PIECE_VALUES[piece_type];
        trace.piece_values[piece_type][color]++;

        score += PIECE_SQUARE_TABLES[piece_type][get_black_relative_square(square, color)];
        trace.piece_square_tables[piece_type][get_black_relative_square(square, color)][color]++;

        evaluation_information.game_phase += GAME_PHASE_SCORES[piece_type];
        evaluation_information.piece_counts[color][piece_type]++;

        BITBOARD piece_attacks = get_piece_attacks(get_piece(piece_type, color), square,
                                                   evaluation_information.piece_relative_occupancies[color][piece_type]);

        if constexpr (piece_type != KING) {
            // MOBILITY
            BITBOARD mobility = piece_attacks &
                    (~evaluation_information.pieces[color]) &
                    (~evaluation_information.pawn_attacks[~color]);

            score += static_cast<SCORE_TYPE>(popcount(mobility)) * MOBILITY_VALUES[piece_type];
            trace.mobility_values[piece_type][color] += popcount(mobility);

            // KING RING ATTACKS
            BITBOARD king_ring_attacks_1 = piece_attacks & king_ring_zone.masks[0][evaluation_information.king_squares[~color]];
            BITBOARD king_ring_attacks_2 = piece_attacks & king_ring_zone.masks[1][evaluation_information.king_squares[~color]];

            score += static_cast<SCORE_TYPE>(popcount(king_ring_attacks_1)) * KING_RING_ATTACKS[0][piece_type];
            score += static_cast<SCORE_TYPE>(popcount(king_ring_attacks_2)) * KING_RING_ATTACKS[1][piece_type];

            trace.king_ring_attacks[0][piece_type][color] += popcount(king_ring_attacks_1);
            trace.king_ring_attacks[1][piece_type][color] += popcount(king_ring_attacks_2);

            evaluation_information.total_king_ring_attacks[color] +=
                    static_cast<int>(2 * popcount(king_ring_attacks_1) + popcount(king_ring_attacks_2));

            // OPPONENT KING TROPISM
            int distance_to_opp_king = get_manhattan_distance(square, evaluation_information.king_squares[~color]);
            score += OPP_KING_TROPISM[piece_type] * distance_to_opp_king;
            trace.opp_king_tropism[piece_type][color] += distance_to_opp_king;
        }

        if constexpr (piece_type == KING || piece_type == QUEEN || piece_type == ROOK) {
            if (!(MASK_FILE[file_of(square)] & evaluation_information.pawns[color])) {
                if (!(MASK_FILE[file_of(square)] & evaluation_information.pawns[~color])) {
                    score += OPEN_FILE_VALUES[piece_type];
                    trace.open_file_values[piece_type][color]++;
                }
                else {
                    score += SEMI_OPEN_FILE_VALUES[piece_type];
                    trace.semi_open_file_values[piece_type][color]++;
                }
            }
        }

        if constexpr (piece_type == KING) {
            File file = file_of(square);
            if (file <= 2) {  // Queen side: Files A, B, C  (0, 1, 2)
                score += evaluate_king_pawn(position, 0, color, evaluation_information, trace);
                score += evaluate_king_pawn(position, 1, color, evaluation_information, trace);
                score += evaluate_king_pawn(position, 2, color, evaluation_information, trace);
            }

            else if (file >= 5) {  // King side: Files F, G, H  (5, 6, 7)
                score += evaluate_king_pawn(position, 5, color, evaluation_information, trace);
                score += evaluate_king_pawn(position, 6, color, evaluation_information, trace);
                score += evaluate_king_pawn(position, 7, color, evaluation_information, trace);
            }
        }

        for (int opp_piece = 0; opp_piece < 6; opp_piece++) {
            score += static_cast<SCORE_TYPE>(popcount(
                    piece_attacks & position.get_pieces(static_cast<PieceType>(opp_piece), ~color))) *
                    PIECE_THREATS[piece_type][opp_piece];

            trace.piece_threats[piece_type][opp_piece][color] += popcount(
                    piece_attacks & position.get_pieces(static_cast<PieceType>(opp_piece), ~color));
        }
    }

    return score;
}

SCORE_TYPE evaluate_pieces(Position& position, EvaluationInformation& evaluation_information, Trace& trace) {
    SCORE_TYPE score = 0;

    score += evaluate_pawns(position, WHITE, evaluation_information, trace);
    score -= evaluate_pawns(position, BLACK, evaluation_information, trace);

    score += evaluate_piece<KNIGHT>(position, WHITE, evaluation_information, trace);
    score -= evaluate_piece<KNIGHT>(position, BLACK, evaluation_information, trace);

    score += evaluate_piece<BISHOP>(position, WHITE, evaluation_information, trace);
    score -= evaluate_piece<BISHOP>(position, BLACK, evaluation_information, trace);

    score += evaluate_piece<ROOK>(position, WHITE, evaluation_information, trace);
    score -= evaluate_piece<ROOK>(position, BLACK, evaluation_information, trace);

    score += evaluate_piece<QUEEN>(position, WHITE, evaluation_information, trace);
    score -= evaluate_piece<QUEEN>(position, BLACK, evaluation_information, trace);

    score += evaluate_piece<KING>(position, WHITE, evaluation_information, trace);
    score -= evaluate_piece<KING>(position, BLACK, evaluation_information, trace);

    return score;
}

double evaluate_drawishness(Position& position, EvaluationInformation& evaluation_information) {

    // This function returns a decimal from 0.0 - 1.0 that will be used to scale the total evaluation

    // Get material counts
    const SCORE_TYPE white_material = evaluation_information.piece_counts[WHITE][PAWN] * CANONICAL_PIECE_VALUES[PAWN] +
                                      evaluation_information.piece_counts[WHITE][KNIGHT] * CANONICAL_PIECE_VALUES[KNIGHT] +
                                      evaluation_information.piece_counts[WHITE][BISHOP] * CANONICAL_PIECE_VALUES[BISHOP] +
                                      evaluation_information.piece_counts[WHITE][ROOK] * CANONICAL_PIECE_VALUES[ROOK] +
                                      evaluation_information.piece_counts[WHITE][QUEEN] * CANONICAL_PIECE_VALUES[QUEEN];

    const SCORE_TYPE black_material = evaluation_information.piece_counts[BLACK][PAWN] * CANONICAL_PIECE_VALUES[PAWN] +
                                      evaluation_information.piece_counts[BLACK][KNIGHT] * CANONICAL_PIECE_VALUES[KNIGHT] +
                                      evaluation_information.piece_counts[BLACK][BISHOP] * CANONICAL_PIECE_VALUES[BISHOP] +
                                      evaluation_information.piece_counts[BLACK][ROOK] * CANONICAL_PIECE_VALUES[ROOK] +
                                      evaluation_information.piece_counts[BLACK][QUEEN] * CANONICAL_PIECE_VALUES[QUEEN];

    Color more_material_side = white_material >= black_material ? WHITE : BLACK;
    Color less_material_side = ~more_material_side;

    SCORE_TYPE more_material = std::max(white_material, black_material);
    SCORE_TYPE less_material = std::min(white_material, black_material);

    // If there are too many rooks, don't evaluate for a draw
    if (evaluation_information.piece_counts[WHITE][ROOK] + evaluation_information.piece_counts[BLACK][ROOK] >= 3) return 1.0;

    // There is a queen on the board
    if (evaluation_information.piece_counts[WHITE][QUEEN] + evaluation_information.piece_counts[BLACK][QUEEN] >= 1) {
        // Guard clause
        if (more_material > CANONICAL_PIECE_VALUES[QUEEN]) return 1.0;

        // Queen vs rook + knight, queen vs rook + bishop, etc.
        if (less_material >= CANONICAL_PIECE_VALUES[ROOK] + MIN_MINOR_PIECE_VALUE) return 0.03;

        // Queen vs two bishops, queen vs two knights, queen vs bishop + knight, etc.
        if (less_material >= 2 * MIN_MINOR_PIECE_VALUE) return 0.25;

        // Queen vs rook + pawn, possible fortress
        if (less_material >= CANONICAL_PIECE_VALUES[ROOK] + CANONICAL_PIECE_VALUES[PAWN]) return 0.6;

        return 1.0;
    }

    // There are pawns on the board
    if (evaluation_information.piece_counts[WHITE][PAWN] + evaluation_information.piece_counts[BLACK][PAWN] >= 1) {

        // Single pawn imbalance drawish endgames
        if (evaluation_information.piece_counts[WHITE][PAWN] + evaluation_information.piece_counts[BLACK][PAWN] == 1) {

            // Pawn vs 2 knights
            if (more_material == 2 * CANONICAL_PIECE_VALUES[KNIGHT]) return 0.01;

            // Pawn + 2 knights vs minor piece / rook
            if (more_material == CANONICAL_PIECE_VALUES[PAWN] + 2 * CANONICAL_PIECE_VALUES[KNIGHT] &&
                less_material >= MIN_MINOR_PIECE_VALUE) {
                return 0.27;
            }

            // Pawn vs minor piece
            if (less_material == CANONICAL_PIECE_VALUES[PAWN] && more_material <= MAX_MINOR_PIECE_VALUE) return 0.02;

            // Pawn + minor vs minor
            if (less_material >= MIN_MINOR_PIECE_VALUE && more_material <= CANONICAL_PIECE_VALUES[PAWN] + MAX_MINOR_PIECE_VALUE) return 0.19;

            // Pawn + minor vs rook or Pawn + minor vs 2 minor pieces
            if (less_material >= CANONICAL_PIECE_VALUES[PAWN] + MIN_MINOR_PIECE_VALUE && more_material <= 2 * MAX_MINOR_PIECE_VALUE &&
                evaluation_information.piece_counts[less_material_side][PAWN] == 1) return 0.1;

            // Pawn + rook vs rook + minor
            if (less_material >= CANONICAL_PIECE_VALUES[PAWN] + CANONICAL_PIECE_VALUES[ROOK] &&
                more_material <= CANONICAL_PIECE_VALUES[ROOK] + MAX_MINOR_PIECE_VALUE) return 0.1;

            // Wrong Colored Bishop
            if (less_material == 0 && more_material == CANONICAL_PIECE_VALUES[BISHOP] + CANONICAL_PIECE_VALUES[PAWN]) {
                BITBOARD pawn_bitboard = evaluation_information.pawns[more_material_side];

                // Pawn is a rook pawn
                if (pawn_bitboard & (MASK_FILE[FILE_A] | MASK_FILE[FILE_H])) {

                    Square promotion_square = get_black_relative_square(static_cast<Square>(lsb(pawn_bitboard) % 8),
                                                                        more_material_side);

                    bool king_in_reach = position.get_pieces(KING, less_material_side) &
                                         (king_ring_zone.masks[0][promotion_square] | from_square(promotion_square));

                    bool wrong_colored_bishop = !same_color(promotion_square,
                                                            static_cast<Square>(lsb(position.get_pieces(BISHOP,
                                                                                                        more_material_side))));

                    if (king_in_reach && wrong_colored_bishop) return 0.0;
                }
            }
        }

        // Double pawn imbalance drawish endgames
        if (evaluation_information.piece_counts[WHITE][PAWN] + evaluation_information.piece_counts[BLACK][PAWN] == 2) {

            // Two Pawns vs Minor Piece (Search should cover the case where two pawns will promote)
            if (less_material == 2 * CANONICAL_PIECE_VALUES[PAWN] && more_material <= MAX_MINOR_PIECE_VALUE) return 0.3;

            // Two Pawns vs Two Knights (Search should cover the case where two pawns will promote)
            if (less_material == 2 * CANONICAL_PIECE_VALUES[PAWN] && more_material == 2 * CANONICAL_PIECE_VALUES[KNIGHT]) return 0.3;

        }

        // if (evaluation_information.piece_counts[WHITE][KNIGHT] + evaluation_information.piece_counts[WHITE][ROOK] +
        //     evaluation_information.piece_counts[BLACK][KNIGHT] + evaluation_information.piece_counts[BLACK][ROOK] >= 1) return 1.0;

        return 1.0;
    }

    // At this point, we know there are no queens on the board, no pawns on the board,
    // and less than or equal to 2 rooks.

    // Always a draw when the side with more material has less than or equal to a minor piece
    if (more_material <= MAX_MINOR_PIECE_VALUE) return 0.0;

    // When the side with more material has less than or equal to two minor pieces
    if (more_material <= 2 * MAX_MINOR_PIECE_VALUE) {

        // With only 2 knights, it's impossible to checkmate
        if (evaluation_information.piece_counts[WHITE][KNIGHT] == 2 || evaluation_information.piece_counts[BLACK][KNIGHT] == 2)
            return 0.0;

        // If one of them has 0 pieces we know that due to the check above, at least one of has more than
        // a bishop's worth of material, or else it would have returned 0.0, and thus it would be a win.
        // Either the player that doesn't have 0 material has two bishops, a bishop and knight, or a rook.
        // All of these are wins.
        if (less_material == 0) {
            return 1.0;
        }

        // Here we know they both do not have 0 material, and they cannot have pawns or queens,
        // this means they either have a rook, and the other player has a minor piece,
        // or this means one player has two minor pieces, and the other players has one minor piece.
        return 0.13;
    }

    // Rook + Minor piece imbalances
    if (more_material <= CANONICAL_PIECE_VALUES[ROOK] + MAX_MINOR_PIECE_VALUE) {
        if (less_material >= 2 * MIN_MINOR_PIECE_VALUE) return 0.09;
        if (less_material == CANONICAL_PIECE_VALUES[ROOK]) return 0.14;
    }

    return 1.0;

}


double evaluate_opposite_colored_bishop_endgames(Position& position, EvaluationInformation& evaluation_information) {

    bool opposite_colored_bishops = evaluation_information.piece_counts[WHITE][BISHOP] == 1 &&
                                    evaluation_information.piece_counts[BLACK][BISHOP] == 1 &&
                                    !same_color(static_cast<Square>(lsb(position.get_pieces(BISHOP, WHITE))),
                                                static_cast<Square>(lsb(position.get_pieces(BISHOP, BLACK))));

    if (opposite_colored_bishops &&
        evaluation_information.piece_counts[WHITE][QUEEN] + evaluation_information.piece_counts[BLACK][QUEEN] +
        evaluation_information.piece_counts[WHITE][KNIGHT] + evaluation_information.piece_counts[BLACK][KNIGHT] +
        evaluation_information.piece_counts[WHITE][ROOK] + evaluation_information.piece_counts[BLACK][ROOK] == 0 &&
        evaluation_information.piece_counts[WHITE][PAWN] + evaluation_information.piece_counts[BLACK][PAWN] >= 1) {

        Color more_pawns_side = (evaluation_information.piece_counts[WHITE][PAWN] >
                                 evaluation_information.piece_counts[BLACK][PAWN]) ? WHITE : BLACK;

        int pawn_difference = evaluation_information.piece_counts[more_pawns_side][PAWN] -
                              evaluation_information.piece_counts[~more_pawns_side][PAWN];

        if (pawn_difference <= 1) return std::min(0.13 +
                                                  evaluation_information.passed_pawn_count[more_pawns_side] *
                                                  evaluation_information.passed_pawn_count[more_pawns_side] * 0.14, 1.0);

        if (pawn_difference >= 3) return std::min(evaluation_information.passed_pawn_count[more_pawns_side] * 0.38
                                                  + pawn_difference * 0.04, 1.0);

        // Pawn Difference must equal two here
        if (evaluation_information.passed_pawn_count[more_pawns_side] >= 3) return 1.0;

        if (evaluation_information.piece_counts[more_pawns_side][PAWN] > 2)
            return std::min(0.2 + evaluation_information.passed_pawn_count[more_pawns_side] *
                                  evaluation_information.passed_pawn_count[more_pawns_side] * 0.14, 1.0);

        BITBOARD more_pawns_bitboard = position.get_pieces(PAWN, more_pawns_side);

        // There are only two pawns here, and they must be passed
        auto pawn_1 = static_cast<Square>(lsb(more_pawns_bitboard));
        auto pawn_2 = static_cast<Square>(msb(more_pawns_bitboard));

        int file_difference = abs(static_cast<int>(file_of(pawn_1)) - static_cast<int>(file_of(pawn_2))) - 1;
        if (file_difference > 2) return 0.85;

        if (file_difference == 1) return 0.1;
        if (file_difference == 2) {
            if (more_pawns_bitboard & (MASK_FILE[FILE_A] | MASK_FILE[FILE_H])) return 0.05;
            if (more_pawns_bitboard & (MASK_FILE[FILE_B] | MASK_FILE[FILE_G])) return 0.15;
            return 0.7;
        }

        // Pawns must be connected here
        if (more_pawns_bitboard & (MASK_FILE[FILE_A] | MASK_FILE[FILE_H])) return 0.05;

        return 0.15;
    }

    return 1.0;
}

SCORE_TYPE evaluate(Position& position, Trace& trace) {

    EvaluationInformation evaluation_information{};
    initialize_evaluation_information(position, evaluation_information);

    SCORE_TYPE score = 0;
    int game_phase = 0;

    score += evaluate_pieces(position, evaluation_information, trace);

    evaluation_information.total_king_ring_attacks[WHITE] = std::min<int>(evaluation_information.total_king_ring_attacks[WHITE], 39);
    evaluation_information.total_king_ring_attacks[BLACK] = std::min<int>(evaluation_information.total_king_ring_attacks[BLACK], 39);

    score += TOTAL_KING_RING_ATTACKS[evaluation_information.total_king_ring_attacks[WHITE]];
    score -= TOTAL_KING_RING_ATTACKS[evaluation_information.total_king_ring_attacks[BLACK]];

    trace.total_king_ring_attacks[evaluation_information.total_king_ring_attacks[WHITE]][WHITE]++;
    trace.total_king_ring_attacks[evaluation_information.total_king_ring_attacks[BLACK]][BLACK]++;

    score += (position.side * -2 + 1) * TEMPO_BONUS;
    trace.tempo_bonus[position.side]++;

    game_phase = std::min(game_phase, 24);

    SCORE_TYPE evaluation = (mg_score(score) * game_phase + eg_score(score) * (24 - game_phase)) / 24;

    double drawishness = evaluate_drawishness(position, evaluation_information);
    double opposite_colored_bishop_scale_factor = evaluate_opposite_colored_bishop_endgames(position, evaluation_information);

    evaluation = static_cast<SCORE_TYPE>(evaluation * drawishness * opposite_colored_bishop_scale_factor);

    return (position.side * -2 + 1) * evaluation;
}


// --------------------------------------------------
//                   TUNING STUFF
// --------------------------------------------------


static int32_t round_value(tune_t value)
{
    return static_cast<int32_t>(round(value));
}

static void print_parameter(std::stringstream& ss, const pair_t parameter)
{
    const auto mg = round_value(parameter[static_cast<int32_t>(PhaseStages::Midgame)]);
    const auto eg = round_value(parameter[static_cast<int32_t>(PhaseStages::Endgame)]);

    auto mg_size = std::to_string(mg).size();
    auto eg_size = std::to_string(eg).size();

    ss << "S(";

    for (int i = 0; i < 4 - mg_size; i++) {
        ss << " ";
    }

    ss << mg << ",";

    for (int i = 0; i < 4 - eg_size; i++) {
        ss << " ";
    }

    ss << eg << ")";

}

static void print_single(std::stringstream& ss, const parameters_t& parameters, int& index, const std::string& name) {
    ss << "constexpr SCORE_TYPE " << name << " = ";
    print_parameter(ss, parameters[index]);
    index++;

    ss << ";" << endl << endl;
}

static void print_array(std::stringstream& ss, const parameters_t& parameters, int& index, const std::string& name, int count) {
    ss << "constexpr SCORE_TYPE " << name << "[" << count << "] = {";

    if (count == 64) ss << "\n\t";

    for (auto i = 0; i < count; i++) {
        print_parameter(ss, parameters[index]);
        index++;

        if (i != count - 1) ss << ", ";
        if (count == 64 && (i + 1) % 8 == 0 && i != count - 1) ss << "\n\t";
    }

    if (count == 64) ss << "\n";
    ss << "};" << endl << endl;
}

static void print_array_2d(std::stringstream& ss, const parameters_t& parameters, int& index, const std::string& name, int count1, int count2) {
    ss << "constexpr SCORE_TYPE " << name << "[" << count1 << "][" << count2 << "] = {\n";

    for (auto i = 0; i < count1; i++) {
        ss << "\t{";
        if (count2 == 64) ss << "\n\t\t";

        for (auto j = 0; j < count2; j++) {
            print_parameter(ss, parameters[index]);
            index++;

            if (j != count2 - 1) ss << ", ";

            if (count2 == 64 && (j + 1) % 8 == 0 && j != count2 - 1) ss << "\n\t\t";
        }

        if (count2 == 64) ss << "\n\t";
        ss << "}";
        if (i != count1 - 1) ss << ",";
        ss << "\n";

    }

    ss << "};\n" << endl << endl;
}


static void rebalance_piece_square_tables(parameters_t& parameters, const int material_offset, const int pst_offset) {

    // loop through all pieces excluding the king
    for (int piece = 0; piece <= 4; piece++) {

        for (int stage = 0; stage < 2; stage++) {

            double sum = 0;

            for (int i = 0; i < 64; i++) {
                const int pst_index = pst_offset + piece * 64 + i;
                sum += parameters[pst_index][stage];
            }

            const double average = sum / 64;

            parameters[material_offset + piece][stage] += average;

            for (int i = 0; i < 64; i++) {
                const int pst_index = pst_offset + piece * 64 + i;
                parameters[pst_index][stage] -= average;
            }

        }
    }
}


static coefficients_t get_coefficients(const Trace& trace)
{
    coefficients_t coefficients;
    get_coefficient_array(coefficients, trace.piece_values, 6);
    get_coefficient_array_2d(coefficients, trace.piece_square_tables, 6, 64);

    get_coefficient_array_2d(coefficients, trace.passed_pawn_bonuses, 3, 8);
    get_coefficient_array_2d(coefficients, trace.passed_pawn_blockers, 6, 8);
    get_coefficient_array_2d(coefficients, trace.passed_pawn_blockers_2, 6, 8);

    get_coefficient_array(coefficients, trace.phalanx_pawn_bonuses, 8);

    get_coefficient_single(coefficients, trace.isolated_pawn_penalty);

    get_coefficient_single(coefficients, trace.bishop_pair_bonus);

    get_coefficient_single(coefficients, trace.tempo_bonus);

    get_coefficient_array(coefficients, trace.mobility_values, 6);

    get_coefficient_array(coefficients, trace.semi_open_file_values, 6);
    get_coefficient_array(coefficients, trace.open_file_values, 6);

    get_coefficient_array_2d(coefficients, trace.piece_threats, 6, 6);

    get_coefficient_array_2d(coefficients, trace.king_ring_attacks, 2, 6);
    get_coefficient_array(coefficients, trace.total_king_ring_attacks, 40);

    get_coefficient_array_2d(coefficients, trace.king_pawn_shield, 5, 8);
    get_coefficient_array_2d(coefficients, trace.king_pawn_storm, 6, 8);

    get_coefficient_array(coefficients, trace.opp_king_tropism, 6);

    get_coefficient_single(coefficients, trace.doubled_pawn_penalty);

    get_coefficient_single(coefficients, trace.square_of_the_pawn);

    get_coefficient_single(coefficients, trace.backwards_pawn_penalty);

    return coefficients;
}


parameters_t AltairEval::get_initial_parameters() {
    parameters_t parameters;
    get_initial_parameter_array(parameters, PIECE_VALUES, 6);
    get_initial_parameter_array_2d(parameters, PIECE_SQUARE_TABLES, 6, 64);

    get_initial_parameter_array_2d(parameters, PASSED_PAWN_BONUSES, 3, 8);
    get_initial_parameter_array_2d(parameters, PASSED_PAWN_BLOCKERS, 6, 8);
    get_initial_parameter_array_2d(parameters, PASSED_PAWN_BLOCKERS_2, 6, 8);

    get_initial_parameter_array(parameters, PHALANX_PAWN_BONUSES, 8);

    get_initial_parameter_single(parameters, ISOLATED_PAWN_PENALTY);

    get_initial_parameter_single(parameters, BISHOP_PAIR_BONUS);

    get_initial_parameter_single(parameters, TEMPO_BONUS);

    get_initial_parameter_array(parameters, MOBILITY_VALUES, 6);

    get_initial_parameter_array(parameters, SEMI_OPEN_FILE_VALUES, 6);
    get_initial_parameter_array(parameters, OPEN_FILE_VALUES, 6);

    get_initial_parameter_array_2d(parameters, PIECE_THREATS, 6, 6);

    get_initial_parameter_array_2d(parameters, KING_RING_ATTACKS, 2, 6);
    get_initial_parameter_array(parameters, TOTAL_KING_RING_ATTACKS, 40);

    get_initial_parameter_array_2d(parameters, KING_PAWN_SHIELD, 5, 8);
    get_initial_parameter_array_2d(parameters, KING_PAWN_STORM, 6, 8);

    get_initial_parameter_array(parameters, OPP_KING_TROPISM, 6);

    get_initial_parameter_single(parameters, DOUBLED_PAWN_PENALTY);

    get_initial_parameter_single(parameters, SQUARE_OF_THE_PAWN);

    get_initial_parameter_single(parameters, BACKWARDS_PAWN_PENALTY);

    return parameters;
}


void AltairEval::print_parameters(const parameters_t &parameters) {
    parameters_t parameters_copy = parameters;
    rebalance_piece_square_tables(parameters_copy, 0, 6);

    int index = 0;
    stringstream ss;

    print_array(ss, parameters_copy, index, "PIECE_VALUES", 6);
    print_array_2d(ss, parameters_copy, index, "PIECE_SQUARE_TABLES", 6, 64);

    print_array_2d(ss, parameters_copy, index, "PASSED_PAWN_BONUSES", 3, 8);
    print_array_2d(ss, parameters_copy, index, "PASSED_PAWN_BLOCKERS", 6, 8);
    print_array_2d(ss, parameters_copy, index, "PASSED_PAWN_BLOCKERS_2", 6, 8);

    print_array(ss, parameters_copy, index, "PHALANX_PAWN_BONUSES", 8);

    print_single(ss, parameters_copy, index, "ISOLATED_PAWN_PENALTY");

    print_single(ss, parameters_copy, index, "BISHOP_PAIR_BONUS");

    print_single(ss, parameters_copy, index, "TEMPO_BONUS");

    print_array(ss, parameters_copy, index, "MOBILITY_VALUES", 6);

    print_array(ss, parameters_copy, index, "SEMI_OPEN_FILE_VALUES", 6);
    print_array(ss, parameters_copy, index, "OPEN_FILE_VALUES", 6);

    print_array_2d(ss, parameters_copy, index, "PIECE_THREATS", 6, 6);

    print_array_2d(ss, parameters_copy, index, "KING_RING_ATTACKS", 2, 6);
    print_array(ss, parameters_copy, index, "TOTAL_KING_RING_ATTACKS", 40);

    print_array_2d(ss, parameters_copy, index, "KING_PAWN_SHIELD", 5, 8);
    print_array_2d(ss, parameters_copy, index, "KING_PAWN_STORM", 6, 8);

    print_array(ss, parameters_copy, index, "OPP_KING_TROPISM", 6);

    print_single(ss, parameters_copy, index, "DOUBLED_PAWN_PENALTY");

    print_single(ss, parameters_copy, index, "SQUARE_OF_THE_PAWN");

    print_single(ss, parameters_copy, index, "BACKWARDS_PAWN_PENALTY");

    std::cout << ss.str() << "\n";
}


Position get_position_from_external(const Chess::Board& board)
{
    Position position;

    position.side = WHITE;

    position.pieces[WHITE_PAWN]   = board.pieces(Chess::PieceType::PAWN, Chess::Color::WHITE);
    position.pieces[WHITE_KNIGHT] = board.pieces(Chess::PieceType::KNIGHT, Chess::Color::WHITE);
    position.pieces[WHITE_BISHOP] = board.pieces(Chess::PieceType::BISHOP, Chess::Color::WHITE);
    position.pieces[WHITE_ROOK]   = board.pieces(Chess::PieceType::ROOK, Chess::Color::WHITE);
    position.pieces[WHITE_QUEEN]  = board.pieces(Chess::PieceType::QUEEN, Chess::Color::WHITE);
    position.pieces[WHITE_KING]   = board.pieces(Chess::PieceType::KING, Chess::Color::WHITE);

    position.pieces[BLACK_PAWN]   = board.pieces(Chess::PieceType::PAWN, Chess::Color::BLACK);
    position.pieces[BLACK_KNIGHT] = board.pieces(Chess::PieceType::KNIGHT, Chess::Color::BLACK);
    position.pieces[BLACK_BISHOP] = board.pieces(Chess::PieceType::BISHOP, Chess::Color::BLACK);
    position.pieces[BLACK_ROOK]   = board.pieces(Chess::PieceType::ROOK, Chess::Color::BLACK);
    position.pieces[BLACK_QUEEN]  = board.pieces(Chess::PieceType::QUEEN, Chess::Color::BLACK);
    position.pieces[BLACK_KING]   = board.pieces(Chess::PieceType::KING, Chess::Color::BLACK);

    for (auto & i : position.board) {
        i = EMPTY;
    }

    for (int i = 0; i < 12; i++) {
        auto piece = static_cast<Piece>(i);
        BITBOARD pieces = position.pieces[piece];

        while (pieces) {
            Square square = poplsb(pieces);
            position.board[square] = piece;
        }
    }

    position.ep_square = static_cast<Square>(static_cast<int>(board.enpassantSquare()));
    if(position.ep_square == 64)
    {
        position.ep_square = NO_SQUARE;
    }

    position.castle_ability_bits = 0;

    position.castle_ability_bits |= 1 * board.castlingRights().hasCastlingRight(Chess::Color::WHITE, Chess::CastleSide::KING_SIDE);
    position.castle_ability_bits |= 2 * board.castlingRights().hasCastlingRight(Chess::Color::BLACK, Chess::CastleSide::QUEEN_SIDE);
    position.castle_ability_bits |= 4 * board.castlingRights().hasCastlingRight(Chess::Color::WHITE, Chess::CastleSide::KING_SIDE);
    position.castle_ability_bits |= 8 * board.castlingRights().hasCastlingRight(Chess::Color::BLACK, Chess::CastleSide::QUEEN_SIDE);

    position.our_pieces = position.get_our_pieces();
    position.opp_pieces = position.get_opp_pieces();
    position.all_pieces = position.get_all_pieces();
    position.empty_squares = position.get_empty_squares();

    return position;
}

EvalResult AltairEval::get_fen_eval_result(const string &fen) {
    Position position;
    position.set_fen(fen);

    Trace trace{};
    trace.score = evaluate(position, trace);

    EvalResult result;
    result.coefficients = get_coefficients(trace);
    result.score = trace.score;

    return result;
}

EvalResult AltairEval::get_external_eval_result(const Chess::Board& board)
{
    auto position = get_position_from_external(board);

    Trace trace{};
    trace.score = evaluate(position, trace);

    EvalResult result;
    result.coefficients = get_coefficients(trace);
    result.score = trace.score;
    result.endgame_scale = 1;

    return result;
}