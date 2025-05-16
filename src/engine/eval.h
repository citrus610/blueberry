#pragma once

#include "../chess/chess.h"

namespace eval::score
{

constexpr i32 DRAW = 0;
constexpr i32 MATE = 32000 + MAX_PLY;
constexpr i32 MATE_FOUND = MATE - MAX_PLY;
constexpr i32 NONE = MATE + 1;
constexpr i32 INFINITE = INT16_MAX;

constexpr i32 create(i32 midgame, i32 endgame)
{
    assert(midgame >= INT16_MIN && midgame <= INT16_MAX);
    assert(endgame >= INT16_MIN && endgame <= INT16_MAX);

    return midgame + static_cast<i32>(static_cast<u32>(endgame) << 16);
};

constexpr i32 get_midgame(i32 score)
{
    return static_cast<int16_t>(static_cast<uint16_t>(static_cast<u32>(score)));
};

constexpr i32 get_endgame(i32 score)
{
    return static_cast<int16_t>(static_cast<uint16_t>(static_cast<u32>(score + 0x8000) >> 16));
};

};

namespace eval
{

#define S(a, b) score::create(a, b)
#define SW(p, w, mg, eg) w.p = score::create(mg.p, eg.p)

// SEE piece values
constexpr i32 SEE_VALUE[6] = {
    100, 320, 330, 500, 900, 100000
};

// Endgame scale
constexpr i32 SCALE_MAX = 128;

// Phase
const i32 PHASE_SCALE = 256;
const i32 PHASE_KNIGHT = 1;
const i32 PHASE_BISHOP = 1;
const i32 PHASE_ROOK = 2;
const i32 PHASE_QUEEN = 4;
const i32 PHASE_MAX = PHASE_KNIGHT * 4 + PHASE_BISHOP * 4 + PHASE_ROOK * 4 + PHASE_QUEEN * 2;

struct Weight
{
    i32 material_pawn;
    i32 material_knight;
    i32 material_bishop;
    i32 material_rook;
    i32 material_queen;
    i32 material_king;

    i32 table[6][64];

    i32 mobility_knight[9];
    i32 mobility_bishop[14];
    i32 mobility_rook[15];
    i32 mobility_queen[28];

    i32 king_defense[9];

