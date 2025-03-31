#pragma once

#include "piece.h"
#include "square.h"

// bit 0- 5: destination square
// bit 6-11: origin square
// bit 12-13: promotion piece type
// bit 14-15: move type: promotion, en passant, castling
namespace chess::move
{

constexpr u16 NONE_MOVE = 0;
constexpr u16 NULL_MOVE = 65;

namespace type
{

constexpr u16 NORMAL = 0;
constexpr u16 PROMOTION = 1 << 14;
constexpr u16 ENPASSANT = 2 << 14;
constexpr u16 CASTLING = 3 << 14;

};

[[nodiscard]] constexpr u16 get(i8 square_from, i8 square_to) noexcept
{
    assert(square::is_valid(square_from));
    assert(square::is_valid(square_to));
    
    return (square_from << 6) + square_to;
};

template <u16 T = type::NORMAL>
[[nodiscard]] constexpr u16 get_make(i8 square_from, i8 square_to, i8 piece_type = piece::type::KNIGHT) noexcept
{
    assert(square::is_valid(square_from));
    assert(square::is_valid(square_to));
    assert(piece_type >= piece::type::KNIGHT && piece_type <= piece::type::QUEEN);
    
    return T + ((piece_type - piece::type::KNIGHT) << 12) + (square_from << 6) + square_to;
};

[[nodiscard]] constexpr i8 get_square_from(u16 move) noexcept
{
    return (move >> 6) & 0x3F;
};

[[nodiscard]] constexpr i8 get_square_to(u16 move) noexcept
{
    return move & 0x3F;
};

[[nodiscard]] constexpr u16 get_type(u16 move) noexcept
{
    return move & (3 << 14);
};

[[nodiscard]] constexpr i8 get_promotion_type(u16 move) noexcept
{
    return i8((move >> 12) & 3) + piece::type::KNIGHT;
};

};