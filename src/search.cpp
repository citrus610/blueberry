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
        this->data[i] = move::NONE;
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
        this->pv_table[i].clear();
    }

    for (i32 i = 0; i < MAX_PLY; ++i) {
        this->killer_table[i] = move::NONE;
    }

    for (i32 p = 0; p < 12; ++p) {
        for (i32 sq = 0; sq < 64; ++sq) {
            this->history_table[p][sq] = 0;
        }
    }

    this->board = Board();
    this->ply = 0;

    for (i32 i = 0; i < MAX_PLY; ++i) {
        this->moves[i] = move::NONE;
    }

    for (i32 i = 0; i < MAX_PLY; ++i) {
        this->evals[i] = eval::score::NONE;
    }

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

            // Does negamax with alpha beta
            auto time_1 = std::chrono::high_resolution_clock::now();

            i32 score = this->pvsearch<node::ROOT>(data, -eval::score::INFINITE, eval::score::INFINITE, i);

            auto time_2 = std::chrono::high_resolution_clock::now();

            // Saves pv line
            if (data.pv_table[0].count != 0 && data.pv_table[0].data[0] != move::NONE) {
                pv_history.push_back(data.pv_table[0]);
            }

            // Prints infos
            u64 dt = std::chrono::duration_cast<std::chrono::milliseconds>(time_2 - time_1).count();

            uci::print_info(i, data.seldepth, score, data.nodes, dt, this->table.hashfull(), pv_history.back());

            // Avoids searching too shallow
            if (i < 4) {
                continue;
            }

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

