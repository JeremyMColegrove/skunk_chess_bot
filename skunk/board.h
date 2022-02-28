//
// Created by Jeremy Colegrove on 11/8/21.
//

#ifndef BITBOT_BOARD_H
#define BITBOT_BOARD_H
#include <stdint.h>
#include <string.h>
#include <climits>
//Our bitboard type
#define U64 unsigned long long



//Some macros for getting and setting bits
//Returns the bit on the square
#define get_bit(bitboard, square) ((bitboard) & (1ULL << (square)))

//Turns the bit at square to 1
#define set_bit(bitboard, square) ((bitboard) |= (1ULL << (square)))

//Turns the bit at square to 0
#define pop_bit(bitboard, square) ((bitboard) &= ~(1ULL << (square)))

//encodes a move
#define encode_move(source, destination, piece, promoted, capture, double_push, enpassant, castling) \
    (source) |                                                                                  \
    (destination << 6) |                                                                        \
    (piece << 12) |                                                                             \
    (promoted << 16) |                                                                          \
    (capture << 20) |                                                                           \
    (double_push << 21) |                                                                            \
    (enpassant << 22) |                                                                         \
    (castling << 23)


#define MAX_PERFT_DEPTH 24
#define decode_source(move) ((move) & 0x3f)
#define decode_destination(move) (((move) & 0xfc0) >> 6)
#define decode_piece(move) (((move) & 0xf000) >> 12)
#define decode_promoted(move) (((move) & 0xf0000) >> 16)
#define decode_capture(move) ((move) & 0x100000)
#define decode_double(move) ((move) & 0x200000)
#define decode_enpassant(move) ((move) & 0x400000)
#define decode_castle(move) ((move) & 0x800000)
#define flip_square(sq) (sq ^ 56)

#define row_4 1095216660480
#define row_5 4278190080



#define copy_board() \
U64 bitboards_copy[12], occupancies_copy[3]; \
int castle_copy, side_copy, enpassant_copy; \
memcpy(bitboards_copy, bitboards, sizeof(bitboards)); \
memcpy(occupancies_copy, occupancies, sizeof(occupancies)); \
enpassant_copy = enpassant; castle_copy = castle; side_copy = side; \

#define restore_board() \
    memcpy(bitboards, bitboards_copy, sizeof(bitboards)); \
    memcpy(occupancies_copy, occupancies, sizeof(occupancies)); \
    enpassant = enpassant_copy; castle = castle_copy; side = side_copy; \

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
} moves;

typedef struct {
    int move;
    int score;
} result;


typedef struct {
    int nodes[MAX_PERFT_DEPTH]; // handle up to 24 levels
    int captures[MAX_PERFT_DEPTH];
    int castles[MAX_PERFT_DEPTH];
    int promotions[MAX_PERFT_DEPTH];
    int checks[MAX_PERFT_DEPTH];
    int discovered_checks[MAX_PERFT_DEPTH];
    int double_checks[MAX_PERFT_DEPTH];
    int checkmates[MAX_PERFT_DEPTH];
} perft;

class Skunk {
public:
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
    
    char *unicode_pieces[12] = { "♙", "♘", "♗", "♖", "♕", "♔","♟︎", "♞", "♝", "♜", "♛", "♚" };

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

    int mg_pawn_table[64] = {
            0,   0,   0,   0,   0,   0,  0,   0,
            98, 134,  61,  95,  68, 126, 34, -11,
            -6,   7,  26,  31,  65,  56, 25, -20,
            -14,  13,   6,  21,  23,  12, 17, -23,
            -27,  -2,  -5,  12,  17,   6, 10, -25,
            -26,  -4,  -4, -10,   3,   3, 33, -12,
            -35,  -1, -20, -23, -15,  24, 38, -22,
            0,   0,   0,   0,   0,   0,  0,   0,
    };

