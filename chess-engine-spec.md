# Chess Engine in C++ — Project Spec & Briefer

A high-level map of what you're building, the components involved, the order to build them in, and how to know each piece works. Written for someone comfortable programming but newish to C++. By the end you'll have a program that plays real chess, talks to chess GUIs, and can be measured against itself and other engines.

---

## 1. The mental model

A chess engine is, at its core, three things wired together:

1. **A board it can manipulate** — represent a position, generate the legal moves, make and undo moves.
2. **A search** — explore the tree of possible move sequences and pick the best line.
3. **An evaluation** — a function that scores a "leaf" position numerically (who's winning, by how much).

Search is the brain; evaluation is its sense of taste; the board is the world it acts on. Wrapped around all of it is a thin **protocol layer (UCI)** so external chess GUIs can drive your engine and pit it against others.

A useful frame: the engine spends ~99% of its runtime in a tight loop of *generate moves → make move → evaluate → unmake move*, called millions of times per second. Almost every design decision is downstream of "this code is on a hot path." That's also exactly why this is such a strong C++ project — it forces you to think about bit manipulation, cache locality, allocation-free hot loops, and profiling. It's the same performance-engineering muscle that low-latency systems work demands, so it's a genuinely high-leverage build for where you're heading.

---

## 2. Components (what's in the box)

### Board representation
How you store a position in memory. Two main schools:

- **Mailbox** — an array of squares (often a padded 10×12 grid so off-board moves are detected by sentinel values). Simple, readable, easy to debug. Slower.
- **Bitboards** — a position is a set of 64-bit integers (`uint64_t`), one bit per square. You keep one bitboard per piece type per color (12 total) plus derived occupancy boards. Set operations become single CPU instructions (AND/OR/XOR/shift). This is what every serious engine uses.

**Recommendation:** go bitboards, because the interesting C++ learning lives there and it's what you'd want on a résumé project. To avoid getting stuck early, implement sliding-piece moves with simple ray-scanning *first*, get everything correct, then swap in magic bitboards (below) as a pure optimization later.

### Move representation
A move packed into a single integer (16 bits suffices): `from` square (6 bits), `to` square (6 bits), and flags (4 bits) for promotion piece, en passant, castling, capture. Packing into an int keeps moves small and cache-friendly versus a fat struct.

### Move generation
Given a position, produce all moves. Two styles:

- **Pseudo-legal then filter** — generate everything that looks like a move ignoring whether it leaves your own king in check, then discard the illegal ones by testing king safety. Simpler; start here.
- **Fully legal** — generate only legal moves directly (requires pin/check-evasion logic). Faster, more complex; do it later if at all.

Per piece type:
- **Knights / kings** — precomputed lookup tables (one bitboard of destinations per square). Trivial.
- **Pawns** — the fiddly one: single push, double push, diagonal captures, en passant, promotion. Most early bugs live here.
- **Sliding pieces (bishop/rook/queen)** — the hard one. Ray-scan first; later use **magic bitboards** (precomputed attack tables indexed by a multiply-and-shift hash of the blocker occupancy). Magic bitboards are a rite of passage and a great thing to be able to explain.
- **Castling** — special-case the two king/rook moves plus the "can't castle through/into check" rules.

### Make / unmake move
Apply a move to the board and be able to reverse it.
- **Make-unmake** (incremental): mutate the board, push the irreversible state (captured piece, castling rights, en passant square, halfmove clock, position hash) onto an undo stack, and restore on unmake. Fast. Bug-prone.
- **Copy-make**: copy the whole board, apply the move to the copy. Slower but nearly bug-free. Fine to start with; many strong engines stayed copy-make for a long time.

**Invariant to enforce in tests:** make-then-unmake must leave the board *byte-for-byte identical*. Assert it.

### Zobrist hashing
Assign a random 64-bit number to every (piece, square) combination (plus side-to-move, castling rights, en passant file). XOR them together to get a 64-bit fingerprint of a position, updatable incrementally as you make/unmake moves. Used for the transposition table and for detecting repetition draws.

### Search
The decision procedure. Built up in layers:
- **Negamax + alpha-beta pruning** — the core tree search. Negamax is a tidy reformulation of minimax for zero-sum games; alpha-beta prunes branches that can't affect the result.
- **Iterative deepening** — search depth 1, then 2, then 3… Lets you stop anytime with a valid move, and (crucially) feeds move-ordering data from the previous depth into the next.
- **Move ordering** — alpha-beta's efficiency lives or dies here. Try the best moves first: hash move from the transposition table, then captures ranked by **MVV-LVA** (most valuable victim, least valuable attacker), then **killer moves** and the **history heuristic**. Good ordering can cut the tree by orders of magnitude.
- **Quiescence search** — at the leaves, don't stop mid-trade. Keep searching *only captures* until the position is "quiet," so you don't evaluate halfway through a capture sequence (the "horizon effect").
- **Transposition table** — a big hash table keyed by Zobrist hash that caches the result of searching a position, so transpositions (same position reached different ways) aren't re-searched.
- **Later pruning/reductions** — null-move pruning, late move reductions (LMR), principal variation search (PVS), aspiration windows. These are strength multipliers you layer on once the basics are solid.

### Evaluation
Scores a quiet position in **centipawns** (100 = one pawn advantage).
- **Material** — sum piece values. This alone plays recognizable chess.
- **Piece-square tables** — per-piece, per-square bonuses (knights like the center, pawns like advancing, king likes safety in the middlegame and activity in the endgame).
- **Tapered eval** — interpolate between middlegame and endgame scores based on remaining material, so the king's "preferred" squares shift smoothly.
- **Structure terms** — mobility, pawn structure (doubled/isolated/passed), king safety, rooks on open files, bishop pair.
- **NNUE** (efficiently updatable neural network) — what top engines use today. Powerful but advanced; a much later milestone, if ever.

### UCI protocol layer
**Universal Chess Interface** — a simple text protocol over stdin/stdout that lets GUIs (Cute Chess, Arena, Banksia) and tournament tools drive your engine. You parse commands (`uci`, `isready`, `position`, `go`, `stop`, `quit`) and reply (`id`, `uciok`, `readyok`, `info ...`, `bestmove ...`). This is what turns your code from a toy into something you can actually play and benchmark.

### FEN parsing
**Forsyth–Edwards Notation** is the standard one-line string for a position. You need to parse it (to set up arbitrary positions, including for tests) and ideally generate it. Small but essential plumbing.

---

## 3. Build sequencing

Each phase ends at a checkpoint where you can *prove* it works before moving on. Resist the urge to skip the gate at Phase 1 — a move-generation bug that surfaces three phases later is the single most painful debugging experience in this whole project.

**Phase 0 — Foundations.**
Board representation, FEN parsing, print a board to the console. Outcome: you can load any position and look at it.

**Phase 1 — Move generation (the make-or-break phase).**
Move encoding, make/unmake, pseudo-legal generation (knights/king/pawns, then sliding via ray-scan), legality filtering.
*Gate: perft (see Testing). Do not proceed until perft matches reference values exactly from multiple test positions.*

**Phase 2 — Minimal brain.**
Material-only evaluation + negamax with alpha-beta. Outcome: it plays full, legal, vaguely sensible games against itself in code.

**Phase 3 — UCI.**
Enough of the protocol to load it into a GUI and click pieces, or have it play itself. Outcome: it's a real, usable engine you can play against.

**Phase 4 — Search strength.**
Iterative deepening, move ordering (MVV-LVA + killers + history), quiescence search, Zobrist hashing + transposition table. Outcome: it stops blundering tactics and searches far deeper for the same time.

**Phase 5 — Evaluation strength.**
Piece-square tables, tapered eval, mobility, pawn structure, king safety. Outcome: it plays *positionally*, not just tactically.

**Phase 6 — Advanced.**
Magic bitboards (if not already), null-move pruning, LMR, PVS, aspiration windows. Possibly NNUE much later. Outcome: meaningful Elo gains per feature.

**Phase 7 — Strength testing & iteration.**
Version your engine; run new-vs-old matches; gate every change behind a measured Elo improvement. Outcome: a disciplined improvement loop.

A bare working engine (Phases 0–3) is a few focused weekends. A genuinely decent ~1800–2000-Elo engine (through Phase 5–6) is weeks to a few months. Stockfish-level is years and a community — don't aim there; aim for "I built this, I understand every line, and it beats most humans."

---

## 4. Testing

Chess engines have an unusually good testing story — use it.

- **Perft (performance test) — the cornerstone.** From a given position, count the exact number of leaf nodes at depth N (every legal move sequence). Reference values for standard positions are published and known to the last digit. If your perft(5) from the start position is off by one node, you have a move-gen bug, full stop. Test several positions (the "Kiwipete" position is the famous tricky one — it stresses castling, en passant, and promotions). This catches the overwhelming majority of correctness bugs.
- **Make/unmake invariant.** After make then unmake, assert the board and Zobrist hash are identical to before. Run it across full perft trees and it'll flush out subtle state-restoration bugs.
- **FEN round-trip.** Parse a FEN, regenerate it, assert equality.
- **Tactical suites.** Feed it positions with known best moves (Win at Chess / "WAC", and similar EPD test files) and count how many it solves at a fixed depth/time. Good regression signal for search + eval.
- **Mate finding.** Known mate-in-N positions should be found at depth N with a forced-mate score.
- **Engine-vs-engine (the real strength test).** Use **cutechess-cli** to run hundreds or thousands of games of new-version-vs-old-version (and against other public engines of known strength). Use **SPRT** (sequential probability ratio test) to decide whether a change is a real improvement rather than noise — it stops the match early once it has statistical confidence. This is exactly how Stockfish development is gated, and it's the discipline that separates "I added a feature" from "I added a feature that demonstrably gains 15 Elo."

Rule of thumb: **perft for correctness, SPRT for strength.** Everything else is supporting evidence.

---

## 5. C++ specifics worth knowing going in

This project will stretch your C++ in productive directions. A few things to internalize:

- **Bit manipulation is your daily bread.** `uint64_t` boards, shifts, masks. Use `std::popcount` (count set bits) and `std::countr_zero` (index of lowest set bit) from C++20's `<bit>` — or the `__builtin_popcountll` / `__builtin_ctzll` intrinsics on older standards. These compile to single instructions.
- **No heap allocation on the hot path.** Move generation, make/unmake, and search run millions of times per second. Use `std::array` and fixed-size buffers, not `std::vector`, inside the search. Allocate once, reuse.
- **Cache locality matters more than cleverness.** Keep your hot data structures small and contiguous. A packed 16-bit move beats a 40-byte move struct because more of them fit in cache.
- **`const` correctness and value semantics.** Be deliberate about what's copied vs referenced; in the search, copies in the wrong place show up directly in your nodes-per-second.
- **Build with optimization.** Use CMake; compile with `-O2` or `-O3` and `-march=native`. The difference between a debug and an optimized build here is often 10×+ in node throughput.
- **Profile, don't guess.** `perf` (Linux), Callgrind/`valgrind`, and Compiler Explorer (godbolt.org) to see what the compiler actually emits. Measure nodes-per-second as your headline performance metric; optimize what the profiler says is hot, not what you assume is hot.
- **Undefined behavior will bite.** Signed overflow, uninitialized state, out-of-bounds indexing into attack tables — run a debug build under sanitizers (`-fsanitize=address,undefined`) periodically.

---

## 6. Suggested file/module layout

A clean starting decomposition (each becomes a `.hpp`/`.cpp` pair):

- `types` — squares, pieces, colors, the packed `Move`, score constants.
- `bitboard` — bit helpers, precomputed knight/king/pawn attack tables, (later) magic tables.
- `position` — the board state, make/unmake, FEN parsing, Zobrist hashing.
- `movegen` — pseudo-legal generation, legality filtering.
- `eval` — static evaluation.
- `search` — negamax/alpha-beta, iterative deepening, quiescence, transposition table.
- `tt` — the transposition table.
- `uci` — protocol parsing and the main loop.
- `perft` — the test harness.
- `main` — entry point.

Keep `position` and `movegen` ruthlessly correct and well-tested; everything else builds on them.

---

## 7. Reference resources

- **Chess Programming Wiki** — the canonical reference for every concept above (bitboards, magic bitboards, alpha-beta, quiescence, NNUE, perft values, etc.). Your primary handbook.
- **Perft results** — published node counts for standard positions, including the Kiwipete position, to validate move gen.
- **cutechess-cli** — for running engine-vs-engine matches and SPRT.
- **A UCI GUI** — Cute Chess, Arena, or Banksia GUI to actually play your engine.
- **Existing small open-source engines** — reading a compact, well-written engine (there are several teaching-oriented ones around 1–2k lines) is one of the fastest ways to see how the pieces fit. Read for structure; write your own.

---

## 8. The one-paragraph version

Represent the board with bitboards. Write move generation and make/unmake, and prove it with perft before doing anything else. Add negamax + alpha-beta with a material evaluation so it plays legal chess, then wrap it in UCI so you can actually use it. Make it strong by improving the *search* (iterative deepening, move ordering, quiescence, transposition table) and the *evaluation* (piece-square tables, tapered eval, structure). Measure every change with engine-vs-engine SPRT matches. Correctness comes from perft; strength comes from SPRT; speed comes from profiling the hot loop. Build it in that order and you'll always have a working engine that's getting better.
