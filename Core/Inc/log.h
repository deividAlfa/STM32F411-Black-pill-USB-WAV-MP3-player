#pragma once

//#include "Inc/term_io.h"

#define log_debug(...) iprintf(__VA_ARGS__)
#define log_error(...) iprintf(__VA_ARGS__)
#define log_fatal_and_die(...) { iprintf(__VA_ARGS__); while (1) {} }
