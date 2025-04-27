#include "eval.h"

namespace eval
{

i32 get(Board& board)
{
    i32 score = 0;

    // Evaluates
    score += eval::get_material(board);
    score += eval::get_table(board);
    score += eval::get_mobility(board);

    // Returns score based on side to move with tempo
    return (board.get_color() == color::WHITE ? score : -score) + eval::DEFAULT.tempo;
};

i32 get_material(Board& board)
{
    const u64 white = board.get_colors(color::WHITE);
    const u64 black = board.get_colors(color::BLACK);

    const u64 pawn = board.get_pieces(piece::type::PAWN);
    const u64 knight = board.get_pieces(piece::type::KNIGHT);
    const u64 bishop = board.get_pieces(piece::type::BISHOP);
    const u64 rook = board.get_pieces(piece::type::ROOK);
    const u64 queen = board.get_pieces(piece::type::QUEEN);
    const u64 king = board.get_pieces(piece::type::KING);

    i32 dt_pawn = bitboard::get_count(pawn & white) - bitboard::get_count(pawn & black);
    i32 dt_knight = bitboard::get_count(knight & white) - bitboard::get_count(knight & black);
    i32 dt_bishop = bitboard::get_count(bishop & white) - bitboard::get_count(bishop & black);
    i32 dt_rook = bitboard::get_count(rook & white) - bitboard::get_count(rook & black);
    i32 dt_queen = bitboard::get_count(queen & white) - bitboard::get_count(queen & black);
    i32 dt_king = bitboard::get_count(king & white) - bitboard::get_count(king & black);

    i32 material = 0;

    material += dt_pawn * eval::DEFAULT.material_pawn;
    material += dt_knight * eval::DEFAULT.material_knight;
    material += dt_bishop * eval::DEFAULT.material_bishop;
    material += dt_rook * eval::DEFAULT.material_rook;
    material += dt_queen * eval::DEFAULT.material_queen;
    material += dt_king * eval::DEFAULT.material_king;

    return material;
};

i32 get_table(Board& board)
{
    i32 table[2] = { 0, 0 };

    for (i8 square = 0; square < 64; ++square) {
        i8 piece = board.get_piece_at(square);

        if (piece == piece::NONE) {
            continue;
        }

        i8 piece_type = piece::get_type(piece);
        i8 piece_color = piece::get_color(piece);

        i8 index = piece_color == color::WHITE ? square : square::get_relative(square, color::BLACK);

        table[piece_color] += eval::DEFAULT.table[piece_type][index];
    }

    return table[0] - table[1];
};

i32 get_mobility(Board& board)
{
    const u64 colors[2] = {
        board.get_colors(color::WHITE),
        board.get_colors(color::BLACK)
    };

    const u64 occupied = colors[0] | colors[1];

    i32 mobility[2] = { 0, 0 };

    for (i8 square = 0; square < 64; ++square) {
        i8 piece = board.get_piece_at(square);

        if (piece == piece::NONE) {
            continue;
        }

        i8 piece_type = piece::get_type(piece);
        i8 piece_color = piece::get_color(piece);

        if (piece_type == piece::type::KNIGHT) {
            u64 attack = attack::get_knight(square);
            
            mobility[piece_color] += eval::DEFAULT.mobility_knight[bitboard::get_count(attack)];
        }
        else if (piece_type == piece::type::BISHOP) {
            u64 attack = attack::get_bishop(square, occupied) & ~colors[piece_color];

            mobility[piece_color] += eval::DEFAULT.mobility_bishop[bitboard::get_count(attack)];
        }
        else if (piece_type == piece::type::ROOK) {
            u64 attack = attack::get_rook(square, occupied) & ~colors[piece_color];

            mobility[piece_color] += eval::DEFAULT.mobility_rook[bitboard::get_count(attack)];
        }
        else if (piece_type == piece::type::QUEEN) {
            u64 attack = 0;
            attack |= attack::get_bishop(square, occupied) & ~colors[piece_color];
            attack |= attack::get_rook(square, occupied) & ~colors[piece_color];

            mobility[piece_color] += eval::DEFAULT.mobility_queen[bitboard::get_count(attack)];
        }
    }

    return mobility[0] - mobility[1];
};

};