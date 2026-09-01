#include "position.hpp"
#include "bitboard.hpp"
#include <bit>
#include <vector>
#include <span>
#include <utility>

// Returns the bitboard for a given piece type and color.
// e.g. get_pieces(WHITE, ROOK) gives you a bitboard with bits set on every square a white rook occupies.
Bitboard Position::get_pieces(Colour c, Piece p) const {
    return pieces[c][p];
}

// Returns a bitboard of all squares occupied by pieces of color c.
// This is the union of all 6 piece bitboards for that color.
Bitboard Position::get_occupied_by(Colour c) const {
    return occupied_by[c];
}

// Returns a bitboard of every occupied square on the board regardless of color.
Bitboard Position::get_occupied() const {
    return occupied;
}

// Returns the piece type on a given square using the mailbox array.
// Returns a sentinel (e.g. NO_PIECE) if the square is empty.
Piece Position::piece_on(Square sq) const {
    return mailbox[sq];
}

// Returns whose turn it is to move — WHITE or BLACK.
Colour Position::get_side_to_move() const {
    return side_to_move;
}

// Returns the en passant target square, or NONE if en passant is not available.
// This is the square a pawn passed through on a double push last move.
Square Position::get_ep_square() const {
    return ep_square;
}

// Returns the castling rights bitmask.
// Bits: K=1 (white kingside), Q=2 (white queenside), k=4 (black kingside), q=8 (black queenside).
// A bit being set means that castle is still legally available.
int Position::get_castling_rights() const {
    return castling_rights;
}

// Returns true if the king of color c is currently in check.
// Implemented by checking whether the king's square is attacked by the opponent.
bool Position::is_in_check(Colour c) const {
    // find the king
    // then check is attached
    Bitboard k = pieces[c][Piece::KING];
    Square sq = static_cast<Square>(std::countr_zero(k));
    if (c == Colour::BLACK) return is_attacked(sq, Colour::WHITE); 
    // when we specify an enum here, are we making a temporary object?
    else return is_attacked(sq, Colour::BLACK); 
}

bool Position::ray_scan_attacked(Square sq, Colour by, Piece p, std::span<pii> threat_dirs) const {
    Colour my = ~by;
    for (const pii d : threat_dirs) {
        Square cur = sq;
        while (true) {
            if (!apply_dir(cur, d)) break;
            if ((occupied_by[my] & cur) == cur) break; // or be blocked by a piece of colour my
            if ((occupied_by[by] & cur) == cur) {
                if (mailbox[cur] == p) return true;
                else break;
            }
        }
    }

    return false;
}

bool Position::attack_tables_attacked(Square sq, Colour by, Piece p, Bitboard bb) const {
    while (bb) {
        int l = lsb(bb);
        if ((l & pieces[by][p]) == l) return true;
        pop_lsb(bb);
    }
    return false;
}

// Returns true if the given square is attacked by any piece of color `by`.
// Used for check detection, castling legality ("can't castle through check"), and legality filtering.
bool Position::is_attacked(Square sq, Colour by) const {
    Colour my = ~by;
    Bitboard kna = KNIGHT_ATTACKS[sq];
    Bitboard kia = KING_ATTACKS[sq];
    Bitboard pa = PAWN_ATTACKS[my][sq];

    std::vector<pii> rd = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };
    std::vector<pii> bd = { {1, 1}, {1, -1}, {-1, 1}, {-1, -1} };
    std::vector<pii> qd;
    qd.reserve(8);
    qd.insert(qd.end(), rd.begin(), rd.end());
    qd.insert(qd.end(), bd.begin(), bd.end());
    
    if (ray_scan_attacked(sq, by, Piece::ROOK, rd)) return true;
    if (ray_scan_attacked(sq, by, Piece::BISHOP, bd)) return true;
    if (ray_scan_attacked(sq, by, Piece::QUEEN, qd)) return true;
    if (attack_tables_attacked(sq, by, Piece::KNIGHT, kna)) return true;
    if (attack_tables_attacked(sq, by, Piece::KING, kia)) return true;
    if (attack_tables_attacked(sq, by, Piece::PAWN, pa)) return true;
    return false;
}

