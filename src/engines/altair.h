//
// Created by Alex Tian on 2/24/2023.
//

#ifndef TUNER_ALTAIR_H
#define TUNER_ALTAIR_H

#include "../base.h"
#include <string>
#include <vector>
#include <cstdint>

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - "

#define NO_PIECE_INDEX      -1

#define WHITE_COLOR         0
#define BLACK_COLOR         1

#define NO_HASH_ENTRY       0
#define USE_HASH_MOVE       1
#define RETURN_HASH_SCORE   2

#define SCORE_INF           1000000
#define NO_EVALUATION       500000
#define MATE_SCORE          100000
#define MATE_BOUND          99000
#define NO_MOVE             0


#define WHITE_PAWN          0
#define WHITE_KNIGHT        1
#define WHITE_BISHOP        2
#define WHITE_ROOK          3
#define WHITE_QUEEN         4
#define WHITE_KING          5

#define BLACK_PAWN          6
#define BLACK_KNIGHT        7
#define BLACK_BISHOP        8
#define BLACK_ROOK          9
#define BLACK_QUEEN         10
#define BLACK_KING          11

#define EMPTY               12
#define PADDING             13

typedef uint16_t PIECE_TYPE;
typedef int16_t SQUARE_TYPE;
typedef int16_t PLY_TYPE;
typedef int32_t SCORE_TYPE;

constexpr int SQUARE_COLOR[64] = {
        0, 1, 0, 1, 0, 1, 0, 1,
        1, 0, 1, 0, 1, 0, 1, 0,
        0, 1, 0, 1, 0, 1, 0, 1,
        1, 0, 1, 0, 1, 0, 1, 0,
        0, 1, 0, 1, 0, 1, 0, 1,
        1, 0, 1, 0, 1, 0, 1, 0,
        0, 1, 0, 1, 0, 1, 0, 1,
        1, 0, 1, 0, 1, 0, 1, 0
};


constexpr SQUARE_TYPE STANDARD_TO_MAILBOX[64] = {
        21, 22, 23, 24, 25, 26, 27, 28,
        31, 32, 33, 34, 35, 36, 37, 38,
        41, 42, 43, 44, 45, 46, 47, 48,
        51, 52, 53, 54, 55, 56, 57, 58,
        61, 62, 63, 64, 65, 66, 67, 68,
        71, 72, 73, 74, 75, 76, 77, 78,
        81, 82, 83, 84, 85, 86, 87, 88,
        91, 92, 93, 94, 95, 96, 97, 98
};

constexpr SQUARE_TYPE MAILBOX_TO_STANDARD[120] = {
        99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
        99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
        99,  0,  1,  2,  3,  4,  5,  6,  7, 99,
        99,  8,  9, 10, 11, 12, 13, 14, 15, 99,
        99, 16, 17, 18, 19, 20, 21, 22, 23, 99,
        99, 24, 25, 26, 27, 28, 29, 30, 31, 99,
        99, 32, 33, 34, 35, 36, 37, 38, 39, 99,
        99, 40, 41, 42, 43, 44, 45, 46, 47, 99,
        99, 48, 49, 50, 51, 52, 53, 54, 55, 99,
        99, 56, 57, 58, 59, 60, 61, 62, 63, 99,
        99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
        99, 99, 99, 99, 99, 99, 99, 99, 99, 99
};

constexpr short WHITE_INCREMENTS[6][8] = {
        {-11,  -9, -10, -20,   0,   0,   0,   0},
        {-21, -19,  -8,  12,  21,  19,   8, -12},
        {-11,  11,   9,  -9,   0,   0,   0,   0},
        {-10,   1,  10,  -1,   0,   0,   0,   0},
        {-11,  11,   9,  -9, -10,   1,  10,  -1},
        {-11, -10,  -9,   1,  11,  10,   9,  -1}
};

constexpr short BLACK_INCREMENTS[6][8] = {
        { 11,   9,  10,  20,   0,   0,   0,   0},
        {-21, -19,  -8,  12,  21,  19,   8, -12},
        {-11,  11,   9,  -9,   0,   0,   0,   0},
        {-10,   1,  10,  -1,   0,   0,   0,   0},
        {-11,  11,   9,  -9, -10,   1,  10,  -1},
        {-11, -10,  -9,   1,  11,  10,   9,  -1}
};

constexpr SCORE_TYPE TEMPO_BONUS = 8;

constexpr SCORE_TYPE DOUBLED_PAWN_PENALTY_MID = -18;
constexpr SCORE_TYPE DOUBLED_PAWN_PENALTY_END = -26;  // Doubled pawns are very easy to target in the endgame.

constexpr SCORE_TYPE ISOLATED_PAWN_PENALTY_MID = -23;
constexpr SCORE_TYPE ISOLATED_PAWN_PENALTY_END = -34;  // Pawn islands are very bad in the endgame

