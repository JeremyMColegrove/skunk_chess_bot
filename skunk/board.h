//
// Created by Jeremy Colegrove on 11/8/21.
//

#ifndef BITBOT_BOARD_H
#define BITBOT_BOARD_H
#include <stdint.h>

class Board {
public:
    Board();
    ~Board();

    void load_fen_string(char *fen);
    void print_board(int64_t board);
private:
    //Functions for getting each pieces valid moves (takes whose turn it should calculate for)
    //WHITE = 1, BLACK = -1
    int64_t get_pawn_movement(int color);

    //All of our bitboards
    int64_t WK, BK, WP, BP, WQ, BQ, WN, BN, WB, BB, WR, BR;

    //USEFUL BOARD MASKS
    //ones all along the a file (left side)
    int64_t file_a = -9187201950435737472;
    //ones all along the h file (right side)
    int64_t file_h = 72340172838076673;

    //ones all along these rows
    int64_t row_1 = 255;
    int64_t row_8 = -72057594037927936;
    int64_t row_4 = 4278190080;
    int64_t row_5 = 1095216660480;

};
#endif //BITBOT_BOARD_H