    i32 tempo;
};

constexpr Weight MG = Weight {
    .material_pawn = 100,
    .material_knight = 320,
    .material_bishop = 330,
    .material_rook = 500,
    .material_queen = 900,
    .material_king = 10000,

    .table = {
        {
            0, 0, 0, 0, 0, 0, 0, 0,
            5, 10, 10, -20, -20, 10, 10, 5,
            5, -5, -10, 0, 0, -10, -5, 5,
            0, 0, 0, 20, 20, 0, 0, 0,
            5, 5, 10, 25, 25, 10, 5, 5,
            10, 10, 20, 30, 30, 20, 10, 10,
            50, 50, 50, 50, 50, 50, 50, 50,
            0, 0, 0, 0, 0, 0, 0, 0
        },
        {
            -50, -40, -30, -30, -30, -30, -40, -50,
            -40, -20, 0, 5, 5, 0, -20, -40,
            -30, 5, 10, 15, 15, 10, 5, -30,
            -30, 0, 15, 20, 20, 15, 0, -30,
            -30, 5, 15, 20, 20, 15, 5, -30,
            -30, 0, 10, 15, 15, 10, 0, -30,
            -40, -20, 0, 0, 0, 0, -20, -40,
            -50, -40, -30, -30, -30, -30, -40, -50
        },
        {
            -20, -10, -10, -10, -10, -10, -10, -20,
            -10, 5, 0, 0, 0, 0, 5, -10,
            -10, 10, 10, 10, 10, 10, 10, -10,
            -10, 0, 10, 10, 10, 10, 0, -10,
            -10, 5, 5, 10, 10, 5, 5, -10,
            -10, 0, 5, 10, 10, 5, 0, -10,
            -10, 0, 0, 0, 0, 0, 0, -10,
            -20, -10, -10, -10, -10, -10, -10, -20
        },
        {
            0, 0, 0, 5, 5, 0, 0, 0,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            5, 10, 10, 10, 10, 10, 10, 5,
            0, 0, 0, 0, 0, 0, 0, 0
        },
        {
            -20, -10, -10, -5, -5, -10, -10, -20,
            -10, 0, 0, 0, 0, 5, 0, -10,
            -10, 0, 5, 5, 5, 5, 5, -10,
            0, 0, 5, 5, 5, 5, 0, -5,
            -5, 0, 5, 5, 5, 5, 0, -5,
            -10, 0, 5, 5, 5, 5, 0, -10,
            -10, 0, 0, 0, 0, 0, 0, -10,
            -20, -10, -10, -5, -5, -10, -10, -20
        },
        {
            20, 30, 10, 0, 0, 10, 30, 20,
            20, 20, 0, 0, 0, 0, 20, 20,
            -10, -20, -20, -20, -20, -20, -20, -10,
            -20, -30, -30, -40, -40, -30, -30, -20,
            -30, -40, -40, -50, -50, -40, -40, -30,
            -30, -40, -40, -50, -50, -40, -40, -30,
            -30, -40, -40, -50, -50, -40, -40, -30,
            -30, -40, -40, -50, -50, -40, -40, -30,
        }
    },

    .mobility_knight = {
        -50, -25, -10, -5, 5, 10, 20, 30, 50
    },
    .mobility_bishop = {
        -50, -40, -15, -5, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50
    },
    .mobility_rook = {
        -50, -40, -25, -15, -10, -10, -10, -5, 5, 10, 15, 20, 25, 40, 50,
    },
    .mobility_queen = {
        -50, -40, -25, -20, -15, -10, -5, 0, 5, 10, 10, 15, 15, 20, 20, 20, 25, 15, 15, 15, 25, 35, 35, 30, 10, 10, -20, -20
    },

    .king_defense = {
        -35, -15, 0, 15, 20, 25, 30, 15, 15
    }
};

constexpr Weight EG = Weight {
    .material_pawn = 100,
    .material_knight = 320,
    .material_bishop = 330,
    .material_rook = 500,
    .material_queen = 1000,
    .material_king = 10000,

    .table = {
        {
            0, 0, 0, 0, 0, 0, 0, 0,
            5, 5, 5, -20, -20, 5, 5, 5,
            10, 10, 10, 10, 10, 10, 10, 10,
            20, 20, 20, 20, 20, 20, 20, 20,
            30, 30, 30, 30, 30, 30, 30, 30,
            40, 40, 40, 40, 40, 40, 40, 40,
            50, 50, 50, 50, 50, 50, 50, 50,
            0, 0, 0, 0, 0, 0, 0, 0
        },
        {
            -50, -40, -30, -30, -30, -30, -40, -50,
            -40, -20, 0, 5, 5, 0, -20, -40,
            -30, 5, 10, 15, 15, 10, 5, -30,
            -30, 0, 15, 20, 20, 15, 0, -30,
            -30, 5, 15, 20, 20, 15, 5, -30,
            -30, 0, 10, 15, 15, 10, 0, -30,
            -40, -20, 0, 0, 0, 0, -20, -40,
            -50, -40, -30, -30, -30, -30, -40, -50
        },
        {
            -20, -10, -10, -10, -10, -10, -10, -20,
            -10, 5, 0, 0, 0, 0, 5, -10,
            -10, 10, 10, 10, 10, 10, 10, -10,
            -10, 0, 10, 10, 10, 10, 0, -10,
            -10, 5, 5, 10, 10, 5, 5, -10,
            -10, 0, 5, 10, 10, 5, 0, -10,
            -10, 0, 0, 0, 0, 0, 0, -10,
            -20, -10, -10, -10, -10, -10, -10, -20
        },
        {
            0, 0, 0, 5, 5, 0, 0, 0,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            5, 10, 10, 10, 10, 10, 10, 5,
            0, 0, 0, 0, 0, 0, 0, 0
        },
        {
            -20, -10, -10, -5, -5, -10, -10, -20,
            -10, 0, 0, 0, 0, 5, 0, -10,
            -10, 0, 5, 5, 5, 5, 5, -10,
            0, 0, 5, 5, 5, 5, 0, -5,
            -5, 0, 5, 5, 5, 5, 0, -5,
            -10, 0, 5, 5, 5, 5, 0, -10,
            -10, 0, 0, 0, 0, 0, 0, -10,
            -20, -10, -10, -5, -5, -10, -10, -20
        },
        {
            -50, -30, -30, -30, -30, -30, -30, -50,
            -30, -30, 0, 0, 0, 0, -30, -30,
            -30, -10, 20, 30, 30, 20, -10, -30,
            -30, -10, 30, 40, 40, 30, -10, -30,
            -30, -10, 30, 40, 40, 30, -10, -30,
            -30, -10, 20, 30, 30, 20, -10, -30,
            -30, -20, -10, 0, 0, -10, -20, -30,
            -50, -40, -30, -20, -20, -30, -40, -50,
        }
    },

    .mobility_knight = {
        -50, -25, -10, -5, 5, 10, 20, 30, 50
    },
    .mobility_bishop = {
        -50, -40, -15, -5, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50
    },
    .mobility_rook = {
        -50, -40, -25, -15, -10, -10, -10, -5, 5, 10, 15, 20, 25, 40, 50,
    },
    .mobility_queen = {
        -50, -40, -25, -20, -15, -10, -5, 0, 5, 10, 10, 15, 15, 20, 20, 20, 25, 15, 15, 15, 25, 35, 35, 30, 10, 10, -20, -20
    },

    .king_defense = {
        -5, 0, 5, 5, 5, 0, -5, -10, -10
    }
};

constexpr Weight DEFAULT = [] {
    Weight w = Weight();

    SW(material_pawn, w, MG, EG);
    SW(material_knight, w, MG, EG);
    SW(material_bishop, w, MG, EG);
    SW(material_rook, w, MG, EG);
    SW(material_queen, w, MG, EG);
    SW(material_king, w, MG, EG);

    for (i32 p = 0; p < 6; ++p) {
        for (i32 sq = 0; sq < 64; ++sq) {
            SW(table[p][sq], w, MG, EG);
        }
    }

    for (i32 i = 0; i < 9; ++i) {
        SW(mobility_knight[i], w, MG, EG);
    }

    for (i32 i = 0; i < 14; ++i) {
        SW(mobility_bishop[i], w, MG, EG);
    }

    for (i32 i = 0; i < 15; ++i) {
        SW(mobility_rook[i], w, MG, EG);
    }

    for (i32 i = 0; i < 28; ++i) {
        SW(mobility_queen[i], w, MG, EG);
    }

    for (i32 i = 0; i < 9; ++i) {
        SW(king_defense[i], w, MG, EG);
    }

    w.tempo = 20;

    return w;
} ();

i32 get(Board& board);

i32 get_material(Board& board);

i32 get_table(Board& board);

i32 get_mobility(Board& board);

i32 get_king_defense(Board& board);

i32 get_scale(Board& board, i32 eval);

bool is_see(Board& board, u16 move, i32 threshold);

};

