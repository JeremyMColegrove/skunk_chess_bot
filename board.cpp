//
// Core board operations for the Skunk chess engine
// Includes: initialization, move generation, make_move, bitboard utilities
//

#include "board.h"
#include <iostream>
#include <sstream>
#include <cmath>
#include <algorithm>


/*****************************\
===============================
         initialization
===============================
\*****************************/

void Skunk::parse_fen(const std::string& fen) {
    // reset board position (bitboards)
    memset(bitboards, 0ULL, sizeof(bitboards));

    // reset occupancies (bitboards)
    memset(occupancies, 0ULL, sizeof(occupancies));
    memset(piece_count, 0, sizeof(piece_count));

    // reset game state variables
    side = 0;
    enpassant = no_square;
    castle = 0;

    int square = 0;
    size_t fen_idx = 0;

    // loop over board squares
    while (square < 64 && fen_idx < fen.length()) {
        if (fen[fen_idx] == '/') {
            fen_idx++;
            square--;
        } else if (fen[fen_idx] >= '0' && fen[fen_idx] <= '9') {
            square += fen[fen_idx] - '1';
            fen_idx++;
        } else {
            int piece = char_pieces[fen[fen_idx]];
            set_bit(bitboards[piece], square);
            piece_count[piece]++;
            fen_idx++;
        }
        square++;
    }
    side = (fen[++fen_idx] == 'w') ? white : black;
    fen_idx += 2;

    while (fen[fen_idx] != ' ' && fen_idx < fen.length()) {
        switch (fen[fen_idx++]) {
            case 'K': castle |= wk; break;
            case 'Q': castle |= wq; break;
            case 'k': castle |= bk; break;
            case 'q': castle |= bq; break;
        }
    }

    if (fen[++fen_idx] != '-') {
        int file = fen[fen_idx++] - 'a';
        int rank = 8 - (fen[fen_idx] - '0');
        enpassant = rank * 8 + file;
    } else {
        enpassant = no_square;
    }

    for (int piece = P; piece <= k; piece++) {
        occupancies[piece <= K ? white : black] |= bitboards[piece];
    }

    occupancies[both] = occupancies[white] | occupancies[black];
    zobrist = generate_zobrist();
    init();
}

void Skunk::fill_occupancies() {
    for (int piece = P; piece <= K; piece++)
        occupancies[white] |= bitboards[piece];

    for (int piece = p; piece <= k; piece++)
        occupancies[black] |= bitboards[piece];

    occupancies[both] |= occupancies[white];
    occupancies[both] |= occupancies[black];
}

Skunk::Skunk() {
    memset(bitboards, 0ULL, sizeof(bitboards));
    memset(occupancies, 0ULL, sizeof(occupancies));
    construct_pawn_tables();
    construct_knight_masks();
    construct_king_tables();
    construct_bishop_tables();
    construct_rook_tables();
    construct_slider_attacks();
    construct_rays();
    construct_direction_rays();
    construct_file_masks();
    side = white;
    castle = 0;
}

void Skunk::init() {
    seed = 4091583267;
    fill_occupancies();
    repitition.count = 0;

    for (int piece = P; piece <= k; piece++) {
        for (int board = 0; board < 64; board++) {
            piece_keys[piece][board] = get_random_U64_number();
        }
    }
    for (int board = 0; board < 64; board++) {
        enpassant_keys[board] = get_random_U64_number();
    }

    for (int i = 0; i < 16; i++) {
        castle_keys[i] = get_random_U64_number();
    }

    side_key = get_random_U64_number();
    zobrist = generate_zobrist();
    init_precomputed_masks();
    init_heuristics();
}

unsigned int Skunk::get_random_U32_number() {
    unsigned int number = seed;
    number ^= number << 13;
    number ^= number >> 17;
    number ^= number << 5;
    seed = number;
    return number;
}

U64 Skunk::generate_zobrist() {
    U64 hash = 0ULL;
    U64 bitboard;

    for (int piece = P; piece <= k; piece++) {
        bitboard = bitboards[piece];
        while (bitboard) {
            int square = __builtin_ctzll(bitboard);
            hash ^= piece_keys[piece][square];
            pop_bit(bitboard, square);
        }
    }

    if (enpassant != no_square) {
        hash ^= enpassant_keys[enpassant];
    }

    hash ^= castle_keys[castle];

    if (side == black) hash ^= side_key;

    return hash;
}

