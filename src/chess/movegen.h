#pragma once

#include "board.h"
#include "mask.h"

#include "../util/arrayvec.h"

namespace move::generate
{

enum class type : i8
{
    ALL,
    NOISY,
    QUIET
};

// Gets the check mask for a color
// Gets the mask for all the enemy's attackers and their lines of attack
template <i8 COLOR>
inline std::pair<u64, i32> get_check_mask(Board &board, i8 square)
{
    const u64 occupied = board.get_occupied();

    const u64 enemy_pawn = board.get_pieces(piece::type::PAWN, !COLOR);
    const u64 enemy_knight = board.get_pieces(piece::type::KNIGHT, !COLOR);
    const u64 enemy_bishop = board.get_pieces(piece::type::BISHOP, !COLOR);
    const u64 enemy_rook = board.get_pieces(piece::type::ROOK, !COLOR);
    const u64 enemy_queen = board.get_pieces(piece::type::QUEEN, !COLOR);

    i32 checker = 0;

    // Knight
    u64 knight_attack = attack::get_knight(square) & enemy_knight;
    checker += knight_attack > 0;

    u64 mask = knight_attack;

    // Pawn
    u64 pawn_attack = attack::get_pawn(square, COLOR) & enemy_pawn;
    mask |= pawn_attack;
    checker += pawn_attack > 0;

    // Bishop
    u64 bishop_attack = attack::get_bishop(square, occupied) & (enemy_bishop | enemy_queen);

    if (bishop_attack) {
        const i8 bishop_square = bitboard::get_lsb(bishop_attack);

        mask |= mask::get_bits_between(square, bishop_square) | bitboard::create(bishop_square);
        checker += 1;
    }

    // Rook
    u64 rook_attack = attack::get_rook(square, occupied) & (enemy_rook | enemy_queen);

    if (rook_attack) {
        if (bitboard::is_many(rook_attack)) {
            return { mask, 2 };
        }

        const i8 rook_square = bitboard::get_lsb(rook_attack);

        mask |= mask::get_bits_between(square, rook_square) | bitboard::create(rook_square);
        checker += 1;
    }

    if (!mask) {
        return { ~0ULL, checker };
    }

    return { mask, checker };
};

// Gets the ray masks of the enemy rooks that pin our pieces
template <i8 COLOR>
inline u64 get_pin_mask_rook(Board &board, i8 square)
{
    const u64 occupied_us = board.get_colors(COLOR);
    const u64 occupied_them = board.get_colors(!COLOR);

    const u64 enemy_rook = board.get_pieces(piece::type::ROOK, !COLOR);
    const u64 enemy_queen = board.get_pieces(piece::type::QUEEN, !COLOR);

    u64 rook_attack = attack::get_rook(square, occupied_them) & (enemy_rook | enemy_queen);
    u64 mask = 0ULL;

    while (rook_attack)
    {
        const i8 rook_square = bitboard::get_lsb(rook_attack);
        rook_attack = bitboard::get_pop_lsb(rook_attack);

        u64 possible_pin = mask::get_bits_between(square, rook_square) | bitboard::create(rook_square);

        if (bitboard::get_count(possible_pin & occupied_us) == 1) {
            mask |= possible_pin;
        }
    }

    return mask;
};

// Gets the ray masks of the enemy bishops that pin our pieces
template <i8 COLOR>
inline u64 get_pin_mask_bishop(Board &board, i8 square)
{
    const u64 occupied_us = board.get_colors(COLOR);
    const u64 occupied_them = board.get_colors(!COLOR);

    const u64 enemy_bishop = board.get_pieces(piece::type::BISHOP, !COLOR);
    const u64 enemy_queen = board.get_pieces(piece::type::QUEEN, !COLOR);

    u64 bishop_attack = attack::get_bishop(square, occupied_them) & (enemy_bishop | enemy_queen);
    u64 mask = 0ULL;

    while (bishop_attack)
    {
        const i8 bishop_square = bitboard::get_lsb(bishop_attack);
        bishop_attack = bitboard::get_pop_lsb(bishop_attack);

        u64 possible_pin = mask::get_bits_between(square, bishop_square) | bitboard::create(bishop_square);

        if (bitboard::get_count(possible_pin & occupied_us) == 1) {
            mask |= possible_pin;
        }
    }

    return mask;
};

// Gets the mask of all seen squares
template <i8 COLOR>
inline u64 get_seen_mask(Board &board, u64 enemy_empty)
{
    i8 enemy_king_square = board.get_king_square(!COLOR);

    if ((attack::get_king(enemy_king_square) & enemy_empty) == 0ULL) {
        return 0ULL;
    }

    const u64 occupied = board.get_occupied() & ~bitboard::create(enemy_king_square);

    u64 pawn = board.get_pieces(piece::type::PAWN, COLOR);
    u64 knight = board.get_pieces(piece::type::KNIGHT, COLOR);
    u64 queen = board.get_pieces(piece::type::QUEEN, COLOR);
    u64 bishop = board.get_pieces(piece::type::BISHOP, COLOR) | queen;
    u64 rook = board.get_pieces(piece::type::ROOK, COLOR) | queen;

    // Pawn
    u64 mask = attack::get_pawn_left<COLOR>(pawn) | attack::get_pawn_right<COLOR>(pawn);

    // Knight
    while (knight)
    {
        const i8 square = bitboard::get_lsb(knight);
        knight = bitboard::get_pop_lsb(knight);

        mask |= attack::get_knight(square);
    }

    // Bishop
    while (bishop)
    {
        const i8 square = bitboard::get_lsb(bishop);
        bishop = bitboard::get_pop_lsb(bishop);

        mask |= attack::get_bishop(square, occupied);
    }

    // Rook
    while (rook)
    {
        const i8 square = bitboard::get_lsb(rook);
        rook = bitboard::get_pop_lsb(rook);

        mask |= attack::get_rook(square, occupied);
    }

    // King
    mask |= attack::get_king(board.get_king_square(COLOR));

    return mask;
};

// Gets the mask of all the castle-possible rooks
template <type TYPE, i8 COLOR>
inline u64 get_castle_rook_mask(Board& board, u64 seen_mask, u64 pin_mask_rook)
{
    if constexpr (TYPE == move::generate::type::NOISY) {
        return 0ULL;
    }

    const i8 king_from = board.get_king_square(COLOR);
    const u64 occupied = board.get_occupied();

    u64 rook_mask = 0ULL;

    i8 castling = board.get_castling_right();

    if constexpr (COLOR == color::WHITE) {
        castling &= castling::WHITE;
    }
    else {
        castling &= castling::BLACK;
    }

    // For all castling rights
    while (castling)
    {
        i8 castling_lsb = castling & (-castling);
        castling = bitboard::get_pop_lsb(castling);

        bool castle_short = castling_lsb & castling::SHORT;

        const i8 rook_from = castling::get_rook_corner(COLOR, castle_short);

        const i8 king_to = castling::get_king_square(COLOR, castle_short);
        const i8 rook_to = castling::get_rook_square(COLOR, castle_short);

        const u64 not_occupied_path = mask::get_bits_between(king_from, rook_from);
        const u64 not_attacked_path = mask::get_bits_between(king_from, king_to);
        const u64 empty_not_attacked = ~seen_mask & ~(occupied & ~bitboard::create(rook_from));
        const u64 without_rook = occupied & ~bitboard::create(rook_from);
        const u64 without_king = occupied & ~bitboard::create(king_from);

        if ((not_attacked_path & empty_not_attacked) == not_attacked_path &&
            ((not_occupied_path & ~occupied) == not_occupied_path) &&
            !(bitboard::create(rook_from) & pin_mask_rook & bitboard::RANK[square::get_rank(king_from)]) &&
            !(bitboard::create(rook_to) & without_rook & without_king) &&
            !(bitboard::create(king_to) & (seen_mask | (without_rook & ~bitboard::create(king_from))))) {
            rook_mask |= bitboard::create(rook_from);
        }
    }

    return rook_mask;
};

// Adds pawn moves to the move list
template <type TYPE, i8 COLOR>
inline void push_pawn(arrayvec<u16, MAX>& list, Board& board, u64 check_mask, u64 pin_mask_rook, u64 pin_mask_bishop)
{
    constexpr i8 UP = direction::get_color_direction(direction::NORTH, COLOR);
    constexpr i8 DOWN = direction::get_color_direction(direction::SOUTH, COLOR);
    constexpr i8 DOWN_LEFT = direction::get_color_direction(direction::SOUTH_WEST, COLOR);
    constexpr i8 DOWN_RIGHT = direction::get_color_direction(direction::SOUTH_EAST, COLOR);
    constexpr i8 UP_LEFT = direction::get_color_direction(direction::NORTH_WEST, COLOR);
    constexpr i8 UP_RIGHT = direction::get_color_direction(direction::NORTH_EAST, COLOR);

    constexpr u64 MASK_PROMOTION = bitboard::RANK[rank::get_color_rank(rank::RANK_8, COLOR)];
    constexpr u64 MASK_PRE_PROMOTION = bitboard::RANK[rank::get_color_rank(rank::RANK_7, COLOR)];
    constexpr u64 MASK_DOUBLE_PUSH = bitboard::RANK[rank::get_color_rank(rank::RANK_3, COLOR)];

    const u64 occupied = board.get_occupied();
    const u64 enemy_occupied = board.get_colors(!COLOR);

    const u64 pawn = board.get_pieces(piece::type::PAWN, COLOR);

    // These pawns can maybe take left or right
    const u64 pawn_lr = pawn & ~pin_mask_rook;
    const u64 pawn_lr_unpinned = pawn_lr & ~pin_mask_bishop;
    const u64 pawn_lr_pinned = pawn_lr & pin_mask_bishop;

    u64 pawn_l = bitboard::get_shift<UP_LEFT>(pawn_lr_unpinned) | (bitboard::get_shift<UP_LEFT>(pawn_lr_pinned) & pin_mask_bishop);
    u64 pawn_r = bitboard::get_shift<UP_RIGHT>(pawn_lr_unpinned) | (bitboard::get_shift<UP_RIGHT>(pawn_lr_pinned) & pin_mask_bishop);

    // Prune moves that don't capture a piece and are not on the checkmask.
    pawn_l &= enemy_occupied & check_mask;
    pawn_r &= enemy_occupied & check_mask;

    // These pawns can walk forward
    const u64 pawn_f = pawn & ~pin_mask_bishop;

    const u64 pawn_f_pinned = pawn_f & pin_mask_rook;
    const u64 pawn_f_unpinned = pawn_f & ~pin_mask_rook;

    // Prune moves that are blocked by a piece
    const u64 push_1_unpinned = bitboard::get_shift<UP>(pawn_f_unpinned) & ~occupied;
    const u64 push_1_pinned = bitboard::get_shift<UP>(pawn_f_pinned) & pin_mask_rook & ~occupied;

    // Prune moves that are not on the checkmask.
    u64 push_1 = (push_1_unpinned | push_1_pinned) & check_mask;
    u64 push_2 = ((bitboard::get_shift<UP>(push_1_unpinned & MASK_DOUBLE_PUSH) & ~occupied) | (bitboard::get_shift<UP>(push_1_pinned & MASK_DOUBLE_PUSH) & ~occupied)) & check_mask;

    // Pushes promotion
    if ((TYPE != move::generate::type::QUIET) && (pawn & MASK_PRE_PROMOTION)) {
        u64 promo_left = pawn_l & MASK_PROMOTION;
        u64 promo_right = pawn_r & MASK_PROMOTION;
        u64 promo_push = push_1 & MASK_PROMOTION;

        while (promo_left)
        {
            const i8 square = bitboard::get_lsb(promo_left);
            promo_left = bitboard::get_pop_lsb(promo_left);

            list.add(move::get_make<move::type::PROMOTION>(square + DOWN_RIGHT, square, piece::type::QUEEN));
            list.add(move::get_make<move::type::PROMOTION>(square + DOWN_RIGHT, square, piece::type::ROOK));
            list.add(move::get_make<move::type::PROMOTION>(square + DOWN_RIGHT, square, piece::type::BISHOP));
            list.add(move::get_make<move::type::PROMOTION>(square + DOWN_RIGHT, square, piece::type::KNIGHT));
        }

        while (promo_right)
        {
            const i8 square = bitboard::get_lsb(promo_right);
            promo_right = bitboard::get_pop_lsb(promo_right);

            list.add(move::get_make<move::type::PROMOTION>(square + DOWN_LEFT, square, piece::type::QUEEN));
            list.add(move::get_make<move::type::PROMOTION>(square + DOWN_LEFT, square, piece::type::ROOK));
            list.add(move::get_make<move::type::PROMOTION>(square + DOWN_LEFT, square, piece::type::BISHOP));
            list.add(move::get_make<move::type::PROMOTION>(square + DOWN_LEFT, square, piece::type::KNIGHT));
        }

        while (promo_push)
        {
            const i8 square = bitboard::get_lsb(promo_push);
            promo_push = bitboard::get_pop_lsb(promo_push);

            list.add(move::get_make<move::type::PROMOTION>(square + DOWN, square, piece::type::QUEEN));
            list.add(move::get_make<move::type::PROMOTION>(square + DOWN, square, piece::type::ROOK));
            list.add(move::get_make<move::type::PROMOTION>(square + DOWN, square, piece::type::BISHOP));
            list.add(move::get_make<move::type::PROMOTION>(square + DOWN, square, piece::type::KNIGHT));
        }
    }

    // Removes promotions
    push_1 &= ~MASK_PROMOTION;
    pawn_l &= ~MASK_PROMOTION;
    pawn_r &= ~MASK_PROMOTION;

    while (TYPE != move::generate::type::QUIET && pawn_l)
    {
        const i8 square = bitboard::get_lsb(pawn_l);
        pawn_l = bitboard::get_pop_lsb(pawn_l);

        list.add(move::get_make<move::type::NORMAL>(square + DOWN_RIGHT, square));
    }

    while (TYPE != move::generate::type::QUIET && pawn_r)
    {
        const i8 square = bitboard::get_lsb(pawn_r);
        pawn_r = bitboard::get_pop_lsb(pawn_r);

        list.add(move::get_make<move::type::NORMAL>(square + DOWN_LEFT, square));
    }

    while (TYPE != move::generate::type::NOISY && push_1)
    {
        const i8 square = bitboard::get_lsb(push_1);
        push_1 = bitboard::get_pop_lsb(push_1);

        list.add(move::get_make<move::type::NORMAL>(square + DOWN, square));
    }

    while (TYPE != move::generate::type::NOISY && push_2)
    {
        const i8 square = bitboard::get_lsb(push_2);
        push_2 = bitboard::get_pop_lsb(push_2);

        list.add(move::get_make<move::type::NORMAL>(square + DOWN + DOWN, square));
    }

    if constexpr (TYPE == move::generate::type::QUIET) {
        return;
    }

    // Enpassant move
    const i8 enpassant_square = board.get_enpassant_square();

    if (enpassant_square == square::NONE) {
        return;
    }

    const i8 king_square = board.get_king_square(COLOR);
    const i8 enpassant_pawn_square = enpassant_square + DOWN;

    const u64 enemy_queen = board.get_pieces(piece::type::QUEEN, !COLOR);
    const u64 enemy_bishop = board.get_pieces(piece::type::BISHOP, !COLOR) | enemy_queen;
    const u64 enemy_rook = board.get_pieces(piece::type::ROOK, !COLOR) | enemy_queen;

    u64 pawn_enpassant_possible = attack::get_pawn(enpassant_square, !COLOR) & pawn_lr;

    while (pawn_enpassant_possible)
    {
        const i8 from = bitboard::get_lsb(pawn_enpassant_possible);
        const i8 to = enpassant_square;

        pawn_enpassant_possible = bitboard::get_pop_lsb(pawn_enpassant_possible);

        // If doing this enpassant move will danger our king then it is illegal
        u64 occupied_enpassed = occupied ^ bitboard::create(from) ^ bitboard::create(to) ^ bitboard::create(enpassant_pawn_square);

        if (attack::get_bishop(king_square, occupied_enpassed) & enemy_bishop) {
            continue;
        }

        if (attack::get_rook(king_square, occupied_enpassed) & enemy_rook) {
            continue;
        }

        // Pushes
        list.add(move::get_make<move::type::ENPASSANT>(from, to));
    }
};

// Helper function
template <typename T>
inline void while_mask_add(arrayvec<u16, MAX>& list, u64 mask, T callback)
{
    while (mask)
    {
        const i8 from = bitboard::get_lsb(mask);
        mask = bitboard::get_pop_lsb(mask);

        u64 moves = callback(from);

        while (moves)
        {
            const i8 to = bitboard::get_lsb(moves);
            moves = bitboard::get_pop_lsb(moves);

            list.add(move::get_make<move::type::NORMAL>(from, to));
        }
    }
}

// Gets all legal moves
template <type TYPE, i8 COLOR>
inline arrayvec<u16, MAX> get_legal(Board& board)
{
    auto list = arrayvec<u16, MAX>();

    const u64 us = board.get_colors(COLOR);
    const u64 them = board.get_colors(!COLOR);
    const u64 occupied = us | them;

    const u64 us_empty = ~us;

    const i8 king_square = board.get_king_square(COLOR);

    const auto [check_mask, checker] = generate::get_check_mask<COLOR>(board, king_square);
    const auto pin_mask_rook = generate::get_pin_mask_rook<COLOR>(board, king_square);
    const auto pin_mask_bishop = generate::get_pin_mask_bishop<COLOR>(board, king_square);

    assert(checker <= 2);

    // Moves have to be on the checkmask
    u64 movable;

    if (TYPE == move::generate::type::ALL) {
        movable = us_empty;
    }
    else if (TYPE == move::generate::type::NOISY) {
        movable = them;
    }
    else {
        movable = ~occupied;
    }

    // King
    const u64 seen_mask = generate::get_seen_mask<!COLOR>(board, us_empty);

    generate::while_mask_add(
        list,
        bitboard::create(king_square),
        [&] (i8 square) {
            return attack::get_king(square) & movable & ~seen_mask;
        }
    );

    if (checker == 0) {
        u64 rook_castle = generate::get_castle_rook_mask<TYPE, COLOR>(board, seen_mask, pin_mask_rook);

        while (rook_castle)
        {
            const i8 king_castle_to = bitboard::get_lsb(rook_castle);
            rook_castle = bitboard::get_pop_lsb(rook_castle);

            list.add(move::get_make<move::type::CASTLING>(king_square, king_castle_to));
        }
    }

    movable &= check_mask;

    // Early return on double check
    if (checker == 2) {
        return list;
    }

    // Pawn
    generate::push_pawn<TYPE, COLOR>(list, board, check_mask, pin_mask_rook, pin_mask_bishop);

    // Knight
    const u64 knight = board.get_pieces(piece::type::KNIGHT, COLOR) & ~(pin_mask_rook | pin_mask_bishop);

    generate::while_mask_add(
        list,
        knight,
        [&] (i8 square) {
            return attack::get_knight(square) & movable;
        }
    );

    // Slider
    const u64 queen = board.get_pieces(piece::type::QUEEN, COLOR);

    // Bishop
    u64 bishop = (board.get_pieces(piece::type::BISHOP, COLOR) | queen) & ~pin_mask_rook;

    generate::while_mask_add(
        list,
        bishop,
        [&] (i8 square) {
            if (pin_mask_bishop & bitboard::create(square)) {
                return attack::get_bishop(square, occupied) & pin_mask_bishop & movable;
            }

            return attack::get_bishop(square, occupied) & movable;
        }
    );

    // Rook
    u64 rook = (board.get_pieces(piece::type::ROOK, COLOR) | queen) & ~pin_mask_bishop;

    generate::while_mask_add(
        list,
        rook,
        [&] (i8 square) {
            if (pin_mask_rook & bitboard::create(square)) {
                return attack::get_rook(square, occupied) & pin_mask_rook & movable;
            }

            return attack::get_rook(square, occupied) & movable;
        }
    );

    return list;
};

// Gets all legal moves for the side to move of this board
template <type TYPE>
inline arrayvec<u16, MAX> get_legal(Board& board)
{
    if (board.get_color() == color::WHITE) {
        return generate::get_legal<TYPE, color::WHITE>(board);
    }

    return generate::get_legal<TYPE, color::BLACK>(board);
};

};

namespace move
{

inline u64 perft(Board& board, i32 depth)
{
    auto moves = generate::get_legal<generate::type::ALL>(board);

    if (depth == 1) {
        return moves.size();
    }

    u64 count = 0;

    for (usize i = 0; i < moves.size(); ++i) {
        board.make(moves[i]);
        count += move::perft(board, depth - 1);
        board.unmake(moves[i]);
    }

    return count;
};

inline u64 perft_divide(Board& board, i32 depth)
{
    auto moves = generate::get_legal<generate::type::ALL>(board);

    if (depth == 1) {
        return moves.size();
    }

    u64 count = 0;

    for (usize i = 0; i < moves.size(); ++i) {
        i8 from = move::get_square_from(moves[i]);
        i8 to = move::get_square_to(moves[i]);

        std::cout << square::get_str(from) << square::get_str(to) << ": ";

        board.make(moves[i]);
        u64 c = move::perft(board, depth - 1);
        board.unmake(moves[i]);

        count += c;
        printf("%llu\n", c);
    }

    return count;
};

};