#pragma once

#include "types.h"

namespace chess::color
{

enum : i8
{
    WHITE,
    BLACK,
    NONE = -1
};

[[nodiscard]] constexpr bool is_valid(i8 color) noexcept
{
    return color == color::WHITE || color == color::BLACK;
};

[[nodiscard]] constexpr i8 get(char c) noexcept
{
    return c == 'w' ? color::WHITE : c == 'b' ? color::BLACK : color::NONE;
};

[[nodiscard]] constexpr i8 get_opposite(i8 color) noexcept
{
    assert(color::is_valid(color));

    return color ^ 1;
};

[[nodiscard]] constexpr char get_char(i8 color) noexcept
{
    assert(color::is_valid(color));
    
    return color == color::WHITE ? 'w' : 'b';
};

};