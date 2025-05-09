#pragma once

#include <atomic>
#include <thread>
#include <cmath>

#include "eval.h"
#include "order.h"
#include "timer.h"
#include "transposition.h"

namespace search
{

namespace constants
{

namespace rfp
{

constexpr i32 DEPTH = 4;
constexpr i32 MARGIN = 50;

};

namespace nmp
{

constexpr i32 REDUCTION = 4;
constexpr i32 REDUCTION_EVAL_MAX = 3;
constexpr i32 DIVISOR_DEPTH = 5;
constexpr i32 DIVISOR_EVAL = 200;

};

namespace lmp
{

constexpr i32 BASE = 3;

};

namespace lmr
{

constexpr i32 DEPTH = 3;

const auto TABLE = [] {
    std::array<std::array<i32, move::MAX_MOVE>, MAX_PLY> table;

    for (i32 depth = 0; depth < MAX_PLY; ++depth) {
        for (i32 moves = 0; moves < move::MAX_MOVE; ++moves) {
            if (depth == 0 || moves == 0) {
                table[depth][moves] = 0;
                continue;
            }

            table[depth][moves] = i32(std::log(depth) * std::log(moves) * 0.35 + 0.6);
        }
    }

    return table;
} ();

};

};

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
    u16 data[MAX_PLY] = { move::NONE };
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
public:
    u16 killer_table[MAX_PLY];
    i32 history_table[12][64];
public:
    Board board;
    i32 ply;
    u16 moves[MAX_PLY];
    i32 evals[MAX_PLY];
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