#pragma once

#include <atomic>
#include <thread>

#include "eval.h"
#include "order.h"
#include "timer.h"

namespace search
{

struct Info
{
    i32 depth;
    u64 time[2];
    u64 inc[2];
    std::optional<i32> movestogo;
    bool infinite;
};

class PV
{
public:
    u16 data[Board::MAX_PLY] = { move::NONE_MOVE };
    i32 count = 0;
public:
    PV();
public:
    void clear();
    void update(u16 move, const PV& other);
};

class Data
{
public:
    PV pv_table[Board::MAX_PLY];
    u16 killer_table[Board::MAX_PLY];
    i32 history_table[12][64];
public:
    Board board;
    i32 ply;
public:
    u64 nodes;
public:
    void clear();
};

class Engine
{
private:
    std::atomic_flag running;
    std::thread* thread;
public:
    Engine();
public:
    void clear();
    bool search(Board board, Info info);
    bool stop();
    bool join();
};

i32 negamax(Data& data, i32 alpha, i32 beta, i32 depth, std::atomic_flag& running);

i32 qsearch(Data& data, i32 alpha, i32 beta, std::atomic_flag& running);

};