void Position::move_piece(int from, int to, bool capture) {
    Piece p = mailbox[from];
    Piece to_p = mailbox[to];
    // mailbox
    mailbox[from] = NO_PIECE;
    mailbox[to] = p;
    // Pieces
    pieces[side_to_move][p] ^= (1 << from);
    pieces[side_to_move][p] |= (1 << to);
    // Occupied By
    occupied_by[side_to_move] ^= (1 << from);
    occupied_by[side_to_move] |= (1 << to);
    // occupied
    occupied ^= (1 << from);
    occupied |= (1 << to);
    
    if (capture) {
        Colour other = static_cast<Colour>(side_to_move ^ 1);
        // remove the piece from opponent's tables
        pieces[other][to_p] ^= (1 << to);
        occupied_by[other] ^= (1 << to);
    }
    // update castling rights if applicable
    if (p == Piece::KING) {
        // if white moved, then we need to remove white's castling rights
        if (side_to_move == Colour::BLACK) castling_rights &= (3 << 2); 
        // if black moved, remove black's castling rights
        if (side_to_move == Colour::WHITE) castling_rights &= 3;
    }
}

// Applies the move to the board, updating:
//   - the bitboards and mailbox for the moving piece (and captured piece if any)
//   - castling rights (revoke if king or rook moves)
//   - ep_square (set if double pawn push, clear otherwise)
//   - halfmove clock (reset on capture or pawn move, increment otherwise)
//   - fullmove number (increment after black's move)
//   - side_to_move (flip)
// Must handle special cases: captures, en passant, castling, promotion.
void Position::make_move(Move m) {
    MoveFlag mf = static_cast<MoveFlag>(m >> 12);
    int to = (m & (63 << 6)) >> 6;
    int from = m & 63;
    Piece mp = mailbox[from];
    // ALWAYS MOVE
    move_piece(from, to, mf == CAPTURE_BIT);
    // EN PASSANTE
    if (mf == EN_PASSANT) {
        Piece take = mailbox[ep_square]; 
        occupied ^= (1 << ep_square);
        occupied_by[side_to_move ^ 1] ^= (1 << ep_square);
        pieces[side_to_move ^ 1][take] ^= (1 << ep_square);
    } 
    // CASTLE
    if (mf == CASTLE_KING) {
        move_piece()   // move rook to the 
    }
    if (mf == CASTLE_QUEEN) {

    }
    // PROMOTION
    //
    // EVERY SINGLE MOVE
    side_to_move = static_cast<Colour>(side_to_move ^ 1);
    fullmove_number++;
    if (mp == Piece::PAWN || mf == CAPTURE_BIT) halfmove_clock = 0;
    else halfmove_clock += 1;
}

// Reverses make_move exactly, restoring the position to its prior state.
// Must restore ALL state that make_move changed: bitboards, mailbox, castling rights,
// ep_square, halfmove clock, fullmove number, side_to_move.
// Tip: push irreversible state onto an undo stack in make_move and pop it here.
void Position::unmake_move(Move m) {}

// Parses a FEN string and sets up the position.
// FEN format: "<pieces> <side> <castling> <ep> <halfmove> <fullmove>"
// e.g. "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
// Must set all private members: bitboards, mailbox, side_to_move,
// castling_rights, ep_square, halfmove_clock, fullmove_number.
void Position::set_from_fen(const char* fen) {}

// Generates a FEN string from the current position state.
// Inverse of set_from_fen. Useful for debugging and testing (FEN round-trip test).
const char* Position::to_fen() const {}

// Prints the board to stdout in a human-readable format for debugging.
// Should show piece characters on their squares, and ideally side to move,
// castling rights, and ep square below the board.
void Position::print() const {}
