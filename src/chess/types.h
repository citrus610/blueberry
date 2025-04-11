#pragma once

#include <iostream>
#include <istream>
#include <ostream>
#include <fstream>
#include <sstream>

#include <functional>
#include <utility>
#include <cstdint>
#include <cstddef>
#include <algorithm>
#include <bit>
#include <bitset>
#include <cassert>
#include <chrono>

#include <vector>
#include <array>
#include <cctype>
#include <optional>

#ifdef USE_PEXT
#include <x86intrin.h>
#endif

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;

using usize = size_t;