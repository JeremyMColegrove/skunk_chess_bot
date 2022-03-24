//
// Created by Jeremy Colegrove on 3/23/22.
//

#ifndef SKUNK_PIECE_TABLES_H
#define SKUNK_PIECE_TABLES_H


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



#endif //SKUNK_PIECE_TABLES_H
