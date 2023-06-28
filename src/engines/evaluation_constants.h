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

constexpr SCORE_TYPE PASSED_PAWN_BONUSES[8] = {0};

constexpr SCORE_TYPE PASSED_PAWN_BLOCKERS[6][8] = {0};

#endif //TUNER_EVALUATION_CONSTANTS_H
