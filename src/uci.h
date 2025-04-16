#pragma once

#include "search.h"

namespace uci
{

std::optional<u16> get_move(const std::string& token, Board& board);

std::optional<Board> get_command_position(std::string in);

std::optional<search::Info> get_command_go(std::string in);

void print_info(i32 depth, i32 score, u64 nodes, search::PV pv);

void print_bestmove(u16 move);

};