// Principle variation search
template <node NODE>
i32 Engine::pvsearch(Data& data, i32 alpha, i32 beta, i32 depth)
{
    // Gets node type
    constexpr bool is_pv = NODE == node::PV || NODE == node::ROOT;
    constexpr bool is_root = NODE == node::ROOT;

    // Quiensence search
    // - We keep searching noisy moves until a quiet position is reached for static evaluation
    // - This reduces the horizon effect
    if (depth <= 0) {
        return this->qsearch(data, alpha, beta);
    }

    // Aborts search
    // - Every now and then check if we have exceeded the hard time limit
    // - Stops searching
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
    data.pv_table[data.ply].count = 0;
    data.nodes += 1;
    data.seldepth = std::max(data.seldepth, data.ply);

    // Early stop conditions
    // - Don't exit early in the root node, since this would prevent us from having a best move
    if (!is_root) {
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
    }

    // Probes transposition table
    // - Checks if the same position has already been searched to at least an equal depth in non PV nodes
    // - Returns when score is exact or produces a cutoff
    auto [table_hit, table_entry] = this->table.get(data.board.get_hash());

    u16 table_move = move::NONE;
    i32 table_eval = eval::score::NONE;
    u8 table_bound = transposition::bound::NONE;
    i32 table_score = eval::score::NONE;
    i32 table_depth = 0;

    if (table_hit) {
        table_move = table_entry->get_move();
        table_eval = table_entry->get_eval();
        table_bound = table_entry->get_bound();
        table_score = table_entry->get_score(data.ply);
        table_depth = table_entry->get_depth();

        // Cut off
        if (table_depth >= depth && !is_pv) {
            if ((table_bound == transposition::bound::EXACT) ||
                (table_bound == transposition::bound::LOWER && table_score >= beta) ||
                (table_bound == transposition::bound::UPPER && table_score <= alpha)) {
                return table_score;
            }
        }
    }

    // Gets important values for search, prunings, extensions
    // - In check
    bool is_in_check = data.board.is_in_check(data.board.get_color());

    // - Gets static eval
    i32 eval = eval::score::NONE;
    i32 eval_raw = eval::score::NONE;

    if (is_in_check) {
        // Don't do anything if we are in check
    }
    else if (table_hit) {
        // Gets the eval value from the table if possible, else gets the board's static eval
        eval_raw = table_eval != eval::score::NONE ? table_eval : eval::get(data.board);
        eval = eval_raw;
        
        // Uses the node's score as a more accurate eval value
        if ((table_bound == transposition::bound::EXACT) ||
            (table_bound == transposition::bound::LOWER && table_score >= beta) ||
            (table_bound == transposition::bound::UPPER && table_score <= alpha)) {
            eval = table_score;
        }
    }
    else {
        // Gets the board's static eval
        eval_raw = eval::get(data.board);
        eval = eval_raw;

        // Stores this eval into the table
        table_entry->set(
            data.board.get_hash(),
            move::NONE,
            eval::score::NONE,
            eval_raw,
            depth,
            transposition::bound::NONE,
            data.ply,
            this->table.age
        );
    }

    data.evals[data.ply] = eval;

    // - Improving
    bool improving = true;

    if (is_root || is_in_check) {
        improving = false;
    }
    else if (data.ply >= 2 && data.evals[data.ply - 2] != eval::score::NONE) {
        improving = data.evals[data.ply] > data.evals[data.ply - 2];
    }
    else if (data.ply >= 4 && data.evals[data.ply - 4] != eval::score::NONE) {
        improving = data.evals[data.ply] > data.evals[data.ply - 4];
    }

    // - Skips quiet
    bool skip_quiet = false;

    // Reverse futility pruning
    // - If our eval is so good we can take a big hit and still get the beta cutoff, then prune
    if (!is_pv &&
        !is_in_check &&
        depth <= constants::rfp::DEPTH &&
        eval != eval::score::NONE &&
        eval >= beta + depth * constants::rfp::MARGIN) {
        return eval;
    }

    // Null move pruning
    // - If our position is too good that giving the enemy a free move still fail high, then prune
    if (!is_pv &&
        !is_in_check &&
        data.moves[data.ply - 1] != move::NONE &&
        eval >= beta &&
        data.board.has_non_pawn(data.board.get_color())) {
        // Calculates reduction count based on depth and eval
        i32 reduction =
            constants::nmp::REDUCTION +
            depth / constants::nmp::DIVISOR_DEPTH +
            std::min((eval - beta) / constants::nmp::DIVISOR_EVAL, constants::nmp::REDUCTION_EVAL_MAX);
        
        // Makes null move
        data.board.make_null();
        data.moves[data.ply] = move::NONE;
        data.ply += 1;

        // Scouts
        i32 score = -this->pvsearch<node::NORMAL>(data, -beta, -beta + 1, depth - reduction);

        // Unmakes
        data.board.unmake_null();
        data.ply -= 1;

        // Returns score if fail high, we don't return false mate score
        if (score >= beta) {
            return score < eval::score::MATE_FOUND ? score : beta;
        }
    }

    // Check extension
    // - Increase the depth to search if we are in check
    // - We don't have to do this before qsearch because our qsearch has check evasion
    i32 extension = is_in_check;

    // Best score
    i32 best = -eval::score::INFINITE;
    u16 best_move = move::NONE;

    i32 alpha_old = alpha;

    // Generate moves and scores them
    auto moves = move::generate::get_legal<move::generate::type::ALL>(data.board);
    auto moves_scores = move::order::get_score(moves, data, table_move);

    // Iterates moves
    for (usize i = 0; i < moves.size(); ++i) {
        // Picks the move to search based on move ordering
        move::order::sort(moves, moves_scores, i);

        // Checks if move is quiet
        bool is_quiet = data.board.is_move_quiet(moves[i]);

        // Skips quiet moves
        if (!is_pv && !is_in_check && skip_quiet && is_quiet) {
            continue;
        }

        // Late move pruning
        // - If we have seen many moves in this position already, and we are not in check, we skip the rest
        if (!is_pv &&
            !skip_quiet &&
            best > -eval::score::MATE_FOUND &&
            i + 1 >= (depth * depth + constants::lmp::BASE) / (2 - improving)) {
            skip_quiet = true;
        }

        // Makes
        data.board.make(moves[i]);
        data.moves[data.ply] = moves[i];
        data.ply += 1;

        // Prefetch table
        this->table.prefetch(data.board.get_hash());

        // Principle variation search
        i32 score;

        // Scouts with null window for non pv nodes
        if (!is_pv || i > 0) {
            score = -this->pvsearch<node::NORMAL>(data, -alpha - 1, -alpha, depth - 1 + extension);
        }

        // Searches as pv node for first child or researches after scouting
        if (is_pv && (i == 0 || score > alpha)) {
            score = -this->pvsearch<node::PV>(data, -beta, -alpha, depth - 1 + extension);
        }

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
                data.pv_table[data.ply].update(moves[i], data.pv_table[data.ply + 1]);
            }
        }

        // Fail-soft cutoff
        // - This move is too good, our enemy won't let us do this
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
        if (is_in_check) {
            return -eval::score::MATE + data.ply;
        }
        else {
            return i32(data.nodes & 0b10) - 1;
        }
    }

    // Updates transposition table
    // - Store results of search into the table
    u8 bound =
        best >= beta ? transposition::bound::LOWER :
        best > alpha_old ? transposition::bound::EXACT :
        transposition::bound::UPPER;
    
    table_entry->set(
        data.board.get_hash(),
        best_move,
        best,
        eval_raw,
        depth,
        bound,
        data.ply,
        this->table.age
    );

    return best;
};

