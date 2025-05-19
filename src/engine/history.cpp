#include "history.h"

namespace history
{

namespace quiet
{

Table::Table()
{
    std::memset(this->data, 0, sizeof(this->data));
};

i16& Table::get(Board& board, const u16& move)
{
    const i8 to = move::get_square_to(move);
    const i8 piece = board.get_piece_at(move::get_square_from(move));
    
    assert(piece != piece::NONE);

    return this->data[piece][to];
};

void Table::update(Board& board, const u16& move, i16 bonus)
{
    history::update(this->get(board, move), bonus);
};

};

namespace noisy
{

Table::Table()
{
    std::memset(this->data, 0, sizeof(this->data));
};

i16& Table::get(Board& board, const u16& move)
{
    return this->get(board, move, board.get_captured_type(move));
};

i16& Table::get(Board& board, const u16& move, i8 captured)
{
    const i8 to = move::get_square_to(move);
    const i8 piece = board.get_piece_at(move::get_square_from(move));
    
    assert(piece != piece::NONE);

    return this->data[piece][to][captured];
};

void Table::update(Board& board, const u16& move, i16 bonus)
{
    history::update(this->get(board, move), bonus);
};

};

namespace cont
{

Entry::Entry()
{
    this->piece = piece::NONE;
    this->to = square::NONE;
};

Entry::Entry(Board& board, const u16& move)
{
    this->to = move::get_square_to(move);
    this->piece = board.get_piece_at(move::get_square_from(move));
};

bool Entry::is_valid() const
{
    return this->piece != piece::NONE && this->to != square::NONE;
};

Table::Table()
{
    std::memset(this->data, 0, sizeof(this->data));
};

i16& Table::get(const Entry& entry, Board& board, const u16& move)
{
    const i8 to = move::get_square_to(move);
    const i8 piece = board.get_piece_at(move::get_square_from(move));
    
    assert(piece != piece::NONE);

    return this->data[entry.piece][entry.to][piece][to];
};

void Table::update(const Entry& entry, Board& board, const u16& move, i16 bonus)
{
    history::update(this->get(entry, board, move), bonus);
};

};