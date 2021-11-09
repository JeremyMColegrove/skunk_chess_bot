//
// Created by Jeremy Colegrove on 11/8/21.
//

#include "board.h"
#include <iostream>
//Default constructor and destructor
Board::~Board() {}

Board::Board() {}

//
////Generates all the corresponding bit boards for each piece, for each color
void Board::load_fen_string(char *fen)
{
    //Loop through our FEN string and append 1's to all of our bitboards
    for (int i=0; i<std::strlen(fen); i++)
    {
        //If it is a slash just ignore it
        if (fen[i] == '/') continue;
        if (fen[i] == ' ') break;
        WK      <<= 1;
        BK      <<= 1;
        WP      <<= 1;
        BP      <<= 1;
        WQ      <<= 1;
        BQ      <<= 1;
        WN      <<= 1;
        BN      <<= 1;
        WB      <<= 1;
        BB      <<= 1;
        WR      <<= 1;
        BR      <<= 1;

        switch (fen[i])
        {
            case 'p':
                BP += 1;
                break;
            case 'P':
                WP += 1;
                break;
            case 'K':
                WK += 1;
                break;
            case 'k':
                BK += 1;
                break;
            case 'Q':
                WQ += 1;
                break;
            case 'q':
                BQ += 1;
                break;
            case 'B':
                WB += 1;
                break;
            case 'b':
                BB += 1;
                break;
            case 'N':
                WN += 1;
                break;
            case 'n':
                BN += 1;
                break;
            case 'R':
                WR += 1;
                break;
            case 'r':
                BR += 1;
                break;
            default:
                int space = atoi(&fen[i]) - 1;
                WK      <<= space;
                BK      <<= space;
                WP      <<= space;
                BP      <<= space;
                WQ      <<= space;
                BQ      <<= space;
                WN      <<= space;
                BN      <<= space;
                WB      <<= space;
                BB      <<= space;
                WR      <<= space;
                BR      <<= space;
                break;
        }
    }

    int64_t pawn_attacks = get_pawn_movement(1);
    print_board(pawn_attacks);
}


//Prints out the given board
void Board::print_board(int64_t board) {
    int64_t mask =  -9223372036854775808;
    std::cout << "\nPrinting board: " << board << std::endl;
    for (int i=0; i<64; i++)
    {
        if (i % 8 == 0 && i>0) std::cout << std::endl;
        if (board & mask) std::cout << "1  ";
        else std::cout << "0  ";
        board <<= 1;
    }
}

int64_t Board::get_pawn_movement(int color) {
    int64_t enemy_pieces, our_pieces, moving_pieces;
    if (color > 0)
    {
        //Includes all enemy pieces
        enemy_pieces = BQ | BK | BP | BB | BN | BR;
        //Excludes the pawns
        our_pieces = WQ | WK | WB | WN | WR | WP;
        //
        moving_pieces = WP;
    } else {
        //Includes all enemy pieces
        enemy_pieces = WQ | WK | WP | WB | WN | WR;
        //Excludes the pawns from our pieces
        our_pieces = BQ | BK | BB | BN | BR | BP;
        moving_pieces = BP;
    }
    int64_t not_our_pieces = ~our_pieces;

    //Now actually shift the board to get valid positions
    int64_t result=0;

    //Get the right attacks
    result |= ((((moving_pieces << 7)&~file_a)&not_our_pieces)&enemy_pieces);

    //Get the left attacks
    result |= ((((moving_pieces << 9)&~file_h)&not_our_pieces)&enemy_pieces);

    //Get forward movement
    result |= ((moving_pieces << 8)&not_our_pieces)&~enemy_pieces;

    //The second position at the start
    result |= (((result << 8)&row_4)&~enemy_pieces)&~(our_pieces<<8)&not_our_pieces;

    return result;
}
