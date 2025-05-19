#include "order.h"
#include "search.h"

namespace move::order
{

arrayvec<i32, move::MAX> get_score(const arrayvec<u16, move::MAX>& moves, search::Data& data, u16 hash_move)
{
    auto scores = arrayvec<i32, move::MAX>();

    for (usize i = 0; i < moves.size(); ++i) {
        // Hash move
        if (moves[i] == hash_move) {
            scores.add(HASH_SCORE);
            continue;
        }

        // Noisy
        i8 piece = data.board.get_piece_at(move::get_square_from(moves[i]));

        if (!data.board.is_move_quiet(moves[i])) {
            // Gets captured piece type
            i8 captured = data.board.get_captured_type(moves[i]);

            // Gets score
            i32 score = eval::SEE_VALUE[captured] * 16 + data.history_noisy.get(data.board, moves[i], captured);

            // SEE
            if (eval::is_see(data.board, moves[i], 0)) {
                // Good noisy :)
                scores.add(score + NOISY_SCORE);
            }
            else {
                // Bad noisy :(
                scores.add(score - NOISY_SCORE);
            }

            continue;
        }

        // Killer
        if (data.killers[data.ply] == moves[i]) {
            scores.add(KILLER_SCORE);
            continue;
        }

        // History
        scores.add(data.history_quiet.get(data.board, moves[i]));
    }

    return scores;
};

void sort(arrayvec<u16, move::MAX>& moves, arrayvec<i32, move::MAX>& moves_scores, usize index)
{
    usize best = index;

    for (usize i = index + 1; i < moves.size(); ++i) {
        if (moves_scores[i] > moves_scores[best]) {
            best = i;
        }
    }

    std::swap(moves[best], moves[index]);
    std::swap(moves_scores[best], moves_scores[index]);
};

};