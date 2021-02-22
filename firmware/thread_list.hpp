#pragma once

#include "threads.hpp"

namespace threads {

  // A temporary short list of threads. Currently implemented as a const array
  // TODO: 
  // * complete the list (source?) and add support for reading/writing on flash memory
  // * add support for filtering (wrt unit type, etc.)
  const thread tpi_list[] = {
    {"28", 28_tpi},
    {"24", 24_tpi},
    {"20", 20_tpi},
    {"18", 18_tpi},
    {"16", 16_tpi},
    {"15", 15_tpi},
    {"12", 12_tpi},
    {"10", 10_tpi},
    {"8", 8_tpi},
    {"6", 6_tpi},
    {"4", 4_tpi},
  };

  const thread mm_list[] = {
    {"M1.6",0.35_mm},
    {"M2 ", 0.40_mm},
    {"M2.5",0.45_mm},
    {"M3 ", 0.50_mm},
    {"M4 ", 0.70_mm},
    {"M5 ", 0.80_mm},
    {"M6 ", 1.00_mm},
    {"M8 ", 1.25_mm},
    {"M10", 1.50_mm},
    {"M12", 1.75_mm}
  };

  const int16_t tpi_list_size = std::extent<decltype(tpi_list)>::value;
  const int16_t mm_list_size = std::extent<decltype(mm_list)>::value;

  constexpr int16_t default_tpi_index = 0;
  constexpr int16_t default_mm_index = 0;
}