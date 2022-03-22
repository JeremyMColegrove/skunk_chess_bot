//
// Created by Jeremy Colegrove on 11/8/21.
//

#ifndef BITBOT_BOARD_H
#define BITBOT_BOARD_H
#include <stdint.h>
#include <string.h>
#include <climits>
#include <algorithm>
#include <chrono>
#include <thread>
#ifdef _WIN32
#include <conio.h>
#include <Windows.h>
#else
#include <unistd.h>
#include <poll.h>
#endif

//Our bitboard type
#define U64 unsigned long long

// flag for enabling asserts in the code for debugging and error checking i.e. zobrist key checking
//#define DEBUG
// flag for enabling the transposition table
#define TRANSPOSITION_TABLE
// flag for enabling NULL MOVE in negamax
//#define NULL_MOVE
#define VERIFIED_NULL_MOVE


//Some macros for getting and setting bits
//Returns the bit on the square
#define get_bit(bitboard, square) ((bitboard) & (1ULL << (square)))

//Turns the bit at square to 1
#define set_bit(bitboard, square) ((bitboard) |= (1ULL << (square)))

//Turns the bit at square to 0
#define pop_bit(bitboard, square) ((bitboard) &= ~(1ULL << (square)))

// pops off the ls1b. This is a faster operation that pop_bit so use this whenever you can
#define pop_lsb(bitboard) (bitboard &= (bitboard-1))

//encodes a move
/* | castling enp d_push capture promoted piece  dest   source
 * | 0        0   0      0000    0000     000000 000000 000000
 */
#define encode_move(source, destination, piece, promoted, enpassant, castling) \
    (source) |                                                                                  \
    (destination << 6) |                                                                        \
    (piece << 12) |                                                                             \
    (promoted << 16) |                                                                          \
    (enpassant << 22) |                                                                         \
    (castling << 23)


#define MAX_PERFT_DEPTH 24
#define decode_source(move) ((move) & 0x3f)
#define decode_destination(move) (((move) & 0xfc0) >> 6)
#define decode_piece(move) (((move) & 0xf000) >> 12)
#define decode_promoted(move) (((move) & 0xf0000) >> 16)
//#define decode_capture(move) ((move) & 0x100000)
//#define decode_double(move) ((move) & 0x200000)
#define is_capture(move) ((1ULL << decode_destination(move)) & occupancies[side ^ 1] || decode_enpassant(move))
#define decode_enpassant(move) ((move) & 0x400000)
#define decode_castle(move) ((move) & 0x800000)


#define flip_square(sq) (sq ^ 56)
#define MAX_PLY 72
#define CHECKMATE 500000
#define NULL_R 3
#define NO_VALUE 9999999
#define DO_NULL 1
#define NO_NULL 0
// transposition table definitions
//#define HASH_SIZE 3999971 //~4 MB (must be prime) for actual games
#define HASH_SIZE 3999971 // small amount for testing
#define HASH_EXACT 0
#define HASH_LOWERBOUND 1
#define HASH_UPPERBOUND 2


#define SEARCH_DEPTH 0
#define SEARCH_MOVETIME 1



#define copy_board() \
U64 bitboards_copy[12], occupancies_copy[3]; \
int castle_copy=0, side_copy=0, enpassant_copy=0; \
memcpy(bitboards_copy, bitboards, sizeof(bitboards)); \
memcpy(occupancies_copy, occupancies, sizeof(occupancies)); \
enpassant_copy = enpassant; castle_copy = castle; side_copy = side; \
U64 zobrist_copy = zobrist; \

#define restore_board() \
    memcpy(bitboards, bitboards_copy, sizeof(bitboards)); \
    memcpy(occupancies, occupancies_copy,  sizeof(occupancies)); \
    enpassant = enpassant_copy; castle = castle_copy; side = side_copy; \
    zobrist = zobrist_copy; \
