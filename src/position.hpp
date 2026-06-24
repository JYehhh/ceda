// position.hpp
#pragma once

#include "types.hpp"

class Position {
public:
    // queries
    Bitboard get_pieces(Colour c, Piece p) const;
    Bitboard get_occupied_by(Colour c) const;
    Bitboard get_occupied() const;
    Piece    piece_on(Square sq) const;
    Colour   get_side_to_move() const;
    Square   get_ep_square() const;
    int      get_castling_rights() const;
    bool     is_in_check(Colour c) const;
    bool     is_attacked(Square sq, Colour by) const;

    // mutation
    void make_move(Move m);
    void unmake_move(Move m);

    // setup / debug
    void        set_from_fen(const char* fen);
    const char* to_fen() const;
    void        print() const;

private:
    Bitboard pieces[N_COLOURS][N_PIECES];
    Bitboard occupied_by[N_COLOURS];
    Bitboard occupied;
    int      mailbox[N_SQUARES];
    Square   ep_square;
    int      castling_rights;
    Colour   side_to_move;
    int      halfmove_clock;
    int      fullmove_number;
};