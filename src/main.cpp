#include "uci.h"

int main()
{
    init();

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
            std::cout << "id name " << NAME << "\n";
            std::cout << "id author " << AUTHOR << "\n";
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