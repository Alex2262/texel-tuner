#ifndef CONFIG_H
#define CONFIG_H 1

#include<cstdint>
#include "engines/altair.h"

#define TAPERED 1

using TuneEval = Altair::AltairEval;
constexpr int32_t thread_count = 8;
constexpr double preferred_k = 2.53815; //2.71333; // 3.04511;
constexpr bool retune_from_zero = true;
constexpr int32_t max_epoch = 50000;
constexpr bool enable_qsearch = false;
constexpr bool print_data_entries = false;
constexpr int32_t data_load_print_interval = 10000;

#endif // !CONFIG_H
