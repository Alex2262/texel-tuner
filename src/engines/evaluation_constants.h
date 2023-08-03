//
// Created by Alexander Tian on 6/27/23.
//

#ifndef TUNER_EVALUATION_CONSTANTS_H
#define TUNER_EVALUATION_CONSTANTS_H

#include <array>
#include "types.h"
#include "../base.h"

constexpr int GAME_PHASES[6] = {0, 1, 1, 2, 4, 0};
int PIECE_VALUES_MID[6] = { 107, 424, 464, 596,1256,   0};

int PIECE_VALUES_END[6] = { 185, 421, 466, 813,1480,   0};

int PIECE_SQUARE_TABLES_MID[6][64] = {
        { -34, -34, -34, -34, -34, -34, -34, -34,
                151, 202, 115, 165, 139, 202,  11, -21,
                -51,  -8,  34,  42,  65,  67,  13, -25,
                -47, -12, -10,   8,  15,  19,   8, -36,
                -54, -22, -22,  -7,  -2,  -9,   4, -36,
                -56, -27, -23, -25, -10, -12,  23, -23,
                -61, -28, -39, -46, -33,   8,  29, -30,
                -34, -34, -34, -34, -34, -34, -34, -34},
        {-250,-123, -76,  -6, 125,-140, -36,-150,
                -17, -40,  74,  57,  60,  98,  -5,   3,
                -43,  45,  61,  95, 133, 151, 103,  64,
                7,  21,  39,  62,  38,  72,  28,  33,
                -13,  -8,  17,  18,  31,  27,  41,  -1,
                -29, -10,  10,   6,  14,  15,  17, -16,
                -52, -43, -19,  -3,  -4,   2, -12,  -3,
                -164, -28, -63, -51, -32, -25, -26, -80},
        { -69, -12,-124, -73, -70, -85, -19,   8,
                -32,  22,  -1, -10,  24,  67,  24, -32,
                -13,  27,  63,  43,  59,  71,  56,   9,
                -19,   8,  17,  60,  41,  50,  10,  -4,
                -16,   7,   6,  21,  40,   4,   4,  13,
                -8,   4,   4,   6,   0,   6,   9,  13,
                -13,   6,  10, -11,  -4,  -8,  25,   6,
                -2, -16, -23, -37, -45, -25, -42, -30},
        {  61,  65,  54,  63, 107,  89,  21, 134,
                28,  18,  58,  75,  77, 122,  47,  69,
                -24,  12,  33,  42,  49,  93, 114,  51,
                -31, -23,   6,  21,   5,  26, -15,  -5,
                -63, -63, -41, -32, -33, -24, -10, -25,
                -76, -52, -52, -51, -45, -46, -14, -49,
                -89, -46, -52, -54, -45, -32, -18,-120,
                -45, -43, -34, -30, -34, -38, -43, -42},
        { -40, -39,  28,  49, 117, 141,  77,  57,
                -15, -55, -22, -27, -17,  80,  31,  36,
                -28, -18,  -5, -12,   7, 109, 109,  40,
                -24, -31, -24, -21, -18,  15,   6,   3,
                -25, -21, -17, -22, -16,  -8,   9,  -7,
                -31,  -2, -20, -16, -19, -13,   8, -16,
                -40, -15,   1, -15,  -5,   3,   0, -33,
                -12, -34, -19,   3, -24, -38, -10, -55},
        {  12, 280, 193, 163,-175, -62,  72,  -6,
                207,  67,  86, 108,  88,  62, -38,-158,
                55, 102,  79,  15,  87, 124, 128, -29,
                -10,  -3,  17, -15, -58,  -5,  -4,-116,
                -58,  69, -28, -80, -54, -60, -34, -92,
                -16, -31, -68, -78, -59, -59,  -2, -27,
                14,   1, -29,-120, -80, -56,  24,  39,
                22,  64,  17,-115, -17, -77,  35,  42}
};

int PIECE_SQUARE_TABLES_END[6][64] = {
        { -74, -74, -74, -74, -74, -74, -74, -74,
                207, 191, 180, 122, 135, 137, 208, 235,
                113, 113,  80,  57,  42,  30,  81,  86,
                15,   4, -14, -34, -40, -34, -14,  -8,
                -10, -17, -39, -42, -45, -45, -32, -36,
                -25, -20, -39, -31, -28, -38, -36, -45,
                -9, -16, -17, -15, -15, -27, -31, -49,
                -74, -74, -74, -74, -74, -74, -74, -74},
        { -61, -23,  16, -17, -33,   2, -60,-125,
                -29,   8,  -7,  26,  11, -19,  -6, -37,
                -12,   6,  40,  42,  14,  19, -13, -42,
                -3,  34,  53,  60,  67,  56,  41,   5,
                -6,  18,  55,  66,  55,  57,  30,  -2,
                -31,   3,  13,  44,  39,  10,  -4, -25,
                -41, -10,  -1,   7,  15, -11, -18, -52,
                -14, -70, -12,  -4, -15, -15, -58, -39},
        { -13,  -9,  10,   7,  11,   0, -12, -29,
                -5,   1,  15,   6,   9, -10,  -2, -11,
                6,   5,   4,  12,  -2,  19,  12,  11,
                0,  23,  25,  24,  31,  12,  26,  10,
                -5,  10,  27,  33,  18,  20,   3, -22,
                -16,   3,  17,  18,  27,  15,  -6, -23,
                -20, -24,  -6,   2,   0, -17, -16, -48,
                -42, -17, -40,  -8,  -9, -24,  -4, -29},
        {  12,  13,  16,  17,   2,   0,  16,  -6,
                17,  25,  18,  18,   7, -16,   3,  -4,
                20,  12,  12,   8,   3,  -8, -21, -14,
                12,  12,  16,  11,  15,   4,   3,   0,
                7,  17,  13,  15,   8,  -7,  -8, -18,
                -13,  -6,  -6,  -1,  -3, -13, -19, -27,
                -10, -18,  -4,  -3, -10, -19, -29,   3,
                -19,   1,   4,   2,  -3, -13,  -3, -41},
        {  -5,  58,  36,  23,  10, -13,   5,   8,
                -24,  35,  55,  68,  88,  48,  49,  31,
                -21,  -4,  17,  76,  89,  35,  22,  69,
                -36,  18,  36,  57,  93,  71,  72,  75,
                -35,  25,  11,  52,  45,  38,  18,  27,
                -36, -64,   4, -13,  -7,   5, -21,  -5,
                -60, -58, -78, -39, -65, -86, -89, -46,
                -73, -71, -73,-114, -66, -84,-100, -86},
        {-109, -89, -60, -62,   1,  25,  -5, -31,
                -56,  24,  14,  -3,   9,  34,  53,  30,
                13,  26,  33,  23,  14,  47,  46,  21,
                3,  40,  40,  51,  53,  52,  43,  22,
                -26, -12,  37,  56,  55,  46,  25,  -2,
                -24,   6,  29,  43,  41,  31,   2, -17,
                -31, -17,  10,  29,  26,  15, -19, -47,
                -109, -62, -37,  -8, -57, -12, -47, -97}
};

int TEMPO_MID =   31;
int TEMPO_END =   41;

/*
constexpr int PIECE_RANK_MID[6][8] = {0};
constexpr int PIECE_RANK_END[6][8] = {0};

constexpr int PIECE_FILE_MID[6][8] = {0};
constexpr int PIECE_FILE_END[6][8] = {0};

constexpr int CENTRALITY_MID[6][4] = {0};
constexpr int CENTRALITY_END[6][4] = {0};
 */



#endif //TUNER_EVALUATION_CONSTANTS_H
