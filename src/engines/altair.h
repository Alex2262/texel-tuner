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


constexpr char PIECE_MATCHER[12] = {'P', 'N', 'B', 'R', 'Q', 'K', 'p', 'n', 'b', 'r', 'q', 'k'};
constexpr int GAME_PHASE_SCORES[6] = {0, 1, 1, 2, 4, 0};

constexpr SCORE_TYPE MAX_MINOR_PIECE_VALUE_MID = 409;
constexpr SCORE_TYPE PIECE_VALUES_MID[6] = { 100, 382, 409, 592,1448,   0};

constexpr SCORE_TYPE PIECE_VALUES_END[6] = { 128, 354, 348, 685,1268,   0};

constexpr SCORE_TYPE PAWN_PST_MID[4][64]{};

constexpr SCORE_TYPE PAWN_PST_END[4][64]{};

constexpr SCORE_TYPE KNIGHT_PST_MID[4][64]{};

constexpr SCORE_TYPE KNIGHT_PST_END[4][64]{};

constexpr SCORE_TYPE BISHOP_PST_MID[4][64]{};

constexpr SCORE_TYPE BISHOP_PST_END[4][64]{};

constexpr SCORE_TYPE ROOK_PST_MID[4][64]{};

constexpr SCORE_TYPE ROOK_PST_END[4][64]{};

constexpr SCORE_TYPE QUEEN_PST_MID[4][64]{};

constexpr SCORE_TYPE QUEEN_PST_END[4][64]{};

constexpr SCORE_TYPE KING_PST_MID[4][64]{};

constexpr SCORE_TYPE KING_PST_END[4][64]{};

constexpr SCORE_TYPE PASSED_PAWN_BONUSES_MID[3][8] = {
        {   0,  -2,  -1,  -8,   9,  15,  26,   0},
        {   0,   3,   2,  -7,  20,  53, 347,   0},
        {   0,   3,  52, -14,  39,  53, 539,   0}
};

constexpr SCORE_TYPE PASSED_PAWN_BONUSES_END[3][8] = {
        {   0,  13,  24,  56,  85, 159, 204,   0},
        {   0,   3,  18,  46,  96, 184, 134,   0},
        {   0,   3,  18,  28,  66, 144, -31,   0}
};

constexpr SCORE_TYPE ISOLATED_PAWN_PENALTY_MID =  -10;
constexpr SCORE_TYPE ISOLATED_PAWN_PENALTY_END =    1;

constexpr SCORE_TYPE ISOLATED_PAWN_SEMI_OPEN_FILE_PENALTY_MID =  -20;
constexpr SCORE_TYPE ISOLATED_PAWN_SEMI_OPEN_FILE_PENALTY_END =  -16;

constexpr SCORE_TYPE DOUBLED_PAWN_PENALTY_MID =   -7;
constexpr SCORE_TYPE DOUBLED_PAWN_PENALTY_END =  -37;

constexpr SCORE_TYPE BACKWARDS_PAWN_PENALTY_MID =   -6;
constexpr SCORE_TYPE BACKWARDS_PAWN_PENALTY_END =    2;

constexpr SCORE_TYPE BACKWARDS_PAWN_SEMI_OPEN_FILE_PENALTY_MID =  -15;
constexpr SCORE_TYPE BACKWARDS_PAWN_SEMI_OPEN_FILE_PENALTY_END =   -9;

constexpr SCORE_TYPE PHALANX_PAWN_BONUS_MID[8] = {   0,   2,  21,  24,  41,  27, -67,   0};

constexpr SCORE_TYPE PHALANX_PAWN_BONUS_END[8] = {   0,  -2,   4,  15,  72, 162, 453,   0};

constexpr SCORE_TYPE PIECE_SUPPORT_MID[6][6] = {
        {  20,   5,   8,   4,  -7, -80},
        {  11,  18,  16,  19,  23,  15},
        {   4,  16, -33,  10,  16,   7},
        {   5,  16,  10,  12,  12,  12},
        {  -1,   7,   5,   7,  39,   4},
        {   0,   0,   0,   0,   0,   0}
};

constexpr SCORE_TYPE PIECE_SUPPORT_END[6][6] = {
        {  14,  26,  32,   7,   0,  31},
        {   8,  15,   6,   3,  -7,   1},
        {   2,  18,  57,  13,  16,  11},
        {   3,   2,   7,  13,  41,  -6},
        {   8,   6, -10,  -9,-538, -13},
        {   0,   0,   0,   0,   0,   0}
};

constexpr SCORE_TYPE PIECE_THREAT_MID[6][6] = {
        {  10,  79,  67, 109,  70, 198},
        {  11,  20,  50,  75,  65, 134},
        {   8,  41,  20,  56,  65,  79},
        {  -7,   7,  12,  10,  66, 138},
        {  -6,   4,   1,  -5,  10,  80},
        {   0,   0,   0,   0,   0,   0}
};

constexpr SCORE_TYPE PIECE_THREAT_END[6][6] = {
        {  10,  30,  70,   1,  36,  18},
        {  24,  20,  53,  27,  15,  33},
        {  25,  60,  20,  42,  69,  84},
        {  26,  33,  39,  10,  50,  33},
        {  15,  14,  39,  36,  10, 109},
        {   0,   0,   0,   0,   0,   0}
};

