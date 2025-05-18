#include "history.h"

namespace history
{

Quiet::Quiet()
{
    for (i32 p = 0; p < 12; ++p) {
        for (i32 sq = 0; sq < 64; ++sq) {
            this->data[p][sq] = 0;
        }
    }
};

i32& Quiet::get(Board& board, const u16& move)
{
    const i8 from = move::get_square_from(move);
    const i8 to = move::get_square_to(move);
    const i8 piece = board.get_piece_at(from);

    assert(square::is_valid(from));
    assert(square::is_valid(to));
    assert(piece != piece::NONE);

    return this->data[piece][to];
};

void Quiet::update(Board& board, const u16& move, i32 bonus)
{
    history::update(this->get(board, move), bonus);
};

Noisy::Noisy()
{
    for (i32 p = 0; p < 12; ++p) {
        for (i32 sq = 0; sq < 64; ++sq) {
            for (i32 c = 0; c < 6; ++c) {
                this->data[p][sq][c] = 0;
            }
        }
    }
};

i32& Noisy::get(Board& board, const u16& move)
{
    return this->get(board, move, board.get_captured_type(move));
};

i32& Noisy::get(Board& board, const u16& move, i8 captured)
{
    const i8 from = move::get_square_from(move);
    const i8 to = move::get_square_to(move);
    const i8 piece = board.get_piece_at(from);

    assert(square::is_valid(from));
    assert(square::is_valid(to));
    assert(piece != piece::NONE);

    return this->data[piece][to][captured];
};

void Noisy::update(Board& board, const u16& move, i32 bonus)
{
    history::update(this->get(board, move), bonus);
};

};