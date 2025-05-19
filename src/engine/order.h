#pragma once

#include "../chess/chess.h"

namespace search
{

class Data;

};

namespace move::order
{

constexpr i32 HASH_SCORE = 10000000;
constexpr i32 NOISY_SCORE = 1000000;
constexpr i32 KILLER_SCORE = 90000;

arrayvec<i32, move::MAX> get_score(const arrayvec<u16, move::MAX>& moves, search::Data& data, u16 hash_move);

void sort(arrayvec<u16, move::MAX>& moves, arrayvec<i32, move::MAX>& scores, usize index);

};