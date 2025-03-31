#pragma once

#include "color.h"

namespace chess::direction
{

enum : i8
{
    NORTH = 8,
    WEST = -1,
    SOUTH = -8,
    EAST = 1,
    NORTH_EAST = 9,
    NORTH_WEST = 7,
    SOUTH_WEST = -9,
    SOUTH_EAST = -7
};

[[nodiscard]] constexpr i8 get_color_direction(i8 direction, i8 color) noexcept
{
    assert(color::is_valid(color));
    
    return color == color::WHITE ? direction : -direction;
};

};

namespace chess::file
{

enum : i8
{
    FILE_A,
    FILE_B,
    FILE_C,
    FILE_D,
    FILE_E,
    FILE_F,
    FILE_G,
    FILE_H,
    NONE = -1
};

[[nodiscard]] constexpr bool is_valid(i8 file) noexcept
{
    return file >= file::FILE_A && file <= file::FILE_H;
};

[[nodiscard]] constexpr i8 get(char c) noexcept
{
    if (c < 'a' || c > 'h') {
        return file::NONE;
    }

    return c - 'a';
};

[[nodiscard]] constexpr char get_char(i8 file) noexcept
{
    assert(file::is_valid(file));

    return 'a' + file;
};

};

namespace chess::rank
{

enum : i8
{
    RANK_1,
    RANK_2,
    RANK_3,
    RANK_4,
    RANK_5,
    RANK_6,
    RANK_7,
    RANK_8,
    NONE = -1
};

[[nodiscard]] constexpr bool is_valid(i8 rank) noexcept
{
    return rank >= rank::RANK_1 && rank <= rank::RANK_8;
};

[[nodiscard]] constexpr bool is_back_rank(i8 rank, i8 color) noexcept
{
    assert(rank::is_valid(rank));
    assert(color::is_valid(color));

    return color == color::WHITE ? rank == rank::RANK_1 : rank == rank::RANK_8;
};

[[nodiscard]] constexpr i8 get(char c) noexcept
{
    if (c < '1' || c > '8'){
        return rank::NONE;
    }

    return c - '1';
};

[[nodiscard]] constexpr i8 get_color_rank(i8 rank, i8 color) noexcept
{
    assert(rank::is_valid(rank));
    assert(color::is_valid(color));

    return color == color::WHITE ? rank : rank::RANK_8 - rank;
};

[[nodiscard]] constexpr i8 get_back_rank(i8 color) noexcept
{
    assert(color::is_valid(color));

    return color == color::WHITE ? rank::RANK_1 : rank::RANK_8;
};

[[nodiscard]] constexpr char get_char(i8 rank) noexcept
{
    assert(rank::is_valid(rank));

    return '1' + rank;
};

};

namespace chess::square
{

enum : i8
{
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    NONE = -1
};

[[nodiscard]] constexpr bool is_valid(i8 square) noexcept
{
    return square >= square::A1 && square <= square::H8;
};

[[nodiscard]] constexpr i8 get(i8 file, i8 rank) noexcept
{
    assert(file::is_valid(file));
    assert(rank::is_valid(rank));

    return file | (rank << 3);
};

[[nodiscard]] constexpr i8 get_file(i8 square) noexcept
{
    assert(square::is_valid(square));
    
    return square & 7;
};

[[nodiscard]] constexpr i8 get_rank(i8 square) noexcept
{
    assert(square::is_valid(square));

    return square >> 3;
};

[[nodiscard]] constexpr i8 get_flip_file(i8 square) noexcept
{
    assert(square::is_valid(square));

    return square ^ 7;
};

[[nodiscard]] constexpr i8 get_flip_rank(i8 square) noexcept
{
    assert(square::is_valid(square));

    return square ^ 56;
};

[[nodiscard]] constexpr i8 get_relative(i8 square, i8 color) noexcept
{
    assert(square::is_valid(square));
    assert(color::is_valid(color));

    return square ^ (color * 56);
};

};