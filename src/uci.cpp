#include "uci.h"

namespace uci
{

std::optional<u16> get_move(const std::string& token, Board& board)
{
    if (token.size() < 4 || token.size() > 5) {
        return {};
    }

    i8 from = square::create(
        file::create(token[0]),
        rank::create(token[1])
    );

    i8 to = square::create(
        file::create(token[2]),
        rank::create(token[3])
    );

    auto moves = move::generate::get_legal<move::generate::type::ALL>(board);

    for (auto move : moves) {
        i8 move_from = move::get_square_from(move);
        i8 move_to = move::get_square_to(move);
        i8 move_to_castle = move_to;

        if (move::get_type(move) == move::type::CASTLING) {
            bool castle_short = castling::create(move_to) & castling::SHORT;

            move_to_castle = castling::get_king_square(board.get_color(), castle_short);
        }

        if (from != move_from || (to != move_to && to != move_to_castle)) {
            continue;
        }

        if (token.size() == 5 && move::get_type(move) == move::type::PROMOTION) {
            switch (move::get_promotion_type(move))
            {
            case piece::type::KNIGHT:
                if (token[4] == 'n') {
                    return move;
                }
            case piece::type::BISHOP:
                if (token[4] == 'b') {
                    return move;
                }
            case piece::type::ROOK:
                if (token[4] == 'r') {
                    return move;
                }
            case piece::type::QUEEN:
                if (token[4] == 'q') {
                    return move;
                }
            }

            continue;
        }

        return move;
    }

    return {};
};

std::optional<Board> get_command_position(std::string in)
{
    Board board;

    if (in.find("startpos") != std::string::npos) {
        board = Board();
    }
    else if (in.find("fen") != std::string::npos) {
        auto fen = in.substr(in.find("fen") + 4);

        board = Board(in.substr(in.find("fen") + 4, std::string::npos));
    }

    if (in.find("moves") != std::string::npos) {
        auto start = in.find("moves") + 6;
        
        if (in.length() >= start) {
            std::string moves_substr = in.substr(start, std::string::npos);

            std::stringstream ss(moves_substr);
            std::string token;

            while (std::getline(ss, token, ' '))
            {
                auto move = uci::get_move(token, board);

                if (!move.has_value()) {
                    return {};
                }

                board.make(move.value());
            }
        }
    }

    return board;
};

std::optional<search::Settings> get_command_go(std::string in)
{
    auto info = search::Settings {
        .depth = MAX_PLY,
        .time = { 0, 0 },
        .inc = { 0, 0 },
        .movestogo = {},
        .infinite = false
    };

    std::stringstream ss(in);
    std::string token;
    std::vector<std::string> tokens;

    while (std::getline(ss, token, ' '))
    {
        tokens.push_back(token);
    }

    for (usize i = 1; i < tokens.size(); ++i) {
        if (tokens[i] == "infinite") {
            info.infinite = true;
        }

        if (tokens[i] == "winc") {
            info.inc[color::WHITE] = std::stoi(tokens[i + 1]);
        }

        if (tokens[i] == "binc") {
            info.inc[color::BLACK] = std::stoi(tokens[i + 1]);
        }

        if (tokens[i] == "wtime") {
            info.time[color::WHITE] = std::stoi(tokens[i + 1]);
        }
        if (tokens[i] == "btime") {
            info.time[color::BLACK] = std::stoi(tokens[i + 1]);
        }

        if (tokens[i] == "depth") {
            info.depth = std::stoi(tokens[i + 1]);
        }

        if (tokens[i] == "movestogo") {
            info.movestogo = std::stoi(tokens[i + 1]);
        }
    }

    return info;
};

void print_info(i32 depth, i32 seldepth, i32 score, u64 nodes, u64 ms, u64 hashfull, search::PV pv)
{
    std::cout << "info ";

    std::cout << "depth " << depth << " ";

    std::cout << "seldepth " << seldepth << " ";

    if (score >= eval::score::MATE_FOUND || score <= -eval::score::MATE_FOUND) {
        std::cout << "score mate " << ((eval::score::MATE - score) / 2) << " ";
    }
    else {
        std::cout << "score cp " << score << " ";
    }

    std::cout << "nodes " << nodes << " ";

    std::cout << "nps " << (nodes * 1000 / std::max(ms, 1ULL)) << " ";

    std::cout << "hashfull " << hashfull <<  " ";

    std::cout << "pv ";
    
    for (i32 i = 0; i < pv.count; ++i) {
        std::cout << move::get_str(pv.data[i]) << " ";
    }
    
    std::cout << std::endl;
};

void print_bestmove(u16 move)
{
    std::cout << "bestmove " << move::get_str(move) << std::endl;
};

};