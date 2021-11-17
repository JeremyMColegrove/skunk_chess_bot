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
    int fen_length = strlen(fen);
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

    //get the number of half moves
    half_moves = fen[index++] - '0';

    // bypass all spaces
    while (index < fen_length && fen[++index]==' ');
    // check if end of string
    if (index == fen_length) return fill_occupancies();

    full_moves = fen[index++] - '0';

//     check if end of string
    if (index == fen_length) return fill_occupancies();

    full_moves = full_moves*10 + fen[index] - '0';

    fill_occupancies();
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
    construct_knight_tables();
    construct_king_tables();
    construct_bishop_tables();
    construct_rook_tables();
    construct_slider_attacks();


    //reset our turn
    side = white;
    //reset our castle rights
    castle = 0;
}

Skunk::~Skunk() {

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

void Skunk::construct_knight_tables() {
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

            std::cout << ((piece==-1) ? '.' : ascii_pieces[piece]) << "  ";
        }
        // print a newline
        std::cout << std::endl;
    }
    // print files
    std::cout << "\n    " << "a  b  c  d  e  f  g  h" << std::endl;
    // print whos turn it is
    std::cout << "\nSide: " << (side == white ? "white" : "black") << std::endl;
    // print en passant square
    std::cout << "\nEn Passant: " << (enpassant != no_square ?  square_to_coordinate[enpassant] : "none") << std::endl;
    //print castling rights
    std::cout << "\nCastling: " << ((castle&wk) ? 'K':'-') << ((castle&wq)?'Q':'-') << ((castle&bk)?'k':'-') << ((castle&bq)?'q':'-') << std::endl;

    std::cout << "\nHalf moves: " << half_moves << std::endl;

    std::cout << "\nFull moves: " << full_moves << std::endl;



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
//

