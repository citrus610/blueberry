#include "history.h"

namespace history
{

Table::Table()
{
    this->clear();
};

void Table::clear()
{
    for (i32 p = 0; p < 12; ++p) {
        for (i32 sq = 0; sq < 64; ++sq) {
            this->data[p][sq] = 0;
        }
    }
};

i32 Table::get(Board& board, u16 move)
{
    return this->data[board.get_piece_at(move::get_square_from(move))][move::get_square_to(move)];
};

void Table::update(Board& board, u16 move, i32 depth, bool cutoff)
{
    const i8 piece = board.get_piece_at(move::get_square_from(move));
    const i8 to = move::get_square_to(move);

    i32 bonus = depth * depth;
    i32 decay = bonus * this->data[piece][to] / MAX;
    
    this->data[piece][to] += cutoff ? bonus : -bonus;
    this->data[piece][to] -= decay;

    this->data[piece][to] = std::clamp(this->data[piece][to], -MAX, MAX);
};

};