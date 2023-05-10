//
// Created by Alex Tian on 2/24/2023.
//

#include "altair.h"
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


int piece_to_num(char piece) {

    auto it = std::find(std::begin(PIECE_MATCHER), std::end(PIECE_MATCHER), piece);
    return it - std::begin(PIECE_MATCHER);
}

bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

class Position {

public:

    Position() = default;

    PLY_TYPE set_fen(const std::string& fen_string);

    PIECE_TYPE board[120]{};
    SQUARE_TYPE piece_list_index[120] = {NO_PIECE_INDEX};

    std::vector<PIECE_TYPE> white_pieces;
    std::vector<PIECE_TYPE> black_pieces;

    SQUARE_TYPE king_positions[2]{};

    SQUARE_TYPE pawn_rank[2][10]{};

    SQUARE_TYPE castle_ability_bits = 0;
    SQUARE_TYPE ep_square = 0;
    SQUARE_TYPE side = 0;

    inline void update_piece_list_index(SQUARE_TYPE left_range, SQUARE_TYPE right_range, SQUARE_TYPE side_to_use) {
        if (!side_to_use) {
            for (int i = left_range; i < right_range; i++) {
                piece_list_index[white_pieces[i]] = i;
            }
        } else {
            for (int i = left_range; i < right_range; i++) {
                piece_list_index[black_pieces[i]] = i;
            }
        }
    }


};


PLY_TYPE Position::set_fen(const std::string& fen_string) {

    std::vector<std::string> fen_tokens = split(fen_string, ' ');

    // Setting the padding around the 8x8 board inscribed within the 10x12 board
    SQUARE_TYPE pos = 0;

    white_pieces.clear();
    black_pieces.clear();

    for (SQUARE_TYPE& i : piece_list_index) {
        i = NO_PIECE_INDEX;
    }

    for (int i = 0; i < 21; i++) {
        board[pos++] = PADDING;
    }

    // Parsing the main 8x8 board part while adding appropriate padding
    for (char c : fen_tokens[0]) {

        if (c == '/') {

            board[pos++] = PADDING;
            board[pos++] = PADDING;

        }
        else if (std::isdigit(c)) {

            for (int empty_amt = 0; empty_amt < c - '0'; empty_amt++) {
                board[pos++] = EMPTY;
            }

        }
        else if (std::isalpha(c)) {

            board[pos] = piece_to_num(c);

            if (std::isupper(c)) {
                white_pieces.push_back(pos);
                piece_list_index[pos] = white_pieces.size() - 1;
            }
            else {
                black_pieces.push_back(pos);
                piece_list_index[pos] = black_pieces.size() - 1;
            }

            if (c == 'K') king_positions[0] = pos;
            else if (c == 'k') king_positions[1] = pos;

            pos++;

        }
    }

    for (int i = 0; i < 21; i++) {
        board[pos++] = PADDING;
    }

    castle_ability_bits = 0;
    for (char c : fen_tokens[2]) {

        if (c == 'K') castle_ability_bits |= 1;
        else if (c == 'Q') castle_ability_bits |= 2;
        else if (c == 'k') castle_ability_bits |= 4;
        else if (c == 'q') castle_ability_bits |= 8;

    }

    if (fen_tokens[3].size() > 1) {
        SQUARE_TYPE square = (8 - (fen_tokens[3][1] - '0')) * 8 + fen_tokens[3][0] - 'a';
        ep_square = STANDARD_TO_MAILBOX[square];
    }
    else {
        ep_square = 0;
    }

    side = 0;
    if (fen_tokens[1] == "b") side = 1;

    if (fen_tokens.size() >= 5) {
        if (!is_number(fen_tokens[4])) return 0;
        return static_cast<PLY_TYPE>(std::stoi(fen_tokens[4]));
    }
    else return 0;
}


// ------------------------------------------- EVALUATION -----------------------------------------------------