void Skunk::generate_moves() {
    U64 bitboard_copy, pawn_single_moves, pawn_double_moves, attacks;
    int source, destination;
    for ( int piece=P; piece<=k; piece++)
    {
        bitboard_copy = bitboards[piece];
        if (side == white)
        {
            if (piece == P)
            {
                // generate single pawn_single_moves
                pawn_single_moves = (bitboard_copy >> 8) & ~occupancies[both];
                // generate double pawn moves
                pawn_double_moves = (pawn_single_moves >> 8) & ~occupancies[both] & row_4;
                // get each move
                while (pawn_single_moves)
                {
                    destination = get_ls1b_index(pawn_single_moves);
                    source = destination + 8;

                    printf("Pawn can move from %s to %s\n", square_to_coordinate[source], square_to_coordinate[destination]);

                    pop_bit(pawn_single_moves, destination);
                }

                // get double pawn moves
                while (pawn_double_moves)
                {
                    destination = get_ls1b_index(pawn_double_moves);
                    source = destination + 16;

                    printf("Pawn can move DOUBLE from %s to %s\n", square_to_coordinate[source], square_to_coordinate[destination]);

                    pop_bit(pawn_double_moves, destination);
                }

                // generate pawn attacks
                // get each pawn
                while (bitboard_copy)
                {
                    // get the square
                    source = get_ls1b_index(bitboard_copy);

                    // get the attacks for that piece
                    attacks = pawn_masks[side][source] & occupancies[!side];
                    // get each attack
                    while (attacks)
                    {
                        destination = get_ls1b_index(attacks);
                        printf("Pawn can ATTACK from square %s to %s\n", square_to_coordinate[source], square_to_coordinate[destination]);
                        pop_bit(attacks, destination);
                    }

                    pop_bit(bitboard_copy, source);
                }

            }



        } else
        {
            if (piece == p)
            {
                // generate single pawn_single_moves
                pawn_single_moves = (bitboard_copy << 8) & ~occupancies[both];
                // generate double pawn moves
                pawn_double_moves = (pawn_single_moves << 8) & ~occupancies[both] & row_5;
                // get each move
                while (pawn_single_moves)
                {
                    destination = get_ls1b_index(pawn_single_moves);
                    source = destination - 8;

                    printf("Pawn can move from %s to %s\n", square_to_coordinate[source], square_to_coordinate[destination]);

                    pop_bit(pawn_single_moves, destination);
                }

                // get double pawn moves
                while (pawn_double_moves)
                {
                    destination = get_ls1b_index(pawn_double_moves);
                    source = destination - 16;

                    printf("Pawn can move DOUBLE from %s to %s\n", square_to_coordinate[source], square_to_coordinate[destination]);

                    pop_bit(pawn_double_moves, destination);
                }

                // generate pawn attacks
                // get each pawn
                while (bitboard_copy)
                {
                    // get the square
                    source = get_ls1b_index(bitboard_copy);

                    // get the attacks for that piece
                    attacks = pawn_masks[side][source] & occupancies[!side];
                    // get each attack
                    while (attacks)
                    {
                        destination = get_ls1b_index(attacks);
                        printf("Pawn can ATTACK from square %s to %s\n", square_to_coordinate[source], square_to_coordinate[destination]);
                        pop_bit(attacks, destination);
                    }

                    pop_bit(bitboard_copy, source);
                }

            }
        }

        // get knight moves
        if ((side == white) ? piece == N : piece == n)
        {
            while (bitboard_copy)
            {
                source = get_ls1b_index(bitboard_copy);

                attacks = knight_masks[source] & ((side == white)?~occupancies[white]:~occupancies[black]);
                print_bitboard(attacks);
                while (attacks)
                {
                    destination = get_ls1b_index(attacks);

                    // a valid knight move TODO encode move
                    printf("Knight can move from %s to %s\n", square_to_coordinate[source], square_to_coordinate[destination]);

                    pop_bit(attacks, destination);
                }
                pop_bit(bitboard_copy, source);
            }
        }
        // rook moves
        if (side == white? piece == R : piece == r)
        {
            while (bitboard_copy)
            {
                source = get_ls1b_index(bitboard_copy);

                attacks = get_rook_attacks(source, occupancies[both]) & (side==white?~occupancies[white]:~occupancies[black]);

                while (attacks)
                {
                    destination = get_ls1b_index(attacks);

                    //TODO valid rook move
                    printf("Rook can move from %s to %s\n", square_to_coordinate[source], square_to_coordinate[destination]);
                    pop_bit(attacks, destination);
                }
                pop_bit(bitboard_copy, source);
            }
        }
        // bishop moves
        if (side == white? piece == B : piece == b)
        {
            while (bitboard_copy)
            {
                source = get_ls1b_index(bitboard_copy);

                attacks = get_bishop_attacks(source, occupancies[both]) & (side==white?~occupancies[white]:~occupancies[black]);

                while (attacks)
                {
                    destination = get_ls1b_index(attacks);

                    //TODO valid rook move
                    printf("Bishop can move from %s to %s\n", square_to_coordinate[source], square_to_coordinate[destination]);
                    pop_bit(attacks, destination);
                }
                pop_bit(bitboard_copy, source);
            }
        }
        // queen moves
        if (side == white? piece == Q : piece == q)
        {
            while (bitboard_copy)
            {
                source = get_ls1b_index(bitboard_copy);

                attacks = (get_bishop_attacks(source, occupancies[both]) \
                | get_rook_attacks(source, occupancies[both])) \
                & (side==white?~occupancies[white]:~occupancies[black]);

                while (attacks)
                {
                    destination = get_ls1b_index(attacks);

                    //TODO valid rook move
                    printf("Queen can move from %s to %s\n", square_to_coordinate[source], square_to_coordinate[destination]);
                    pop_bit(attacks, destination);
                }
                pop_bit(bitboard_copy, source);
            }
        }
        // king moves
        if (side == white? piece == K : piece == k)
        {
            while (bitboard_copy)
            {
                source = get_ls1b_index(bitboard_copy);

                attacks = king_masks[source] & (side==white?~occupancies[white]:~occupancies[black]);

                while (attacks)
                {
                    destination = get_ls1b_index(attacks);

                    //TODO valid rook move
                    printf("King can move from %s to %s\n", square_to_coordinate[source], square_to_coordinate[destination]);
                    pop_bit(attacks, destination);
                }
                pop_bit(bitboard_copy, source);
            }
        }
    }


    // check castling
    if (side == white)
    {
        // white can castle king side
        if (castle && wk)
        {
            // check if pieces are in the way or the king is attacks or passing through attacks
            if (!get_bit(occupancies[both], f1) && !get_bit(occupancies[both], g1))
            {
                // check if the squares are under attack
                if (!is_square_attacked(f1, black))
                {
                    printf("White can castle king-side!\n");
                }
            }
        }
        if (castle && wq)
        {
            // check if pieces are in the way or the king is attacks or passing through attacks
            if (!get_bit(occupancies[both], d1) && !get_bit(occupancies[both], c1) && !get_bit(occupancies[both], b1))
            {
                // check if the squares are under attack
                if (!is_square_attacked(d1, black) && !is_square_attacked(c1, black))
                {
                    printf("White can castle queen-side!\n");
                }
            }
        }
    }else
    {
        // black can castle king side
        if (castle && bk)
        {
            // check if pieces are in the way or the king is attacks or passing through attacks
            if (!get_bit(occupancies[both], f8) && !get_bit(occupancies[both], g8))
            {
                // check if the squares are under attack
                if (!is_square_attacked(f8, white))
                {
                    printf("Black can castle king-side!\n");
                }
            }
        }
        if (castle && wq)
        {
            // check if pieces are in the way or the king is attacks or passing through attacks
            if (!get_bit(occupancies[both], d8) && !get_bit(occupancies[both], c8) && !get_bit(occupancies[both], b8))
            {
                // check if the squares are under attack
                if (!is_square_attacked(d8, white) && !is_square_attacked(c8, white))
                {
                    printf("Black can castle queen-side!\n");
                }
            }
        }
    }


    // print the board
    print_board();
}