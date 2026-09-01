// position.hpp
#pragma once

#include "types.hpp"
#include <span>

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
    bool     ray_scan_attacked(Square sq, Colour by, Piece p, std::span<pii> threat_dirs) const; 
    bool     attack_tables_attacked(Square sq, Colour by, Piece p, Bitboard bb) const;

    // mutation
    void move_piece(int from, int to, bool capture);
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
    Piece    mailbox[N_SQUARES];
    Square   ep_square;
    int      castling_rights; // KQkq
    Colour   side_to_move;
    int      halfmove_clock;
    int      fullmove_number;
};