SCORE_TYPE evaluate(Position& position, Trace& trace) {

    Score_Struct white_material{};
    Score_Struct white_scores{};

    Score_Struct black_material{};
    Score_Struct black_scores{};

    int game_phase = 0;

    for (SQUARE_TYPE pos : position.white_pieces) {
        PIECE_TYPE piece = position.board[pos];
        SQUARE_TYPE standard_pos = MAILBOX_TO_STANDARD[pos];

        game_phase += GAME_PHASE_SCORES[piece];

        white_material.mid += PIECE_VALUES_MID[piece];
        white_material.end += PIECE_VALUES_END[piece];
        trace.material[piece][0]++;
        // std::cout << piece << std::endl;

        // std::cout << white_scores.mid << " " << white_scores.end << std::endl;
        if (piece == WHITE_PAWN) {
            white_scores.mid += PAWN_PST_MID[standard_pos];
            white_scores.end += PAWN_PST_END[standard_pos];
            trace.pawn_pst[standard_pos][WHITE_COLOR]++;
        }
        else if (piece == WHITE_KNIGHT) {
            white_scores.mid += KNIGHT_PST_MID[standard_pos];
            white_scores.end += KNIGHT_PST_END[standard_pos];
            trace.knight_pst[standard_pos][WHITE_COLOR]++;
        }
        else if (piece == WHITE_BISHOP) {
            white_scores.mid += BISHOP_PST_MID[standard_pos];
            white_scores.end += BISHOP_PST_END[standard_pos];
            trace.bishop_pst[standard_pos][WHITE_COLOR]++;
        }
        else if (piece == WHITE_ROOK) {
            white_scores.mid += ROOK_PST_MID[standard_pos];
            white_scores.end += ROOK_PST_END[standard_pos];
            trace.rook_pst[standard_pos][WHITE_COLOR]++;
        }
        else if (piece == WHITE_QUEEN) {
            white_scores.mid += QUEEN_PST_MID[standard_pos];
            white_scores.end += QUEEN_PST_END[standard_pos];
            trace.queen_pst[standard_pos][WHITE_COLOR]++;
        }
        else if (piece == WHITE_KING) {
            white_scores.mid += KING_PST_MID[standard_pos];
            white_scores.end += KING_PST_END[standard_pos];
            trace.king_pst[standard_pos][WHITE_COLOR]++;
        }

        // std::cout << white_scores.mid << " " << white_scores.end << std::endl;
    }

    for (SQUARE_TYPE pos : position.black_pieces) {
        PIECE_TYPE piece = position.board[pos];
        SQUARE_TYPE standard_pos = MAILBOX_TO_STANDARD[pos];

        game_phase += GAME_PHASE_SCORES[piece-6];

        black_material.mid += PIECE_VALUES_MID[piece - BLACK_PAWN];
        black_material.end += PIECE_VALUES_END[piece - BLACK_PAWN];
        trace.material[piece - BLACK_PAWN][1]++;
        // std::cout << piece << std::endl;

        // std::cout << black_scores.mid << " " << black_scores.end << std::endl;
        if (piece == BLACK_PAWN) {
            black_scores.mid += PAWN_PST_MID[standard_pos ^ 56];
            black_scores.end += PAWN_PST_END[standard_pos ^ 56];
            trace.pawn_pst[standard_pos ^ 56][BLACK_COLOR]++;
        }
        else if (piece == BLACK_KNIGHT) {
            black_scores.mid += KNIGHT_PST_MID[standard_pos ^ 56];
            black_scores.end += KNIGHT_PST_END[standard_pos ^ 56];
            trace.knight_pst[standard_pos ^ 56][BLACK_COLOR]++;
        }
        else if (piece == BLACK_BISHOP) {
            black_scores.mid += BISHOP_PST_MID[standard_pos ^ 56];
            black_scores.end += BISHOP_PST_END[standard_pos ^ 56];
            trace.bishop_pst[standard_pos ^ 56][BLACK_COLOR]++;
        }
        else if (piece == BLACK_ROOK) {
            black_scores.mid += ROOK_PST_MID[standard_pos ^ 56];
            black_scores.end += ROOK_PST_END[standard_pos ^ 56];
            trace.rook_pst[standard_pos ^ 56][BLACK_COLOR]++;
        }
        else if (piece == BLACK_QUEEN) {
            black_scores.mid += QUEEN_PST_MID[standard_pos ^ 56];
            black_scores.end += QUEEN_PST_END[standard_pos ^ 56];
            trace.queen_pst[standard_pos ^ 56][BLACK_COLOR]++;
        }
        else if (piece == BLACK_KING) {
            black_scores.mid += KING_PST_MID[standard_pos ^ 56];
            black_scores.end += KING_PST_END[standard_pos ^ 56];
            trace.king_pst[standard_pos ^ 56][BLACK_COLOR]++;
        }

        // std::cout << black_scores.mid << " " << black_scores.end << std::endl;
    }


    white_scores.mid += white_material.mid;
    white_scores.end += white_material.end;

    black_scores.mid += black_material.mid;
    black_scores.end += black_material.end;


    if (game_phase > 24) game_phase = 24; // In case of early promotions
    SCORE_TYPE white_score = (white_scores.mid * game_phase +
                              (24 - game_phase) * white_scores.end) / 24;

    SCORE_TYPE black_score = (black_scores.mid * game_phase +
                              (24 - game_phase) * black_scores.end) / 24;

    // std::cout << white_score << " " << black_score << std::endl;

    return (white_score - black_score);
}


