#pragma once

#include "piece.h"
#include "square.h"

namespace move
{

constexpr usize MAX_MOVE = 256;

constexpr u16 NONE_MOVE = 0;
constexpr u16 NULL_MOVE = 65;

namespace type
{

constexpr u16 NORMAL = 0;
constexpr u16 PROMOTION = 1 << 14;
constexpr u16 ENPASSANT = 2 << 14;
constexpr u16 CASTLING = 3 << 14;

};

constexpr u16 create(i8 square_from, i8 square_to)
{
    assert(square::is_valid(square_from));
    assert(square::is_valid(square_to));
    
    return (square_from << 6) + square_to;
};

template <u16 T = type::NORMAL>
constexpr u16 get_make(i8 square_from, i8 square_to, i8 promotion_type = piece::type::KNIGHT)
{
    assert(square::is_valid(square_from));
    assert(square::is_valid(square_to));
    assert(promotion_type >= piece::type::KNIGHT && promotion_type <= piece::type::QUEEN);
    
    return T + ((promotion_type - piece::type::KNIGHT) << 12) + (square_from << 6) + square_to;
};

constexpr i8 get_square_from(u16 move)
{
    return (move >> 6) & 0x3F;
};

constexpr i8 get_square_to(u16 move)
{
    return move & 0x3F;
};

constexpr u16 get_type(u16 move)
{
    return move & (3 << 14);
};

constexpr i8 get_promotion_type(u16 move)
{
    return i8((move >> 12) & 3) + piece::type::KNIGHT;
};

};