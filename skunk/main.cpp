#include <iostream>
#include <stdint.h>
#include "board.h"

using namespace std;

int main(int argc, char **argv) {

    //Get the string passed in
    char *fen = argv[argc-1];

    U64 blocks = 0ULL;

    set_bit(blocks, b2);
    set_bit(blocks, g7);
    set_bit(blocks, d6);
    set_bit(blocks, e3);
    set_bit(blocks, c6);
    Skunk bot;
    bot.parse_fen("r1b1k1nr/p2p1pNp/n2B4/1p1NP2P/6P1/3P1Q2/P1P1K3/q5b1 w KQ-q -");
    bot.print_board();
    bot.print_attacks(white);

    return 0;
}
