//
// Created by Jeremy Colegrove on 11/8/21.
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
            // match rank separator
            fen_idx++;
            square--;
        } else if (fen[fen_idx] >= '0' && fen[fen_idx] <= '9') {
            // match empty square numbers within FEN string
            square += fen[fen_idx] - '1'; // use '1' to offset the loop increment
            fen_idx++;
        } else {
            // match ascii pieces within FEN string
            int piece = char_pieces[fen[fen_idx]];
            set_bit(bitboards[piece], square);
            piece_count[piece]++;
            fen_idx++;
        }
        square++;
    }
    // parse side to move
    side = (fen[++fen_idx] == 'w') ? white : black;
    fen_idx += 2;

    // parse castling rights
    while (fen[fen_idx] != ' ' && fen_idx < fen.length()) {
        switch (fen[fen_idx++]) {
            case 'K': castle |= wk; break;
            case 'Q': castle |= wq; break;
            case 'k': castle |= bk; break;
            case 'q': castle |= bq; break;
        }
    }

    // parse enpassant square
    if (fen[++fen_idx] != '-') {
        int file = fen[fen_idx++] - 'a';
        int rank = 8 - (fen[fen_idx] - '0');
        enpassant = rank * 8 + file;
    } else {
        enpassant = no_square;
    }

    // update occupancies
    for (int piece = P; piece <= k; piece++) {
        occupancies[piece <= K ? white : black] |= bitboards[piece];
    }

    // init all occupancies
    occupancies[both] = occupancies[white] | occupancies[black];

    // init hash key
    zobrist = generate_zobrist();

    init();

}



void Skunk::fill_occupancies() {
    // loop over white pieces bitboards
    for (int piece = P; piece <= K; piece++)
        // populate white occupancy bitboard
        occupancies[white] |= bitboards[piece];

    // loop over black pieces bitboards
    for (int piece = p; piece <= k; piece++)
        // populate white occupancy bitboard
        occupancies[black] |= bitboards[piece];


    // init all occupancies
    occupancies[both] |= occupancies[white];
    occupancies[both] |= occupancies[black];
}

//Default constructor and destructor
Skunk::Skunk() {
    //reset all of our bitboards
    memset(bitboards, 0ULL, sizeof(bitboards));
    memset(occupancies, 0ULL, sizeof(occupancies));
    //init all of our saved tables for piece attacks
    construct_pawn_tables();
    construct_knight_masks();
    construct_king_tables();
    construct_bishop_tables();
    construct_rook_tables();
    construct_slider_attacks();
    construct_rays();
    construct_direction_rays();
    construct_file_masks();

    //reset our turn
    side = white;
    //reset our castle rights
    castle = 0;
}

void Skunk::init() {
    seed = 4091583267;//1804289383;

    fill_occupancies();

    repitition.count = 0;

    // intialize zobrist hashing random keys
    for (int piece=P; piece <= k; piece++) {
        for (int board = 0; board<64; board++) {
            piece_keys[piece][board] = get_random_U64_number();
        }
    }
    for (int board=0; board<64; board++) {
        enpassant_keys[board] = get_random_U64_number();
    }

    for (int i=0; i<16; i++) {
        castle_keys[i] = get_random_U64_number();
    }

    side_key = get_random_U64_number();

    zobrist = generate_zobrist();

    init_precomputed_masks();
    
    init_heuristics();

}



// generate 32-bit pseudo legal numbers
unsigned int Skunk::get_random_U32_number()
{
    // get current state
    unsigned int number = seed;

    // XOR shift algorithm
    number ^= number << 13;
    number ^= number >> 17;
    number ^= number << 5;

    seed = number;
    // return random number
    return number;
}

