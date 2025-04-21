#pragma once

#include "chess/chess.h"

namespace eval::score
{

constexpr i32 DRAW = 0;
constexpr i32 MATE = 32000 + MAX_PLY;
constexpr i32 MATE_FOUND = MATE - MAX_PLY;
constexpr i32 NONE = MATE + 1;
constexpr i32 INFINITE = INT16_MAX;

constexpr i32 create(i32 midgame, i32 endgame)
{
    assert(midgame >= INT16_MIN && midgame <= INT16_MAX);
    assert(endgame >= INT16_MIN && endgame <= INT16_MAX);

    return midgame + static_cast<i32>(static_cast<u32>(endgame) << 16);
};

constexpr i32 get_midgame(i32 score)
{
    return static_cast<int16_t>(static_cast<uint16_t>(static_cast<u32>(score)));
};

constexpr i32 get_endgame(i32 score)
{
    return static_cast<int16_t>(static_cast<uint16_t>(static_cast<u32>(score + 0x8000) >> 16));
};

};

namespace eval
{

#define S(a, b) score::create(a, b)

struct Weight
{
    i32 material_pawn;
    i32 material_knight;
    i32 material_bishop;
    i32 material_rook;
    i32 material_queen;
    i32 material_king;

    i32 table[6][64];

    i32 mobility_knight;
    i32 mobility_bishop;
    i32 mobility_rook;
    i32 mobility_queen;
    i32 mobility_king;

    i32 tempo;
};

constexpr Weight DEFAULT = Weight {
    .material_pawn = 100,
    .material_knight = 320,
    .material_bishop = 330,
    .material_rook = 500,
    .material_queen = 900,
    .material_king = 10000,

    .table = {
        {
            0, 0, 0, 0, 0, 0, 0, 0,
            50, 50, 50, 50, 50, 50, 50, 50,
            10, 10, 20, 30, 30, 20, 10, 10,
            5, 5, 10, 25, 25, 10, 5, 5,
            0, 0, 0, 20, 20, 0, 0, 0,
            5, -5, -10, 0, 0, -10, -5, 5,
            5, 10, 10, -20, -20, 10, 10, 5,
            0, 0, 0, 0, 0, 0, 0, 0
        },
        {
            -50, -40, -30, -30, -30, -30, -40, -50,
            -40, -20, 0, 0, 0, 0, -20, -40,
            -30, 0, 10, 15, 15, 10, 0, -30,
            -30, 5, 15, 20, 20, 15, 5, -30,
            -30, 0, 15, 20, 20, 15, 0, -30,
            -30, 5, 10, 15, 15, 10, 5, -30,
            -40, -20, 0, 5, 5, 0, -20, -40,
            -50, -40, -30, -30, -30, -30, -40, -50,
        },
        {
            -20, -10, -10, -10, -10, -10, -10, -20,
            -10, 0, 0, 0, 0, 0, 0, -10,
            -10, 0, 5, 10, 10, 5, 0, -10,
            -10, 5, 5, 10, 10, 5, 5, -10,
            -10, 0, 10, 10, 10, 10, 0, -10,
            -10, 10, 10, 10, 10, 10, 10, -10,
            -10, 5, 0, 0, 0, 0, 5, -10,
            -20, -10, -10, -10, -10, -10, -10, -20,
        },
        {
            0, 0, 0, 0, 0, 0, 0, 0,
            5, 10, 10, 10, 10, 10, 10, 5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            0, 0, 0, 5, 5, 0, 0, 0
        },
        {
            -20, -10, -10, -5, -5, -10, -10, -20,
            -10, 0, 0, 0, 0, 0, 0, -10,
            -10, 0, 5, 5, 5, 5, 0, -10,
            -5, 0, 5, 5, 5, 5, 0, -5,
            0, 0, 5, 5, 5, 5, 0, -5,
            -10, 5, 5, 5, 5, 5, 0, -10,
            -10, 0, 5, 0, 0, 0, 0, -10,
            -20, -10, -10, -5, -5, -10, -10, -20
        },
        {
            -30, -40, -40, -50, -50, -40, -40, -30,
            -30, -40, -40, -50, -50, -40, -40, -30,
            -30, -40, -40, -50, -50, -40, -40, -30,
            -30, -40, -40, -50, -50, -40, -40, -30,
            -20, -30, -30, -40, -40, -30, -30, -20,
            -10, -20, -20, -20, -20, -20, -20, -10,
            20, 20, 0, 0, 0, 0, 20, 20,
            20, 30, 10, 0, 0, 10, 30, 20
        }
    },

    .mobility_knight = 10,
    .mobility_bishop = 5,
    .mobility_rook = 0,
    .mobility_queen = 0,
    .mobility_king = 0,

    .tempo = 20
};

i32 get(Board& board);

i32 get_material(Board& board);

i32 get_table(Board& board);

i32 get_mobility(Board& board);

};