//move types
enum {all_moves, only_captures};
/**
 *          not_a_file
 *  8    	0  1  1  1  1  1  1  1
    7		0  1  1  1  1  1  1  1
    6		0  1  1  1  1  1  1  1
    5		0  1  1  1  1  1  1  1
    4		0  1  1  1  1  1  1  1
    3		0  1  1  1  1  1  1  1
    2		0  1  1  1  1  1  1  1
    1		0  1  1  1  1  1  1  1

            a  b  c  d  e  f  g  h
 */
#define not_a_file 18374403900871474942ULL

/**
 *          not_h_file
 *  8		1  1  1  1  1  1  1  0
    7		1  1  1  1  1  1  1  0
    6		1  1  1  1  1  1  1  0
    5		1  1  1  1  1  1  1  0
    4		1  1  1  1  1  1  1  0
    3		1  1  1  1  1  1  1  0
    2		1  1  1  1  1  1  1  0
    1		1  1  1  1  1  1  1  0

            a  b  c  d  e  f  g  h
 */
#define not_h_file 9187201950435737471ULL


/**
 *          not_gh_file
 * 8		1  1  1  1  1  1  0  0
    7		1  1  1  1  1  1  0  0
    6		1  1  1  1  1  1  0  0
    5		1  1  1  1  1  1  0  0
    4		1  1  1  1  1  1  0  0
    3		1  1  1  1  1  1  0  0
    2		1  1  1  1  1  1  0  0
    1		1  1  1  1  1  1  0  0

            a  b  c  d  e  f  g  h
 */
#define not_gh_file 4557430888798830399ULL

/**
 *          not_ab_file
 * 8		0  0  1  1  1  1  1  1
    7		0  0  1  1  1  1  1  1
    6		0  0  1  1  1  1  1  1
    5		0  0  1  1  1  1  1  1
    4		0  0  1  1  1  1  1  1
    3		0  0  1  1  1  1  1  1
    2		0  0  1  1  1  1  1  1
    1		0  0  1  1  1  1  1  1

            a  b  c  d  e  f  g  h
 */
#define not_ab_file 18229723555195321596ULL

/**
 * row2 for double pawn pushes
8		0  0  0  0  0  0  0  0
7		0  0  0  0  0  0  0  0
6		0  0  0  0  0  0  0  0
5		0  0  0  0  0  0  0  0
4		0  0  0  0  0  0  0  0
3		0  0  0  0  0  0  0  0
2		1  1  1  1  1  1  1  1
1		0  0  0  0  0  0  0  0

		a  b  c  d  e  f  g  h
 */

#define row2 71776119061217280ULL

/**
 * row7 for double pawn pushes
 8		0  0  0  0  0  0  0  0
7		1  1  1  1  1  1  1  1
6		0  0  0  0  0  0  0  0
5		0  0  0  0  0  0  0  0
4		0  0  0  0  0  0  0  0
3		0  0  0  0  0  0  0  0
2		0  0  0  0  0  0  0  0
1		0  0  0  0  0  0  0  0

		a  b  c  d  e  f  g  h
		Board number: 65280
 */
#define row7 65280


/**
8		1  1  1  1  1  1  1  1
7		0  0  0  0  0  0  0  0
6		0  0  0  0  0  0  0  0
5		0  0  0  0  0  0  0  0
4		0  0  0  0  0  0  0  0
3		0  0  0  0  0  0  0  0
2		0  0  0  0  0  0  0  0
1		0  0  0  0  0  0  0  0

		a  b  c  d  e  f  g  h
		Board number: 255
 */
#define row8 255

/**
8		0  0  0  0  0  0  0  0
7		0  0  0  0  0  0  0  0
6		0  0  0  0  0  0  0  0
5		0  0  0  0  0  0  0  0
4		0  0  0  0  0  0  0  0
3		0  0  0  0  0  0  0  0
2		0  0  0  0  0  0  0  0
1		1  1  1  1  1  1  1  1

		a  b  c  d  e  f  g  h
		Board number: 18374686479671623680
 */
#define row1 18374686479671623680

/**
 * castling mask
8		0  0  0  0  0  0  0  0
7		0  0  0  0  0  0  0  0
6		0  0  0  0  0  0  0  0
5		0  0  0  0  0  0  0  0
4		0  0  0  0  0  0  0  0
3		0  0  0  0  0  0  0  0
2		0  0  0  0  0  0  0  0
1		0  0  0  0  0  1  1  0

		a  b  c  d  e  f  g  h
		Board number: 6917529027641081856
 */