U64 Skunk::get_random_U64_number() {
    U64 n1, n2, n3, n4;
    n1 = (U64)(get_random_U32_number()) & 0xFFFF;
    n2 = (U64)(get_random_U32_number()) & 0xFFFF;
    n3 = (U64)(get_random_U32_number()) & 0xFFFF;
    n4 = (U64)(get_random_U32_number()) & 0xFFFF;
    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

Skunk::~Skunk() {}

void Skunk::construct_file_masks() {
    memset(file_masks, 0ULL, sizeof(file_masks));
    U64 mask;

    for (int square = 0; square < 64; square++) {
        mask = 0ULL;
        for (int row = 0; row < 8; row++) {
            set_bit(mask, row * 8 + (square & 7));
        }
        file_masks[MIDDLE][square] = mask;
    }

    for (int square = 0; square < 64; square++) {
        mask = 0ULL;
        if ((square & 7) == 0) continue;
        for (int row = 0; row < 8; row++) {
            set_bit(mask, row * 8 + ((square - 1) & 7));
        }
        file_masks[LEFT][square] = mask;
    }

    for (int square = 0; square < 64; square++) {
        mask = 0ULL;
        if ((square & 7) == 7) continue;
        for (int row = 0; row < 8; row++) {
            set_bit(mask, row * 8 + ((square + 1) & 7));
        }
        file_masks[RIGHT][square] = mask;
    }
}

void Skunk::construct_direction_rays() {
    for (int i = 0; i < 64; i++) {
        nearest_square[DE][i] = i + (7 - (i & 7));
        nearest_square[DN][i] = i & 7;
        nearest_square[DW][i] = i - (i & 7);
        nearest_square[DS][i] = 56 + (i & 7);
        nearest_square[DNE][i] = i - std::min(i >> 3, 7 - (i & 7)) * 7;
        nearest_square[DSE][i] = i + std::min(7 - (i >> 3), 7 - (i & 7)) * 9;
        nearest_square[DSW][i] = i + std::min(7 - (i >> 3), (i & 7)) * 7;
        nearest_square[DNW][i] = i - std::min(i >> 3, (i & 7)) * 9;
    }
}

void Skunk::construct_rays() {
    memset(rays, 0ULL, sizeof(rays));
    U64 ray = 0ULL;
    for (int i = 0; i < 64; i++) {
        int left_square = (i >> 3) * 8;
        for (int j = left_square; j < i; j++) {
            ray = 0ULL;
            int square = j;
            while (square < i - 1) {
                square++;
                set_bit(ray, square);
            }
            rays[j][i] = ray;
            rays[i][j] = ray;
        }

        int top_square = i & 7;
        for (int j = top_square; j < i; j += 8) {
            ray = 0ULL;
            int square = j;
            while (square < i - 8) {
                square += 8;
                set_bit(ray, square);
            }
            rays[i][j] = ray;
            rays[j][i] = ray;
        }

        int l_diag = std::min(i & 7, i >> 3);
        for (int j = 1; j <= l_diag; j++) {
            ray = 0ULL;
            int square = i - 9 * (j - 1);
            while (square < i) {
                set_bit(ray, square);
                square += 9;
            }
            rays[i][i - 9 * j] = ray;
            rays[i - 9 * j][i] = ray;
        }

        int r_diag = std::min(7 - (i & 7), i >> 3);
        for (int j = 1; j <= r_diag; j++) {
            ray = 0ULL;
            int square = i - 7 * (j - 1);
            while (square < i) {
                set_bit(ray, square);
                square += 7;
            }
            rays[i][i - 7 * j] = ray;
            rays[i - 7 * j][i] = ray;
        }
    }
}

void Skunk::construct_pawn_tables() {
    for (int square = 0; square < 64; square++) {
        U64 white_board = 0UL;
        U64 black_board = 0UL;
        U64 white_attacks = 0UL;
        U64 black_attacks = 0UL;
        set_bit(white_board, square);
        set_bit(black_board, square);

        if (white_board >> 7 & not_a_file) white_attacks |= white_board >> 7;
        if (white_board >> 9 & not_h_file) white_attacks |= white_board >> 9;

        if (black_board << 7 & not_h_file) black_attacks |= black_board << 7;
        if (black_board << 9 & not_a_file) black_attacks |= black_board << 9;

        pawn_masks[white][square] = white_attacks;
        pawn_masks[black][square] = black_attacks;
    }
}

void Skunk::construct_knight_masks() {
    for (int square = 0; square < 64; square++) {
        U64 white_board = 0UL;
        U64 attacks = 0UL;
        set_bit(white_board, square);

        if (white_board >> 15 & not_a_file) attacks |= white_board >> 15;
        if (white_board >> 17 & not_h_file) attacks |= white_board >> 17;
        if (white_board >> 6 & not_ab_file) attacks |= white_board >> 6;
        if (white_board >> 10 & not_gh_file) attacks |= white_board >> 10;
        if (white_board << 15 & not_h_file) attacks |= white_board << 15;
        if (white_board << 17 & not_a_file) attacks |= white_board << 17;
        if (white_board << 6 & not_gh_file) attacks |= white_board << 6;
        if (white_board << 10 & not_ab_file) attacks |= white_board << 10;

        knight_masks[square] = attacks;
    }
}

void Skunk::init_precomputed_masks() {
    for (int square = 0; square < 64; square++) {
        pawn_attack_span_masks[white][square] = pawn_attack_span(white, square);
        pawn_attack_span_masks[black][square] = pawn_attack_span(black, square);
    }
}

U64 Skunk::pawn_attack_span(int color, int square) {
    U64 attacks = 0ULL;
    int file = square & 7;

    if (color == white) {
        if (file > 0) {
            attacks |= rays[square - 1][nearest_square[DN][square - 1]] & not_h_file;
        }
        if (file < 7) {
            attacks |= rays[square + 1][nearest_square[DN][square + 1]] & not_a_file;
        }
    } else {
        if (file > 0) {
            attacks |= rays[square - 1][nearest_square[DS][square - 1]] & not_h_file;
        }
        if (file < 7) {
            attacks |= rays[square + 1][nearest_square[DS][square + 1]] & not_a_file;
        }
    }

    return attacks;
}

void Skunk::construct_king_tables() {
    for (int square = 0; square < 64; square++) {
        U64 white_board = 0UL;
        U64 attacks = 0UL;
        set_bit(white_board, square);

        if (white_board >> 1 & not_h_file) attacks |= white_board >> 1;
        if (white_board >> 9 & not_h_file) attacks |= white_board >> 9;
        if (white_board >> 7 & not_a_file) attacks |= white_board >> 7;
        if (white_board << 1 & not_a_file) attacks |= white_board << 1;
        if (white_board << 9 & not_a_file) attacks |= white_board << 9;
        if (white_board << 7 & not_h_file) attacks |= white_board << 7;
        attacks |= white_board >> 8;
        attacks |= white_board << 8;

        king_masks[square] = attacks;
    }
}

void Skunk::construct_bishop_tables() {
    for (int square = 0; square < 64; square++) {
        U64 attacks = 0UL;
        int tr = square >> 3;
        int tf = square & 7;
        for (int r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++) set_bit(attacks, r * 8 + f);
        for (int r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++) set_bit(attacks, r * 8 + f);
        for (int r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--) set_bit(attacks, r * 8 + f);
        for (int r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--) set_bit(attacks, r * 8 + f);
        bishop_masks[square] = attacks;
    }
}

void Skunk::construct_rook_tables() {
    for (int square = 0; square < 64; square++) {
        U64 attacks = 0UL;
        int tr = square & 7;
        int tf = square >> 3;
        for (int f = tf - 1; f >= 1; f--) set_bit(attacks, f * 8 + tr);
        for (int f = tf + 1; f <= 6; f++) set_bit(attacks, f * 8 + tr);
        for (int r = tr + 1; r <= 6; r++) set_bit(attacks, tf * 8 + r);
        for (int r = tr - 1; r >= 1; r--) set_bit(attacks, tf * 8 + r);
        rook_masks[square] = attacks;
    }
}

U64 Skunk::construct_bishop_attacks(int square, unsigned long long int blockers) {
    U64 moves = 0UL;
    int tr = square >> 3;
    int tf = square & 7;
    for (int r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++) {
        set_bit(moves, r * 8 + f);
        if (get_bit(blockers, r * 8 + f)) break;
    }
    for (int r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++) {
        set_bit(moves, r * 8 + f);
        if (get_bit(blockers, r * 8 + f)) break;
    }
    for (int r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--) {
        set_bit(moves, r * 8 + f);
        if (get_bit(blockers, r * 8 + f)) break;
    }
    for (int r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--) {
        set_bit(moves, r * 8 + f);
        if (get_bit(blockers, r * 8 + f)) break;
    }
    return moves;
}

U64 Skunk::construct_rook_attacks(int square, U64 blockers) {
    U64 attacks = 0UL;
    int tr = square & 7;
    int tf = square >> 3;
    for (int f = tf - 1; f >= 0; f--) {
        set_bit(attacks, f * 8 + tr);
        if (get_bit(blockers, f * 8 + tr)) break;
    }
    for (int f = tf + 1; f <= 7; f++) {
        set_bit(attacks, f * 8 + tr);
        if (get_bit(blockers, f * 8 + tr)) break;
    }
    for (int r = tr + 1; r <= 7; r++) {
        set_bit(attacks, tf * 8 + r);
        if (get_bit(blockers, tf * 8 + r)) break;
    }
    for (int r = tr - 1; r >= 0; r--) {
        set_bit(attacks, tf * 8 + r);
        if (get_bit(blockers, tf * 8 + r)) break;
    }
    return attacks;
}

void Skunk::construct_slider_attacks() {
    for (int square = 0; square < 64; square++) {
        U64 rook_attack = rook_masks[square];
        U64 bishop_attack = bishop_masks[square];
        int rook_relevant_bit_count = bit_count(rook_attack);
        int bishop_relevant_bit_count = bit_count(bishop_attack);
        int rook_occupancy_indicies = (1 << rook_relevant_bit_count);
        int bishop_occupancy_indicies = (1 << bishop_relevant_bit_count);

        for (int index = 0; index < rook_occupancy_indicies; index++) {
            U64 rook_occupancy = set_occupancy(index, rook_relevant_bit_count, rook_attack);
            int magic_index = (rook_occupancy * rook_magic_numbers[square]) >> (64 - rook_relevant_bits[square]);
            rook_attacks[square][magic_index] = construct_rook_attacks(square, rook_occupancy);
        }

        for (int index = 0; index < bishop_occupancy_indicies; index++) {
            U64 bishop_occupancy = set_occupancy(index, bishop_relevant_bit_count, bishop_attack);
            int magic_index = (bishop_occupancy * bishop_magic_numbers[square]) >> (64 - bishop_relevant_bits[square]);
            bishop_attacks[square][magic_index] = construct_bishop_attacks(square, bishop_occupancy);
        }
    }
}

U64 Skunk::get_rook_attacks(int square, U64 occupancy) {
    occupancy &= rook_masks[square];
    occupancy *= rook_magic_numbers[square];
    occupancy >>= 64 - rook_relevant_bits[square];
    return rook_attacks[square][occupancy];
}

U64 Skunk::get_bishop_attacks(int square, U64 occupancy) {
    occupancy &= bishop_masks[square];
    occupancy *= bishop_magic_numbers[square];
    occupancy >>= 64 - bishop_relevant_bits[square];
    return bishop_attacks[square][occupancy];
}

U64 Skunk::get_queen_attacks(int square, U64 occupancy) {
    return get_rook_attacks(square, occupancy) | get_bishop_attacks(square, occupancy);
}

bool Skunk::is_square_attacked(int square, int side) {
#ifdef DEBUG
    if (side != 0 && side != 1) {
        printf("Weird, side is %d\n", side);
    }
    assert(side == 0 || side == 1);
    assert(square > -1 && square < 64);
#endif

    if ((side == white) && (pawn_masks[black][square] & bitboards[P])) return true;
    if ((side == black) && (pawn_masks[white][square] & bitboards[p])) return true;
    if (knight_masks[square] & ((side == white) ? bitboards[N] : bitboards[n])) return true;
    if (get_bishop_attacks(square, occupancies[both]) & ((side == white) ? bitboards[B] : bitboards[b])) return true;
    if (get_rook_attacks(square, occupancies[both]) & ((side == white) ? bitboards[R] : bitboards[r])) return true;
    if (get_queen_attacks(square, occupancies[both]) & ((side == white) ? bitboards[Q] : bitboards[q])) return true;
    if (king_masks[square] & ((side == white) ? bitboards[K] : bitboards[k])) return true;

    return false;
}

/*****************************\
===============================
           printing
===============================
\*****************************/

void Skunk::print_bitboard(U64 board) {
    for (int i = 0; i < 64; i++) {
        if ((i & 7) == 0) {
            std::cout << std::endl;
            std::cout << 8 - (i / 8) << "\t\t";
        }
        if (get_bit(board, i)) std::cout << "1  ";
        else std::cout << "0  ";
    }
    std::cout << "\n\n\t\ta  b  c  d  e  f  g  h" << std::endl;
    std::cout << "\t\tBoard number: " << board << std::endl;
}

void Skunk::print_attacks(int side) {
    for (int i = 0; i < 64; i++) {
        if ((i & 7) == 0) {
            std::cout << std::endl;
            std::cout << 8 - (i / 8) << "\t\t";
        }
        if (is_square_attacked(i, side)) std::cout << "1  ";
        else std::cout << "0  ";
    }
    std::cout << "\n\n\t\ta  b  c  d  e  f  g  h" << std::endl;
}

void Skunk::print_board() {
    std::cout << std::endl;
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            if (!file) {
                std::cout << 8 - rank << "   ";
            }
            int piece = -1;
            for (int count = P; count <= k; count++) {
                if (get_bit(bitboards[count], square)) {
                    piece = count;
                }
            }
#ifdef _WIN32
            printf(" %c", (piece == -1) ? '.' : ascii_pieces[piece]);
#else
            printf(" %s ", (piece == -1) ? "." : unicode_pieces[piece]);
#endif
        }
        std::cout << std::endl;
    }
    std::cout << "\n     " << "a  b  c  d  e  f  g  h" << std::endl;
    std::cout << "\nSide: " << (side == white ? "white" : "black") << std::endl;
    printf("Enpassant: %s\n", enpassant == no_square ? "None" : square_to_coordinate[enpassant]);
    std::cout << "\nCastling: " << ((castle & wk) ? 'K' : '-') << ((castle & wq) ? 'Q' : '-')
              << ((castle & bk) ? 'k' : '-') << ((castle & bq) ? 'q' : '-') << std::endl;
    printf("Positional score: %d\n", evaluate());
    printf("Ply: %d\n", ply);
}