// Quiescence search
i32 Engine::qsearch(Data& data, i32 alpha, i32 beta)
{
    // Aborts search
    // - Every now and then check if we have exceeded the hard time limit
    // - Stops searching
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
    data.pv_table[data.ply].count = 0;
    data.nodes += 1;
    data.seldepth = std::max(data.seldepth, data.ply);

    // In check
    bool is_in_check = data.board.is_in_check(data.board.get_color());
    
    // Max ply reached
    if (data.ply >= MAX_PLY) {
        return is_in_check ? 0 : eval::get(data.board);
    }

    // Probes table
    auto [table_hit, table_entry] = this->table.get(data.board.get_hash());

    u16 table_move = move::NONE;
    i32 table_eval = eval::score::NONE;
    u8 table_bound = transposition::bound::NONE;
    i32 table_score = eval::score::NONE;
    i32 table_depth = 0;

    if (table_hit) {
        table_move = table_entry->get_move();
        table_eval = table_entry->get_eval();
        table_bound = table_entry->get_bound();
        table_score = table_entry->get_score(data.ply);
        table_depth = table_entry->get_depth();

        // Returns when score is exact or produces a cutoff in pv nodes
        if ((table_bound == transposition::bound::EXACT) ||
            (table_bound == transposition::bound::LOWER && table_score >= beta) ||
            (table_bound == transposition::bound::UPPER && table_score <= alpha)) {
            return table_score;
        }
    }

    // Gets static eval
    i32 eval = eval::score::NONE;
    i32 eval_raw = eval::score::NONE;

    if (is_in_check) {
        // Don't do anything if we are in check
    }
    else if (table_hit) {
        // Gets the eval value from the table if possible, else gets the board's static eval
        eval_raw = table_eval != eval::score::NONE ? table_eval : eval::get(data.board);
        eval = eval_raw;
        
        // Uses the node's score as a more accurate eval value
        if ((table_bound == transposition::bound::EXACT) ||
            (table_bound == transposition::bound::LOWER && table_score >= beta) ||
            (table_bound == transposition::bound::UPPER && table_score <= alpha)) {
            eval = table_score;
        }
    }
    else {
        // Gets the board's static eval
        eval_raw = eval::get(data.board);
        eval = eval_raw;

        // Stores this eval into the table
        table_entry->set(
            data.board.get_hash(),
            move::NONE,
            eval::score::NONE,
            eval_raw,
            0,
            transposition::bound::NONE,
            data.ply,
            this->table.age
        );
    }

    // Best score
    i32 best = -eval::score::MATE + data.ply;
    u16 best_move = move::NONE;

    i32 alpha_old = alpha;

    if (!is_in_check) {
        best = eval;
    
        // Prunes
        if (eval >= beta) {
            return eval;
        }
    
        // Updates alpha
        if (alpha < eval) {
            alpha = eval;
        }
    }

    // If we're in check, generates all legal moves, else generates noisy moves
    auto moves = 
        is_in_check ?
        move::generate::get_legal<move::generate::type::ALL>(data.board) :
        move::generate::get_legal<move::generate::type::NOISY>(data.board);
    
    // Scores moves
    auto moves_scores = move::order::get_score(moves, data, table_move);

    // Iterates moves
    for (usize i = 0; i < moves.size(); ++i) {
        // Picks the move to search based on move ordering
        move::order::sort(moves, moves_scores, i);

        // Makes
        data.board.make(moves[i]);
        data.ply += 1;

        // Searches deeper
        i32 score = -this->qsearch(data, -beta, -alpha);

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
            best_move = moves[i];

            if (score > alpha) {
                alpha = score;

                // Updates pv line
                data.pv_table[data.ply].update(moves[i], data.pv_table[data.ply + 1]);
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
    
    table_entry->set(
        data.board.get_hash(),
        best_move,
        best,
        eval_raw,
        0,
        bound,
        data.ply,
        this->table.age
    );

    return best;
};

};