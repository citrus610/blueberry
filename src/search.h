#pragma once

#include <atomic>
#include <thread>

#include "eval.h"
#include "order.h"
#include "timer.h"
#include "transposition.h"

namespace search
{

enum class node
{
    ROOT,
    PV,
    NORMAL
};

struct Settings
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
    u16 data[MAX_PLY] = { move::NONE_MOVE };
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
    PV pv_table[MAX_PLY];
    u16 killer_table[MAX_PLY];
    i32 history_table[12][64];
public:
    Board board;
    i32 ply;
public:
    u64 nodes;
    i32 seldepth;
public:
    void clear();
};

class Engine
{
private:
    std::atomic_flag running;
    std::thread* thread;
private:
    u64 time_end_soft;
    u64 time_end_hard;
public:
    transposition::Table table;
public:
    Engine();
public:
    void init();
    void clear();
    bool search(Board board, Settings info);
    bool stop();
    bool join();
public:
    template <node NODE>
    i32 pvsearch(Data& data, i32 alpha, i32 beta, i32 depth);
    i32 qsearch(Data& data, i32 alpha, i32 beta);
};

};