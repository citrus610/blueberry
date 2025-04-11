#pragma once

#include "types.h"
#include "color.h"
#include "square.h"
#include "piece.h"
#include "move.h"
#include "bitboard.h"
#include "castling.h"
#include "attack.h"
#include "zobrist.h"
#include "mask.h"
#include "board.h"
#include "movegen.h"

inline void init()
{
    zobrist::init();
    attack::init();
    mask::init();
};