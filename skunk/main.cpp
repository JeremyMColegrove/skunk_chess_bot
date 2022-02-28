#include <iostream>
#include <stdint.h>
#include "board.h"

using namespace std;

int main(int argc, char **argv) {

    //Get the string passed in
//    char *fen = argv[1];
//    int depth = atoi(argv[2]);

    char *fen = "4Q3/p7/3kpp2/8/3P4/4qP2/b5PP/6K1 w - - 0 37";
//    char *difficult =  "r1b1k1nr/pP1p1pNp/n2b1P2/Pp1N12P/3B2PR/3P1Q2/P1P1K3/q5b1 w KQ-q e4 0 1";
//    char *castle_kingside = "rnbqk2r/pppppppp/5n2/8/4PP1N/NBQB1b2/PPPP2PP/R3K2R w KQkq - 0 1";


    Skunk bot;

    bot.parse_fen(fen);
    // bot.perft_test(6);
    bot.play();
//    int move = bot.search(depth);

    // print to stdout so we can display on screen
//    printf("%s\n", move);

    return 0;
}
