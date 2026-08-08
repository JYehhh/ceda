#include "bitboard.hpp"
#include <bit>
#include <utility>
#include <span>

Bitboard KNIGHT_ATTACKS[64];
Bitboard KING_ATTACKS[64];
Bitboard PAWN_ATTACKS[2][64];  // [color][square]

using pii = std::pair<int, int>;

// How many pieces are on this bitboard?
int popcount(Bitboard b) {
	return std::popcount(b);	
}

// Index of the lowest set bit (e.g. "what's the first square in this set?")
Square lsb(Bitboard b) {
	return Square(std::countr_zero(b));
}

// Clear the lowest set bit (i.e. "consumed" that square, move to next)
Bitboard pop_lsb(Bitboard& b) {
	b = b ^ (b & -b);
	return b;	
}

// Check if a specific row and column is within the bounds of a board
static bool on_board(int r, int c) {
	return r < 0 || r > 7 || c < 0 || c > 7;
}

// Apply a direction to a square
bool apply_dir(Square &sq, std::pair<int, int> d) {
	auto r = sq / 8;
	auto c = sq % 8;
	int new_r = r + d.first;
	int new_c = c + d.second;
    if (on_board(new_r, new_c)) return false;
    sq = static_cast<Square>((new_r * 8) + new_c);
    return true;
}

void fill_table(std::span<Bitboard, 64> table, std::span<pii> dirs) {
    for (size_t i = 0; i < 64; ++i) { // for every position on the board 
        for (const auto &d : dirs) { // for every direction from this position
            Square move = static_cast<Square>(i);
            if (apply_dir(move, d)) table[i] |= 1ULL << move;
        }
    }
}

void init() {
	// KNIGHT MOVES
	pii knd[8] = { {2, 1}, {-2, 1}, {2, -1}, {-2, -1}, {1, 2}, {-1, 2}, {1, -2}, {-1, -2} };
	pii kid[8] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1} };
    pii pwd[2] = { {1, -1}, {1, 1} };
    pii pbd[2] = { {-1, -1}, {-1, 1} };
    fill_table(KNIGHT_ATTACKS, knd);
    fill_table(KING_ATTACKS, kid); 
    fill_table(PAWN_ATTACKS[Colour::WHITE], pwd);
    fill_table(PAWN_ATTACKS[Colour::BLACK], pbd);
}
