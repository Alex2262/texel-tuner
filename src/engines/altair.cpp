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

Square get_white_relative_square(Square square, Color color) {
    return static_cast<Square>(square ^ (color * 56));
}

Square get_black_relative_square(Square square, Color color) {
    return static_cast<Square>(square ^ (~color * 56));
}

SCORE_TYPE evaluate_pawns(Position& position, Color color, int& game_phase, Trace& trace) {
    SCORE_TYPE score = 0;
    BITBOARD our_pawns = position.get_pieces(PAWN, color);
    BITBOARD opp_pawns = position.get_pieces(PAWN, ~color);

    while (our_pawns) {
        Square square = poplsb(our_pawns);
        Square black_relative_square = get_black_relative_square(square, color);
        Rank relative_rank = rank_of(get_white_relative_square(square, color));

        score += PIECE_VALUES[PAWN];
        trace.piece_values[PAWN][color]++;

        score += PIECE_SQUARE_TABLES[PAWN][black_relative_square];
        trace.piece_square_tables[PAWN][black_relative_square][color]++;

        game_phase += GAME_PHASE_SCORES[PAWN];

        Direction up = color == WHITE ? NORTH : SOUTH;

        // PASSED PAWN
        if (!(passed_pawn_masks[color][square] & opp_pawns)) {
            score += PASSED_PAWN_BONUSES[relative_rank];
            trace.passed_pawn_bonuses[relative_rank][color]++;

            // BLOCKERS
            auto blocker_square = static_cast<Square>(square + static_cast<Square>(up));
            if (from_square(blocker_square) & position.get_pieces(~color)) {
                score += PASSED_PAWN_BLOCKERS[get_piece_type(position.board[blocker_square], ~color)][rank_of(
                        get_white_relative_square(blocker_square, color))];
                trace.passed_pawn_blockers[get_piece_type(position.board[blocker_square], ~color)][rank_of(
                        get_white_relative_square(blocker_square, color))][color]++;
            }
        }
    }

    return score;
}


SCORE_TYPE evaluate_piece(Position& position, PieceType piece_type, Color color, int& game_phase, Trace& trace) {
    SCORE_TYPE score = 0;
    BITBOARD pieces = position.get_pieces(piece_type, color);

    while (pieces) {
        Square square = poplsb(pieces);
        score += PIECE_VALUES[piece_type];
        trace.piece_values[piece_type][color]++;

        score += PIECE_SQUARE_TABLES[piece_type][get_black_relative_square(square, color)];
        trace.piece_square_tables[piece_type][get_black_relative_square(square, color)][color]++;

        game_phase += GAME_PHASE_SCORES[piece_type];
    }

    return score;
}

SCORE_TYPE evaluate_pieces(Position& position, int& game_phase, Trace& trace) {
    SCORE_TYPE score = 0;

    score += evaluate_pawns(position, WHITE, game_phase, trace);
    score -= evaluate_pawns(position, BLACK, game_phase, trace);

    for (int piece = 1; piece < 6; piece++) {
        score += evaluate_piece(position, static_cast<PieceType>(piece), WHITE, game_phase, trace);
        score -= evaluate_piece(position, static_cast<PieceType>(piece), BLACK, game_phase, trace);
    }

    return score;
}

SCORE_TYPE evaluate(Position& position, Trace& trace) {

    SCORE_TYPE score = 0;
    int game_phase = 0;

    score += evaluate_pieces(position, game_phase, trace);
    game_phase = std::min(game_phase, 24);

    SCORE_TYPE evaluation = (mg_score(score) * game_phase + eg_score(score) * (24 - game_phase)) / 24;

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

    get_coefficient_array(coefficients, trace.passed_pawn_bonuses, 8);

    get_coefficient_array_2d(coefficients, trace.passed_pawn_blockers, 6, 8);

    return coefficients;
}


parameters_t AltairEval::get_initial_parameters() {
    parameters_t parameters;
    get_initial_parameter_array(parameters, PIECE_VALUES, 6);
    get_initial_parameter_array_2d(parameters, PIECE_SQUARE_TABLES, 6, 64);

    get_initial_parameter_array(parameters, PASSED_PAWN_BONUSES, 8);

    get_initial_parameter_array_2d(parameters, PASSED_PAWN_BLOCKERS, 6, 8);

    return parameters;
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


void AltairEval::print_parameters(const parameters_t &parameters) {
    parameters_t parameters_copy = parameters;
    rebalance_piece_square_tables(parameters_copy, 0, 6);

    int index = 0;
    stringstream ss;

    print_array(ss, parameters_copy, index, "PIECE_VALUES", 6);
    print_array_2d(ss, parameters_copy, index, "PIECE_SQUARE_TABLES", 6, 64);

    print_array(ss, parameters_copy, index, "PASSED_PAWN_BONUSES", 8);

    print_array_2d(ss, parameters_copy, index, "PASSED_PAWN_BLOCKERS", 6, 8);

    std::cout << ss.str() << "\n";
}
