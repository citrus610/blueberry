#pragma once

#include "../chess/chess.h"

namespace history
{

constexpr i16 MAX = 1 << 14;

constexpr i16 BONUS_COEF = 300;
constexpr i16 BONUS_BIAS = -250;

namespace quiet
{

class Table
{
private:
    i16 data[12][64];
public:
    Table();
public:
    i16& get(Board& board, const u16& move);
    void update(Board& board, const u16& move, i16 bonus);
};

};

namespace noisy
{

class Table
{
private:
    i16 data[12][64][6];
public:
    Table();
public:
    i16& get(Board& board, const u16& move);
    i16& get(Board& board, const u16& move, i8 captured);
    void update(Board& board, const u16& move, i16 bonus);
};

};

namespace cont
{

class Entry
{
public:
    i8 piece;
    i8 to;
public:
    Entry();
    Entry(Board& board, const u16& move);
public:
    bool is_valid();
};

class Table
{
private:
    i16 data[12][64][12][64];
public:
    Table();
public:
    i16& get(Board& board, const u16& move);
    void update(const Entry& entry, Board& board, const u16& move, i16 bonus);
};

};

inline void update(i16& entry, i16 bonus)
{
    bonus = std::clamp(bonus, i16(-MAX), i16(MAX));
    entry += bonus - entry * std::abs(bonus) / MAX;
};

inline i16 get_bonus(i32 depth)
{
    return depth * BONUS_COEF + BONUS_BIAS;
};

};