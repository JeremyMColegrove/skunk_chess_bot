#include <iostream>
#include <stdint.h>
#include "board.h"

using namespace std;

int main(int argc, char **argv) {

    //Get the string passed in
//    char *fen = argv[1];
    int depth = atoi(argv[2]);

    char *difficult =  "r1b1k1nr/pP1p1pNp/n2b1P2/Pp1N12P/3B2PR/3P1Q2/P1P1K3/q5b1 w KQ-q e4 0 1";
//    char *castle_kingside = "rnbqk2r/pppppppp/5n2/8/4PP1N/NBQB1b2/PPPP2PP/R3K2R w KQkq - 0 1";
    char *checkmate = "r2q1k1r/ppp1bB1p/2np4/6N1/3PP1bP/8/PPP5/RNB2RK1 b - - 0 1";
    char *tricky_position = "r3k2r/p1ppqpb1/bn2pnp1/1p1PN3/22P3/2N2Q1p/PPPBBPPP/R3K2R b KQkq - 0 1 ";

    char *fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    Skunk bot;
    bot.parse_fen(fen);
    bot.play(6);

//    int move = bot.search(depth);
//    bot.print_move(move);

    return 0;
}
