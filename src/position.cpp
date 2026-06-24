#include "position.hpp"

// Returns the bitboard for a given piece type and color.
// e.g. get_pieces(WHITE, ROOK) gives you a bitboard with bits set on every square a white rook occupies.
Bitboard Position::get_pieces(Colour c, Piece p) const {}

// Returns a bitboard of all squares occupied by pieces of color c.
// This is the union of all 6 piece bitboards for that color.
Bitboard Position::get_occupied_by(Colour c) const {}

// Returns a bitboard of every occupied square on the board regardless of color.
Bitboard Position::get_occupied() const {}

// Returns the piece type on a given square using the mailbox array.
// Returns a sentinel (e.g. NO_PIECE) if the square is empty.
Piece Position::piece_on(Square sq) const {}

// Returns whose turn it is to move — WHITE or BLACK.
Colour Position::get_side_to_move() const {}

// Returns the en passant target square, or NONE if en passant is not available.
// This is the square a pawn passed through on a double push last move.
Square Position::get_ep_square() const {}

// Returns the castling rights bitmask.
// Bits: K=1 (white kingside), Q=2 (white queenside), k=4 (black kingside), q=8 (black queenside).
// A bit being set means that castle is still legally available.
int Position::get_castling_rights() const {}

// Returns true if the king of color c is currently in check.
// Implemented by checking whether the king's square is attacked by the opponent.
bool Position::is_in_check(Colour c) const {}

// Returns true if the given square is attacked by any piece of color `by`.
// Used for check detection, castling legality ("can't castle through check"), and legality filtering.
bool Position::is_attacked(Square sq, Colour by) const {}

// Applies the move to the board, updating:
//   - the bitboards and mailbox for the moving piece (and captured piece if any)
//   - castling rights (revoke if king or rook moves)
//   - ep_square (set if double pawn push, clear otherwise)
//   - halfmove clock (reset on capture or pawn move, increment otherwise)
//   - fullmove number (increment after black's move)
//   - side_to_move (flip)
// Must handle special cases: captures, en passant, castling, promotion.
void Position::make_move(Move m) {}

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