constexpr SCORE_TYPE BLOCKER_VALUES_MID[6][8] = {
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, -5, -5, -5, -5, -5, -5, 0},
        {0, 5, 5, 5, 5, 5, 5, 0},
        {0, 14, 14, 14, 14, 14, 14, 0},
        {0, 14, 14, 14, 14, 14, 14, 0},
        {0, 5, 5, 5, 0, -67, -67, 0},
};

constexpr SCORE_TYPE BLOCKER_VALUES_END[6][8] = {
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, -5, -5, -5, -5, -5, -5, 0},
        {0, 5, 5, 5, 5, 5, 5, 0},
        {0, 14, 14, 14, 14, 14, 14, 0},
        {0, 14, 14, 14, 14, 14, 14, 0},
        {0, 5, 5, 5, 0, -67, -67, 0},
};

constexpr SCORE_TYPE BLOCKER_TWO_SQUARE_VALUES_MID[6][8] = {
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, -5, -5, -5, -5, -5, 0, 0},
        {0, 5, 5, 5, 5, 5, 0, 0},
        {0, 14, 14, 14, 14, 14, 0, 0},
        {0, 14, 14, 14, 14, 14, 0, 0},
        {0, 5, 5, 5, 0, -67, 0, 0},
};

constexpr SCORE_TYPE BLOCKER_TWO_SQUARE_VALUES_END[6][8] = {
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, -5, -5, -5, -5, -5, 0, 0},
        {0, 5, 5, 5, 5, 5, 0, 0},
        {0, 14, 14, 14, 14, 14, 0, 0},
        {0, 14, 14, 14, 14, 14, 0, 0},
        {0, 5, 5, 5, 0, -67, 0, 0},
};

constexpr SCORE_TYPE ROOK_SEMI_OPEN_FILE_BONUS_MID =   19;
constexpr SCORE_TYPE ROOK_SEMI_OPEN_FILE_BONUS_END =    7;

constexpr SCORE_TYPE ROOK_OPEN_FILE_BONUS_MID =   42;
constexpr SCORE_TYPE ROOK_OPEN_FILE_BONUS_END =    2;

constexpr SCORE_TYPE QUEEN_SEMI_OPEN_FILE_BONUS_MID =    3;
constexpr SCORE_TYPE QUEEN_SEMI_OPEN_FILE_BONUS_END =   29;

constexpr SCORE_TYPE QUEEN_OPEN_FILE_BONUS_MID =   -6;
constexpr SCORE_TYPE QUEEN_OPEN_FILE_BONUS_END =   33;

constexpr SCORE_TYPE KING_SEMI_OPEN_FILE_PENALTY_MID =  -21;
constexpr SCORE_TYPE KING_SEMI_OPEN_FILE_PENALTY_END =    1;

constexpr SCORE_TYPE KING_OPEN_FILE_PENALTY_MID =  -13;
constexpr SCORE_TYPE KING_OPEN_FILE_PENALTY_END =  -10;

constexpr SCORE_TYPE BISHOP_PAIR_BONUS_MID =   27;
constexpr SCORE_TYPE BISHOP_PAIR_BONUS_END =   72;

constexpr SCORE_TYPE TEMPO_BONUS_MID =   26;
constexpr SCORE_TYPE TEMPO_BONUS_END =   25;

constexpr double MOBILITY_COEFFICIENTS_MID[6] = {      0,      5.83,      3.2,      1.78,      0.672,      0};

constexpr double MOBILITY_COEFFICIENTS_END[6] = {      0,      1.54,      2.85,      1.1,      0.229,      0};

constexpr double OWN_KING_DISTANCE_COEFFICIENTS_MID[6] = {      0,     -5.8,      0,      0,      0,      0};

constexpr double OWN_KING_DISTANCE_COEFFICIENTS_END[6] = {      0,      0.827,      0,      0,      0,      0};

constexpr double OPP_KING_DISTANCE_COEFFICIENTS_MID[6] = {      0,     -1.02,      -0.165,     -4.35,    -9.62,      0};

constexpr double OPP_KING_DISTANCE_COEFFICIENTS_END[6] = {      0,     -1.07,      1.79,      4.06,      6.29,      0};

constexpr SCORE_TYPE KING_PAWN_SHIELD_OWN_PENALTIES_MID[3][8] = {
        {  -1,  -1, -33,   0,   0, -14, -13,   4},
        {   0, -42, -34,   0,   0, -20, -21,   1},
        { -41, -63, -58,   0,   0, -33, -55, -25}
};

constexpr SCORE_TYPE KING_PAWN_SHIELD_OWN_PENALTIES_END[3][8] = {
        {  13,   6,  19,   0,   0,  -4,  11,  20},
        {  32,  31,  21,   0,   0, -17,  13,  24},
        {  33,  19,  14,   0,   0,  -7,  27,  38}
};