#define castle_mask_wk 6917529027641081856

/**
8		0  0  0  0  0  1  1  0
7		0  0  0  0  0  0  0  0
6		0  0  0  0  0  0  0  0
5		0  0  0  0  0  0  0  0
4		0  0  0  0  0  0  0  0
3		0  0  0  0  0  0  0  0
2		0  0  0  0  0  0  0  0
1		0  0  0  0  0  0  0  0

		a  b  c  d  e  f  g  h
		Board number: 96
 */
#define castle_mask_bk 96

/**
8               0  0  0  0  0  0  0  0
7               0  0  0  0  0  0  0  0
6               0  0  0  0  0  0  0  0
5               0  0  0  0  0  0  0  0
4               0  0  0  0  0  0  0  0
3               0  0  0  0  0  0  0  0
2               0  0  0  0  0  0  0  0
1               0  0  1  1  0  0  0  0

                a  b  c  d  e  f  g  h
                Board number: 864691128455135232
 */
#define castle_attack_mask_wq 864691128455135232
/**
8		0  0  0  0  0  0  0  0
7		0  0  0  0  0  0  0  0
6		0  0  0  0  0  0  0  0
5		0  0  0  0  0  0  0  0
4		0  0  0  0  0  0  0  0
3		0  0  0  0  0  0  0  0
2		0  0  0  0  0  0  0  0
1		0  1  1  1  0  0  0  0

		a  b  c  d  e  f  g  h
		Board number: 1008806316530991104
 */
#define castle_piece_mask_wq 1008806316530991104

/**
8               0  0  1  1  0  0  0  0
7               0  0  0  0  0  0  0  0
6               0  0  0  0  0  0  0  0
5               0  0  0  0  0  0  0  0
4               0  0  0  0  0  0  0  0
3               0  0  0  0  0  0  0  0
2               0  0  0  0  0  0  0  0
1               0  0  0  0  0  0  0  0

                a  b  c  d  e  f  g  h
                Board number: 12
 */
#define castle_attack_mask_bq 12
/**
8		0  1  1  1  0  0  0  0
7		0  0  0  0  0  0  0  0
6		0  0  0  0  0  0  0  0
5		0  0  0  0  0  0  0  0
4		0  0  0  0  0  0  0  0
3		0  0  0  0  0  0  0  0
2		0  0  0  0  0  0  0  0
1		0  0  0  0  0  0  0  0

		a  b  c  d  e  f  g  h
		Board number: 14
 */
#define castle_piece_mask_bq 14
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

enum { black, white };

enum { rook, bishop, both };
//Encode the pieces here as they would be output to the screen
enum {P, N, B, R, Q, K, p, n, b, r, q, k};

//Everything is public right now for testing purposes
/**
 * Castling rights
 * bin      dec     meaning
 * 0001     1       white king can castle to king side
 * 0010     2       white king can castle to queen side
 * 0100     4       black king can castle to king side
 * 1000     8       black king can castle to queen side
 *
 *
 *
 */
enum {wk = 1, wq = 2, bk = 4, bq = 8};

const char ascii_pieces[] = "PNBRQKpnbrqk";

typedef struct {
    int moves[256];
    int count;
} t_moves;

typedef struct {
    int cmove;              // Number of t_moves in the line.
    int argmove[MAX_PLY];  // The line.
}   t_line;

typedef struct {
    int move;
    int score;
} t_result;

typedef struct {
    U64 hash;
    int depth;
    int flags;
    int score;
    int move;
} t_entry;

typedef struct {
    long long int total_nodes;
    int nodes[MAX_PERFT_DEPTH]; // handle up to 24 levels
    int captures[MAX_PERFT_DEPTH];
    int castles[MAX_PERFT_DEPTH];
    int promotions[MAX_PERFT_DEPTH];
    int checks[MAX_PERFT_DEPTH];
    int discovered_checks[MAX_PERFT_DEPTH];
    int double_checks[MAX_PERFT_DEPTH];
    int checkmates[MAX_PERFT_DEPTH];
    int enpassants[MAX_PERFT_DEPTH];
} perft;

