#pragma once

#include "attack.h"
#include "zobrist.h"

#include "../util/arrayvec.h"

constexpr i32 MAX_PLY = 256;

struct Undo
{
    u64 hash;
    i8 castling;
    i8 enpassant;
    i8 captured;
    i32 halfmove;
};

class Board
{
public:
    static constexpr auto STARTPOS = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
private:
    u64 pieces[6];
    u64 colors[2];
    i8 board[64];
    i8 color;
    i8 castling;
    i8 enpassant;
    i32 halfmove;
    i32 ply;
private:
    u64 hash;
    std::vector<Undo> history;
public:
    Board(const std::string& fen = STARTPOS);
public:
    void set_fen(const std::string& fen);
    void set_ply(i32 ply);
public:
    u64 get_occupied();
    u64 get_pieces(i8 piece_type);
    u64 get_pieces(i8 piece_type, i8 color);
    u64 get_colors(i8 color);
    i8 get_color();
    i8 get_piece_type_at(i8 square);
    i8 get_color_at(i8 square);
    i8 get_piece_at(i8 square);
    i8 get_king_square(i8 color);
    i8 get_castling_right();
    i8 get_enpassant_square();
    i32 get_halfmove_count();
    i32 get_fullmove_count();
    i32 get_ply();
    u64 get_square_attacker(i8 square, u64 occupied);
    u64 get_hash();
    std::string get_fen();
public:
    bool is_drawn();
    bool is_drawn_repitition();
    bool is_drawn_fifty_move();
    bool is_drawn_insufficient();
    bool is_square_attacked(i8 square, i8 color);
    bool is_in_check(i8 color);
    bool is_move_quiet(u16 move);
    bool has_non_pawn(i8 color);
public:
    void make(u16 move);
    void unmake(u16 move);
    void make_null();
    void unmake_null();
    void remove(i8 piece_type, i8 color, i8 square);
    void place(i8 piece_type, i8 color, i8 square);
    void print();
};