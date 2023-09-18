//
// Created by Alexander Tian on 6/27/23.
//

#ifndef TUNER_EVALUATION_CONSTANTS_H
#define TUNER_EVALUATION_CONSTANTS_H

#include <array>
#include "types.h"
#include "../base.h"

constexpr int GAME_PHASES[6] = {0, 1, 1, 2, 4, 0};
int PIECE_VALUES_MID[6]{};

int PIECE_VALUES_END[6]{};

int PIECE_SQUARE_TABLES_MID[6][64]{};

int PIECE_SQUARE_TABLES_END[6][64]{};

// int MOBILITY_MID[4]{};
// int MOBILITY_END[4]{};

int BISHOP_PAIR_MID = 0;
int BISHOP_PAIR_END = 0;

int TEMPO_MID =   0;
int TEMPO_END =   0;

/*
constexpr int PIECE_RANK_MID[6][8] = {0};
constexpr int PIECE_RANK_END[6][8] = {0};

constexpr int PIECE_FILE_MID[6][8] = {0};
constexpr int PIECE_FILE_END[6][8] = {0};

constexpr int CENTRALITY_MID[6][4] = {0};
constexpr int CENTRALITY_END[6][4] = {0};
 */



#endif //TUNER_EVALUATION_CONSTANTS_H
