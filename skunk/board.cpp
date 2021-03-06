//
// Created by Jeremy Colegrove on 11/8/21.
//

#include "board.h"
#include <iostream>

#include <cmath>


/*****************************\
===============================
         initialization
===============================
\*****************************/

void Skunk::parse_fen(char *fen) {
// reset board position (bitboards)
    memset(bitboards, 0ULL, sizeof(bitboards));

    // reset occupancies (bitboards)
    memset(occupancies, 0ULL, sizeof(occupancies));
    memset(piece_count, 0, sizeof(piece_count));

    // reset game state variables
    side = 0;
    enpassant = no_square;
    castle = 0;

    // loop over board ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop over board files
        for (int file = 0; file < 8; file++)
        {
            // init current square
            int square = rank * 8 + file;

            // match ascii pieces within FEN string
            if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z'))
            {
                // init piece type
                int piece = char_pieces[*fen];

                // set piece on corresponding bitboard
                set_bit(bitboards[piece], square);

                piece_count[piece] ++;

                // increment pointer to FEN string
                fen++;
            }

            // match empty square numbers within FEN string
            if (*fen >= '0' && *fen <= '9')
            {
                // init offset (convert char 0 to int 0)
                int offset = *fen - '0';

                // define piece variable
                int piece = -1;

                // loop over all piece bitboards
                for (int bb_piece = P; bb_piece <= k; bb_piece++)
                {
                    // if there is a piece on current square
                    if (get_bit(bitboards[bb_piece], square))
                        // get piece code
                        piece = bb_piece;
                }

                // on empty current square
                if (piece == -1)
                    // decrement file
                    file--;

                // adjust file counter
                file += offset;

                // increment pointer to FEN string
                fen++;
            }

            // match rank separator
            if (*fen == '/')
                // increment pointer to FEN string
                fen++;
        }
    }

    // got to parsing side to move (increment pointer to FEN string)
    fen++;

    // parse side to move
    (*fen == 'w') ? (side = white) : (side = black);

    // go to parsing castling rights
    fen += 2;

    // parse castling rights
    while (*fen != ' ')
    {
        switch (*fen)
        {
            case 'K': castle |= wk; break;
            case 'Q': castle |= wq; break;
            case 'k': castle |= bk; break;
            case 'q': castle |= bq; break;
            case '-': break;
        }

        // increment pointer to FEN string
        fen++;
    }

    // got to parsing enpassant square (increment pointer to FEN string)
    fen++;

    // parse enpassant square
    if (*fen != '-')
    {
        // parse enpassant file & rank
        int file = fen[0] - 'a';
        int rank = 8 - (fen[1] - '0');

        // init enpassant square
        enpassant = rank * 8 + file;
    }

        // no enpassant square
    else
        enpassant = no_square;

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

    // construct the piece array char_pieces
    // white
    char_pieces['P'] = P;
    char_pieces['N'] = N;
    char_pieces['B'] = B;
    char_pieces['R'] = R;
    char_pieces['Q'] = Q;
    char_pieces['K'] = K;

    //black
    char_pieces['p'] = p;
    char_pieces['n'] = n;
    char_pieces['b'] = b;
    char_pieces['r'] = r;
    char_pieces['q'] = q;
    char_pieces['k'] = k;
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


    //transposition table init
    // try and init the transposition table
#ifdef TRANSPOSITION_TABLE
    transposition_table = new t_entry [HASH_SIZE];

    if (transposition_table == NULL) {
        perror("Can not initialize transposition table, out of memory\n");
        exit(1);
    }
    clear_transposition_tables();
#endif
}

void Skunk::clear_transposition_tables() {
#ifdef TRANSPOSITION_TABLE
    memset(transposition_table, 0ULL, sizeof(t_entry) * HASH_SIZE);
#endif
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
            int square = get_ls1b_index(bitboard);
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
    if (transposition_table != NULL) {
        delete[] transposition_table;
    }
}

void Skunk::construct_file_masks() {

    memset(file_masks, 0ULL, sizeof(file_masks));

    U64 mask;

    // middle
    for (int square = 0; square < 64; square++) {
        mask = 0ULL;
        for (int row = 0; row < 8; row++) {
            set_bit(mask, row * 8 + (square % 8));
        }
        file_masks[MIDDLE][square] = mask;
    }

    // left
    for (int square = 0; square < 64; square++) {
        mask = 0ULL;
        if (square % 8 == 0) continue;
        for (int row = 0; row < 8; row++) {
            set_bit(mask, row * 8 + ((square-1) % 8));
        }
        file_masks[LEFT][square] = mask;
    }

    // right
    for (int square = 0; square < 64; square++) {
        mask = 0ULL;
        if (square % 8 == 7) continue;
        for (int row = 0; row < 8; row++) {
            set_bit(mask, row * 8 + ((square+1) % 8));
        }
        file_masks[RIGHT][square] = mask;
    }
}



void Skunk::construct_direction_rays() {

    for (int i=0; i<64; i++) {
        // right
        nearest_square[DE][i] = i + (7 - i % 8);

        // up
        nearest_square[DN][i] = i % 8;

        // left
        nearest_square[DW][i] = i - i % 8;

        // down
        nearest_square[DS][i] = 56 + i % 8;


        // upper right
        nearest_square[DNE][i] = i - std::min(i / 8, 7 - i % 8) * 7;

        // lower right
        nearest_square[DSE][i] = i + std::min(7 - i / 8, 7 - i % 8) * 9;

        // lower left
        nearest_square[DSW][i] = i + std::min(7 - i / 8, i % 8) * 7;

        // upper left
        nearest_square[DNW][i] = i - std::min(i / 8, i % 8) * 9;
    }



}
// generate rays from source square to destination square for every square
void Skunk::construct_rays() {
    memset(rays, 0ULL, sizeof(rays));
    int count = 0;
    U64 ray = 0ULL;
    for (int i=0; i<64; i++) {
        // horizontal
        int left_square = (i / 8)*8;
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
        int top_square = i % 8;
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
        int l_diag = std::min(i % 8, i / 8);
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
        int r_diag = std::min(7 - (i % 8), i / 8);
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

        int tr = square / 8;
        int tf = square % 8;
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

        int tr = square % 8;
        int tf = square / 8;
        for (int f=tf-1; f>=1; f--) set_bit(attacks, f*8+tr);
        for (int f=tf+1; f<=6; f++) set_bit(attacks, f*8+tr);
        for (int r=tr+1; r<=6; r++) set_bit(attacks, tf*8+r);
        for (int r=tr-1; r>=1; r--) set_bit(attacks, tf*8+r);

        rook_masks[square] = attacks;
    }
}

