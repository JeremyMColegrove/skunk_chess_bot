#include <iostream>
#include <stdint.h>
#include <vector>
#include "board.h"

using namespace std;

/*
 * struct for handling the command parsing
 */
typedef struct {
    int count;
    char *command[12]; // handle at most 12 simulataneous commands
} t_commands;

void uci_loop();

void parse_command(char * command, Skunk *skunk);

t_commands split_command(char *command, char * seperator);

int main(int argc, char **argv) {
//    automate_tests();
//    uci_loop();
    Skunk *skunk = new Skunk();
    skunk->parse_fen("2kr1b1r/ppp2ppp/4p3/8/1n6/2Q2P2/qR2BPPP/2B2RK1 b - - 1 14");
    skunk->search(8);
//    skunk->print_board();
//    printf("%d\n", skunk->evaluate());
//    int eval = skunk->evaluate();
//    skunk->print_board();
//    printf("Eval is %d\n", eval);
//    t_moves moves_list;
//    skunk->generate_moves(moves_list);
//    skunk->sort_moves(moves_list);
//    skunk->print_moves(moves_list);
//    skunk->enpassant = g3;
//    t_moves moves;
//    skunk->generate_moves(moves);
//    skunk->print_moves(moves);


//    U64 board = 0ULL;
//    Skunk *skunk = new Skunk();
//    for (int i=0; i<64; i++) {
//        set_bit(board, i);
//    }
//    skunk->print_bitboard(board);
    return 0;
}


void automate_tests() {
    std::vector<char *> tests;
    tests.push_back((char *)"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    Skunk *skunk = new Skunk();

    for (int test=0; test<tests.size(); test++) {
        // this will test the fen position for x couple of seconds
        skunk->parse_fen(tests.at(test));
        skunk->parse_go((char *)"go movetime 1000");
    }
}


void uci_loop() {
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    char input[200];

    Skunk *skunk = new Skunk();

    int stop = 0;
    while (!stop) {

        memset(input, 0, sizeof(input));

        fflush(stdout);

        if (!fgets(input, 2000, stdin) || input[0]=='\n')
            continue;
        // split the command here
        t_commands commands = split_command(input, (char *)"\n\r");
        for (int i=0; i<commands.count; i++) {
            if  (strlen(commands.command[i]) > 3 && strncmp(commands.command[i], "quit", 4)==0) {
                stop = 1;
            }
            parse_command(commands.command[i], skunk);
        }
    }
    delete skunk;
}

t_commands split_command(char *command, char *seperator) {
    t_commands result = {.count = 0};

    result.command[result.count] = strtok(command, seperator);

    while (result.command[result.count] != NULL)
    {
        result.count ++ ;
        result.command[result.count] = strtok(NULL, seperator);
    }

    return result;
}

void parse_command(char *input, Skunk *skunk) {

    if (strncmp(input, "isready", 7)==0) {
        printf("readyok\n");
    } else if (strncmp(input, "position", 8)==0) {
        skunk->parse_position(input);
    } else if (strncmp(input, "ucinewgame", 10)==0) {
        skunk->parse_position("position startpos");
    } else if (strncmp(input, "go", 2)==0) {
        skunk->parse_go(input);
    } else if (strncmp(input, "uci", 3)==0) {
        printf("id name Complex3\n"
               "id author Jeremy Colegrove\n"
               "option name UCI_AnalyseMode type spin default true\n"
               "option name UCI_DefaultDepth type spin default 7\n"
               "option name UCI_DefaultDuration type spin default 2000\n"
               "uciok\n");
    } else if (strncmp(input, "setoption", 9)==0) {
        skunk->parse_option(input);
    } else if (strncmp(input, "debug", 5)==0) {
        skunk->parse_debug(input);
    } else if (strncmp(input, "perft", 5)==0) {
        skunk->parse_perft(input);
    }
}