typedef struct {
    int count;
    U64 ray;
} t_attackers;

typedef struct {
    U64 table[500];
    int count;
} t_repitition;

class Skunk {
public:
    const int lsb_64_table[64] =
            {
                    63, 30,  3, 32, 59, 14, 11, 33,
                    60, 24, 50,  9, 55, 19, 21, 34,
                    61, 29,  2, 53, 51, 23, 41, 18,
                    56, 28,  1, 43, 46, 27,  0, 35,
                    62, 31, 58,  4,  5, 49, 54,  6,
                    15, 52, 12, 40,  7, 42, 45, 16,
                    25, 57, 48, 13, 10, 39,  8, 44,
                    20, 47, 38, 22, 17, 37, 36, 26
            };
    const char *square_to_coordinate[64] = {
            "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
            "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
            "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
            "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
            "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
            "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
            "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
            "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"};

    const int bishop_relevant_bits[64] = {
            6, 5, 5, 5, 5, 5, 5, 6,
            5, 5, 5, 5, 5, 5, 5, 5,
            5, 5, 7, 7, 7, 7, 5, 5,
            5, 5, 7, 9, 9, 7, 5, 5,
            5, 5, 7, 9, 9, 7, 5, 5,
            5, 5, 7, 7, 7, 7, 5, 5,
            5, 5, 5, 5, 5, 5, 5, 5,
            6, 5, 5, 5, 5, 5, 5, 6,
    };
    const int rook_relevant_bits[64] = {
            12, 11, 11, 11, 11, 11, 11, 12,
            11, 10, 10, 10, 10, 10, 10, 11,
            11, 10, 10, 10, 10, 10, 10, 11,
            11, 10, 10, 10, 10, 10, 10, 11,
            11, 10, 10, 10, 10, 10, 10, 11,
            11, 10, 10, 10, 10, 10, 10, 11,
            11, 10, 10, 10, 10, 10, 10, 11,
            12, 11, 11, 11, 11, 11, 11, 12,
    };
    U64 rook_magic_numbers[64] = {
            0x8a80104000800020ULL,
            0x140002000100040ULL,
            0x2801880a0017001ULL,
            0x100081001000420ULL,
            0x200020010080420ULL,
            0x3001c0002010008ULL,
            0x8480008002000100ULL,
            0x2080088004402900ULL,
            0x800098204000ULL,
            0x2024401000200040ULL,
            0x100802000801000ULL,
            0x120800800801000ULL,
            0x208808088000400ULL,
            0x2802200800400ULL,
            0x2200800100020080ULL,
            0x801000060821100ULL,
            0x80044006422000ULL,
            0x100808020004000ULL,
            0x12108a0010204200ULL,
            0x140848010000802ULL,
            0x481828014002800ULL,
            0x8094004002004100ULL,
            0x4010040010010802ULL,
            0x20008806104ULL,
            0x100400080208000ULL,
            0x2040002120081000ULL,
            0x21200680100081ULL,
            0x20100080080080ULL,
            0x2000a00200410ULL,
            0x20080800400ULL,
            0x80088400100102ULL,
            0x80004600042881ULL,
            0x4040008040800020ULL,
            0x440003000200801ULL,
            0x4200011004500ULL,
            0x188020010100100ULL,
            0x14800401802800ULL,
            0x2080040080800200ULL,
            0x124080204001001ULL,
            0x200046502000484ULL,
            0x480400080088020ULL,
            0x1000422010034000ULL,
            0x30200100110040ULL,
            0x100021010009ULL,
            0x2002080100110004ULL,
            0x202008004008002ULL,
            0x20020004010100ULL,
            0x2048440040820001ULL,
            0x101002200408200ULL,
            0x40802000401080ULL,
            0x4008142004410100ULL,
            0x2060820c0120200ULL,
            0x1001004080100ULL,
            0x20c020080040080ULL,
            0x2935610830022400ULL,
            0x44440041009200ULL,
            0x280001040802101ULL,
            0x2100190040002085ULL,
            0x80c0084100102001ULL,
            0x4024081001000421ULL,
            0x20030a0244872ULL,
            0x12001008414402ULL,
            0x2006104900a0804ULL,
            0x1004081002402ULL
    };

// bishop magic numbers
    U64 bishop_magic_numbers[64] = {
            0x40040844404084ULL,
            0x2004208a004208ULL,
            0x10190041080202ULL,
            0x108060845042010ULL,
            0x581104180800210ULL,
            0x2112080446200010ULL,
            0x1080820820060210ULL,
            0x3c0808410220200ULL,
            0x4050404440404ULL,
            0x21001420088ULL,
            0x24d0080801082102ULL,
            0x1020a0a020400ULL,
            0x40308200402ULL,
            0x4011002100800ULL,
            0x401484104104005ULL,
            0x801010402020200ULL,
            0x400210c3880100ULL,
            0x404022024108200ULL,
            0x810018200204102ULL,
            0x4002801a02003ULL,
            0x85040820080400ULL,
            0x810102c808880400ULL,
            0xe900410884800ULL,
            0x8002020480840102ULL,
            0x220200865090201ULL,
            0x2010100a02021202ULL,
            0x152048408022401ULL,
            0x20080002081110ULL,
            0x4001001021004000ULL,
            0x800040400a011002ULL,
            0xe4004081011002ULL,
            0x1c004001012080ULL,
            0x8004200962a00220ULL,
            0x8422100208500202ULL,
            0x2000402200300c08ULL,
            0x8646020080080080ULL,
            0x80020a0200100808ULL,
            0x2010004880111000ULL,
            0x623000a080011400ULL,
            0x42008c0340209202ULL,
            0x209188240001000ULL,
            0x400408a884001800ULL,
            0x110400a6080400ULL,
            0x1840060a44020800ULL,
            0x90080104000041ULL,
            0x201011000808101ULL,
            0x1a2208080504f080ULL,
            0x8012020600211212ULL,
            0x500861011240000ULL,
            0x180806108200800ULL,
            0x4000020e01040044ULL,
            0x300000261044000aULL,
            0x802241102020002ULL,
            0x20906061210001ULL,
            0x5a84841004010310ULL,
            0x4010801011c04ULL,
            0xa010109502200ULL,
            0x4a02012000ULL,
            0x500201010098b028ULL,
            0x8040002811040900ULL,
            0x28000010020204ULL,
            0x6000020202d0240ULL,
            0x8918844842082200ULL,
            0x4010011029020020ULL
    };
    // convert ASCII character pieces to encoded constants
#ifndef _WIN32
    char *unicode_pieces[12] = { "♙", "♘", "♗", "♖", "♕", "♔","♟︎", "♞", "♝", "♜", "♛", "♚" };
#endif
//     int char_pieces[12] = {P, N, B, R, Q, K, p, n, b, r, q, k};

