#include <iostream>
#include <stdint.h>
#include "board.h"

using namespace std;

int main(int argc, char **argv) {

    //Get the string passed in
    char *fen = argv[argc-1];
    Board board;
    board.load_fen_string(fen);


    return 0;
}

