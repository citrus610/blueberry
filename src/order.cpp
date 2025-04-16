#include "order.h"
#include "search.h"

namespace move::order
{

arrayvec<i32, move::MAX_MOVE> get_score(const arrayvec<u16, move::MAX_MOVE>& moves, search::Data& data)
{
    auto scores = arrayvec<i32, move::MAX_MOVE>();

    for (usize i = 0; i < moves.size(); ++i) {
        // PV
        if (moves[i] == data.pv_table[data.ply].data[0]) {
            scores.add(move::order::PV_SCORE);
            continue;
        }

        // MVV LVA
        i8 piece = data.board.get_piece_at(move::get_square_from(moves[i]));
        i8 captured = data.board.get_piece_type_at(move::get_square_to(moves[i]));

        if (move::get_type(moves[i]) != move::type::CASTLING && captured != piece::NONE) {
            scores.add(move::order::MVV_LVA[captured][piece::get_type(piece)] + move::order::MVV_LVA_SCORE);
            continue;
        }

        // Killer
        if (data.killer_table[data.ply] == moves[i]) {
            scores.add(move::order::KILLER_SCORE);
            continue;
        }

        // History
        scores.add(data.history_table[piece][move::get_square_to(moves[i])]);
    }

    return scores;
};

void sort(arrayvec<u16, move::MAX_MOVE>& moves, arrayvec<i32, move::MAX_MOVE>& moves_scores, usize index)
{
    usize best = 0;

    for (usize i = index; i < moves.size(); ++i) {
        if (moves_scores[i] > moves_scores[best]) {
            best = i;
        }
    }

    std::swap(moves[best], moves[index]);
    std::swap(moves_scores[best], moves_scores[index]);
};

};