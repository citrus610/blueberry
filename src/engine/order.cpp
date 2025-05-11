#include "order.h"
#include "search.h"

namespace move::order
{

arrayvec<i32, move::MAX> get_score(const arrayvec<u16, move::MAX>& moves, search::Data& data, u16 hash_move)
{
    constexpr i32 MVV_LVA[5][6] = {
        { 150, 140, 130, 120, 110, 100 },
        { 250, 240, 230, 220, 210, 200 },
        { 350, 340, 330, 320, 210, 300 },
        { 450, 440, 430, 420, 410, 400 },
        { 550, 540, 530, 520, 510, 500 }
    };

    constexpr i32 HASH_SCORE = 1000000;
    constexpr i32 MVV_LVA_SCORE = 100000;
    constexpr i32 KILLER_SCORE = 90000;

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
            i8 captured =
                move::get_type(moves[i]) == move::type::ENPASSANT ?
                piece::type::PAWN :
                data.board.get_piece_type_at(move::get_square_to(moves[i]));
            
            // If this move is a promotion, we treat the captured piece as a pawn
            if (captured == piece::NONE) {
                captured = piece::type::PAWN;
            }

            scores.add(MVV_LVA[captured][piece::get_type(piece)] + MVV_LVA_SCORE);
            continue;
        }

        // Killer
        if (data.killers[data.ply] == moves[i]) {
            scores.add(KILLER_SCORE);
            continue;
        }

        // History
        scores.add(data.history.get(data.board, moves[i]));
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