#include "attack.h"

namespace attack
{

alignas(64) const u64 PAWN[2][64] =
{
    {
        0x0000000000000200ULL, 0x0000000000000500ULL, 0x0000000000000A00ULL, 0x0000000000001400ULL,
        0x0000000000002800ULL, 0x0000000000005000ULL, 0x000000000000A000ULL, 0x0000000000004000ULL,
        0x0000000000020000ULL, 0x0000000000050000ULL, 0x00000000000A0000ULL, 0x0000000000140000ULL,
        0x0000000000280000ULL, 0x0000000000500000ULL, 0x0000000000A00000ULL, 0x0000000000400000ULL,
        0x0000000002000000ULL, 0x0000000005000000ULL, 0x000000000A000000ULL, 0x0000000014000000ULL,
        0x0000000028000000ULL, 0x0000000050000000ULL, 0x00000000A0000000ULL, 0x0000000040000000ULL,
        0x0000000200000000ULL, 0x0000000500000000ULL, 0x0000000A00000000ULL, 0x0000001400000000ULL,
        0x0000002800000000ULL, 0x0000005000000000ULL, 0x000000A000000000ULL, 0x0000004000000000ULL,
        0x0000020000000000ULL, 0x0000050000000000ULL, 0x00000A0000000000ULL, 0x0000140000000000ULL,
        0x0000280000000000ULL, 0x0000500000000000ULL, 0x0000A00000000000ULL, 0x0000400000000000ULL,
        0x0002000000000000ULL, 0x0005000000000000ULL, 0x000A000000000000ULL, 0x0014000000000000ULL,
        0x0028000000000000ULL, 0x0050000000000000ULL, 0x00A0000000000000ULL, 0x0040000000000000ULL,
        0x0200000000000000ULL, 0x0500000000000000ULL, 0x0A00000000000000ULL, 0x1400000000000000ULL,
        0x2800000000000000ULL, 0x5000000000000000ULL, 0xA000000000000000ULL, 0x4000000000000000ULL,
        0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL,
        0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL
    },
    {
        0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL,
        0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL,
        0x0000000000000002ULL, 0x0000000000000005ULL, 0x000000000000000AULL, 0x0000000000000014ULL,
        0x0000000000000028ULL, 0x0000000000000050ULL, 0x00000000000000A0ULL, 0x0000000000000040ULL,
        0x0000000000000200ULL, 0x0000000000000500ULL, 0x0000000000000A00ULL, 0x0000000000001400ULL,
        0x0000000000002800ULL, 0x0000000000005000ULL, 0x000000000000A000ULL, 0x0000000000004000ULL,
        0x0000000000020000ULL, 0x0000000000050000ULL, 0x00000000000A0000ULL, 0x0000000000140000ULL,
        0x0000000000280000ULL, 0x0000000000500000ULL, 0x0000000000A00000ULL, 0x0000000000400000ULL,
        0x0000000002000000ULL, 0x0000000005000000ULL, 0x000000000A000000ULL, 0x0000000014000000ULL,
        0x0000000028000000ULL, 0x0000000050000000ULL, 0x00000000A0000000ULL, 0x0000000040000000ULL,
        0x0000000200000000ULL, 0x0000000500000000ULL, 0x0000000A00000000ULL, 0x0000001400000000ULL,
        0x0000002800000000ULL, 0x0000005000000000ULL, 0x000000A000000000ULL, 0x0000004000000000ULL,
        0x0000020000000000ULL, 0x0000050000000000ULL, 0x00000A0000000000ULL, 0x0000140000000000ULL,
        0x0000280000000000ULL, 0x0000500000000000ULL, 0x0000A00000000000ULL, 0x0000400000000000ULL,
        0x0002000000000000ULL, 0x0005000000000000ULL, 0x000A000000000000ULL, 0x0014000000000000ULL,
        0x0028000000000000ULL, 0x0050000000000000ULL, 0x00A0000000000000ULL, 0x0040000000000000ULL
    }
};

alignas(64) const u64 KNIGHT[64] =
{
    0x0000000000020400ULL, 0x0000000000050800ULL, 0x00000000000A1100ULL, 0x0000000000142200ULL,
    0x0000000000284400ULL, 0x0000000000508800ULL, 0x0000000000A01000ULL, 0x0000000000402000ULL,
    0x0000000002040004ULL, 0x0000000005080008ULL, 0x000000000A110011ULL, 0x0000000014220022ULL,
    0x0000000028440044ULL, 0x0000000050880088ULL, 0x00000000A0100010ULL, 0x0000000040200020ULL,
    0x0000000204000402ULL, 0x0000000508000805ULL, 0x0000000A1100110AULL, 0x0000001422002214ULL,
    0x0000002844004428ULL, 0x0000005088008850ULL, 0x000000A0100010A0ULL, 0x0000004020002040ULL,
    0x0000020400040200ULL, 0x0000050800080500ULL, 0x00000A1100110A00ULL, 0x0000142200221400ULL,
    0x0000284400442800ULL, 0x0000508800885000ULL, 0x0000A0100010A000ULL, 0x0000402000204000ULL,
    0x0002040004020000ULL, 0x0005080008050000ULL, 0x000A1100110A0000ULL, 0x0014220022140000ULL,
    0x0028440044280000ULL, 0x0050880088500000ULL, 0x00A0100010A00000ULL, 0x0040200020400000ULL,
    0x0204000402000000ULL, 0x0508000805000000ULL, 0x0A1100110A000000ULL, 0x1422002214000000ULL,
    0x2844004428000000ULL, 0x5088008850000000ULL, 0xA0100010A0000000ULL, 0x4020002040000000ULL,
    0x0400040200000000ULL, 0x0800080500000000ULL, 0x1100110A00000000ULL, 0x2200221400000000ULL,
    0x4400442800000000ULL, 0x8800885000000000ULL, 0x100010A000000000ULL, 0x2000204000000000ULL,
    0x0004020000000000ULL, 0x0008050000000000ULL, 0x00110A0000000000ULL, 0x0022140000000000ULL,
    0x0044280000000000ULL, 0x0088500000000000ULL, 0x0010A00000000000ULL, 0x0020400000000000ULL
};

alignas(64) const u64 KING[64] =
{
    0x0000000000000302ULL, 0x0000000000000705ULL, 0x0000000000000E0AULL, 0x0000000000001C14ULL,
    0x0000000000003828ULL, 0x0000000000007050ULL, 0x000000000000E0A0ULL, 0x000000000000C040ULL,
    0x0000000000030203ULL, 0x0000000000070507ULL, 0x00000000000E0A0EULL, 0x00000000001C141CULL,
    0x0000000000382838ULL, 0x0000000000705070ULL, 0x0000000000E0A0E0ULL, 0x0000000000C040C0ULL,
    0x0000000003020300ULL, 0x0000000007050700ULL, 0x000000000E0A0E00ULL, 0x000000001C141C00ULL,
    0x0000000038283800ULL, 0x0000000070507000ULL, 0x00000000E0A0E000ULL, 0x00000000C040C000ULL,
    0x0000000302030000ULL, 0x0000000705070000ULL, 0x0000000E0A0E0000ULL, 0x0000001C141C0000ULL,
    0x0000003828380000ULL, 0x0000007050700000ULL, 0x000000E0A0E00000ULL, 0x000000C040C00000ULL,
    0x0000030203000000ULL, 0x0000070507000000ULL, 0x00000E0A0E000000ULL, 0x00001C141C000000ULL,
    0x0000382838000000ULL, 0x0000705070000000ULL, 0x0000E0A0E0000000ULL, 0x0000C040C0000000ULL,
    0x0003020300000000ULL, 0x0007050700000000ULL, 0x000E0A0E00000000ULL, 0x001C141C00000000ULL,
    0x0038283800000000ULL, 0x0070507000000000ULL, 0x00E0A0E000000000ULL, 0x00C040C000000000ULL,
    0x0302030000000000ULL, 0x0705070000000000ULL, 0x0E0A0E0000000000ULL, 0x1C141C0000000000ULL,
    0x3828380000000000ULL, 0x7050700000000000ULL, 0xE0A0E00000000000ULL, 0xC040C00000000000ULL,
    0x0203000000000000ULL, 0x0507000000000000ULL, 0x0A0E000000000000ULL, 0x141C000000000000ULL,
    0x2838000000000000ULL, 0x5070000000000000ULL, 0xA0E0000000000000ULL, 0x40C0000000000000ULL
};

alignas(64) u64 BISHOP[0x1480];
alignas(64) u64 ROOK[0x19000];

alignas(64) magic::Entry TABLE_BISHOP[64];
alignas(64) magic::Entry TABLE_ROOK[64];

u64 get_sliders(i8 square, u64 occupied, const i8 delta[4][2])
{
    u64 result = 0ULL;

    for (auto i = 0; i < 4; ++i) {
        i8 rank = square::get_rank(square);
        i8 file = square::get_file(square);

        while (true)
        {
            rank += delta[i][0];
            file += delta[i][1];

            if (!rank::is_valid(rank) || !file::is_valid(file)) {
                break;
            }

            i8 sq = square::create(file, rank);

            result = bitboard::get_set_bit(result, sq);

            if (bitboard::is_set(occupied, sq)) {
                break;
            }
        }
    }

    return result;
};

void init_sliders(i8 square, magic::Entry table[], u64 magic, const i8 delta[4][2])
{
    u64 edges =
        ((bitboard::RANK_1 | bitboard::RANK_8) & ~bitboard::RANK[square::get_rank(square)]) |
        ((bitboard::FILE_A | bitboard::FILE_H) & ~bitboard::FILE[square::get_file(square)]);

    u64 occupied = 0ULL;

    table[square].magic = magic;
    table[square].mask = attack::get_sliders(square, occupied, delta) & ~edges;
    table[square].shift = 64 - bitboard::get_count(table[square].mask);

    if (square < 63) {
        table[square + 1].data = table[square].data + (1ULL << bitboard::get_count(table[square].mask));
    }

    do {
        table[square].data[magic::get_index(table[square], occupied)] = attack::get_sliders(square, occupied, delta);
        occupied = (occupied - table[square].mask) & table[square].mask;
    }
    while (occupied);
};

void init()
{
    const i8 DELTA_BISHOP[4][2] = {{ -1, -1 }, { -1, 1 }, { 1, -1 }, { 1, 1 }};
    const i8 DELTA_ROOK[4][2] = {{ -1, 0 }, { 0, -1 }, { 0, 1 }, { 1, 0 }};

    TABLE_BISHOP[0].data = BISHOP;
    TABLE_ROOK[0].data = ROOK;

    for (i8 square = 0; square < 64; ++square) {
        attack::init_sliders(square, TABLE_BISHOP, magic::BISHOP[square], DELTA_BISHOP);
        attack::init_sliders(square, TABLE_ROOK, magic::ROOK[square], DELTA_ROOK);
    }
};

u64 get_pawn(i8 square, i8 color)
{
    assert(square::is_valid(square));
    assert(color::is_valid(color));

    return attack::PAWN[color][square];
};

u64 get_knight(i8 square)
{
    assert(square::is_valid(square));

    return attack::KNIGHT[square];
};

u64 get_bishop(i8 square, u64 occupied)
{
    assert(square::is_valid(square));

    return attack::TABLE_BISHOP[square].data[magic::get_index(attack::TABLE_BISHOP[square], occupied)];
};

u64 get_rook(i8 square, u64 occupied)
{
    assert(square::is_valid(square));

    return attack::TABLE_ROOK[square].data[magic::get_index(attack::TABLE_ROOK[square], occupied)];
};

u64 get_queen(i8 square, u64 occupied)
{
    assert(square::is_valid(square));

    return attack::get_bishop(square, occupied) | attack::get_rook(square, occupied);
};

u64 get_king(i8 square)
{
    assert(square::is_valid(square));

    return attack::KING[square];
};

};