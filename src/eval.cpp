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
    score += eval::get_pawn_structure(board);

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

i32 get_pawn_structure(Board& board)
{
    i32 pawn_structure = 0;

    const u64 pawns[2] = {
        board.get_pieces(piece::type::PAWN) & board.get_colors(color::WHITE),
        board.get_pieces(piece::type::PAWN) & board.get_colors(color::BLACK)
    };

    i32 stacked[2] = { 0, 0 };
    i32 isolated[2] = { 0, 0 };
    i32 passed[2] = { 0, 0 };

    // Files nearby masks
    constexpr std::array<u64, 8> FILES_NEARBY = [] {
        std::array<u64, 8> result = { 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL };
    
        for (i8 file = 0; file < 8; ++file) {
            if (file > 0) {
                result[file] |= bitboard::FILE[file - 1];
            }
    
            if (file < 7) {
                result[file] |= bitboard::FILE[file + 1];
            }
        }
    
        return result;
    } ();

    // Pawns forward pass masks
    constexpr std::array<std::array<u64, 64>, 2> FORWARD_PASS = [] {
        std::array<std::array<u64, 64>, 2> result = { 0ULL };

        for (i8 color = 0; color < 2; ++color) {
            for (i8 sq = 0; sq < 64; ++sq) {
                u64 mask = bitboard::create(sq);

                if (color == color::WHITE) {
                    mask |= attack::get_pawn_left<color::WHITE>(bitboard::create(sq));
                    mask |= attack::get_pawn_right<color::WHITE>(bitboard::create(sq));

                    mask |= mask << 8;
                    mask |= mask << 16;
                    mask |= mask << 32;
                }
                else {
                    mask |= attack::get_pawn_left<color::BLACK>(bitboard::create(sq));
                    mask |= attack::get_pawn_right<color::BLACK>(bitboard::create(sq));

                    mask |= mask >> 8;
                    mask |= mask >> 16;
                    mask |= mask >> 32;
                }

                result[color][sq] = mask;
            }
        }
    
        return result;
    } ();

    // Pawns stacked and isolated
    for (i8 color = 0; color < 2; ++color) {
        for (i8 file = 0; file < 8; ++file) {
            const u64 pawns_file = pawns[color] & bitboard::FILE[file];

            if (pawns_file) {
                const i32 count = bitboard::get_count(pawns_file);

                stacked[color] += (count - 1) * eval::DEFAULT.pawn_stacked[file];
                isolated[color] += (pawns[color] & FILES_NEARBY[file]) ? 0 : count * eval::DEFAULT.pawn_isolated[file];
            }
        }
    }

    // Pawns passed
    for (i8 color = 0; color < 2; ++color) {
        u64 pawns_color = pawns[color];

        while (pawns_color)
        {
            i8 sq = bitboard::get_lsb(pawns_color);
            pawns_color = bitboard::get_pop_lsb(pawns_color);

            if ((FORWARD_PASS[color][sq] & pawns[!color]) == 0) {
                passed[color] += eval::DEFAULT.pawn_passed[square::get_file(sq)];
            }
        }
    }

    pawn_structure += stacked[0] - stacked[1];
    pawn_structure += isolated[0] - isolated[1];
    pawn_structure += passed[0] - passed[1];

    return pawn_structure;
};

};