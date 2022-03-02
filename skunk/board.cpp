//
// Created by Jeremy Colegrove on 11/8/21.
//

#include "board.h"
#include <iostream>
/*****************************\
===============================
         initialization
===============================
\*****************************/

void Skunk::parse_fen(char *fen) {
    // get the length of the string
    size_t fen_length = strlen(fen);
    // reset the occupancies
    memset(occupancies, 0ULL, sizeof(occupancies));
    // reset the bitboards
    memset(bitboards, 0ULL, sizeof(bitboards));
    // reset the side
    side = white;
    // reset castling
    castle = 0;
    // reset enpassant
    enpassant = no_square;
    // init index of current fen character
    int index = 0;
    // loop through fen until space is hit
    for (int square=0; index < strlen(fen); index++)
    {
        // get the fen letter
        char c = fen[index];
        // if it is a space, we are done parsing fen
        if (c == ' ') break;
        // if it is a newline just continue
        if (c == '/') {
            // move on to the next letter
            continue;
        }

        //if the fen is a digit
        if (std::isdigit(c))
        {
            // convert char to integer and move forward that many spaces
            square += c - '0';
            continue;
        }

        // get the piece number
        int piece = char_pieces[c];
        // switch (piece) {
        //     case ''
        // }

        // set the bit on the correct bitboard
        set_bit(bitboards[piece], square);
        square ++;
    }

    // bypass all spaces
    while (index<fen_length && fen[++index]==' ' );
    // check if end of string
    if (index == fen_length) return fill_occupancies();


    //we have made it to the parsing of the sides

    // get which side
    if (fen[index++] == 'w') side = white; else side = black;

    // bypass all spaces
    while (index<fen_length && fen[++index]==' ' );
    // check if end of string
    if (index == fen_length) return fill_occupancies();

    bool space_encountered = false;

    // check white castle king side
    if (fen[index++] == 'K' && !space_encountered) castle |= wk;

    space_encountered = space_encountered || fen[index] == ' ';

    // check white castle queen side
    if (!space_encountered && fen[index++] == 'Q') castle |= wq;

    space_encountered = space_encountered || fen[index] == ' ';

    // check black castle king side
    if (!space_encountered && fen[index++] == 'k') castle |= bk;

    space_encountered = space_encountered || fen[index] == ' ';

    // check black castle queen side
    if (!space_encountered && fen[index++] == 'q') castle |= bq;

    // bypass all spaces
    while (index < fen_length && fen[++index]==' ');
    // check if end of string
    if (index == fen_length) return fill_occupancies();

    // if there is an en passant
    if (fen[index] != '-') {
        // get the file
        int file = fen[index++] - 'a';
        // get the rank
        int rank = 8 - (fen[index++] - '0');
        // convert to square
        enpassant = rank * 8 + file;
    }

    // bypass all spaces
    while (index < fen_length && fen[++index]==' ');
    // check if end of string
    if (index == fen_length) return fill_occupancies();

    //get the number of half t_moves
//    half_moves = fen[index++] - '0';

    // bypass all spaces
    while (index < fen_length && fen[++index]==' ');
    // check if end of string
    if (index == fen_length) return fill_occupancies();

    full_moves = fen[index++] - '0';

//     check if end of string
    if (index == fen_length) return fill_occupancies();

    full_moves = full_moves*10 + fen[index] - '0';

    fill_occupancies();

    zobrist = generate_zobrist();

    print_board();

    printf("Hash for position is %llx\n", zobrist);
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

    init_transposition();
}

void Skunk::init_transposition() {
    seed = 4091583267;//1804289383;
    // intialize zobryk hashing random keys
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

    // try and init the transposition table
    transposition_table = new t_entry [HASH_SIZE];
    if (transposition_table == NULL) {
        perror("Can not initialize transposition table, out of memory\n");
        exit(1);
    }
    memset(transposition_table, 0ULL, sizeof(t_entry) * HASH_SIZE);
}

void Skunk::transposition_table_write(int depth, int score, int flags) {
    t_entry *entry = &transposition_table[zobrist % HASH_SIZE];
    entry->score = score;
    entry->hash = zobrist;
    entry->flags = flags;
    entry->depth = depth;
}