/*****************************\
===============================
        bit helpers
===============================
\*****************************/

int Skunk::bit_count(U64 board) {
    int count = 0;
    while (board) {
        count++;
        board &= board - 1;
    }
    return count;
}

U64 Skunk::set_occupancy(int index, int bits_in_mask, U64 attack_mask) {
    U64 occupancy = 0ULL;
    for (int i = 0; i < bits_in_mask; i++) {
        int square = __builtin_ctzll(attack_mask);
        pop_bit(attack_mask, square);
        if (index & (1ULL << i)) {
            occupancy |= 1ULL << square;
        }
    }
    return occupancy;
}

/*****************************\
===============================
        move generation
===============================
\*****************************/

U64 Skunk::get_slider_attacks() {
    U64 pieces;
    U64 sliders = 0ULL;
    int knight = N, bishop = B, rook = R, queen = Q;
    if (side == white) {
        knight = n, bishop = b, rook = r, queen = q;
    }

    pieces = bitboards[bishop];
    while (pieces) {
        sliders |= get_bishop_attacks(__builtin_ctzll(pieces), occupancies[both]);
        pop_lsb(pieces);
    }

    pieces = bitboards[rook];
    while (pieces) {
        sliders |= get_rook_attacks(__builtin_ctzll(pieces), occupancies[both]);
        pop_lsb(pieces);
    }

    pieces = bitboards[queen];
    while (pieces) {
        sliders |= get_rook_attacks(__builtin_ctzll(pieces), occupancies[both]);
        sliders |= get_bishop_attacks(__builtin_ctzll(pieces), occupancies[both]);
        pop_lsb(pieces);
    }

    return sliders;
}

