#include <iostream>
#include <stdint.h>
#include <vector>
#include "board.h"
#include <sstream>

using namespace std;

/*
 * struct for handling the command parsing
 */
typedef struct {
    int count;
    char *command[12]; // handle at most 12 simulataneous commands
} t_commands;

void uci_loop();

void parse_command(const std::string& cmd, Skunk* skunk);

std::vector<std::string> split_command(const std::string& input);

int main(int argc, char **argv) {
    uci_loop();
    return 0;
}


void uci_loop() {
    Skunk *skunk = new Skunk();
    std::string input;

    while (true) {
        std::getline(std::cin, input);

        if (input.empty()) {
            continue;
        }

        std::vector<std::string> commands = split_command(input);

        bool shouldStop = false;
        for (const std::string& command : commands) {
            if (command.substr(0, 4) == "quit") {
                shouldStop = true;
                break;
            }
            parse_command(command, skunk);
        }

        if (shouldStop) {
            break;
        }
    }

    delete skunk;
}

std::vector<std::string> split_command(const std::string& input) {
    std::stringstream ss(input);
    std::string item;
    std::vector<std::string> tokens;
    while (std::getline(ss, item, '\n')) {
        tokens.push_back(item);
    }
    return tokens;
}

void parse_command(const std::string& cmd, Skunk* skunk) {
    if (cmd == "uci") {
        // Respond to the "uci" command by printing the engine name and options
        std::cout << "id name Skunk" << std::endl;
        std::cout << "id author Jeremy Colegrove" << std::endl;
        std::cout << "uciok" << std::endl;
    } else if (cmd == "isready") {
        // Respond to the "isready" command by indicating that the engine is ready
        std::cout << "readyok" << std::endl;
    } else if (cmd.substr(0, 9) == "setoption") {
        // Parse and process any options sent with the "setoption" command
        // ...
    } else if (cmd.substr(0, 8) == "position") {
        // Parse and set the position on the board sent with the "position" command
        skunk->parse_position(cmd);
    } else if (cmd.substr(0, 2) == "go") {
        // Parse and set the search parameters sent with the "go" command
        // ...
        // Start the search and print the best move when it is finished
        skunk->parse_go(cmd);
    } else if (cmd.substr(0, 10) == "ucinewgame") {
        skunk->parse_position("position startpos");
    } else if (cmd == "stop") {
        // Stop the search in response to the "stop" command
        // ...
    } else if (cmd == "quit") {
        // Exit the program in response to the "quit" command
        exit(0);
    } else if (cmd.substr(0, 5) == "perft") {
        skunk->parse_perft(cmd);
    } else if (cmd == "board") {
        skunk->print_board();
    } else if (cmd == "score") {
        std::cout << skunk->evaluate() << std::endl;
    } else if (cmd == "sort") {
        skunk->show_sort();
    }
}

