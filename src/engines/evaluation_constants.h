//
// Created by Alexander Tian on 6/27/23.
//

#ifndef TUNER_EVALUATION_CONSTANTS_H
#define TUNER_EVALUATION_CONSTANTS_H

#include <array>
#include "types.h"
#include "../base.h"

constexpr SCORE_TYPE PIECE_VALUES[6] = {0};

constexpr SCORE_TYPE PIECE_SQUARE_TABLES[6][64] = {0};

constexpr SCORE_TYPE PASSED_PAWN_BONUSES[3][8] = {0};

constexpr SCORE_TYPE PASSED_PAWN_BLOCKERS[6][8] = {0};
constexpr SCORE_TYPE PASSED_PAWN_BLOCKERS_2[6][8] = {0};

constexpr SCORE_TYPE PHALANX_PAWN_BONUSES[8] = {0};

constexpr SCORE_TYPE ISOLATED_PAWN_PENALTY = 0;

constexpr SCORE_TYPE BISHOP_PAIR_BONUS = 0;

constexpr SCORE_TYPE TEMPO_BONUS = 0;

constexpr SCORE_TYPE MOBILITY_VALUES[6] = {0};

constexpr SCORE_TYPE SEMI_OPEN_FILE_VALUES[6] = {0};
constexpr SCORE_TYPE OPEN_FILE_VALUES[6] = {0};

constexpr SCORE_TYPE PIECE_THREATS[6][6] = {0};

constexpr SCORE_TYPE KING_RING_ATTACKS[2][6] = {0};
constexpr SCORE_TYPE TOTAL_KING_RING_ATTACKS[30] = {0};

constexpr SCORE_TYPE KING_PAWN_SHIELD[5][8] = {0};
constexpr SCORE_TYPE KING_PAWN_STORM[6][8] = {0};


#endif //TUNER_EVALUATION_CONSTANTS_H
