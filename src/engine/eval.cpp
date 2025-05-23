#include "eval.h"

namespace eval
{

i32 get(Board& board)
{
    i32 score = 0;

    // Info
    const u64 pawn_white = board.get_pieces(piece::type::PAWN, color::WHITE);
    const u64 pawn_black = board.get_pieces(piece::type::PAWN, color::BLACK);

    const u64 semi_open_white = ~(bitboard::get_fill_up(pawn_white) | bitboard::get_fill_down(pawn_white));
    const u64 semi_open_black = ~(bitboard::get_fill_up(pawn_black) | bitboard::get_fill_down(pawn_black));

    // Evaluates
    score += eval::get_material(board);
    score += eval::get_table(board);
    score += eval::get_mobility(board);
    score += eval::get_king_defense(board);
    score += eval::get_bishop_pair(board);
    score += eval::get_rook_file(board, semi_open_white, semi_open_black);
    score += eval::get_king_file(board, semi_open_white, semi_open_black);
    score += eval::get_pawn_structure(board);

    // Gets midgame and engame values
    i32 midgame = score::get_midgame(score);
    i32 endgame = score::get_endgame(score);

    // Calculates phase
    i32 phase = PHASE_MAX;

    phase -= bitboard::get_count(board.get_pieces(piece::type::KNIGHT)) * PHASE_KNIGHT;
    phase -= bitboard::get_count(board.get_pieces(piece::type::BISHOP)) * PHASE_BISHOP;
    phase -= bitboard::get_count(board.get_pieces(piece::type::ROOK)) * PHASE_ROOK;
    phase -= bitboard::get_count(board.get_pieces(piece::type::QUEEN)) * PHASE_QUEEN;

    phase = (phase * PHASE_SCALE + (PHASE_MAX / 2)) / PHASE_MAX;

    // Gets endgame scale
    i32 scale = eval::get_scale(board, score);

    // Mixes midgame and endgame values
    score = (midgame * (PHASE_SCALE - phase) + endgame * phase * scale / SCALE_MAX) / PHASE_SCALE;

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

i32 get_king_defense(Board& board)
{
    i32 defense[2] = { 0, 0 };

    const u64 minor =
        board.get_pieces(piece::type::PAWN) |
        board.get_pieces(piece::type::KNIGHT) |
        board.get_pieces(piece::type::BISHOP);

    for (i8 color = color::WHITE; color < 2; ++color) {
        const i8 king_square = board.get_king_square(color);
        const u64 defender = minor & board.get_colors(color) & attack::get_king(king_square);
        const i32 count = bitboard::get_count(defender);

        defense[color] += eval::DEFAULT.king_defense[count];
    }

    return defense[0] - defense[1];
};

i32 get_bishop_pair(Board& board)
{
    i32 bishop_pair = 0;

    const u64 bishop_white = board.get_pieces(piece::type::BISHOP, color::WHITE);
    const u64 bishop_black = board.get_pieces(piece::type::BISHOP, color::BLACK);

    if (bitboard::is_many(bishop_white) && (bishop_white & bitboard::LIGHT) && (bishop_white & bitboard::DARK)) {
        bishop_pair += eval::DEFAULT.bishop_pair;
    }

    if (bitboard::is_many(bishop_black) && (bishop_black & bitboard::LIGHT) && (bishop_black & bitboard::DARK)) {
        bishop_pair -= eval::DEFAULT.bishop_pair;
    }

    return bishop_pair;
};

i32 get_rook_file(Board& board, u64 semi_open_white, u64 semi_open_black)
{
    i32 rook_file = 0;

    const u64 open = semi_open_white & semi_open_black;

    const u64 rook_white = board.get_pieces(piece::type::ROOK, color::WHITE);
    const u64 rook_black = board.get_pieces(piece::type::ROOK, color::BLACK);

    const u64 rook_open_white = rook_white & open;
    const u64 rook_semi_open_white = rook_white & semi_open_white;

    const u64 rook_open_black = rook_black & open;
    const u64 rook_semi_open_black = rook_black & semi_open_black;

    rook_file += bitboard::get_count(rook_open_white) * eval::DEFAULT.rook_open_file;
    rook_file += bitboard::get_count(rook_semi_open_white) * eval::DEFAULT.rook_semi_open_file;

    rook_file -= bitboard::get_count(rook_open_black) * eval::DEFAULT.rook_open_file;
    rook_file -= bitboard::get_count(rook_semi_open_black) * eval::DEFAULT.rook_semi_open_file;

    return rook_file;
};

i32 get_king_file(Board& board, u64 semi_open_white, u64 semi_open_black)
{
    i32 king_file = 0;

    const u64 open = semi_open_white & semi_open_black;

    const u64 king_white = board.get_pieces(piece::type::KING, color::WHITE);
    const u64 king_black = board.get_pieces(piece::type::KING, color::BLACK);

    const u64 king_open_white = king_white & open;
    const u64 king_semi_open_white = king_white & semi_open_white;

    const u64 king_open_black = king_black & open;
    const u64 king_semi_open_black = king_black & semi_open_black;

    king_file += bitboard::get_count(king_open_white) * eval::DEFAULT.king_open_file;
    king_file += bitboard::get_count(king_semi_open_white) * eval::DEFAULT.king_semi_open_file;

    king_file -= bitboard::get_count(king_open_black) * eval::DEFAULT.king_open_file;
    king_file -= bitboard::get_count(king_semi_open_black) * eval::DEFAULT.king_semi_open_file;

    return king_file;
};

i32 get_pawn_structure(Board& board)
{
    constexpr std::array<std::array<u64, 64>, 2> FORWARD_PASS = [] {
        std::array<std::array<u64, 64>, 2> result = { 0ULL };

        for (i8 color = 0; color < 2; ++color) {
            for (i8 sq = 0; sq < 64; ++sq) {
                u64 mask = bitboard::create(sq);

                if (color == color::WHITE) {
                    mask |= attack::get_pawn_left<color::WHITE>(bitboard::create(sq));
                    mask |= attack::get_pawn_right<color::WHITE>(bitboard::create(sq));
                    mask |= bitboard::get_fill_up(mask);
                }
                else {
                    mask |= attack::get_pawn_left<color::BLACK>(bitboard::create(sq));
                    mask |= attack::get_pawn_right<color::BLACK>(bitboard::create(sq));
                    mask |= bitboard::get_fill_down(mask);
                }

                result[color][sq] = mask;
            }
        }
    
        return result;
    } ();

    i32 pawn_passed[2] = { 0, 0 };

    for (i8 color = 0; color < 2; ++color) {
        u64 pawn_us = board.get_pieces(piece::type::PAWN, color);
        u64 pawn_them = board.get_pieces(piece::type::PAWN, !color);

        while (pawn_us)
        {
            i8 sq = bitboard::get_lsb(pawn_us);
            pawn_us = bitboard::get_pop_lsb(pawn_us);

            if ((FORWARD_PASS[color][sq] & pawn_them) == 0) {
                pawn_passed[color] += eval::DEFAULT.pawn_passed[rank::get_color_rank(square::get_rank(sq), color)];
            }
        }
    }

    return pawn_passed[0] - pawn_passed[1];
};

// Scales endgame eval based on the amount of pawn on the board
i32 get_scale(Board& board, i32 eval)
{
    const i8 strong = eval > 0 ? color::WHITE : color::BLACK;
    const u64 strong_pawn = board.get_pieces(piece::type::PAWN, strong);
    const i32 strong_pawn_count = bitboard::get_count(strong_pawn);
    const i32 x = 8 - strong_pawn_count;

    return SCALE_MAX - x * x;
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
    i8 piece_from = board.get_piece_type_at(from);
    i8 piece_to = board.get_piece_type_at(to);

    // If we still lose after making the move, then stop
    i32 value = (piece_to == piece::type::NONE ? 0 : SEE_VALUE[piece_to]) - threshold;

    if (value < 0) {
        return false;
    }

    // If we still win after losing the piece, then stop
    value -= SEE_VALUE[piece_from];

    if (value >= 0) {
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