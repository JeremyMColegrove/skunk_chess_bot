#include <iostream>
#include <stdint.h>
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
    uci_loop();
//    Skunk *skunk = new Skunk();
//    skunk->parse_position("position fen r1bqkb2/pppppppp/2n2n2/4r3/5P2/BNQP1NB1/PPPP1P1P/R3K2R w KQkq - 0 1");
//    t_moves moves;
//    skunk->generate_moves(moves);
//    skunk->print_moves(moves);
//    int move = encode_move(f7, f8, P, Q, 0, 0);
//    skunk->make_move(move, all_moves);
//    skunk->print_board();
//    skunk->print_bitboard(skunk->occupancies[white]);


//    U64 board = 0ULL;
//    Skunk skunk;
//    set_bit(board, b8);
//    set_bit(board, c8);
//    set_bit(board, d8);
//    skunk.print_bitboard(board);
    return 0;
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
        printf("id name Skunk\n"
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
