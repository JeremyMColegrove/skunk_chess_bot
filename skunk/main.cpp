#include <iostream>
#include <stdint.h>
#include "board.h"

using namespace std;


void dual(Skunk bot1, Skunk bot2);

int main(int argc, char **argv) {

    //Get the string passed in
//    char *start = argv[1];
    int depth = atoi(argv[2]);

    char *difficult =  "r1b1k1nr/pP1p1pNp/n2b1P2/Pp1N12P/3B2PR/3P1Q2/P1P1K3/q5b1 w KQ-q e4 0 1";
//    char *castle_kingside = "rnbqk2r/pppppppp/5n2/8/4PP1N/NBQB1b2/PPPP2PP/R3K2R w KQkq - 0 1";
    char *checkmate = "r2q1k1r/ppp1bB1p/2np4/6N1/3PP1bP/8/PPP5/RNB2RK1 b - - 0 1";
    char *tricky_position = "r3k2r/p1ppqpb1/bn2pnp1/1p1PN3/22P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 ";

    char *start = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    char *drop_piece = "r1bqkb1r/ppp1pppp/5n2/3p4/1n1P4/2PBPN2/PP3PPP/RNBQ1K1R b KQkq - 0 1";
    char *checkmate_1 = "2k4r/ppp3bp/5n2/Q7/1B3N2/5PK1/2q5/6r1 w - - 2 31";
    char *checkmate_2 = "1R6/2R5/7K/8/k7/8/8/8 w - - 0 1";
    char *checkmate_2_flipped = "1r6/2r4k/8/8/K7/8/8/8 b - - 0 1";
    char *horizon_test = "rnbqkbnr/ppp1pppp/8/8/2PPp3/8/PP3PPP/RNBQKBNR b KQkq 0 1";
    char *jocelyns_game = "r1bq1rk1/ppp1bpp1/2n1p2p/n7/2PPN1P1/PP3N2/1B3PBP/R2Q1RK1 w ---- 0 1";
    char *enpassant_capture = "r4bk1/pPpQ3p/2p2p2/3p4/2PPp3/BP6/5PPP/2KRR3 w ---- 0 1";

    char *g = "2r3k1/1p1n2p1/pBn1p2p/8/P2P3P/3R1NP1/1P3P2/5K2 w ---- 0 1";
    char *illegal_move = "r1bqkb1r/ppp2ppp/2n1pn2/8/3PN3/5N2/PPP2PPP/R1BQK2R w ---- 0 1";
    Skunk bot;
    bot.parse_fen(start);


//    printf("With transposition:\n");
//    bot.transposition = 1;
//    bot.search(6, true);
//    printf("Without transposition:\n");
//    bot.transposition = 0;
//    bot.search(6, true);
//    bot.dual(6, 1000);
//      bot.perft_test(6);
//    bot.perft_test(4);
//    int move = bot.search(7);
//    bot.make_move(move, all_moves);
//    bot.print_board();
    bot.play(7);
//    bot.search(7);
    // Hash should be: df8aa96e8bbf11ad


//    int move = bot.search(depth);
//    bot.print_move(move);

    return 0;
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