#pragma once

#include <atomic>
#include <thread>
#include <cmath>

#include "eval.h"
#include "history.h"
#include "order.h"
#include "timer.h"
#include "table.h"

namespace search::params
{

namespace aw
{
    constexpr i32 DEPTH = 4;
    constexpr i32 DELTA = 25;
};

namespace rfp
{
    constexpr i32 DEPTH = 4;
    constexpr i32 MARGIN = 50;
};

namespace nmp
{
    constexpr i32 DEPTH = 3;
    constexpr i32 REDUCTION = 4;
    constexpr i32 REDUCTION_EVAL_MAX = 3;
    constexpr i32 DIVISOR_DEPTH = 5;
    constexpr i32 DIVISOR_EVAL = 200;
};

namespace lmp
{
    constexpr i32 BASE = 3;
};

namespace fp
{
    constexpr i32 BASE = 50;
    constexpr i32 COEF = 50;
    constexpr i32 DEPTH = 10;
    constexpr i32 QS_MARGIN = 150;
};

namespace hp
{
    constexpr i32 DEPTH = 6;
    constexpr i32 MARGIN = -1024;
};

namespace lmr
{
    constexpr i32 DEPTH = 3;
    constexpr i32 HISTORY_DIVISOR_QUIET = 8192;
};

namespace see
{
    constexpr i32 MARGIN_QUIET = -100;
    constexpr i32 MARGIN_NOISY = -25;
    constexpr i32 QS_MARGIN = -50;
};

namespace history
{
    constexpr i32 BONUS_COEF = 300;
    constexpr i32 BONUS_BIAS = -250;
};

};

namespace search
{

void init();

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
    PV pvs[MAX_PLY];
public:
    Board board;
    i32 ply;
public:
    history::Quiet history_quiet;
    history::Noisy history_noisy;
public:
    u16 killers[MAX_PLY];
    u16 moves[MAX_PLY];
    i32 evals[MAX_PLY];
public:
    u64 nodes;
    i32 seldepth;
public:
    Data(Board board);
public:
    void clear();
};

class Engine
{
private:
    std::atomic_flag running;
    std::thread* thread;
private:
    u64 time_soft;
    u64 time_hard;
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
    i32 aspiration_window(Data& data, i32 depth, i32 score_old);
    template <bool PV>
    i32 pvsearch(Data& data, i32 alpha, i32 beta, i32 depth);
    template <bool PV>
    i32 qsearch(Data& data, i32 alpha, i32 beta);
};

};