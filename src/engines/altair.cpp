//
// Created by Alex Tian on 2/24/2023.
//

#include "altair.h"
#include "bitboard.h"
#include "evaluation_constants.h"
#include "tables.h"
#include "position.h"
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
    int scores[] = {0, 0};

    for (int piece = 0; piece < 12; piece++) {
        auto color = static_cast<Color>(piece >= 6);
        PieceType pieceType = get_piece_type(static_cast<Piece>(piece), color);
        BITBOARD pieceBB = position.get_pieces(static_cast<Piece>(piece));

        scores[color] += popcount(pieceBB) * PIECE_VALUES[pieceType];
        trace.piece_values[pieceType][color] += popcount(pieceBB);

        /*
        while (pieceBB) {
            Square square = poplsb(pieceBB);

            BITBOARD attacks = get_piece_attacks(static_cast<Piece>(piece), square, position.all_pieces);
            scores[color] += popcount(attacks) * MOBILITY[pieceType];
            trace.mobility[pieceType][color] += popcount(attacks);
        }
        */
    }

    short fifty = 0;

    position.set_state(position.state_stack[0], fifty);
    position.get_pseudo_legal_captures(position.scored_moves[0]);

    int count = 0;

    for (ScoredMove& scored_move : position.scored_moves[0]) {
        Move move = scored_move.move;

        bool attempt = position.make_move(move, position.state_stack[0], fifty);

        position.undo_move(move, position.state_stack[0], fifty);

        if (attempt) count++;
    }

    scores[position.side] += count * CAPTURE_BONUS;
    trace.capture_bonus[position.side] += count;

    return (scores[0] - scores[1]) * (position.side == WHITE ? 1 : -1);
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
    for (int i = 0; i < 5 - param_size; i++) {
        ss << " ";
    }
    ss << param;

}

static void print_array(std::stringstream& ss, const parameters_t& parameters, int& index, const std::string& name, int count)
{

    string type = "int ";
    ss << type << name << "[" << std::to_string(count) << "] = {";

    for (auto i = 0; i < count; i++) {
        print_parameter(ss, parameters[index]);
        index++;

        if (i != count - 1) {
            ss << ",";
        }
    }

    ss << "}; \n" << endl;
}


static coefficients_t get_coefficients(const Trace& trace)
{
    coefficients_t coefficients;

    get_coefficient_array(coefficients, trace.piece_values, 6);
    get_coefficient_single(coefficients, trace.capture_bonus);

    /*
    get_coefficient_array_2d(coefficients, trace.piece_rank, 6, 8);
    get_coefficient_array_2d(coefficients, trace.piece_file, 6, 8);
    get_coefficient_array_2d(coefficients, trace.centrality, 6, 4);
     */

    return coefficients;
}


parameters_t AltairEval::get_initial_parameters() {
    parameters_t parameters;

    get_initial_parameter_array(parameters, PIECE_VALUES, 6);
    get_initial_parameter_single(parameters, CAPTURE_BONUS);
    
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

    /*
    rebalance_piece_terms(parameters_copy, 0, 6, 8);
    rebalance_piece_terms(parameters_copy, 0, 6 + 6 * 8, 8);
    rebalance_piece_terms(parameters_copy, 0, 6 + 6 * 8 + 6 * 8, 4);
     */

    int index = 0;
    stringstream ss;

    print_array(ss, parameters, index, "PIECE_VALUES", 6);
    print_array(ss, parameters, index, "CAPTURE_BONUS", 1);



    std::cout << ss.str() << "\n";
}

// p7g57sm3