int *Skunk::transposition_table_read(int &alpha, int &beta, int depth) {
    t_entry *result = (t_entry *)malloc(sizeof(t_entry));
    if (result == NULL) perror(strerror(errno));

    t_entry *entry = &transposition_table[zobrist % HASH_SIZE];
    if (entry->hash == zobrist) {
        cache_hit ++;
        if (entry->depth >= depth) {

            if (entry->flags == HASH_ALPHA && entry->score <= alpha) {
                return &alpha;
            }

            if (entry->flags == HASH_BETA && entry->score >= beta) {
                return &beta;
            }
        }
    }

    cache_miss ++;
    return NULL;
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
    delete[] transposition_table;
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
    //Generate the kings tables
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
            #ifdef WIN64
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
//    std::cout << "\nSide: " << (side == white ? "white" : "black") << std::endl;
//    // print en passant square
//    std::cout << "\nEn Passant: " << (enpassant != no_square ?  square_to_coordinate[enpassant] : "none") << std::endl;
//    //print castling rights
//    std::cout << "\nCastling: " << ((castle&wk) ? 'K':'-') << ((castle&wq)?'Q':'-') << ((castle&bk)?'k':'-') << ((castle&bq)?'q':'-') << std::endl;
//
//    std::cout << "\nHalf t_moves: " << half_moves << std::endl;
//
//    std::cout << "\nFull t_moves: " << full_moves << std::endl;



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
    if (board)
    {
        return bit_count((board & -board)-1);
    }
    return -1;
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

// generate all t_moves
void Skunk::generate_moves(t_moves *moves_list)
{
    // init move count
    moves_list->count = 0;

    // define source & target squares
    int source_square, target_square;

    // define current piece's bitboard copy & it's attacks
    U64 bitboard, attacks;

    // loop over all the bitboards
    for (int piece = P; piece <= k; piece++)
    {
        // init piece bitboard copy
        bitboard = bitboards[piece];

        // generate white pawns & white king castling t_moves
        if (side == white)
        {
            // pick up white pawn bitboards index
            if (piece == P)
            {
                // loop over white pawns within white pawn bitboard
                while (bitboard)
                {
                    // init source square
                    source_square = get_ls1b_index(bitboard);

                    // init target square
                    target_square = source_square - 8;

                    // generate quite pawn t_moves
                    if (!(target_square < a8) && !get_bit(occupancies[both], target_square))
                    {
                        // pawn promotion
                        if (source_square >= a7 && source_square <= h7)
                        {
                            add_move(moves_list, encode_move(source_square, target_square, piece, Q, 0, 0, 0, 0));
                            add_move(moves_list, encode_move(source_square, target_square, piece, R, 0, 0, 0, 0));
                            add_move(moves_list, encode_move(source_square, target_square, piece, B, 0, 0, 0, 0));
                            add_move(moves_list, encode_move(source_square, target_square, piece, N, 0, 0, 0, 0));
                        }

                        else
                        {
                            // one square ahead pawn move
                            add_move(moves_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));

                            // two squares ahead pawn move
                            if ((source_square >= a2 && source_square <= h2) && !get_bit(occupancies[both], target_square - 8))
                                add_move(moves_list, encode_move(source_square, (target_square - 8), piece, 0, 0, 1, 0, 0));
                        }
                    }

                    // init pawn attacks bitboard
                    attacks = pawn_masks[side][source_square] & occupancies[black];

                    // generate pawn captures
                    while (attacks)
                    {
                        // init target square
                        target_square = get_ls1b_index(attacks);

                        // pawn promotion
                        if (source_square >= a7 && source_square <= h7)
                        {
                            add_move(moves_list, encode_move(source_square, target_square, piece, Q, 1, 0, 0, 0));
                            add_move(moves_list, encode_move(source_square, target_square, piece, R, 1, 0, 0, 0));
                            add_move(moves_list, encode_move(source_square, target_square, piece, B, 1, 0, 0, 0));
                            add_move(moves_list, encode_move(source_square, target_square, piece, N, 1, 0, 0, 0));
                        }

                        else
                            // one square ahead pawn move
                            add_move(moves_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

                        // pop ls1b of the pawn attacks
                        pop_bit(attacks, target_square);
                    }

                    // generate enpassant captures
                    if (enpassant != no_square)
                    {
                        // lookup pawn attacks and bitwise AND with enpassant square (bit)
                        U64 enpassant_attacks = pawn_masks[side][source_square] & (1ULL << enpassant);

                        // make sure enpassant capture available
                        if (enpassant_attacks)
                        {
                            // init enpassant capture target square
                            int target_enpassant = get_ls1b_index(enpassant_attacks);
                            add_move(moves_list, encode_move(source_square, target_enpassant, piece, 0, 1, 0, 1, 0));
                        }
                    }

                    // pop ls1b from piece bitboard copy
                    pop_bit(bitboard, source_square);
                }
            }

            // castling t_moves
            if (piece == K)
            {
                // king side castling is available
                if (castle & wk)
                {
                    // make sure square between king and king's rook are empty
                    if (!get_bit(occupancies[both], f1) && !get_bit(occupancies[both], g1))
                    {
                        // make sure king and the f1 squares are not under attacks
                        if (!is_square_attacked(e1, black) && !is_square_attacked(f1, black))
                            add_move(moves_list, encode_move(e1, g1, piece, 0, 0, 0, 0, 1));
                    }
                }

                // queen side castling is available
                if (castle & wq)
                {
                    // make sure square between king and queen's rook are empty
                    if (!get_bit(occupancies[both], d1) && !get_bit(occupancies[both], c1) && !get_bit(occupancies[both], b1))
                    {
                        // make sure king and the d1 squares are not under attacks
                        if (!is_square_attacked(e1, black) && !is_square_attacked(d1, black))
                            add_move(moves_list, encode_move(e1, c1, piece, 0, 0, 0, 0, 1));
                    }
                }
            }
        }

            // generate black pawns & black king castling t_moves
        else
        {
            // pick up black pawn bitboards index
            if (piece == p)
            {
                // loop over white pawns within white pawn bitboard
                while (bitboard)
                {
                    // init source square
                    source_square = get_ls1b_index(bitboard);

                    // init target square
                    target_square = source_square + 8;

                    // generate quite pawn t_moves
                    if (!(target_square > h1) && !get_bit(occupancies[both], target_square))
                    {
                        // pawn promotion
                        if (source_square >= a2 && source_square <= h2)
                        {
                            add_move(moves_list, encode_move(source_square, target_square, piece, q, 0, 0, 0, 0));
                            add_move(moves_list, encode_move(source_square, target_square, piece, r, 0, 0, 0, 0));
                            add_move(moves_list, encode_move(source_square, target_square, piece, b, 0, 0, 0, 0));
                            add_move(moves_list, encode_move(source_square, target_square, piece, n, 0, 0, 0, 0));
                        }

                        else
                        {
                            // one square ahead pawn move
                            add_move(moves_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));

                            // two squares ahead pawn move
                            if ((source_square >= a7 && source_square <= h7) && !get_bit(occupancies[both], target_square + 8))
                                add_move(moves_list, encode_move(source_square, (target_square + 8), piece, 0, 0, 1, 0, 0));
                        }
                    }

                    // init pawn attacks bitboard
                    attacks = pawn_masks[side][source_square] & occupancies[white];

                    // generate pawn captures
                    while (attacks)
                    {
                        // init target square
                        target_square = get_ls1b_index(attacks);

                        // pawn promotion
                        if (source_square >= a2 && source_square <= h2)
                        {
                            add_move(moves_list, encode_move(source_square, target_square, piece, q, 1, 0, 0, 0));
                            add_move(moves_list, encode_move(source_square, target_square, piece, r, 1, 0, 0, 0));
                            add_move(moves_list, encode_move(source_square, target_square, piece, b, 1, 0, 0, 0));
                            add_move(moves_list, encode_move(source_square, target_square, piece, n, 1, 0, 0, 0));
                        }

                        else
                            // one square ahead pawn move
                            add_move(moves_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

                        // pop ls1b of the pawn attacks
                        pop_bit(attacks, target_square);
                    }

                    // generate enpassant captures
                    if (enpassant != no_square)
                    {
                        // lookup pawn attacks and bitwise AND with enpassant square (bit)
                        U64 enpassant_attacks = pawn_masks[side][source_square] & (1ULL << enpassant);

                        // make sure enpassant capture available
                        if (enpassant_attacks)
                        {
                            // init enpassant capture target square
                            int target_enpassant = get_ls1b_index(enpassant_attacks);
                            add_move(moves_list, encode_move(source_square, target_enpassant, piece, 0, 1, 0, 1, 0));
                        }
                    }

                    // pop ls1b from piece bitboard copy
                    pop_bit(bitboard, source_square);
                }
            }

            // castling t_moves
            if (piece == k)
            {
                // king side castling is available
                if (castle & bk)
                {
                    // make sure square between king and king's rook are empty
                    if (!get_bit(occupancies[both], f8) && !get_bit(occupancies[both], g8))
                    {
                        // make sure king and the f8 squares are not under attacks
                        if (!is_square_attacked(e8, white) && !is_square_attacked(f8, white))
                            add_move(moves_list, encode_move(e8, g8, piece, 0, 0, 0, 0, 1));
                    }
                }

                // queen side castling is available
                if (castle & bq)
                {
                    // make sure square between king and queen's rook are empty
                    if (!get_bit(occupancies[both], d8) && !get_bit(occupancies[both], c8) && !get_bit(occupancies[both], b8))
                    {
                        // make sure king and the d8 squares are not under attacks
                        if (!is_square_attacked(e8, white) && !is_square_attacked(d8, white))
                            add_move(moves_list, encode_move(e8, c8, piece, 0, 0, 0, 0, 1));
                    }
                }
            }
        }

        // genarate knight t_moves
        if ((side == white) ? piece == N : piece == n)
        {
            // loop over source squares of piece bitboard copy
            while (bitboard)
            {
                // init source square
                source_square = get_ls1b_index(bitboard);

                // init piece attacks in order to get set of target squares
                attacks = knight_masks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    target_square = get_ls1b_index(attacks);

                    // quite move
                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                        add_move(moves_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));

                    else
                        // capture move
                        add_move(moves_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

                    // pop ls1b in current attacks set
                    pop_bit(attacks, target_square);
                }


                // pop ls1b of the current piece bitboard copy
                pop_bit(bitboard, source_square);
            }
        }

        // generate bishop t_moves
        if ((side == white) ? piece == B : piece == b)
        {
            // loop over source squares of piece bitboard copy
            while (bitboard)
            {
                // init source square
                source_square = get_ls1b_index(bitboard);

                // init piece attacks in order to get set of target squares
                attacks = get_bishop_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);


                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    target_square = get_ls1b_index(attacks);

                    // quite move
                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                        add_move(moves_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));

                    else
                        // capture move
                        add_move(moves_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

                    // pop ls1b in current attacks set
                    pop_bit(attacks, target_square);
                }


                // pop ls1b of the current piece bitboard copy
                pop_bit(bitboard, source_square);
            }
        }

        // generate rook t_moves
        if ((side == white) ? piece == R : piece == r)
        {
            // loop over source squares of piece bitboard copy
            while (bitboard)
            {
                // init source square
                source_square = get_ls1b_index(bitboard);

                // init piece attacks in order to get set of target squares
                attacks = get_rook_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    target_square = get_ls1b_index(attacks);

                    // quite move
                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square)) {
                        add_move(moves_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));

                    }

                    else
                        // capture move
                    {
                        add_move(moves_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                    }

                    // pop ls1b in current attacks set
                    pop_bit(attacks, target_square);
                }


                // pop ls1b of the current piece bitboard copy
                pop_bit(bitboard, source_square);
            }
        }

        // generate queen t_moves
        if ((side == white) ? piece == Q : piece == q)
        {
            // loop over source squares of piece bitboard copy
            while (bitboard)
            {
                // init source square
                source_square = get_ls1b_index(bitboard);

                // init piece attacks in order to get set of target squares
                attacks = get_queen_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    target_square = get_ls1b_index(attacks);

                    // quite move
                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                        add_move(moves_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));

                    else
                        // capture move
                        add_move(moves_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

                    // pop ls1b in current attacks set
                    pop_bit(attacks, target_square);
                }


                // pop ls1b of the current piece bitboard copy
                pop_bit(bitboard, source_square);
            }
        }

        // generate king t_moves
        if ((side == white) ? piece == K : piece == k)
        {
            // loop over source squares of piece bitboard copy
            while (bitboard)
            {
                // init source square
                source_square = get_ls1b_index(bitboard);

                // init piece attacks in order to get set of target squares
                attacks = king_masks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    target_square = get_ls1b_index(attacks);

                    // quite move
                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                        add_move(moves_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));

                    else
                        // capture move
                        add_move(moves_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

                    // pop ls1b in current attacks set
                    pop_bit(attacks, target_square);
                }

                // pop ls1b of the current piece bitboard copy
                pop_bit(bitboard, source_square);
            }
        }
    }
}

