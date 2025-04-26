#include "board.h"

Board::Board(const std::string& fen)
{
    this->set_fen(fen);
    this->history.reserve(512);
};

void Board::set_fen(const std::string& fen)
{
    // Zero init
    for (i8 p = 0; p < 6; ++p) {
        this->pieces[p] = 0ULL;
    }

    for (i8 c = 0; c < 2; ++c) {
        this->colors[c] = 0ULL;
    }

    for (i8 sq = 0; sq < 64; ++sq) {
        this->board[sq] = piece::NONE;
    }
    
    this->color = color::WHITE;
    this->castling = castling::NONE;
    this->enpassant = square::NONE;
    this->halfmove = 0;
    this->ply = 0;
    this->hash = 0;

    // Reads fen
    std::stringstream ss(fen);
    std::string word;
    std::vector<std::string> data;

    while (std::getline(ss, word, ' '))
    {
        data.push_back(word);
    }

    assert(data.size() >= 4);
    assert(!data[0].empty());

    auto str_board = data.size() > 0 ? data[0] : "";
    auto str_color = data.size() > 1 ? data[1] : "w";
    auto str_castling = data.size() > 2 ? data[2] : "-";
    auto str_enpassant = data.size() > 3 ? data[3] : "-";
    auto str_halfmove = data.size() > 4 ? data[4] : "0";
    auto str_fullmove = data.size() > 5 ? data[5] : "1";

    // Sets board
    auto square = 56;

    for (char c : str_board) {
        if (std::isdigit(c)) {
            square += c - '0';
        }
        else if (c == '/') {
            square -= 16;
        }
        else {
            i8 piece = piece::create(c);
            i8 piece_type = piece::get_type(piece);
            i8 piece_color = piece::get_color(piece);

            this->board[square] = piece;
            this->pieces[piece_type] |= bitboard::create(square);
            this->colors[piece_color] |= bitboard::create(square);

            this->hash ^= zobrist::get_piece(piece, square);

            square += 1;
        }
    }

    // Sets color
    this->color = str_color == "w" ? color::WHITE : color::BLACK;

    if (this->color == color::WHITE) {
        this->hash ^= zobrist::get_color();
    }

    // Sets castling
    for (char c : str_castling) {
        this->castling |= (c == 'K') ? castling::WHITE_SHORT : castling::NONE;
        this->castling |= (c == 'Q') ? castling::WHITE_LONG : castling::NONE;
        this->castling |= (c == 'k') ? castling::BLACK_SHORT : castling::NONE;
        this->castling |= (c == 'q') ? castling::BLACK_LONG : castling::NONE;
    }

    if (this->castling) {
        this->hash ^= zobrist::get_castling(this->castling);
    }

    // Sets enpassant square
    if (str_enpassant == "-") {
        this->enpassant = square::NONE;
    }
    else {
        assert(str_enpassant.size() == 2);

        i8 enpassant_file = file::create(str_enpassant[0]);
        i8 enpassant_rank = rank::create(str_enpassant[1]);

        this->enpassant = square::create(enpassant_file, enpassant_rank);

        this->hash ^= zobrist::get_enpassant(enpassant_file);
    }

    // Sets move count
    this->halfmove = std::stoi(str_halfmove);
    this->ply = std::stoi(str_fullmove) * 2 - 2 + this->color;
};

void Board::set_ply(i32 ply)
{
    this->ply = ply;
};

u64 Board::get_occupied()
{
    return this->colors[color::WHITE] | this->colors[color::BLACK];
};

u64 Board::get_pieces(i8 piece_type)
{
    return this->pieces[piece_type];
};

u64 Board::get_pieces(i8 piece_type, i8 color)
{
    return this->pieces[piece_type] & this->colors[color];
};

u64 Board::get_colors(i8 color)
{
    return this->colors[color];
};

i8 Board::get_color()
{
    return this->color;
};

i8 Board::get_piece_type_at(i8 square)
{
    assert(square::is_valid(square));

    if (this->board[square] == piece::NONE) {
        return piece::type::NONE;
    }

    return piece::get_type(this->board[square]);
};

