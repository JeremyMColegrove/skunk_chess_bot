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
    return 0;
}

void uci_loop() {
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    char input[2000];
//    printf("id name Skunk\n");
//    printf("id author Jeremy Colegrove\n");
//    printf("uciok\n");

    Skunk skunk; // init the skunk

    int stop = 0;
    while (!stop) {

        memset(input, 0, sizeof(input));

        fflush(stdout);

        if (!fgets(input, 2000, stdin) || input[0]=='\n')
            continue;
        // split the command here
        t_commands commands = split_command(input, "\n\r");
        for (int i=0; i<commands.count; i++) {
            if  (strlen(commands.command[i]) > 3 && strncmp(commands.command[i], "quit", 4)==0) {
                stop = 1;
            }
            parse_command(commands.command[i], &skunk);
        }
    }
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
        printf("id name SkunkBot\n"
               "id author Jeremy Colegrove\n"
               "option name UCI_AnalyseMode type spin default true\n"
               "option name UCI_DefaultDepth type spin default 7\n"
               "option name UCI_DefaultDuration type spin default 2000\n"
               "uciok\n");
    } else if (strncmp(input, "setoption", 9)==0) {
        skunk->parse_option(input);
    } else if (strncmp(input, "debug", 5)==0) {
        skunk->parse_debug(input);
    }
}
