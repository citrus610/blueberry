#pragma once

#include "chess/chess.h"

namespace timer
{

inline u64 get_available(u64 remain, u64 increment, std::optional<u64> movestogo = {})
{
    u64 mtg = movestogo.value_or(45) + 5;

    return (remain - increment) / mtg + increment / 2;
};

};