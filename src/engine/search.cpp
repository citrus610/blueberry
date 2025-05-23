#include "search.h"
#include "uci.h"

namespace search::params
{

namespace lmr
{
    i32 TABLE[MAX_PLY][move::MAX];
};

};

namespace search
{

void init()
{
    // Late move reduction table
    for (i32 i = 0; i < MAX_PLY; ++i) {
        for (i32 k = 0; k < move::MAX; ++k) {
            if (i == 0 || k == 0) {
                params::lmr::TABLE[i][k] = 0;
                continue;
            }

            params::lmr::TABLE[i][k] = i32(std::log(i) * std::log(k) * 0.35 + 0.8);
        }
    }
};

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

Data::Data(Board board)
{
    this->board = board;

    this->history_quiet = history::quiet::Table();
    this->history_noisy = history::noisy::Table();
    this->history_cont = history::cont::Table();

    this->clear();
};

// Clears for search
void Data::clear()
{
    this->ply = 0;

    // Clears stack
    for (i32 i = 0; i < MAX_PLY; ++i) {
        this->pvs[i].clear();
        this->killers[i] = move::NONE;
        this->moves[i] = move::NONE;
        this->evals[i] = eval::score::NONE;
        this->cont_entries[i] = history::cont::Entry();
    }

    // Clears stats
    this->nodes = 0;
    this->seldepth = 0;
};

void Data::make(const u16& move)
{
    // Stores move
    this->moves[this->ply] = move;

    // Gets continuation history entry
    this->cont_entries[this->ply] = history::cont::Entry(this->board, move);

    // Makes move
    this->board.make(move);

    // Updates ply
    this->ply += 1;
};

void Data::unmake(const u16& move)
{
    this->board.unmake(move);
    this->ply -= 1;
};

void Data::make_null()
{
    // Stores null move
    this->moves[this->ply] = move::NONE;

    // Clears continuation history entry
    this->cont_entries[this->ply] = history::cont::Entry();

    // Makes null move
    this->board.make_null();

    // Updates ply
    this->ply += 1;
};

void Data::unmake_null()
{
    this->board.unmake_null();
    this->ply -= 1;
};

i16 Data::get_history_quiet(const u16& move)
{
    return
        this->history_quiet.get(this->board, move) +
        this->get_history_cont(move, 1);
};

i16 Data::get_history_noisy(const u16& move)
{
    return this->history_noisy.get(this->board, move);
};

i16 Data::get_history_cont(const u16& move, i32 offset)
{
    if (this->ply < offset) {
        return 0;
    }

    const auto entry = this->cont_entries[this->ply - offset];

    if (!entry.is_valid()) {
        return 0;
    }

    return this->history_cont.get(entry, this->board, move);
};

void Data::update_history_cont(const u16& move, i16 bonus, i32 offset)
{
    if (this->ply < offset) {
        return;
    }

    const auto entry = this->cont_entries[this->ply - offset];

    if (!entry.is_valid()) {
        return;
    }

    this->history_cont.update(entry, this->board, move, bonus);
};

Engine::Engine()
{
    this->clear();
};

void Engine::init()
{
    this->table.init(16);
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

    this->time_soft = time_start + timer::get_available_soft(uci_setting.time[uci_board.get_color()], uci_setting.inc[uci_board.get_color()], uci_setting.movestogo);
    this->time_hard = time_start + timer::get_available_hard(uci_setting.time[uci_board.get_color()]);

    if (uci_setting.infinite) {
        this->time_soft = UINT64_MAX;
        this->time_hard = UINT64_MAX;
    }

    // Starts search thread
    this->running.test_and_set();

    this->thread = new std::thread([&] (Board board, Settings settings) {
        // Inits search data
        auto data = Data(board);

        // Storing best pv lines found in each iteration
        std::vector<PV> pv_history = {};

        // Preivous search score
        i32 score_old = -eval::score::INFINITE;

        // Iterative deepening
        for (i32 i = 1; i < settings.depth; ++i) {
            // Clear search data
            data.clear();

            // Principle variation search
            auto time_1 = std::chrono::high_resolution_clock::now();

            i32 score = this->aspiration_window(data, i, score_old);

            auto time_2 = std::chrono::high_resolution_clock::now();

            // Avoids returning false score when stopped early
            if (timer::get_current() >= this->time_hard && !this->running.test()) {
                score = score_old;
            }
            
            // Updates score
            score_old = score;

            // Saves pv line
            if (data.pvs[0].count != 0 && data.pvs[0].data[0] != move::NONE) {
                pv_history.push_back(data.pvs[0]);
            }

            // Prints infos
            u64 dt = std::chrono::duration_cast<std::chrono::milliseconds>(time_2 - time_1).count();

            uci::print_info(i, data.seldepth, score, data.nodes, dt, this->table.hashfull(), pv_history.back());

            // Avoids searching too shallow
            if (i < 4) {
                continue;
            }

            // Checks time
            if (!settings.infinite && timer::get_current() >= this->time_soft) {
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

// Aspiration windows
i32 Engine::aspiration_window(Data& data, i32 depth, i32 score_old)
{
    // Values
    i32 score = -eval::score::INFINITE;

    i32 delta = params::aw::DELTA;

    i32 alpha = -eval::score::INFINITE;
    i32 beta = eval::score::INFINITE;

    // Sets the window if the depth is big enough
    if (depth >= params::aw::DEPTH) {
        alpha = std::max(score_old - delta, -eval::score::INFINITE);
        beta = std::min(score_old + delta, eval::score::INFINITE);
    }

    // Loops
    while (true)
    {
        // Principle variation search
        score = this->pvsearch<true>(data, alpha, beta, depth);

        // Aborts
        if (!this->running.test()) {
            break;
        }

        // Failed low
        if (score <= alpha) {
            beta = (alpha + beta) / 2;
            alpha = std::max(score - delta, -eval::score::INFINITE);
        }
        // Failed high
        else if (score >= beta) {
            beta = std::min(score + delta, eval::score::INFINITE);
        }
        else {
            break;
        }

        // Increase delta
        delta = delta + delta / 2;
    }

    return score;
};

// Principle variation search
template <bool PV>
i32 Engine::pvsearch(Data& data, i32 alpha, i32 beta, i32 depth)
{
    // Gets is root node
    const bool is_root = data.ply == 0;

    // Quiensence search
    if (depth <= 0) {
        return this->qsearch<PV>(data, alpha, beta);
    }

    // Aborts search
    if ((data.nodes & 0xFFF) == 0) {
        u64 time_now = timer::get_current();

        if (time_now >= this->time_hard) {
            this->running.clear();
        }
    }

    if (!this->running.test()) {
        return eval::score::DRAW;
    }
    
    // Updates data
    data.pvs[data.ply].count = 0;
    data.nodes += 1;
    data.seldepth = std::max(data.seldepth, data.ply);

    // Early stop conditions
    if (!is_root) {
        // Checks drawn
        if (data.board.is_drawn()) {
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
    auto [table_hit, table_entry] = this->table.get(data.board.get_hash());

    u16 table_move = move::NONE;
    i32 table_eval = eval::score::NONE;
    i32 table_score = eval::score::NONE;
    i32 table_depth = 0;
    u8 table_bound = transposition::bound::NONE;
    bool table_pv = PV;

    if (table_hit) {
        table_move = table_entry->get_move();
        table_eval = table_entry->get_eval();
        table_score = table_entry->get_score(data.ply);
        table_depth = table_entry->get_depth();
        table_bound = table_entry->get_bound();
        table_pv |= table_entry->is_pv();

        // Cut off
        if (!PV && table_score != eval::score::NONE && table_depth >= depth) {
            if ((table_bound == transposition::bound::EXACT) ||
                (table_bound == transposition::bound::LOWER && table_score >= beta) ||
                (table_bound == transposition::bound::UPPER && table_score <= alpha)) {
                return table_score;
            }
        }
    }

    // Gets important values for search, prunings, extensions
    // In check
    bool is_in_check = data.board.is_in_check(data.board.get_color());

    // Resets killer moves
    data.killers[data.ply + 1] = move::NONE;

    // Improving
    bool is_improving = false;

    // Gets static eval
    i32 eval = eval::score::NONE;
    i32 eval_static = eval::score::NONE;

    if (is_in_check) {
        // Don't do anything if we are in check
        data.evals[data.ply] = eval::score::NONE;

        // Skips pre-search prunings
        goto loop;
    }
    else if (table_hit) {
        // Gets the eval value from the table if possible, else gets the board's static eval
        eval_static = table_eval != eval::score::NONE ? table_eval : eval::get(data.board);
        eval = eval_static;
        data.evals[data.ply] = eval;
        
        // Uses the node's score as a more accurate eval value
        if ((table_bound == transposition::bound::EXACT) ||
            (table_bound == transposition::bound::LOWER && table_score >= beta) ||
            (table_bound == transposition::bound::UPPER && table_score <= alpha)) {
            eval = table_score;
        }
    }
    else {
        // Gets the board's static eval
        eval_static = eval::get(data.board);
        eval = eval_static;
        data.evals[data.ply] = eval;

        // Stores this eval into the table
        table_entry->set(
            data.board.get_hash(),
            move::NONE,
            eval::score::NONE,
            eval_static,
            depth,
            this->table.age,
            table_pv,
            transposition::bound::NONE,
            data.ply
        );
    }

    // Gets improving heuristic if we're not in check
    is_improving = data.ply >= 2 && data.evals[data.ply] > data.evals[data.ply - 2];

    // Reverse futility pruning
    if (!PV &&
        depth <= params::rfp::DEPTH &&
        eval != eval::score::NONE &&
        eval >= beta + depth * params::rfp::MARGIN) {
        return eval;
    }

    // Null move pruning
    if (!PV &&
        data.moves[data.ply - 1] != move::NONE &&
        eval >= beta &&
        depth >= params::nmp::DEPTH &&
        data.board.has_non_pawn(data.board.get_color())) {
        // Calculates reduction count based on depth and eval
        i32 reduction =
            params::nmp::REDUCTION +
            depth / params::nmp::DIVISOR_DEPTH +
            std::min((eval - beta) / params::nmp::DIVISOR_EVAL, params::nmp::REDUCTION_EVAL_MAX);
        
        // Makes null move
        data.make_null();

        // Scouts
        i32 score = -this->pvsearch<false>(data, -beta, -beta + 1, depth - reduction);

        // Unmakes
        data.unmake_null();

        // Returns score if fail high, we don't return false mate score
        if (score >= beta) {
            return score < eval::score::MATE_FOUND ? score : beta;
        }
    }

    // Internal iterative reduction
    if ((!table_hit || table_depth + 4 < depth) && depth >= 4) {
        depth -= 1;
    }

    // Moves loop
    loop:

    // Check extension
    i32 extension = is_in_check;

    // Best score
    i32 best = -eval::score::INFINITE;
    u16 best_move = move::NONE;

    i32 alpha_old = alpha;

    // Generate moves and scores them
    auto moves = move::generate::get_legal<move::generate::type::ALL>(data.board);
    auto moves_scores = move::order::get_score(moves, data, table_move);

    // Moves seen list
    auto quiets = arrayvec<u16, move::MAX>();
    auto noisies = arrayvec<u16, move::MAX>();

    i32 legals = 0;

    // Skip quiets flag
    bool skip_quiets = false;

    // Iterates moves
    for (usize i = 0; i < moves.size(); ++i) {
        // Picks the move to search based on move ordering
        move::order::sort(moves, moves_scores, i);

        // Checks if move is quiet
        bool is_quiet = data.board.is_move_quiet(moves[i]);

        // Skip quiets
        if (skip_quiets && is_quiet) {
            continue;
        }

        // Updates moves count
        legals += 1;

        // Pruning
        if (!is_root && best > -eval::score::MATE_FOUND) {
            // Late move pruning
            if (!skip_quiets &&
                legals >= (params::lmp::BASE + depth * depth) / (2 - is_improving)) {
                skip_quiets = true;
            }

            // Futility pruning
            i32 lmr_reduction = params::lmr::TABLE[depth][legals];
            i32 lmr_depth = std::max(0, depth - lmr_reduction);

            if (!skip_quiets &&
                !is_in_check &&
                is_quiet &&
                lmr_depth <= params::fp::DEPTH &&
                eval + params::fp::BASE + lmr_depth * params::fp::COEF <= alpha) {
                skip_quiets = true;
            }

            // History pruning
            if (is_quiet && depth <= params::hp::DEPTH) {
                i32 history_score = data.get_history_quiet(moves[i]);

                if (history_score < params::hp::MARGIN * depth) {
                    skip_quiets = true;
                    continue;
                }
            }

            // SEE pruning
            i32 see_margin =
                is_quiet ?
                params::see::MARGIN_QUIET * lmr_depth :
                params::see::MARGIN_NOISY * lmr_depth * lmr_depth;
            
            if (moves_scores[i] < move::order::KILLER_SCORE && !eval::is_see(data.board, moves[i], see_margin)) {
                continue;
            }
        }

        // Makes
        data.make(moves[i]);

        // Principle variation search
        i32 score = -eval::score::INFINITE;
        i32 depth_next = depth - 1 + extension;

        // Late moves reduction
        if (legals > 1 + is_root * 2 &&
            depth >= params::lmr::DEPTH &&
            is_quiet) {
            // Gets reduction count
            i32 reduction = params::lmr::TABLE[depth][legals];

            // Clamps depth to avoid qsearch
            i32 depth_reduced = std::clamp(depth_next - reduction, 1, depth_next + 1);

            // Scouts
            score = -this->pvsearch<false>(data, -alpha - 1, -alpha, depth_reduced);

            // Failed high
            if (score > alpha && depth_reduced < depth_next) {
                score = -this->pvsearch<false>(data, -alpha - 1, -alpha, depth_next);
            }
        }
        // Scouts with null window for non pv nodes
        else if (!PV || i > 0) {
            score = -this->pvsearch<false>(data, -alpha - 1, -alpha, depth_next);
        }

        // Searches as pv node for first child or researches after scouting
        if (PV && (i == 0 || score > alpha)) {
            score = -this->pvsearch<true>(data, -beta, -alpha, depth_next);
        }

        // Unmakes
        data.unmake(moves[i]);

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
                data.pvs[data.ply].update(moves[i], data.pvs[data.ply + 1]);
            }
        }

        // Fail-soft cutoff
        if (score >= beta) {
            // History bonus
            const i16 bonus = history::get_bonus(depth);

            if (is_quiet) {
                // Stores killer moves
                data.killers[data.ply] = moves[i];

                // Updates history table
                data.history_quiet.update(data.board, moves[i], bonus);
                data.update_history_cont(moves[i], bonus, 1);

                for (usize k = 0; k < quiets.size(); ++k) {
                    data.history_quiet.update(data.board, quiets[k], -bonus);
                    data.update_history_cont(quiets[k], -bonus, 1);
                }
            }
            else {
                // Updates noisy history
                data.history_noisy.update(data.board, moves[i], bonus);
            }

            // Even if the best move wasn't noisy, we still decrease the other noisy moves' history scores
            for (usize k = 0; k < noisies.size(); ++k) {
                data.history_noisy.update(data.board, noisies[k], -bonus);
            }

            break;
        }

        // Adds moves seen to list
        if (is_quiet) {
            quiets.add(moves[i]);
        }
        else {
            noisies.add(moves[i]);
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
    u8 bound =
        best >= beta ? transposition::bound::LOWER :
        best > alpha_old ? transposition::bound::EXACT :
        transposition::bound::UPPER;
    
    table_entry->set(
        data.board.get_hash(),
        best_move,
        best,
        eval_static,
        depth,
        this->table.age,
        table_pv,
        bound,
        data.ply
    );

    return best;
};

// Quiescence search
template <bool PV>
i32 Engine::qsearch(Data& data, i32 alpha, i32 beta)
{
    // Aborts search
    if ((data.nodes & 0xFFF) == 0) {
        u64 time_now = timer::get_current();

        if (time_now >= this->time_hard) {
            this->running.clear();
        }
    }

    if (!this->running.test()) {
        return eval::score::DRAW;
    }
    
    // Updates data
    data.pvs[data.ply].count = 0;
    data.nodes += 1;
    data.seldepth = std::max(data.seldepth, data.ply);

    // Checks drawn
    if (data.board.is_drawn()) {
        return i32(data.nodes & 0b10) - 1;
    }

    // In check
    bool is_in_check = data.board.is_in_check(data.board.get_color());
    
    // Max ply reached
    if (data.ply >= MAX_PLY) {
        return is_in_check ? 0 : eval::get(data.board);
    }

    // Probes transposition table
    auto [table_hit, table_entry] = this->table.get(data.board.get_hash());

    u16 table_move = move::NONE;
    i32 table_eval = eval::score::NONE;
    i32 table_score = eval::score::NONE;
    i32 table_depth = 0;
    u8 table_bound = transposition::bound::NONE;
    bool table_pv = PV;

    if (table_hit) {
        table_move = table_entry->get_move();
        table_eval = table_entry->get_eval();
        table_score = table_entry->get_score(data.ply);
        table_depth = table_entry->get_depth();
        table_bound = table_entry->get_bound();
        table_pv |= table_entry->is_pv();

        // Cut off
        if (!PV && table_score != eval::score::NONE) {
            if ((table_bound == transposition::bound::EXACT) ||
                (table_bound == transposition::bound::LOWER && table_score >= beta) ||
                (table_bound == transposition::bound::UPPER && table_score <= alpha)) {
                return table_score;
            }
        }
    }

    // Gets static eval
    i32 eval = eval::score::NONE;
    i32 eval_static = eval::score::NONE;

    if (is_in_check) {
        // Don't do anything if we are in check
        data.evals[data.ply] = eval::score::NONE;
    }
    else if (table_hit) {
        // Gets the eval value from the table if possible, else gets the board's static eval
        eval_static = table_eval != eval::score::NONE ? table_eval : eval::get(data.board);
        eval = eval_static;
        data.evals[data.ply] = eval;
        
        // Uses the node's score as a more accurate eval value
        if (table_score != eval::score::NONE) {
            if ((table_bound == transposition::bound::EXACT) ||
                (table_bound == transposition::bound::LOWER && table_score >= beta) ||
                (table_bound == transposition::bound::UPPER && table_score <= alpha)) {
                eval = table_score;
            }
        }
    }
    else {
        // Gets the board's static eval
        eval_static = eval::get(data.board);
        eval = eval_static;
        data.evals[data.ply] = eval;

        // Stores this eval into the table
        table_entry->set(
            data.board.get_hash(),
            move::NONE,
            eval::score::NONE,
            eval_static,
            0,
            this->table.age,
            table_pv,
            transposition::bound::NONE,
            data.ply
        );
    }

    // Best score
    i32 best = -eval::score::INFINITE;
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

        // Skips quiets if we're in check and we've found non-mate score
        if (is_in_check && best > -eval::score::MATE_FOUND && data.board.is_move_quiet(moves[i])) {
            continue;
        }

        // Pruning
        if (!is_in_check && best > -eval::score::MATE_FOUND) {
            // Futility pruning
            i32 futility = data.evals[data.ply] + params::fp::QS_MARGIN;

            if (futility <= alpha && !eval::is_see(data.board, moves[i], 1)) {
                best = std::max(best, futility);
                continue;
            }

            // SEE pruning
            if (!eval::is_see(data.board, moves[i], params::see::QS_MARGIN)) {
                continue;
            }
        }

        // Makes
        data.make(moves[i]);

        // Searches deeper
        i32 score = -this->qsearch<PV>(data, -beta, -alpha);

        // Unmakes
        data.unmake(moves[i]);

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
                data.pvs[data.ply].update(moves[i], data.pvs[data.ply + 1]);
            }
        }

        // Cut off
        if (score >= beta) {
            break;
        }
    }

    // Returns mate score
    if (moves.size() == 0 && is_in_check) {
        return -eval::score::MATE + data.ply;
    }

    // Updates transposition table
    u8 bound =
        best >= beta ? transposition::bound::LOWER :
        best > alpha_old ? transposition::bound::EXACT :
        transposition::bound::UPPER;
    
    table_entry->set(
        data.board.get_hash(),
        best_move,
        best,
        eval_static,
        0,
        this->table.age,
        table_pv,
        bound,
        data.ply
    );

    return best;
};

};