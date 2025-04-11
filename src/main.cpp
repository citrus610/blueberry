#include "chess/chess.h"

int main()
{
    init();

    auto board = Board();

    printf("depth?\n");

    i32 depth = 1;

    std::cin >> depth;

    printf("perft...\n");

    auto t1 = std::chrono::high_resolution_clock::now();

    u64 count = move::perft(board, depth);

    auto t2 = std::chrono::high_resolution_clock::now();

    u64 time = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

    printf("depth %d: %llu nodes - %llu ms - %llu kn/s\n", depth, count, time, count / time);

    return 0;
};