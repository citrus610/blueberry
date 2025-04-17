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
    for (i32 i = 0; i < Board::MAX_PLY; ++i) {
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
    for (i32 i = 0; i < Board::MAX_PLY; ++i) {
        this->pv_table[i].clear();
    }

    for (i32 i = 0; i < Board::MAX_PLY; ++i) {
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
};

Engine::Engine()
{
    this->clear();
};

void Engine::clear()
{
    this->running.clear();
    this->thread = nullptr;
};

bool Engine::search(Board uci_board, Info uci_info)
{
    if (this->running.test() || this->thread != nullptr) {
        return false;
    }

    this->clear();

    // Starts search thread
    this->running.test_and_set();

    this->thread = new std::thread([&] (Board board, Info info) {
        // Gets start time
        auto time_start = std::chrono::high_resolution_clock::now();

        // Storing best pv lines found in each iteration
        std::vector<PV> pv_history = {};

        // Iterative deepening
        for (i32 i = 1; i < info.depth; ++i) {
            // Inits search data
            Data data;
            data.clear();

            data.board = board;

            // Does negamax with alpha beta
            i32 score = search::negamax(data, -eval::score::INFINITE, eval::score::INFINITE, i, running);

            // Stops when the uci client sends stop
            // We don't save the pv line when we stop the search abruptly
            if (!this->running.test()) {
                break;
            }

            // Saves pv line
            pv_history.push_back(data.pv_table[0]);

            // Prints infos
            uci::print_info(i, score, data.nodes, pv_history.back());

            // Checks time
            auto time_now = std::chrono::high_resolution_clock::now();

            u64 time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(time_now - time_start).count();

            if (i >= 7) {
                this->running.clear();
                break;
            }

            if (!info.infinite && time_elapsed >= timer::get_available(info.time[board.get_color()], info.inc[board.get_color()], info.movestogo)) {
                this->running.clear();
                break;
            }
        }

        // Prints best move
        uci::print_bestmove(pv_history.back().data[0]);
    }, uci_board, uci_info);

    return true;
};

bool Engine::stop()
{
    if (!this->running.test() || this->thread == nullptr) {
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

i32 negamax(Data& data, i32 alpha, i32 beta, i32 depth, std::atomic_flag& running)
{
    // Quiensence search
    if (depth <= 0) {
        return search::qsearch(data, alpha, beta, running);
    }

    // Updates nodes count
    data.nodes += 1;

    // Checks drawn
    if (data.board.is_drawn_repitition() || data.board.is_drawn_fifty_move() || data.board.is_drawn_insufficient()) {
        return i32(data.nodes & 0b10) - 1;
    }

    // Aborts search
    if (!running.test()) {
        return -eval::score::INFINITE;
    }

    // Best score
    i32 best = -eval::score::INFINITE;

    // Generate moves
    auto moves = move::generate::get_legal<move::generate::type::ALL>(data.board);
    auto moves_scores = move::order::get_score(moves, data);

    // Continues searching
    for (usize i = 0; i < moves.size(); ++i) {
        // Picks the move to search based on move ordering
        move::order::sort(moves, moves_scores, i);

        // Makes
        data.board.make(moves[i]);
        data.ply += 1;

        // Searchs deeper
        i32 score = -search::negamax(data, -beta, -alpha, depth - 1, running);

        // Unmakes
        data.board.unmake(moves[i]);
        data.ply -= 1;

        bool is_quiet = data.board.get_piece_at(move::get_square_to(moves[i])) == piece::NONE || move::get_type(moves[i]) == move::type::CASTLING;

        // Updates values
        if (score > best) {
            best = score;

            if (score > alpha) {
                alpha = score;
    
                // Stores history moves
                if (is_quiet) {
                    i8 piece = data.board.get_piece_at(move::get_square_from(moves[i]));
                    i8 to = move::get_square_to(moves[i]);
    
                    data.history_table[piece][to] += depth;
                }
    
                // Updates pv line
                data.pv_table[data.ply].update(moves[i], data.pv_table[data.ply + 1]);
            }
        }

        // Fail-soft cutoff
        if (score >= beta) {
            // Stores killer moves
            if (is_quiet) {
                data.killer_table[data.ply] = moves[i];
            }

            return best;
        }
    }

    // Checks stalemate or checkmate
    if (moves.size() == 0) {
        if (data.board.is_in_check(data.board.get_color())) {
            return -eval::score::MATE + data.board.get_ply();
        }
        else {
            return eval::score::DRAW;
        }
    }

    return best;
};

i32 qsearch(Data& data, i32 alpha, i32 beta, std::atomic_flag& running)
{
    // Updates nodes count
    data.nodes += 1;

    // Aborts search
    if (!running.test()) {
        return alpha;
    }

    // Gets static eval
    i32 standpat = eval::get(data.board);
    i32 best = standpat;

    if (data.ply >= Board::MAX_PLY) {
        return standpat;
    }

    // Prunes
    if (standpat >= beta) {
        return standpat;
    }

    if (alpha < standpat) {
        alpha = standpat;
    }

    // Gets capture moves
    auto moves = move::generate::get_legal<move::generate::type::NOISY>(data.board);
    auto moves_scores = move::order::get_score(moves, data);

    // Makes moves
    for (usize i = 0; i < moves.size(); ++i) {
        // Picks the move to search based on move ordering
        move::order::sort(moves, moves_scores, i);

        // Makes
        data.board.make(moves[i]);
        data.ply += 1;

        // Searches deeper
        i32 score = -search::qsearch(data, -beta, -alpha, running);

        // Unmakes
        data.board.unmake(moves[i]);
        data.ply -= 1;

        if (score > best) {
            best = score;
        }

        if (score > alpha) {
            alpha = score;

            // Updates pv line
            data.pv_table[data.ply].update(moves[i], data.pv_table[data.ply + 1]);
        }

        // Cut off
        if (score >= beta) {
            break;
        }
    }

    // Mates
    if (moves.size() == 0 && data.board.is_in_check(data.board.get_color())) {
        return -eval::score::MATE + data.board.get_ply();
    }

    return best;
};

};