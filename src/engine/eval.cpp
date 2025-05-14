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

    // Gets midgame and engame values
    i32 midgame = score::get_midgame(score);
    i32 endgame = score::get_endgame(score);

    // Calculates phase
    const i32 PHASE_SCALE = 256;
    const i32 PHASE_KNIGHT = 1;
    const i32 PHASE_BISHOP = 1;
    const i32 PHASE_ROOK = 2;
    const i32 PHASE_QUEEN = 4;
    const i32 PHASE_MAX = PHASE_KNIGHT * 4 + PHASE_BISHOP * 4 + PHASE_ROOK * 4 + PHASE_QUEEN * 2;

    i32 phase = PHASE_MAX;

    phase -= bitboard::get_count(board.get_pieces(piece::type::KNIGHT)) * PHASE_KNIGHT;
    phase -= bitboard::get_count(board.get_pieces(piece::type::BISHOP)) * PHASE_BISHOP;
    phase -= bitboard::get_count(board.get_pieces(piece::type::ROOK)) * PHASE_ROOK;
    phase -= bitboard::get_count(board.get_pieces(piece::type::QUEEN)) * PHASE_QUEEN;

    phase = (phase * PHASE_SCALE + (PHASE_MAX / 2)) / PHASE_MAX;

    // Mixes midgame and endgame values
    score = ((midgame * (PHASE_SCALE - phase)) + (endgame * phase)) / PHASE_SCALE;

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

// Static exchange evaluation
// Copied from Weiss
bool is_see(Board& board, u16 move, i32 threshold)
{
    // If move is special, then we're good
    if (move::get_type(move) != move::type::NORMAL) {
        return true;
    }

    // Move data
    i8 from = move::get_square_from(move);
    i8 to = move::get_square_to(move);
    i8 piece_from = board.get_piece_at(from);
    i8 piece_to = board.get_piece_at(to);

    assert(piece_to != piece::NONE);

    // If we still lose after making the capture, then stop
    i32 value = SEE_VALUE[piece_to] - threshold;

    if (value < 0) {
        return false;
    }

    // If we still win after losing the piece, then stop
    value -= SEE_VALUE[piece_from];

    if (value > 0) {
        return true;
    }

    // Masks
    u64 occupied = board.get_occupied() ^ bitboard::create(from);
    u64 attacker = board.get_square_attacker(to, occupied);

    u64 bishop = board.get_pieces(piece::type::BISHOP) | board.get_pieces(piece::type::QUEEN);
    u64 rook = board.get_pieces(piece::type::ROOK) | board.get_pieces(piece::type::QUEEN);

    i8 color = !board.get_color();

    // Makes captures until one side runs out or loses
    while (true)
    {
        // Removes used piece from the attackers
        attacker &= occupied;

        // Checks if we run out of captures to make
        u64 attacker_us = attacker & board.get_colors(color);

        if (!attacker_us) {
            break;
        }

        // Picks next least valuable piece to capture with
        i8 pt;

        for (pt = piece::type::PAWN; pt < piece::type::KING; ++pt) {
            if (attacker_us & board.get_pieces(pt)) {
                break;
            }
        }

        // Flips side to move
        color = !color;
        value = -value - 1 - SEE_VALUE[pt];

        // Negamax
        if (value >= 0) {
            if (pt == piece::type::KING && (attacker & board.get_colors(color))) {
                color = !color;
            }

            break;
        }

        // Removes used piece from occupied
        occupied ^= bitboard::create(bitboard::get_lsb(attacker_us & board.get_pieces(pt)));

        // Adds possible discovered attacks
        if (pt == piece::type::PAWN || pt == piece::type::BISHOP || pt == piece::type::QUEEN) {
            attacker |= attack::get_bishop(to, occupied) & bishop;
        }

        if (pt == piece::type::ROOK || pt == piece::type::QUEEN) {
            attacker |= attack::get_rook(to, occupied) & rook;
        }
    }
    
    return color != board.get_color();
};

};