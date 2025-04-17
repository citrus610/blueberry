#include "uci.h"

int main()
{
    init();

    // {
    //     auto board = Board("r6k/p2r2pp/2bp1p2/4p3/8/PP4N1/R1P2PPP/4R2K b - - 9 38");

    //     auto ms = move::generate::get_legal<move::generate::type::NOISY>(board);

    //     for (usize i = 0; i < ms.size(); ++i) {
    //         std::cout << move::get_str(ms[i]) << std::endl;
    //     }

    //     auto data = search::Data();
    //     data.board = board;

    //     std::atomic_flag f;
    //     f.test_and_set();

    //     auto score = search::negamax(data, -eval::score::INFINITE, eval::score::INFINITE, 7, f);

    //     uci::print_info(7, score, data.nodes, data.pv_table[0]);
    // }

    // return 0;

    auto board = Board();
    auto info = search::Info();
    auto engine = search::Engine();

    const std::string NAME = "blueberry v0.1";
    const std::string AUTHOR = "citrus610";

    while (true)
    {
        // Gets input
        std::string input;
        std::getline(std::cin, input);

        if (input.empty()) {
            continue;
        }

        // Splits into tokens
        std::stringstream ss(input);
        std::string token;
        std::vector<std::string> tokens;

        while (std::getline(ss, token, ' '))
        {
            tokens.push_back(token);
        }

        // Reads input
        if (tokens[0] == "uci") {
            std::cout << "id name " << NAME << std::endl;
            std::cout << "id author " << AUTHOR << std::endl;
            std::cout << "uciok" << std::endl;

            continue;
        }

        if (tokens[0] == "isready") {
            std::cout << "readyok" << std::endl;

            continue;
        }

        if (tokens[0] == "ucinewgame") {
            board = Board();
            info = search::Info();

            engine.stop();
            engine.clear();

            continue;
        }

        if (tokens[0] == "position") {
            auto board_uci = uci::get_command_position(input);

            if (!board_uci.has_value()) {
                continue;
            }

            board = board_uci.value();

            continue;
        }

        if (tokens[0] == "go") {
            // Reads go infos
            auto info_uci = uci::get_command_go(input);

            if (!info_uci.has_value()) {
                continue;
            }

            info = info_uci.value();

            // Stops thread
            engine.stop();
            engine.clear();

            // Starts search thread
            engine.search(board, info);

            continue;
        }

        if (tokens[0] == "stop") {
            // Stops thread
            engine.stop();
            engine.clear();

            continue;
        }

        if (tokens[0] == "quit" || tokens[0] == "exit") {
            // Stops thread
            engine.stop();
            engine.clear();

            break;
        }
    }
    
    return 0;
};