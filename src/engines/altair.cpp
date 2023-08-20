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
}

int evaluate(Position& position, Trace& trace) {
    int scores_mid[] = {0, 0};
    int scores_end[] = {0, 0};
    int phase = 0;

    for (int piece = 0; piece < 12; piece++) {
        auto color = static_cast<Color>(piece >= 6);
        PieceType pieceType = get_piece_type(static_cast<Piece>(piece), color);
        BITBOARD pieceBB = position.get_pieces(static_cast<Piece>(piece));

        phase += popcount(pieceBB) * GAME_PHASES[pieceType];
        scores_mid[color] += popcount(pieceBB) * PIECE_VALUES_MID[pieceType];
        scores_end[color] += popcount(pieceBB) * PIECE_VALUES_END[pieceType];
        trace.piece_values[pieceType][color] += popcount(pieceBB);

        while (pieceBB) {
            Square square = poplsb(pieceBB);
            auto relative_square = static_cast<Square>(square ^ (56 * ~color));

            scores_mid[color] += PIECE_SQUARE_TABLES_MID[pieceType][relative_square];
            scores_end[color] += PIECE_SQUARE_TABLES_END[pieceType][relative_square];
            trace.piece_square_tables[pieceType][relative_square][color]++;
        }
    }

    scores_mid[position.side] += TEMPO_MID;
    scores_end[position.side] += TEMPO_END;
    trace.tempo[position.side]++;

    return ((scores_mid[0] - scores_mid[1]) * phase + (scores_end[0] - scores_end[1]) * (24 - phase)) / 24 * (position.side == WHITE ? 1 : -1);
}


// --------------------------------------------------
//                   TUNING STUFF
// --------------------------------------------------


static int32_t round_value(tune_t value)
{
    return static_cast<int32_t>(round(value));
}

static void print_parameter(std::stringstream& ss, const tune_t parameter)
{

    const auto param = static_cast<SCORE_TYPE>(parameter + 0.5 - (parameter < 0.0));
    auto param_size = std::to_string(param).size();
    for (int i = 0; i < 4 - param_size; i++) {
        ss << " ";
    }
    ss << param;

}

static void print_single(std::stringstream& ss, const parameters_t& parameters, int& index, const std::string& name)
{
    string type = "int ";
    ss << type << name << "_MID = ";
    print_parameter(ss, parameters[index][0]);
    ss << ";" << endl;

    ss << type << name << "_END = ";
    print_parameter(ss, parameters[index][1]);
    ss << ";\n" << endl;

    index++;
}

static void print_array(std::stringstream& ss, const parameters_t& parameters, int& index, const std::string& name, int count)
{
    for (int c = 0; c < 2; c++) {

        string type = "int ";
        if (c == 0) {
            ss << type << name << "_MID[" << std::to_string(count) << "] = {";
        } else {
            ss << type << name << "_END[" << std::to_string(count) << "] = {";
        }

        if (count == 64) ss << "\n\t";

        for (auto i = 0; i < count; i++)
        {
            print_parameter(ss, parameters[index][c]);
            index++;

            if (i != count - 1)
            {
                ss << ",";
            }

            if (count == 64 && (i + 1) % 8 == 0 && i != count - 1) {
                ss << "\n\t";
            }
        }
        if (count == 64) ss << "\n";
        ss << "}; \n" << endl;

        if (c == 0) index -= count;
    }

}


static void print_array_2d(std::stringstream& ss, const parameters_t& parameters, int& index, const std::string& name, int count1, int count2)
{
    for (int c = 0; c < 2; c++) {
        string type = "int ";
        if (c == 0) {
            ss << type << name << "_MID[" << std::to_string(count1) << "][" << std::to_string(count2) << "] = {\n";
        } else {
            ss << type << name << "_END[" << std::to_string(count1) << "][" << std::to_string(count2) << "] = {\n";
        }

        for (auto i = 0; i < count1; i++)
        {
            ss << "\t{";
            for (auto j = 0; j < count2; j++) {
                print_parameter(ss, parameters[index][c]);
                index++;

                if (j != count2 - 1)
                {
                    ss << ",";
                }

                if (count2 == 64 && (j + 1) % 8 == 0 && j != count2 - 1) {
                    ss << "\n\t ";
                }
            }

            ss << "}";
            if (i != count1 - 1)
            {
                ss << ",";
            }
            ss << "\n";
        }
        ss << "}; \n" << endl;

        if (c == 0) index -= count1 * count2;
    }
}


static void rebalance_piece_square_tables(parameters_t& parameters, const int material_offset, const int pst_offset) {

    // loop through all pieces excluding the king
    for (int piece = 0; piece <= 4; piece++) {

        for (int stage = 0; stage < 2; stage++) {

            double sum = 0;
            int start = piece == 0 ? 8 : 0;

            for (int i = start; i < 64 - start; i++) {
                const int pst_index = pst_offset + piece * 64 + i;
                sum += parameters[pst_index][stage];
            }

            const double average = sum / 64;

            parameters[material_offset + piece][stage] += average;

            for (int i = start; i < 64 - start; i++) {
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

    get_coefficient_single(coefficients, trace.tempo);
    /*
    get_coefficient_array_2d(coefficients, trace.piece_rank, 6, 8);
    get_coefficient_array_2d(coefficients, trace.piece_file, 6, 8);
    get_coefficient_array_2d(coefficients, trace.centrality, 6, 4);
     */

    return coefficients;
}


parameters_t AltairEval::get_initial_parameters() {
    parameters_t parameters;
    get_initial_parameter_array_double(parameters, PIECE_VALUES_MID, PIECE_VALUES_END, 6);

    get_initial_parameter_array_2d_double(parameters, PIECE_SQUARE_TABLES_MID, PIECE_SQUARE_TABLES_END, 6, 64);

    get_initial_parameter_single_double(parameters, TEMPO_MID, TEMPO_END);
    /*
    get_initial_parameter_array_2d_double(parameters, PIECE_RANK_MID, PIECE_RANK_END, 6, 8);
    get_initial_parameter_array_2d_double(parameters, PIECE_FILE_MID, PIECE_FILE_END, 6, 8);
    get_initial_parameter_array_2d_double(parameters, CENTRALITY_MID, CENTRALITY_END, 6, 4);
     */

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

    /*
    rebalance_piece_terms(parameters_copy, 0, 6, 8);
    rebalance_piece_terms(parameters_copy, 0, 6 + 6 * 8, 8);
    rebalance_piece_terms(parameters_copy, 0, 6 + 6 * 8 + 6 * 8, 4);
     */

    int index = 0;
    stringstream ss;

    print_array(ss, parameters_copy, index, "PIECE_VALUES", 6);

    print_array_2d(ss, parameters_copy, index, "PIECE_SQUARE_TABLES", 6, 64);

    print_single(ss, parameters_copy, index, "TEMPO");

    /*
    print_piece_ranks(ss, parameters_copy, 6);
    print_array_2d(ss, parameters_copy, index, "PIECE_RANK", 6, 8);

    print_piece_files(ss, parameters_copy, 6 + 6 * 8);
    print_array_2d(ss, parameters_copy, index, "PIECE_FILE", 6, 8);

    print_centrality(ss, parameters_copy, 6 + 6 * 8 + 6 * 8);
    print_array_2d(ss, parameters_copy, index, "CENTRALITY", 6, 4);
     */



    std::cout << ss.str() << "\n";
}

// p7g57sm3