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

void test_positions();

std::vector<std::string> split_command(const std::string& input);

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "test_suite")==0) {
        test_positions();
    } else if (argc > 2) {
        
        Skunk *skunk = new Skunk();

        int depth = atoi(argv[1]);
        // set the position
        skunk->parse_fen(argv[2]);

        // std::cout << depth << ":" << argv[2] << ":" << argv[3] << std::endl;
        if (argc > 3) {
            // parse each of the moves as well
            std::string moves_str = argv[3];
            std::stringstream moves_ss(moves_str);
            std::string move_str;
            while (std::getline(moves_ss, move_str, ' ')) {
                int move = skunk->parse_move(move_str);

                if (move == 0) break;

                skunk->make_move(move, all_moves);
            }
        }

        skunk->perft_test(depth);

    } else {
        uci_loop();
    }
    return 0;

}

void test_positions() {
    Skunk *skunk = new Skunk();

    // skunk->parse_fen("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");
    // for (int dir=0; dir<8; dir++) {
    //     for (int sq=0; sq<64; sq++) {
    //         int nearest_sq = skunk->nearest_square[dir][sq];
    //         std::cout << nearest_sq << std::endl;
    //         if (nearest_sq < 0 || nearest_sq > 63) {
    //             exit(0);
    //         }
    //     }
    // }
    
    //Wikipedia FEN
    // skunk->perft_test_position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 119060324, 6);
    // skunk->perft_test_position("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ", 4085603, 4);
    // skunk->perft_test_position("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w -- ", 11030083, 6);
    // skunk->perft_test_position("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 15833292, 5);
    // skunk->perft_test_position("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 89941194, 5);
    // skunk->perft_test_position("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 164075551, 5);

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
        t_moves moves;
        skunk->generate_moves(moves);
        skunk->print_board();
        skunk->sort_moves(moves.moves, moves.count);
        skunk->print_moves(moves);
    } else if (cmd == "score") {
        std::cout << skunk->evaluate() << std::endl;
    } else if (cmd == "sort") {
        skunk->show_sort();
    }
}