constexpr SCORE_TYPE ISOLATED_PAWN_SEMI_OPEN_FILE_PENALTY_MID = 35;
constexpr SCORE_TYPE ISOLATED_PAWN_SEMI_OPEN_FILE_PENALTY_END = 27;

constexpr SCORE_TYPE BACKWARDS_PAWN_PENALTY_MID = -6;
constexpr SCORE_TYPE BACKWARDS_PAWN_PENALTY_END = -8;  // Give this a higher base score, but we reduce multipliers in eval_pawn()

constexpr SCORE_TYPE BACKWARDS_PAWN_SEMI_OPEN_FILE_PENALTY_MID = -18;
constexpr SCORE_TYPE BACKWARDS_PAWN_SEMI_OPEN_FILE_PENALTY_END = 0;

constexpr SCORE_TYPE BISHOP_PAIR_BONUS_MID = 38;
constexpr SCORE_TYPE BISHOP_PAIR_BONUS_END = 51;

constexpr SCORE_TYPE ROOK_SEMI_OPEN_FILE_BONUS_MID = 18;
constexpr SCORE_TYPE ROOK_SEMI_OPEN_FILE_BONUS_END = 20;

constexpr SCORE_TYPE ROOK_OPEN_FILE_BONUS_MID = 27;
constexpr SCORE_TYPE ROOK_OPEN_FILE_BONUS_END = 32;

constexpr SCORE_TYPE QUEEN_SEMI_OPEN_FILE_BONUS_MID = 5;
constexpr SCORE_TYPE QUEEN_SEMI_OPEN_FILE_BONUS_END = 8;

constexpr SCORE_TYPE QUEEN_OPEN_FILE_BONUS_MID = 22;
constexpr SCORE_TYPE QUEEN_OPEN_FILE_BONUS_END = 26;


constexpr char PIECE_MATCHER[12] = {'P', 'N', 'B', 'R', 'Q', 'K', 'p', 'n', 'b', 'r', 'q', 'k'};
constexpr int GAME_PHASE_SCORES[6] = {0, 1, 1, 2, 4, 0};

constexpr int MVV_LVA_VALUES[6] = {100, 300, 350, 500, 900, 2000};

constexpr double KING_PAWN_SHIELD_COEFFICIENTS[8] = {0.8, 1.0, 0.6, 0.0, 0.0, 0.3, 1.0, 0.5};
constexpr int KING_PAWN_SHIELD_OUR_PENALTIES[3] = {-6, -14, -23};
constexpr int KING_PAWN_SHIELD_OPP_PENALTIES[3] = {-19, -8, -16};

constexpr SCORE_TYPE PIECE_VALUES_MID[6] = { 103, 268, 277, 342, 684,   0};
constexpr SCORE_TYPE PIECE_VALUES_END[6] = {  71, 174, 185, 295, 539,   0};

constexpr SCORE_TYPE PASSED_PAWN_BONUSES_MID[8] = {0,   5,  18,  27,  45,  60,  78,   0};
constexpr SCORE_TYPE PASSED_PAWN_BONUSES_END[8] = {0,  25,  40,  50,  80, 100, 130,   0};

constexpr SCORE_TYPE BLOCKER_VALUES_MID[6] = {0, -31, -14, -22,  -8,  -3};
constexpr SCORE_TYPE BLOCKER_VALUES_END[6] = {0, -37, -21, -26, -11, -28};

constexpr SCORE_TYPE BLOCKER_TWO_SQUARE_VALUES_MID[6] = {0, -15, -7, -11,  -4,  -1};
constexpr SCORE_TYPE BLOCKER_TWO_SQUARE_VALUES_END[6] = {0, -20, -11, -13, -6, -14};

constexpr SCORE_TYPE PIECE_ATTACK_MOBILITY[6] = {6, 8, 8, 10, 14, 35};
constexpr SCORE_TYPE PIECE_ATTACK_MOBILITY_PENALTY[6] = {0, 2, 1, 3, 4, 0};

constexpr double MOBILITY_COEFFICIENTS_MID[6] = {0.0, 1.6, 1.2, 0.55, 0.36, 0.0};
constexpr double MOBILITY_COEFFICIENTS_END[6] = {0.0, 1.5, 1.1, 1.0, 1.0, 0.0};

constexpr double OUR_KING_DISTANCE_COEFFICIENTS_MID[6] = {0.0, 1.0, 0.0, 0.0, 0.0, 0.0};
constexpr double OUR_KING_DISTANCE_COEFFICIENTS_END[6] = {0.0, 0.3, 0.0, 0.0, 0.0, 0.0};

