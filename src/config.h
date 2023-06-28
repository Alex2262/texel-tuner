#ifndef CONFIG_H
#define CONFIG_H 1

#include<cstdint>
#include "engines/altair.h"

#define TAPERED 1

using TuneEval = Altair::AltairEval;
constexpr int32_t thread_count = 6;
constexpr double preferred_k = 2.53815; //2.71333; // 3.04511;
constexpr bool retune_from_zero = true;

#endif // !CONFIG_H
