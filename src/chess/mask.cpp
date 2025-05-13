#include "mask.h"

namespace mask
{

alignas(64) u64 BITS_BETWEEN[64][64];

void init()
{
    // Bits between squares masks
    for (i8 square_1 = 0; square_1 < 64; ++square_1) {
        for (i8 square_2 = 0; square_2 < 64; ++square_2) {
            u64 squares = bitboard::create(square_1) | bitboard::create(square_2);

            mask::BITS_BETWEEN[square_1][square_2] = 0ULL;

            if (square_1 == square_2) {
                continue;
            }

            if (square::get_file(square_1) == square::get_file(square_2) || square::get_rank(square_1) == square::get_rank(square_2)) {
                mask::BITS_BETWEEN[square_1][square_2] = attack::get_rook(square_1, squares) & attack::get_rook(square_2, squares);
            }
            else if (square::get_distance_file(square_1, square_2) == square::get_distance_rank(square_1, square_2)) {
                mask::BITS_BETWEEN[square_1][square_2] = attack::get_bishop(square_1, squares) & attack::get_bishop(square_2, squares);
            }
        }
    }
};

u64 get_bits_between(i8 square_1, i8 square_2)
{
    assert(square::is_valid(square_1));
    assert(square::is_valid(square_2));

    return mask::BITS_BETWEEN[square_1][square_2];
};

};