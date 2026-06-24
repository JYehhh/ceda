// bitboard.hpp
#pragma once

#include "types.hpp"

// Precomputed for all 64 squares
extern Bitboard KNIGHT_ATTACKS[64];
extern Bitboard KING_ATTACKS[64];
extern Bitboard PAWN_ATTACKS[2][64];  // [color][square]

// How many pieces are on this bitboard?
inline int popcount(Bitboard b);

// Index of the lowest set bit (e.g. "what's the first square in this set?")
inline Square lsb(Bitboard b);

// Clear the lowest set bit (i.e. "consumed" that square, move to next)
inline Bitboard pop_lsb(Bitboard& b);
