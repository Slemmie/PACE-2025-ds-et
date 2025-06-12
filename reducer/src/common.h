#pragma once

#include <cstdint>

typedef uint32_t szt;

#include <cassert>

#ifdef NO_ASSERT
#define ASSERT(x)
#else
#define ASSERT(x) assert(x)
#endif