void Skunk::add_move(t_moves *moves_list, int move) {
    moves_list->moves[moves_list->count] = move;
    moves_list->count++;
}


void Skunk::print_moves(t_moves *moves_list)
{
    printf("%-9s %-9s %-9s %-9s %-9s %-9s %-9s %-9s %-9s\n","num", "source", "target", "piece", "promoted", "capture", "d-push", "enpassant", "castle");

    for (int i=0; i<moves_list->count; i++)
    {
        int move = moves_list->moves[i];
        printf("%-9d %-9s %-9s %-9c %-9c %-9d %-9d %-9d %-9d\n",
               move,
               square_to_coordinate[decode_source(move)],
               square_to_coordinate[decode_destination(move)],
               ascii_pieces[decode_piece(move)],
               ascii_pieces[decode_promoted(move)],
               decode_capture(move),
               decode_double(move),
               decode_enpassant(move),
               decode_castle(move)
               );
    }
    printf("Total of %d moves.\n", moves_list->count);

    //RESTORE A MOVE
}

void Skunk::test_moves_sort() {
    t_moves moves_list;
    generate_moves(&moves_list);
    sort_moves(moves_list);
    // print the scores for checking
    for (int i = 0; i < moves_list.count; i++) {
        printf("%d\n", score_move(moves_list.moves[i]));
    }
}