U64 Skunk::get_jumper_attacks() {
    U64 pieces;
    U64 jumpers = 0ULL;
    int pawn = P, knight = N, king = K;
    if (side == white) {
        pawn = p, knight = n, king = k;
    }

    pieces = bitboards[pawn];
    while (pieces) {
        jumpers |= pawn_masks[side ^ 1][__builtin_ctzll(pieces)];
        pop_lsb(pieces);
    }

    pieces = bitboards[knight];
    while (pieces) {
        jumpers |= knight_masks[__builtin_ctzll(pieces)];
        pop_lsb(pieces);
    }

    pieces = bitboards[king];
    while (pieces) {
        jumpers |= king_masks[__builtin_ctzll(pieces)];
        pop_lsb(pieces);
    }
    return jumpers;
}

void Skunk::generate_moves(t_moves& moves_list) {
    moves_list.count = 0;

    U64 pieces = 0ULL;
    U64 attacked_squares, attack_sliders, attack_jumpers;
    int square;

    int pawn, knight, bishop, rook, queen, king;
    U64* opponent_bitboards;
    if (side == black) {
        pawn = p, knight = n, bishop = b, rook = r, queen = q, king = k;
        opponent_bitboards = bitboards;
    } else {
        pawn = P, knight = N, bishop = B, rook = R, queen = Q, king = K;
        opponent_bitboards = bitboards + 6;
    }

    if (bitboards[K] == 0 || bitboards[k] == 0) {
        std::cout << "KING IS OFF OF THE BOARD" << std::endl;
    }

    int king_square = __builtin_ctzll(bitboards[king]);

    pop_bit(occupancies[both], king_square);
    attack_sliders = get_slider_attacks();
    attack_jumpers = get_jumper_attacks();
    attacked_squares = attack_jumpers | attack_sliders;
    set_bit(occupancies[both], king_square);

    pieces = king_masks[__builtin_ctzll(bitboards[king])] & ~occupancies[side] & ~attacked_squares;
    while (pieces) {
        square = __builtin_ctzll(pieces);
        moves_list.moves[moves_list.count++] = encode_move(__builtin_ctzll(bitboards[king]), square, king, 0, 0, 0);
        pop_lsb(pieces);
    }

    attack_sliders = get_slider_attacks();
    attack_jumpers = get_jumper_attacks();
    attacked_squares = attack_jumpers | attack_sliders;

    U64 capture_mask = 0ULL;
    U64 push_mask = 0ULL;

    capture_mask |= get_bishop_attacks(king_square, occupancies[both]) & (opponent_bitboards[B] | opponent_bitboards[Q]);
    capture_mask |= get_rook_attacks(king_square, occupancies[both]) & (opponent_bitboards[R] | opponent_bitboards[Q]);

    U64 sliders = capture_mask;
    while (sliders) {
        square = __builtin_ctzll(sliders);
        push_mask |= rays[square][king_square];
        pop_lsb(sliders);
    }

    capture_mask |= knight_masks[king_square] & opponent_bitboards[N];
    capture_mask |= pawn_masks[side][king_square] & opponent_bitboards[P];

    if (capture_mask == 0 && push_mask == 0) {
        capture_mask = 0xFFFFFFFFFFFFFFFF;
        push_mask = 0xFFFFFFFFFFFFFFFF;
    } else {
        int count = __builtin_popcountll(capture_mask);
        if (count > 1) {
            return;
        }
    }

    U64 unpinned_pieces[12];
    memcpy(unpinned_pieces, bitboards, 12 * sizeof(U64));

    U64 slider_king = get_bishop_attacks(king_square, occupancies[both]) | get_rook_attacks(king_square, occupancies[both]);
    int pinner_pieces[] = {R, B, Q};
    int pinner_piece, enemy_square, pinned_square, piece, destination;

    for (int direction = 0; direction < 8; direction++) {
        U64 king_ray = slider_king & rays[king_square][nearest_square[7 - direction][king_square]];

        for (int piece_index = 0; piece_index < 3; piece_index++) {
            pinner_piece = pinner_pieces[piece_index];
            pieces = opponent_bitboards[pinner_piece];
            while (pieces != 0) {
                enemy_square = __builtin_ctzll(pieces);
                if (pieces) pop_lsb(pieces);

                U64 intersection = get_attacks(pinner_piece, enemy_square, side ^ 1);
                int nearest_sq = nearest_square[direction][enemy_square];

                intersection &= rays[enemy_square][nearest_sq];
                intersection &= king_ray;
                intersection &= occupancies[side];
                if (intersection == 0) continue;

                pinned_square = __builtin_ctzll(intersection);
                piece = get_piece(pinned_square);

                pop_bit(unpinned_pieces[piece], pinned_square);
                U64 attacks = get_attacks(piece, pinned_square, side) & (rays[king_square][enemy_square] | (1ULL << enemy_square)) & (push_mask | capture_mask);

                while (attacks) {
                    destination = __builtin_ctzll(attacks);
                    moves_list.moves[moves_list.count++] = encode_move(pinned_square, destination, piece, 0, 0, 0);
                    pop_lsb(attacks);
                }
            }
        }
    }

    U64 attacks;
    U64 filtered_capture_mask = capture_mask, filtered_push_mask = push_mask;

    if (enpassant != no_square) {
        filtered_capture_mask |= (1ULL << enpassant);
        filtered_push_mask |= (1ULL << enpassant);
    }

    pieces = unpinned_pieces[pawn];
    while (pieces) {
        square = __builtin_ctzll(pieces);
        attacks = get_attacks(pawn, square, side) & (filtered_capture_mask | filtered_push_mask);

        U64 filtered = attacks & ~row8 & ~row1;

        while (filtered) {
            int destination = __builtin_ctzll(filtered);
            if (destination == enpassant) {
                int victim = enpassant + 8 * (side == white ? 1 : -1);
                pop_bit(occupancies[both], square);
                pop_bit(occupancies[both], victim);
                set_bit(occupancies[both], enpassant);

                int lsq = nearest_square[DW][king_square], rsq = nearest_square[DE][king_square];
                int usq = nearest_square[DN][king_square], dsq = nearest_square[DS][king_square];
                U64 horizontal_mask = rays[king_square][lsq] | rays[king_square][rsq] | rays[king_square][usq] | rays[king_square][dsq] | (1ULL << lsq) | (1ULL << rsq) | (1ULL << usq) | (1ULL << dsq) | (1ULL << king_square);
                U64 king_possible_pin_masks = get_queen_attacks(king_square, occupancies[both]) | (1ULL << king_square);

                U64 horizontal_attackers = (opponent_bitboards[R] | opponent_bitboards[Q]) & (king_possible_pin_masks & horizontal_mask);
                U64 diagonal_attackers = (opponent_bitboards[B] | opponent_bitboards[Q]) & (king_possible_pin_masks & ~horizontal_mask);

                U64 sliders = 0ULL;
                U64 diagonals = 0ULL;

                while (horizontal_attackers) {
                    int horizontal_attacker_square = __builtin_ctzll(horizontal_attackers);
                    sliders |= get_rook_attacks(horizontal_attacker_square, occupancies[both]);
                    pop_lsb(horizontal_attackers);
                }

                while (diagonal_attackers) {
                    int diagonal_attacker_square = __builtin_ctzll(diagonal_attackers);
                    diagonals |= get_bishop_attacks(diagonal_attacker_square, occupancies[both]);
                    pop_lsb(diagonal_attackers);
                }

                if (((sliders | diagonals) & bitboards[king]) == 0) {
                    moves_list.moves[moves_list.count++] = encode_move(square, destination, pawn, 0, 1, 0);
                }

                pop_bit(occupancies[both], enpassant);
                set_bit(occupancies[both], square);
                set_bit(occupancies[both], victim);
            } else {
                moves_list.moves[moves_list.count++] = encode_move(square, destination, pawn, 0, 0, 0);
            }
            pop_lsb(filtered);
        }

        filtered = attacks & (row8 | row1);
        while (filtered) {
            int destination = __builtin_ctzll(filtered);
            moves_list.moves[moves_list.count++] = encode_move(square, destination, pawn, rook, 0, 0);
            moves_list.moves[moves_list.count++] = encode_move(square, destination, pawn, bishop, 0, 0);
            moves_list.moves[moves_list.count++] = encode_move(square, destination, pawn, queen, 0, 0);
            moves_list.moves[moves_list.count++] = encode_move(square, destination, pawn, knight, 0, 0);
            pop_lsb(filtered);
        }
        pop_lsb(pieces);
    }

    int regular_pieces[] = {rook, queen, bishop, knight};
    for (int i = 0; i < 4; i++) {
        piece = regular_pieces[i];
        pieces = unpinned_pieces[piece];

        while (pieces) {
            square = __builtin_ctzll(pieces);
            attacks = get_attacks(piece, square, side) & (capture_mask | push_mask);
            while (attacks) {
                destination = __builtin_ctzll(attacks);
                moves_list.moves[moves_list.count++] = encode_move(square, destination, piece, 0, 0, 0);
                pop_lsb(attacks);
            }
            pop_lsb(pieces);
        }
    }

    if (side == white && (attacked_squares & bitboards[K]) == 0) {
        if (castle & wk && ((attacked_squares | occupancies[both]) & castle_mask_wk) == 0) {
            moves_list.moves[moves_list.count++] = encode_move(e1, g1, K, 0, 0, 1);
            moves_list.contains_castle = 1;
        }
        if (castle & wq && (attacked_squares & castle_attack_mask_wq) == 0 && (occupancies[both] & castle_piece_mask_wq) == 0) {
            moves_list.moves[moves_list.count++] = encode_move(e1, c1, K, 0, 0, 1);
            moves_list.contains_castle = 1;
        }
    } else if (side == black && (attacked_squares & bitboards[k]) == 0) {
        if (castle & bk && ((attacked_squares | occupancies[both]) & castle_mask_bk) == 0) {
            moves_list.moves[moves_list.count++] = encode_move(e8, g8, k, 0, 0, 1);
            moves_list.contains_castle = 1;
        }
        if (castle & bq && (attacked_squares & castle_attack_mask_bq) == 0 && (occupancies[both] & castle_piece_mask_bq) == 0) {
            moves_list.moves[moves_list.count++] = encode_move(e8, c8, k, 0, 0, 1);
            moves_list.contains_castle = 1;
        }
    }
}