i8 Board::get_color_at(i8 square)
{
    assert(square::is_valid(square));

    if (this->board[square] == piece::NONE) {
        return color::NONE;
    }

    return piece::get_color(this->board[square]);
};

i8 Board::get_piece_at(i8 square)
{
    assert(square::is_valid(square));

    return this->board[square];
};

i8 Board::get_king_square(i8 color)
{
    assert(this->get_pieces(piece::type::KING, color) > 0);

    return bitboard::get_lsb(this->get_pieces(piece::type::KING, color));
};

i8 Board::get_castling_right()
{
    return this->castling;
};

i8 Board::get_enpassant_square()
{
    return this->enpassant;
};

i32 Board::get_halfmove_count()
{
    return this->halfmove;
};

i32 Board::get_fullmove_count()
{
    return this->ply / 2 + 1;
};

i32 Board::get_ply()
{
    return this->ply;
};

u64 Board::get_square_attacker(i8 square)
{
    u64 occupied = this->get_occupied();

    return
        (attack::get_pawn(square, color::WHITE) & this->colors[color::BLACK] & this->pieces[piece::type::PAWN]) |
        (attack::get_pawn(square, color::BLACK) & this->colors[color::WHITE] & this->pieces[piece::type::PAWN]) |
        (attack::get_knight(square) & this->pieces[piece::type::KNIGHT]) |
        (attack::get_bishop(square, occupied) & (this->pieces[piece::type::BISHOP] | this->pieces[piece::type::QUEEN])) |
        (attack::get_rook(square, occupied) & (this->pieces[piece::type::ROOK] | this->pieces[piece::type::QUEEN])) |
        (attack::get_king(square) & this->pieces[piece::type::KING]);
};

u64 Board::get_hash()
{
    return this->hash;
};

std::string Board::get_fen()
{
    std::string fen;

    for (i8 rank = 7; rank >= 0; --rank) {
        i32 space = 0;

        for (i8 file = 0; file < 8; ++file) {
            i8 square = square::create(file, rank);
            i8 piece = this->get_piece_at(square);

            if (piece != piece::NONE) {
                if (space) {
                    fen += std::to_string(space);
                    space = 0;
                }

                fen += piece::get_char(piece);
            }
            else {
                space += 1;
            }
        }

        if (space) {
            fen += std::to_string(space);
        }

        if (rank > 0) {
            fen += "/";
        }
    }

    fen += " ";
    fen += color::get_char(this->color);

    if (this->castling == castling::NONE) {
        fen += " -";
    }
    else {
        fen += " ";
        fen += (this->castling & castling::WHITE_SHORT) ? "K" : "";
        fen += (this->castling & castling::WHITE_LONG) ? "Q" : "";
        fen += (this->castling & castling::BLACK_SHORT) ? "k" : "";
        fen += (this->castling & castling::BLACK_LONG) ? "q" : "";
    }

    if (this->enpassant == square::NONE) {
        fen += " -";
    }
    else {
        fen += " ";
        fen += file::get_char(square::get_file(this->enpassant));
        fen += rank::get_char(square::get_rank(this->enpassant));
    }

    fen += " ";
    fen += std::to_string(this->halfmove);
    fen += " ";
    fen += std::to_string(this->get_fullmove_count());

    return fen;
};

bool Board::is_drawn_repitition()
{
    i32 count = 0;
    i32 size = static_cast<i32>(this->history.size());

    for (i32 i = size - 2; i >= 0 && i >= size - this->halfmove - 1; i -= 2) {
        if (this->history[i].hash == this->hash) {
            count += 1;
        }

        if (count == 2) {
            return true;
        }
    }

    return false;
};

bool Board::is_drawn_fifty_move()
{
    return this->halfmove >= 100;
};

bool Board::is_drawn_insufficient()
{
    i32 count = bitboard::get_count(this->get_occupied());

    if (count == 2) {
        return true;
    }

    if (count == 3) {
        if (this->pieces[piece::type::KNIGHT] || this->pieces[piece::type::BISHOP]) {
            return true;
        }
    }

    if (count == 4) {
        if (bitboard::is_many(this->pieces[piece::type::BISHOP]) &&
            square::is_same_color(bitboard::get_lsb(this->pieces[piece::type::BISHOP]), bitboard::get_msb(this->pieces[piece::type::BISHOP]))) {
            return true;
        }
    }

    return false;
};