void Skunk::sort_moves(t_moves &moves_list) {
    // do an insertion sort for best-case O(n) time
    int scores[moves_list.count];
    // compute the scores for each move
    for (int i=0; i<moves_list.count; i++) {
        scores[i] = score_move(moves_list.moves[i]);
    }

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
    if (decode_capture(move)) {
        // consult the lookup table
        int attacker = decode_piece(move);
        int victim = 0;
        for (int piece=P; piece <= k; piece++) {
            if (get_bit(bitboards[piece], decode_destination(move))) {
                victim = piece;
                break;
            }
        }
        return mvv_lva[attacker][victim];
    } else {
        // score killer move 1
        if (killer_moves[0][ply] == move) {
            return 100;
        }
        // score killer move 2
        if (killer_moves[1][ply] == move) {
            return 50;
        }

        return history_moves[decode_piece(move)][decode_destination(move)];
        // score history move
    }
    return 0;
}

int Skunk::quiesence(int alpha, int beta, t_line *pline) {

    int check = is_check();

    // check current evaluation
    int evaluation = evaluate();

    if (evaluation > alpha) {
        alpha = evaluation;
    }

    if (alpha >= beta) return beta;

    t_moves moves_list;

    generate_moves(&moves_list);

    sort_moves(moves_list);

    int legal_moves = 0;

    for (int i=0; i<moves_list.count; i++) {


        int move = moves_list.moves[i];

        copy_board();

        if (!make_move(move, only_captures)) {

            continue;
        }

        legal_moves ++ ;

        ply ++;

        int score = -quiesence(-beta, -alpha, NULL);

        ply --;

        restore_board();


        if (score > alpha) {
            alpha = score;
        }

        if (alpha >= beta) {
            return beta;
        }
    }

    // return terminal state
    if (check && legal_moves == 0) {
        return -CHECKMATE + ply;
    }

    return alpha;
}