int Skunk::get_piece(int square) {
    int type = -1;
    U64 mask = (1ULL << square);
    for (int piece = P; piece <= k; piece++) {
        if (bitboards[piece] & mask) {
            type = piece;
            break;
        }
    }
    return type;
}

U64 Skunk::get_attacks(int piece, int square, int side) {
    U64 attacks = 0ULL;
    U64 piece_bitboard = (1ULL << square);

    switch (piece) {
        case r:
        case R:
            return get_rook_attacks(square, occupancies[both]) & ~occupancies[side];
        case b:
        case B:
            return get_bishop_attacks(square, occupancies[both]) & ~occupancies[side];
        case q:
        case Q:
            return (get_bishop_attacks(square, occupancies[both]) | get_rook_attacks(square, occupancies[both])) & ~occupancies[side];
        case p:
            attacks |= ((piece_bitboard << 8) & ~occupancies[both]) | (pawn_masks[side][square] & occupancies[side ^ 1]);
            attacks |= ((((piece_bitboard & row7) << 8) & ~occupancies[both]) << 8) & ~occupancies[both];
            if (enpassant != no_square) {
                attacks |= pawn_masks[side][square] & (1ULL << enpassant);
            }
            return attacks;
        case P:
            attacks |= ((piece_bitboard >> 8) & ~occupancies[both]) | (pawn_masks[side][square] & occupancies[side ^ 1]);
            attacks |= ((((piece_bitboard & row2) >> 8) & ~occupancies[both]) >> 8) & ~occupancies[both];
            if (enpassant != no_square) {
                attacks |= pawn_masks[side][square] & (1ULL << enpassant);
            }
            return attacks;
        case n:
        case N:
            return knight_masks[square] & ~occupancies[side];
        case k:
        case K:
            return king_masks[square] & ~occupancies[side];
    }
    return attacks;
}

