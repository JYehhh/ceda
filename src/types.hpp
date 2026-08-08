// types.hpp
#pragma once
#include <cstdint>
#include <utility>

using pii = std::pair<int, int>;

using Bitboard  = uint64_t;
using Move      = uint16_t;
/*
Move has:
    - From:     num 0 - 63 (6 bits)
    - To:       num 0 - 63 (6 bits)
    - Flags:    num 0 - 15 (4 bits)
bit index: 15 14 13 12 | 11 10  9  8  7  6 |  5  4  3  2  1  0
           [  flags   ] [       to         ] [      from       ]

0	QUIET	                Normal move, no capture
1	DOUBLE_PUSH	            Pawn moves two squares
2	CASTLE_KING	            Kingside castling
3	CASTLE_QUEEN	        Queenside castling
4	CAPTURE	                Normal capture
5	EN_PASSANT	            En passant capture
8	PROMO_KNIGHT	        Promote to knight
9	PROMO_BISHOP	        Promote to bishop
10	PROMO_ROOK	            Promote to rook
11	PROMO_QUEEN	            Promote to queen
12	PROMO_CAPTURE_KNIGHT	Capture + promote to knight
13	PROMO_CAPTURE_BISHOP	Capture + promote to bishop
14	PROMO_CAPTURE_ROOK	    Capture + promote to rook
15	PROMO_CAPTURE_QUEEN	    Capture + promote to queen
*/

enum Colour : int { WHITE, BLACK };
enum Piece  : int { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NO_PIECE };
enum Square : int {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    NONE
};

constexpr int N_COLOURS = 2;
constexpr int N_PIECES = 6;
constexpr int N_SQUARES = 64;