int Skunk::is_checkmate() {
    t_moves moves_list;

    generate_moves(&moves_list);
    int legal_moves = 0;
    for (int i=0; i<moves_list.count; i++) {
        copy_board();
        if (!make_move(moves_list.moves[i], all_moves)) {
            continue;
        }
        restore_board();
        legal_moves++;
    }
    if (legal_moves == 0 && is_check()) {
        return 1;
    }
    return 0;
}

int Skunk::is_check() {
    return is_square_attacked(get_ls1b_index(bitboards[side==white?K:k]), side ^ 1);
}

int Skunk::null_ok() {
    // we use 2 criteria
    // not ok if only king and pawns remaining OR
    // side to move has small number of pieces remaining

    return 1;
}

int Skunk::negamax(int alpha, int beta, int depth, t_line *pline) {

    t_line line;

    int check = is_check();

    if (check) depth ++;

    if (depth == 0) {
        pline->cmove = 0;
        // make all capture t_moves
        return evaluate();//quiesence(alpha, beta, NULL);
    }

    if (ply > MAX_PLY - 1) {
        return evaluate();
    }

    int hash_flag = HASH_ALPHA;
    int *entry = transposition_table_read(alpha, beta, depth);
    if (entry != NULL) {
        return *entry;
    }

    // do null move heuristic, give opponent an extra move to try and hit beta cuttof faster
    // using extended null move http://www.elidavid.com/pubs/nmr.pdf
    // gives a more stable search and is better at tactics
//    if (!check && depth > NULL_R + 1 && ply && null_ok()) {
//        // conduct a null-move search if it is legal and desired
//        copy_board();
//
//        side ^= 1;
//
//        zobrist ^= side_key;
//
//        if (enpassant != no_square) {
//            zobrist ^= enpassant;
//        }
//
//        enpassant = no_square;
//
//        int score = -negamax(-beta, -beta + 1, depth - 1 - NULL_R, &line);
//
//        restore_board();
//
//        if (score >= beta) {
//            return beta;
//        }
//    }

    // generate some t_moves
    t_moves moves_list;

    generate_moves(&moves_list);

    sort_moves(moves_list);


    int legal_moves = 0;

    // loop through each move
    for (int i = 0; i<moves_list.count; i++) {

        int move = moves_list.moves[i];

        copy_board();

        if (!make_move(move, all_moves)) {
            continue;
        }

        legal_moves ++;

        ply ++ ;

        int score = -negamax(-beta, -alpha, depth - 1, &line);

        ply -- ;

        restore_board();

        if (score >= beta) {
            transposition_table_write(depth, score, HASH_BETA);
            if (!decode_capture(move)) {
                killer_moves[1][ply] = killer_moves[0][ply];
                killer_moves[0][ply] = move;
            }
            return beta;
        }

        if (score > alpha) {
            hash_flag = HASH_EXACT;
            if (!decode_capture(move)) {
                history_moves[decode_piece(move)][decode_destination(move)] += depth;
            }

            alpha = score;

            // keep track of PV lines (lines of moves the computer thinks will be played
            pline->argmove[0] = move;
            memcpy(pline->argmove + 1, line.argmove, line.cmove * sizeof(int));
            pline->cmove = line.cmove + 1;
        }
    }

    // return terminal state
    if (check && legal_moves == 0) {
        return -CHECKMATE + ply;
    } else if (legal_moves == 0) {
        return 0;
    }


    transposition_table_write(depth, alpha, hash_flag);

    return alpha;
}


// the top level call to get the best move
int Skunk::search(int maxDepth) {

    ply = 0;

    // reset data structures
    memset(killer_moves, 0, sizeof(killer_moves));

    memset(history_moves, 0, sizeof(history_moves));

    // create the structs to pass in as alpha and beta

    t_line line;

    int best = negamax(-CHECKMATE - 10000, CHECKMATE + 10000, maxDepth, &line);


    // print out stats about the search
    printf("Run Stats\n%-15s\t%-15s\t%-15s\t%-15s\n%-15d\t%-20d\t%-15d\t%-15d\n", "TT Hits", "TT Misses","Depth","Best Score", cache_hit, cache_miss, maxDepth, best);

    printf("Principle Variation Moves (Expected Line)\n");

    for (int i=0; i<line.cmove; i++) {
        printf("# %-4d %-2s %-2s | ", line.argmove[0], square_to_coordinate[decode_source(line.argmove[i])], square_to_coordinate[decode_destination(line.argmove[i])]);
    }

    printf("\n");

    return line.argmove[0];
}