        // must initialize these like char_pieces['P'] = Pw
    int char_pieces[115];

    const int castling_rights[64] = {
            7, 15, 15, 15,  3, 15, 15, 11,
            15, 15, 15, 15, 15, 15, 15, 15,
            15, 15, 15, 15, 15, 15, 15, 15,
            15, 15, 15, 15, 15, 15, 15, 15,
            15, 15, 15, 15, 15, 15, 15, 15,
            15, 15, 15, 15, 15, 15, 15, 15,
            15, 15, 15, 15, 15, 15, 15, 15,
            13, 15, 15, 15, 12, 15, 15, 14
    };

    // score tables for evaluating pieces
    const int piece_scores[12] = {
            100,
            300,
            350,
            500,
            1000,
            10000,
            -100,
            -300,
            -350,
            -500,
            -1000,
            -10000
    };

    // pawn positional score
    const int pawn_score[64] =
            {
                    90,  90,  90,  90,  90,  90,  90,  90,
                    30,  30,  30,  40,  40,  30,  30,  30,
                    20,  20,  20,  30,  30,  30,  20,  20,
                    10,  10,  10,  20,  20,  10,  10,  10,
                    5,   5,  10,  20,  20,   5,   5,   5,
                    0,   0,   0,   5,   5,   0,   0,   0,
                    0,   0,   0, -10, -10,   0,   0,   0,
                    0,   0,   0,   0,   0,   0,   0,   0
            };

// knight positional score
    const int knight_score[64] =
            {
                    -5,   0,   0,   0,   0,   0,   0,  -5,
                    -5,   0,   0,  10,  10,   0,   0,  -5,
                    -5,   5,  20,  20,  20,  20,   5,  -5,
                    -5,  10,  20,  30,  30,  20,  10,  -5,
                    -5,  10,  20,  30,  30,  20,  10,  -5,
                    -5,   5,  20,  10,  10,  20,   5,  -5,
                    -5,   0,   0,   0,   0,   0,   0,  -5,
                    -5, -10,   0,   0,   0,   0, -10,  -5
            };

// bishop positional score
    const int bishop_score[64] =
            {
                    0,   0,   0,   0,   0,   0,   0,   0,
                    0,   0,   0,   0,   0,   0,   0,   0,
                    0,   0,   0,  10,  10,   0,   0,   0,
                    0,   0,  10,  20,  20,  10,   0,   0,
                    0,   0,  10,  20,  20,  10,   0,   0,
                    0,  10,   0,   0,   0,   0,  10,   0,
                    0,  30,   0,   0,   0,   0,  30,   0,
                    0,   0, -10,   0,   0, -10,   0,   0

            };

// rook positional score
    const int rook_score[64] =
            {
                    50,  50,  50,  50,  50,  50,  50,  50,
                    50,  50,  50,  50,  50,  50,  50,  50,
                    0,   0,  10,  20,  20,  10,   0,   0,
                    0,   0,  10,  20,  20,  10,   0,   0,
                    0,   0,  10,  20,  20,  10,   0,   0,
                    0,   0,  10,  20,  20,  10,   0,   0,
                    0,   0,  10,  20,  20,  10,   0,   0,
                    0,   0,   0,  20,  20,   0,   0,   0

            };

// king positional score
    const int king_score[64] =
            {
                    0,   0,   0,   0,   0,   0,   0,   0,
                    0,   0,   5,   5,   5,   5,   0,   0,
                    0,   5,   5,  10,  10,   5,   5,   0,
                    0,   5,  10,  20,  20,  10,   5,   0,
                    0,   5,  10,  20,  20,  10,   5,   0,
                    0,   0,   5,  10,  10,   5,   0,   0,
                    0,   5,   5,  -5,  -5,   0,   5,   0,
                    0,   0,   5,   0, -15,   0,  10,   0
            };

// mirror positional score tables for opposite side
    const int mirror_score[128] =
            {
                    a1, b1, c1, d1, e1, f1, g1, h1,
                    a2, b2, c2, d2, e2, f2, g2, h2,
                    a3, b3, c3, d3, e3, f3, g3, h3,
                    a4, b4, c4, d4, e4, f4, g4, h4,
                    a5, b5, c5, d5, e5, f5, g5, h5,
                    a6, b6, c6, d6, e6, f6, g6, h6,
                    a7, b7, c7, d7, e7, f7, g7, h7,
                    a8, b8, c8, d8, e8, f8, g8, h8
            };


