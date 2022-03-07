#include <iostream>
#include <stdint.h>
#include "board.h"

using namespace std;


void uci_loop();

int main(int argc, char **argv) {

    uci_loop();
    return 0;
}

void uci_loop() {
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    char input[2000];
    printf("id name Skunk\n");
    printf("id author Jeremy Colegrove\n");
    printf("uciok\n");

    Skunk bot;

    while (true) {
        memset(input, 0, sizeof(input));

        fflush(stdout);

        if (!fgets(input, 2000, stdin) || input[0]=='\n')
            continue;

        if (strncmp(input, "isready", 7)==0) {
            printf("readyok\n");
            continue;
        } else if (strncmp(input, "position", 8)==0) {
            bot.parse_position(input);
        } else if (strncmp(input, "ucinewgame", 10)==0) {
            bot.parse_position("position startpos");
        } else if (strncmp(input, "go", 2)==0) {
            bot.parse_go(input);
        } else if (strncmp(input, "uci", 3)==0) {
            printf("id name SkunkBot\n");
            printf("id author Jeremy Colegrove\n");
            printf("option name UCI_AnalysisMode type check default true\n");   // option to enable extra info printed like pvs lines
            printf("option name UCI_PVSSearchMode type check default true\n");  // option to enable PVS search

            printf("uciok\n");
        } else if (strncmp(input, "setoption", 9)==0) {
            bot.parse_option(input);
        } else if (strncmp(input, "quit", 4)==0) {
            break;
        }
    }
}


//void dual(Skunk bot1, Skunk bot2) {
//    char *start = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
//
//    bot1.parse_fen(start);
//    bot2.parse_fen(start);
//
//    /*
//     * Bot 1 makes a move first (white)
//     */
//    while (!check)
//    int move1 = bot1.search(7, all_moves);
//
//
//}