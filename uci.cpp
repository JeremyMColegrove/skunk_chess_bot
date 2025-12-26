//
// UCI protocol handling for the Skunk chess engine
//

#include "board.h"
#include <sstream>
#include <algorithm>
#include <iostream>
#include <cstring>

//==============================================================================
// Time Management & Communication
//==============================================================================

void Skunk::communicate() {
    // Check if time elapsed has passed
    if (move_time > 0) {
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_time).count();

        if (elapsed > move_time) {
            force_stop = 1;
            return;
        }
    }

    // Poll stdin for input
#ifdef _WIN32
    struct timeval timeout;
    timeout.tv_usec = 1;
    timeout.tv_sec = 0;
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    int is_ready = _kbhit();
#else
    struct pollfd fd;
    fd.fd = STDIN_FILENO;
    fd.events = POLLIN;
    fd.revents = 0;
    int is_ready = (poll(&fd, 1, 0) > 0 && ((fd.revents & POLLIN) != 0));
#endif

    if (!is_ready) return;

    char input[20];
    if (fgets(input, 20, stdin) && input[0] != '\n') {
        if (strncmp(input, "quit", 4) == 0) {
            force_stop = 1;
        } else if (strncmp(input, "stop", 4) == 0) {
            force_stop = 1;
        }
    }
    fflush(stdin);
}

//==============================================================================
// UCI Commands
//==============================================================================

void Skunk::parse_go(const std::string& cmd) {
    search_depth = 0;
    move_time = 0;
    btime = 0;
    wtime = 0;
    winc = 0;
    binc = 0;
    int movestogo = 0;
    int nodes_limit = 0;
    bool infinite = false;
    bool ponder = false;

    std::stringstream ss(cmd);
    std::string token;
    while (ss >> token) {
        if (token == "depth") {
            ss >> search_depth;
        } else if (token == "movetime") {
            ss >> move_time;
        } else if (token == "wtime") {
            ss >> wtime;
        } else if (token == "btime") {
            ss >> btime;
        } else if (token == "winc") {
            ss >> winc;
        } else if (token == "binc") {
            ss >> binc;
        } else if (token == "movestogo") {
            ss >> movestogo;
        } else if (token == "nodes") {
            ss >> nodes_limit;
        } else if (token == "infinite") {
            infinite = true;
        } else if (token == "ponder") {
            ponder = true;
        }
    }

    force_stop = 0;

    if (infinite || ponder) {
        move_time = 0;
        search(INT_MAX);
    } else if (move_time > 0) {
        search(INT_MAX);
    } else if (search_depth > 0) {
        search(search_depth);
    } else if (wtime > 0 || btime > 0) {
        int time_left = (side == white) ? wtime : btime;
        int inc = (side == white) ? winc : binc;

        if (movestogo > 0) {
            move_time = time_left / movestogo;
            move_time += inc * 3 / 4;
        } else {
            move_time = time_left / 30;
            move_time += inc * 3 / 4;
        }

        move_time = std::min(move_time, time_left / 2);

        if (time_left > 100) {
            move_time = std::max(move_time, 50);
        }

        move_time = std::min(move_time, DEFAULT_MOVETIME);

        search(INT_MAX);
    } else {
        search(10);
    }
}

void Skunk::parse_position(const std::string& command) {
    std::string cmd = command.substr(9);

    if (cmd.substr(0, 8) == "startpos") {
        parse_fen(fen_start);
    } else {
        size_t fen_pos = cmd.find("fen");
        if (fen_pos != std::string::npos) {
            parse_fen(cmd.substr(fen_pos + 4));
        } else {
            parse_fen(fen_start);
        }
    }

    size_t moves_pos = cmd.find("moves");
    repitition.count = 0;

    if (moves_pos != std::string::npos) {
        std::string moves_str = cmd.substr(moves_pos + 6);
        std::stringstream moves_ss(moves_str);
        std::string move_str;

        while (std::getline(moves_ss, move_str, ' ')) {
            int move = parse_move(move_str);
            if (move == 0) break;

            make_move(move, all_moves);
            repitition.table[repitition.count++] = zobrist;
        }
    }
}

int Skunk::parse_move(const std::string& move_string) {
    t_moves moves;
    generate_moves(moves);
    if (move_string.length() < 4) return 0;

    int source = (move_string[0] - 'a') + (8 - (move_string[1] - '0')) * 8;
    int target = (move_string[2] - 'a') + (8 - (move_string[3] - '0')) * 8;

    for (int i = 0; i < moves.count; i++) {
        int move = moves.moves[i];
        if (decode_source(move) == source && decode_destination(move) == target) {
            int promoted = decode_promoted(move);
            if (promoted) {
                if ((move_string[4] == 'r' || move_string[4] == 'R') && (promoted == r || promoted == R)) return move;
                if ((move_string[4] == 'b' || move_string[4] == 'B') && (promoted == b || promoted == B)) return move;
                if ((move_string[4] == 'q' || move_string[4] == 'Q') && (promoted == q || promoted == Q)) return move;
                if ((move_string[4] == 'n' || move_string[4] == 'N') && (promoted == n || promoted == N)) return move;
                continue;
            }
            return move;
        }
    }
    return 0;
}

//==============================================================================
// Perft Testing
//==============================================================================

perft perft_results;

void Skunk::parse_perft(const std::string& command) {
    int depth = 5;
    std::string depth_str = command.substr(6);
    depth = std::stoi(depth_str);
    perft_test(depth);
}

int Skunk::perft_test(int depth) {
    nodes = 0;

    t_moves moves;
    generate_moves(moves);

    auto start = std::chrono::steady_clock::now();

    for (int i = 0; i < moves.count; i++) {
        copy_board();
        make_move(moves.moves[i], all_moves);

        int cummulative_nodes = nodes;
        perft_test_helper(depth - 1);
        int old_nodes = nodes - cummulative_nodes;

        restore_board();

        printf("%s%s%c %d\n",
               square_to_coordinate[decode_source(moves.moves[i])],
               square_to_coordinate[decode_destination(moves.moves[i])],
               decode_promoted(moves.moves[i]) ? char_pieces[(decode_promoted(moves.moves[i]) % 6) + 6] : ' ',
               old_nodes);
    }

    auto end = std::chrono::steady_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << std::endl << nodes << std::endl;

    return nodes;
}

void Skunk::perft_test_helper(int depth) {
    if (depth == 0) {
        nodes++;
        return;
    }

    t_moves moves;
    generate_moves(moves);

    for (int move_count = 0; move_count < moves.count; move_count++) {
        copy_board();
        make_move(moves.moves[move_count], all_moves);
        perft_test_helper(depth - 1);
        restore_board();
    }
}

bool Skunk::perft_test_position(const std::string& fen, int expected_result, int depth) {
    parse_fen(fen);
    int result = perft_test(depth);
    if (result == expected_result) {
        std::cout << "\u2713\tPassed (" << result << ")" << std::endl;
        return true;
    }

    std::cout << "\tFailed (result was " << result << ", expected was " << expected_result << ")" << std::endl;
    exit(0);
    return false;
}

//==============================================================================
// Coordinate Conversion
//==============================================================================

int Skunk::coordinate_to_square(char* coordinate) {
    for (int i = 0; i < 64; i++) {
        if (strcmp(square_to_coordinate[i], coordinate) == 0) {
            return i;
        }
    }
    return -1;
}
