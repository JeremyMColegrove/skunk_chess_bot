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
    if (index == fen_length) return init();


    //we have made it to the parsing of the sides

    // get which side
    if (fen[index++] == 'w') side = white; else side = black;

    // bypass all spaces
    while (index<fen_length && fen[++index]==' ' );
    // check if end of string
    if (index == fen_length) return init();

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
    if (index == fen_length) return init();

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
    if (index == fen_length) return init();

    //get the number of half t_moves
//    half_moves = fen[index++] - '0';

    // bypass all spaces
    while (index < fen_length && fen[++index]==' ');
    // check if end of string
    if (index == fen_length) return init();

    full_moves = fen[index++] - '0';

//     check if end of string
    if (index == fen_length) return init();

    full_moves = full_moves*10 + fen[index] - '0';

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
int Skunk::score_to_tt(int score, int ply)
{
    return score;//(score > CHECKMATE-1000) ? score + ply : (score < -CHECKMATE+1000) ? score - ply : score;
}

int Skunk::score_from_tt(int score, int ply)
{
    return score;//(score > CHECKMATE) ? score - ply : (score < -CHECKMATE) ? score + ply : score;
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
void Skunk::generate_moves(t_moves &moves_list)
{
    // init move count
    moves_list.count = 0;

    // define source & target squares
    int source_square = 0, target_square = 0;

    // define current piece's bitboard copy & it's attacks
    U64 bitboard = 0ULL, attacks = 0ULL;

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

    // at the very end, sort the moves

}

void Skunk::add_move(t_moves &moves_list, int move) {
    moves_list.moves[moves_list.count] = move;
    moves_list.count++;
}


void Skunk::print_moves(t_moves &moves_list)
{
    printf("%-9s %-6s %-8s %-7s %-9s %-9s %-9s %-9s %-9s\n","num", "source", "target", "piece", "score", "capture", "d-push", "enpassant", "castle");

    for (int i=0; i<moves_list.count; i++)
    {
        int move = moves_list.moves[i];
        printf("%-9d %-6s %-8s %-7c %-9d %-9d %-9d %-9d %-9d\n",
               move,
               square_to_coordinate[decode_source(move)],
               square_to_coordinate[decode_destination(move)],
               ascii_pieces[decode_piece(move)],
               score_move(move),
               decode_capture(move),
               decode_double(move),
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

    // check if the move exists in the previous search results
    if (ply < previous_pv_line.cmove && move == previous_pv_line.argmove[ply]) {
        return 20000;
    }


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
        return mvv_lva[attacker][victim] + 10000;
    } else {
        // score killer move 1
        if (ply < MAX_PLY && killer_moves[0][ply] == move) {
            return 9000;
        }
        // score killer move 2
        else if (ply < MAX_PLY && killer_moves[1][ply] == move) {
            return 8000;
        }

        return history_moves[side][decode_source(move)][decode_destination(move)];
        // score history move
    }
    return 0;
}


int Skunk::is_checkmate() {
    t_moves moves_list;

    generate_moves(moves_list);
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
    return is_square_attacked(get_ls1b_index(bitboards[side==white?K:k]), side^1);
}

int Skunk::quiesence(int alpha, int beta, int depth) {

    nodes ++;

    // check if we should return or not
//    if (nodes % time_check_node_interval == 0) {
//        if (check_time()) {
//            return NO_VALUE;
//        }
//    }

    int evaluation = evaluate();

    if (evaluation >= beta) {
        return evaluation;
    }

    if (evaluation > alpha) {
        alpha = evaluation;
    }

    t_moves moves_list;

    generate_moves(moves_list);

    sort_moves(moves_list);

    int score = INT_MIN;


    for (int i=0; i<moves_list.count; i++) {

        int move = moves_list.moves[i];

        copy_board();

        if (!make_move(move, only_captures)) {
            continue;
        }

        int test = -quiesence(-beta, -alpha, depth -1);
        score = std::max(score, test);



#ifdef DEBUG
        assert(zobrist == generate_zobrist());
#endif
        restore_board();

        // cutoff for time expiration
        if (abs(test) == NO_VALUE) {
            return NO_VALUE;
        }

        if (score >= beta) {
            return beta;
        }

        if (score > alpha) {
            alpha = score;
        }
    }

    return alpha;
}



int Skunk::negamax(int alpha, int beta,int depth, t_line *pline) {

    nodes ++ ;

    // check if we should return or not
//    if (nodes % time_check_node_interval == 0) {
//        if (check_time()) {
//            return NO_VALUE;
//        }
//    }

    /*
     * Check for repetitions (does not work?)
     */
    if (ply && is_repitition()) {
        return 0;
    }

    /*
     * Base case
     * Here we need to set the number of moves in our PV to 0 so that when collapsing up the tree the number of moves in cmove is correct
     * (otherwise it could have too large of a number from a previous search and segfault)
     */
    if (depth == 0) {
        if (pline != NULL) pline->cmove = 0;
        int evaluation = quiesence(alpha, beta, 8); // limit quiescence search to depth 8
        return evaluation;
    }

    /*
     * // BUG FIXED:
     * Make sure to set cmove to 0 because otherwise it may allocate the struct but not assign a value. This can cause a segfault on some systems
     */
    t_line line = {.cmove = 0 };
    int score = INT_MIN;
    int check = is_check();
    // increase the depth if we are in check, don't want to miss any tactics
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
     * If we find an exact value and the ply is 1, we need to send move to principle variation line
     */
    t_entry *entry = &transposition_table[zobrist % HASH_SIZE];
    if (entry->hash == zobrist && entry->depth>=depth) {
        switch (entry->flags) {
            case HASH_EXACT:
                cache_hit++;
                /*
                 * If the ply is 0 and we find a hit we need to send move to principle variation line so it has a move
                 * However, if the ply is greater than one we do not NEED to put move in PV line.
                 */
                if (ply == 0 && pline != NULL) {
                    // write the move to our PV line
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


        /*
         * This call will never happen at the root since alpha = -INF and bet = INF. There needs to be further search for pruning
         */
        if (alpha >= beta) {
            cache_hit++;
            return entry->score;
        }
    }
#endif

#ifdef NULL_MOVE
    /*
     * Lets do some null move pruning here to see if our opponent has a response to this move
     */
    if (!check && ply && depth >= NULL_R + 1) {
        // switch sides
        copy_board();
        side ^= 1;
        zobrist ^= side_key;
        if (enpassant != no_square) zobrist ^= enpassant_keys[enpassant];
        enpassant = no_square;
        /*
         * Here we make a call to negamax again, so our opponent makes two moves in a row.
         * We do not want this move to contribute to our PV, so we pass null in for that arg
         */
        int score = -negamax(-beta, -beta+1, depth - 1 - NULL_R, NULL);
        side ^= 1;
        restore_board();
        if (score >= beta) {
            return  beta;
        }
    }
#endif




   /*
    * Here we actually loop through all of our moves and make each one, checking if it is valid, and doing it
    * Maybe quiescence search can use this same negamax function with different flag?
    */

    t_moves moves_list;

    generate_moves(moves_list);

    sort_moves(moves_list);

    int valid_moves = 0;

    int found_principle_variation = false;

    int best = 0;

    // loop through each move
    for (int i = 0; i<moves_list.count; i++) {

        int move = moves_list.moves[i];

        copy_board();

        if (!make_move(move, all_moves)) {
            continue;
        }

#ifdef DEBUG
        assert(zobrist == generate_zobrist());
#endif

        valid_moves ++;
        ply ++;
        repitition.count ++;
        repitition.table[repitition.count] = zobrist;

        int test;
        if (found_principle_variation) {
            test = -negamax(-alpha -1, -alpha, depth - 1, NULL);
            if (test > alpha && test < beta)
                test = -negamax(-beta, -alpha, depth - 1, pline);
        } else
            test = -negamax(-beta, -alpha, depth - 1, &line);


        ply --;
        repitition.count--;

        if (test > score) {
            score = test;
            best = move;
        }

        restore_board();

        if (score >= beta) {
            // ADD mask ply check here to avoid segfaults
            if (!decode_capture(move) && ply < MAX_PLY) {
                history_moves[side][decode_source(move)][decode_destination(move)] += depth*depth;
                killer_moves[1][ply] = killer_moves[0][ply];
                killer_moves[0][ply] = move;
            }
            // we want to be able to score TT entries here, if not we may not hit as many
#ifdef TRANSPOSITION_TABLE
            entry->score = beta;
            entry->flags = HASH_LOWERBOUND;
            entry->depth = depth;
            entry->hash = zobrist;
            entry->move = best;
#endif
            return beta;
        }

        if (score > alpha) {
            alpha = score;

            if (UCI_PVSSearchMode) found_principle_variation = true;
            // write the move to our PV line
            if (pline != NULL) {
                pline->argmove[0] = move;
                memcpy(pline->argmove + 1, line.argmove, line.cmove * sizeof(int));
                pline->cmove = line.cmove + 1;
            }
        }
    }

    // check for checkmate or stalemate only if no beta cuttoff
    if (valid_moves == 0 ) {
        if (check)
            score = (-CHECKMATE + ply);
        else score = 0;
    }
#ifdef TRANSPOSITION_TABLE
    /*
     * We update our transposition table entry to reflect current score of node, etc.
     */
    entry->score = score;
    if (score <= _alpha) {
        entry->flags = HASH_UPPERBOUND;
    } else if (score >= beta) {
        entry->flags = HASH_LOWERBOUND;
    } else entry->flags = HASH_EXACT;
    entry->depth = depth;
    entry->hash = zobrist;
    entry->move = best;
#endif

    return score;
}

int Skunk::check_time() {
    if(std::chrono::steady_clock::now() - start_time > std::chrono::seconds(2))
        return 1;
    return 0;
}

// the top level call to get the best move
int Skunk::search(int maxDepth) {

    /*
     * Think about making this multithreaded ? Split up each move amongst a seperate thread. Constraints:
     * 1) TT synchronization is expensive, maybe each thread gets their own smaller TT?
     */

    memset(killer_moves, 0, sizeof(killer_moves));

    memset(history_moves, 0, sizeof(history_moves));

    // iterate through deepening as we go
    t_line pline;

    start_time = std::chrono::steady_clock::now();

    int depth;
    for (depth = 1; depth <= maxDepth; depth++) {
        quiesence_moves = 0;
        nodes=0;
        cache_miss = 0;
        cache_hit = 0;
        ply = 0;
        pline.cmove = 0;

        int score = negamax(-INT_MIN + 1, INT_MAX, depth, &pline);

        previous_pv_line = pline;

        if (UCI_AnalysisMode)
        {
            printf("info score cp %d depth %d nodes %d time 0 multipv %d pv ", score, depth, nodes, depth);
            for (int i=0; i<pline.cmove; i++) {
                // loop over the moves within a PV line
                // print PV move
                print_move(pline.argmove[i]);
                printf(" ");
            }
            printf("\n");
        }
    }

//    print_move(line.argmove[0]);
//    printf("Score: %d\n", best);

    // print out stats about the search
//    printf("Run Stats\n%-15s\t%-15s\t%-15s\t%-15s\t%-10s\n%-15d\t%-15d\t%-15d\t%-15d\t%-10d\n", "TT Hits", "TT Misses","Depth","Best Score","Nodes", cache_hit, cache_miss, maxDepth, best, nodes);

//    printf("Principle Variation Moves (Expected Line)\n");
//

    return pline.argmove[0];
}

int Skunk::evaluate() {


    // simply evaluates the piece scores

    U64 bitboard;
    int score = 0;
    for (int piece = P; piece <= k; piece++) {
        bitboard = bitboards[piece];
        while (bitboard) {
            int square = get_ls1b_index(bitboard);
            // add score for the piece existing

            // score positional piece scores
            switch (piece)
            {
                // evaluate white pieces
                case P: score += pawn_score[square]; break;
                case N: score += knight_score[square]; break;
                case B: score += bishop_score[square]; break;
                case R: score += rook_score[square]; break;
                case K: score += king_score[square]; break;

                    // evaluate black pieces
                case p: score -= pawn_score[mirror_score[square]]; break;
                case n: score -= knight_score[mirror_score[square]]; break;
                case b: score -= bishop_score[mirror_score[square]]; break;
                case r: score -= rook_score[mirror_score[square]]; break;
                case k: score -= king_score[mirror_score[square]]; break;
            }
            score += piece_scores[piece]*10;

            pop_bit(bitboard, square);
        }
    }

    return side==white?score:-score;
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

void Skunk::parse_option(char *command) {
    // Not implemented
}

void Skunk::parse_go(char *command) {
    int depth = 7;
    char *current_depth = NULL;
    if ((current_depth = strstr(command, "depth"))) {
        depth = atoi(current_depth + 6);
    }

    int move = search(depth);
    printf("bestmove ");
    print_move(move);
    printf("\n");
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

    if (current_char != NULL) {
        current_char += 6;
        while (*current_char) {
            int move = parse_move(current_char);
            if (move == 0) {
                break;
            }
            make_move(move, all_moves);
            while (*current_char && *current_char != ' ') current_char ++;
            current_char ++;
        }
    }
    print_board();
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
                if (move_string[4]=='r' && (promoted==r || promoted==R)) return move;
                if (move_string[4]=='b' && (promoted==b || promoted==B)) return move;
                if (move_string[4]=='q' && (promoted==q || promoted==Q)) return move;
                if (move_string[4]=='n' && (promoted==n || promoted==N)) return move;
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

    generate_moves(moves_list);

    for (int move_count = 0; move_count < moves_list.count; move_count++) {

        int  move = moves_list.moves[move_count];

        copy_board();

        if (!make_move(move, all_moves)) {
            continue;
        }

        // check if the hashes are the same
#ifdef DEBUG
        assert(zobrist == generate_zobrist());
#endif

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

int Skunk::is_repitition() {
    int seen = 0;
    for (int i=repitition.count - 1; i>=0; i--) {
        if (repitition.table[i] == zobrist) {
            return 1;
        }
    }
    return 0;
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

#ifdef DEBUG
                    assert(piece != K && piece != k);
#endif
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

        if (is_square_attacked((side == white) ? get_ls1b_index(bitboards[k]) : get_ls1b_index(bitboards[K]),
                               side)) {

            restore_board();
            return 0;
        }

        return 1;
    } else {
        // call make_move recursively
        if (decode_capture(move))
            return make_move(move, all_moves);

        // move is not a capture, return 0
        return 0;
    }
}


void Skunk::print_move(int move) {
    if (decode_promoted(move))
        printf("%s%s%c", square_to_coordinate[decode_source(move)],
               square_to_coordinate[decode_destination(move)],
               char_pieces[decode_promoted(move)]);
    else
        printf("%s%s", square_to_coordinate[decode_source(move)],
               square_to_coordinate[decode_destination(move)]);
}