namespace eval::test
{

inline void see()
{
    struct Test
    {
        std::string fen;
        u16 move;
        i32 value;
        bool result;
    };

    std::vector<Test> tests = {
        Test { .fen = "k6b/8/8/8/8/8/1p6/BK6 w - - 0 1", .move = move::get_make(square::A1, square::B2), .value = SEE_VALUE[piece::type::PAWN], .result = true },
        Test { .fen = "k6b/8/8/8/8/2p5/1p6/BK6 w - - 0 1", .move = move::get_make(square::A1, square::B2), .value = SEE_VALUE[piece::type::PAWN], .result = false },
        Test { .fen = "k7/8/8/8/8/2p5/1p6/BK6 w - - 0 1", .move = move::get_make(square::A1, square::B2), .value = SEE_VALUE[piece::type::PAWN], .result = false },
        Test { .fen = "k7/8/8/8/8/2q5/1p6/BK6 w - - 0 1", .move = move::get_make(square::A1, square::B2), .value = SEE_VALUE[piece::type::PAWN], .result = true },
        Test { .fen = "k6b/8/8/8/8/2q5/1p6/BK6 w - - 0 1", .move = move::get_make(square::A1, square::B2), .value = SEE_VALUE[piece::type::PAWN], .result = false },
        Test { .fen = "rn2k2r/p3bpp1/2p4p/8/2P3Q1/1P1q4/P4P1P/RNB1K2R w KQkq - 0 8", .move = move::get_make(square::G4, square::G7), .value = 0, .result = true },
        Test { .fen = "r1bq1rk1/pppp1Npp/2nb1n2/4p3/2B1P3/2P5/PP1P1PPP/RNBQK2R b KQ - 0 6", .move = move::get_make(square::F8, square::F7), .value = 0, .result = true },
        Test { .fen = "r1bqkb1r/ppp1pppp/2n2n2/8/2BPP3/5P2/PP4PP/RNBQK1NR b KQkq - 0 5", .move = move::get_make(square::C6, square::D4), .value = 0, .result = true },
        Test { .fen = "3b2k1/1b6/8/3R2p1/4K3/5N2/8/8 w - - 0 1", .move = move::get_make(square::F3, square::G5), .value = 0, .result = false },
        Test { .fen = "5k2/1b6/8/3B4/4K3/8/8/8 w - - 0 1", .move = move::get_make(square::D5, square::B7), .value = 0, .result = true },
    };

    i32 index = 0;

    for (auto& test : tests) {
        auto board = Board(test.fen);

        std::cout << "test #" << index << ":" << std::endl;
        std::cout << " - " << test.fen << " " << move::get_str(test.move) << std::endl;
        std::cout << " - " << "result: " << eval::is_see(board, test.move, test.value) << std::endl;
        std::cout << " - " << "expect: " << test.result << std::endl;
        std::cout << std::endl;

        index += 1;
    }
};

};