    int eg_pawn_table[64] = {
            0,   0,   0,   0,   0,   0,   0,   0,
            178, 173, 158, 134, 147, 132, 165, 187,
            94, 100,  85,  67,  56,  53,  82,  84,
            32,  24,  13,   5,  -2,   4,  17,  17,
            13,   9,  -3,  -7,  -7,  -8,   3,  -1,
            4,   7,  -6,   1,   0,  -5,  -1,  -8,
            13,   8,   8,  10,  13,   0,   2,  -7,
            0,   0,   0,   0,   0,   0,   0,   0,
    };

    int mg_knight_table[64] = {
            -167, -89, -34, -49,  61, -97, -15, -107,
            -73, -41,  72,  36,  23,  62,   7,  -17,
            -47,  60,  37,  65,  84, 129,  73,   44,
            -9,  17,  19,  53,  37,  69,  18,   22,
            -13,   4,  16,  13,  28,  19,  21,   -8,
            -23,  -9,  12,  10,  19,  17,  25,  -16,
            -29, -53, -12,  -3,  -1,  18, -14,  -19,
            -105, -21, -58, -33, -17, -28, -19,  -23,
    };

    int eg_knight_table[64] = {
            -58, -38, -13, -28, -31, -27, -63, -99,
            -25,  -8, -25,  -2,  -9, -25, -24, -52,
            -24, -20,  10,   9,  -1,  -9, -19, -41,
            -17,   3,  22,  22,  22,  11,   8, -18,
            -18,  -6,  16,  25,  16,  17,   4, -18,
            -23,  -3,  -1,  15,  10,  -3, -20, -22,
            -42, -20, -10,  -5,  -2, -20, -23, -44,
            -29, -51, -23, -15, -22, -18, -50, -64,
    };

    int mg_bishop_table[64] = {
            -29,   4, -82, -37, -25, -42,   7,  -8,
            -26,  16, -18, -13,  30,  59,  18, -47,
            -16,  37,  43,  40,  35,  50,  37,  -2,
            -4,   5,  19,  50,  37,  37,   7,  -2,
            -6,  13,  13,  26,  34,  12,  10,   4,
            0,  15,  15,  15,  14,  27,  18,  10,
            4,  15,  16,   0,   7,  21,  33,   1,
            -33,  -3, -14, -21, -13, -12, -39, -21,
    };

    int eg_bishop_table[64] = {
            -14, -21, -11,  -8, -7,  -9, -17, -24,
            -8,  -4,   7, -12, -3, -13,  -4, -14,
            2,  -8,   0,  -1, -2,   6,   0,   4,
            -3,   9,  12,   9, 14,  10,   3,   2,
            -6,   3,  13,  19,  7,  10,  -3,  -9,
            -12,  -3,   8,  10, 13,   3,  -7, -15,
            -14, -18,  -7,  -1,  4,  -9, -15, -27,
            -23,  -9, -23,  -5, -9, -16,  -5, -17,
    };

    int mg_rook_table[64] = {
            32,  42,  32,  51, 63,  9,  31,  43,
            27,  32,  58,  62, 80, 67,  26,  44,
            -5,  19,  26,  36, 17, 45,  61,  16,
            -24, -11,   7,  26, 24, 35,  -8, -20,
            -36, -26, -12,  -1,  9, -7,   6, -23,
            -45, -25, -16, -17,  3,  0,  -5, -33,
            -44, -16, -20,  -9, -1, 11,  -6, -71,
            -19, -13,   1,  17, 16,  7, -37, -26,
    };

    int eg_rook_table[64] = {
            13, 10, 18, 15, 12,  12,   8,   5,
            11, 13, 13, 11, -3,   3,   8,   3,
            7,  7,  7,  5,  4,  -3,  -5,  -3,
            4,  3, 13,  1,  2,   1,  -1,   2,
            3,  5,  8,  4, -5,  -6,  -8, -11,
            -4,  0, -5, -1, -7, -12,  -8, -16,
            -6, -6,  0,  2, -9,  -9, -11,  -3,
            -9,  2,  3, -1, -5, -13,   4, -20,
    };