int Skunk::evaluate() {
    // simply evaluates the piece scores
    U64 bitboard;
    int score = 0;
    for (int board = P; board <=k; board++) {
        bitboard = bitboards[board];
        while (bitboard) {
            int square = get_ls1b_index(bitboard);
            // add score for the piece existing
            score += piece_scores[board];

            // evaluate positional square
            score += mg_tables[(board<p)?board:board-p][(board<p)?square:flip_square(square)];

            pop_bit(bitboard, square);
        }
    }

    // evaluates the position of each of the pieces
    return (side==white)?score:-score;
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

void Skunk::play(int difficulty) {
    // repeatedly play best move then wait for user to play move
    if (side == white) {
        printf("It is whites turn!\n");
    } else {
        printf("It is blacks turn!\n");
    }

    for (int i = 0; i<50; i++) {



        print_board();

        if (is_checkmate()) {
            printf("Checkmate!");
            break;
        }

        // get user input
        char s[3], d[3];
        int source, destination; // real source destination
        printf("enter square to move (source): ");
        scanf("%2s", s);
        source = coordinate_to_square(s);
        printf("enter square to move to (destination): ");
        scanf("%2s", d);
        destination = coordinate_to_square(d);

        if (source == -1 || destination == -1) {
            printf("The entered coords are not valid!\n");
            continue;
        }

        // get the piece on the square
        int piece = -1;
        for (int i=P; i<=k; i++) {
            if (get_bit(bitboards[i], source)) {
                piece = i;
                break;
            }
        }

        if (piece == -1) {
            printf("No piece on that square!\n");
            continue;
        }

        int capture = 0;
        for (int i=P; i<= k; i++) {
            if (get_bit(bitboards[i], destination)) {
                capture = 1;
                break;
            }
        }


        int move = encode_move(source, destination, piece, 0, capture, 0, 0, 0);

        t_moves moves_list;
        generate_moves(&moves_list);
        int valid = 0;
        for (int i=0; i<moves_list.count; i++) {
            if (decode_source(move) == decode_source(moves_list.moves[i])
            && decode_destination(move) == decode_destination(moves_list.moves[i])) {
                valid  = 1;
                break;
            }
        }

        // try to make the t_moves
        if (!valid) {
            continue;
        }

        copy_board();
        if (!make_move(move, all_moves)) {
            printf("Invalid move!\n");
            continue;
        }


        print_board();

        if (is_checkmate()) {
            printf("Checkmate!");
            break;
        }

        printf("Computer is thinking...\n");

        // now let the bot make a move
        int response = search(difficulty);

        make_move(response, all_moves);
    }
}

perft perft_results;

void Skunk::perft_test(int depth) {
    printf("Starting PERFT test...\n");
    memset(perft_results.nodes, 0, sizeof(perft_results.nodes));
    memset(perft_results.captures, 0, sizeof(perft_results.captures));
    memset(perft_results.castles, 0, sizeof(perft_results.castles));
    memset(perft_results.promotions, 0, sizeof(perft_results.promotions));

    perft_test_helper(depth);

    printf("%-10s\t%-10s\t%-10s\t%-10s\t%-10s\n", "depth", "nodes", "captures", "castles", "promoted");

    for (int i=depth; i>=0; i--) {
        printf("%-10d\t%-10d\t%-10d\t%-10d\t%-10d\n",
               depth-i,
               perft_results.nodes[i+1],
               perft_results.captures[i+1],
               perft_results.castles[i+1],
               perft_results.promotions[i+1]);
    }

}



void Skunk::perft_test_helper(int depth) {

    if (depth < 1) return;

    t_moves moves_list;

    generate_moves(&moves_list);

    for (int move_count = 0; move_count < moves_list.count; move_count++) {

        int  move = moves_list.moves[move_count];

        copy_board();

        if (!make_move(move, all_moves)) {
            continue;
        }

        if (decode_capture(move)) {
            perft_results.captures[depth] ++;
        }

        if (decode_castle(move)) {
            perft_results.castles[depth] ++;
        }
        if (decode_promoted(move)) {
            perft_results.promotions[depth] ++;
        }

        perft_results.nodes[depth] ++;

        perft_test_helper(depth - 1);


        restore_board();
    }
}

 int Skunk::make_move(int move, int move_flag) {
//
    if (move_flag == all_moves) {
        // make a move
        copy_board();

        int source = decode_source(move);
        int target = decode_destination(move);
        int enp = decode_enpassant(move);
        int castling = decode_castle(move);
        int piece = decode_piece(move);
        int doube_push = decode_double(move);
        int capture = decode_capture(move);
        int promoted = decode_promoted(move);

        pop_bit(bitboards[piece], source);
        set_bit(bitboards[piece], target);

        zobrist ^= piece_keys[piece][source];
        zobrist ^= piece_keys[piece][target];

        if (capture) {
            int start_piece = (side == white) ? p : P;
            int end_piece = (side == white) ? k : K;

            for (int piece = start_piece; piece < end_piece; piece++) {
                if (get_bit(bitboards[piece], target)) {
                    // remove piece from bitboard
                    pop_bit(bitboards[piece], decode_destination(move));

                    // remove the piece from the board hash
                    zobrist ^= piece_keys[piece][target];
                    break;
                }
            }
        }
        if (promoted) {
            pop_bit(bitboards[side == white ? P : p], target);
            set_bit(bitboards[promoted], target);
            zobrist ^= piece_keys[side == white ? P : p][target];
            zobrist ^= piece_keys[promoted][target];
        }
        if (enp) {
            if (side == white) {
                pop_bit(bitboards[p], target + 8);
                zobrist ^= piece_keys[p][target + 8];
            } else {
                pop_bit(bitboards[P], target - 8);
                zobrist ^= piece_keys[P][target - 8];

            }
        }

        if (enpassant != no_square) {
            zobrist ^= enpassant_keys[enpassant];
        }

        enpassant = no_square;

        if (doube_push) {
            if (side == white) {
                enpassant = target + 8;
                zobrist ^= enpassant_keys[target + 8];
            } else {
                enpassant = target - 8;
                zobrist ^= enpassant_keys[target - 8];

            }
        }
        if (castling) {
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

        zobrist ^= side_key;


//        zobrist = generate_zobrist();
        //
        // ====== debug hash key incremental update ======= //
        //

        // build hash key for the updated position (after move is made) from scratch
//        U64 hash_from_scratch = generate_zobrist();
//
//        // in case if hash key built from scratch doesn't match
//        // the one that was incrementally updated we interrupt execution
//        if (zobrist != hash_from_scratch) {
//            print_board();
//            printf("\n\nWrong hash in make move\n");
//            print_move(move);
//            printf("%llx should be: %llx\n",zobrist, hash_from_scratch);
//            getchar();
//        }

        if (is_square_attacked((side == white) ? get_ls1b_index(bitboards[k]) : get_ls1b_index(bitboards[K]),
                               side)) {
            restore_board();
            return 0;
        }
        return 1;
    } else {
        // call make_move recursively
        if (decode_capture(move))
            make_move(move, all_moves);

        // move is not a capture, return 0
        return 0;
    }
//// quiet moves
//     if (move_flag == all_moves)
//     {
//         // preserve board state
//         copy_board();
//
//         // parse move
//         int source_square = decode_source(move);
//         int target_square = decode_destination(move);
//         int piece = decode_piece(move);
//         int promoted_piece = decode_promoted(move);
//         int capture = decode_capture(move);
//         int double_push = decode_double(move);
//         int enpass = decode_enpassant(move);
//         int castling = decode_castle(move);
//
//         // move piece
//         pop_bit(bitboards[piece], source_square);
//         set_bit(bitboards[piece], target_square);
//
//         // hash piece
//         zobrist ^= piece_keys[piece][source_square]; // remove piece from source square in hash key
//         zobrist ^= piece_keys[piece][target_square]; // set piece to the target square in hash key
//
//         // handling capture moves
//         if (capture)
//         {
//             // pick up bitboard piece index ranges depending on side
//             int start_piece, end_piece;
//
//             // white to move
//             if (side == white)
//             {
//                 start_piece = p;
//                 end_piece = k;
//             }
//
//                 // black to move
//             else
//             {
//                 start_piece = P;
//                 end_piece = K;
//             }
//
//             // loop over bitboards opposite to the current side to move
//             for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++)
//             {
//                 // if there's a piece on the target square
//                 if (get_bit(bitboards[bb_piece], target_square))
//                 {
//                     // remove it from corresponding bitboard
//                     pop_bit(bitboards[bb_piece], target_square);
//
//                     // remove the piece from hash key
//                     zobrist ^= piece_keys[bb_piece][target_square];
//                     break;
//                 }
//             }
//         }
//
//         // handle pawn promotions
//         if (promoted_piece)
//         {
//             // erase the pawn from the target square
//             //pop_bit(bitboards[(side == white) ? P : p], target_square);
//
//
//             // white to move
//             if (side == white)
//             {
//                 // erase the pawn from the target square
//                 pop_bit(bitboards[P], target_square);
//
//                 // remove pawn from hash key
//                 zobrist ^= piece_keys[P][target_square];
//             }
//
//                 // black to move
//             else
//             {
//                 // erase the pawn from the target square
//                 pop_bit(bitboards[p], target_square);
//
//                 // remove pawn from hash key
//                 zobrist ^= piece_keys[p][target_square];
//             }
//
//             // set up promoted piece on chess board
//             set_bit(bitboards[promoted_piece], target_square);
//
//             // add promoted piece into the hash key
//             zobrist ^= piece_keys[promoted_piece][target_square];
//         }
//
//         // handle enpassant captures
//         if (enpass)
//         {
//             // erase the pawn depending on side to move
//             (side == white) ? pop_bit(bitboards[p], target_square + 8) :
//             pop_bit(bitboards[P], target_square - 8);
//
//             // white to move
//             if (side == white)
//             {
//                 // remove captured pawn
//                 pop_bit(bitboards[p], target_square + 8);
//
//                 // remove pawn from hash key
//                 zobrist ^= piece_keys[p][target_square + 8];
//             }
//
//                 // black to move
//             else
//             {
//                 // remove captured pawn
//                 pop_bit(bitboards[P], target_square - 8);
//
//                 // remove pawn from hash key
//                 zobrist ^= piece_keys[P][target_square - 8];
//             }
//         }
//
//         // hash enpassant if available (remove enpassant square from hash key )
//         if (enpassant != no_square) zobrist ^= enpassant_keys[enpassant];
//
//         // reset enpassant square
//         enpassant = no_square;
//
//         // handle double pawn push
//         if (double_push)
//         {
//             // set enpassant aquare depending on side to move
//             //(side == white) ? (enpassant = target_square + 8) :
//             //                  (enpassant = target_square - 8);
//
//             // white to move
//             if (side == white)
//             {
//                 // set enpassant square
//                 enpassant = target_square + 8;
//
//                 // hash enpassant
//                 zobrist ^= enpassant_keys[target_square + 8];
//             }
//
//                 // black to move
//             else
//             {
//                 // set enpassant square
//                 enpassant = target_square - 8;
//
//                 // hash enpassant
//                 zobrist ^= enpassant_keys[target_square - 8];
//             }
//         }
//
//         // handle castling moves
//         if (castling)
//         {
//             // switch target square
//             switch (target_square)
//             {
//                 // white castles king side
//                 case (g1):
//                     // move H rook
//                     pop_bit(bitboards[R], h1);
//                     set_bit(bitboards[R], f1);
//
//                     // hash rook
//                     zobrist ^= piece_keys[R][h1];  // remove rook from h1 from hash key
//                     zobrist ^= piece_keys[R][f1];  // put rook on f1 into a hash key
//                     break;
//
//                     // white castles queen side
//                 case (c1):
//                     // move A rook
//                     pop_bit(bitboards[R], a1);
//                     set_bit(bitboards[R], d1);
//
//                     // hash rook
//                     zobrist ^= piece_keys[R][a1];  // remove rook from a1 from hash key
//                     zobrist ^= piece_keys[R][d1];  // put rook on d1 into a hash key
//                     break;
//
//                     // black castles king side
//                 case (g8):
//                     // move H rook
//                     pop_bit(bitboards[r], h8);
//                     set_bit(bitboards[r], f8);
//
//                     // hash rook
//                     zobrist ^= piece_keys[r][h8];  // remove rook from h8 from hash key
//                     zobrist ^= piece_keys[r][f8];  // put rook on f8 into a hash key
//                     break;
//
//                     // black castles queen side
//                 case (c8):
//                     // move A rook
//                     pop_bit(bitboards[r], a8);
//                     set_bit(bitboards[r], d8);
//
//                     // hash rook
//                     zobrist ^= piece_keys[r][a8];  // remove rook from a8 from hash key
//                     zobrist ^= piece_keys[r][d8];  // put rook on d8 into a hash key
//                     break;
//             }
//         }
//
//         // hash castling
//         zobrist ^= castle_keys[castle];
//
//         // update castling rights
//         castle &= castling_rights[source_square];
//         castle &= castling_rights[target_square];
//
//         // hash castling
//         zobrist ^= castle_keys[castle];
//
//         // reset occupancies
//         memset(occupancies, 0ULL, 24);
//
//         // loop over white pieces bitboards
//         for (int bb_piece = P; bb_piece <= K; bb_piece++)
//             // update white occupancies
//             occupancies[white] |= bitboards[bb_piece];
//
//         // loop over black pieces bitboards
//         for (int bb_piece = p; bb_piece <= k; bb_piece++)
//             // update black occupancies
//             occupancies[black] |= bitboards[bb_piece];
//
//         // update both sides occupancies
//         occupancies[both] |= occupancies[white];
//         occupancies[both] |= occupancies[black];
//
//         // change side
//         side ^= 1;
//
//         // hash side
//         zobrist ^= side_key;
//
//         //
//         // ====== debug hash key incremental update ======= //
//         //
//
//         // build hash key for the updated position (after move is made) from scratch
//         /*U64 hash_from_scratch = generate_hash_key();
//
//         // in case if hash key built from scratch doesn't match
//         // the one that was incrementally updated we interrupt execution
//         if (hash_key != hash_from_scratch)
//         {
//             printf("\n\nMake move\n");
//             printf("move: "); print_move(move);
//             print_board();
//             printf("hash key should be: %llx\n", hash_from_scratch);
//             getchar();
//         }*/
//
//
//         // make sure that king has not been exposed into a check
//         if (is_square_attacked((side == white) ? get_ls1b_index(bitboards[k]) : get_ls1b_index(bitboards[K]), side))
//         {
//             // take move back
//             restore_board();
//
//             // return illegal move
//             return 0;
//         }
//
//             //
//         else
//             // return legal move
//             return 1;
//
//
//     }
//
//         // capture moves
//     else
//     {
//         // make sure move is the capture
//         if (decode_capture(move))
//             make_move(move, all_moves);
//
//             // otherwise the move is not a capture
//         else
//             // don't make it
//             return 0;
//     }
}


void Skunk::print_move(int move) {
    printf("%-9d %-9s %-9s %-9c %-9c %-9d %-9d %-9d %-9d\n",
           move,
           square_to_coordinate[decode_source(move)],
           square_to_coordinate[decode_destination(move)],
           ascii_pieces[decode_piece(move)],
           ascii_pieces[decode_promoted(move)],
           decode_capture(move),
           decode_double(move),
           decode_enpassant(move),
           decode_castle(move)
    );
}