    // MVV LVA [attacker][victim]
    int mvv_lva[12][12] = {
            105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
            104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
            103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
            102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
            101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
            100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600,

            105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
            104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
            103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
            102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
            101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
            100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600
    };
    unsigned int seed = 1804289383;


    Skunk();
    ~Skunk();

    void parse_fen(char *fen);
    void print_bitboard(U64 board);
    void print_board();
    void print_attacks(int side);

    U64 pawn_masks[2][64];
    U64 knight_masks[64];
    U64 king_masks[64];
    U64 bishop_masks[64];
    U64 rook_masks[64];
    U64 rook_attacks[64][4096];
    U64 bishop_attacks[64][512];
    U64 occupancies[3];
    U64 bitboards[12];
    U64 rays[64][64];
    int get_piece(int square);
    U64 get_attacks(int piece, int square, int side);
    enum {DE, DN, DNW, DNE, DSW, DSE, DS, DW}; // set up so the opposite direction is 8-i
    int nearest_square[8][64]; // given a direction and a square, give me the furthest square in that direction

    // ZOBRISK HASHING
    U64 piece_keys[12][64];
    U64 enpassant_keys[64];
    U64 castle_keys[16];
    U64 side_key;

    int piece_count[12];
    // killer t_moves
    int killer_moves[2][MAX_PLY];
    //history move source->destination (alternative could be piece->destination)
    int history_moves[2][64][64];