    int mg_queen_table[64] = {
            -28,   0,  29,  12,  59,  44,  43,  45,
            -24, -39,  -5,   1, -16,  57,  28,  54,
            -13, -17,   7,   8,  29,  56,  47,  57,
            -27, -27, -16, -16,  -1,  17,  -2,   1,
            -9, -26,  -9, -10,  -2,  -4,   3,  -3,
            -14,   2, -11,  -2,  -5,   2,  14,   5,
            -35,  -8,  11,   2,   8,  15,  -3,   1,
            -1, -18,  -9,  10, -15, -25, -31, -50,
    };

    int eg_queen_table[64] = {
            -9,  22,  22,  27,  27,  19,  10,  20,
            -17,  20,  32,  41,  58,  25,  30,   0,
            -20,   6,   9,  49,  47,  35,  19,   9,
            3,  22,  24,  45,  57,  40,  57,  36,
            -18,  28,  19,  47,  31,  34,  39,  23,
            -16, -27,  15,   6,   9,  17,  10,   5,
            -22, -23, -30, -16, -16, -23, -36, -32,
            -33, -28, -22, -43,  -5, -32, -20, -41,
    };

    int mg_king_table[64] = {
            -65,  23,  16, -15, -56, -34,   2,  13,
            29,  -1, -20,  -7,  -8,  -4, -38, -29,
            -9,  24,   2, -16, -20,   6,  22, -22,
            -17, -20, -12, -27, -30, -25, -14, -36,
            -49,  -1, -27, -39, -46, -44, -33, -51,
            -14, -14, -22, -46, -44, -30, -15, -27,
            1,   7,  -8, -64, -43, -16,   9,   8,
            -15,  36,  12, -54,   8, -28,  24,  14,
    };

    int eg_king_table[64] = {
            -74, -35, -18, -18, -11,  15,   4, -17,
            -12,  17,  14,  17,  17,  38,  23,  11,
            10,  17,  23,  15,  20,  45,  44,  13,
            -8,  22,  24,  27,  26,  33,  26,   3,
            -18,  -4,  21,  24,  27,  23,   9, -11,
            -19,  -3,  11,  21,  23,  16,   7,  -9,
            -27, -11,   4,  13,  14,   4,  -5, -17,
            -53, -34, -21, -11, -28, -14, -24, -43
    };

    int *mg_tables[6] = {
            mg_pawn_table,
            mg_knight_table,
            mg_bishop_table,
            mg_rook_table,
            mg_queen_table,
            mg_king_table,
    };

    int *eg_tables[6] = {
            eg_pawn_table,
            eg_knight_table,
            eg_bishop_table,
            eg_rook_table,
            eg_queen_table,
            eg_king_table,
    };


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

    U64 construct_bishop_attacks(int square, U64 blockers);
    U64 construct_rook_attacks(int square, U64 blockers);
    U64 get_rook_attacks(int square, U64 occupancy);
    U64 get_bishop_attacks(int square, U64 occupancy);
    U64 get_queen_attacks(int square, U64 occupancy);
    int is_square_attacked(int square, int side);
    int bit_count(U64 board);
    int get_ls1b_index(U64 board);
    U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask);
    void fill_occupancies();
    void generate_moves(moves *moves_list);
    U64 occupancies[3];
    int make_move(int move, int move_flag);
    U64 bitboards[12];
    void perft_test(int depth);
    int evaluate();
    int search(int depth);
    int negamax(int alpha, int beta, int depth);
    void play();
    int coordinate_to_square(char *coordinate);
private:
    void perft_test_helper(int depth);

    //Functions for getting each pieces valid moves (takes whose turn it should calculate for)
    //State for our pseudo random number generator
    //All of our bitboards
    void construct_pawn_tables();
    void construct_knight_masks();
    void construct_king_tables();
    void construct_bishop_tables();
    void construct_rook_tables();
    void construct_slider_attacks();
    void add_move(moves *moves_list, int move);
    void clear_moves();
    void print_moves(moves *moves_list);
//    U64 occupancies[3];
    int side = white;
    int enpassant = no_square;
    int castle = 0;
    int half_moves = 0;
    int full_moves = 0;

};
#endif //BITBOT_BOARD_H
