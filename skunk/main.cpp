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

    Skunk bot;

    bot.print_board(blocks);
    bot.print_board(bot.get_rook_attacks(e5, blocks));

    return 0;
}
