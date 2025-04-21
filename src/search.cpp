#include "search.h"
#include "uci.h"

namespace search
{

PV::PV()
{
    this->clear();
};

void PV::clear()
{
    for (i32 i = 0; i < MAX_PLY; ++i) {
        this->data[i] = move::NONE_MOVE;
    }

    this->count = 0;
};

void PV::update(u16 move, const PV& other)
{
    this->data[0] = move;

    for (i32 i = 0; i < other.count; ++i) {
        this->data[i + 1] = other.data[i];
    }

    this->count = other.count + 1;
};

void Data::clear()
{
    for (i32 i = 0; i < MAX_PLY; ++i) {
        this->killer_table[i] = move::NONE_MOVE;
    }

    for (i32 p = 0; p < 12; ++p) {
        for (i32 sq = 0; sq < 64; ++sq) {
            this->history_table[p][sq] = 0;
        }
    }

    this->board = Board();

    this->ply = 0;

    this->nodes = 0;
    this->seldepth = 0;
};

Engine::Engine()
{
    this->clear();
};

void Engine::init()
{
    this->table.init(2);
};

void Engine::clear()
{
    this->running.clear();
    this->thread = nullptr;

    this->table.clear();
};

bool Engine::search(Board uci_board, Settings uci_setting)
{
    if (this->running.test() || this->thread != nullptr) {
        return false;
    }

    // Updates table
    this->table.update();

    // Sets search time
    u64 time_start = timer::get_current();

    this->time_end_soft = time_start + timer::get_available_soft(uci_setting.time[uci_board.get_color()], uci_setting.inc[uci_board.get_color()], uci_setting.movestogo);
    this->time_end_hard = time_start + timer::get_available_hard(uci_setting.time[uci_board.get_color()]);

    if (uci_setting.infinite) {
        this->time_end_soft = UINT64_MAX;
        this->time_end_hard = UINT64_MAX;
    }

    // Starts search thread
    this->running.test_and_set();

    this->thread = new std::thread([&] (Board board, Settings settings) {
        // Storing best pv lines found in each iteration
        std::vector<PV> pv_history = {};

        // Iterative deepening
        for (i32 i = 1; i < settings.depth; ++i) {
            // Inits search data
            Data data;
            data.clear();

            data.board = board;

            auto pv = PV();

            // Does negamax with alpha beta
            auto time_1 = std::chrono::high_resolution_clock::now();

            i32 score = this->negamax(data, -eval::score::INFINITE, eval::score::INFINITE, i, pv);

            auto time_2 = std::chrono::high_resolution_clock::now();

            // Saves pv line
            if (pv.count != 0 && pv.data[0] != move::NONE_MOVE) {
                pv_history.push_back(pv);
            }

            // Prints infos
            u64 dt = std::chrono::duration_cast<std::chrono::milliseconds>(time_2 - time_1).count();

            uci::print_info(i, data.seldepth, score, data.nodes, dt, this->table.hashfull(), pv_history.back());

            // Checks time
            if (!settings.infinite && timer::get_current() >= this->time_end_soft) {
                this->running.clear();
            }

            if (!this->running.test()) {
                break;
            }
        }

        // Prints best move
        uci::print_bestmove(pv_history.back().data[0]);
    }, uci_board, uci_setting);

    return true;
};

bool Engine::stop()
{
    if (this->thread == nullptr) {
        return false;
    }

    this->running.clear();

    return this->join();
};

bool Engine::join()
{
    if (this->thread == nullptr) {
        return false;
    }

    if (!this->thread->joinable()) {
        return false;
    }

    this->thread->join();

    delete this->thread;
    this->thread = nullptr;

    return true;
};

// Alpha-beta
i32 Engine::negamax(Data& data, i32 alpha, i32 beta, i32 depth, PV& pv)
{
    // Quiensence search
    if (depth <= 0) {
        return this->qsearch(data, alpha, beta, pv);
    }

    // Aborts search
    if ((data.nodes & 0xFFF) == 0) {
        u64 time_now = timer::get_current();

        if (time_now >= this->time_end_hard) {
            this->running.clear();
        }
    }

    if (!this->running.test()) {
        return eval::score::DRAW;
    }

    // Updates data
    data.nodes += 1;
    data.seldepth = std::max(data.seldepth, data.ply);

    // Checks drawn
    if (data.board.is_drawn_repitition() || data.board.is_drawn_fifty_move() || data.board.is_drawn_insufficient()) {
        return i32(data.nodes & 0b10) - 1;
    }

    // Mate distance pruning
    alpha = std::max(alpha, data.ply - eval::score::MATE);
    beta = std::min(beta, eval::score::MATE - data.ply - 1);

    if (alpha >= beta) {
        return alpha;
    }

    // Probes table
    auto [table_hit, table_entry] = this->table.get(data.board.get_hash());

    u16 table_move = move::NONE_MOVE;

    if (table_hit) {
        table_move = table_entry->get_move();

        u8 table_bound = table_entry->get_bound();
        i32 table_score = table_entry->get_score(data.ply);
        i32 table_depth = table_entry->get_depth();

        // Checks if the same position has already been searched to at least an equal depth
        // Returns when score is exact or produces a cutoff
        if (table_depth >= depth) {
            if ((table_bound == transposition::bound::EXACT) ||
                (table_bound == transposition::bound::LOWER && table_score >= beta) ||
                (table_bound == transposition::bound::UPPER && table_score <= alpha)) {
                return table_score;
            }
        }
    }

    // Inits data
    pv.count = 0;
    auto pv_next = PV();

    i32 best = -eval::score::INFINITE;
    u16 best_move = move::NONE_MOVE;

    i32 alpha_old = alpha;

    // Generate moves and scores them
    auto moves = move::generate::get_legal<move::generate::type::ALL>(data.board);
    auto moves_scores = move::order::get_score(moves, data, table_move);

    // Continues searching
    for (usize i = 0; i < moves.size(); ++i) {
        // Picks the move to search based on move ordering
        move::order::sort(moves, moves_scores, i);

        // Makes
        data.board.make(moves[i]);
        data.ply += 1;

        // Prefetch table
        this->table.prefetch(data.board.get_hash());

        // Searchs deeper
        i32 score = -this->negamax(data, -beta, -alpha, depth - 1, pv_next);

        // Unmakes
        data.board.unmake(moves[i]);
        data.ply -= 1;

        // Aborts search
        if (!this->running.test()) {
            return eval::score::DRAW;
        }

        bool is_quiet = data.board.get_piece_at(move::get_square_to(moves[i])) == piece::NONE || move::get_type(moves[i]) == move::type::CASTLING;

        // Updates values
        if (score > best) {
            best = score;
            best_move = moves[i];

            if (score > alpha) {
                alpha = score;
    
                // Stores history moves
                if (is_quiet) {
                    i8 piece = data.board.get_piece_at(move::get_square_from(moves[i]));
                    i8 to = move::get_square_to(moves[i]);
    
                    data.history_table[piece][to] += depth;
                }
    
                // Updates pv line
                pv.update(moves[i], pv_next);
            }
        }

        // Fail-soft cutoff
        if (score >= beta) {
            // Stores killer moves
            if (is_quiet) {
                data.killer_table[data.ply] = moves[i];
            }

            break;
        }
    }

    // Checks stalemate or checkmate
    if (moves.size() == 0) {
        if (data.board.is_in_check(data.board.get_color())) {
            return -eval::score::MATE + data.ply;
        }
        else {
            return i32(data.nodes & 0b10) - 1;
        }
    }

    // Updates table
    u8 bound =
        best >= beta ? transposition::bound::LOWER :
        best > alpha_old ? transposition::bound::EXACT :
        transposition::bound::UPPER;
    
    table_entry->set(data.board.get_hash(), best_move, best, eval::score::NONE, depth, true, bound, data.ply, this->table.age);

    return best;
};

// Quiescence search
i32 Engine::qsearch(Data& data, i32 alpha, i32 beta, PV& pv)
{
    // Aborts search
    if ((data.nodes & 0xFFF) == 0) {
        u64 time_now = timer::get_current();

        if (time_now >= this->time_end_hard) {
            this->running.clear();
        }
    }

    if (!this->running.test()) {
        return eval::score::DRAW;
    }

    // Updates data
    data.nodes += 1;
    data.seldepth = std::max(data.seldepth, data.ply);

    // Checks if we're in check
    bool is_in_check = data.board.is_in_check(data.board.get_color());

    // Gets static eval
    i32 standpat = eval::get(data.board);
    
    // Max ply reached
    if (data.ply >= MAX_PLY) {
        return is_in_check ? 0 : standpat;
    }

    // Probes table
    auto [table_hit, table_entry] = this->table.get(data.board.get_hash());

    u16 table_move = move::NONE_MOVE;

    if (table_hit) {
        table_move = table_entry->get_move();

        u8 table_bound = table_entry->get_bound();
        i32 table_score = table_entry->get_score(data.ply);
        i32 table_depth = table_entry->get_depth();

        // Returns when score is exact or produces a cutoff
        if ((table_bound == transposition::bound::EXACT) ||
            (table_bound == transposition::bound::LOWER && table_score >= beta) ||
            (table_bound == transposition::bound::UPPER && table_score <= alpha)) {
            return table_score;
        }
    }

    // Inits data
    pv.count = 0;
    auto pv_next = PV();

    i32 best;
    u16 best_move = move::NONE_MOVE;

    i32 alpha_old = alpha;

    if (is_in_check) {
        // Possible mate here
        best = -eval::score::MATE + data.ply;
    }
    else {
        best = standpat;
    
        // Prunes
        if (standpat >= beta) {
            return standpat;
        }
    
        // Updates alpha
        if (alpha < standpat) {
            alpha = standpat;
        }
    }

    // If we're in check, generates all legal moves, else generates noisy moves
    auto moves = 
        is_in_check ?
        move::generate::get_legal<move::generate::type::ALL>(data.board) :
        move::generate::get_legal<move::generate::type::NOISY>(data.board);
    
    // Scores moves
    auto moves_scores = move::order::get_score(moves, data, table_move);

    // Makes moves
    for (usize i = 0; i < moves.size(); ++i) {
        // Picks the move to search based on move ordering
        move::order::sort(moves, moves_scores, i);

        // Makes
        data.board.make(moves[i]);
        data.ply += 1;

        // Searches deeper
        i32 score = -this->qsearch(data, -beta, -alpha, pv_next);

        // Unmakes
        data.board.unmake(moves[i]);
        data.ply -= 1;

        // Aborts search
        if (!this->running.test()) {
            return eval::score::DRAW;
        }

        // Updates values
        if (score > best) {
            best = score;

            if (score > alpha) {
                alpha = score;

                // Updates pv line
                pv.update(moves[i], pv_next);
            }
        }

        // Cut off
        if (score >= beta) {
            break;
        }
    }

    // Updates table
    u8 bound =
        best >= beta ? transposition::bound::LOWER :
        best > alpha_old ? transposition::bound::EXACT :
        transposition::bound::UPPER;
    
    table_entry->set(data.board.get_hash(), best_move, best, eval::score::NONE, 0, true, bound, data.ply, this->table.age);

    return best;
};

};