    t_entry *transposition_table = NULL; // tt table for negamax search

    // repitition array for 3 move repitition
    t_repitition repitition;


    U64 zobrist = 0ULL;
    t_line previous_pv_line;

    inline int is_repitition();
    inline void construct_rays();
    inline void construct_direction_rays();
    inline U64 construct_bishop_attacks(int square, U64 blockers);
    inline U64 construct_rook_attacks(int square, U64 blockers);
    inline U64 get_rook_attacks(int square, U64 occupancy);
    inline U64 get_bishop_attacks(int square, U64 occupancy);
    inline U64 get_queen_attacks(int square, U64 occupancy);
    inline int is_square_attacked(int square, int side);
    inline U64 get_slider_attacks();
    inline U64 get_jumper_attacks();
    int bit_count(U64 board);
    int get_ls1b_index(U64 board);
    U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask);
    inline void fill_occupancies();
    inline void generate_moves(t_moves &moves_list);
    inline void generate_moves_old(t_moves &moves_list);
    inline void print_difference(t_moves &a, t_moves &b);
    inline void print_move_detailed(int move);
    inline int make_move(int move, int move_flag);
    void perft_test(int depth);
    int evaluate();
    int null_ok();
    int search(int maxDepth);
    int negamax(int alpha, int beta, int depth, int verify, int do_null, t_line *pline);
    int quiesence(int alpha, int beta);

    int is_checkmate();
    int is_check();
    int coordinate_to_square(char *coordinate);
    int score_move(int move);
    void sort_moves(t_moves &moves_list);
    unsigned int get_random_U32_number();
    U64 get_random_U64_number();
    U64 generate_zobrist();
    void print_move(int move);
    void init();
    void test_moves_sort();
    void print_moves(t_moves &moves_list);
    int score_to_tt(int score, int ply);
    int score_from_tt(int score, int ply);
    void clear_transposition_tables();
    void write_hash_entry(int score, int depth, int move, int flag);
    // time functions to incorporate time checking
    std::chrono::steady_clock::time_point start_time;

    // evaluation commands



    // UCI commands/helper functions
    void communicate();
    int parse_move(char *move_string);
    void parse_position(char *command);
    void parse_go(char *command);
    void parse_option(char *command);
    void parse_debug(char *command);
    void parse_perft(char *command);
    char *fen_start = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    int force_stop = 0;
    int search_type = SEARCH_DEPTH;
    int wtime = 0;
    int btime = 0;
    int winc = 0;
    int binc = 0;
    int UCI_DefaultDepth = 7;
    int UCI_DebugMode = 0;
    int UCI_DefaultDuration = 8000; // default time to search in milliseconds
    int UCI_AnalyseMode = 1;
    int time_check_node_interval = 50000;


    int enpassant = no_square;

private:
    void perft_test_helper(int depth);

    //Functions for getting each pieces valid t_moves (takes whose turn it should calculate for)
    //State for our pseudo random number generator
    //All of our bitboards
    void construct_pawn_tables();
    void construct_knight_masks();
    void construct_king_tables();
    void construct_bishop_tables();
    void construct_rook_tables();
    void construct_slider_attacks();
    void add_move(t_moves &moves_list, int move);
    void clear_moves();
    int side = white;
    int castle = 0;
    int full_moves = 0;
    int ply = 0;
    int nodes = 0;
    int quiesence_moves = 0;
    int cache_hit = 0;
    int cache_miss = 0;
};
#endif //BITBOT_BOARD_H