constexpr double OPP_KING_DISTANCE_COEFFICIENTS_MID[6] = {0.0, 2.6, 0.7, 1.8, 0.7, 0.0};
constexpr double OPP_KING_DISTANCE_COEFFICIENTS_END[6] = {0.0, 1.2, 0.2, 1.1, 1.0, 0.0};

constexpr SCORE_TYPE PAWN_PST_MID[64] = {   0,   0,   0,   0,   0,   0,   0,   0,
                                            28, 112,  26,  71,  51, 135,  -5, -86,
                                            -33,   4,  14,  10,  66, 109,  47, -23,
                                            -42,  17,  13,  32,  39,  37,  39, -32,
                                            -49,  -4,  -2,  18,  24,  25,  19, -36,
                                            -42,  -9,  -2,  -5,  11,  20,  38, -18,
                                            -58, -21, -26, -18, -18,  35,  21, -30,
                                            0,   0,   0,   0,   0,   0,   0,   0
};

constexpr SCORE_TYPE PAWN_PST_END[64] = {   0,   0,   0,   0,   0,   0,   0,   0,
                                            133, 110,  92,  59,  78,  55, 117, 156,
                                            73,  73,  51,  17,   3,  15,  57,  58,
                                            20,  12,  -3, -31, -11,  -2,  14,   8,
                                            8,  12,  -5, -21, -10,  -2,   7,  -3,
                                            -5,   5,  -8,  -4,   4,   0,  -8, -13,
                                            11,  10,  11,  16,  16,   5,  -3,  -8,
                                            0,   0,   0,   0,   0,   0,   0,   0
};

constexpr SCORE_TYPE KNIGHT_PST_MID[64] = { -55,  34,  88,  79, 221,  15, 106,  16,
                                            68,  90, 206, 175, 142, 210, 126, 137,
                                            99, 218, 175, 203, 233, 282, 221, 189,
                                            156, 175, 168, 207, 188, 221, 167, 181,
                                            157, 169, 174, 170, 182, 174, 171, 157,
                                            148, 154, 173, 172, 180, 174, 184, 150,
                                            142, 117, 153, 162, 160, 177, 149, 152,
                                            43, 150, 106, 132, 153, 136, 148, 139
};

constexpr SCORE_TYPE KNIGHT_PST_END[64] = { 107, 113, 138, 112,  99, 118,  78,  48,
                                            127, 142, 111, 136, 130, 103, 118,  88,
                                            122, 112, 144, 141, 118, 109, 108,  91,
                                            124, 142, 157, 156, 154, 139, 140, 116,
                                            126, 128, 148, 163, 151, 147, 139, 119,
                                            116, 136, 128, 146, 145, 123, 116, 109,
                                            103, 125, 130, 134, 139, 114, 116,  96,
                                            140,  94, 124, 131, 122, 125, 101,  72
};

constexpr SCORE_TYPE BISHOP_PST_MID[64] = { 104, 124, -10,  44,  70,  91, 138, 145,
                                            116, 135, 104,  92, 160, 176, 135,  88,
                                            125, 164, 167, 166, 155, 196, 155, 139,
                                            138, 146, 155, 182, 174, 175, 146, 137,
                                            145, 148, 153, 166, 174, 152, 147, 152,
                                            142, 162, 158, 160, 157, 173, 162, 153,
                                            152, 164, 164, 147, 157, 163, 183, 151,
                                            102, 150, 135, 124, 134, 136,  92, 114
};

constexpr SCORE_TYPE BISHOP_PST_END[64] = { 129, 128, 147, 140, 143, 134, 134, 116,
                                            139, 137, 153, 134, 136, 129, 141, 134,
                                            144, 131, 135, 133, 134, 137, 141, 147,
                                            138, 144, 147, 146, 146, 142, 139, 142,
                                            132, 144, 150, 155, 140, 142, 134, 132,
                                            132, 141, 147, 146, 150, 134, 137, 125,
                                            135, 118, 131, 141, 139, 133, 113, 109,
                                            128, 134, 136, 141, 135, 137, 144, 129
};

constexpr SCORE_TYPE ROOK_PST_MID[64] = { 253, 273, 229, 290, 277, 207, 218, 233,
                                          244, 251, 286, 302, 311, 314, 233, 251,
                                          199, 220, 233, 231, 211, 279, 275, 237,
                                          189, 195, 213, 232, 227, 257, 204, 197,
                                          200, 193, 202, 211, 229, 218, 235, 214,
                                          188, 201, 210, 205, 223, 234, 228, 202,
                                          194, 214, 213, 221, 231, 243, 227, 163,
                                          223, 224, 233, 245, 247, 238, 201, 218
};

