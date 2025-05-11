#include "history.h"

namespace history
{

Table::Table()
{
    for (i32 p = 0; p < 12; ++p) {
        for (i32 sq = 0; sq < 64; ++sq) {
            this->data[p][sq] = 0;
        }
    }
};

i32 Table::get(Board& board, u16 move)
{
    const i8 from = move::get_square_from(move);
    const i8 to = move::get_square_to(move);

    assert(square::is_valid(from));
    assert(square::is_valid(to));
    assert(board.get_piece_at(from) != piece::NONE);

    return this->data[board.get_piece_at(from)][to];
};

void Table::update(Board& board, u16 move, i32 bonus)
{
    const i8 from = move::get_square_from(move);
    const i8 to = move::get_square_to(move);

    assert(square::is_valid(from));
    assert(square::is_valid(to));
    assert(board.get_piece_at(from) != piece::NONE);

    bonus = std::clamp(bonus, -MAX, MAX);

    auto& entry = this->data[board.get_piece_at(from)][to];
    entry += bonus - entry * std::abs(bonus) / MAX;
};


};