U64 Skunk::construct_bishop_attacks(int square, unsigned long long int blockers) {
    U64 moves = 0UL;

    int tr = square / 8;
    int tf = square % 8;
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

    int tr = square % 8;
    int tf = square / 8;
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
int Skunk::is_square_attacked(int square, int side)
{
#ifdef DEBUG
    if (side != 0 && side != 1) {
        printf("Weird, side is %d\n", side);
    }

    assert(side == 0 || side == 1);
    assert(square > -1 && square < 64);
#endif

    // attacked by white pawns
    if ((side == white) && (pawn_masks[black][square] & bitboards[P])) return 1;

    // attacked by black pawns
    if ((side == black) && (pawn_masks[white][square] & bitboards[p])) return 1;

    // attacked by knights
    if (knight_masks[square] & ((side == white) ? bitboards[N] : bitboards[n])) return 1;

    // attacked by bishops
    if (get_bishop_attacks(square, occupancies[both]) & ((side == white) ? bitboards[B] : bitboards[b])) return 1;

    // attacked by rooks
    if (get_rook_attacks(square, occupancies[both]) & ((side == white) ? bitboards[R] : bitboards[r])) return 1;

    // attacked by bishops
    if (get_queen_attacks(square, occupancies[both]) & ((side == white) ? bitboards[Q] : bitboards[q])) return 1;

    // attacked by kings
    if (king_masks[square] & ((side == white) ? bitboards[K] : bitboards[k])) return 1;

    // by default return false
    return 0;
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
        if (i % 8 == 0) {
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
        if (i % 8 == 0) {
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

int Skunk::get_ls1b_index(U64 board) {
    if (board == 0) return -1;
    unsigned int folded;
    folded  = (int)((board ^ (board-1)) >> 32);
    folded ^= (int)( board ^ (board-1)); // lea
    return lsb_64_table[folded * 0x78291ACF >> 26];
}

U64 Skunk::set_occupancy(int index, int bits_in_mask, U64 attack_mask) {
    U64 occupancy = 0ULL;

    for (int i=0; i<bits_in_mask; i++)
    {
        int square = get_ls1b_index(attack_mask);
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
        sliders |= get_bishop_attacks(get_ls1b_index(pieces), occupancies[both]);
        pop_lsb(pieces);
    }

    pieces = bitboards[rook];
    while (pieces) {
        sliders |= get_rook_attacks(get_ls1b_index(pieces), occupancies[both]);
        pop_lsb(pieces);
    }

    // calculate queen attacks
    pieces = bitboards[queen];
    while (pieces) {
        sliders |= get_rook_attacks(get_ls1b_index(pieces), occupancies[both]);
        sliders |= get_bishop_attacks(get_ls1b_index(pieces), occupancies[both]);
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
        jumpers |= pawn_masks[side^1][get_ls1b_index(pieces)];
        pop_lsb(pieces);
    }

    // calculate knight attacks
    pieces = bitboards[knight];
    while (pieces) {
        jumpers |= knight_masks[get_ls1b_index(pieces)];
        pop_lsb(pieces);
    }

    //calculate king attacks
    pieces = bitboards[king];
    while (pieces) {
        jumpers |= king_masks[get_ls1b_index(pieces)];
        pop_lsb(pieces);
    }
    return jumpers;
}



// generate all t_moves
void Skunk::generate_moves(t_moves &moves_list)
{
    // the goal of this move generator is to use the least branching possible, even at the cost of calculation

    // gets the attacked squares that the other side is attacking
    // init move count
    moves_list.count = 0;

    // init variables
    // try and leverage faster registers by using one variable, trying to keep this variable in reg rather than swapping for each piece
    U64 pieces = 0ULL;
    U64 attacked_squares, attack_sliders, attack_jumpers;
    int square;
    /******************************************\
     *
     *                  KING MOVES
     *
    \******************************************/

    // get the right pieces
    int pawn = P, knight=N, bishop=B, rook=R, queen=Q, king=K;
    U64 *opponent_bitboards=bitboards+6;
    if (side == black) {
        pawn = p, knight=n, bishop=b, rook=r, queen=q, king=k;
        opponent_bitboards=bitboards;
    }

    // make sure there are no pieces of same color, and the opponent is not attacking the square
    // the attacked_squares for the king is a little bit more complicated. We must remove the king from the board, then generate enemy_attacks

    int king_square = get_ls1b_index(bitboards[king]);

    pop_bit(occupancies[both], king_square);
    attack_sliders = get_slider_attacks();
    attack_jumpers = get_jumper_attacks();
    attacked_squares = attack_jumpers | attack_sliders;
    set_bit(occupancies[both], king_square);

    pieces = king_masks[get_ls1b_index(bitboards[king])] & ~occupancies[side] & ~attacked_squares;
    // get all of the destinations for the king
    while (pieces) {
        square = get_ls1b_index(pieces);
        // encode the move
        moves_list.moves[moves_list.count++] = encode_move(get_ls1b_index(bitboards[king]), square, king, 0, 0, 0);
        pop_lsb(pieces);
    }

    /******************************************\
     *
     *             INIT OTHER MOVES
     *
    \******************************************/

    // generate the attacked squares as normal after the king is done
    attack_sliders = get_slider_attacks();
    attack_jumpers = get_jumper_attacks();
    attacked_squares = attack_jumpers | attack_sliders;


    // calculate the capture mask (if a piece is giving check, a valid move is capturing the piece)

    U64 capture_mask = 0ULL;
    U64 push_mask    = 0ULL;

    // get a bitboard with all attackers that have king in check on them
    // do pawn first
    capture_mask |= get_bishop_attacks(king_square, occupancies[both]) & (opponent_bitboards[B] | opponent_bitboards[Q]);
    capture_mask |= get_rook_attacks(king_square, occupancies[both]) & (opponent_bitboards[R] | opponent_bitboards[Q]);

    // here we have our slider pieces, we can use these to fill our push mask before adding other pieces
    U64 sliders = capture_mask;
    while (sliders) {
        square = get_ls1b_index(sliders);
        push_mask |= rays[square][king_square];
        pop_lsb(sliders);
    }


    capture_mask |= knight_masks[king_square] & opponent_bitboards[N];
    capture_mask |= pawn_masks[side][king_square] & opponent_bitboards[P];


    // if no pieces are checking the king, then any move on the board is a valid move and we do not check for early escape
    if (capture_mask == 0 && push_mask == 0) {
        capture_mask = 0xFFFFFFFFFFFFFFFF;
        push_mask = 0xFFFFFFFFFFFFFFFF;

    } else {

        // the capture mask contains how many pieces are checking the king.
        // //If more than one piece, the only valid moves are king moves. We can escape early here.
        int bit = get_ls1b_index(capture_mask);
        pop_lsb(capture_mask);
        if (capture_mask > 0) {
            return;
        }
        set_bit(capture_mask, bit);
    }

    /******************************************\
     *
     *              PINNED PIECES
     *
    \******************************************/

    // calculate pinned pieces first and remove them from the board for further move generation
    U64 unpinned_pieces[12];
    memcpy(unpinned_pieces, bitboards, 12 * sizeof(U64));

    // make king a slider piece, and detect intersection with opponent sliding enemy_attacks. Then, any piece on those intersections is pinned
    U64 slider_king = get_bishop_attacks(king_square, occupancies[both]) | get_rook_attacks(king_square, occupancies[both]);
    int pinner_pieces[] = {R, B, Q};
    int pinner_piece, enemy_square, pinned_square, piece, destination;
    // calculate the moves for each piece in each direction
    for (int direction=0; direction<8; direction++) {
        // get the king ray in the opposite direction
        U64 king_ray = slider_king & rays[king_square][nearest_square[7 - direction][king_square]];

        // check pins by each type of piece
        for (int piece_index = 0; piece_index<3; piece_index++) {
            pinner_piece = pinner_pieces[piece_index];

            pieces = opponent_bitboards[pinner_piece];
            while (pieces) {
                enemy_square = get_ls1b_index(pieces);
                pop_lsb(pieces);

                // which type of piece is it? Shoot, we need to know this to generate its attacks
                U64 intersection = get_attacks(pinner_piece, enemy_square, side ^ 1) & rays[enemy_square][nearest_square[direction][enemy_square]] & king_ray;
                intersection &= occupancies[side];
                // if there is no pinned piece, do not try and pop a bit off...just return early here (happens more often than not)
                if (intersection == 0) {
                    continue;
                }

                // get the square that the pinned piece is on
                pinned_square = get_ls1b_index(intersection);
                // get the piece type on that square
                piece = get_piece(pinned_square);

                pop_bit(unpinned_pieces[piece], pinned_square);
//                 we are only able to move on the ray between king and enemy piece, and must move so that it can block check
                U64 attacks = get_attacks(piece, pinned_square, side) & (rays[king_square][enemy_square] | (1ULL << enemy_square)) & (push_mask | capture_mask);

//              go through each valid attack and add it to the list of moves
                while (attacks) {
                    destination = get_ls1b_index(attacks);
                    moves_list.moves[moves_list.count++] = encode_move(pinned_square, destination, piece, 0, 0, 0);
                    pop_lsb(attacks);
                }
            }
        }
    }
    /******************************************\
     *
     *              REGULAR MOVES
     *
    \******************************************/

//    // we now are only dealing with unpinned pieces and no king movements, just plain and simple movements from here on out...
    U64 attacks;


    U64 filtered_capture_mask = capture_mask, filtered_push_mask = push_mask;
    // do pawn attacks first
    if (enpassant != no_square) {
        filtered_capture_mask |= (1ULL << enpassant);
        filtered_push_mask |= (1ULL << enpassant);
    }

    pieces = unpinned_pieces[pawn];
    while (pieces) {
        square = get_ls1b_index(pieces);
        // get this persons attacks

        attacks = get_attacks(pawn, square, side) & (filtered_capture_mask | filtered_push_mask);
        // add each filtered (non promotion) attack
        U64 filtered = attacks & ~row8 & ~row1;

        // filter out pawn promotions
        while (filtered) {
            int destination = get_ls1b_index(filtered);
            if (destination == enpassant) {
                // remove both pawns from the board, check for check
                int victim = enpassant + 8*(side==white?1:-1);

                pop_bit(occupancies[both], square);

                pop_bit(occupancies[both], victim);
                // check if the king is horizontally in check by any queen or rook
                // include any pinned pieces in this check
                // check each queen
                int lsq = nearest_square[DW][king_square], rsq=nearest_square[DE][king_square];

                U64 horizontal_mask = (rays[king_square][lsq] | rays[king_square][rsq] | (1ULL << lsq) | (1ULL << rsq) | (1ULL << king_square));

                // only get rooks and queens on the horizontal row
                U64 horizontal_attackers = (opponent_bitboards[R] | opponent_bitboards[Q]) & horizontal_mask;

                U64 sliders = 0ULL;

                while (horizontal_attackers) {
                    int horizontal_attacker_square = get_ls1b_index(horizontal_attackers);
                    sliders |= get_rook_attacks(horizontal_attacker_square, occupancies[both]);
                    pop_lsb(horizontal_attackers);
                }

                if ((sliders & bitboards[king]) == 0) {
                    moves_list.moves[moves_list.count++] = encode_move(square, destination, pawn, 0, 1, 0);
                }

                set_bit(occupancies[both], square);

                set_bit(occupancies[both], victim);
            } else {
                moves_list.moves[moves_list.count++] = encode_move(square, destination, pawn, 0, 0, 0);
            }
            pop_lsb(filtered);
        }
        // get pawn pushes/captures onto the 8th rank
        filtered = attacks & (row8 | row1);
        while (filtered) {
            int destination = get_ls1b_index(filtered);
            moves_list.moves[moves_list.count++] = encode_move(square, destination, pawn, rook, 0, 0);
            moves_list.moves[moves_list.count++] = encode_move(square, destination, pawn, bishop, 0, 0);
            moves_list.moves[moves_list.count++] = encode_move(square, destination, pawn, queen, 0, 0);
            moves_list.moves[moves_list.count++] = encode_move(square, destination, pawn, knight, 0, 0);

            pop_lsb(filtered);
        }
        pop_lsb(pieces);
    }

    // do rook, queen, bishop, and knight moves
    int regular_pieces[] = {rook, queen, bishop, knight};
    for (int i=0; i<4; i++) {
        piece = regular_pieces[i];
        pieces = unpinned_pieces[piece];

        while (pieces) {
            square = get_ls1b_index(pieces);
            attacks = get_attacks(piece, square, side) & (capture_mask | push_mask);
            while (attacks) {
                destination = get_ls1b_index(attacks);

                moves_list.moves[moves_list.count++] = encode_move(square, destination, piece, 0, 0, 0);
                pop_lsb(attacks);
            }
            pop_lsb(pieces);
        }
    }

    /******************************************\
     *
     *             CASTLING MOVES
     *
    \******************************************/
    if (side == white && (attacked_squares & bitboards[K]) == 0) {
        // if castling is available and king is not in check
        if (castle & wk && ((attacked_squares | occupancies[both]) & castle_mask_wk) == 0) {
            moves_list.moves[moves_list.count++] = encode_move(e1, g1, K, 0, 0, 1);
            moves_list.contains_castle = 1;
        }

        if (castle & wq && (attacked_squares & castle_attack_mask_wq) ==0 && (occupancies[both] & castle_piece_mask_wq) == 0 ) {
            moves_list.moves[moves_list.count++] = encode_move(e1, c1, K, 0, 0, 1);
            moves_list.contains_castle = 1;
        }
    } else if (side == black && (attacked_squares & bitboards[k]) == 0) {
        if (castle & bk && ((attacked_squares | occupancies[both]) & castle_mask_bk) == 0) {
            moves_list.moves[moves_list.count++] = encode_move(e8, g8, k, 0, 0, 1);
            moves_list.contains_castle = 1;
        }

        if (castle & bq && (attacked_squares & castle_attack_mask_bq) ==0 && (occupancies[both] & castle_piece_mask_bq) == 0 ) {
            moves_list.moves[moves_list.count++] = encode_move(e8, c8, k, 0, 0, 1);
            moves_list.contains_castle = 1;
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

void Skunk::add_move(t_moves &moves_list, int move) {
    // check if it is a legal move
    moves_list.moves[moves_list.count] = move;
    moves_list.count++;
}

void Skunk::print_moves(t_moves &moves_list)
{
    printf("%-9s %-6s %-8s %-7s %-9s %-9s %-9s\n","num", "source", "target", "piece", "score", "enpassant", "castle");

    for (int i=0; i<moves_list.count; i++)
    {
        int move = moves_list.moves[i];
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
    printf("Total of %d moves.\n", moves_list.count);
}

void Skunk::test_moves_sort() {
    t_moves moves_list;
    generate_moves(moves_list);
    sort_moves(moves_list);
    // print the scores for checking
    for (int i = 0; i < moves_list.count; i++) {
        printf("%d\n", score_move(moves_list.moves[i]));
    }
}

void Skunk::sort_moves(t_moves &moves_list) {
    /****************************************\
     *             SCORE MOVES
    \****************************************/
    // do an insertion sort for best-case O(n) time
    int scores[moves_list.count];
    // compute the scores for each move
    for (int i=0; i<moves_list.count; i++) {
        scores[i] = score_move(moves_list.moves[i]);
    }

    /****************************************\
     *             INSERTION SORT
    \****************************************/
    for (int i=1; i<moves_list.count; i++) {
        int score = scores[i];
        int move = moves_list.moves[i];

        int j = i - 1;
        while (j >= 0 && scores[j] < score) {
            scores[j+1] = scores[j];
            moves_list.moves[j+1] = moves_list.moves[j];
            j --;
        }
        scores[j + 1] = score;
        moves_list.moves[j + 1] = move;
    }
}

int Skunk::score_move(int move) {

    int score = 0;

    // check if the move exists in the previous search results
    if (ply < previous_pv_line.cmove && move == previous_pv_line.argmove[ply]) {
        score += 20000;
    }

    // consult the lookup table
    int piece = decode_piece(move);
    int victim = get_piece(decode_destination(move));



    if (decode_promoted(move)) {
        // we want to check promotions high as well
        score += 5000;

    }

    if (is_capture(move)) {
        score += mvv_lva[piece][victim] + 10000;
    } else {

#ifdef KILLER_HISTORY
        // score killer move 1
        if (ply < MAX_PLY && killer_moves[0][ply] == move) {
            return 9000;
        }
        // score killer move 2
        else if (ply < MAX_PLY && killer_moves[1][ply] == move) {
            return 8000;
        }

        // score history move
        return history_moves[side][decode_source(move)][decode_destination(move)];
#endif

    }

    return score;
}

int Skunk::is_check() {
    return is_square_attacked(get_ls1b_index(bitboards[side==white?K:k]), side^1);
}

void Skunk::write_hash_entry(int score, int depth, int move, int flag) const {
#ifdef TRANSPOSITION_TABLE
    t_entry  *entry = &transposition_table[zobrist % HASH_SIZE];

    // if there is a clash, choose the TT with the higher nodes searched
    entry->score = score;
    entry->depth = depth;
    entry->hash = zobrist;
    entry->move = move;
    entry->flags = flag;
    entry->nodes = nodes;
#endif
}

int Skunk::quiesence(int alpha, int beta) {

    q_nodes ++;
    if (force_stop) return 0;

    // check if we should return or not
    if (nodes % time_check_node_interval == 0) {
        communicate();
    }

    t_moves moves_list;

    generate_moves(moves_list);

    int check = is_check();

    int evaluation = evaluate() + ((int)log(moves_list.count + 1) * 0.5 * evaluation_weights[MOBILITY_WEIGHT]) - (check?evaluation_weights[KING_SAFETY_WEIGHT]:0);

#ifdef ALPHA_BETA
    if (evaluation >= beta) {
        return evaluation;
    }
#endif


    if (evaluation > alpha) {
        alpha = evaluation;
    }

    sort_moves(moves_list);

    int score = INT_MIN;

    copy_board();

    for (int i=0; i<moves_list.count; i++) {

        int move = moves_list.moves[i];

        if (!is_capture(move)) continue;

#ifdef FUTILITY_PRUNE
        // futility pruning. If the piece we are capturing does not help us improve, just ignore going down this branch + some margin for error (two queens)
        int victim = get_piece(decode_destination(move));

        if (!is_check() && (abs(piece_scores[victim]) + evaluation + piece_scores[Q]*10) < alpha) {
            return alpha;
        }
#endif

        make_move(move, only_captures);

        int test = -quiesence(-beta, -alpha);

        score = (((score) > (test)) ? (score) : (test));

#ifdef DEBUG
        assert(zobrist == generate_zobrist());
#endif
        restore_board();
        moves --;

#ifdef ALPHA_BETA
        if (score >= beta) {
            return beta;
        }
#endif

        if (score > alpha) {
            alpha = score;
        }
    }

    return alpha;
}



int Skunk::negamax(int alpha, int beta, int depth, int verify, int do_null, t_line *pline) {

    nodes ++ ;

    // check if we should return or not
    if ((nodes % time_check_node_interval) == 0) {
        communicate();
    }

    if (force_stop) return 0;

    t_moves moves_list;

    generate_moves(moves_list);

    sort_moves(moves_list);

    int check = is_check();

    // base case
    if (depth < 1) {
        if (pline != NULL) pline->cmove = 0;
        return quiesence(alpha, beta);
    }

    // checkmate / stalemate
    if (moves_list.count == 0) {
        if (check) {
            return (-CHECKMATE) + ply;
        } else return 0;
    }



    // 3-fold repetition
    if (ply && is_repetition()) {
        return -evaluate()*0.25; // instead of returning just a draw best_score, include a contempt factor (simply evaluate x weight). If white is losing, we want to draw so invert eval
    }


    t_line line = {.cmove = 0 };

    int best_score = INT_MIN, best_move = 0, current_move, current_score;

    // check extension
    if (check) depth ++;


#ifdef TRANSPOSITION_TABLE
    /*
     * A copy of alpha so that we can compare for our TT later
     */
    int _alpha = alpha;
    /*
     * Check if there is a valid transposition table entry that we can use instead
     * Try to get a transposition table entry and check its relationship with alpha, beta.
     * We might have to restore alpha, beta instead of exact value.
     * If we find an exact value and the ply is 1, we need to send current_move to principle variation line
     */
    t_entry *entry = &transposition_table[zobrist % HASH_SIZE];

    int is_pv = (beta - alpha) > 1;

    if (entry->hash == zobrist && entry->depth>=depth && !is_pv) {
        int score = entry->score;

        switch (entry->flags) {
            case HASH_EXACT:
                cache_hit++;
                /*
                 * If the ply is 0 and we find a hit we need to send current_move to principle variation line so it has a current_move
                 * However, if the ply is greater than one we do not NEED to put current_move in PV line.
                 */
                if (ply == 0 && pline != NULL) {
                    // write the current_move to our PV line
                    pline->argmove[0] = entry->move;
                    memcpy(pline->argmove + 1, line.argmove, line.cmove * sizeof(int));
                    pline->cmove = line.cmove + 1;
                }
                return entry->score;
            case HASH_LOWERBOUND:
                alpha = std::max(alpha, entry->score);
                break;
            case HASH_UPPERBOUND:
                beta = std::min(beta, entry->score);
            default:
                break;
        }

        if (alpha >= beta) {
            cache_hit++;
            return score;
        }
    }
#endif

#ifdef VERIFIED_NULL_MOVE
    int fail_high = 0;

    if (!check && (!verify || depth > 1) && do_null == DO_NULL) {
        // make null current_move
        copy_board();
        side ^= 1;
        zobrist ^= side_key;
        if (enpassant != no_square) zobrist ^= enpassant_keys[enpassant];
        enpassant = no_square;
        int score = -negamax(-beta, -beta+1, depth - 1 - NULL_R, verify, NO_NULL, NULL);
        side ^= 1;
        restore_board();

        if (score >= beta) {
            if (verify) {
                verify = 0;
                depth--;
                fail_high = 1;
            } else {
                return score;
            }
        }
    }
#endif
#ifdef NULL_MOVE
    /*
     * Lets do some null current_move pruning here to see if our opponent has a response to this current_move
     */
    if (do_null==DO_NULL && !check && ply && depth >= NULL_R + 1) {
        // switch sides
        copy_board();
        side ^= 1;
        zobrist ^= side_key;
        if (enpassant != no_square) zobrist ^= enpassant_keys[enpassant];
        enpassant = no_square;
        /*
         * Here we make a call to negamax again, so our opponent makes two moves in a row.
         * We do not want this current_move to contribute to our PV, so we pass null in for that arg
         */
        int best_score = -negamax(-beta, -beta+1, depth - 1 - NULL_R, verify, NO_NULL, NULL);
        side ^= 1;
        restore_board();
        if (best_score >= beta) {
//            depth -= NULL_R;
            return  beta;
        }
    }
#endif

    copy_board();

    // loop through each current_move
    for (int i = 0; i<moves_list.count; i++) {

        current_move = moves_list.moves[i];

        make_move(current_move, all_moves);

#ifdef DEBUG
        assert(zobrist == generate_zobrist());
#endif

        ply ++;

        repitition.table[repitition.count++] = zobrist;


        search:
        if (i == 0) {
            current_score = -negamax(-beta, -alpha, depth -1, verify, DO_NULL, &line);
        }
        else {
            // LMR
            if (i > 3 && depth > 2 && !check && is_capture(current_move) && decode_promoted(current_move) == 0) {
                int LMR_R = 2;
                current_score = -negamax(-alpha - 1, -alpha, depth - LMR_R, verify, NO_NULL, NULL);
            } else current_score = alpha + 1;

            // PVS
            if (current_score > alpha)
            {
                current_score = -negamax(-alpha - 1, -alpha, depth - 1, verify,NO_NULL, NULL);
                if (current_score > alpha && current_score < beta) current_score = -negamax(-beta, -alpha, depth - 1, verify,DO_NULL, &line);
            }
        }

        if (current_score > best_score) {
            best_score = current_score;
            best_move = current_move;
        }

#ifdef VERIFIED_NULL_MOVE
        // check if zugzwang is detected, if so, re-search with increased depth
        if (fail_high && best_score < beta) {
            depth++;
            fail_high = false;
            verify = 1;
            goto search;
        }
#endif

        restore_board();
        ply --;
        repitition.count--;
        moves --;

        if (best_score > alpha) {
            alpha = best_score;
            // write the current_move to our PV line
            if (pline != NULL) {
                pline->argmove[0] = current_move;
                memcpy(pline->argmove + 1, line.argmove, line.cmove * sizeof(int));
                pline->cmove = line.cmove + 1;
            }
        }

        // beta cutoff
        if (alpha >= beta) {

#ifdef KILLER_HISTORY
            // ADD mask ply check here to avoid segfaults
            if (!is_capture(current_move) && ply < MAX_PLY) {
                history_moves[side][decode_source(current_move)][decode_destination(current_move)] += depth*depth;
                killer_moves[1][ply] = killer_moves[0][ply];
                killer_moves[0][ply] = current_move;
            }
#endif

            write_hash_entry(beta, depth, best_move, HASH_LOWERBOUND);

#ifdef ALPHA_BETA
            return beta;
#endif
        }
    }

    /*
     * We update our transposition table entry to reflect current best_score of node, etc.
     */
#ifdef TRANSPOSITION_TABLE
    int flag = HASH_EXACT;
    if (best_score <= _alpha) flag = HASH_UPPERBOUND;
    else if (best_score >= beta) flag = HASH_LOWERBOUND;
    write_hash_entry(best_score, depth, best_move, flag);
#endif

    return best_score;
}


// the top level call to get the best move
int Skunk::search(int maxDepth) {

    memset(killer_moves, 0, sizeof(killer_moves));

    memset(history_moves, 0, sizeof(history_moves));

    // iterate through deepening as we go
    t_line pline = {.cmove = 0};

    start_time = std::chrono::steady_clock::now();

    force_stop = 0;

    int score, useable_depth;
    cache_hit = 0;
    q_nodes = 0;
    nodes = 0;

    int alpha = INT_MIN + 1, beta = INT_MAX, test;

    for (int depth = 1; depth <= maxDepth; depth++) {

        ply = 0;
        pline.cmove = 0;

        test = negamax(alpha, beta, depth, 1, DO_NULL, &pline);

        // only use odd depths, for some reason even depths are slightly weaker
        if (force_stop) break;

        previous_pv_line = pline;
        score = test;
        useable_depth = depth;
    }

    if (UCI_AnalyseMode)
    {
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_time).count();

        if (score < -CHECKMATE + 2000) {
            printf("info transpositions %d ttp: %.4f score mate %d depth %d nodes %d q_nodes %d time %ld pv ", cache_hit, ((float)cache_hit)/nodes, -(score + CHECKMATE) / 2 - 1, useable_depth, nodes, q_nodes, elapsed);
        } else if (score > CHECKMATE - 2000) {
            printf("info transpositions %d ttp: %.4f score mate %d depth %d nodes %d q_nodes %d time %ld pv ", cache_hit,((float)cache_hit)/nodes, (CHECKMATE - score) / 2 + 1, useable_depth, nodes, q_nodes, elapsed);
        } else {
            printf("info transpositions %d ttp: %.4f score cp %d depth %d nodes %d q_nodes %d time %ld pv ", cache_hit, ((float)cache_hit)/nodes, score, useable_depth, nodes, q_nodes, elapsed);
        }

        for (int i=0; i<previous_pv_line.cmove; i++) {
            // loop over the moves within a PV line
            // print PV move
            print_move(previous_pv_line.argmove[i]);
            printf(" ");
        }
        printf("\n");
    }

    printf("bestmove ");
    print_move(previous_pv_line.argmove[0]);
    repitition.table[repitition.count++] = previous_pv_line.argmove[0]; // add this move into repetition table
    printf("\n");

    return pline.argmove[0];
}

int Skunk::evaluate() {

    // count the number of major pieces
    is_endgame = 0;
    int num_majors = 0;
    for (int piece = P; piece<=k; piece++) {
        if (piece == P || piece == p) continue;
        num_majors += piece_count[piece];
    }

    // idk about this lower bound...
    if (num_majors < 8) {
        is_endgame = 1;
    }

    // simply evaluates the piece scores
    U64 bitboard;
    float score = 0;
    int empty_files = 0, isolated_pawns = 0, square_score;
    for (int piece = P; piece <= k; piece++) {
        bitboard = bitboards[piece];

        while (bitboard) {
            int square = get_ls1b_index(bitboard);

            /*********************\
                  SQUARE TABLE
            \*********************/


            if (piece >= p)
            {
                score -= (1 - is_endgame)*square_scores[piece % 6][mirror_score[square]] + is_endgame*eg_tables[piece % 6][square] ;
            }
            else
            {
                score += (1 - is_endgame)*square_scores[piece % 6][mirror_score[square]] + is_endgame*eg_tables[piece % 6][square] ;
            }

            /*********************\
                  PIECE SCORE
            \*********************/

            score += piece_scores[piece] * evaluation_weights[PIECE_SCORE_WEIGHT];

            pop_lsb(bitboard);
        }
    }

    /**************************************\
         DOUBLED PAWNS & ISOLATED PAWNS
    \**************************************/

    // check for doubled pawns for white
    U64 left, middle, right;
    for (int file = 0; file<8; file++) {
        middle = file_masks[MIDDLE][file] & bitboards[P];
        left = file_masks[LEFT][file] & bitboards[P];
        right = file_masks[RIGHT][file] & bitboards[P];

        //isolated pawns
        if (middle && !left && !right)
        {
            score -= evaluation_weights[ISOLATED_PAWNS_WEIGHT]; // the pawn may as well be half gone
        }

        if (!middle)
            empty_files++;
    }

    // check white doubled pawns penalty
    if (empty_files + piece_count[P] > 8) {
        score -=  evaluation_weights[DOUBLED_PAWNS_WEIGHT] * ((empty_files + piece_count[P]) - 8);
    }

    empty_files = 0;

    // check for doubled pawns for black
    for (int file = 0; file<8; file++) {
        middle = (file_masks[MIDDLE][file] & bitboards[p]);
        left = (file_masks[LEFT][file] & bitboards[p]);
        right = (file_masks[RIGHT][file] & bitboards[p]);

        if (middle && !left && !right) {
            // isolated pawn
            score += evaluation_weights[ISOLATED_PAWNS_WEIGHT]; // the pawn may as well be half gone
        }

        if (!middle) {
            empty_files++;
        }
    }

    // check black doubled pawns penalty
    if (empty_files + piece_count[p] > 8) {
        score += evaluation_weights[DOUBLED_PAWNS_WEIGHT] * ((empty_files + piece_count[p]) - 8);
    }

    /*********************\
        PASSED PAWNS
    \*********************/

    int square;
    U64 mask;

    // white passed pawns
    bitboard = bitboards[P];
    while (bitboard) {
        square = get_ls1b_index(bitboard);
        mask = (rays[square-1][nearest_square[DN][square-1]] & not_h_file) | rays[square][nearest_square[DN][square]] | (rays[square+1][nearest_square[DN][square+1]] & not_a_file) ;
        if (!(mask & bitboards[p])) {
            score += evaluation_weights[PASSED_PAWN];
        }
        pop_lsb(bitboard);
    }

    // black passed pawns
    bitboard = bitboards[p];
    while (bitboard) {
        square = get_ls1b_index(bitboard);
        mask = (rays[square-1][nearest_square[DS][square-1]] & not_h_file) | rays[square][nearest_square[DS][square]] | (rays[square+1][nearest_square[DS][square+1]] & not_a_file) ;
        if (!(mask & bitboards[P])) {
            score -= evaluation_weights[PASSED_PAWN];
        }
        pop_lsb(bitboard);
    }

    /*********************\
            MOBILITY
    \*********************/

    // piece white mobility (knights, rooks, bishops)
    int pieces[] = {N, R, B, n, r, b};
    for (int i = 0; i<6; i++) {
        int piece = pieces[piece];
        bitboard = bitboards[pieces[piece]];
        while (bitboard) {
            square = get_ls1b_index(bitboard);

            // get this pieces moves
            if (piece >= n) {

                score -= log(bit_count(get_attacks(piece, square, black) + 1)) * evaluation_weights[MOBILITY_WEIGHT];
            } else {
                score += log(bit_count(get_attacks(piece, square, white) + 1)) * evaluation_weights[MOBILITY_WEIGHT];
            }
            pop_lsb(bitboard);
        }
    }

    /*********************\
         KING SAFETY
    \*********************/

    // get the distance to all of the other pieces
    //white
    // this should only count towards the beginning of the game
    int king_square = get_ls1b_index(bitboards[K]), distance, total, row_attacker, column_attacker, row_king = king_square/8, column_king=king_square%8;
    if (!is_endgame) {
        for (int piece = p; piece < k; piece++) {
            bitboard = bitboards[piece];
            total = 0;
            while (bitboard) {
                square = get_ls1b_index(bitboard);
                // get distance to the piece
                // get row and column
                row_attacker = square / 8;
                column_attacker = square % 8;

                distance = 16 - (abs(row_king - row_attacker) + abs(column_king - column_attacker));

                total -= distance * king_distance_heuristic[piece % 6];

                pop_lsb(bitboard);
            }
            score += (total/10);
        }
    }


    return (side==white?score:-score);
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
     * If the search is move_time, check if
     */
    if (search_type == SEARCH_MOVETIME) {
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_time).count();

        if (elapsed > UCI_DefaultDuration) {
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

void Skunk::parse_option(char *command) {
    // Not implemented
    // get the name
    if ((command = strstr(command, "name"))) {
        command += 5;
        // get the value command
        if (strncmp(command, "UCI_AnalyseMode", 15) == 0 && (command = strstr(command, "value"))) {
            command += 6;

        }
        else if (strncmp(command, "UCI_DefaultDepth", 16) == 0 && (command = strstr(command, "value"))) {
            command += 6;
            UCI_DefaultDepth = strtol(command, NULL, 10);//atoi(current_char);
        }
        else if (strncmp(command, "UCI_DefaultDuration", 19) == 0 && (command = strstr(command, "value"))) {
            command += 6;
            UCI_DefaultDuration = strtol(command, NULL, 10);
        }
    }
}



void Skunk::parse_debug(char *command) {
    // not implemented

}

void Skunk::parse_go(char *command) {
    int depth = UCI_DefaultDepth;
    int duration = UCI_DefaultDuration;
    char *current_char = NULL;
    if ((current_char = strstr(command, "depth"))) {
        search_type = SEARCH_DEPTH;
        current_char += 6;
        depth = strtol(current_char, NULL, 10);
    } else if ((current_char = strstr(command, "movetime"))) {
        current_char += 9;
        search_type = SEARCH_MOVETIME;
        UCI_DefaultDuration = strtol(current_char, NULL, 10);
    }

    search(search_type==SEARCH_DEPTH?depth:INT_MAX);
}

void Skunk::parse_position(char *command) {
    /*
     * command looks something like "position startpos" or "position fen <fen>"
     */
    command += 9;
    char *current_char = command;
    if (strncmp(command, "startpos", 8) == 0) {
        parse_fen(fen_start);
    } else {
        current_char = strstr(command, "fen");
        if (current_char == NULL) {
            parse_fen(fen_start);
        } else {
            current_char += 4;
            parse_fen(current_char);
        }
    }

    current_char = strstr(command, "moves");

    repitition.count = 0;
    if (current_char != NULL) {
        current_char += 6;
        while (*current_char) {
            int move = parse_move(current_char);

            if (move == 0) break;

            make_move(move, all_moves);
            repitition.table[repitition.count++] = zobrist;
            while (*current_char && *current_char != ' ') current_char ++;
            current_char ++;
        }
    }
}

void Skunk::parse_perft(char *command) {
    int depth = 5;
    depth = atoi(command + 6);
    perft_test(depth);
}

int Skunk::parse_move(char *move_string) {
    t_moves moves;
    generate_moves(moves);

    if (strlen(move_string) < 4) return 0;

    int source = (move_string[0] - 'a') + (8 - (move_string[1] - '0')) * 8;
    int target = (move_string[2] - 'a') + (8 - (move_string[3] - '0')) * 8;
    for (int i=0; i<moves.count; i++) {
        int move = moves.moves[i];
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

void Skunk::perft_test(int depth) {
    printf("Starting PERFT test...\n");
    memset(&perft_results, 0, sizeof(perft));

    perft_results.total_nodes = 0;
    auto start = std::chrono::steady_clock::now();
    perft_test_helper(depth);
    auto end = std::chrono::steady_clock::now();

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    int per_second = -1;
    if (elapsed > 0) {
        per_second = perft_results.total_nodes/elapsed;
    }

    printf("%-10s\t%-10s\t%-10s\t%-10s\t%-10s\t%-10s\n", "depth", "nodes", "captures", "enpassants", "castles", "promoted");

    for (int i=depth; i>=0; i--) {
        printf("%-10d\t%-10d\t%-10d\t%-10d\t%-10d\t%-10d\n",
               depth - i,
               perft_results.nodes[i+1],
               perft_results.captures[i+1],
               perft_results.enpassants[i+1],
               perft_results.castles[i+1],
               perft_results.promotions[i+1]);
    }
    printf("Nodes: %lld\nSeconds: %f\nNPS: %d\n", perft_results.total_nodes, (float)elapsed/1000, per_second*1000);
}


void Skunk::perft_test_helper(int depth) {

    if (depth < 1) return;

    t_moves new_moves;
    t_moves psuedo;
    t_moves valid = {.count = 0};

    generate_moves(new_moves);

    copy_board();
    int made_moves = 0;
    for (int move_count = 0; move_count < new_moves.count; move_count++) {

        int  move = new_moves.moves[move_count];

        if (is_capture(move)) {
            perft_results.captures[depth]++;
        }
        if (decode_enpassant(move)) {

            perft_results.enpassants[depth] ++;
        }
        if (decode_castle(move)) {
            perft_results.castles[depth] ++;
        }
        if (decode_promoted(move)) {
            perft_results.promotions[depth] ++;
        }

        make_move(move, all_moves);


        valid.moves[valid.count++] = move;

#ifdef DEBUG
        assert(zobrist == generate_zobrist());
#endif

        perft_results.nodes[depth] ++;
        perft_results.total_nodes ++;
        perft_test_helper(depth - 1);


        restore_board();
    }

    // move counts differ
//    if (valid.count != new_moves.count) {
//        printf("Move count differ (%d to %d)\n", valid.count, new_moves.count);
//        print_board();
//        getchar();
//    }
}

int Skunk::is_repetition() {
    for (int i = repitition.count-2; i>=0; i--) {
        if (repitition.table[i] == zobrist) {
            return 1;
        }
    }
    return 0;
}

 int Skunk::make_move(int move, int move_flag) {
//
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

        return 1;
    } else {
        // call make_move recursively
        if (is_capture(move))
            return make_move(move, all_moves);

        // move is not a capture, return 0
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
           decode_castle(move)
    );
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