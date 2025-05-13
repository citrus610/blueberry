#include "engine/uci.h"

int main()
{
    chess::init();
    search::init();

    auto board = Board();
    auto settings = search::Settings();
    auto engine = search::Engine();

    const std::string NAME = "blueberry v0.1";
    const std::string AUTHOR = "citrus610";

    // Init table
    // TODO: remove this and move it to when uci sends options
    engine.init();

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
            settings = search::Settings();

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

            std::cout << board.get_fen() << std::endl;

            continue;
        }

        if (tokens[0] == "go") {
            // Reads go infos
            auto settings_uci = uci::get_command_go(input);

            if (!settings_uci.has_value()) {
                continue;
            }

            settings = settings_uci.value();

            // Stops thread
            engine.stop();

            // Starts search thread
            engine.search(board, settings);

            continue;
        }

        if (tokens[0] == "stop") {
            // Stops thread
            engine.stop();

            continue;
        }

        if (tokens[0] == "quit" || tokens[0] == "exit") {
            // Stops thread
            engine.stop();

            break;
        }
    }
    
    return 0;
};