U64 Skunk::generate_zobrist() {

    U64 hash = 0ULL;
    U64 bitboard;

    for (int piece = P; piece <=k; piece ++) {
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

// generate 64-bit pseudo legal numbers
U64 Skunk::get_random_U64_number()
{
    // define 4 random numbers
    U64 n1, n2, n3, n4;

    // init random numbers slicing 16 bits from MS1B side
    n1 = (U64)(get_random_U32_number()) & 0xFFFF;
    n2 = (U64)(get_random_U32_number()) & 0xFFFF;
    n3 = (U64)(get_random_U32_number()) & 0xFFFF;
    n4 = (U64)(get_random_U32_number()) & 0xFFFF;

    // return random number
    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}
Skunk::~Skunk() {

}

void Skunk::construct_file_masks() {

    memset(file_masks, 0ULL, sizeof(file_masks));

    U64 mask;

    // middle
    for (int square = 0; square < 64; square++) {
        mask = 0ULL;
        for (int row = 0; row < 8; row++) {
            set_bit(mask, row * 8 + (square & 7));
        }
        file_masks[MIDDLE][square] = mask;
    }

    // left
    for (int square = 0; square < 64; square++) {
        mask = 0ULL;
        if (square & 7 == 0) continue;
        for (int row = 0; row < 8; row++) {
            set_bit(mask, row * 8 + ((square-1) & 7));
        }
        file_masks[LEFT][square] = mask;
    }

    // right
    for (int square = 0; square < 64; square++) {
        mask = 0ULL;
        if (square & 7 == 7) continue;
        for (int row = 0; row < 8; row++) {
            set_bit(mask, row * 8 + ((square+1) & 7));
        }
        file_masks[RIGHT][square] = mask;
    }
}



void Skunk::construct_direction_rays() {

    for (int i=0; i<64; i++) {
        // right
        nearest_square[DE][i] = i + (7 - (i & 7));

        // up
        nearest_square[DN][i] = i & 7;

        // left
        nearest_square[DW][i] = i - (i & 7);

        // down
        nearest_square[DS][i] = 56 + (i & 7);


        // upper right
        nearest_square[DNE][i] = i - std::min(i >> 3, 7 - (i & 7)) * 7;

        // lower right
        nearest_square[DSE][i] = i + std::min(7 - (i >> 3), 7 - (i & 7)) * 9;

        // lower left
        nearest_square[DSW][i] = i + std::min(7 - (i >> 3), (i & 7)) * 7;

        // upper left
        nearest_square[DNW][i] = i - std::min(i >> 3, (i & 7)) * 9;
    }



}
// generate rays from source square to destination square for every square
void Skunk::construct_rays() {
    memset(rays, 0ULL, sizeof(rays));
    int count = 0;
    U64 ray = 0ULL;
    for (int i=0; i<64; i++) {
        // horizontal
        int left_square = (i >> 3)*8;
        for (int j=left_square; j<i; j++) {
            ray = 0ULL;
            int square = j;
            while (square < i-1) {
                square++;
                set_bit(ray, square);
            }
            rays[j][i] = ray;
            rays[i][j] = ray;
        }

        // vertical
        int top_square = i & 7;
        for (int j=top_square; j<i; j+=8) {
            ray = 0ULL;
            int square = j;
            while (square < i-8) {
                square+=8;
                set_bit(ray, square);
            }
            rays[i][j] = ray;
            rays[j][i] = ray;
        }

        // diagonal se->nw
        int l_diag = std::min(i & 7, i >> 3);
        for (int j = 1; j <= l_diag; j++ ) {
            ray = 0ULL;
            int square = i - 9 * (j-1);
            while (square < i) {
                set_bit(ray, square);
                square += 9;
            }
            rays[i][i - 9 * j] = ray;
            rays[i - 9 * j][i] = ray;
        }

        // diagonal sw->ne
        int r_diag = std::min(7 - (i & 7), i >> 3);
        for (int j = 1; j <= r_diag; j++ ) {
            ray = 0ULL;
            int square = i - 7 * (j-1);
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
    //Generate the pawn tables
    for (int square=0; square<64; square++)
    {

        U64 white_board = 0UL;
        U64 black_board = 0UL;
        U64 white_attacks = 0UL;
        U64 black_attacks = 0UL;
        set_bit(white_board, square);
        set_bit(black_board, square);

        //WHITE pawn attacks//
        if (white_board >> 7 & not_a_file) white_attacks |= white_board >> 7;
        if (white_board >> 9 & not_h_file) white_attacks |= white_board >> 9;

        //BLACK pawn attacks
        if (black_board << 7 & not_h_file) black_attacks |= black_board << 7;
        if (black_board << 9 & not_a_file) black_attacks |= black_board << 9;

        pawn_masks[white][square] = white_attacks;
        pawn_masks[black][square] = black_attacks;
    }
}

void Skunk::construct_knight_masks() {
    //Generate the knight tables
    for (int square=0; square<64; square++) {
        U64 white_board = 0UL;
        U64 attacks = 0UL;
        set_bit(white_board, square);

        //Knight attacks  attacks//

        if (white_board >> 15 & not_a_file) attacks |= white_board >> 15;
        if (white_board >> 17 & not_h_file) attacks |= white_board >> 17;

        if (white_board >> 6 & not_ab_file)  attacks |= white_board >> 6;
        if (white_board >> 10 & not_gh_file) attacks |= white_board >> 10;


        if (white_board << 15 & not_h_file) attacks |= white_board << 15;
        if (white_board << 17 & not_a_file) attacks |= white_board << 17;

        if (white_board << 6 & not_gh_file)  attacks |= white_board << 6;
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

    // do it just for white
    if (color == white) {
        attacks |= rays[square-1][nearest_square[DN][square-1]] & not_h_file;
        attacks |= rays[square+1][nearest_square[DN][square+1]] & not_a_file;
    } else {
        attacks |= rays[square-1][nearest_square[DS][square-1]] & not_h_file;
        attacks |= rays[square+1][nearest_square[DS][square+1]] & not_a_file;
    }
    
    return attacks;
}

void Skunk::construct_king_tables() {
    //Generate the kings tables
    for (int square=0; square<64; square++) {
        U64 white_board = 0UL;
        U64 attacks = 0UL;
        set_bit(white_board, square);

        //King attacks  attacks//

        //Left
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
    //Generate the bishop tables
    for (int square=0; square<64; square++) {
        U64 attacks = 0UL;

        int tr = square >> 3;
        int tf = square & 7;
        for (int r=tr+1, f=tf+1; r<=6 && f<=6; r++, f++) set_bit(attacks, r*8+f);
        for (int r=tr-1, f=tf+1; r>=1 && f<=6; r--, f++) set_bit(attacks, r*8+f);
        for (int r=tr+1, f=tf-1; r<=6 && f>=1; r++, f--) set_bit(attacks, r*8+f);
        for (int r=tr-1, f=tf-1; r>=1 && f>=1; r--, f--) set_bit(attacks, r*8+f);

        bishop_masks[square] = attacks;
    }
}

void Skunk::construct_rook_tables() {
    //Generate the kings tables
    for (int square=0; square<64; square++) {
        U64 attacks = 0UL;

        int tr = square & 7;
        int tf = square >> 3;
        for (int f=tf-1; f>=1; f--) set_bit(attacks, f*8+tr);
        for (int f=tf+1; f<=6; f++) set_bit(attacks, f*8+tr);
        for (int r=tr+1; r<=6; r++) set_bit(attacks, tf*8+r);
        for (int r=tr-1; r>=1; r--) set_bit(attacks, tf*8+r);

        rook_masks[square] = attacks;
    }
}

U64 Skunk::construct_bishop_attacks(int square, unsigned long long int blockers) {
    U64 moves = 0UL;

    int tr = square >> 3;
    int tf = square & 7;
    for (int r=tr+1, f=tf+1; r<=7 && f<=7; r++, f++) {
        set_bit(moves, r*8+f);
        if (get_bit(blockers, r*8+f)) break;
    }

    for (int r=tr-1, f=tf+1; r>=0 && f<=7; r--, f++) {
        set_bit(moves, r*8+f);
        if (get_bit(blockers, r*8+f)) break;
    }
    for (int r=tr+1, f=tf-1; r<=7 && f>=0; r++, f--) {
        set_bit(moves, r*8+f);
        if (get_bit(blockers, r*8+f)) break;
    }
    for (int r=tr-1, f=tf-1; r>=0 && f>=0; r--, f--) {
        set_bit(moves, r*8+f);
        if (get_bit(blockers, r*8+f)) break;
    }

    return moves;
}

U64 Skunk::construct_rook_attacks(int square, U64 blockers) {
    //Generate the kings tables
    U64 attacks = 0UL;

    int tr = square & 7;
    int tf = square >> 3;
    for (int f=tf-1; f>=0; f--) {
        set_bit(attacks, f*8+tr);
        if (get_bit(blockers, f*8+tr)) break;
    }
    for (int f=tf+1; f<=7; f++) {
        set_bit(attacks, f*8+tr);
        if (get_bit(blockers, f*8+tr)) break;
    }
    for (int r=tr+1; r<=7; r++) {
        set_bit(attacks, tf*8+r);
        if (get_bit(blockers, tf*8+r)) break;
    }
    for (int r=tr-1; r>=0; r--) {
        set_bit(attacks, tf*8+r);
        if (get_bit(blockers, tf*8+r)) break;
    }

    return attacks;
}

void Skunk::construct_slider_attacks() {
    for (int square=0; square<64; square++)
    {
        // get our rook attack mask
        U64 rook_attack = rook_masks[square];
        // get our bishop attack mask
        U64 bishop_attack = bishop_masks[square];

        // get the number of bits in the rook mask
        int rook_relevant_bit_count = bit_count(rook_attack);

        //get the number of bits in the bishop mask
        int bishop_relevant_bit_count = bit_count(bishop_attack);

        // get the number of rook indicies
        int rook_occupancy_indicies = (1 << rook_relevant_bit_count);

        // get the number of bishop indicies
        int bishop_occupancy_indicies = (1 << bishop_relevant_bit_count);

        //Construct all attacks for this square for the rook
        for (int index=0; index<rook_occupancy_indicies; index++)
        {
            //The rooks occupancy mask
            U64 rook_occupancy = set_occupancy(index, rook_relevant_bit_count, rook_attack);
            //The rooks magic index
            int magic_index = (rook_occupancy * rook_magic_numbers[square]) >> (64 - rook_relevant_bits[square]);
            //Insert the attack into our table
            rook_attacks[square][magic_index] = construct_rook_attacks(square, rook_occupancy);
        }

        //Construct all attacks for this square for the rook
        for (int index=0; index<bishop_occupancy_indicies; index++)
        {
            //The rooks occupancy mask
            U64 bishop_occupancy = set_occupancy(index, bishop_relevant_bit_count, bishop_attack);
            //The rooks magic index
            int magic_index = (bishop_occupancy * bishop_magic_numbers[square]) >> (64 - bishop_relevant_bits[square]);
            //Insert the attack into our table
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

// is square current given attacked by the current given side
bool Skunk::is_square_attacked(int square, int side)
{
#ifdef DEBUG
    if (side != 0 && side != 1) {
        printf("Weird, side is %d\n", side);
    }

    assert(side == 0 || side == 1);
    assert(square > -1 && square < 64);
#endif

    // attacked by white pawns
    if ((side == white) && (pawn_masks[black][square] & bitboards[P])) return true;

    // attacked by black pawns
    if ((side == black) && (pawn_masks[white][square] & bitboards[p])) return true;

    // attacked by knights
    if (knight_masks[square] & ((side == white) ? bitboards[N] : bitboards[n])) return true;

    // attacked by bishops
    if (get_bishop_attacks(square, occupancies[both]) & ((side == white) ? bitboards[B] : bitboards[b])) return true;

    // attacked by rooks
    if (get_rook_attacks(square, occupancies[both]) & ((side == white) ? bitboards[R] : bitboards[r])) return true;

    // attacked by bishops
    if (get_queen_attacks(square, occupancies[both]) & ((side == white) ? bitboards[Q] : bitboards[q])) return true;

    // attacked by kings
    if (king_masks[square] & ((side == white) ? bitboards[K] : bitboards[k])) return true;

    // by default return false
    return false;
}

/*****************************\
===============================
           printing
===============================
\*****************************/


//Prints out the given board
void Skunk::print_bitboard(U64 board) {
    for (int i=0; i<64; i++)
    {
        if ((i & 7) == 0) {
            std::cout << std::endl;
            std::cout << 8-(i/8) << "\t\t";
        }
        if (get_bit(board, i)) std::cout << "1  ";
        else std::cout << "0  ";
    }
    std::cout << "\n\n\t\ta  b  c  d  e  f  g  h" << std::endl;
    std::cout << "\t\tBoard number: " << board << std::endl;
}

//Prints out the attacks for a side
void Skunk::print_attacks(int side) {
    for (int i=0; i<64; i++)
    {
        if ((i & 7) == 0) {
            std::cout << std::endl;
            std::cout << 8-(i/8) << "\t\t";
        }
        if (is_square_attacked(i, side)) std::cout << "1  ";
        else std::cout << "0  ";
    }
    std::cout << "\n\n\t\ta  b  c  d  e  f  g  h" << std::endl;
}

void Skunk::print_board() {
    std::cout << std::endl;
    //loop over rank and files
    for (int rank = 0; rank<8; rank++)
    {
        for (int file=0; file<8; file++)
        {
            int square = rank*8 + file;
            if (!file)
            {
                std::cout << 8-rank << "   ";
            }
            //initialize piece
            int piece = -1;
            int num_found = 0;
            for (int count = P; count <= k; count++)
            {
                if (get_bit(bitboards[count], square))
                {
                    num_found += 1;
                    piece = count;
                }
            }

            // print different piece set depending on OS
            #ifdef _WIN32
                        printf(" %c", (piece == -1) ? '.' : ascii_pieces[piece]);
            #else
                        printf(" %s ", (piece == -1) ? "." : unicode_pieces[piece]);
            #endif
        }
        // print a newline
        std::cout << std::endl;
    }
    // print files
    std::cout << "\n     " << "a  b  c  d  e  f  g  h" << std::endl;
    // print whos turn it is
    std::cout << "\nSide: " << (side == white ? "white" : "black") << std::endl;
    // print en passant square
    printf("Enpassant: %s\n", enpassant==no_square?"None":square_to_coordinate[enpassant]);
    //print castling rights
    std::cout << "\nCastling: " << ((castle&wk) ? 'K':'-') << ((castle&wq)?'Q':'-') << ((castle&bk)?'k':'-') << ((castle&bq)?'q':'-') << std::endl;

    printf("Positional score: %f\n", evaluate());

    printf("Ply: %d\n", ply);
}

/*****************************\
===============================
        bit helpers
===============================
\*****************************/


int Skunk::bit_count(U64 board) {
    int count = 0;
    while (board)
    {
        count ++;
        board &= board - 1;
    }
    return count;
}


U64 Skunk::set_occupancy(int index, int bits_in_mask, U64 attack_mask) {
    U64 occupancy = 0ULL;

    for (int i=0; i<bits_in_mask; i++)
    {
        int square = __builtin_ctzll(attack_mask);
        pop_bit(attack_mask, square);
        if (index & (1ULL << i))
        {
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
    int knight=N, bishop=B, rook=R, queen=Q;
    if (side == white) {
        knight=n, bishop=b, rook=r, queen=q;
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

    // calculate queen attacks
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
    int pawn = P, knight=N, king=K;
    if (side == white) {
        pawn = p, knight=n, king=k;
    }

    // calculate white pawn attacks
    pieces = bitboards[pawn];
    while (pieces) {
        jumpers |= pawn_masks[side^1][__builtin_ctzll(pieces)];
        pop_lsb(pieces);
    }

    // calculate knight attacks
    pieces = bitboards[knight];
    while (pieces) {
        jumpers |= knight_masks[__builtin_ctzll(pieces)];
        pop_lsb(pieces);
    }

    //calculate king attacks
    pieces = bitboards[king];
    while (pieces) {
        jumpers |= king_masks[__builtin_ctzll(pieces)];
        pop_lsb(pieces);
    }
    return jumpers;
}

inline bool Skunk::is_promotion_square(int square, int side) {
    if (side == black) {
        return (square >= 56 && square <= 63);
    } else {
        return (square >= 0 && square <= 7);
    }
}

// generate all moves in list
void Skunk::generate_moves(Moves &moves)
{
    moves.count = 0;

    int square;

    // get the right pieces (for easier lookup later)
    int pawn, knight, bishop, rook, queen, king;
    U64 *opponent_bitboards;
    if (side == black) {
        pawn = p, knight=n, bishop=b, rook=r, queen=q, king=k;
        opponent_bitboards=bitboards;
    } else {
        pawn = P, knight=N, bishop=B, rook=R, queen=Q, king=K;
        opponent_bitboards=bitboards+6;
    }

    //KING MOVES
    U64 empty_squares = ~occupancies[side];
    int king_square = __builtin_ctzll(bitboards[king]);
    U64 king_moves = king_masks[king_square] & empty_squares;

    // get all of the destinations for the king
    while (king_moves) {
        square = __builtin_ctzll(king_moves);
        // encode the move
        moves.list[moves.count++] = encode_move(king_square, square, king, 0, 0, 0);
        pop_lsb(king_moves);
    }


    // PAWN MOVES
    U64 pawns = bitboards[pawn];
    while (pawns) {
        square = __builtin_ctzll(pawns);

        U64 pawn_moves = get_attacks(pawn, square, side);

        while (pawn_moves) {
            int destination = __builtin_ctzll(pawn_moves);
            if (is_promotion_square(destination, side)) {
                moves.list[moves.count++] = encode_move(square, destination, pawn, rook, 0, 0);
                moves.list[moves.count++] = encode_move(square, destination, pawn, bishop, 0, 0);
                moves.list[moves.count++] = encode_move(square, destination, pawn, queen, 0, 0);
                moves.list[moves.count++] = encode_move(square, destination, pawn, knight, 0, 0);
            } else if (destination == enpassant) {
                moves.list[moves.count++] = encode_move(square, destination, pawn, 0, 1, 0);
            } else {
                moves.list[moves.count++] = encode_move(square, destination, pawn, 0, 0, 0);
            }
            pop_lsb(pawn_moves);
        }
        pop_lsb(pawns);
    }

    // Rook, queen, bishop, and knight moves
    int regular_pieces[] = {rook, queen, bishop, knight};
    int piece;
    U64 attacks;
    for (int i=0; i<4; i++) {
        piece = regular_pieces[i];
        U64 pieces = bitboards[piece];
        while (pieces) {
            square = __builtin_ctzll(pieces);
            attacks = get_attacks(piece, square, side);
            while (attacks) {
                int destination = __builtin_ctzll(attacks);
                moves.list[moves.count++] = encode_move(square, destination, piece, 0, 0, 0);
                pop_lsb(attacks);
            }
            pop_lsb(pieces);
        }
    }

    // generate the attacked squares as normal after the king is done
    U64 attacked_squares = get_slider_attacks() | get_jumper_attacks();

    // CASTLING MOVES
    if (side == white && (attacked_squares & bitboards[K]) == 0) {
        // if castling is available and king is not in check
        if (castle & wk && ((attacked_squares | occupancies[both]) & castle_mask_wk) == 0) {
            moves.list[moves.count++] = encode_move(e1, g1, king, 0, 0, 1);
        }
        if (castle & wq && (attacked_squares & castle_attack_mask_wq) ==0 && (occupancies[both] & castle_piece_mask_wq) == 0 ) {
            moves.list[moves.count++] = encode_move(e1, c1, king, 0, 0, 1);
        }
    } else if (side == black && (attacked_squares & bitboards[k]) == 0) {
        if (castle & bk && ((attacked_squares | occupancies[both]) & castle_mask_bk) == 0) {
            moves.list[moves.count++] = encode_move(e8, g8, king, 0, 0, 1);
        }
        if (castle & bq && (attacked_squares & castle_attack_mask_bq) ==0 && (occupancies[both] & castle_piece_mask_bq) == 0 ) {
            moves.list[moves.count++] = encode_move(e8, c8, king, 0, 0, 1);
        }
    }
}

int Skunk::get_piece(int square) {
    int type = -1;
    U64 mask = (1ULL << square);
    for (int piece=P; piece<=k; piece++) {
        if (bitboards[piece] & mask) {
            type = piece;
            break;
        }
    }
    return type;
}

U64 Skunk::get_attacks(int piece, int square, int side) {
    // go through all of the bitboards and check which piece is on the piece, assuming it is not an invalid piece
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
//             single pushes
            attacks |= ((piece_bitboard << 8) & ~occupancies[both]) | (pawn_masks[side][square] & occupancies[side ^ 1]);
//             double pushes
            attacks |= ((((piece_bitboard & row7) << 8) & ~occupancies[both]) << 8) & ~occupancies[both];
            // handle enpassants here as well
            if (enpassant != no_square) {
                attacks |= pawn_masks[side][square] & (1ULL << enpassant);
            }
            return attacks;
        case P:
            // single pushes
            attacks |= ((piece_bitboard >> 8) & ~occupancies[both]) | (pawn_masks[side][square] & occupancies[side ^ 1]);
            // double pushes
            attacks |= ((((piece_bitboard & row2) >> 8) & ~occupancies[both]) >> 8) & ~occupancies[both];

            // handle enpassants here as well
            if (enpassant != no_square) {
                // check if enpassant square and capture mask collide
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

void Skunk::print_moves(Moves &moves)
{
    printf("%-9s %-6s %-8s %-7s %-9s %-9s %-9s\n","num", "source", "target", "piece", "score", "enpassant", "castle");

    for (int i=0; i<moves.count; i++) {
        int move = moves.list[i];

        printf("%-9d %-6s %-8s %-7c %-9d %-9d %-9d\n",
               move,
               square_to_coordinate[decode_source(move)],
               square_to_coordinate[decode_destination(move)],
               ascii_pieces[decode_piece(move)],
               score_move(move),
               decode_enpassant(move),
               decode_castle(move)
               );
    }
    printf("Total of %d moves.\n", moves.count);
}

void Skunk::init_heuristics() {
    // Initialize killer moves and history table to zero
    for (int i = 0; i < MAX_PLY; ++i) {
        killer_moves[i][0] = -1;
        killer_moves[i][1] = -1;
    }
    for (int piece = P; piece <= k; ++piece) {
        for (int square = 0; square < 64; ++square) {
            history_table[piece][square] = 0;
        }
    }
}

// Update the killer moves and history table
void Skunk::update_heuristics(int ply, int move, int depth) {
    // Update killer moves
    if (move != killer_moves[ply][0]) {
        killer_moves[ply][1] = killer_moves[ply][0];
        killer_moves[ply][0] = move;
    }

    // Update history table
    int piece = decode_piece(move);
    int destination = decode_destination(move);
    history_table[piece][destination] += depth * depth;
}

void Skunk::sort_moves(Moves *moves) {

    std::sort(moves->list, moves->list + moves->count, [&](const int &a, const int &b) {
        int a_score = score_move(a);
        int b_score = score_move(b);

        return a_score > b_score;
    });
}

int Skunk::see(int move) {
    int from = decode_source(move);
    int to = decode_destination(move);
    int attacked_piece = get_piece(from);
    int attacked_value = piece_scores[attacked_piece];
    int victim = get_piece(to);
    int victim_value = piece_scores[victim];

    // If the move is not a capture or the captured piece is more valuable, no need to perform SEE
    if (victim_value == 0 || attacked_value <= victim_value) {
        return victim_value - attacked_value;
    }

    // Temporary board to perform SEE calculation
    copy_board();

    // Perform the capture
    if (make_move(move)==false) {
        restore_board();
        return 0;
    };

    // Recursively calculate the SEE value
    int see_value = victim_value - see(get_smallest_attacker(to));

    // Restore the original board
    restore_board();

    return see_value;
}

int Skunk::get_smallest_attacker(int to) {
    // This function returns the move of the smallest attacker to the square 'to'
    // Generate all moves for the opponent's pieces
    Moves moves;
    generate_moves(moves);

    int smallest_attacker_value = INT_MAX;
    int smallest_attacker_move = 0;

    for (int i=0; i<moves.count; i++) {
        int move = moves.list[i];

        // Check if the move is a capture and the destination is the target square
        if (is_capture(move) && decode_destination(move) == to) {
            int attacker_piece = get_piece(decode_source(move));
            int attacker_value = piece_scores[attacker_piece];

            // Update the smallest attacker if the current attacker has a lower value
            if (attacker_value < smallest_attacker_value) {
                smallest_attacker_value = attacker_value;
                smallest_attacker_move = move;
            }
        }
    }

    // If there is no attacker, return 0
    if (smallest_attacker_value == INT_MAX) {
        return 0;
    }

    return smallest_attacker_move;
}


int Skunk::score_move(int move) {

    int score = 0;

    // look for move in previous pv line
    for (int i = 0; i < previous_pv_line.cmove; ++i) {
        if (move == previous_pv_line.argmove[i]) {
            score += 20000 - (i * 100);
        }
    }
   
    // consult the lookup table
    int piece = decode_piece(move);
    int destination = decode_destination(move);
    int victim = get_piece(destination);
    
    if (decode_promoted(move)) {
        // we want to check promotions high as well
        score += 5000;
    }

    // if it is a capture, use a piece victim lookup table
    // if (is_capture(move)) {
    //     score += mvv_lva[piece][victim] * 100;
    // } 

    // if it is a capture, use a piece victim lookup table
    if (is_capture(move)) {
        score += mvv_lva[piece][victim] * 100;
        // Adjust capture score based on SEE value
        // int see_value = see(move);
        // score += see_value * 10;
    }

#ifdef KILLER_HISTORY
    // Killer moves
    if (move == killer_moves[ply][0]) {
        score += 9000;
    } else if (move == killer_moves[ply][1]) {
        score += 8000;
    }

    // History heuristic
    score += history_table[piece][decode_destination(move)];
#endif

    return score;
}

bool Skunk::is_check() {
    return is_square_attacked(__builtin_ctzll(bitboards[side==white?K:k]), side^1);
}


int Skunk::quiesence(int alpha, int beta) {
    q_nodes++;
    if (force_stop) return 0;

    // Communicate with the user interface periodically
    if ((nodes % time_check_node_interval) == 0) {
        communicate();
    }

    // Generate all pseudo-legal moves
    Moves moves;
    generate_moves(moves);

    sort_moves(&moves);

    // Check if the king is in check
    int check = is_check();

    // Calculate the evaluation score
    int evaluation = evaluate();// + static_cast<int>(log(moves_list.count + 1) * 0.5 * MOBILITY_WEIGHT) - (check ? KING_SAFETY_WEIGHT : 0);
    

    // Alpha-beta pruning
    if (evaluation >= beta) {
        return evaluation;
    }

    if (evaluation > alpha) {
        alpha = evaluation;
    }

    // Sort the moves to improve search efficiency
    // sort_moves(moves_list.moves, moves_list.count);

    int score = INT_MIN;

    bool made_capture = false;

    for (int i=0; i<moves.count; i++) {
        int move = moves.list[i];

        // Only consider capture moves in quiescence search
        if (!is_capture(move)) continue;

        int victim = get_piece(decode_destination(move));

        #ifdef FUTILITY_PRUNE
        // // Futility pruning: skip moves that are unlikely to improve the position
        // if (!is_check() && (abs(piece_scores[victim]) + evaluation + piece_scores[Q] * 10) < alpha) {
        //     return alpha;
        // }
        // Futility pruning: skip moves that are unlikely to improve the position
        int futility_margin = 100; // Adjust this value based on your engine's requirements
        if (!is_check() && (evaluation + piece_scores[victim] + futility_margin <= alpha)) {
            continue;
        }
        #endif

        

        // Keep track of whether a capture move was made
        made_capture = true;

        // Copy the board
        copy_board();

        // Make the move on the board, but skip illegal moves and non-captures
        if (make_move(move) == false) {
            restore_board();
            continue;
        }

        // Recursively call quiescence search with negamax
        int test = -quiesence(-beta, -alpha);

        // Update the best score
        score = std::max(score, test);

        // Restore the board to its previous state
        restore_board();

        // Alpha-beta pruning
        if (score >= beta) {
            return beta;
        }

        if (score > alpha) {
            alpha = score;
        }
    }

    // If no legal capture moves and the king is in check, this is a checkmate
    if (!made_capture && check) {
        return -CHECKMATE + ply;
    }

    return alpha;
}

void Skunk::store_transposition_table(U64 zobristKey, int16_t value, int16_t depth, int move, NodeType type) {
    // std::cout << "storing transposition" << std::endl;
    stored_transpositions++;
    if (type == LOWER_BOUND) {
        stored_lower ++;
    } else if (type == UPPER_BOUND) {
        stored_upper ++;
    } else {
        stored_exact ++;
    }

    TTEntry *entry = &transpositionTable[zobristKey & (HASH_SIZE - 1)];
    entry->zobristKey = zobrist;
    entry->value = value;
    entry->depth = depth;
    entry->move = move;
    entry->type = type;
}

TTEntry *Skunk::probe_transposition_table(U64 zobristKey) {
    TTEntry *entry = &transpositionTable[zobrist & (HASH_SIZE - 1)];
    if (entry->zobristKey == zobrist) {
        returned_transpositions++;
        return entry;
    }
    return nullptr;
}

bool Skunk::should_do_null_move() {
    /* should NOT do null move if 
    the side to move has only its king and pawns remaining
    the side to move has a small number of pieces remaining
    the previous move in the search was also a null move.
    */

    int num_major;
    // count the number of major pieces
    if (side == white) {
        num_major = piece_count[B] + piece_count[R] + piece_count[Q] + piece_count[N];
    } else {
        num_major = piece_count[b] + piece_count[r] + piece_count[q] + piece_count[n];
    }


    // side to move has too few of pieces
    // if (num_major <= 3) return false;

    return true;
}

// new negamax
int Skunk::negamax(int alpha, int beta, int depth, int verify, int do_null, t_line *pline) {
    int best_move = 0, current_move, best_score = -INT_MAX, current_score, null_move_score;
    bool fail_high = false, check = false;
    t_line line = {.cmove = 0};

    nodes++;

    if ((nodes % time_check_node_interval) == 0) {
        communicate();
    }

    if (force_stop) return 0;

    if (depth < 1) {
        if (pline != nullptr) pline->cmove = 0;
        return quiesence(alpha, beta);
    }

    if (ply && is_repetition()) {
        return -evaluate() * 0.25;
    }

    // Transposition table lookup
    TTEntry *entry = probe_transposition_table(zobrist);
    if (entry != nullptr && !verify) {
        if (entry->depth >= depth) {
            if (entry->type == EXACT) {
                cache_hit++;
                if (ply == 0 && pline != nullptr) {
                    pline->argmove[0] = entry->move;
                    memcpy(pline->argmove + 1, line.argmove, line.cmove * sizeof(int));
                    pline->cmove = line.cmove + 1;
                }
                return entry->value;
            } else if (entry->type == LOWER_BOUND) {
                alpha = std::max(alpha, (entry->value));
            } else if (entry->type == UPPER_BOUND) {
                beta = std::min(beta, (entry->value));
            }
            if (alpha >= beta) {
                cache_hit++;
                return entry->value;
            }
        }
    }

    Moves moves;
    generate_moves(moves);
    sort_moves(&moves);

    check = is_check();

    if (check) depth++;

    // Null move pruning
    if (!check && do_null == DO_NULL && (!verify || depth > 1)) {
        copy_board();
        side ^= 1;
        zobrist ^= side_key;
        if (enpassant != no_square) {
            zobrist ^= enpassant_keys[enpassant];
        }
        enpassant = no_square;

        ply++;

        null_move_score = -negamax(-beta, -beta + 1, depth - 1 - NULL_R, verify, NO_NULL, nullptr);

        ply--;
        restore_board();

        if (null_move_score >= beta) {
            if (verify) {
                depth--;
                verify = false;
                fail_high = true;
            } else {
                return null_move_score;
            }
        }
    }

    copy_board();
    int searched_moves = 0;
    int legal_moves = 0;

    for (int i = 0; i < moves.count; i++) {
        current_move = moves.list[i];


        if (make_move(current_move) == false) {
            restore_board();
            continue;
        };
        legal_moves ++;
        ply++;
        repitition.table[repitition.count++] = zobrist;

        re_search:
        // Apply PVS and LMR
        if (searched_moves == 0) {
            current_score = -negamax(-beta, -alpha, depth - 1, verify, DO_NULL, &line);
        } else {
            if (searched_moves >= LMR_DEPTH && depth >= LMR_MIN_DEPTH && !check && !is_capture(current_move) && decode_promoted(current_move) == 0) {
                // Apply LMR
                current_score = -negamax(-alpha - 1, -alpha, depth - 1 - LMR_REDUCTION, verify, NO_NULL, &line);

                if (current_score > alpha && current_score < beta) {
                    // Re-search with full depth, as the move is better than expected
                    current_score = -negamax(-beta, -alpha, depth - 1, verify, DO_NULL, &line);
                }
            } else {
                // Apply PVS
                current_score = -negamax(-alpha - 1, -alpha, depth - 1, verify, NO_NULL, &line);

                if (current_score > alpha && current_score < beta) {
                    // Re-search with full window, as the move is better than expected
                    current_score = -negamax(-beta, -alpha, depth - 1, verify, DO_NULL, &line);
                }
            }
        }

        // Check for best score and move
        if (current_score > best_score) {
            best_score = current_score;
            best_move = current_move;
        }

        // Null move verification re-check if no beta cut-off was found
        if (fail_high && current_score < beta) {
            depth++;
            fail_high = false;
            verify = true;
            goto re_search;
        }

        restore_board();
        ply--;
        repitition.count--;
        searched_moves++;

        if (best_score > alpha) {
            alpha = best_score;
            if (pline != nullptr) {
                pline->argmove[0] = current_move;
                memcpy(pline->argmove + 1, line.argmove, line.cmove * sizeof(int));
                pline->cmove = line.cmove + 1;
            }

            if (alpha >= beta) {
                // Update killer moves and history here...
                #ifdef KILLER_HISTORY
                if (!is_capture(current_move) && ply < MAX_PLY) {
                    update_heuristics(ply, current_move, depth);
                }
                #endif

                #ifdef TRANSPOSITION_TABLE
                store_transposition_table(zobrist, beta, depth, best_move, LOWER_BOUND);
                #endif
                return beta;
            }
        }
    }


    if (legal_moves == 0) {
        return check ? (-CHECKMATE) + ply : 0;
    }

    // Transposition table store
    #ifdef TRANSPOSITION_TABLE
    NodeType type;
    if (best_score <= alpha) {
        type = UPPER_BOUND;
    } else if (best_score >= beta) {
        type = LOWER_BOUND;
    } else {
        type = EXACT;
    }
    store_transposition_table(zobrist, best_score, depth, best_move, type);
    #endif

    return best_score;
}






// the top level call to get the best move
int Skunk::search(int maxDepth) {


    // iterate through deepening as we go
    t_line pline = {.cmove = 0};

    start_time = std::chrono::steady_clock::now();

    force_stop = 0;

    int score, result;
    cache_hit = 0;
    q_nodes = 0;
    nodes = 0;

    null_move_pruned = 0;

    int alpha = -INT_MAX, beta = INT_MAX;

    returned_transpositions = 0;
    stored_transpositions = 0;
    exact_hits = 0;
    lower_hits = 0;
    upper_hits = 0;
    stored_upper = 0;
    stored_lower = 0;
    stored_exact = 0;

    for (int depth = 0; depth < maxDepth; depth++) {

        init_heuristics();

        ply = 0;
        pline.cmove = 0;

        score = negamax(alpha, beta, depth + 1, 1, DO_NULL, &pline);

        if (force_stop) break;

        // print for each depth
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_time).count();
        
        if (score < -CHECKMATE + 2000) {
            printf("info transpositions %d ttp: %.4f score mate %d depth %d nodes %d q_nodes %d time %ld pv ", cache_hit, ((float)cache_hit)/nodes, -(score + CHECKMATE) / 2 - 1, depth + 1, nodes, q_nodes, elapsed);
        } else if (score > CHECKMATE - 2000) {
            printf("info transpositions %d ttp: %.4f score mate %d depth %d nodes %d q_nodes %d time %ld pv ", cache_hit,((float)cache_hit)/nodes, (CHECKMATE - score) / 2 + 1, depth + 1, nodes, q_nodes, elapsed);
        } else {
            std::cout << "info transpositions " << cache_hit << " stored_exact " << stored_exact << " stored_lower " << stored_lower << " stored_upper " << stored_upper << " t_stored " << stored_transpositions << " t_returned " << returned_transpositions << " exact_hits " << exact_hits << " lower_hits " << lower_hits << " upper_hits " << upper_hits << " alpha_hits " << alpha_cutoffs << " pruned: " << null_move_pruned << " score cp " << score << " depth " << depth + 1 << " nodes " << nodes << " time " << elapsed << " pv ";
        } 
        // copy this pline to the previous pline struct so we can use it in next search
        memcpy(&previous_pv_line, &pline, sizeof(t_line));
        // previous_pv_line = pline;
        // print pv lines

        for (int i=0; i<previous_pv_line.cmove; i++) {
            print_move(previous_pv_line.argmove[i]);
            printf(" ");
        }
        std::cout << std::endl;

    }


    printf("bestmove ");
    print_move(previous_pv_line.argmove[0]);
    repitition.table[repitition.count++] = previous_pv_line.argmove[0]; // add this move into repetition table
    printf("\n");

    return pline.argmove[0];
}


float Skunk::evaluation_weights[NUM_WEIGHTS] = {
    15.0f,  // MATERIAL_WEIGHT
    1.0f,  // PIECE_SCORE_WEIGHT
    4.0f,  // DOUBLED_PAWNS_WEIGHT
    4.0f,  // ISOLATED_PAWNS_WEIGHT
    10.0f, // PASSED_PAWN_WEIGHT
    6.0f,  // MOBILITY_WEIGHT
    5.0f   // KING_SAFETY_WEIGHT
};


int Skunk::evaluate() {
    // material_score is how many total pieces are on the board
    int material_score = 0;
    int white_material_score = 0;
    int black_material_score = 0;
    int square_occupancy_score = 0;
    int square_occupancy_score_endgame = 0;
    int pawn_structure_score = 0;
    int passed_pawn_score = 0;
    int mobility_score = 0;
    int king_safety_score = 0;
    float game_phase;

    // get total points on the board to taper endgame
    for (int piece = P; piece <= K; piece++) {
        white_material_score += bit_count(bitboards[piece]) * piece_scores[piece];
    }

    // get total points on the board to taper endgame
    for (int piece = p; piece <= k; piece++) {
        black_material_score += bit_count(bitboards[piece]) * piece_scores[piece];
    }

    // Calculate the scores for each aspect of the game
    material_score = calculate_material_score();
    square_occupancy_score = calculate_square_occupancy_score();
    square_occupancy_score_endgame = calculate_square_occupancy_score_endgame();
    pawn_structure_score = calculate_pawn_structure_score();
    mobility_score = calculate_mobility_score();
    king_safety_score = calculate_king_safety_score();

    // Determine the game phase
    game_phase = calculate_game_phase(white_material_score + -black_material_score);

    // Combine the scores for each aspect, considering the game phase
    int opening_score = material_score + pawn_structure_score + mobility_score + king_safety_score + square_occupancy_score;
    int endgame_score = material_score + pawn_structure_score + square_occupancy_score_endgame;

    int score = opening_score * game_phase + endgame_score * (1.0f - game_phase);

    if (-abs(score)<-99999) {
        std::cout << opening_score <<  ":" << game_phase << ":" << endgame_score << std::endl;
    }
    // Return the score based on the side to move
    return (side == white ? score : -score);
}

float Skunk::calculate_game_phase(int total_material) {
    int max_material = 2 * (piece_scores[Q] + 2 * piece_scores[R] + 2 * piece_scores[B] + 2 * piece_scores[N] + 8 * piece_scores[P] + piece_scores[K]);
    return (float)total_material / max_material;
}

int Skunk::calculate_material_score() {
    int material_score = 0;

    for (int piece = P; piece <= k; piece++) {
        material_score += piece_count[piece] * piece_scores[piece];
    }

    return material_score;
}

int Skunk::calculate_square_occupancy_score() {
    int score = 0;
    U64 bitboard;

    // Iterate through each piece type (P, N, B, R, Q, K) for both colors
    for (int piece = P; piece <= k; piece++) {
        bitboard = bitboards[piece];

        // Loop through all instances of the current piece type on the board
        while (bitboard) {
            // Get the least significant bit index for the current piece
            int square = __builtin_ctzll(bitboard);

            // Add/subtract the square score based on the piece color (white or black)
            if (piece >= p)
                score -= square_scores[piece % 6][mirror_score[square]] * evaluation_weights[PIECE_SCORE_WEIGHT]; // Black piece (lowercase)
            else
                score += square_scores[piece % 6][square] * evaluation_weights[PIECE_SCORE_WEIGHT]; // White piece (uppercase)

            // Remove the least significant bit from the bitboard
            pop_lsb(bitboard);
        }
    }

    return score;
}

int Skunk::calculate_square_occupancy_score_endgame() {
    int score = 0;
    U64 bitboard;

    // Iterate through each piece type (P, N, B, R, Q, K) for both colors
    for (int piece = P; piece <= k; piece++) {
        bitboard = bitboards[piece];

        // Loop through all instances of the current piece type on the board
        while (bitboard) {
            // Get the least significant bit index for the current piece
            int square = __builtin_ctzll(bitboard);

            // Add/subtract the square score based on the piece color (white or black)
            if (piece >= p)
                score -= eg_tables[piece % 6][mirror_score[square]] * evaluation_weights[PIECE_SCORE_WEIGHT]; // Black piece (lowercase)
            else
                score += eg_tables[piece % 6][square] * evaluation_weights[PIECE_SCORE_WEIGHT]; // White piece (uppercase)

            // Remove the least significant bit from the bitboard
            pop_lsb(bitboard);
        }
    }

    return score;
}

int Skunk::calculate_pawn_structure_score() {
    int pawn_structure_score = 0;
    U64 white_pawns = bitboards[P];
    U64 black_pawns = bitboards[p];

    for (int file = 0; file < 8; file++) {
        U64 white_pawns_on_file = file_masks[MIDDLE][file] & white_pawns;
        U64 black_pawns_on_file = file_masks[MIDDLE][file] & black_pawns;

        // Evaluate doubled pawns
        if (bit_count(white_pawns_on_file) > 1) {
            pawn_structure_score -= evaluation_weights[DOUBLED_PAWNS_WEIGHT] * (float)(bit_count(white_pawns_on_file) - 1);
        }

        if (bit_count(black_pawns_on_file) > 1) {
            pawn_structure_score += evaluation_weights[DOUBLED_PAWNS_WEIGHT] * (float)(bit_count(black_pawns_on_file) - 1);
        }


        // Evaluate isolated pawns
        bool white_pawns_on_left_file = file > 0 && (file_masks[MIDDLE][file - 1] & bitboards[P]);
        bool white_pawns_on_right_file = file < 7 && (file_masks[MIDDLE][file + 1] & bitboards[P]);

        bool black_pawns_on_left_file = file > 0 && (file_masks[MIDDLE][file - 1] & bitboards[p]);
        bool black_pawns_on_right_file = file < 7 && (file_masks[MIDDLE][file + 1] & bitboards[p]);

        if (white_pawns_on_file && !white_pawns_on_left_file && !white_pawns_on_right_file) {
            pawn_structure_score -= evaluation_weights[ISOLATED_PAWNS_WEIGHT] * bit_count(white_pawns_on_file);
        }

        if (black_pawns_on_file && !black_pawns_on_left_file && !black_pawns_on_right_file) {
            pawn_structure_score += evaluation_weights[ISOLATED_PAWNS_WEIGHT] * bit_count(black_pawns_on_file);
        }
    }

    // evaluate passed pawns
    int white_passed_pawns = 0, black_passed_pawns = 0;
    
    U64 pawns = white_pawns;
    while (pawns > 0) {
        int square = __builtin_ctzll(pawns);

        // get the attacks for this pawn and and it with black pawns to get if it is a passed pawn
        if ((pawn_attack_span_masks[white][square] & black_pawns) == 0) {
            white_passed_pawns ++;
        }
        pop_lsb(pawns);
    }

    pawns = black_pawns;
     while (pawns > 0) {
        int square = __builtin_ctzll(pawns);

        // get the attacks for this pawn and and it with black pawns to get if it is a passed pawn
        if ((pawn_attack_span_masks[black][square] & white_pawns) == 0) {
            black_passed_pawns ++;
        }

        pop_lsb(pawns);
    }



    pawn_structure_score += evaluation_weights[PASSED_PAWN_WEIGHT] * bit_count(white_passed_pawns);
    pawn_structure_score -= evaluation_weights[PASSED_PAWN_WEIGHT] * bit_count(black_passed_pawns);

    return pawn_structure_score;
}


int Skunk::calculate_mobility_score() {
    int mobility_score = 0;
    int pieces[] = {N, R, B, n, r, b};

    for (int i = 0; i < 6; i++) {
        int piece = pieces[i];
        U64 bitboard = bitboards[piece];
        while (bitboard) {
            int square = __builtin_ctzll(bitboard);

            // Get this piece's moves
            if (piece >= n) {
                mobility_score -= log(bit_count(get_attacks(piece, square, black)) + 1) * evaluation_weights[MOBILITY_WEIGHT];
            } else {
                mobility_score += log(bit_count(get_attacks(piece, square, white)) + 1) * evaluation_weights[MOBILITY_WEIGHT];
            }
            pop_lsb(bitboard);
        }
    }

    return mobility_score;
}

int Skunk::calculate_king_safety_score() {
    int king_safety_score = 0;


    int king_square_white = __builtin_ctzll(bitboards[K]);
    int king_square_black = __builtin_ctzll(bitboards[k]);

    // White king safety
    int white_king_distance = 0;
    for (int piece = p; piece < k; piece++) {
        U64 bitboard = bitboards[piece];
        while (bitboard) {
            int square = __builtin_ctzll(bitboard);
            int row_attacker = square >> 3;
            int column_attacker = square & 7;
            int row_king = king_square_white >> 3;
            int column_king = king_square_white & 7;

            int distance = 16 - (abs(row_king - row_attacker) + abs(column_king - column_attacker));
            white_king_distance -= distance * king_distance_heuristic[piece % 6];

            pop_lsb(bitboard);
        }
    }

    king_safety_score += (white_king_distance / 10);

    // Black king safety
    int black_king_distance = 0;
    for (int piece = P; piece < K; piece++) {
        U64 bitboard = bitboards[piece];
        while (bitboard) {
            int square = __builtin_ctzll(bitboard);
            int row_attacker = square >> 3;
            int column_attacker = square & 7;
            int row_king = king_square_black >> 3;
            int column_king = king_square_black & 7;

            int distance = 16 - (abs(row_king - row_attacker) + abs(column_king - column_attacker));
            black_king_distance -= distance * king_distance_heuristic[piece % 6];

            pop_lsb(bitboard);
        }
    }
    king_safety_score -= (black_king_distance / 10);

    return king_safety_score * evaluation_weights[KING_SAFETY_WEIGHT];
}


int Skunk::coordinate_to_square(char *coordinate) {
    for (int i=0; i<64; i++) {
        // just bruteforce check which square matches
        if (strcmp(square_to_coordinate[i], coordinate) == 0) {
            return i;
        }
    }
    return -1;
}


// what will stop the search and use the PV line
void Skunk::communicate() {

    /*
     * If the search is move_time, check if time elapsed has passed
     */
    if (move_time > 0) {
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_time).count();

        if (elapsed > move_time) {
            force_stop = 1;
            return ;
        }
    }

    /*
     * Polls stdin to see if there is any data to read
     */

#ifdef _WIN32
    struct timeval timeout;
    timeout.tv_usec = 1;
    timeout.tv_sec = 0;
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    int is_ready = _kbhit();
#else
    struct pollfd fd;
    fd.fd = STDIN_FILENO;
    fd.events = POLLIN;
    fd.revents = 0;
    int is_ready = (poll(&fd, 1, 0)>0 && ((fd.revents & POLLIN) != 0));
#endif

    // checks if it is ready
    if (!is_ready) return;

    char input[20];
    if (fgets(input, 20, stdin) && input[0] != '\n') {
        // got some data in input, lets parse it
        if (strncmp(input, "quit", 4)==0) {
            force_stop = 1;
        } else if (strncmp(input, "stop", 4)==0) {
            force_stop = 1;
        }
    }
    fflush(stdin);
}

void Skunk::parse_go(const std::string& cmd) {
    search_depth = 0;
    move_time = 0;
    btime = 0;
    wtime = 0;

    std::stringstream ss(cmd);
    std::string token;
    while (ss >> token) {
        if (token == "depth") {
            ss >> search_depth;
        } else if (token == "movetime") {
            ss >> move_time;
        } else if (token == "wtime") {
            ss >> wtime;
        } else if (token == "btime") {
            ss >> btime;
        }
    }

    // Check which type of search to do
    if (move_time > 0) {
        search(INT_MAX);
    } else if (search_depth > 0) {
        // Do a depth-limited search
        search(search_depth);
    } else if (wtime > 0 && btime > 0) {
        // calculate movetime intelligently
        move_time = 1000;

        if (side == white) {
            move_time = std::min(DEFAULT_MOVETIME, wtime);
        } else {
            move_time = std::min(DEFAULT_MOVETIME, btime);
        }
        
        // printf("%d\n", move_time);
        search(INT_MAX);
    }
}


void Skunk::parse_position(const std::string& command) {
    /*
     * command looks something like "position startpos" or "position fen <fen>"
     */
    std::string cmd = command.substr(9);

    if (cmd.substr(0, 8) == "startpos") {
        parse_fen(fen_start);
    } else {
        size_t fen_pos = cmd.find("fen");
        if (fen_pos != std::string::npos) {
            parse_fen(cmd.substr(fen_pos + 4));
        } else {
            parse_fen(fen_start);
        }
    }

    size_t moves_pos = cmd.find("moves");
    repitition.count = 0;

    if (moves_pos != std::string::npos) {
        std::string moves_str = cmd.substr(moves_pos + 6);
        std::stringstream moves_ss(moves_str);
        std::string move_str;

        while (std::getline(moves_ss, move_str, ' ')) {
            int move = parse_move(move_str);

            if (move == 0) break;

            if (make_move(move) == 0) continue;
            repitition.table[repitition.count++] = zobrist;
        }
    }
}




int Skunk::parse_move(const std::string& move_string) {
    Moves moves;
    generate_moves(moves);
    if (move_string.length() < 4) return 0;

    int source = (move_string[0] - 'a') + (8 - (move_string[1] - '0')) * 8;
    int target = (move_string[2] - 'a') + (8 - (move_string[3] - '0')) * 8;

    for (int i=0; i<moves.count; i++) {
        int move = moves.list[i];

        if (decode_source(move)==source && decode_destination(move)==target) {
            int promoted = decode_promoted(move);
            // check if it is a promotion or not
            if (promoted) { // there is a promoted piece available
                if ((move_string[4]=='r' || move_string[4] == 'R') && (promoted==r || promoted==R)) return move;
                if ((move_string[4]=='b' || move_string[4] == 'B') && (promoted==b || promoted==B)) return move;
                if ((move_string[4]=='q' || move_string[4] == 'Q') && (promoted==q || promoted==Q)) return move;
                if ((move_string[4]=='n' || move_string[4] == 'N') && (promoted==n || promoted==N)) return move;
                continue;
            }

            return move;
        }
    }
    return 0;
}


perft perft_results;

void Skunk::parse_perft(const std::string& command) {
    int depth = 5;
    std::string depth_str = command.substr(6); // Extract the depth substring
    depth = std::stoi(depth_str); // Convert the depth substring to an integer
    perft_test(depth);
}


int Skunk::perft_test(int depth) {
    
    nodes = 0;

    Moves moves;
    generate_moves(moves);

    auto start = std::chrono::steady_clock::now();

    copy_board();

    for (int i=0; i<moves.count; i++) {
        int move = moves.list[i];

        // check if make_move returned illegal move
        if (make_move(move) == false) {
            restore_board();
            continue;
        };

        int cummulative_nodes = nodes;

        perft_test_helper(depth - 1);

        // old nodes
        int old_nodes = nodes - cummulative_nodes;

        restore_board();

        printf("%s%s%c %d\n", 
            square_to_coordinate[decode_source(move)],
            square_to_coordinate[decode_destination(move)],
            decode_promoted(move) ? ascii_pieces[(decode_promoted(move) % 6) + 6] : ' ',
            old_nodes);
    }

    auto end = std::chrono::steady_clock::now();

    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << std::endl <<  nodes << std::endl;
}

bool Skunk::perft_test_position(const std::string &fen, int expected_result, int depth) {
    // parse_command("position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", skunk);
    parse_fen(fen);
    int result = perft_test(depth);
    if (result == expected_result) {
        std::cout << "\u2713\tPassed (" << result << ")" << std::endl;
        return true;
    }
        
    std::cout << "\tFailed (result was " << result << ", expected was "<< expected_result << ")"  << std::endl;
    exit(0);
    return false;
}


void Skunk::perft_test_helper(int depth) {
    if (depth == 0) {
        nodes ++;
        return;
    }

    Moves moves;
    generate_moves(moves);

    copy_board();
    
    for (int i=0; i<moves.count; i++) {
        int move = moves.list[i];

        if (make_move(move)==false) {
            restore_board();
            continue;
        }

        perft_test_helper(depth - 1);

        restore_board();
    }
}

int Skunk::is_repetition() {
    for (int i = repitition.count-2; i>=0; i--) {
        if (repitition.table[i] == zobrist) {
            return 1;
        }
    }
    return 0;
}

 bool Skunk::make_move(int move) {
    int source = decode_source(move);
    int target = decode_destination(move);
    int enp = decode_enpassant(move);
    int castling = decode_castle(move);
    int piece = decode_piece(move);
    int promoted = decode_promoted(move);

    pop_bit(bitboards[piece], source);
    zobrist ^= piece_keys[piece][source];

    int pawn = P, knight = N, king = K, queen = Q, bishop = B, rook = R;
    U64 *opponent_bitboards = bitboards + 6;
    if (side == black) {
        opponent_bitboards = bitboards;
        pawn = p, knight = n, king = k, queen = q, bishop = b, rook = r;
    }

    int victim = get_piece(target);

    if (victim > -1) {
        pop_bit(bitboards[victim], target);
        piece_count[victim] --;
        zobrist ^= piece_keys[victim][target];
    }

    set_bit(bitboards[piece], target);
    zobrist ^= piece_keys[piece][target];

    if (promoted) {
        pop_bit(bitboards[pawn], target);
        set_bit(bitboards[promoted], target);
        piece_count[pawn] --;
        piece_count[promoted] ++;
        zobrist ^= piece_keys[pawn][target];
        zobrist ^= piece_keys[promoted][target];
    }

    if (enp) {
        if (side == white) {
            pop_bit(bitboards[p], target + 8);
            piece_count[p] --;
            zobrist ^= piece_keys[p][target + 8];
        } else {
            pop_bit(bitboards[P], target - 8);
            piece_count[P] --;
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

    // loop over white pieces bitboards
    for (int bb_piece = P; bb_piece <= K; bb_piece++)
        // update white occupancies
        occupancies[white] |= bitboards[bb_piece];

    // loop over black pieces bitboards
    for (int bb_piece = p; bb_piece <= k; bb_piece++)
        // update black occupancies
        occupancies[black] |= bitboards[bb_piece];

    // update both sides occupancies
    occupancies[both] |= occupancies[white];
    occupancies[both] |= occupancies[black];

    side ^= 1;

    moves ++;

    zobrist ^= side_key;

    // make sure that king has not been exposed into a check
    if (is_square_attacked((side == white) ? __builtin_ctzll(bitboards[k]) : __builtin_ctzll(bitboards[K]), side))
    {    
        // return illegal move
        return false;
    }
    
    return true;
}

void Skunk::print_move_detailed(int move) {
    printf("%-9d %-6s %-8s %-7c %-9d %-9d %-9d\n",
           move,
           square_to_coordinate[decode_source(move)],
           square_to_coordinate[decode_destination(move)],
           ascii_pieces[decode_piece(move)],
           score_move(move),
           decode_enpassant(move),
           decode_castle(move)
    );
}
void Skunk::show_sort() {
    Moves moves;
    generate_moves(moves);
    printf("Moves before sort:\n");
    for (int i =0; i<moves.count; i++) {
        int move = moves.list[i];
        print_move(move);
        printf("\n");
    }
    // sort_moves(moves_list.moves, moves_list.count);
    // printf("After sort:\n");
    // for (int i=0; i<moves_list.count; i++) {
    //     print_move(moves_list.moves[i]);
    //     printf("\n");
    // }
}
void Skunk::print_move(int move) {
    int promoted = decode_promoted(move);
    if (promoted) {
        printf("%s%s%c", square_to_coordinate[decode_source(move)],
               square_to_coordinate[decode_destination(move)],
               ascii_pieces[promoted]);
    }

    else
        printf("%s%s", square_to_coordinate[decode_source(move)],
               square_to_coordinate[decode_destination(move)]);
}