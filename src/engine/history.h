#pragma once

#include "../chess/chess.h"

namespace history
{

constexpr i32 MAX = 1 << 14;

class Quiet
{
private:
    i32 data[12][64];
public:
    Quiet();
public:
    i32& get(Board& board, const u16& move);
    void update(Board& board, const u16& move, i32 bonus);
};

class Noisy
{
private:
    i32 data[12][64][6];
public:
    Noisy();
public:
    i32& get(Board& board, const u16& move);
    i32& get(Board& board, const u16& move, i8 captured);
    void update(Board& board, const u16& move, i32 bonus);
};

};