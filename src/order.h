#pragma once

#include "chess/chess.h"

namespace search
{

struct Data;

};

namespace move::order
{

constexpr i32 MVV_LVA[5][6] = {
    { 150, 140, 130, 120, 110, 100 },
    { 250, 240, 230, 220, 210, 200 },
    { 350, 340, 330, 320, 210, 300 },
    { 450, 440, 430, 420, 410, 400 },
    { 550, 540, 530, 520, 510, 500 }
};

constexpr i32 HASH_SCORE = 100000;
constexpr i32 MVV_LVA_SCORE = 10000;
constexpr i32 KILLER_SCORE = 9000;

arrayvec<i32, move::MAX_MOVE> get_score(const arrayvec<u16, move::MAX_MOVE>& moves, search::Data& data, u16 hash_move);

void sort(arrayvec<u16, move::MAX_MOVE>& moves, arrayvec<i32, move::MAX_MOVE>& moves_scores, usize index);

};