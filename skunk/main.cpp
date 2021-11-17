#include <iostream>
#include <stdint.h>
#include "board.h"

using namespace std;

int main(int argc, char **argv) {

    //Get the string passed in
    char *fen = argv[argc-1];
//    char *difficult =  "r1b1k1nr/pP1p1pNp/n2b1P2/Pp1N12P/3B2PR/3P1Q2/P1P1K3/q5b1 w KQ-q e4 0 1";
//    char *castle_kingside = "rnbqk2r/pppppppp/5n2/8/4PP1N/NBQB1b2/PPPP2PP/R3K2R w KQkq - 0 1";
    Skunk bot;
    bot.parse_fen(fen);
    bot.generate_moves();

    return 0;
}