constexpr SCORE_TYPE KING_PAWN_SHIELD_OPP_PENALTIES_MID[3][8] = {
        {   0, -27,  -4,   0,   0,  -4, -18,   9},
        {  -7, -12, -36,   0,   0, -31,   0, -13},
        { -15, -90,-162,   0,   0, -91, -66, -49}
};

constexpr SCORE_TYPE KING_PAWN_SHIELD_OPP_PENALTIES_END[3][8] = {
        { -15,  -1,  -9,   0,   0,  -6,  -6, -17},
        {  36,  33,  21,   0,   0,  26,  23,  23},
        {  76, 104,  81,   0,   0,  56,  90,  94}
};

constexpr SCORE_TYPE KING_RING_ATTACKS_MID[2][6] = {
        {   0,  13,  17,  26,  19,   0},
        {   0,   8,   3,   4,   6,   0}
};

constexpr SCORE_TYPE KING_RING_ATTACKS_END[2][6] = {
        {   0, -10,  -4,  -7,  12,   0},
        {   0,   1,   0,   1,  14,   0}
};

constexpr SCORE_TYPE OUTPOST_PENALTY_MID = -5;
constexpr SCORE_TYPE OUTPOST_PENALTY_END = -4;

constexpr SCORE_TYPE OUTPOST_KNIGHT_PENALTY_MID = -8;
constexpr SCORE_TYPE OUTPOST_KNIGHT_PENALTY_END = -4;

constexpr SCORE_TYPE OUTPOST_BISHOP_PENALTY_MID = -8;
constexpr SCORE_TYPE OUTPOST_BISHOP_PENALTY_END = -3;

constexpr SCORE_TYPE SQUARE_OF_THE_PAWN_MID = 0;
constexpr SCORE_TYPE SQUARE_OF_THE_PAWN_END = 10;

struct Trace {
    int score={};

    short material[6][2]{};

    short pawn_pst[4][64][2]{};
    short knight_pst[4][64][2]{};
    short bishop_pst[4][64][2]{};
    short rook_pst[4][64][2]{};
    short queen_pst[4][64][2]{};
    short king_pst[4][64][2]{};

    short passed_pawns[3][8][2]{};
    short isolated_pawns[2]{};
    short isolated_pawns_semi_open_file[2]{};
    short doubled_pawns[2]{};
    short backward_pawns[2]{};
    short backward_pawns_semi_open_file[2]{};
    short pawn_phalanx[8][2]{};

    short piece_support[6][6][2]{};
    short piece_threat[6][6][2]{};

    short blockers[6][8][2]{};
    short blockers_two_squares[6][8][2]{};

    short rook_semi_open[2]{};
    short rook_open[2]{};

    short queen_semi_open[2]{};
    short queen_open[2]{};

    short king_semi_open[2]{};
    short king_open[2]{};

    short bishop_bonus[2]{};

    short tempo_bonus[2]{};

    short mobility[6][2]{};

    short own_king_tropism[6][2]{};
    short opp_king_tropism[6][2]{};

    short own_king_pawn_shield[3][8][2]{};
    short opp_king_pawn_shield[3][8][2]{};

    short king_ring_attacks[2][6][2]{};

    short outpost_penalty[2]{};
    short outpost_knight_penalty[2]{};
    short outpost_bishop_penalty[2]{};

    short square_of_the_pawn[2]{};
};

struct Score_Struct {
    SCORE_TYPE mid;
    SCORE_TYPE end;
};

inline SCORE_TYPE get_distance(SQUARE_TYPE square_1, SQUARE_TYPE square_2) {
    SQUARE_TYPE row_1 = 8 - square_1 / 8, col_1 = square_1 % 8 + 1;
    SQUARE_TYPE row_2 = 8 - square_2 / 8, col_2 = square_2 % 8 + 1;

    return abs(row_1 - row_2) + abs(col_1 - col_2);
}

inline SCORE_TYPE get_chebyshev_distance(SQUARE_TYPE square_1, SQUARE_TYPE square_2) {
    SQUARE_TYPE row_1 = 8 - square_1 / 8, col_1 = square_1 % 8;
    SQUARE_TYPE row_2 = 8 - square_2 / 8, col_2 = square_2 % 8;

    return std::max(abs(row_1 - row_2), abs(col_1 - col_2));
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




template<int n>
struct KingRing {
    constexpr KingRing() : masks() {
        for (int rings = 1; rings <= n; rings++) {
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    masks[rings-1][i * 8 + j] = 0ULL;
                    for (int y = i - rings; y <= i + rings; y++){
                        for (int x = j - rings; x <= j + rings; x++) {
                            if (y < 0 || y >= 8 || x < 0 || x >= 8) continue;
                            if (y == i - rings || y == i + rings || x == j - rings || x == j + rings) {
                                masks[rings-1][i * 8 + j] |= 1ULL << (y * 8 + x);
                            }
                        }
                    }
                    //std::cout << i * 8 + j << " " << masks[rings-1][i * 8 + j] << std::endl;
                }
            }
        }
    }

    uint64_t masks[n][64]{};
};

constexpr KingRing king_ring_zone = KingRing<2>();


#endif //TUNER_ALTAIR_H
