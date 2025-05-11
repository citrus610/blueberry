#pragma once

#include "../chess/chess.h"

namespace history
{

constexpr i32 MAX = 1 << 14;

class Table
{
private:
    i32 data[12][64];
public:
    Table();
public:
    i32 get(Board& board, u16 move);
    void update(Board& board, u16 move, i32 bonus);
};

};