void Skunk::add_move(t_moves& moves_list, int move) {
    moves_list.moves[moves_list.count] = move;
    moves_list.count++;
}

void Skunk::print_moves(t_moves& moves_list) {
    printf("%-9s %-6s %-8s %-7s %-9s %-9s %-9s\n", "num", "source", "target", "piece", "score", "enpassant", "castle");

    for (int i = 0; i < moves_list.count; i++) {
        int move = moves_list.moves[i];
        printf("%-9d %-6s %-8s %-7c %-9d %-9d %-9d\n",
               move,
               square_to_coordinate[decode_source(move)],
               square_to_coordinate[decode_destination(move)],
               ascii_pieces[decode_piece(move)],
               score_move(move),
               decode_enpassant(move),
               decode_castle(move));
    }
    printf("Total of %d moves.\n", moves_list.count);
}

bool Skunk::is_check() {
    return is_square_attacked(__builtin_ctzll(bitboards[side == white ? K : k]), side ^ 1);
}

int Skunk::is_repetition() {
    for (int i = repitition.count - 2; i >= 0; i--) {
        if (repitition.table[i] == zobrist) {
            return 1;
        }
    }
    return 0;
}

int Skunk::make_move(int move, int move_flag) {
    if (move_flag == all_moves) {
        int source = decode_source(move);
        int target = decode_destination(move);
        int enp = decode_enpassant(move);
        int castling = decode_castle(move);
        int piece = decode_piece(move);
        int promoted = decode_promoted(move);

        pop_bit(bitboards[piece], source);
        zobrist ^= piece_keys[piece][source];

        int pawn = P, knight = N, king = K, queen = Q, bishop = B, rook = R;
        U64* opponent_bitboards = bitboards + 6;
        if (side == black) {
            opponent_bitboards = bitboards;
            pawn = p, knight = n, king = k, queen = q, bishop = b, rook = r;
        }

        int victim = get_piece(target);

        if (victim > -1) {
            pop_bit(bitboards[victim], target);
            piece_count[victim]--;
            zobrist ^= piece_keys[victim][target];
        }

        set_bit(bitboards[piece], target);
        zobrist ^= piece_keys[piece][target];

        if (promoted) {
            pop_bit(bitboards[pawn], target);
            set_bit(bitboards[promoted], target);
            piece_count[pawn]--;
            piece_count[promoted]++;
            zobrist ^= piece_keys[pawn][target];
            zobrist ^= piece_keys[promoted][target];
        }

        if (enp) {
            if (side == white) {
                pop_bit(bitboards[p], target + 8);
                piece_count[p]--;
                zobrist ^= piece_keys[p][target + 8];
            } else {
                pop_bit(bitboards[P], target - 8);
                piece_count[P]--;
                zobrist ^= piece_keys[P][target - 8];
            }
        }

        if (enpassant != no_square) {
            zobrist ^= enpassant_keys[enpassant];
        }

        enpassant = no_square;

        if (piece == pawn && abs(source - target) == 16) {
            if (side == white) {
                enpassant = target + 8;
                zobrist ^= enpassant_keys[target + 8];
            } else {
                enpassant = target - 8;
                zobrist ^= enpassant_keys[target - 8];
            }
        }

        if (castling) {
            castled = 1;
            switch (target) {
                case g1:
                    pop_bit(bitboards[R], h1);
                    set_bit(bitboards[R], f1);
                    zobrist ^= piece_keys[R][h1];
                    zobrist ^= piece_keys[R][f1];
                    break;
                case c1:
                    pop_bit(bitboards[R], a1);
                    set_bit(bitboards[R], d1);
                    zobrist ^= piece_keys[R][a1];
                    zobrist ^= piece_keys[R][d1];
                    break;
                case g8:
                    pop_bit(bitboards[r], h8);
                    set_bit(bitboards[r], f8);
                    zobrist ^= piece_keys[r][h8];
                    zobrist ^= piece_keys[r][f8];
                    break;
                case c8:
                    pop_bit(bitboards[r], a8);
                    set_bit(bitboards[r], d8);
                    zobrist ^= piece_keys[r][a8];
                    zobrist ^= piece_keys[r][d8];
                    break;
            }
        }

        zobrist ^= castle_keys[castle];
        castle &= castling_rights[source];
        castle &= castling_rights[target];
        zobrist ^= castle_keys[castle];

        occupancies[white] = 0;
        occupancies[black] = 0;
        occupancies[both] = 0;

        for (int bb_piece = P; bb_piece <= K; bb_piece++)
            occupancies[white] |= bitboards[bb_piece];

        for (int bb_piece = p; bb_piece <= k; bb_piece++)
            occupancies[black] |= bitboards[bb_piece];

        occupancies[both] |= occupancies[white];
        occupancies[both] |= occupancies[black];

        side ^= 1;
        moves++;
        zobrist ^= side_key;

        return 1;
    } else {
        if (is_capture(move))
            return make_move(move, all_moves);

        return 0;
    }
}

void Skunk::print_move_detailed(int move) {
    printf("%-9d %-6s %-8s %-7c %-9d %-9d %-9d\n",
           move,
           square_to_coordinate[decode_source(move)],
           square_to_coordinate[decode_destination(move)],
           ascii_pieces[decode_piece(move)],
           score_move(move),
           decode_enpassant(move),
           decode_castle(move));
}

void Skunk::show_sort() {
    t_moves moves_list;
    generate_moves(moves_list);
    printf("Moves before sort:\n");
    for (int i = 0; i < moves_list.count; i++) {
        print_move(moves_list.moves[i]);
        printf("\n");
    }
    sort_moves(moves_list.moves, moves_list.count);
    printf("After sort:\n");
    for (int i = 0; i < moves_list.count; i++) {
        print_move(moves_list.moves[i]);
        printf("\n");
    }
}

void Skunk::print_move(int move) {
    int promoted = decode_promoted(move);
    if (promoted) {
        printf("%s%s%c", square_to_coordinate[decode_source(move)],
               square_to_coordinate[decode_destination(move)],
               ascii_pieces[promoted]);
    } else {
        printf("%s%s", square_to_coordinate[decode_source(move)],
               square_to_coordinate[decode_destination(move)]);
    }
}