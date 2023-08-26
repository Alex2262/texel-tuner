//
// Created by Alex Tian on 2/24/2023.
//

#ifndef TUNER_ALTAIR_H
#define TUNER_ALTAIR_H

#include "../base.h"
#include <string>
#include <vector>
#include <cstdint>
#include "types.h"
#include "bitboard.h"
#include "fixed_vector.h"

typedef unsigned __int128 uint128_t;

inline int centerDistance(int square)
{
    return (int)(2 * ((0xFFFFC3C3C3C3FFFF >> square) & 1) + ((0xFF81BDA5A5BD81FF >> square) & 1));
}

struct Trace {
    int score={};

    // short mobility[6][2]{};
    short piece_values[6][2]{};
    short capture_bonus[2]{};

};


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