constexpr SCORE_TYPE ROOK_PST_END[64] = { 276, 266, 279, 255, 267, 288, 288, 279,
                                          279, 274, 261, 251, 239, 255, 284, 274,
                                          279, 275, 263, 262, 262, 251, 254, 262,
                                          276, 270, 274, 256, 259, 259, 266, 276,
                                          270, 273, 269, 258, 249, 258, 253, 256,
                                          268, 268, 255, 257, 251, 250, 254, 250,
                                          267, 264, 260, 261, 249, 251, 254, 272,
                                          277, 274, 269, 257, 253, 265, 275, 258
};

constexpr SCORE_TYPE QUEEN_PST_MID[64] = { 491, 511, 517, 514, 638, 614, 599, 572,
                                           511, 473, 513, 520, 455, 588, 535, 591,
                                           527, 510, 535, 505, 548, 610, 580, 601,
                                           496, 502, 508, 501, 515, 535, 524, 531,
                                           534, 503, 524, 518, 525, 534, 532, 537,
                                           524, 545, 526, 540, 533, 540, 554, 539,
                                           512, 533, 553, 548, 554, 567, 543, 547,
                                           541, 533, 541, 554, 531, 519, 513, 492
};

constexpr SCORE_TYPE QUEEN_PST_END[64] = { 516, 549, 553, 550, 491, 503, 493, 548,
                                           494, 539, 535, 546, 610, 520, 567, 510,
                                           479, 507, 497, 562, 546, 527, 537, 515,
                                           524, 526, 518, 542, 562, 557, 585, 564,
                                           473, 534, 507, 541, 523, 537, 556, 536,
                                           499, 464, 510, 482, 501, 530, 518, 529,
                                           488, 468, 467, 472, 470, 463, 458, 471,
                                           459, 469, 468, 491, 495, 482, 490, 458
};

constexpr SCORE_TYPE KING_PST_MID[64] = { -91, 252, 210,  98,-202, -97,  94,  69,
                                          282, 121,  86, 124,  19,  71, -25,-156,
                                          117, 111, 149, -14,  26, 142, 154,  -2,
                                          41,  18,  50, -66, -66, -35,   5, -61,
                                          -39,  76, -32,-145,-145, -66, -49, -74,
                                          36,  26, -11, -96, -91, -52,  -3, -31,
                                          53,  44,   3, -97, -80, -24,  21,  25,
                                          7,  64,  33,-102, -21, -34,  39,  30
};

constexpr SCORE_TYPE KING_PST_END[64] = {-70, -75, -46, -44, 18, 33, -18, -28,
                                         -64, 7, 9, -3, 16, 32, 27, 40,
                                         -5, 14, 9, 23, 17, 33, 24, 11,
                                         -14, 28, 30, 46, 41, 48, 32, 12,
                                         -11, -6, 42, 58, 58, 46, 25, 2,
                                         -20, 7, 30, 45, 47, 39, 18, 1,
                                         -35, -5, 22, 33, 33, 21, 1, -21,
                                         -65, -43, -20, 8, -20, -3, -35, -63
};

struct Trace {
    int score={};

    short material[6][2]{};

    short pawn_pst[64][2]{};
    short knight_pst[64][2]{};
    short bishop_pst[64][2]{};
    short rook_pst[64][2]{};
    short queen_pst[64][2]{};
    short king_pst[64][2]{};

    //short king_pawn_shield_coefficients[8]{};
    //short our_king_pawn_shield[3][2]{};
    //short opp_king_pawn_shield[3][2]{};

    short passed_pawns[8][2]{};
    short isolated_pawns[2]{};
    short isolated_pawns_semi_open_file[2]{};
    short doubled_pawns[2]{};
    short backward_pawns[2]{};
    short backward_pawns_semi_open_file[2]{};

    short blockers[6][2]{};
    short blockers_two_squares[6][2]{};

    short rook_semi_open[2]{};
    short rook_open[2]{};

    short queen_semi_open[2]{};
    short queen_open[2]{};

    short bishop_bonus[2]{};

    //short mobility[6][2]{};
    //short own_king_tropism[6][2]{};
    //short opp_king_tropism[6][2]{};
};

struct Score_Struct {
    SCORE_TYPE mid;
    SCORE_TYPE end;
};

inline double get_distance(SQUARE_TYPE square_1, SQUARE_TYPE square_2) {
    SQUARE_TYPE row_1 = 8 - square_1 / 8, col_1 = square_1 % 8 + 1;
    SQUARE_TYPE row_2 = 8 - square_2 / 8, col_2 = square_2 % 8 + 1;

    return abs(row_1 - row_2) + abs(col_1 - col_2);
}

namespace Altair {
    class AltairEval
    {
    public:
        constexpr static bool includes_additional_score = true;
        static parameters_t get_initial_parameters();
        static EvalResult get_fen_eval_result(const std::string& fen);
        static void print_parameters(const parameters_t& parameters);
    };
}


#endif //TUNER_ALTAIR_H