static void print_parameter(std::stringstream& ss, const tune_t parameter, bool decimal)
{
    if (decimal) {
        const auto test_param = static_cast<SCORE_TYPE>(parameter + 0.5 - (parameter < 0.0));
        auto param_size = std::to_string(test_param).size();
        for (int i = 0; i < 7 - param_size; i++) {
            ss << " ";
        }
        ss << std::setprecision(3) << parameter;
    } else {
        const auto param = static_cast<SCORE_TYPE>(parameter + 0.5 - (parameter < 0.0));
        auto param_size = std::to_string(param).size();
        for (int i = 0; i < 4 - param_size; i++) {
            ss << " ";
        }
        ss << param;
    }
}

static void print_single(std::stringstream& ss, const parameters_t& parameters, int& index, const std::string& name, bool decimal)
{
    string type = decimal ? " double " : " SCORE_TYPE ";
    ss << "constexpr" << type << name << "_MID = ";
    print_parameter(ss, parameters[index][0], decimal);
    ss << ";" << endl;

    ss << "constexpr" << type << name << "_END = ";
    print_parameter(ss, parameters[index][1], decimal);
    ss << ";\n" << endl;

    index++;
}

static void print_array(std::stringstream& ss, const parameters_t& parameters, int& index, const std::string& name, int count, bool decimal)
{
    for (int c = 0; c < 2; c++) {

        string type = decimal ? " double " : " SCORE_TYPE ";
        if (c == 0) {
            ss << "constexpr" << type << name << "_MID[" << std::to_string(count) << "] = {";
        } else {
            ss << "constexpr" << type << name << "_END[" << std::to_string(count) << "] = {";
        }

        if (count == 64) ss << "\n\t";

        for (auto i = 0; i < count; i++)
        {
            print_parameter(ss, parameters[index][c], decimal);
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


static void print_array_2d(std::stringstream& ss, const parameters_t& parameters, int& index, const std::string& name, int count1, int count2, bool decimal)
{
    for (int c = 0; c < 2; c++) {
        string type = decimal ? " double " : " SCORE_TYPE ";
        if (c == 0) {
            ss << "constexpr" << type << name << "_MID[" << std::to_string(count1) << "][" << std::to_string(count2) << "] = {\n";
        } else {
            ss << "constexpr" << type << name << "_END[" << std::to_string(count1) << "][" << std::to_string(count2) << "] = {\n";
        }

        for (auto i = 0; i < count1; i++)
        {
            ss << "\t{";
            for (auto j = 0; j < count2; j++) {
                print_parameter(ss, parameters[index][c], decimal);
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


static coefficients_t get_coefficients(const Trace& trace)
{
    coefficients_t coefficients;
    get_coefficient_array(coefficients, trace.material, 6);

    get_coefficient_array(coefficients, trace.pawn_pst, 64);
    get_coefficient_array(coefficients, trace.knight_pst, 64);
    get_coefficient_array(coefficients, trace.bishop_pst, 64);
    get_coefficient_array(coefficients, trace.rook_pst, 64);
    get_coefficient_array(coefficients, trace.queen_pst, 64);
    get_coefficient_array(coefficients, trace.king_pst, 64);

    return coefficients;
}


parameters_t AltairEval::get_initial_parameters() {
    parameters_t parameters;
    get_initial_parameter_array_double(parameters, PIECE_VALUES_MID, PIECE_VALUES_END, 6);

    get_initial_parameter_array_double(parameters, PAWN_PST_MID, PAWN_PST_END, 64);
    get_initial_parameter_array_double(parameters, KNIGHT_PST_MID, KNIGHT_PST_END, 64);
    get_initial_parameter_array_double(parameters, BISHOP_PST_MID, BISHOP_PST_END, 64);
    get_initial_parameter_array_double(parameters, ROOK_PST_MID, ROOK_PST_END, 64);
    get_initial_parameter_array_double(parameters, QUEEN_PST_MID, QUEEN_PST_END, 64);
    get_initial_parameter_array_double(parameters, KING_PST_MID, KING_PST_END, 64);

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

    int index = 0;
    stringstream ss;

    print_array(ss, parameters_copy, index, "PIECE_VALUES", 6, false);

    print_array(ss, parameters_copy, index, "PAWN_PST", 64, false);
    print_array(ss, parameters_copy, index, "KNIGHT_PST", 64, false);
    print_array(ss, parameters_copy, index, "BISHOP_PST", 64, false);
    print_array(ss, parameters_copy, index, "ROOK_PST", 64, false);
    print_array(ss, parameters_copy, index, "QUEEN_PST", 64, false);
    print_array(ss, parameters_copy, index, "KING_PST", 64, false);

    std::cout << ss.str() << "\n";
}