bool Board::is_square_attacked(i8 square, i8 color)
{
    assert(square::is_valid(square));
    assert(color::is_valid(color));

    const u64 enemy = this->colors[!color];
    const u64 occupied = this->get_occupied();

    const u64 enemy_pawns = enemy & this->pieces[piece::type::PAWN];

    if (attack::get_pawn(square, color) & enemy_pawns) {
        return true;
    }

    const u64 enemy_knights = enemy & this->pieces[piece::type::KNIGHT];

    if (attack::get_knight(square) & enemy_knights) {
        return true;
    }

    const u64 enemy_bishops = enemy & (this->pieces[piece::type::BISHOP] | this->pieces[piece::type::QUEEN]);

    if (attack::get_bishop(square, occupied) & enemy_bishops) {
        return true;
    }

    const u64 enemy_rooks = enemy & (this->pieces[piece::type::ROOK] | this->pieces[piece::type::QUEEN]);

    if (attack::get_rook(square, occupied) & enemy_rooks) {
        return true;
    }

    const u64 enemy_kings = enemy & this->pieces[piece::type::KING];

    return attack::get_king(square) & enemy_kings;
};

bool Board::is_in_check(i8 color)
{
    assert(color::is_valid(color));

    const i8 square = this->get_king_square(color);

    const u64 enemy = this->colors[!color];
    const u64 occupied = this->get_occupied();

    const u64 enemy_pawns = enemy & this->pieces[piece::type::PAWN];

    if (attack::get_pawn(square, color) & enemy_pawns) {
        return true;
    }

    const u64 enemy_knights = enemy & this->pieces[piece::type::KNIGHT];

    if (attack::get_knight(square) & enemy_knights) {
        return true;
    }

    const u64 enemy_bishops = enemy & (this->pieces[piece::type::BISHOP] | this->pieces[piece::type::QUEEN]);

    if (attack::get_bishop(square, occupied) & enemy_bishops) {
        return true;
    }

    const u64 enemy_rooks = enemy & (this->pieces[piece::type::ROOK] | this->pieces[piece::type::QUEEN]);

    if (attack::get_rook(square, occupied) & enemy_rooks) {
        return true;
    }

    return false;
};

bool Board::is_move_quiet(u16 move)
{
    return move::get_type(move) != move::type::PROMOTION && (this->board[move::get_square_to(move)] == piece::NONE || move::get_type(move) == move::type::CASTLING);
};

bool Board::has_non_pawn(i8 color)
{
    return this->colors[this->color] ^ this->pieces[piece::type::PAWN] ^ this->pieces[piece::type::KING];
};

