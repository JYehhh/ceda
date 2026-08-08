// bitboard.hpp
#pragma once

#include "types.hpp"
#include <utility>

// Precomputed for all 64 squares
extern Bitboard KNIGHT_ATTACKS[64];
extern Bitboard KING_ATTACKS[64];
extern Bitboard PAWN_ATTACKS[2][64];  // [color][square]

// How many pieces are on this bitboard?
int popcount(Bitboard b);

// Index of the lowest set bit (e.g. "what's the first square in this set?")
Square lsb(Bitboard b);

// Clear the lowest set bit (i.e. "consumed" that square, move to next)
Bitboard pop_lsb(Bitboard& b);

// Check if a specific row and column is within the bounds of a board
static bool on_board(int r, int c); 

bool apply_dir(Square &sq, std::pair<int, int> d);

void init();
