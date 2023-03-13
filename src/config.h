#ifndef CONFIG_H
#define CONFIG_H 1

#include<cstdint>
#include "engines/fourku.h"
#include "engines/toy.h"
#include "engines/toy_tapered.h"
#include "engines/altair.h"

#define TAPERED 1

using TuneEval = Altair::AltairEval;
constexpr int32_t thread_count = 4;
constexpr double preferred_k = 3;
constexpr bool retune_from_zero = false;

#endif // !CONFIG_H