void Board::make(u16 move)
{
    // Gets move data
    i8 move_from = move::get_square_from(move);
    i8 move_to = move::get_square_to(move);
    u16 move_type = move::get_type(move);

    i8 piece_type = this->get_piece_type_at(move_from);
    i8 captured =
        move_type == move::type::CASTLING ?
        piece::type::NONE :
        this->get_piece_type_at(move_to);

    // Validates
    assert(piece_type != piece::type::NONE);
    assert(this->get_color_at(move_from) == this->color);
    assert(this->get_color_at(move_to) != this->color || move_type == move::type::CASTLING);

    // Saves info
    this->history.push_back(Undo {
        .hash = this->hash,
        .castling = this->castling,
        .enpassant = this->enpassant,
        .captured = captured,
        .halfmove = this->halfmove
    });

    // Updates move counter
    this->halfmove += 1;
    this->ply += 1;

    // Removes enpassant square
    if (this->enpassant != square::NONE) {
        this->hash ^= zobrist::get_enpassant(square::get_file(this->enpassant));
        this->enpassant = square::NONE;
    }

    // Checks capture
    if (captured != piece::type::NONE) {
        // Updates half move counter
        this->halfmove = 0;

        // Removes piece
        this->remove(captured, !this->color, move_to);

        // Updates hash
        this->hash ^= zobrist::get_piece(piece::create(captured, !this->color), move_to);

        // Removes castling right if a rook is captured
        if (captured == piece::type::ROOK) {
            i8 castling_removed = castling::create(move_to) & this->castling;

            if (castling_removed) {
                this->castling ^= castling_removed;
                this->hash ^= zobrist::get_castling(castling_removed);
            }
        }
    }

    // Checks piece-specific actions
    if (piece_type == piece::type::KING) {
        // Removes castling rights
        i8 castling_removed =
            (this->color == color::WHITE) ?
            (this->castling & castling::WHITE) :
            (this->castling & castling::BLACK);

        if (castling_removed) {
            this->castling ^= castling_removed;
            this->hash ^= zobrist::get_castling(castling_removed);
        }
    }
    else if (piece_type == piece::type::ROOK) {
        // Removes castling right
        i8 castling_removed = castling::create(move_from) & this->castling;

        if (castling_removed) {
            this->castling ^= castling_removed;
            this->hash ^= zobrist::get_castling(castling_removed);
        }
    }
    else if (piece_type == piece::type::PAWN) {
        // Updates half move counter
        this->halfmove = 0;

        // Double push
        if (std::abs(move_from - move_to) == 16) {
            // Updates enpassant
            this->enpassant = move_to ^ 8;
            this->hash ^= zobrist::get_enpassant(square::get_file(this->enpassant));
        }
    }

    // Checks move type
    if (move_type == move::type::CASTLING) {
        assert(this->get_piece_type_at(move_from) == piece::type::KING);
        assert(this->get_piece_type_at(move_to) == piece::type::ROOK);

        bool castle_short = move_to > move_from;

        i8 king_to = castling::get_king_square(this->color, castle_short);
        i8 rook_to = castling::get_rook_square(this->color, castle_short);

        this->remove(piece::type::KING, this->color, move_from);
        this->remove(piece::type::ROOK, this->color, move_to);

        this->place(piece::type::KING, this->color, king_to);
        this->place(piece::type::ROOK, this->color, rook_to);

        i8 king = piece::create(piece::type::KING, this->color);
        i8 rook = piece::create(piece::type::ROOK, this->color);

        this->hash ^= zobrist::get_piece(king, move_from);
        this->hash ^= zobrist::get_piece(king, king_to);
        this->hash ^= zobrist::get_piece(rook, move_to);
        this->hash ^= zobrist::get_piece(rook, rook_to);
    }
    else if (move_type == move::type::PROMOTION) {
        assert(piece_type == piece::type::PAWN);

        i8 promotion = move::get_promotion_type(move);

        this->remove(piece_type, this->color, move_from);
        this->place(promotion, this->color, move_to);

        this->hash ^= zobrist::get_piece(piece::create(piece_type, this->color), move_from);
        this->hash ^= zobrist::get_piece(piece::create(promotion, this->color), move_to);
    }
    else {
        assert(this->get_piece_type_at(move_to) == piece::type::NONE);
        
        this->remove(piece_type, this->color, move_from);
        this->place(piece_type, this->color, move_to);

        i8 piece = piece::create(piece_type, this->color);

        this->hash ^= zobrist::get_piece(piece, move_from);
        this->hash ^= zobrist::get_piece(piece, move_to);
    }

    // Captures enpassant pawn
    if (move_type == move::type::ENPASSANT) {
        i8 enpassant_square = move_to ^ 8;

        assert(piece_type == piece::type::PAWN);
        assert(move_to == this->history.back().enpassant);

        this->remove(piece::type::PAWN, !this->color, enpassant_square);

        this->hash ^= zobrist::get_piece(piece::create(piece::type::PAWN, !this->color), enpassant_square);
    }

    // Updates color
    this->color = !this->color;
    this->hash ^= zobrist::get_color();
};

