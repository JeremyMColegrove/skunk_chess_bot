//
// Types, constants, and macros for the Skunk chess engine
//

#ifndef SKUNK_TYPES_H
#define SKUNK_TYPES_H

#include <stdint.h>
#include <string.h>

// Our bitboard type
#define U64 unsigned long long

/*********************\
      CONSTANTS
\*********************/

#define MAX_PLY 72
#define CHECKMATE 500000
#define NULL_R 3
#define DO_NULL 1
#define NO_NULL 0
#define HASH_EXACT 0
#define HASH_LOWERBOUND 1
#define HASH_UPPERBOUND 2
#define DEFAULT_MOVETIME 2500  // how many ms to search for
#define MAX_PERFT_DEPTH 24

// Transposition table size (256MB = ~10M entries)
#define HASH_SIZE (1 << 24)

// Aspiration window size
#define ASPIRATION_WINDOW 50

// LMR constants
#define LMR_DEPTH 3
#define LMR_MIN_DEPTH 3

// Late Move Pruning thresholds by depth
const int LMP_THRESHOLD[] = {0, 8, 12, 20, 30, 40, 50, 60};

// Razoring margins by depth (aggressive pruning at frontier nodes)
const int RAZORING_MARGIN[] = {0, 125, 250, 375};

// Futility margins by depth (prune quiet moves that can't improve alpha)
const int FUTILITY_MARGIN[] = {0, 150, 300, 450, 600};

// Delta pruning margin for quiescence search
#define DELTA_MARGIN 200

// LMR reduction table [depth][move_count] - initialized at startup
extern int LMR_TABLE[64][64];

// Initialize the LMR table with logarithmic reductions
void init_lmr_table();

/*********************\
    BITWISE MACROS
\*********************/

// Returns the bit on the square
#define get_bit(bitboard, square) ((bitboard) & (1ULL << (square)))

// Turns the bit at square to 1
#define set_bit(bitboard, square) ((bitboard) |= (1ULL << (square)))

// Turns the bit at square to 0
#define pop_bit(bitboard, square) ((bitboard) &= ~(1ULL << (square)))

// Pops off the ls1b. This is a faster operation than pop_bit so use this whenever you can
#define pop_lsb(bitboard) (bitboard &= (bitboard - 1))

// Encodes a move
/* | castling enp d_push capture promoted piece  dest   source
 * | 0        0   0      0000    0000     000000 000000 000000
 */
#define encode_move(source, destination, piece, promoted, enpassant, castling) \
    (source) |                                                                  \
    (destination << 6) |                                                        \
    (piece << 12) |                                                             \
    (promoted << 16) |                                                          \
    (enpassant << 22) |                                                         \
    (castling << 23)

// Move decoding macros
#define decode_source(move) ((move) & 0x3f)
#define decode_destination(move) (((move) & 0xfc0) >> 6)
#define decode_piece(move) (((move) & 0xf000) >> 12)
#define decode_promoted(move) (((move) & 0xf0000) >> 16)
#define decode_enpassant(move) ((move) & 0x400000)
#define decode_castle(move) ((move) & 0x800000)

/*********************\
     ENUMERATIONS
\*********************/

enum EvaluationWeights {
    MATERIAL_WEIGHT,
    PIECE_SCORE_WEIGHT,
    DOUBLED_PAWNS_WEIGHT,
    ISOLATED_PAWNS_WEIGHT,
    PASSED_PAWN_WEIGHT,
    MOBILITY_WEIGHT,
    KING_SAFETY_WEIGHT,
    PAWN_SHIELD_WEIGHT,
    BACKWARD_PAWN_WEIGHT,
    SPACE_WEIGHT,
    NUM_WEIGHTS
};

enum MoveType { all_moves, only_captures };

// Square enumeration
enum {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1, no_square
};

// File mask indices
enum { LEFT = 0, MIDDLE = 1, RIGHT = 2 };

// Colors
enum { black, white, both };

// Pieces (uppercase = white, lowercase = black)
enum { P, N, B, R, Q, K, p, n, b, r, q, k };

// Directions (set up so the opposite direction is at index 7-i)
enum { DE, DN, DNW, DNE, DSW, DSE, DS, DW };

// Castling rights
enum { wk = 1, wq = 2, bk = 4, bq = 8 };

// Node types for transposition table
enum NodeType { LOWER_BOUND, UPPER_BOUND, EXACT };

/*********************\
    PIECE VALUES
\*********************/

const int piece_scores[12] = {
    100,   // Pawn (P)
    320,   // Knight (N)
    330,   // Bishop (B)
    500,   // Rook (R)
    900,   // Queen (Q)
    0,     // King (K) (not used in the evaluation function)
    -100,  // Pawn (p)
    -320,  // Knight (n)
    -330,  // Bishop (b)
    -500,  // Rook (r)
    -900,  // Queen (q)
    0      // King (k) (not used in the evaluation function)
};

const char ascii_pieces[] = "PNBRQKpnbrqk";

/*********************\
       STRUCTS
\*********************/

// Transposition table entry
struct TTEntry {
    U64 zobristKey;
    int value;
    int move;
    int16_t depth;
    uint8_t type;
};

// Move list
typedef struct {
    int moves[256];
    int count;
    int contains_castle;
} t_moves;

// Principal variation line
typedef struct {
    int cmove;             // Number of moves in the line
    int argmove[MAX_PLY];  // The line
} t_line;

// Search result
typedef struct {
    int move;
    int score;
} t_result;

// Perft results
typedef struct {
    long long int total_nodes;
    int nodes[MAX_PERFT_DEPTH];
    int captures[MAX_PERFT_DEPTH];
    int castles[MAX_PERFT_DEPTH];
    int promotions[MAX_PERFT_DEPTH];
    int checks[MAX_PERFT_DEPTH];
    int discovered_checks[MAX_PERFT_DEPTH];
    int double_checks[MAX_PERFT_DEPTH];
    int checkmates[MAX_PERFT_DEPTH];
    int enpassants[MAX_PERFT_DEPTH];
} perft;

// Attacker info
typedef struct {
    int count;
    U64 ray;
} t_attackers;

// Repetition tracking
typedef struct {
    U64 table[500];
    int count;
} t_repitition;

#endif  // SKUNK_TYPES_H