void Board::unmake(u16 move)
{
    // Reverts history
    auto undo = this->history.back();
    this->history.pop_back();

    this->hash = undo.hash;
    this->castling = undo.castling;
    this->enpassant = undo.enpassant;
    this->halfmove = undo.halfmove;

    this->color = !this->color;
    this->ply -= 1;

    // Gets move data
    i8 move_from = move::get_square_from(move);
    i8 move_to = move::get_square_to(move);
    u16 move_type = move::get_type(move);

    // Checks move type
    if (move_type == move::type::CASTLING) {
        bool castle_short = move_to > move_from;

        i8 king_to = castling::get_king_square(this->color, castle_short);
        i8 rook_to = castling::get_rook_square(this->color, castle_short);

        assert(this->get_piece_type_at(king_to) == piece::type::KING);
        assert(this->get_piece_type_at(rook_to) == piece::type::ROOK);

        this->remove(piece::type::KING, this->color, king_to);
        this->remove(piece::type::ROOK, this->color, rook_to);

        this->place(piece::type::KING, this->color, move_from);
        this->place(piece::type::ROOK, this->color, move_to);

        return;
    }
    else if (move_type == move::type::PROMOTION) {
        i8 promotion = move::get_promotion_type(move);

        assert(promotion == this->get_piece_type_at(move_to));

        this->remove(promotion, this->color, move_to);
        this->place(piece::type::PAWN, this->color, move_from);

        if (undo.captured != piece::type::NONE) {
            this->place(undo.captured, !this->color, move_to);
        }

        return;
    }
    else {
        assert(this->get_piece_type_at(move_to) != piece::type::NONE);
        assert(this->get_piece_type_at(move_from) == piece::type::NONE);

        i8 piece_type = this->get_piece_type_at(move_to);

        this->remove(piece_type, this->color, move_to);
        this->place(piece_type, this->color, move_from);
    }

    // Places captured piece
    if (move_type == move::type::ENPASSANT) {
        assert(this->get_piece_type_at(this->enpassant ^ 8) == piece::type::NONE);

        this->place(piece::type::PAWN, !this->color, this->enpassant ^ 8);
    }
    else if (undo.captured != piece::type::NONE) {
        this->place(undo.captured, !this->color, move_to);
    }
};

void Board::make_null()
{
    // Saves info
    this->history.push_back(Undo {
        .hash = this->hash,
        .castling = this->castling,
        .enpassant = this->enpassant,
        .captured = piece::type::NONE,
        .halfmove = this->halfmove
    });

    // Updates color
    this->color = !this->color;
    this->hash ^= zobrist::get_color();

    // Updates enpassant square
    if (this->enpassant != square::NONE) {
        this->hash ^= zobrist::get_enpassant(square::get_file(this->enpassant));
        this->enpassant = square::NONE;
    }

    // Updates move counter
    this->ply += 1;
};

void Board::unmake_null()
{
    // Reverts history
    auto undo = this->history.back();
    this->history.pop_back();

    this->hash = undo.hash;
    this->castling = undo.castling;
    this->enpassant = undo.enpassant;
    this->halfmove = undo.halfmove;

    this->color = !this->color;
    this->ply -= 1;
};

void Board::remove(i8 piece_type, i8 color, i8 square)
{
    assert(piece::type::is_valid(piece_type));
    assert(color::is_valid(color));
    assert(square::is_valid(square));

    assert(this->get_piece_type_at(square) == piece_type);
    assert(this->get_color_at(square) == color);
    assert(this->get_piece_at(square) == piece::create(piece_type, color));

    this->pieces[piece_type] ^= 1ULL << square;
    this->colors[color] ^= 1ULL << square;
    this->board[square] = piece::NONE;
};

void Board::place(i8 piece_type, i8 color, i8 square)
{
    assert(piece::type::is_valid(piece_type));
    assert(color::is_valid(color));
    assert(square::is_valid(square));

    assert(this->get_piece_type_at(square) == piece::type::NONE);
    assert(this->get_color_at(square) == color::NONE);
    assert(this->get_piece_at(square) == piece::NONE);

    this->pieces[piece_type] |= 1ULL << square;
    this->colors[color] |= 1ULL << square;
    this->board[square] = piece::create(piece_type, color);
};

void Board::print()
{
    for (i32 rank = 7; rank >= 0; --rank) {
        char line[] = ". . . . . . . .";

        for (i32 file = 0; file < 8; ++file) {
            i8 square = square::create(file, rank);

            if (this->board[square] == piece::NONE) {
                continue;
            }

            line[2 * file] = piece::get_char(this->board[square]);
        }

        printf("%s\n", line);
    }

    printf("\n");
};