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
    // uci_loop();

    if (argc > 1 && strcmp(argv[1], "test_suite")==0) {
        test_positions();
    } else if (argc > 2) {
        
        Skunk *skunk = new Skunk();

        int depth = atoi(argv[1]);
        // set the position
        skunk->parse_fen(argv[2]);

        std::cout << depth << ":" << argv[2] << ":" << argv[3] << std::endl;
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
    // skunk->perft_test_position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 119060324, 6);
    // skunk->perft_test_position("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ", 4085603, 4);
    // skunk->perft_test_position("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w -- ", 11030083, 6);
    // skunk->perft_test_position("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 15833292, 5);
    // skunk->perft_test_position("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 89941194, 5);
    // skunk->perft_test_position("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 164075551, 5);


    // test even more chess960 positions
    // skunk->perft_test_position("3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1", 1134888, 6);
    // skunk->perft_test_position("8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1", 1015133, 6);
    // skunk->perft_test_position("8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1", 1440467, 6);

    // skunk->perft_test_position("5k2/8/8/8/8/8/8/4K2R w K - 0 1", 661072, 6);
    // skunk->perft_test_position("3k4/8/8/8/8/8/8/R3K3 w Q - 0 1", 803711, 6);


    // skunk->perft_test_position("r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1", 1274206, 4);
    // skunk->perft_test_position("r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1", 1720476, 4);
    // skunk->perft_test_position("2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1", 3821001, 6);

    // skunk->perft_test_position("8/8/1P2K3/8/2n5/1q6/8/5k2 b - - 0 1", 1004658, 5);
    // skunk->perft_test_position("4k3/1P6/8/8/8/8/K7/8 w - - 0 1", 217342, 6);
    // skunk->perft_test_position("8/P1k5/K7/8/8/8/8/8 w - - 0 1", 92683, 6);
    uci_loop();
    // skunk->perft_test_position("bqnb1rkr/pp3ppp/3ppn2/2p5/5P2/P2P4/NPP1P1PP/BQ1BNRKR w KQkq - 2 9", 12189, 3);
// skunk->perft_test_position("2nnrbkr/p1qppppp/8/1ppb4/6PP/3PP3/PPP2P2/BQNNRBKR w HEhe - 1 9", 16253601, 5);
// skunk->perft_test_position("b1q1rrkb/pppppppp/3nn3/8/P7/1PPP4/4PPPP/BQNNRKRB w GE - 1 9", 6417013, 5);
// skunk->perft_test_position("qbbnnrkr/2pp2pp/p7/1p2pp2/8/P3PP2/1PPP1KPP/QBBNNR1R w hf - 0 9", 9183776, 5);
// skunk->perft_test_position("1nbbnrkr/p1p1ppp1/3p4/1p3P1p/3Pq2P/8/PPP1P1P1/QNBBNRKR w HFhf - 0 9", 34030312, 5);
// skunk->perft_test_position("qnbnr1kr/ppp1b1pp/4p3/3p1p2/8/2NPP3/PPP1BPPP/QNB1R1KR w HEhe - 1 9", 24851983, 5);
// skunk->perft_test_position("q1bnrkr1/ppppp2p/2n2p2/4b1p1/2NP4/8/PPP1PPPP/QNB1RRKB w ge - 1 9", 21093346, 5);
// skunk->perft_test_position("qbn1brkr/ppp1p1p1/2n4p/3p1p2/P7/6PP/QPPPPP2/1BNNBRKR w HFhf - 0 9", 13203304, 5);
// skunk->perft_test_position("qnnbbrkr/1p2ppp1/2pp3p/p7/1P5P/2NP4/P1P1PPP1/Q1NBBRKR w HFhf - 0 9", 11110203, 5);
// skunk->perft_test_position("qn1rbbkr/ppp2p1p/1n1pp1p1/8/3P4/P6P/1PP1PPPK/QNNRBB1R w hd - 2 9", 19836606, 5);
// skunk->perft_test_position("qnr1bkrb/pppp2pp/3np3/5p2/8/P2P2P1/NPP1PP1P/QN1RBKRB w GDg - 3 9", 23114629, 5);
// skunk->perft_test_position("qb1nrkbr/1pppp1p1/1n3p2/p1B4p/8/3P1P1P/PPP1P1P1/QBNNRK1R w HEhe - 0 9", 21796206, 5);
// skunk->perft_test_position("qnnbrk1r/1p1ppbpp/2p5/p4p2/2NP3P/8/PPP1PPP1/Q1NBRKBR w HEhe - 0 9", 17819770, 5);
// skunk->perft_test_position("1qnrkbbr/1pppppp1/p1n4p/8/P7/1P1N1P2/2PPP1PP/QN1RKBBR w HDhd - 0 9", 29370838, 5);
// skunk->perft_test_position("qn1rkrbb/pp1p1ppp/2p1p3/3n4/4P2P/2NP4/PPP2PP1/Q1NRKRBB w FDfd - 1 9", 9482310, 5);
// skunk->perft_test_position("bb1qnrkr/pp1p1pp1/1np1p3/4N2p/8/1P4P1/P1PPPP1P/BBNQ1RKR w HFhf - 0 9", 24219627, 5);
// skunk->perft_test_position("bnqbnr1r/p1p1ppkp/3p4/1p4p1/P7/3NP2P/1PPP1PP1/BNQB1RKR w HF - 0 9", 23701014, 5);
// skunk->perft_test_position("bnqnrbkr/1pp2pp1/p7/3pP2p/4P1P1/8/PPPP3P/BNQNRBKR w HEhe d6 0 9", 29032175, 5);
// skunk->perft_test_position("b1qnrrkb/ppp1pp1p/n2p1Pp1/8/8/P7/1PPPP1PP/BNQNRKRB w GE - 0 9", 6718715, 5);
// skunk->perft_test_position("n1bqnrkr/pp1ppp1p/2p5/6p1/2P2b2/PN6/1PNPPPPP/1BBQ1RKR w HFhf - 2 9", 14481581, 5);
// skunk->perft_test_position("n1bb1rkr/qpnppppp/2p5/p7/P1P5/5P2/1P1PPRPP/NQBBN1KR w Hhf - 1 9", 14226907, 5);
// skunk->perft_test_position("nqb1rbkr/pppppp1p/4n3/6p1/4P3/1NP4P/PP1P1PP1/1QBNRBKR w HEhe - 1 9", 13780398, 5);
// skunk->perft_test_position("n1bnrrkb/pp1pp2p/2p2p2/6p1/5B2/3P4/PPP1PPPP/NQ1NRKRB w GE - 2 9", 10815324, 5);
// skunk->perft_test_position("nbqnbrkr/2ppp1p1/pp3p1p/8/4N2P/1N6/PPPPPPP1/1BQ1BRKR w HFhf - 0 9", 12719546, 5);
// skunk->perft_test_position("nq1bbrkr/pp2nppp/2pp4/4p3/1PP1P3/1B6/P2P1PPP/NQN1BRKR w HFhf - 2 9", 7697880, 5);
// skunk->perft_test_position("nqnrb1kr/2pp1ppp/1p1bp3/p1B5/5P2/3N4/PPPPP1PP/NQ1R1BKR w HDhd - 0 9", 13454573, 5);
// skunk->perft_test_position("nqn2krb/p1prpppp/1pbp4/7P/5P2/8/PPPPPKP1/NQNRB1RB w g - 3 9", 6194124, 5);
// skunk->perft_test_position("nb1n1kbr/ppp1rppp/3pq3/P3p3/8/4P3/1PPPRPPP/NBQN1KBR w Hh - 1 9", 8047916, 5);
// skunk->perft_test_position("nqnbrkbr/1ppppp1p/p7/6p1/6P1/P6P/1PPPPP2/NQNBRKBR w HEhe - 1 9", 4708975, 5);
// skunk->perft_test_position("nq1rkb1r/pp1pp1pp/1n2bp1B/2p5/8/5P1P/PPPPP1P1/NQNRKB1R w HDhd - 2 9", 17647882, 5);
// skunk->perft_test_position("nqnrkrb1/pppppp2/7p/4b1p1/8/PN1NP3/1PPP1PPP/1Q1RKRBB w FDfd - 1 9", 13055173, 5);
// skunk->perft_test_position("bb1nqrkr/1pp1ppp1/pn5p/3p4/8/P2NNP2/1PPPP1PP/BB2QRKR w HFhf - 0 9", 17454857, 5);
// skunk->perft_test_position("bnn1qrkr/pp1ppp1p/2p5/b3Q1p1/8/5P1P/PPPPP1P1/BNNB1RKR w HFhf - 2 9", 29742670, 5);
// skunk->perft_test_position("bnnqrbkr/pp1p2p1/2p1p2p/5p2/1P5P/1R6/P1PPPPP1/BNNQRBK1 w Ehe - 0 9", 32898113, 5);
// skunk->perft_test_position("b1nqrkrb/2pppppp/p7/1P6/1n6/P4P2/1P1PP1PP/BNNQRKRB w GEge - 0 9", 11735969, 5);
// skunk->perft_test_position("n1bnqrkr/3ppppp/1p6/pNp1b3/2P3P1/8/PP1PPP1P/NBB1QRKR w HFhf - 1 9", 15621236, 5);
// skunk->perft_test_position("n2bqrkr/p1p1pppp/1pn5/3p1b2/P6P/1NP5/1P1PPPP1/1NBBQRKR w HFhf - 3 9", 8088751, 5);
// skunk->perft_test_position("nnbqrbkr/1pp1p1p1/p2p4/5p1p/2P1P3/N7/PPQP1PPP/N1B1RBKR w HEhe - 0 9", 13755384, 5);
// skunk->perft_test_position("nnbqrkr1/pp1pp2p/2p2b2/5pp1/1P5P/4P1P1/P1PP1P2/NNBQRKRB w GEge - 1 9", 36218182, 5);
// skunk->perft_test_position("nb1qbrkr/p1pppp2/1p1n2pp/8/1P6/2PN3P/P2PPPP1/NB1QBRKR w HFhf - 0 9", 8697700, 5);
// skunk->perft_test_position("nnq1brkr/pp1pppp1/8/2p4P/8/5K2/PPPbPP1P/NNQBBR1R w hf - 0 9", 15338230, 5);
// skunk->perft_test_position("nnqrbb1r/pppppk2/5pp1/7p/1P6/3P2PP/P1P1PP2/NNQRBBKR w HD - 0 9", 17166700, 5);
// skunk->perft_test_position("nnqr1krb/p1p1pppp/2bp4/8/1p1P4/4P3/PPP2PPP/NNQRBKRB w GDgd - 0 9", 18162741, 5);
// skunk->perft_test_position("nbnqrkbr/p2ppp2/1p4p1/2p4p/3P3P/3N4/PPP1PPPR/NB1QRKB1 w Ehe - 0 9", 10630667, 5);
// skunk->perft_test_position("n1qbrkbr/p1ppp2p/2n2pp1/1p6/1P6/2P3P1/P2PPP1P/NNQBRKBR w HEhe - 0 9", 10356818, 5);
// skunk->perft_test_position("2qrkbbr/ppn1pppp/n1p5/3p4/5P2/P1PP4/1P2P1PP/NNQRKBBR w HDhd - 1 9", 16819085, 5);
// skunk->perft_test_position("1nqr1rbb/pppkp1pp/1n3p2/3p4/1P6/5P1P/P1PPPKP1/NNQR1RBB w - - 1 9", 11594634, 5);
// skunk->perft_test_position("bbn1rqkr/pp1pp2p/4npp1/2p5/1P6/2BPP3/P1P2PPP/1BNNRQKR w HEhe - 0 9", 14496370, 5);
// skunk->perft_test_position("bn1brqkr/pppp2p1/3npp2/7p/PPP5/8/3PPPPP/BNNBRQKR w HEhe - 0 9", 14284338, 5);
// skunk->perft_test_position("bn1rqbkr/ppp1ppp1/1n6/2p4p/7P/3P4/PPP1PPP1/BN1RQBKR w HDhd - 0 9", 18486027, 5);
// skunk->perft_test_position("bnnr1krb/ppp2ppp/3p4/3Bp3/q1P3PP/8/PP1PPP2/BNNRQKR1 w GDgd - 0 9", 31801525, 5);
// skunk->perft_test_position("1bbnrqkr/pp1ppppp/8/2p5/n7/3PNPP1/PPP1P2P/NBB1RQKR w HEhe - 1 9", 11394778, 5);
// skunk->perft_test_position("nnbbrqkr/p2ppp1p/1pp5/8/6p1/N1P5/PPBPPPPP/N1B1RQKR w HEhe - 0 9", 8846766, 5);
// skunk->perft_test_position("nnbrqbkr/2p1p1pp/p4p2/1p1p4/8/NP6/P1PPPPPP/N1BRQBKR w HDhd - 0 9", 7103549, 5);
// skunk->perft_test_position("nnbrqk1b/pp2pprp/2pp2p1/8/3PP1P1/8/PPP2P1P/NNBRQRKB w d - 1 9", 24714401, 5);
// skunk->perft_test_position("1bnrbqkr/ppnpp1p1/2p2p1p/8/1P6/4PPP1/P1PP3P/NBNRBQKR w HDhd - 0 9", 15964771, 5);
// skunk->perft_test_position("n1rbbqkr/pp1pppp1/7p/P1p5/1n6/2PP4/1P2PPPP/NNRBBQKR w HChc - 0 9", 10911545, 5);
// skunk->perft_test_position("n1rqb1kr/p1pppp1p/1pn4b/3P2p1/P7/1P6/2P1PPPP/NNRQBBKR w HChc - 0 9", 7419372, 5);
// skunk->perft_test_position("nnrqbkrb/pppp1pp1/7p/4p3/6P1/2N2B2/PPPPPP1P/NR1QBKR1 w Ggc - 2 9", 14233587, 5);
// skunk->perft_test_position("n1nrqkbr/ppb2ppp/3pp3/2p5/2P3P1/5P2/PP1PPB1P/NBNRQK1R w HDhd - 1 9", 22318948, 5);
// skunk->perft_test_position("2rbqkbr/p1pppppp/1nn5/1p6/7P/P4P2/1PPPP1PB/NNRBQK1R w HChc - 2 9", 13189156, 5);
// skunk->perft_test_position("nn1qkbbr/pp2ppp1/2rp4/2p4p/P2P4/1N5P/1PP1PPP1/1NRQKBBR w HCh - 1 9", 16420659, 5);
// skunk->perft_test_position("nnrqk1bb/p1ppp2p/5rp1/1p3p2/1P4P1/5P1P/P1PPP3/NNRQKRBB w FCc - 1 9", 17342527, 5);
// skunk->perft_test_position("bb1nrkqr/ppppn2p/4ppp1/8/1P4P1/4P3/P1PPKP1P/BBNNR1QR w he - 0 9", 15373803, 5);
// skunk->perft_test_position("bnnbrkqr/1p1ppp2/8/p1p3pp/1P6/N4P2/PBPPP1PP/2NBRKQR w HEhe - 0 9", 22562080, 5);
// skunk->perft_test_position("1nnrkbqr/p1pp1ppp/4p3/1p6/1Pb1P3/6PB/P1PP1P1P/BNNRK1QR w HDhd - 0 9", 19153245, 5);
// skunk->perft_test_position("bnr1kqrb/pppp1pp1/1n5p/4p3/P3P3/3P2P1/1PP2P1P/BNNRKQRB w GDg - 0 9", 11906515, 5);
// skunk->perft_test_position("nbbnrkqr/p1ppp1pp/1p3p2/8/2P5/4P3/PP1P1PPP/NBBNRKQR w HEhe - 1 9", 10817378, 5);
// skunk->perft_test_position("nn1brkqr/pp1bpppp/8/2pp4/P4P2/1PN5/2PPP1PP/N1BBRKQR w HEhe - 1 9", 13242252, 5);
// skunk->perft_test_position("n1brkbqr/ppp1pp1p/6pB/3p4/2Pn4/8/PP2PPPP/NN1RKBQR w HDhd - 0 9", 29436320, 5);
// skunk->perft_test_position("nnbrkqrb/p2ppp2/Q5pp/1pp5/4PP2/2N5/PPPP2PP/N1BRK1RB w GDgd - 0 9", 24321197, 5);
// skunk->perft_test_position("nbnrbk1r/pppppppq/8/7p/8/1N2QPP1/PPPPP2P/NB1RBK1R w HDhd - 2 9", 37143354, 5);
// skunk->perft_test_position("nnrbbkqr/2pppp1p/p7/6p1/1p2P3/4QPP1/PPPP3P/NNRBBK1R w HChc - 0 9", 16836636, 5);
// skunk->perft_test_position("nnrkbbqr/1p2pppp/p2p4/2p5/8/1N2P1P1/PPPP1P1P/1NKRBBQR w hc - 0 9", 13342771, 5);
// skunk->perft_test_position("n1rkbqrb/pp1ppp2/2n3p1/2p4p/P5PP/1P6/2PPPP2/NNRKBQRB w GCgc - 0 9", 18761475, 5);
// skunk->perft_test_position("nbkr1qbr/1pp1pppp/pn1p4/8/3P2P1/5R2/PPP1PP1P/NBN1KQBR w H - 2 9", 12815016, 5);
// skunk->perft_test_position("nnr1kqbr/pp1pp1p1/2p5/b4p1p/P7/1PNP4/2P1PPPP/N1RBKQBR w HChc - 1 9", 4266410, 5);
// skunk->perft_test_position("n1rkqbbr/p1pp1pp1/np2p2p/8/8/N4PP1/PPPPP1BP/N1RKQ1BR w HChc - 0 9", 14708490, 5);
// skunk->perft_test_position("nnr1qrbb/p2kpppp/1p1p4/2p5/6P1/PP1P4/2P1PP1P/NNRKQRBB w FC - 0 9", 11993332, 5);
// skunk->perft_test_position("bbnnrkrq/ppp1pp2/6p1/3p4/7p/7P/PPPPPPP1/BBNNRRKQ w ge - 0 9", 8427161, 5);
// skunk->perft_test_position("bnnbrkr1/ppp2p1p/5q2/3pp1p1/4P3/1N4P1/PPPPRP1P/BN1B1KRQ w Gge - 0 9", 28286576, 5);
// skunk->perft_test_position("bn1rkbrq/1pppppp1/p6p/1n6/3P4/6PP/PPPRPP2/BNN1KBRQ w Ggd - 2 9", 14333034, 5);
// skunk->perft_test_position("b1nrkrqb/1p1npppp/p2p4/2p5/5P2/4P2P/PPPP1RP1/BNNRK1QB w Dfd - 1 9", 7545536, 5);
// skunk->perft_test_position("1bbnrkrq/ppppppp1/8/7p/1n4P1/1PN5/P1PPPP1P/NBBR1KRQ w Gge - 0 9", 23443854, 5);
// skunk->perft_test_position("nnbbrkrq/2pp1pp1/1p5p/pP2p3/7P/N7/P1PPPPP1/N1BBRKRQ w GEge - 0 9", 6131124, 5);
// skunk->perft_test_position("nnbrkbrq/1pppp1p1/p7/7p/1P2Pp2/BN6/P1PP1PPP/1N1RKBRQ w GDgd - 0 9", 8084701, 5);
// skunk->perft_test_position("n1brkrqb/pppp3p/n3pp2/6p1/3P1P2/N1P5/PP2P1PP/N1BRKRQB w FDfd - 0 9", 14529434, 5);
// skunk->perft_test_position("nbnrbk2/p1pppp1p/1p3qr1/6p1/1B1P4/1N6/PPP1PPPP/1BNR1RKQ w d - 2 9", 20120565, 5);
// skunk->perft_test_position("nnrbbrkq/1pp2ppp/3p4/p3p3/3P1P2/1P2P3/P1P3PP/NNRBBKRQ w GC - 1 9", 19979594, 5);
// skunk->perft_test_position("nnrkbbrq/1pp2p1p/p2pp1p1/2P5/8/8/PP1PPPPP/NNRKBBRQ w Ggc - 0 9", 16838099, 5);
// skunk->perft_test_position("nnr1brqb/1ppkp1pp/8/p2p1p2/1P1P4/N1P5/P3PPPP/N1RKBRQB w FC - 1 9", 11343507, 5);
// skunk->perft_test_position("nbnrkrbq/2ppp2p/p4p2/1P4p1/4PP2/8/1PPP2PP/NBNRKRBQ w FDfd - 0 9", 23555139, 5);
// skunk->perft_test_position("1nrbkr1q/1pppp1pp/1n6/p4p2/N1b4P/8/PPPPPPPB/N1RBKR1Q w FCfc - 2 9", 22027695, 5);
// skunk->perft_test_position("nnrkrbbq/pppp2pp/8/4pp2/4P3/P7/1PPPBPPP/NNKRR1BQ w c - 0 9", 16473376, 5);
// skunk->perft_test_position("n1rk1qbb/pppprpp1/2n4p/4p3/2PP3P/8/PP2PPP1/NNRKRQBB w ECc - 1 9", 11420973, 5);
// skunk->perft_test_position("bbq1rnkr/pnp1pp1p/1p1p4/6p1/2P5/2Q1P2P/PP1P1PP1/BB1NRNKR w HEhe - 2 9", 28127620, 5);
// skunk->perft_test_position("bq1brnkr/1p1ppp1p/1np5/p5p1/8/1N5P/PPPPPPP1/BQ1BRNKR w HEhe - 0 9", 9359618, 5);
// skunk->perft_test_position("bq1rn1kr/1pppppbp/Nn4p1/8/8/P7/1PPPPPPP/BQ1RNBKR w HDhd - 1 9", 14692779, 5);
// skunk->perft_test_position("bqnr1kr1/pppppp1p/6p1/5n2/4B3/3N2PP/PbPPPP2/BQNR1KR1 w GDgd - 2 9", 43256823, 5);
// skunk->perft_test_position("qbb1rnkr/ppp3pp/4n3/3ppp2/1P3PP1/8/P1PPPN1P/QBB1RNKR w HEhe - 0 9", 16492398, 5);
// skunk->perft_test_position("qnbbr1kr/pp1ppp1p/4n3/6p1/2p3P1/2PP1P2/PP2P2P/QNBBRNKR w HEhe - 0 9", 11767038, 5);
// skunk->perft_test_position("1nbrnbkr/p1ppp1pp/1p6/5p2/4q1PP/3P4/PPP1PP2/QNBRNBKR w HDhd - 1 9", 36048727, 5);
// skunk->perft_test_position("q1brnkrb/p1pppppp/n7/1p6/P7/3P1P2/QPP1P1PP/1NBRNKRB w GDgd - 0 9", 23143989, 5);
// skunk->perft_test_position("qbnrb1kr/ppp1pp1p/3p4/2n3p1/1P6/6N1/P1PPPPPP/QBNRB1KR w HDhd - 2 9", 19555214, 5);
// skunk->perft_test_position("q1rbbnkr/pppp1p2/2n3pp/2P1p3/3P4/8/PP1NPPPP/Q1RBBNKR w HChc - 2 9", 21694330, 5);
// skunk->perft_test_position("q1r1bbkr/pnpp1ppp/2n1p3/1p6/2P2P2/2N1N3/PP1PP1PP/Q1R1BBKR w HChc - 2 9", 31204371, 5);
// skunk->perft_test_position("2rnbkrb/pqppppp1/1pn5/7p/2P5/P1R5/QP1PPPPP/1N1NBKRB w Ggc - 4 9", 11856964, 5);
// skunk->perft_test_position("qbnr1kbr/p2ppppp/2p5/1p6/4n2P/P4N2/1PPP1PP1/QBNR1KBR w HDhd - 0 9", 21855658, 5);
// skunk->perft_test_position("qnrbnk1r/pp1pp2p/5p2/2pbP1p1/3P4/1P6/P1P2PPP/QNRBNKBR w HChc - 0 9", 24415089, 5);
// skunk->perft_test_position("qnrnk1br/p1p2ppp/8/1pbpp3/8/PP2N3/1QPPPPPP/1NR1KBBR w HChc - 0 9", 17477825, 5);
// skunk->perft_test_position("qnrnkrbb/Bpppp2p/6p1/5p2/5P2/3PP3/PPP3PP/QNRNKR1B w FCfc - 1 9", 25251641, 5);
// skunk->perft_test_position("bbnqrn1r/ppppp2k/5p2/6pp/7P/1QP5/PP1PPPP1/B1N1RNKR w HE - 0 9", 16693640, 5);
// skunk->perft_test_position("b1qbrnkr/ppp1pp2/2np4/6pp/4P3/2N4P/PPPP1PP1/BQ1BRNKR w HEhe - 0 9", 22197063, 5);
// skunk->perft_test_position("bnqr1bkr/pp1ppppp/2p5/4N3/5P2/P7/1PPPPnPP/BNQR1BKR w HDhd - 3 9", 8601011, 5);
// skunk->perft_test_position("b1qr1krb/pp1ppppp/n2n4/8/2p5/2P3P1/PP1PPP1P/BNQRNKRB w GDgd - 0 9", 15583376, 5);
// skunk->perft_test_position("nbbqr1kr/1pppp1pp/8/p1n2p2/4P3/PN6/1PPPQPPP/1BB1RNKR w HEhe - 0 9", 19478789, 5);
// skunk->perft_test_position("nqbbrn1r/p1pppp1k/1p4p1/7p/4P3/1R3B2/PPPP1PPP/NQB2NKR w H - 0 9", 9002073, 5);
// skunk->perft_test_position("nqbr1bkr/p1p1ppp1/1p1n4/3pN2p/1P6/8/P1PPPPPP/NQBR1BKR w HDhd - 0 9", 24703467, 5);
// skunk->perft_test_position("nqbrn1rb/pppp1kp1/5p1p/4p3/P4B2/3P2P1/1PP1PP1P/NQ1RNKRB w GD - 0 9", 15556806, 5);
// skunk->perft_test_position("nb1r1nkr/ppp1ppp1/2bp4/7p/3P2qP/P6R/1PP1PPP1/NBQRBNK1 w Dhd - 1 9", 88557078, 5);
// skunk->perft_test_position("n1rbbnkr/1p1pp1pp/p7/2p1qp2/1B3P2/3P4/PPP1P1PP/NQRB1NKR w HChc - 0 9", 19866918, 5);
// skunk->perft_test_position("nqrnbbkr/p2p1p1p/1pp5/1B2p1p1/1P3P2/4P3/P1PP2PP/NQRNB1KR w HChc - 0 9", 28053260, 5);
// skunk->perft_test_position("nqr1bkrb/ppp1pp2/2np2p1/P6p/8/2P4P/1P1PPPP1/NQRNBKRB w GCgc - 0 9", 12681936, 5);
// skunk->perft_test_position("nb1rnkbr/pqppppp1/1p5p/8/1PP4P/8/P2PPPP1/NBQRNKBR w HDhd - 1 9", 22616076, 5);
// skunk->perft_test_position("nqrbnkbr/2p1p1pp/3p4/pp3p2/6PP/3P1N2/PPP1PP2/NQRB1KBR w HChc - 0 9", 9698432, 5);
// skunk->perft_test_position("nqrnkbbr/pp1p1p1p/4p1p1/1p6/8/5P1P/P1PPP1P1/NQRNKBBR w HChc - 0 9", 34914919, 5);
// skunk->perft_test_position("nqrnkrbb/p2ppppp/1p6/2p5/2P3P1/5P2/PP1PPN1P/NQR1KRBB w FCfc - 1 9", 21141738, 5);
// skunk->perft_test_position("bbnrqrk1/pp2pppp/4n3/2pp4/P7/1N5P/BPPPPPP1/B2RQNKR w HD - 2 9", 14343443, 5);
// skunk->perft_test_position("bnr1qnkr/p1pp1p1p/1p4p1/4p1b1/2P1P3/1P6/PB1P1PPP/1NRBQNKR w HChc - 1 9", 30026687, 5);
// skunk->perft_test_position("b1rqnbkr/ppp1ppp1/3p3p/2n5/P3P3/2NP4/1PP2PPP/B1RQNBKR w HChc - 0 9", 11099382, 5);
// skunk->perft_test_position("bnrqnr1b/pp1pkppp/2p1p3/P7/2P5/7P/1P1PPPP1/BNRQNKRB w GC - 0 9", 11208688, 5);
// skunk->perft_test_position("n1brq1kr/bppppppp/p7/8/4P1Pn/8/PPPP1P2/NBBRQNKR w HDhd - 0 9", 9919113, 5);
// skunk->perft_test_position("1rbbqnkr/ppn1ppp1/3p3p/2p5/3P4/1N4P1/PPPBPP1P/1R1BQNKR w HBhb - 0 9", 31059587, 5);
// skunk->perft_test_position("nrbq2kr/ppppppb1/5n1p/5Pp1/8/P5P1/1PPPP2P/NRBQNBKR w HBhb - 1 9", 7809837, 5);
// skunk->perft_test_position("nrb1nkrb/pp3ppp/1qBpp3/2p5/8/P5P1/1PPPPP1P/NRBQNKR1 w GBgb - 2 9", 21981567, 5);
// skunk->perft_test_position("1br1bnkr/ppqppp1p/1np3p1/8/1PP4P/4N3/P2PPPP1/NBRQB1KR w HChc - 1 9", 22076141, 5);
// skunk->perft_test_position("nrqbb1kr/1p1pp1pp/2p3n1/p4p2/3PP3/P5N1/1PP2PPP/NRQBB1KR w HBhb - 0 9", 23239122, 5);
// skunk->perft_test_position("nrqn1bkr/ppppp1pp/4b3/8/4P1p1/5P2/PPPP3P/NRQNBBKR w HBhb - 0 9", 15236287, 5);
// skunk->perft_test_position("nrqnbrkb/pppp1p2/4p2p/3B2p1/8/1P4P1/PQPPPP1P/NR1NBKR1 w GB - 0 9", 21284835, 5);
// skunk->perft_test_position("nbrq1kbr/Bp3ppp/2pnp3/3p4/5P2/2P4P/PP1PP1P1/NBRQNK1R w HChc - 0 9", 56588117, 5);
// skunk->perft_test_position("nrqbnkbr/1p2ppp1/p1p4p/3p4/1P6/8/PQPPPPPP/1RNBNKBR w HBhb - 0 9", 21427496, 5);
// skunk->perft_test_position("nrqn1bbr/2ppkppp/4p3/pB6/8/2P1P3/PP1P1PPP/NRQNK1BR w HB - 1 9", 11872805, 5);
// skunk->perft_test_position("nrqnkrb1/p1ppp2p/1p4p1/4bp2/4PP1P/4N3/PPPP2P1/NRQ1KRBB w FBfb - 1 9", 28520172, 5);
// skunk->perft_test_position("1bnrnqkr/pbpp2pp/8/1p2pp2/P6P/3P1N2/1PP1PPP1/BBNR1QKR w HDhd - 0 9", 21581178, 5);
// skunk->perft_test_position("b1rbnqkr/1pp1ppp1/2n4p/p2p4/5P2/1PBP4/P1P1P1PP/1NRBNQKR w HChc - 0 9", 9537260, 5);
// skunk->perft_test_position("1nrnqbkr/p1pppppp/1p6/8/2b2P2/P1N5/1PP1P1PP/BNR1QBKR w HChc - 2 9", 14216070, 5);
// skunk->perft_test_position("1nrnqkrb/2ppp1pp/p7/1p3p2/5P2/N5K1/PPPPP2P/B1RNQ1RB w gc - 0 9", 18547476, 5);
// skunk->perft_test_position("nbbr1qkr/p1pppppp/8/1p1n4/3P4/1N3PP1/PPP1P2P/1BBRNQKR w HDhd - 1 9", 16555068, 5);
// skunk->perft_test_position("1rbbnqkr/1pnppp1p/p5p1/2p5/2P4P/5P2/PP1PP1PR/NRBBNQK1 w Bhb - 1 9", 9863080, 5);
// skunk->perft_test_position("nrb1qbkr/2pppppp/2n5/p7/2p5/4P3/PPNP1PPP/1RBNQBKR w HBhb - 0 9", 12044358, 5);
// skunk->perft_test_position("nrb1qkrb/2ppppp1/p3n3/1p1B3p/2P5/6P1/PP1PPPRP/NRBNQK2 w Bgb - 2 9", 11806808, 5);
// skunk->perft_test_position("nbrn1qkr/ppp1pp2/3p2p1/3Q3P/b7/8/PPPPPP1P/NBRNB1KR w HChc - 2 9", 42201531, 5);
// skunk->perft_test_position("nr1bbqkr/pp1pp2p/1n3pp1/2p5/8/1P4P1/P1PPPPQP/NRNBBK1R w hb - 0 9", 11582539, 5);
// skunk->perft_test_position("nr2bbkr/ppp1pppp/1n1p4/8/6PP/1NP4q/PP1PPP2/1RNQBBKR w HBhb - 1 9", 13287051, 5);
// skunk->perft_test_position("1rnqbkrb/ppp1p1p1/1n3p2/3p3p/P6P/4P3/1PPP1PP1/NRNQBRKB w gb - 0 9", 9968830, 5);
// skunk->perft_test_position("nb1rqkbr/1pppp1pp/4n3/p4p2/6PP/5P2/PPPPPN2/NBR1QKBR w HCh - 0 9", 13378840, 5);
// skunk->perft_test_position("nrnbqkbr/2pp2pp/4pp2/pp6/8/1P3P2/P1PPPBPP/NRNBQ1KR w hb - 0 9", 12525939, 5);
// skunk->perft_test_position("nrnqkbbr/ppppp1p1/7p/5p2/8/P4PP1/NPPPP2P/NR1QKBBR w HBhb - 0 9", 15952533, 5);
// skunk->perft_test_position("1rnqkr1b/ppppp2p/1n3pp1/8/2P3P1/Pb1N4/1P1PPP1P/NR1QKRBB w FBfb - 0 9", 15865528, 5);
// skunk->perft_test_position("bbnrnkqr/1pppp1pp/5p2/p7/7P/1P6/PBPPPPPR/1BNRNKQ1 w D - 2 9", 14375839, 5);
// skunk->perft_test_position("bnrbk1qr/1ppp1ppp/p2np3/8/P7/2N2P2/1PPPP1PP/B1RBNKQR w HC - 0 9", 13514201, 5);
// skunk->perft_test_position("br1nkbqr/ppppppp1/8/n6p/8/N1P2PP1/PP1PP2P/B1RNKBQR w HCh - 1 9", 16125924, 5);
// skunk->perft_test_position("bnr1kqrb/pp1pppp1/2n5/2p5/1P4Pp/4N3/P1PPPP1P/BNKR1QRB w gc - 0 9", 27792175, 5);
// skunk->perft_test_position("1bbrnkqr/pp1p1ppp/2p1p3/1n6/5P2/3Q4/PPPPP1PP/NBBRNK1R w HDhd - 2 9", 26998966, 5);
// skunk->perft_test_position("nrbbnk1r/pp2pppq/8/2pp3p/3P2P1/1N6/PPP1PP1P/1RBBNKQR w HBhb - 0 9", 35627310, 5);
// skunk->perft_test_position("nr1nkbqr/ppp3pp/5p2/3pp3/6b1/3PP3/PPP2PPP/NRBNKBQR w hb - 0 9", 10658989, 5);
// skunk->perft_test_position("nrbnk1rb/ppp1pq1p/3p4/5pp1/2P1P3/1N6/PP1PKPPP/1RBN1QRB w gb - 2 9", 23957242, 5);
// skunk->perft_test_position("1brnbkqr/pppppp2/6p1/7p/1Pn5/P1NP4/2P1PPPP/NBR1BKQR w HChc - 0 9", 9627826, 5);
// skunk->perft_test_position("nrnbbk1r/p1pppppq/8/7p/1p6/P5PP/1PPPPPQ1/NRNBBK1R w HBhb - 2 9", 27229468, 5);
// skunk->perft_test_position("n1nkb1qr/prppppbp/6p1/1p6/2P2P2/P7/1P1PP1PP/NRNKBBQR w HBh - 1 9", 21952444, 5);
// skunk->perft_test_position("nr2bqrb/ppkpp1pp/1np5/5p1P/5P2/2P5/PP1PP1P1/NRNKBQRB w GB - 0 9", 9244693, 5);
// skunk->perft_test_position("nbr1kqbr/p3pppp/2ppn3/1p4P1/4P3/1P6/P1PP1P1P/NBRNKQBR w HChc - 1 9", 9692630, 5);
// skunk->perft_test_position("nr1bkqbr/1p1pp1pp/pnp2p2/8/6P1/P1PP4/1P2PP1P/NRNBKQBR w HBhb - 0 9", 9305533, 5);
// skunk->perft_test_position("nr1kqbbr/np2pppp/p1p5/1B1p1P2/8/4P3/PPPP2PP/NRNKQ1BR w HBhb - 0 9", 18103280, 5);
// skunk->perft_test_position("nrnk1rbb/p1p2ppp/3pq3/Qp2p3/1P1P4/8/P1P1PPPP/NRN1KRBB w fb - 2 9", 23868737, 5);
// skunk->perft_test_position("bbnrnkrq/pp1ppp1p/6p1/2p5/6P1/P5RP/1PPPPP2/BBNRNK1Q w Dgd - 3 9", 54843403, 5);
// skunk->perft_test_position("bnrb1rkq/ppnpppp1/3Q4/2p4p/7P/N7/PPPPPPP1/B1RBNKR1 w GC - 2 9", 28784300, 5);
// skunk->perft_test_position("bnrnkbrq/p1ppppp1/1p5p/8/P2PP3/5P2/1PP3PP/BNRNKBRQ w GCgc - 1 9", 11965544, 5);
// skunk->perft_test_position("bnrnkrqb/pp2p2p/2pp1pp1/8/P7/2PP1P2/1P2P1PP/BNRNKRQB w FCfc - 0 9", 15966934, 5);
// skunk->perft_test_position("nbbrnkr1/1pppp1p1/p6q/P4p1p/8/5P2/1PPPP1PP/NBBRNRKQ w gd - 2 9", 6629293, 5);
// skunk->perft_test_position("nrb1nkrq/2pp1ppp/p4b2/1p2p3/P4B2/3P4/1PP1PPPP/NR1BNRKQ w gb - 0 9", 9227883, 5);
// skunk->perft_test_position("nrbnkbrq/p3p1pp/1p6/2pp1P2/8/3PP3/PPP2P1P/NRBNKBRQ w GBgb - 0 9", 21019301, 5);
// skunk->perft_test_position("nrbnkrqb/pppp1p1p/4p1p1/8/7P/2P1P3/PPNP1PP1/1RBNKRQB w FBfb - 0 9", 5760165, 5);
// skunk->perft_test_position("nbrn1krq/ppp1p2p/6b1/3p1pp1/8/4N1PP/PPPPPP2/NBR1BRKQ w gc - 1 9", 22667987, 5);
// skunk->perft_test_position("nrnbbkrq/p1pp2pp/5p2/1p6/2P1pP1B/1P6/P2PP1PP/NRNB1KRQ w GBgb - 0 9", 11489727, 5);
// skunk->perft_test_position("nrn1bbrq/1ppkppp1/p2p3p/8/1P3N2/4P3/P1PP1PPP/NR1KBBRQ w GB - 2 9", 12069159, 5);
// skunk->perft_test_position("n1krbrqb/1ppppppp/p7/8/4n3/P4P1P/1PPPPQP1/NRNKBR1B w FB - 2 9", 12167153, 5);
// skunk->perft_test_position("n1rnkrbq/1p1ppp1p/8/p1p1b1p1/3PQ1P1/4N3/PPP1PP1P/NBR1KRB1 w FCfc - 0 9", 35738410, 5);
// skunk->perft_test_position("nrnbkrbq/2pp1pp1/pp6/4p2p/P7/5PPP/1PPPP3/NRNBKRBQ w FBfb - 0 9", 11920087, 5);
// skunk->perft_test_position("1rnkrbbq/pp1p2pp/1n3p2/1Bp1p3/1P6/1N2P3/P1PP1PPP/1RNKR1BQ w EBeb - 0 9", 31703749, 5);
// skunk->perft_test_position("nr1krqbb/p1ppppp1/8/1p5p/1Pn5/5P2/P1PPP1PP/NRNKRQBB w EBeb - 0 9", 11371067, 5);
// skunk->perft_test_position("bbq1rkr1/1ppppppp/p1n2n2/8/2P2P2/1P6/PQ1PP1PP/BB1NRKNR w HEe - 3 9", 24085223, 5);
// skunk->perft_test_position("b1nbrknr/1qppp1pp/p4p2/1p6/6P1/P2NP3/1PPP1P1P/BQ1BRKNR w HEhe - 1 9", 13157826, 5);
// skunk->perft_test_position("bqnrk1nr/pp2ppbp/6p1/2pp4/2P5/5P2/PPQPP1PP/B1NRKBNR w HDhd - 0 9", 21341087, 5);
// skunk->perft_test_position("bqnrknrb/1ppp1p1p/p7/6p1/1P2p3/P1PN4/3PPPPP/BQ1RKNRB w GDgd - 0 9", 16391601, 5);
// skunk->perft_test_position("q1b1rknr/pp1pppp1/4n2p/2p1b3/1PP5/4P3/PQ1P1PPP/1BBNRKNR w HEhe - 1 9", 32649943, 5);
// skunk->perft_test_position("qnbbrknr/1p1ppppp/8/p1p5/5P2/PP1P4/2P1P1PP/QNBBRKNR w HEhe - 0 9", 11562434, 5);
// skunk->perft_test_position("q1brkb1r/p1pppppp/np3B2/8/6n1/1P5N/P1PPPPPP/QN1RKB1R w HDhd - 0 9", 32597704, 5);
// skunk->perft_test_position("qn1rk1rb/p1pppppp/1p2n3/8/2b5/4NPP1/PPPPP1RP/QNBRK2B w Dgd - 4 9", 17761431, 5);
// skunk->perft_test_position("qbnrbknr/ppp2p1p/8/3pp1p1/1PP1B3/5N2/P2PPPPP/Q1NRBK1R w HDhd - 0 9", 32523099, 5);
// skunk->perft_test_position("qnrbb1nr/pp1p1ppp/2p2k2/4p3/4P3/5PPP/PPPP4/QNRBBKNR w HC - 0 9", 5846781, 5);
// skunk->perft_test_position("qnr1bbnr/ppk1p1pp/3p4/2p2p2/8/2P5/PP1PPPPP/QNKRBBNR w - - 1 9", 7994547, 5);
// skunk->perft_test_position("qnrkbnrb/1p1p1ppp/2p5/4p3/p7/N1BP4/PPP1PPPP/Q1R1KNRB w gc - 0 9", 10845146, 5);
// skunk->perft_test_position("qbnrkn1r/1pppp1p1/p3bp2/2BN3p/8/5P2/PPPPP1PP/QBNRK2R w HDhd - 0 9", 38511307, 5);
// skunk->perft_test_position("qnrbknbr/1pp2ppp/4p3/p6N/2p5/8/PPPPPPPP/Q1RBK1BR w HChc - 0 9", 7403327, 5);
// skunk->perft_test_position("1qkrnbbr/p1pppppp/2n5/1p6/8/5NP1/PPPPPP1P/QNRK1BBR w HC - 4 9", 9396521, 5);
// skunk->perft_test_position("q1rknr1b/1ppppppb/2n5/p2B3p/8/1PN3P1/P1PPPP1P/Q1RKNRB1 w FCfc - 3 9", 27463479, 5);
// skunk->perft_test_position("bbnqrk1r/pp1pppp1/2p4p/8/6n1/1N1P1P2/PPP1P1PP/BBQ1RKNR w HEhe - 4 9", 18024195, 5);
// skunk->perft_test_position("bn1brknr/ppp1p1pp/5p2/3p4/6qQ/3P3P/PPP1PPP1/BN1BRKNR w HEhe - 4 9", 20290974, 5);
// skunk->perft_test_position("1nqrkbnr/2pp1ppp/pp2p3/3b4/2P5/N7/PP1PPPPP/B1QRKBNR w HDhd - 0 9", 13133439, 5);
// skunk->perft_test_position("bnqrk1rb/1pp1pppp/p2p4/4n3/2PPP3/8/PP3PPP/BNQRKNRB w GDgd - 1 9", 27610213, 5);
// skunk->perft_test_position("nbb1rknr/1ppq1ppp/3p4/p3p3/4P3/1N2R3/PPPP1PPP/1BBQ1KNR w Hhe - 2 9", 30894863, 5);
// skunk->perft_test_position("nqbbrknr/2ppp2p/pp4p1/5p2/7P/3P1P2/PPPBP1P1/NQ1BRKNR w HEhe - 0 9", 7583292, 5);
// skunk->perft_test_position("1qbrkb1r/pppppppp/8/3n4/4P1n1/PN6/1PPP1P1P/1QBRKBNR w HDhd - 3 9", 17313279, 5);
// skunk->perft_test_position("1qbrknrb/1p1ppppp/1np5/8/p4P1P/4P1N1/PPPP2P1/NQBRK1RB w GDgd - 0 9", 6218644, 5);
// skunk->perft_test_position("nbqrbkr1/ppp1pppp/8/3p4/6n1/2P2PPN/PP1PP2P/NBQRBK1R w HDd - 1 9", 24138518, 5);
// skunk->perft_test_position("nqrb1knr/1ppbpp1p/p7/3p2p1/2P3P1/5P1P/PP1PP3/NQRBBKNR w HChc - 1 9", 21998733, 5);
// skunk->perft_test_position("1qrkbbr1/pppp1ppp/1n3n2/4p3/5P2/1N6/PPPPP1PP/1QRKBBNR w HCc - 0 9", 15514933, 5);
// skunk->perft_test_position("nqrkb1rb/pp2pppp/2p1n3/3p4/3PP1N1/8/PPP2PPP/NQRKB1RB w GCgc - 0 9", 19185851, 5);
// skunk->perft_test_position("nb1rknbr/pp2ppp1/8/2Bp3p/6P1/2P2P1q/PP1PP2P/NBQRKN1R w HDhd - 0 9", 53033675, 5);
// skunk->perft_test_position("nqrbkn1r/pp1pp1pp/8/2p2p2/5P2/P3B2P/1PbPP1P1/NQRBKN1R w HChc - 0 9", 18296195, 5);
// skunk->perft_test_position("nqrknbbr/pp1pppp1/7p/2p5/7P/1P1N4/P1PPPPPB/NQRK1B1R w HChc - 2 9", 19429491, 5);
// skunk->perft_test_position("1qrknrbb/B1p1pppp/8/1p1p4/2n2P2/1P6/P1PPP1PP/NQRKNR1B w FCfc - 0 9", 16065378, 5);
// skunk->perft_test_position("bbnrqk1r/1ppppppp/8/7n/1p6/P6P/1BPPPPP1/1BNRQKNR w HDhd - 0 9", 10697065, 5);
// skunk->perft_test_position("bnrbqknr/ppp3p1/3ppp1Q/7p/3P4/1P6/P1P1PPPP/BNRB1KNR w HChc - 0 9", 23717883, 5);
// skunk->perft_test_position("bn1qkb1r/pprppppp/8/2p5/2PPP1n1/8/PPR2PPP/BN1QKBNR w Hh - 1 9", 25245957, 5);
// skunk->perft_test_position("1nrqknrb/p1pp1ppp/1p2p3/3N4/5P1P/5b2/PPPPP3/B1RQKNRB w GCgc - 2 9", 25128076, 5);
// skunk->perft_test_position("nbbrqrk1/pppppppp/8/2N1n3/P7/6P1/1PPPPP1P/1BBRQKNR w HD - 3 9", 9153089, 5);
// skunk->perft_test_position("1rbbqknr/1ppp1pp1/1n2p3/p6p/4P1P1/P6N/1PPP1P1P/NRBBQK1R w HBhb - 0 9", 15133381, 5);
// skunk->perft_test_position("nrq1kbnr/p1pbpppp/3p4/1p6/6P1/1N3N2/PPPPPP1P/1RBQKB1R w HBhb - 4 9", 12871967, 5);
// skunk->perft_test_position("nr1qknr1/p1pppp1p/b5p1/1p6/8/P4PP1/1bPPP1RP/NRBQKN1B w Bgb - 0 9", 7777833, 5);
// skunk->perft_test_position("nbrqbknr/1ppp2pp/8/4pp2/p2PP1P1/7N/PPP2P1P/NBRQBK1R w HChc - 0 9", 22305910, 5);
// skunk->perft_test_position("nr1b1k1r/ppp1pppp/2bp1n2/6P1/2P3q1/5P2/PP1PP2P/NRQBBKNR w HBhb - 1 9", 35121759, 5);
// skunk->perft_test_position("nrqkbbnr/2pppp1p/p7/1p6/2P1Pp2/8/PPNP2PP/1RQKBBNR w HBhb - 0 9", 13097064, 5);
// skunk->perft_test_position("1rqkbnrb/pp1ppp1p/1n4p1/B1p5/3PP3/4N3/PPP2PPP/NRQK2RB w GBgb - 0 9", 19715083, 5);
// skunk->perft_test_position("nbrqkn1r/1pppp2p/5pp1/p2b4/5P2/P2PN3/1PP1P1PP/NBRQK1BR w HChc - 2 9", 11026383, 5);
// skunk->perft_test_position("nrqbknbr/pp1pppp1/8/2p4p/P3PP2/8/1PPP2PP/NRQBKNBR w HBhb - 1 9", 16058815, 5);
// skunk->perft_test_position("nrqknbbr/p2pppp1/1pp5/6Qp/3P4/1P3P2/P1P1P1PP/NR1KNBBR w HBhb - 0 9", 29263502, 5);
// skunk->perft_test_position("nrqknrbb/1p3ppp/p2p4/2p1p3/1P6/3PP1P1/P1P2P1P/NRQKNRBB w FBfb - 0 9", 19532077, 5);
// skunk->perft_test_position("1bnrkqnr/p1pppp2/7p/1p4p1/4b3/7N/PPPP1PPP/BBNRKQ1R w HDhd - 0 9", 16661676, 5);
// skunk->perft_test_position("bnrbkq1r/pp2p1pp/5n2/2pp1p2/P7/N1PP4/1P2PPPP/B1RBKQNR w HChc - 1 9", 15079602, 5);
// skunk->perft_test_position("2rkqbnr/p1pppppp/2b5/1pn5/1P3P1Q/2B5/P1PPP1PP/1NRK1BNR w HChc - 3 9", 28194726, 5);
// skunk->perft_test_position("bnrkqnrb/2pppp2/8/pp4pp/1P5P/6P1/P1PPPPB1/BNRKQNR1 w GCgc - 0 9", 33195397, 5);
// skunk->perft_test_position("1bbrkq1r/pppp2pp/1n2pp1n/8/2PP4/1N4P1/PP2PP1P/1BBRKQNR w HDhd - 1 9", 26970098, 5);
// skunk->perft_test_position("nrbbkqnr/1p2pp1p/p1p3p1/3p4/8/1PP5/P2PPPPP/NRBBKQNR w HBhb - 0 9", 9539687, 5);
// skunk->perft_test_position("1rbkqbr1/ppp1pppp/1n5n/3p4/3P4/1PP3P1/P3PP1P/NRBKQBNR w HBb - 1 9", 16986290, 5);
// skunk->perft_test_position("nrbkq1rb/1ppp1pp1/4p1n1/p6p/2PP4/5P2/PPK1P1PP/NRB1QNRB w gb - 0 9", 16906409, 5);
// skunk->perft_test_position("nbrkbqnr/p2pp1p1/5p2/1pp4p/7P/3P2P1/PPP1PP2/NBKRBQNR w hc - 0 9", 12879258, 5);
// skunk->perft_test_position("nrkb1qnr/ppppp1p1/6bp/5p2/1PP1P1P1/8/P2P1P1P/NRKBBQNR w HBhb - 1 9", 20671433, 5);
// skunk->perft_test_position("nrk1bbnr/p1q1pppp/1ppp4/8/3P3P/4K3/PPP1PPP1/NR1QBBNR w hb - 0 9", 16278120, 5);
// skunk->perft_test_position("nrkqbr1b/1pppp1pp/5pn1/p6N/1P3P2/8/P1PPP1PP/NRKQB1RB w GBb - 0 9", 8763742, 5);
// skunk->perft_test_position("nbrkq2r/pppp1bpp/4p1n1/5p2/7P/2P3N1/PP1PPPP1/NBKRQ1BR w hc - 0 9", 15394667, 5);
// skunk->perft_test_position("nrkbqnbr/2ppp2p/pp6/5pp1/P1P5/8/1P1PPPPP/NRKBQNBR w HBhb - 0 9", 7218486, 5);
// skunk->perft_test_position("nr1qnbbr/pk1pppp1/1pp4p/8/3P4/5P1P/PPP1P1P1/NRKQNBBR w HB - 0 9", 9587439, 5);
// skunk->perft_test_position("nrkq1rbb/pp1ppp1p/2pn4/8/PP3Pp1/7P/2PPP1P1/NRKQNRBB w FBfb - 0 9", 19867117, 5);
// skunk->perft_test_position("b2rknqr/pp1ppppp/8/2P5/n7/P7/1PPNPPPb/BBNRK1QR w HDhd - 2 9", 17734818, 5);
// skunk->perft_test_position("bnrbknqr/pp2p2p/2p3p1/3p1p2/8/3P4/PPPNPPPP/B1RBKNQR w HChc - 0 9", 10133092, 5);
// skunk->perft_test_position("bnrknb1r/pppp2pp/8/4pp2/6P1/3P3P/qPP1PPQ1/BNRKNB1R w HChc - 0 9", 36142423, 5);
// skunk->perft_test_position("b1rknqrb/ppp1p1p1/2np1p1p/8/4N3/6PQ/PPPPPP1P/B1RKN1RB w GCgc - 0 9", 16897544, 5);
// skunk->perft_test_position("nb1rknqr/pbppp2p/6p1/1p3p2/5P2/3KP3/PPPP2PP/NBBR1NQR w hd - 2 9", 5822387, 5);
// skunk->perft_test_position("nr1bknqr/1ppb1ppp/p7/3pp3/B7/2P3NP/PP1PPPP1/NRB1K1QR w HBhb - 2 9", 15153092, 5);
// skunk->perft_test_position("nrbkn2r/pppp1pqp/4p1p1/8/3P2P1/P3B3/P1P1PP1P/NR1KNBQR w HBhb - 1 9", 22094260, 5);
// skunk->perft_test_position("nrbknqrb/2p1ppp1/1p6/p2p2Bp/1P6/3P1P2/P1P1P1PP/NR1KNQRB w GBgb - 0 9", 12225742, 5);
// skunk->perft_test_position("nbr1knqr/1pp1p1pp/3p1pb1/8/7P/5P2/PPPPPQP1/NBRKBN1R w HC - 2 9", 24965592, 5);
// skunk->perft_test_position("n1kbbnqr/prp2ppp/1p1p4/4p3/1P2P3/3P1B2/P1P2PPP/NRK1BNQR w HBh - 2 9", 12187583, 5);
// skunk->perft_test_position("nrknbbqr/pp3p1p/B3p1p1/2pp4/4P3/2N3P1/PPPP1P1P/NRK1B1QR w HBhb - 0 9", 14684565, 5);
// skunk->perft_test_position("n1knbqrb/pr1p1ppp/Qp6/2p1p3/4P3/6P1/PPPP1P1P/NRKNB1RB w GBg - 2 9", 11663330, 5);
// skunk->perft_test_position("nbrknqbr/p3p1pp/1p1p1p2/2p5/2Q1PP2/8/PPPP2PP/NBRKN1BR w HChc - 0 9", 28899548, 5);
// skunk->perft_test_position("nrkb1qbr/pp1pppp1/5n2/7p/2p5/1N1NPP2/PPPP2PP/1RKB1QBR w HBhb - 0 9", 15045589, 5);
// skunk->perft_test_position("nrk2bbr/pppqpppp/3p4/8/1P3nP1/3P4/P1P1PP1P/NRKNQBBR w HBhb - 1 9", 17603960, 5);
// skunk->perft_test_position("nrknqrbb/1p2ppp1/2pp4/Q6p/P2P3P/8/1PP1PPP1/NRKN1RBB w FBfb - 0 9", 9569590, 5);
// skunk->perft_test_position("bbnrk1rq/pp2p1pp/2ppn3/5p2/8/3NNP1P/PPPPP1P1/BB1RK1RQ w GDgd - 1 9", 15301879, 5);
// skunk->perft_test_position("bnrbknrq/ppppp2p/6p1/5p2/4QPP1/8/PPPPP2P/BNRBKNR1 w GCgc - 0 9", 31385912, 5);
// skunk->perft_test_position("bnkrnbrq/ppppp1p1/B6p/5p2/8/4P3/PPPP1PPP/BNKRN1RQ w - - 0 9", 5980981, 5);
// skunk->perft_test_position("bnrk1rqb/2pppp1p/3n4/pp4p1/3Q1P2/2N3P1/PPPPP2P/B1RKNR1B w FCfc - 0 9", 107234294, 5);
// skunk->perft_test_position("nbbrk1rq/pp2pppp/2pp4/8/2P2n2/6N1/PP1PP1PP/NBBRKR1Q w Dgd - 0 9", 26083252, 5);
// skunk->perft_test_position("nrbb2rq/pppk1ppp/4p1n1/3p4/6P1/1BP5/PP1PPPQP/NRB1KNR1 w GB - 0 9", 18588316, 5);
// skunk->perft_test_position("nrbk1brq/p1ppppp1/7p/1p6/4P1nP/P7/1PPP1PP1/NRBKNBRQ w GBgb - 0 9", 8525056, 5);
// skunk->perft_test_position("nrbk1rqb/1pp2ppp/5n2/p2pp3/5B2/1N1P2P1/PPP1PP1P/1R1KNRQB w FBfb - 0 9", 28465693, 5);
// skunk->perft_test_position("nbrkb1rq/p1pp1ppp/4n3/4p3/Pp6/6N1/1PPPPPPP/NBRKBRQ1 w Cgc - 0 9", 6124625, 5);
// skunk->perft_test_position("nrkb1nrq/p2pp1pp/1pp2p2/7b/6PP/5P2/PPPPP2N/NRKBB1RQ w GBgb - 0 9", 6696458, 5);
// skunk->perft_test_position("nr1nbbr1/pppkpp1p/6p1/3p4/P6P/1P6/1RPPPPP1/N1KNBBRQ w G - 1 9", 7197322, 5);
// skunk->perft_test_position("nrknbrqb/3p1ppp/ppN1p3/8/6P1/8/PPPPPP1P/1RKNBRQB w FBfb - 0 9", 10755190, 5);
// skunk->perft_test_position("nbrkn1bq/p1pppr1p/1p6/5pp1/8/1N2PP2/PPPP2PP/1BKRNRBQ w c - 1 9", 6230616, 5);
// skunk->perft_test_position("nrkbnrbq/ppppppp1/8/8/7p/PP3P2/2PPPRPP/NRKBN1BQ w Bfb - 0 9", 3008668, 5);
// skunk->perft_test_position("nrknrbbq/p4ppp/2p1p3/1p1p4/1P2P3/2P5/P1NP1PPP/1RKNRBBQ w EBeb - 0 9", 18231199, 5);
// skunk->perft_test_position("nrknr1bb/pppp1p2/7p/2qPp1p1/8/1P5P/P1P1PPP1/NRKNRQBB w EBeb - 0 9", 11132758, 5);
// skunk->perft_test_position("bbqnrrkn/ppp2p1p/3pp1p1/8/1PP5/2Q5/P1BPPPPP/B2NRKRN w GE - 0 9", 16764576, 5);
// skunk->perft_test_position("bqn1rkrn/p1p2ppp/1p1p4/4p3/3PP2b/8/PPP2PPP/BQNBRKRN w GEge - 2 9", 16632403, 5);
// skunk->perft_test_position("bqnrkb1n/p1p1pprp/3p4/1p2P1p1/2PP4/8/PP3PPP/BQNRKBRN w GDd - 1 9", 27233018, 5);
// skunk->perft_test_position("bqr1krnb/ppppppp1/7p/3n4/1P4P1/P4N2/2PPPP1P/BQNRKR1B w FDf - 3 9", 18608857, 5);
// skunk->perft_test_position("qbbn1krn/pp3ppp/4r3/2ppp3/P1P4P/8/1P1PPPP1/QBBNRKRN w GEg - 1 9", 18476807, 5);
// skunk->perft_test_position("qnbbrkrn/1p1pp2p/p7/2p2pp1/8/4P2P/PPPP1PPK/QNBBRR1N w ge - 0 9", 10260500, 5);
// skunk->perft_test_position("qnbrkbrn/1ppp2p1/p3p2p/5p2/P4P2/1P6/2PPP1PP/QNBRKBRN w GDgd - 0 9", 11640416, 5);
// skunk->perft_test_position("1nbrkrnb/p1pppp1p/1pq3p1/8/4P3/P1P4N/1P1P1PPP/QNBRKR1B w FDfd - 1 9", 8604788, 5);
// skunk->perft_test_position("qb1r1krn/pppp2pp/1n2ppb1/4P3/7P/8/PPPP1PP1/QBNRBKRN w GDgd - 0 9", 7939483, 5);
// skunk->perft_test_position("qnr1bkrn/p3pppp/1bpp4/1p6/2P2PP1/8/PP1PPN1P/QNRBBKR1 w GCgc - 0 9", 24475596, 5);
// skunk->perft_test_position("1nkrbbrn/qppppppp/8/8/p2P4/1P5P/P1P1PPP1/QNKRBBRN w - - 0 9", 14065717, 5);
// skunk->perft_test_position("1qrkbrnb/ppp1p1pp/n2p4/5p2/4N3/8/PPPPPPPP/Q1RKBRNB w Ffc - 2 9", 14404324, 5);
// skunk->perft_test_position("q1nrkrbn/pp1pppp1/2p4p/8/P7/5Pb1/BPPPPNPP/Q1NRKRB1 w FDfd - 0 9", 8516966, 5);
// skunk->perft_test_position("qnrbkrbn/1p1p1pp1/p1p5/4p2p/8/3P1P2/PPP1P1PP/QNRBKRBN w FCfc - 0 9", 12055174, 5);
// skunk->perft_test_position("qnrkr1bn/p1pp1ppp/8/1p2p3/3P1P2/bP4P1/P1P1P2P/QNRKRBBN w ECec - 1 9", 19939053, 5);
// skunk->perft_test_position("q1krrnbb/p1p1pppp/2np4/1pB5/5P2/8/PPPPP1PP/QNRKRN1B w EC - 0 9", 18110831, 5);
// skunk->perft_test_position("bbn1rkrn/pp1p1ppp/8/2p1p1q1/6P1/P7/BPPPPP1P/B1NQRKRN w GEge - 0 9", 24984621, 5);
// skunk->perft_test_position("bn1brkrn/pp1qpp1p/2p3p1/3p4/1PPP4/P7/4PPPP/BNQBRKRN w GEge - 1 9", 20128587, 5);
// skunk->perft_test_position("b2rkbrn/p1pppppp/qp6/8/1n6/2B2P2/P1PPP1PP/1NQRKBRN w GDgd - 0 9", 20840078, 5);
// skunk->perft_test_position("b2rkrnb/pqp1pppp/n7/1p1p4/P7/N1P2N2/1P1PPPPP/B1QRKR1B w FDfd - 4 9", 16109522, 5);
// skunk->perft_test_position("1bbqrkrn/ppppp1p1/8/5p1p/P1n3P1/3P4/1PP1PP1P/NBBQRRKN w ge - 1 9", 12173245, 5);
// skunk->perft_test_position("nqb1rrkn/ppp1bppp/3pp3/8/3P4/1P6/PQP1PPPP/N1BBRRKN w - - 1 9", 7626054, 5);
// skunk->perft_test_position("nqbrkbr1/p1pppppp/1p6/2N2n2/2P5/5P2/PP1PP1PP/1QBRKBRN w GDgd - 1 9", 15167248, 5);
// skunk->perft_test_position("nqbrkrn1/1ppppp2/6pp/p7/1P6/2Q5/P1PPPPPP/N1BRKRNB w FDfd - 0 9", 13706856, 5);
// skunk->perft_test_position("nbqrbrkn/pp1p1pp1/2p5/4p2p/2P3P1/1P3P2/P2PP2P/NBQRBKRN w GD - 0 9", 16613630, 5);
// skunk->perft_test_position("nqrbbrkn/1p1pppp1/8/p1p4p/4P2P/1N4P1/PPPP1P2/1QRBBKRN w GC - 0 9", 10096863, 5);
// skunk->perft_test_position("nqrkbbrn/2p1p1pp/pp1p1p2/8/P2N4/2P5/1P1PPPPP/1QRKBBRN w GCgc - 0 9", 17597164, 5);
// skunk->perft_test_position("n1krbrnb/q1pppppp/p7/1p6/3Q4/2P2P2/PP1PP1PP/N1RKBRNB w FC - 1 9", 40918952, 5);
// skunk->perft_test_position("nb1rkrbn/p1pp1p1p/qp6/4p1p1/5PP1/P7/1PPPPB1P/NBQRKR1N w FDfd - 2 9", 11911314, 5);
// skunk->perft_test_position("nqr1krbn/pppp1ppp/8/8/3pP3/5P2/PPPb1NPP/NQRBKRB1 w FCfc - 3 9", 612305, 5);
// skunk->perft_test_position("n1rkrbbn/pqppppp1/7p/1p6/8/1NPP4/PP1KPPPP/1QR1RBBN w ec - 0 9", 13421727, 5);
// skunk->perft_test_position("1qrkrnbb/1p1p1ppp/pnp1p3/8/3PP3/P6P/1PP2PP1/NQRKRNBB w ECec - 0 9", 13322502, 5);
// skunk->perft_test_position("1bnrqkrn/2ppppp1/p7/1p1b3p/3PP1P1/8/PPPQ1P1P/BBNR1KRN w GDgd - 1 9", 30458921, 5);
// skunk->perft_test_position("bnrbqkr1/ppp2pp1/6n1/3pp2p/1P6/2N3N1/P1PPPPPP/B1RBQRK1 w gc - 0 9", 14154852, 5);
// skunk->perft_test_position("1nrqkbrn/p1pppppp/8/1p1b4/P6P/5P2/1PPPP1P1/BNRQKBRN w GCgc - 1 9", 6450025, 5);
// skunk->perft_test_position("b1rqkrnb/ppppppp1/8/6p1/3n4/NP6/P1PPPP1P/B1RQKRNB w FCfc - 0 9", 10391021, 5);
// skunk->perft_test_position("nbbrqkrn/ppp3p1/3pp3/5p1p/1P2P3/P7/2PPQPPP/NBBR1KRN w GDgd - 0 9", 22873901, 5);
// skunk->perft_test_position("nr1bqrk1/ppp1pppp/6n1/3pP3/8/5PQb/PPPP2PP/NRBB1KRN w GB - 3 9", 17199594, 5);
// skunk->perft_test_position("1rbqkbr1/ppppp1pp/1n6/4np2/3P1P2/6P1/PPPQP2P/NRB1KBRN w GBgb - 1 9", 13038519, 5);
// skunk->perft_test_position("nr1qkr1b/ppp1pp1p/4bn2/3p2p1/4P3/1Q6/PPPP1PPP/NRB1KRNB w FBfb - 4 9", 30995969, 5);
// skunk->perft_test_position("nb1qbkrn/pprp1pp1/7p/2p1pB2/Q1PP4/8/PP2PPPP/N1R1BKRN w GCg - 2 9", 56747878, 5);
// skunk->perft_test_position("nrqb1rkn/pp2pppp/2bp4/2p5/6P1/2P3N1/PP1PPP1P/NRQBBRK1 w - - 3 9", 19506135, 5);
// skunk->perft_test_position("nrq1bbrn/ppkpp2p/2p3p1/P4p2/8/4P1N1/1PPP1PPP/NRQKBBR1 w GB - 0 9", 8250997, 5);
// skunk->perft_test_position("Br1kbrn1/pqpppp2/8/6pp/3b2P1/1N6/PPPPPP1P/1RQKBRN1 w FBfb - 3 9", 17735648, 5);
// skunk->perft_test_position("nbrqkrbn/2p1p1pp/p7/1p1p1p2/4P1P1/5P2/PPPP3P/NBRQKRBN w FCfc - 0 9", 19192982, 5);
// skunk->perft_test_position("1rqbkrbn/1ppppp1p/1n6/p1N3p1/8/2P4P/PP1PPPP1/1RQBKRBN w FBfb - 0 9", 8652810, 5);
// skunk->perft_test_position("1rqkrbbn/ppnpp1pp/8/2p5/6p1/3P4/PPP1PPPP/NRK1RBBN w eb - 0 9", 6506674, 5);
// skunk->perft_test_position("nrqkrnbb/p1pp2pp/5p2/4P3/2p5/4N3/PP1PP1PP/NRQKR1BB w EBeb - 0 9", 23952941, 5);
// skunk->perft_test_position("bbnrkqrn/pp3pp1/4p2p/2pp4/4P1P1/1PB5/P1PP1P1P/1BNRKQRN w GDgd - 0 9", 29602610, 5);
// skunk->perft_test_position("bnrbkqr1/1p2pppp/6n1/p1pp4/7P/P3P3/1PPPKPP1/BNRB1QRN w gc - 0 9", 5356253, 5);
// skunk->perft_test_position("b1rkqbrn/pp1p2pp/2n1p3/2p2p2/3P2PP/8/PPP1PP2/BNKRQBRN w gc - 0 9", 32684185, 5);
// skunk->perft_test_position("b1rkqrnb/2ppppp1/np6/p6p/1P6/P2P3P/2P1PPP1/BNRKQRNB w FCfc - 0 9", 14561181, 5);
// skunk->perft_test_position("nbbrkqrn/1ppp1p2/p6p/4p1p1/5P2/1P5P/P1PPPNP1/NBBRKQR1 w GDgd - 0 9", 9307003, 5);
// skunk->perft_test_position("nrbbkqrn/p1pppppp/8/1p6/4P3/7Q/PPPP1PPP/NRBBK1RN w GBgb - 0 9", 23091070, 5);
// skunk->perft_test_position("nrbkqbrn/1pppp2p/8/p4pp1/P4PQ1/8/1PPPP1PP/NRBK1BRN w GBgb - 0 9", 8887567, 5);
// skunk->perft_test_position("nr1kqr1b/pp2pppp/5n2/2pp4/P5b1/5P2/1PPPPRPP/NRBK1QNB w Bfb - 2 9", 9465555, 5);
// skunk->perft_test_position("nbkrbqrn/1pppppp1/8/4P2p/pP6/P7/2PP1PPP/NBRKBQRN w GC - 0 9", 4160034, 5);
// skunk->perft_test_position("nrkb1qrn/pp1pp1pp/8/5p1b/P1p4P/6N1/1PPPPPP1/NRKBBQR1 w GBgb - 2 9", 5862341, 5);
// skunk->perft_test_position("1rkq1brn/ppppp1pp/1n6/3b1p2/3N3P/5P2/PPPPP1P1/1RKQBBRN w GBgb - 3 9", 11090645, 5);
// skunk->perft_test_position("nrk1brnb/pp1ppppp/2p5/3q4/5P2/PP6/1KPPP1PP/NR1QBRNB w fb - 1 9", 19318837, 5);
// skunk->perft_test_position("nbrkqr1n/1pppp2p/p4pp1/2Bb4/5P2/6P1/PPPPP2P/NBRKQ1RN w Cfc - 2 9", 20145765, 5);
// skunk->perft_test_position("n1kbqrbn/2p1pppp/1r6/pp1p4/P7/3P4/1PP1PPPP/NRKBQRBN w FBf - 2 9", 10295086, 5);
// skunk->perft_test_position("nrkqrbb1/ppp1pppp/3p4/8/4P3/2Pn1P2/PP4PP/NRKQRBBN w EBeb - 0 9", 2640555, 5);
// skunk->perft_test_position("nrkqrnbb/ppppp1p1/7p/1P3p2/3P4/2P5/P3PPPP/NRKQRNBB w EBeb - 0 9", 16226660, 5);
// skunk->perft_test_position("bbnr1rqn/pp2pkpp/2pp1p2/8/4P1P1/8/PPPP1P1P/BBNRKRQN w FD - 0 9", 6826249, 5);
// skunk->perft_test_position("bnrbk1qn/1pppprpp/8/p4p1P/6P1/3P4/PPP1PP2/BNRBKRQN w FCc - 0 9", 7371098, 5);
// skunk->perft_test_position("1nrkrbqn/p1pp1ppp/4p3/1p6/1PP5/6PB/P2PPPbP/BNRKR1QN w ECec - 0 9", 28412902, 5);
// skunk->perft_test_position("b1rkr1nb/pppppqp1/n4B2/7p/8/1P4P1/P1PPPP1P/1NKRRQNB w ec - 1 9", 30392925, 5);
// skunk->perft_test_position("nbbrkrqn/p1ppp1p1/8/1p3p1p/2P3PP/8/PP1PPPQ1/NBBRKR1N w FDfd - 0 9", 31185844, 5);
// skunk->perft_test_position("1rbbkrqn/ppp1pp2/1n1p2p1/7p/P3P1P1/3P4/1PP2P1P/NRBBKRQN w FBfb - 0 9", 14006203, 5);
// skunk->perft_test_position("nrbkrbq1/Qpppp1pp/2n5/5p2/P4P2/6N1/1PPPP1PP/NRBKRB2 w EBeb - 1 9", 11718463, 5);
// skunk->perft_test_position("1rbkr1nb/pppp1qpp/1n6/4pp2/1PP1P3/8/PB1P1PPP/NR1KRQNB w EBeb - 1 9", 35483796, 5);
// skunk->perft_test_position("nbrk1rqn/p1ppp2p/1p6/5ppb/8/1N2P2P/PPPP1PP1/1BKRBRQN w fc - 0 9", 9329122, 5);
// skunk->perft_test_position("nrkbbrqn/3pppp1/7p/ppp5/P7/1N5P/1PPPPPP1/1RKBBRQN w FBfb - 0 9", 5236331, 5);
// skunk->perft_test_position("nrkr1bqn/ppp1pppp/3p4/1b6/7P/P7/1PPPPPP1/NRKRBBQN w DBdb - 1 9", 5503579, 5);
// skunk->perft_test_position("nrkrbqnb/p4ppp/1p2p3/2pp4/6P1/2P2N2/PPNPPP1P/1RKRBQ1B w DBdb - 0 9", 17883987, 5);
// skunk->perft_test_position("nbkrr1bn/ppB2ppp/4p3/2qp4/4P3/5P2/PPPP2PP/NBRKRQ1N w EC - 1 9", 68070015, 5);
// skunk->perft_test_position("n1kbrqbn/p1pp1pp1/4p2p/2B5/1r3P2/8/PPPPP1PP/NRKBRQ1N w EBe - 2 9", 32318550, 5);
// skunk->perft_test_position("nrkrqbbn/2pppp1p/8/pp6/1P1P2p1/P5P1/2P1PP1P/NRKRQBBN w DBdb - 0 9", 5754555, 5);
// skunk->perft_test_position("nrkr1nbb/1ppp2pp/p3q3/4pp2/2P5/P3P3/1PKP1PPP/NR1RQNBB w db - 0 9", 9905109, 5);
// skunk->perft_test_position("bbnrkrnq/1pp1p2p/6p1/p2p1p2/8/1P2P3/P1PP1PPP/BBNRKRNQ w FDfd - 0 9", 19133881, 5);
// skunk->perft_test_position("bnrbkrn1/pp1ppp2/2p3pp/8/2Pq4/P4PP1/1P1PP2P/BNRBKRNQ w FCfc - 1 9", 13581691, 5);
// skunk->perft_test_position("b1rkrbnq/1pp1pppp/2np4/p5N1/8/1P2P3/P1PP1PPP/BNRKRB1Q w ECec - 0 9", 21156664, 5);
// skunk->perft_test_position("b1krrnqb/pp1ppp1p/n1p3p1/2N5/6P1/8/PPPPPP1P/B1RKRNQB w EC - 0 9", 25360295, 5);
// skunk->perft_test_position("1bbr1rnq/ppppkppp/8/3np3/4P3/3P4/PPP1KPPP/NBBRR1NQ w - - 1 9", 12817011, 5);
// skunk->perft_test_position("nrbbk1nq/p1p1prpp/1p6/N2p1p2/P7/8/1PPPPPPP/R1BBKRNQ w Fb - 2 9", 9236564, 5);
// skunk->perft_test_position("1rbkrb1q/1pppp1pp/1n5n/p4p2/P3P3/1P6/2PPNPPP/NRBKRB1Q w EBeb - 1 9", 5735644, 5);
// skunk->perft_test_position("nrbkr1qb/1pp1pppp/6n1/p2p4/2P1P3/1N4N1/PP1P1PPP/1RBKR1QB w EBeb - 0 9", 14192779, 5);
// skunk->perft_test_position("nbrkbrnq/p3p1pp/1pp2p2/3p4/1PP5/4P3/P1KP1PPP/NBR1BRNQ w fc - 0 9", 14322279, 5);
// skunk->perft_test_position("nrk1brnq/pp1p1pp1/7p/b1p1p3/1P6/6P1/P1PPPPQP/NRKBBRN1 w FBfb - 2 9", 15316285, 5);
// skunk->perft_test_position("nrkr1bnq/1p2pppp/p2p4/1bp5/PP6/1R5N/2PPPPPP/N1KRBB1Q w Ddb - 2 9", 16188945, 5);
// skunk->perft_test_position("nrk1b1qb/pppn1ppp/3rp3/3p4/2P3P1/3P4/PPN1PP1P/1RKRBNQB w DBb - 3 9", 33150360, 5);
// skunk->perft_test_position("nb1rrnbq/ppkp1ppp/8/2p1p3/P7/1N2P3/1PPP1PPP/1BKRRNBQ w - - 1 9", 5506897, 5);
// skunk->perft_test_position("nrkbrnbq/4pppp/1ppp4/p7/2P1P3/3P2N1/PP3PPP/NRKBR1BQ w EBeb - 0 9", 11245508, 5);
// skunk->perft_test_position("nrkrnbbq/3p1ppp/1p6/p1p1p3/3P2P1/P4Q2/1PP1PP1P/NRKRNBB1 w DBdb - 0 9", 22654797, 5);
// skunk->perft_test_position("nr1rnqbb/ppp1pp1p/3k2p1/3p4/1P5P/3P1N2/P1P1PPP1/NRKR1QBB w DB - 1 9", 13890077, 5);
// skunk->perft_test_position("bbqrnnkr/1ppp1p1p/5p2/p5p1/P7/1P4P1/2PPPP1P/1BQRNNKR w HDhd - 0 9", 3588435, 5);
// skunk->perft_test_position("bqrb2k1/pppppppr/5nnp/8/3P1P2/4P1N1/PPP3PP/BQRBN1KR w HCc - 1 9", 11162476, 5);
// skunk->perft_test_position("bqrnn1kr/1pppbppp/8/4p3/1p6/2P1N2P/P2PPPP1/BQR1NBKR w HChc - 1 9", 30126510, 5);
// skunk->perft_test_position("bqr1nkr1/pppppp2/2n3p1/7p/1P1b1P2/8/PQP1P1PP/B1RNNKRB w GCgc - 0 9", 20849374, 5);
// skunk->perft_test_position("qbbrnn1r/1pppp1pk/p7/5p1p/P2P3P/3N4/1PP1PPP1/QBBR1NKR w HD - 0 9", 19494094, 5);
// skunk->perft_test_position("qrbb2kr/p1pppppp/1p1n4/8/1P3n2/P7/Q1PPP1PP/1RBBNNKR w HBhb - 0 9", 27802999, 5);
// skunk->perft_test_position("qrb2bkr/1pp1pppp/2np1n2/pN6/3P4/4B3/PPP1PPPP/QR2NBKR w HBhb - 0 9", 17005916, 5);
// skunk->perft_test_position("qrbnnkrb/pp2pp1p/8/2pp2p1/7P/P1P5/QP1PPPP1/1RBNNKRB w GBgb - 0 9", 19615756, 5);
// skunk->perft_test_position("1brnb1kr/p1pppppp/1p6/8/4q2n/1P2P1P1/PNPP1P1P/QBR1BNKR w HChc - 3 9", 11032633, 5);
// skunk->perft_test_position("1rnbbnkr/1pp1pppp/1q1p4/p7/4P3/5PN1/PPPP1BPP/QRNB2KR w HBhb - 1 9", 20292750, 5);
// skunk->perft_test_position("qrnnbb1Q/ppp1pk1p/3p2p1/5p2/PP6/5P2/2PPP1PP/1RNNBBKR w HB - 0 9", 22443036, 5);
// skunk->perft_test_position("qrnnbkrb/p3p1pp/3p1p2/1pp5/PP2P3/8/2PP1PPP/QRNNBRKB w gb - 0 9", 27658191, 5);
// skunk->perft_test_position("qbrnnkbr/1p2pp1p/p1p3p1/3p4/6P1/P1N4P/1PPPPP2/QBR1NKBR w HChc - 0 9", 14733245, 5);
// skunk->perft_test_position("qr1b1kbr/1p1ppppp/1n1n4/p1p5/4P3/5NPP/PPPP1P2/QRNB1KBR w HBhb - 1 9", 12367604, 5);
// skunk->perft_test_position("qrnnkb1r/1pppppp1/7p/p4b2/4P3/5P1P/PPPP2PR/QRNNKBB1 w Bhb - 1 9", 30307554, 5);
// skunk->perft_test_position("qr1nkrbb/p2ppppp/1pp5/8/3Pn3/1NP3P1/PP2PP1P/QR1NKRBB w FBfb - 1 9", 7046501, 5);
// skunk->perft_test_position("bbrqn1kr/1pppp1pp/4n3/5p2/p5P1/3P4/PPP1PPKP/BBRQNN1R w hc - 0 9", 8191054, 5);
// skunk->perft_test_position("brqb1nkr/pppppp1p/8/4N1pn/5P2/6P1/PPPPP2P/BRQB1NKR w HBhb - 0 9", 8903754, 5);
// skunk->perft_test_position("brqnn1kr/pp3ppp/2pbp3/3p4/8/2NPP3/PPP1BPPP/BRQ1N1KR w HBhb - 0 9", 16243731, 5);
// skunk->perft_test_position("brq1nkrb/ppp2ppp/8/n2pp2P/P7/4P3/1PPP1PP1/BRQNNKRB w GBgb - 1 9", 5048497, 5);
// skunk->perft_test_position("rbbqn1kr/pp2p1pp/6n1/2pp1p2/2P4P/P7/BP1PPPP1/R1BQNNKR w HAha - 0 9", 26302461, 5);
// skunk->perft_test_position("1qbbn1kr/1ppppppp/r3n3/8/p1P5/P7/1P1PPPPP/RQBBNNKR w HAh - 1 9", 22147642, 5);
// skunk->perft_test_position("rqbnnbkr/ppp1ppp1/7p/3p4/PP6/7P/1NPPPPP1/RQB1NBKR w HAa - 1 9", 10416981, 5);
// skunk->perft_test_position("r1bnnkrb/q1ppp1pp/p7/1p3pB1/2P1P3/3P4/PP3PPP/RQ1NNKRB w GAga - 2 9", 26316355, 5);
// skunk->perft_test_position("rbqnb1kr/ppppp1pp/5p2/5N2/7P/1n3P2/PPPPP1P1/RBQNB1KR w HAha - 1 9", 24738875, 5);
// skunk->perft_test_position("rqnbbn1r/ppppppp1/6k1/8/6Pp/2PN4/PP1PPPKP/RQ1BBN1R w - - 0 9", 9714509, 5);
// skunk->perft_test_position("rqnnbbkr/p1p2pp1/1p1p3p/4p3/4NP2/6P1/PPPPP2P/RQN1BBKR w HAha - 0 9", 13307890, 5);
// skunk->perft_test_position("1qnnbrkb/rppp1ppp/p3p3/8/4P3/2PP1P2/PP4PP/RQNNBKRB w GA - 1 9", 7204345, 5);
// skunk->perft_test_position("rbqnn1br/p1pppk1p/1p4p1/5p2/8/P1P2P2/1PBPP1PP/R1QNNKBR w HA - 0 9", 20036784, 5);
// skunk->perft_test_position("rqnbnkbr/1ppppp2/p5p1/8/1P4p1/4PP2/P1PP3P/RQNBNKBR w HAha - 0 9", 16013189, 5);
// skunk->perft_test_position("rq1nkbbr/1p2pppp/p2n4/2pp4/1P4P1/P2N4/2PPPP1P/RQ1NKBBR w HAha - 1 9", 16685687, 5);
// skunk->perft_test_position("r1nnkrbb/pp1pppp1/2p3q1/7p/8/1PPP3P/P3PPP1/RQNNKRBB w FAfa - 1 9", 7508201, 5);
// skunk->perft_test_position("bbrnqk1r/pppp3p/6p1/4pp2/3P2P1/8/PPP1PP1P/BBRN1NKR w HC - 0 9", 8721079, 5);
// skunk->perft_test_position("brnb1nkr/pppqpp2/3p2pp/8/3PP3/1P6/PBP2PPP/1RNBQNKR w HBhb - 0 9", 27734108, 5);
// skunk->perft_test_position("brnq1b1r/ppp1ppkp/3p1np1/8/8/5P1P/PPPPPKPR/BRNQNB2 w - - 0 9", 6372681, 5);
// skunk->perft_test_position("brnq1rkb/1pppppp1/3n3p/p7/8/P4NP1/1PPPPPRP/BRNQ1K1B w B - 0 9", 9015901, 5);
// skunk->perft_test_position("rbb1qnkr/p1ppp1pp/1p3p2/6n1/8/1PN1P2P/P1PP1PP1/RBB1QNKR w HAha - 0 9", 12099119, 5);
// skunk->perft_test_position("rnbb1nkr/1ppp1ppp/4p3/p5q1/6P1/1PP5/PB1PPP1P/RN1BQNKR w HAha - 1 9", 11491355, 5);
// skunk->perft_test_position("rnbqnbkr/1pp1p2p/3p1p2/p5p1/5PP1/2P5/PPNPP2P/RNBQ1BKR w HAha - 0 9", 12649636, 5);
// skunk->perft_test_position("rnb2krb/pppqppnp/8/3p2p1/1P4P1/7P/P1PPPPB1/RNBQNKR1 w GAga - 1 9", 16609220, 5);
// skunk->perft_test_position("rbnqb1kr/pppn1pp1/3p3p/4p3/1P6/P7/R1PPPPPP/1BNQBNKR w Hha - 1 9", 8687621, 5);
// skunk->perft_test_position("rnqb1nkr/p1pbp1pp/8/1pPp1p2/P2P4/8/1P2PPPP/RNQBBNKR w HAha - 1 9", 22592380, 5);
// skunk->perft_test_position("rnq1bbkr/1p1ppp1p/4n3/p1p3p1/P1PP4/8/RP2PPPP/1NQNBBKR w Hha - 0 9", 17597398, 5);
// skunk->perft_test_position("1nqnbkrb/1pppp2p/r7/p4pp1/3P4/8/PPPBPPPP/RNQNK1RB w g - 0 9", 30251988, 5);
// skunk->perft_test_position("rbnqnkbr/p1pp1p1p/8/1p2p3/3P2pP/2P5/PP2PPP1/RBNQNKBR w HAha - 0 9", 24945574, 5);
// skunk->perft_test_position("rnq1nkbr/1p1p1ppp/2p1pb2/p7/7P/2P5/PPNPPPPB/RNQB1K1R w HAha - 2 9", 19919434, 5);
// skunk->perft_test_position("rnqnk1br/p1ppp1bp/1p3p2/6p1/4N3/P5P1/1PPPPP1P/R1QNKBBR w HAha - 2 9", 16525239, 5);
// skunk->perft_test_position("rnq1krbb/p1p1pppp/8/1p1p4/1n5B/2N2P2/PPPPP1PP/RNQ1KR1B w FAfa - 0 9", 21112751, 5);
// skunk->perft_test_position("bbrnnqkr/1pp1pppp/3p4/p7/P3P3/7P/1PPP1PP1/BBRNNQKR w HChc - 0 9", 6196438, 5);
// skunk->perft_test_position("brnbnqkr/p1ppp3/1p5p/5Pp1/5P2/3N4/PPPPP2P/BRNB1QKR w HBhb g6 0 9", 20687969, 5);
// skunk->perft_test_position("br1nqbkr/1ppppp2/pn6/6pp/2PP4/1N4P1/PP2PP1P/BR1NQBKR w HBhb - 0 9", 12185361, 5);
// skunk->perft_test_position("1rnnqkrb/p2ppp1p/1pp5/2N3p1/8/1P6/P1PPPPKP/BR1NQ1RB w gb - 0 9", 32490040, 5);
// skunk->perft_test_position("rbbnnqkr/pp3pp1/2p1p3/3p3p/3P3P/1PP5/P3PPP1/RBBNNQKR w HAha - 0 9", 19885037, 5);
// skunk->perft_test_position("rn1bnqkr/p1ppppp1/8/1p5p/P4P1P/3N4/1PPPP1b1/RNBB1QKR w HAha - 0 9", 18862234, 5);
// skunk->perft_test_position("1nbnqbkr/1p1p1ppp/r3p3/p1p5/P3P3/3Q4/1PPP1PPP/RNBN1BKR w HAh - 2 9", 19648535, 5);
// skunk->perft_test_position("rnbnqkrb/2pppppp/1p6/p7/1PP5/4N2P/P2PPPP1/RNB1QKRB w GAg - 0 9", 10022614, 5);
// skunk->perft_test_position("rbnnbq1r/ppppppkp/6p1/N7/4P3/P7/1PPP1PPP/RB1NBQKR w HA - 5 9", 13909432, 5);
// skunk->perft_test_position("r1nbbqkr/pppppp1p/8/8/1n3Pp1/3N1QP1/PPPPP2P/RN1BB1KR w HAha - 0 9", 22408813, 5);
// skunk->perft_test_position("rnq1bbkr/pp1p1ppp/2pnp3/8/7P/1QP5/PP1PPPPR/RNN1BBK1 w Aha - 2 9", 12242780, 5);
// skunk->perft_test_position("rnnqbrkb/2ppppp1/1p1N4/p6p/4P3/8/PPPP1PPP/R1NQBKRB w GA - 0 9", 14395828, 5);
// skunk->perft_test_position("rbnnq1br/pppp1kp1/4pp2/7p/PP6/2PP4/4PPPP/RBNNQKBR w HA - 0 9", 8239159, 5);
// skunk->perft_test_position("rnnbqkbr/p2ppp2/7p/1pp3p1/2P2N2/8/PP1PPPPP/RN1BQKBR w HAha - 0 9", 9079829, 5);
// skunk->perft_test_position("rnn1kbbr/ppppqp2/6p1/2N1p2p/P7/2P5/1P1PPPPP/RN1QKBBR w HAha - 2 9", 20334071, 5);
// skunk->perft_test_position("rnnqkrbb/p1p1p1pp/1p3p2/8/3p2Q1/P1P1P3/1P1P1PPP/RNN1KRBB w FAfa - 0 9", 32921537, 5);
// skunk->perft_test_position("bbrnk1qr/1pppppp1/p4n1p/8/P2P2N1/8/1PP1PPPP/BBR1NKQR w HC - 1 9", 7015419, 5);
// skunk->perft_test_position("brnbnkqr/1pp1p1p1/p2p1p2/7p/1P4PP/8/PBPPPP2/1RNBNKQR w HBhb - 0 9", 22391185, 5);
// skunk->perft_test_position("br2kbqr/ppppp1pp/3n1p2/3P4/3n3P/3N4/PPP1PPP1/BR1NKBQR w HBhb - 3 9", 20281962, 5);
// skunk->perft_test_position("br1nkqrb/ppppppp1/8/7p/4P3/n1P2PP1/PP1P3P/BRNNKQRB w GBgb - 0 9", 11607818, 5);
// skunk->perft_test_position("rbbn1kqr/pp1pp1p1/2pn3p/5p2/5P2/1P1N4/PNPPP1PP/RBB2KQR w HAha - 1 9", 19239812, 5);
// skunk->perft_test_position("rnbbnk1r/pp1ppp1p/6q1/2p5/PP4p1/4P3/2PP1PPP/RNBBNKQR w HAha - 1 9", 28469879, 5);
// skunk->perft_test_position("rnbnkbqr/1pp3pp/3p4/p3pp2/3P2P1/2N1N3/PPP1PP1P/R1B1KBQR w HAha - 0 9", 36025223, 5);
// skunk->perft_test_position("r1bnkqrb/1ppppppp/p3n3/8/6P1/4N3/PPPPPPRP/RNB1KQ1B w Aga - 1 9", 6666787, 5);
// skunk->perft_test_position("rbn1bkqr/p1pp1pp1/1pn5/4p2p/7P/1PBP4/P1P1PPP1/RBNN1KQR w HAha - 0 9", 6963287, 5);
// skunk->perft_test_position("rnnbbkqr/3ppppp/p7/1pp5/P6P/6P1/1PPPPP2/RNNBBKQR w HAha - 0 9", 11008114, 5);
// skunk->perft_test_position("r1nk1bqr/1pppp1pp/2n5/p4p1b/5P2/1N4B1/PPPPP1PP/RN1K1BQR w HAha - 2 9", 20904119, 5);
// skunk->perft_test_position("r1nkbqrb/p2pppp1/npp4p/8/4PP2/2N4P/PPPP2P1/R1NKBQRB w GAga - 0 9", 11469548, 5);
// skunk->perft_test_position("rbnnkqbr/ppppp2p/5p2/6p1/2P1B3/P6P/1P1PPPP1/R1NNKQBR w HAha - 1 9", 21247414, 5);
// skunk->perft_test_position("1r1bkqbr/pppp1ppp/2nnp3/8/2P5/N4P2/PP1PP1PP/1RNBKQBR w Hh - 0 9", 20188622, 5);
// skunk->perft_test_position("rn1kqbbr/p1pppp1p/1p4p1/1n6/1P2P3/4Q2P/P1PP1PP1/RNNK1BBR w HAha - 1 9", 25594662, 5);
// skunk->perft_test_position("rn1kqrbb/pppppppp/8/8/2nP2P1/1P2P3/P1P2P1P/RNNKQRBB w FAfa - 1 9", 16944425, 5);
// skunk->perft_test_position("b1rnnkrq/bpppppp1/7p/8/1p6/2B5/PNPPPPPP/1BR1NKRQ w GCgc - 2 9", 12865247, 5);
// skunk->perft_test_position("brnb1krq/pppppppp/8/5P2/2P1n2P/8/PP1PP1P1/BRNBNKRQ w GBgb - 1 9", 10776855, 5);
// skunk->perft_test_position("b1nnkbrq/pr1pppp1/1p5p/2p5/P2N1P2/8/1PPPP1PP/BR1NKBRQ w GBg - 0 9", 7370758, 5);
// skunk->perft_test_position("br1nkrqb/p1p1p1pp/3n4/1p1p1p2/5N1P/4P3/PPPP1PP1/BR1NKRQB w FBfb - 0 9", 16429837, 5);
// skunk->perft_test_position("rbbnnkrq/p2pp1pp/2p5/5p2/1pPP1B2/P7/1P2PPPP/RB1NNKRQ w GAga - 0 9", 28095833, 5);
// skunk->perft_test_position("rnbbnkr1/1p1ppp1p/2p3p1/p7/2Pq4/1P1P4/P2BPPPP/RN1BNKRQ w GAga - 2 9", 32825932, 5);
// skunk->perft_test_position("1rbnkbrq/pppppp2/n5pp/2P5/P7/4N3/1P1PPPPP/RNB1KBRQ w GAg - 2 9", 10203438, 5);
// skunk->perft_test_position("1nbnkr1b/rppppppq/p7/7p/1P5P/3P2P1/P1P1PP2/RNBNKRQB w FAf - 1 9", 23266182, 5);
// skunk->perft_test_position("rbn1bkrq/ppppp3/4n2p/5pp1/1PN5/2P5/P2PPPPP/RBN1BKRQ w GAga - 0 9", 23075785, 5);
// skunk->perft_test_position("r1nbbkrq/1ppp2pp/2n2p2/p3p3/5P2/1N4BP/PPPPP1P1/RN1B1KRQ w GAga - 0 9", 16718577, 5);
// skunk->perft_test_position("rnnkbbrq/1pppp1p1/5p2/7p/p6P/3N1P2/PPPPP1PQ/RN1KBBR1 w GAga - 0 9", 15545590, 5);
// skunk->perft_test_position("r1nkbrqb/pppp1p2/n3p1p1/7p/2P2P2/1P6/P2PPQPP/RNNKBR1B w FAfa - 0 9", 18742426, 5);
// skunk->perft_test_position("rbnnkr1q/1ppp2pp/p4p2/P2bp3/4P2P/8/1PPP1PP1/RBNNKRBQ w FAfa - 1 9", 21591790, 5);
// skunk->perft_test_position("rn1bkrb1/1ppppp1p/pn4p1/8/P2q3P/3P4/NPP1PPP1/RN1BKRBQ w FAfa - 1 9", 15847763, 5);
// skunk->perft_test_position("rn1krbbq/pppp1npp/4pp2/8/4P2P/3P2P1/PPP2P2/RNNKRBBQ w EAea - 1 9", 20361517, 5);
// skunk->perft_test_position("rnn1rqbb/ppkp1pp1/2p1p2p/2P5/8/3P1P2/PP2P1PP/RNNKRQBB w EA - 0 9", 7287368, 5);
// skunk->perft_test_position("bbqr1knr/pppppp1p/8/4n1p1/2P1P3/6P1/PPQP1P1P/BB1RNKNR w HDhd - 0 9", 14301029, 5);
// skunk->perft_test_position("bq1bnknr/pprppp1p/8/2p3p1/4PPP1/8/PPPP3P/BQRBNKNR w HCh - 0 9", 9374021, 5);
// skunk->perft_test_position("bqrnkb1r/1p2pppp/p1pp3n/5Q2/2P4P/5N2/PP1PPPP1/B1RNKB1R w HChc - 0 9", 26130444, 5);
// skunk->perft_test_position("bq1rknrb/pppppp1p/4n3/6p1/4P1P1/3P1P2/PPP4P/BQRNKNRB w GCg - 0 9", 10606831, 5);
// skunk->perft_test_position("q1brnknr/pp1pp1p1/8/2p2p1p/5b2/P4N2/1PPPP1PP/QBBRK1NR w hd - 0 9", 12077228, 5);
// skunk->perft_test_position("qrbbnknr/1p1ppp1p/p1p5/8/1P2P1p1/3P1B2/P1P2PPP/QRB1NKNR w HBhb - 0 9", 19584539, 5);
// skunk->perft_test_position("qrb1kbnr/p3pppp/2n5/1ppp4/7P/3P1P2/PPP1P1PR/QRBNKBN1 w Bhb - 0 9", 20500804, 5);
// skunk->perft_test_position("qrbnknrb/ppp1pp2/6p1/7p/PPNp4/8/2PPPPPP/QRB1KNRB w GBgb - 0 9", 24422614, 5);
// skunk->perft_test_position("qbrnbknr/pp1pp1pp/8/2p2p2/3Q4/PP6/2PPPPPP/1BRNBKNR w HChc - 0 9", 41108769, 5);
// skunk->perft_test_position("qr1bbk1r/pppppp1p/1n6/5np1/4B3/1PP5/P2PPPPP/QRN1BKNR w HBhb - 0 9", 12164609, 5);
// skunk->perft_test_position("qrnkbbnr/1p1pp2p/p7/2p1Npp1/6P1/7P/PPPPPP2/QR1KBBNR w HBhb - 0 9", 11409633, 5);
// skunk->perft_test_position("qrnkbnrb/pp1p1p2/2p1p1pp/4N3/P4P2/8/1PPPP1PP/QR1KBNRB w GBgb - 0 9", 15037464, 5);
// skunk->perft_test_position("qbrnknbr/1pppppp1/p6p/8/1P6/3PP3/PQP2PPP/1BRNKNBR w HChc - 3 9", 12214768, 5);
// skunk->perft_test_position("qrnbk1br/1ppppp1p/p5p1/8/4Pn2/4K1P1/PPPP1P1P/QRNB1NBR w hb - 0 9", 8538539, 5);
// skunk->perft_test_position("qrnk1bbr/1pnp1ppp/p1p1p3/8/3Q4/1P1N3P/P1PPPPP1/1RNK1BBR w HBhb - 0 9", 41695761, 5);
// skunk->perft_test_position("qrnknrb1/pppppp2/8/6pp/4P2P/3P1P2/PbP3P1/QRNKNRBB w FBfb - 0 9", 14457245, 5);
// skunk->perft_test_position("bbrqnrk1/ppp2ppp/7n/3pp3/8/P4N1N/1PPPPPPP/BBRQ1RK1 w - - 1 9", 8080951, 5);
// skunk->perft_test_position("brqbnk1r/1ppp1ppp/8/p3pn2/8/2PP1P2/PP2PKPP/BRQBN1NR w hb - 1 9", 15520298, 5);
// skunk->perft_test_position("brqnkbnr/pp2pp1p/3p4/2p5/5p2/3P3P/PPP1PPP1/B1RNKBNR w Hhb - 0 9", 6995034, 5);
// skunk->perft_test_position("brq1kn1b/1ppppprp/2n3p1/p7/P1N5/6P1/1PPPPP1P/BRQNK1RB w GBb - 2 9", 10840256, 5);
// skunk->perft_test_position("rbbq1k1r/ppp1pppp/7n/1n1p4/5P2/P2P4/1PPBP1PP/RB1QNKNR w HAha - 1 9", 17438715, 5);
// skunk->perft_test_position("r1bbnk1r/qpp1pppp/p6n/3p4/1P6/5N1P/P1PPPPP1/RQBBK1NR w ha - 0 9", 16053564, 5);
// skunk->perft_test_position("rqbnkbnr/1pp2p1p/3p4/p3p1p1/8/2P2P2/PP1PPNPP/RQBNKB1R w HAha - 0 9", 19571559, 5);
// skunk->perft_test_position("r1bnknrb/pqppp1p1/1p5p/5p2/7P/3P2N1/PPP1PPP1/RQBNK1RB w GAga - 2 9", 16324542, 5);
// skunk->perft_test_position("rbqnbknr/pp1pppp1/8/2p5/3P3p/5N1P/PPP1PPPR/RBQNBK2 w Aha - 0 9", 26363334, 5);
// skunk->perft_test_position("rqnbbrk1/ppppppp1/8/5n1p/3P3P/2B3P1/PPP1PP2/RQNB1KNR w HA - 0 9", 7055215, 5);
// skunk->perft_test_position("rqnkbbnr/pp2p1p1/8/2pp1p1p/3PPP2/8/PPP1N1PP/RQNKBB1R w HAha - 0 9", 20429246, 5);
// skunk->perft_test_position("rqnkbnr1/pppp2bp/6p1/4pp2/1P2P3/3NN3/P1PP1PPP/RQ1KB1RB w GAga - 0 9", 14038570, 5);
// skunk->perft_test_position("rbq2kbr/pppppppp/2n5/P7/3P1n2/2P5/1P2PPPP/RBQNKNBR w HA - 1 9", 24299415, 5);
// skunk->perft_test_position("rq1bkn1r/ppppp2p/3n4/5pp1/2b3P1/1N1P1P2/PPP1P2P/RQ1BKNBR w HAha - 1 9", 18719949, 5);
// skunk->perft_test_position("r1nknbbr/p2ppp1p/1pp3p1/8/1P6/4P3/P1PPNPPq/R1QKNBBR w HAha - 0 9", 21862776, 5);
// skunk->perft_test_position("rqnknrbb/ppp1p3/5ppp/2Np4/2P5/4P3/PP1P1PPP/RQNK1RBB w FAfa - 0 9", 17664543, 5);
// skunk->perft_test_position("1brnqknr/2p1pppp/p2p4/1P6/6P1/4Nb2/PP1PPP1P/BBR1QKNR w HChc - 1 9", 33322477, 5);
// skunk->perft_test_position("brn1qknr/1p1pppp1/pb5p/Q1p5/3P3P/8/PPP1PPPR/BRNB1KN1 w Bhb - 2 9", 15454749, 5);
// skunk->perft_test_position("brnqkbnr/pppppp2/8/6pp/6P1/P2P1P2/1PP1P2P/BRNQKBNR w HBhb - 0 9", 5770284, 5);
// skunk->perft_test_position("2nqknrb/1rpppppp/5B2/pp6/1PP1b3/3P4/P3PPPP/1RNQKNRB w GBg - 1 9", 38505058, 5);
// skunk->perft_test_position("rb1nqknr/1pp1pppp/8/3p4/p2P4/6PN/PPPQPP1P/RBBN1K1R w HAha - 0 9", 17820605, 5);
// skunk->perft_test_position("rnbbqknr/pppp4/5p2/4p1pp/P7/2N2PP1/1PPPP2P/R1BBQKNR w HAha - 0 9", 10881112, 5);
// skunk->perft_test_position("rn1qkbnr/p1p1pp1p/bp4p1/3p4/1P6/4P3/P1PP1PPP/RNBQKBNR w HAha - 0 9", 21657601, 5);
// skunk->perft_test_position("r1bqk1rb/pppnpppp/5n2/3p4/2P3PP/2N5/PP1PPP2/R1BQKNRB w GAga - 1 9", 24923473, 5);
// skunk->perft_test_position("rbnqbknr/1p1ppp1p/6p1/p1p5/7P/3P4/PPP1PPP1/RBNQBKNR w HAha - 0 9", 15992882, 5);
// skunk->perft_test_position("r1qbbk1r/pp1ppppp/n1p5/5n2/B1P3P1/8/PP1PPP1P/RNQ1BKNR w HAha - 0 9", 19948650, 5);
// skunk->perft_test_position("rnqkbb1r/p1pppppp/8/8/1p4n1/PP4PP/2PPPP2/RNQKBBNR w HAha - 0 9", 6065231, 5);
// skunk->perft_test_position("rnqk1nrb/pppbpp2/7p/3p2p1/4B3/2N1N1P1/PPPPPP1P/R1QKB1R1 w GAga - 0 9", 42109356, 5);
// skunk->perft_test_position("rbnqknbr/1pp1ppp1/3p4/7p/p2P2PP/2P5/PP2PP2/RBNQKNBR w HAha - 0 9", 26632459, 5);
// skunk->perft_test_position("rn1bknbr/pq2pppp/1p6/2pp4/P7/1P1P4/2PNPPPP/RNQBK1BR w HAha - 0 9", 13200921, 5);
// skunk->perft_test_position("r1qk1bbr/ppp1pp1p/2np1n2/6p1/2PP4/3BP3/PP3PPP/RNQKN1BR w HAha - 2 9", 30397368, 5);
// skunk->perft_test_position("r1qknrbb/pppp1p2/2n3p1/4p2p/8/QPP5/P1NPPPPP/RN1K1RBB w FAfa - 2 9", 16813114, 5);
// skunk->perft_test_position("bbkr1qnr/2pppppp/2n5/pp6/8/PPN5/1BPPPPPP/1BR1KQNR w HC - 2 9", 10554668, 5);
// skunk->perft_test_position("1rnbkqnr/1bpppppp/1p6/7P/p2P4/5P2/PPP1P1P1/BRNBKQNR w HBhb - 0 9", 7679979, 5);
// skunk->perft_test_position("brnkqbnr/2p1pppp/1p6/3p4/1pP5/P6P/3PPPP1/BRNKQBNR w HBhb - 0 9", 17354516, 5);
// skunk->perft_test_position("br1kqnrb/npp1pppp/8/3p4/p4N2/PP6/2PPPPPP/BR1KQNRB w GBgb - 0 9", 22376575, 5);
// skunk->perft_test_position("rbbnkq1r/pppppp1p/7n/6p1/P5P1/2P2N2/1P1PPP1P/RBBNKQ1R w HAha - 1 9", 12730970, 5);
// skunk->perft_test_position("rnbbk1nr/pp2qppp/2ppp3/8/3P4/P1N4N/1PP1PPPP/R1BBKQ1R w HAha - 0 9", 21100580, 5);
// skunk->perft_test_position("rnbk1b1r/ppppn1pp/4pp2/7q/7P/P5PB/1PPPPP2/RNBKQ1NR w HAha - 3 9", 14507076, 5);
// skunk->perft_test_position("r2kqnrb/pbppppp1/np5p/8/4Q1P1/3P4/PPP1PP1P/RNBK1NRB w GAga - 2 9", 65239153, 5);
// skunk->perft_test_position("rbnkbq1r/p1p2ppp/1p2pn2/3p4/P3P3/3P4/1PP1KPPP/RBN1BQNR w ha - 2 9", 26202752, 5);
// skunk->perft_test_position("rk1bb1nr/ppppqppp/n7/1N2p3/6P1/7N/PPPPPP1P/R1KBBQ1R w HA - 6 9", 16049807, 5);
// skunk->perft_test_position("rnkqbbnr/p1ppp2p/1p4p1/8/1B3p1P/2NP4/PPP1PPP1/R1KQ1BNR w HAha - 0 9", 14020041, 5);
// skunk->perft_test_position("rnkqb1rb/pp1p1ppp/4p3/2P3n1/8/1PP5/P3PPPP/RNKQBNRB w GAga - 0 9", 17000613, 5);
// skunk->perft_test_position("rb1kqnbr/pp1pp1p1/1np2p2/7p/P1P3PP/8/1P1PPP2/RBNKQNBR w HAha - 0 9", 37415304, 5);
// skunk->perft_test_position("rnkbq1br/ppp2ppp/3p4/Q3p1n1/5P2/3P2P1/PPP1P2P/RNKB1NBR w HAha - 0 9", 52991625, 5);
// skunk->perft_test_position("rn1qnbbr/pp2pppp/2ppk3/8/2PP4/3Q1N2/PP2PPPP/RNK2BBR w HA - 1 9", 15860369, 5);
// skunk->perft_test_position("rnkqnr1b/ppppp1pp/5p2/8/Q1P2P2/8/PP1P2PP/RbK1NRBB w FAfa - 0 9", 29022529, 5);
// skunk->perft_test_position("bbrn1nqr/ppp1k1pp/5p2/3pp3/7P/3PN3/PPP1PPP1/BBRK1NQR w - - 1 9", 10522064, 5);
// skunk->perft_test_position("brnbkn1r/1pppp1p1/4q3/p4p1p/7P/1N3P2/PPPPP1PQ/BR1BKN1R w HBhb - 2 9", 26000648, 5);
// skunk->perft_test_position("br1knbqr/pp2p1pp/1n6/2pp1p2/6P1/2P4B/PP1PPPQP/BRNKN2R w HBhb - 0 9", 14954779, 5);
// skunk->perft_test_position("brnk1qrb/p1ppppp1/1p5p/8/P3n3/1N4P1/1PPPPPRP/BR1KNQ1B w Bgb - 0 9", 9760752, 5);
// skunk->perft_test_position("rbbnknqr/pppp3p/5pp1/8/1P1pP3/7P/P1P2PP1/RBBNKNQR w HAha - 0 9", 17602252, 5);
// skunk->perft_test_position("1nbbknqr/rpp1ppp1/1Q1p3p/p7/2P2PP1/8/PP1PP2P/RNBBKN1R w HAh - 2 9", 33695089, 5);
// skunk->perft_test_position("rnb2bqr/ppkpppp1/3n3p/2p5/6PP/2N2P2/PPPPP3/R1BKNBQR w HA - 2 9", 15115531, 5);
// skunk->perft_test_position("rn1k1qrb/p1pppppp/bp6/8/4n3/P4BPP/1PPPPP2/RNBKNQR1 w GAga - 2 9", 11199653, 5);
// skunk->perft_test_position("rb2bnqr/nppkpppp/3p4/p7/1P6/P2N2P1/2PPPP1P/RB1KBNQR w HA - 3 9", 6831555, 5);
// skunk->perft_test_position("r1kbb1qr/2pppppp/np2n3/p7/2P3P1/8/PP1PPPQP/RNKBBN1R w HAha - 1 9", 19472074, 5);
// skunk->perft_test_position("rnknbb1r/p1ppp1pp/8/1p1P1p1q/8/P1P5/1P2PPPP/RNKNBBQR w HAha - 1 9", 9753617, 5);
// skunk->perft_test_position("rnkn1qrb/pp1bp1pp/2p5/1N1p1p2/8/2P5/PPKPPPPP/R2NBQRB w ga - 2 9", 9206957, 5);
// skunk->perft_test_position("r1nknqbr/pp2p1pp/2p2p2/3p4/6P1/PP1P4/2P1PP1b/RBNKNQBR w HAha - 0 9", 10708639, 5);
// skunk->perft_test_position("rnkb1qbr/p1pp1p1p/1p2pn2/1Q4p1/4P3/N4P2/PPPP2PP/R1KBN1BR w HAha - 0 9", 39145902, 5);
// skunk->perft_test_position("rn2qbbr/1pkppp1p/p3n1p1/8/8/2P2P2/PP1PP1PP/RNKN1BBR w HA - 0 9", 9687507, 5);
// skunk->perft_test_position("rn1nqrbb/p1kppp1p/8/1pp3p1/1P6/2N1P3/P1PP1PPP/RK1NQRBB w - - 0 9", 8436136, 5);
// skunk->perft_test_position("bbrnknrq/1pp3pp/p2p1p2/4p3/P7/1P2N3/2PPPPPP/BBRN1RKQ w gc - 0 9", 9139962, 5);
// skunk->perft_test_position("brnb1nrq/pppp1kpp/4p3/8/5p1P/P1P3P1/1P1PPP2/BRNBKNRQ w GB - 1 9", 20503775, 5);
// skunk->perft_test_position("br1k1brq/ppppp2p/1n1n1pp1/8/P1P5/3P2P1/1P2PP1P/BRNKNBRQ w GBgb - 0 9", 19913758, 5);
// skunk->perft_test_position("1r1knrqb/n1pppppp/p1b5/1p6/8/3N1P2/PPPPP1PP/BRNK1RQB w fb - 3 9", 20044474, 5);
// skunk->perft_test_position("rbbnk1rq/pppppppp/8/3Pn3/8/4P1P1/PPP2P1P/RBBNKNRQ w GAga - 1 9", 8204171, 5);
// skunk->perft_test_position("rnbbk1rq/2pppp1p/p3n1p1/1p6/P3N3/8/1PPPPPPP/RNBB1KRQ w ga - 0 9", 16787080, 5);
// skunk->perft_test_position("rnbkn1rq/ppppppb1/6p1/7p/2B2P2/1P2P3/P1PP2PP/RNBKN1RQ w GAga - 1 9", 20755098, 5);
// skunk->perft_test_position("rn1knrqb/p2pppp1/b1p5/1p5p/2P2P2/1P6/P2PP1PP/RNBKNRQB w FAfa - 1 9", 13257198, 5);
// skunk->perft_test_position("rbnkbnrq/pp2p1Np/2p2p2/8/3p4/8/PPPPPPPP/RBNKBR1Q w Aga - 0 9", 13012378, 5);
// skunk->perft_test_position("rk1bbnrq/ppp1pppp/n7/3p4/5P2/3P2NP/PPP1P1P1/RNKBB1RQ w GA - 0 9", 11269462, 5);
// skunk->perft_test_position("r1knbbrq/pppp2p1/2n1p2p/5p2/4P3/P1PP4/1P3PPP/RNKNBBRQ w GAga - 1 9", 9416862, 5);
// skunk->perft_test_position("rnknbrqb/p1p1pp1p/3p4/1p1N2p1/8/N7/PPPPPPPP/1RK1BRQB w Ffa - 0 9", 15257204, 5);
// skunk->perft_test_position("rbnknrb1/1p1ppp1p/p1p3p1/8/1P3P2/1R6/PqPPP1PP/RBNKN1BQ w Afa - 0 9", 38722152, 5);
// skunk->perft_test_position("rnkbnrbq/2p1ppp1/p7/1p1p3p/3P4/1P4P1/P1P1PP1P/RNKBNRBQ w FAfa - 0 9", 8086100, 5);
// skunk->perft_test_position("r1knrbbq/pp1ppppp/2p1n3/8/2P3P1/P7/1PKPPP1P/RN1NRBBQ w ea - 0 9", 10278695, 5);
// skunk->perft_test_position("rnknrq1b/ppp1p1p1/4b3/3p1p1p/6P1/P4P2/1PPPPQ1P/RNKNR1BB w EAea - 2 9", 19252739, 5);
// skunk->perft_test_position("bbqr1krn/pppp1p1p/5n2/4p1p1/3P4/P3QP2/1PP1P1PP/BB1RNKRN w GDgd - 0 9", 22172123, 5);
// skunk->perft_test_position("bq1b1krn/pp1ppppp/3n4/2r5/3p3N/6N1/PPP1PPPP/BQRB1KR1 w GCg - 2 9", 17546069, 5);
// skunk->perft_test_position("bqrnkbrn/2pp1pp1/p7/1p2p2p/1P6/4N3/P1PPPPPP/BQR1KBRN w GCgc - 0 9", 20059741, 5);
// skunk->perft_test_position("bqr1krnb/1np1pppp/8/pp1p4/8/2P2N2/PP1PPPPP/BQRNKR1B w FCfc - 0 9", 14237097, 5);
// skunk->perft_test_position("qbb1rkrn/1ppppppp/p7/7n/8/P2P4/1PP1PPPP/QBBRNKRN w Gg - 0 9", 8849383, 5);
// skunk->perft_test_position("1rbbnkrn/p1p1pp1p/2q5/1p1p2p1/8/2P3P1/PP1PPP1P/QRBBNKRN w GBgb - 2 9", 24328258, 5);
// skunk->perft_test_position("qrb1kbrn/ppp1p2p/4npp1/3p4/8/1PP4P/PR1PPPP1/Q1BNKBRN w Ggb - 1 9", 5568106, 5);
// skunk->perft_test_position("qr2krnb/p1p1pppp/b1np4/1p6/3NP3/7P/PPPP1PP1/QRBNKR1B w FBfb - 2 9", 12458875, 5);
// skunk->perft_test_position("qbrnbkrn/ppp3pp/3p4/5p2/2P1pP2/6PP/PP1PP3/QBRNBKRN w GCgc - 0 9", 12187382, 5);
// skunk->perft_test_position("qrnb1krn/ppp1p1pp/5p2/2Np4/b2P4/2P5/PP2PPPP/QR1BBKRN w GBgb - 0 9", 12103076, 5);
// skunk->perft_test_position("qrnkbbrn/pp2pp2/8/2pp2pp/6PP/3P4/PPPKPP2/QRN1BBRN w gb - 0 9", 9014737, 5);
// skunk->perft_test_position("qrnkbrnb/p1p1ppp1/1p6/3p4/3P3p/5N1P/PPP1PPP1/QRNKBR1B w FBfb - 0 9", 8295874, 5);
// skunk->perft_test_position("qbr1krbn/1pppp1pp/p7/5pn1/2PP4/8/PPB1PPPP/Q1RNKRBN w FCfc - 0 9", 18961456, 5);
// skunk->perft_test_position("1rnbkrbn/1qp1pppp/3p4/pp6/4P3/1NP4P/PP1P1PP1/QR1BKRBN w FBfb - 0 9", 10832084, 5);
// skunk->perft_test_position("q1rkrbbn/ppp1pppp/8/3p4/1PnP4/P7/1RP1PPPP/Q1NKRBBN w Ee - 1 9", 6452205, 5);
// skunk->perft_test_position("qrnkrn1b/ppppp1pp/4b3/7P/6p1/P7/1PPPPP2/QRNKRNBB w EBeb - 0 9", 10940750, 5);
// skunk->perft_test_position("bbr1nkrn/ppp1pppp/3q4/3p4/8/P7/1PPPPPPP/BBRQNRKN w gc - 5 9", 10870247, 5);
// skunk->perft_test_position("brqbnkrn/pp1pp2p/5pp1/2p5/4P3/P2P1N2/1PP2PPP/BRQB1KRN w GBgb - 0 9", 16391730, 5);
// skunk->perft_test_position("2qnkbrn/p1pppppp/8/1r6/1p2bP2/7N/PPPPP1PP/BR1QKBRN w GBg - 4 9", 14371755, 5);
// skunk->perft_test_position("r1qnkr1b/p1pppppp/7n/1p6/8/1P3b1N/PRPPPPPP/B1QNK1RB w f - 5 9", 12463801, 5);
// skunk->perft_test_position("rbbqn1rn/pppp1pp1/3k4/4p2Q/2PPP3/8/PP3PPP/RBB1NKRN w GA - 1 9", 21852196, 5);
// skunk->perft_test_position("rqbbnkrn/3pppp1/p1p4p/1p6/5P2/P2N4/1PPPP1PP/RQBBK1RN w ga - 0 9", 12794736, 5);
// skunk->perft_test_position("r2nkbrn/pp2pppp/8/2ppqb2/2P3P1/5P2/PP1PPN1P/RQB1KBRN w GAga - 3 9", 34780853, 5);
// skunk->perft_test_position("rqbnk1nb/p1pppr1p/5p2/1p4p1/1PP1P3/8/P2P1PPP/RQBNKRNB w FAa - 1 9", 14565370, 5);
// skunk->perft_test_position("rbqnb1rn/p1pp1kpp/1p2pp2/8/4P2P/P5P1/1PPP1P2/RBQNBKRN w GA - 0 9", 5282124, 5);
// skunk->perft_test_position("rqnbbkrn/p1p1pppp/3p4/1p5B/8/1P1NP3/P1PP1PPP/RQ2BKRN w GAga - 0 9", 12989786, 5);
// skunk->perft_test_position("rqnkbbr1/ppppp1pp/5p2/7n/8/2PNP2P/PP1P1PP1/RQ1KBBRN w GAga - 1 9", 8430874, 5);
// skunk->perft_test_position("r1nkbrnb/2ppppp1/1q6/pp5p/1P6/P3P3/2PPKPPP/RQN1BRNB w fa - 2 9", 19290675, 5);
// skunk->perft_test_position("rbqnkrbn/p1ppppp1/7p/1p6/7P/2N1P3/PPPP1PPB/RBQ1KR1N w FAfa - 1 9", 12976682, 5);
// skunk->perft_test_position("r1nbkrbn/p1qp1ppp/8/1pp1p3/2P1P3/6P1/PP1PBP1P/RQN1KRBN w FAfa - 2 9", 10850952, 5);
// skunk->perft_test_position("rqnkr1bn/ppp1ppb1/3p2pp/8/P7/2P2P2/1PKPP1PP/RQN1RBBN w ea - 1 9", 15661072, 5);
// skunk->perft_test_position("r2krnbb/qppp1ppp/1n6/p3p3/PP6/4N3/N1PPPPPP/RQ1KR1BB w EAea - 4 9", 13837270, 5);
// skunk->perft_test_position("bbr1qk1n/1ppppp1p/2n5/p7/P7/1P2P3/2PP1PrP/1BRNQKRN w GCc - 0 9", 7215306, 5);
// skunk->perft_test_position("brnbq1rn/2ppppkp/p5p1/1p6/8/1BP3P1/PP1PPP1P/BRN1QRKN w - - 0 9", 9929336, 5);
// skunk->perft_test_position("brn1kbrn/pp2p1pp/3p4/q1p2p2/2P4P/6P1/PP1PPP2/BRNQKBRN w GBgb - 1 9", 6720181, 5);
// skunk->perft_test_position("brn1krnb/p3pppp/1qpp4/1p6/2P3P1/1P6/P2PPP1P/BRNQKRNB w FBfb - 1 9", 21806428, 5);
// skunk->perft_test_position("r1b1qkrn/1p1ppppp/p1p1n3/8/4P3/1PN5/P1PPQPPb/RBB2KRN w GAga - 0 9", 22079005, 5);
// skunk->perft_test_position("r1bbqk1n/p1pppprp/n7/1p4p1/5P2/2N3N1/PPPPP1PP/1RBBQKR1 w Ga - 4 9", 10271111, 5);
// skunk->perft_test_position("rnbqkbrn/p1pp1pp1/4p3/7p/2p4P/2P5/PP1PPPP1/R1BQKBRN w GAga - 0 9", 5918310, 5);
// skunk->perft_test_position("rnbqkrnb/1p1pp1p1/2p4p/p4p2/3P2P1/7N/PPPBPP1P/RN1QKR1B w FAfa - 0 9", 21285553, 5);
// skunk->perft_test_position("rbnqbkr1/1ppppp2/p5n1/6pp/4P3/1N6/PPPP1PPP/RBQ1BRKN w ga - 2 9", 6051500, 5);
// skunk->perft_test_position("rnqb1krn/ppppp1p1/7p/7b/P1P2pPP/8/1P1PPP2/RNQBBKRN w GAga - 0 9", 11039042, 5);
// skunk->perft_test_position("rnqkbbr1/p1pp1ppp/4p3/1p6/P3P2n/5P2/1PPP1NPP/RNQKBBR1 w GAga - 2 9", 20666099, 5);
// skunk->perft_test_position("rn1kbrnb/1qppp1pp/1p6/p4p2/1B1P4/1P5N/P1P1PPPP/RNQK1R1B w FAfa - 0 9", 49748034, 5);
// skunk->perft_test_position("rbnqkrbn/Bppp1p2/p5pp/4p3/5P2/6PP/PPPPP3/RBNQKR1N w FAfa - 0 9", 15384362, 5);
// skunk->perft_test_position("rnqbkr1n/1p1ppbpp/3p1p2/p7/8/1P6/P1PPPPPP/R1QBKRBN w FAfa - 0 9", 11843134, 5);
// skunk->perft_test_position("rnqkrb1n/ppppp3/6p1/5p1p/2b2P2/P1N5/1PPPP1PP/RQ1KRBBN w EAea - 1 9", 15379233, 5);
// skunk->perft_test_position("rnqk1nbb/1pp2ppp/3pr3/p3p3/3P1P2/2N3N1/PPP1P1PP/R1QKR1BB w EAa - 1 9", 25144295, 5);
// skunk->perft_test_position("bbr1kqrn/p1p1ppp1/1p2n2p/3p4/1P1P4/2N5/P1P1PPPP/BBR1KQRN w GCgc - 0 9", 6825123, 5);
// skunk->perft_test_position("brnbkq1n/ppp1ppr1/7p/3p2p1/2P3PP/8/PPBPPP2/BRN1KQRN w GBb - 2 9", 13674310, 5);
// skunk->perft_test_position("brnkqbr1/1pppp1pp/5p2/p7/P1P1P2n/8/1P1P1PP1/BRNKQBRN w GBgb - 0 9", 7778289, 5);
// skunk->perft_test_position("b1rkqrnb/p1ppp1pp/1p1n4/5p2/5P2/PN5P/1PPPP1P1/BR1KQRNB w FBf - 0 9", 14228372, 5);
// skunk->perft_test_position("1bbnkqrn/rppppp2/p5p1/7p/7P/P1P1P3/1P1P1PP1/RBBNKQRN w GAg - 1 9", 7752404, 5);
// skunk->perft_test_position("rnbbkqr1/1pppppp1/7p/p3n3/PP5P/8/1BPPPPP1/RN1BKQRN w GAga - 0 9", 7549008, 5);
// skunk->perft_test_position("r1bkqbrn/ppppp1pp/8/5p2/3nPP2/1P4N1/P1PP2PP/RNBKQBR1 w GAga - 1 9", 17989920, 5);
// skunk->perft_test_position("rnbkqr1b/1p1pp1pp/p4p1n/2p5/1P5P/N4P2/P1PPP1P1/R1BKQRNB w FAfa - 0 9", 7808375, 5);
// skunk->perft_test_position("rbnkbqrn/p1p3pp/1p1p4/B3pp2/3P2P1/6N1/PPP1PP1P/RBNK1QR1 w GAga - 0 9", 33318567, 5);
// skunk->perft_test_position("r1kbbqrn/ppp3pp/2np1p2/1P2p3/3P1P2/8/P1P1P1PP/RNKBBQRN w GAga - 0 9", 26763259, 5);
// skunk->perft_test_position("rk1qbbrn/p2npppp/1p6/2p4Q/8/4P3/PPPP1PPP/RNK1B1RN w GA - 2 9", 16662477, 5);
// skunk->perft_test_position("rnk1brnb/pp1p1pp1/8/q1p1p2p/5P2/NP6/P1PPP1PP/R1KQBRNB w FAfa - 1 9", 16987110, 5);
// skunk->perft_test_position("rb1kqrbn/npp1ppp1/p7/3P3p/2PP4/8/PP3PPP/RBNKQRBN w FAfa - 0 9", 23983464, 5);
// skunk->perft_test_position("rnkb1rbn/pp1p2pp/8/2p1pp1q/P6P/1PN5/2PPPPP1/R1KBQRBN w FAfa - 1 9", 21518343, 5);
// skunk->perft_test_position("rnkqrbbn/1pppp1p1/8/p2N1p1p/2P4P/8/PP1PPPP1/R1KQRBBN w EAea - 0 9", 12238776, 5);
// skunk->perft_test_position("rnk1r1bb/pp1ppppp/1q4n1/2p5/5P1P/3PP3/PPP3P1/RNKQRNBB w EAea - 1 9", 23698701, 5);
// skunk->perft_test_position("bbrnkrqn/1ppp1p2/6pp/p3p3/5PP1/2PB4/PP1PP2P/B1RNKRQN w FCfc - 0 9", 20138432, 5);
// skunk->perft_test_position("b1rbkrqn/ppp2ppp/1n2p3/3p4/6P1/2PP4/PP2PP1P/BRNBKRQN w FBf - 1 9", 6307276, 5);
// skunk->perft_test_position("brnkrb1n/1pp1p1pp/3p4/p1Nq1p2/2P5/8/PP1PPPPP/BRK1RBQN w eb - 2 9", 12604078, 5);
// skunk->perft_test_position("brn1r1nb/ppppkppp/4p3/8/2PP1P2/8/PP1KP1PP/BRN1RQNB w - - 1 9", 12290985, 5);
// skunk->perft_test_position("rbb1krqn/1pp1pp1p/p3n1p1/3pP3/8/1PN5/P1PP1PPP/RBB1KRQN w FAfa d6 0 9", 7861413, 5);
// skunk->perft_test_position("r1bbkrqn/p1pppppp/8/4n3/1p5P/P2P2P1/1PP1PP2/RNBBKRQN w FAfa - 0 9", 8699448, 5);
// skunk->perft_test_position("rnbkrbqn/p1pp1ppp/4p3/1p6/8/BPN3P1/P1PPPP1P/R2KRBQN w EAea - 2 9", 14904192, 5);
// skunk->perft_test_position("rnbkrqn1/pppppp2/8/1Q2b1pp/P3P3/5P2/1PPP2PP/RNBKR1NB w EAea - 0 9", 35626426, 5);
// skunk->perft_test_position("rbnkbrqn/p1pppp2/7p/1p4pP/3P1P2/8/PPP1P1P1/RBNKBRQN w FAfa - 0 9", 11859538, 5);
// skunk->perft_test_position("1nkbbrqn/3ppppp/r1p5/pp6/8/4PP2/PPPPN1PP/RNKBBRQ1 w FAf - 2 9", 9556962, 5);
// skunk->perft_test_position("rnkrbbq1/pppppnp1/7p/8/1B1Q1p2/3P1P2/PPP1P1PP/RNKR1B1N w DAda - 2 9", 33185346, 5);
// skunk->perft_test_position("1rkrbqnb/pppppp2/2n3p1/7p/3P3P/P4N2/1PP1PPP1/RNKRBQ1B w DAd - 0 9", 10786140, 5);
// skunk->perft_test_position("rbnkr1bn/pp1pqp1p/2p1p3/6p1/3P4/7P/PPP1PPP1/RBNKRQBN w EAea - 0 9", 9107175, 5);
// skunk->perft_test_position("r1kbrqb1/pppp2pp/2n1p1n1/5p1B/4PP2/P7/1PPP2PP/RNK1RQBN w EAea - 2 9", 73871486, 5);
// skunk->perft_test_position("rnkrqbbn/p1p3pp/1p1ppp2/8/1P6/3P2P1/PKP1PP1P/RN1RQBBN w da - 0 9", 16884013, 5);
// skunk->perft_test_position("rnkrqnbb/ppp2p1p/3p4/4p1p1/3P3P/N1Q5/PPP1PPP1/R1KR1NBB w DAda - 0 9", 52620163, 5);
// skunk->perft_test_position("bbrnkrn1/p1pppp2/1p6/6pp/3q4/1P3QP1/P1PPPP1P/BBRNKRN1 w FCfc - 0 9", 57268492, 5);
// skunk->perft_test_position("br1bkrnq/1p2pppp/pnp5/3p4/P1P5/5P2/1P1PPKPP/BRNB1RNQ w fb - 2 9", 7049659, 5);
// skunk->perft_test_position("brnkrbn1/pppppp1q/B6p/6p1/8/1P2PP2/P1PP2PP/BRNKR1NQ w EBeb - 0 9", 22006883, 5);
// skunk->perft_test_position("br1krnqb/pppppp1p/1n4p1/8/8/P2NN3/2PPPPPP/BR1K1RQB w Beb - 2 9", 36214583, 5);
// skunk->perft_test_position("rbbnkr1q/p1p2ppp/1p1ppn2/8/1PP4P/8/P2PPPP1/RBBNKRNQ w FAfa - 0 9", 18972778, 5);
// skunk->perft_test_position("r1b1krnq/pp2pppp/1bn5/2pp4/4N3/5P2/PPPPPRPP/R1BBK1NQ w Afa - 0 9", 13532966, 5);
// skunk->perft_test_position("1nbkrbn1/rpppppqp/p7/6p1/4P3/3P2P1/PPP1KP1P/RNB1RBNQ w e - 1 9", 21193292, 5);
// skunk->perft_test_position("r1bkrnqb/pp3ppp/n1ppp3/8/1P5P/P7/R1PPPPP1/1NBKRNQB w Eea - 0 9", 7112890, 5);
// skunk->perft_test_position("rbnkbrnq/ppp1p2p/5p2/3p2p1/1B1P4/1N4P1/PPP1PP1P/RB1K1RNQ w FAfa - 0 9", 20756770, 5);
// skunk->perft_test_position("rnk1brnq/pp1ppppp/2p5/b7/8/1P2P2P/P1PP1PPQ/RNKBBRN1 w FAfa - 3 9", 13722785, 5);
// skunk->perft_test_position("rnkrbbnq/p1p3pp/5p2/1p1pp3/P7/1PN2P2/2PPP1PP/R1KRBBNQ w DAda - 0 9", 18916370, 5);
// skunk->perft_test_position("r1krbnqb/p1pp1ppp/2n1p3/8/1p4P1/PPP5/3PPP1P/RNKRBNQB w DAda - 1 9", 9491817, 5);
// skunk->perft_test_position("rbnkrnbq/ppp1pp2/3p2p1/2N5/P6p/2P5/1P1PPPPP/RB1KRNBQ w EAea - 0 9", 20906017, 5);
// skunk->perft_test_position("rnkbrn1q/1ppppppb/8/p4N1p/8/P1N5/1PPPPPPP/R1KBR1BQ w EAea - 0 9", 15308408, 5);
// skunk->perft_test_position("rnkrnbbq/p1p2ppp/3pp3/1p6/6P1/4PQ1B/PPPP1P1P/RNKRN1B1 w DAda - 0 9", 10825379, 5);
// skunk->perft_test_position("rnkrnqbb/pp2p1p1/3p3p/2p2p2/5P2/1P1N4/P1PPPQPP/RNKR2BB w DAda - 0 9", 20522675, 5);
// skunk->perft_test_position("bb1rknnr/ppqppppp/8/2p5/3P1N2/1P6/P1P1PPPP/BBQRKN1R w HDhd - 1 9", 34552118, 5);
// skunk->perft_test_position("bqrbknnr/ppp1p2p/8/3p1p2/5p2/P3N2P/1PPPP1P1/BQRBK1NR w HChc - 0 9", 4834319, 5);
// skunk->perft_test_position("b1rk1bnr/qpp1pppp/p4n2/3p4/3PPP2/7N/PPP3PP/BQRKNB1R w HChc - 1 9", 12200870, 5);
// skunk->perft_test_position("bqkrnnrb/pppp2p1/4pp2/4P2p/6P1/7P/PPPP1P2/BQRKNNRB w GC - 1 9", 8786998, 5);
// skunk->perft_test_position("q1brknnr/1p1ppppp/p7/2p5/8/1PPP4/P2RPPPP/QBB1KNNR w Hhd - 0 9", 7982978, 5);
// skunk->perft_test_position("qrb1k1nr/ppppb1pp/6n1/4ppN1/3P4/4N3/PPP1PPPP/QRBBK2R w HBhb - 2 9", 22493014, 5);
// skunk->perft_test_position("1rbknbnr/1ppp1pp1/q6p/p3p3/5P2/2PPB3/PP2P1PP/QR1KNBNR w HBhb - 0 9", 27484692, 5);
// skunk->perft_test_position("qrbk2rb/1ppp1ppp/5nn1/p3p3/1N6/P7/1PPPPPPP/QRB1KNRB w gb - 0 9", 10098215, 5);
// skunk->perft_test_position("qbrk1nnr/1pp1pppp/2b5/p2p4/P2P2P1/8/1PP1PP1P/QBKRBNNR w hc - 1 9", 13740891, 5);
// skunk->perft_test_position("qrkbbnnr/ppp2p1p/4p3/3p2p1/P7/2PP4/1P2PPPP/QRKBBNNR w HBhb - 0 9", 12079406, 5);
// skunk->perft_test_position("qr1kbbnr/ppp1pp1p/4n1p1/2Pp4/6P1/4N3/PP1PPP1P/QRK1BBNR w HB d6 0 9", 13353359, 5);
// skunk->perft_test_position("qrk1b1rb/p1pppppp/3nnQ2/1p6/1P3P2/3P4/P1P1P1PP/1RKNBNRB w GBgb - 3 9", 71514365, 5);
// skunk->perft_test_position("qbrk1nbr/pppp3p/5n2/4ppp1/3P1P2/4N3/PPP1P1PP/QBKRN1BR w hc - 0 9", 17493373, 5);
// skunk->perft_test_position("qrkb1nbr/1pppppQp/3n4/p7/5p2/1P1N4/P1PPP1PP/1RKB1NBR w HBhb - 0 9", 39736157, 5);
// skunk->perft_test_position("qrk1nbbr/ppp1p1p1/4n2p/3p1p2/1P5P/3P2P1/P1P1PP2/QRKNNBBR w HBhb - 1 9", 21717615, 5);
// skunk->perft_test_position("qrkn1rbb/pp2pppp/2p5/3p4/P2Qn1P1/1P6/2PPPP1P/1RKNNRBB w FBfb - 0 9", 31909835, 5);
// skunk->perft_test_position("bbrqknnr/ppp4p/3pp3/5pp1/4PP2/5Q2/PPPP2PP/BBR1KNNR w HChc - 0 9", 26828059, 5);
// skunk->perft_test_position("1rqbkn1r/p1p1pppp/1p5n/P2p4/3Pb1P1/8/1PP1PP1P/BRQBKNNR w HBhb - 0 9", 17337683, 5);
// skunk->perft_test_position("br1knbnr/1qp1pppp/pp1p4/8/8/PP6/2PPPPPP/BRQKNBNR w HBhb - 2 9", 15280079, 5);
// skunk->perft_test_position("brqk2rb/ppppp1pp/4np2/8/2n5/3P1Q2/PP2PPPP/BR1KNNRB w GBgb - 0 9", 29821322, 5);
// skunk->perft_test_position("r1bqknnr/pp1pp1p1/5p1p/2p1b2N/2P5/8/PPQPPPPP/RBB1K1NR w HAha - 0 9", 22244193, 5);
// skunk->perft_test_position("rqbbknnr/ppppp2p/5pp1/8/8/1P3PP1/PQPPP2P/R1BBKNNR w HAha - 0 9", 5576671, 5);
// skunk->perft_test_position("rqbknbnr/1pp1p2p/p7/3p1pp1/7N/1PP5/P2PPPPP/RQBK1BNR w HAha - 0 9", 15955388, 5);
// skunk->perft_test_position("rqb1nnrb/2ppkppp/1p2p3/p7/2PPP3/1P6/P4PPP/RQBKNNRB w GA - 1 9", 18361051, 5);
// skunk->perft_test_position("rb1kbn1r/p1ppppp1/qp5n/7p/P7/RPP5/3PPPPP/1BQKBNNR w Hha - 2 9", 21279560, 5);
// skunk->perft_test_position("rqkbb1nr/p1p2ppp/1p1p2n1/3Np3/4P3/5N2/PPPP1PPP/RQKBB2R w HAha - 0 9", 16347343, 5);
// skunk->perft_test_position("rqknbbr1/p1pppp1p/1p3np1/8/4P3/2P2P1P/PP1P2P1/RQKNBBNR w HAa - 0 9", 13847463, 5);
// skunk->perft_test_position("r1k1bnrb/1qpppppp/1p2n3/p7/1P5P/6P1/P1PPPP2/RQKNBNR1 w GAga - 1 9", 19382263, 5);
// skunk->perft_test_position("rb1knnbr/1pp1ppp1/p2p3p/5q2/3B2P1/3P1P2/PPP1P2P/RBQKNN1R w HAha - 0 9", 51973672, 5);
// skunk->perft_test_position("rqkb1nbr/p1p1ppp1/1p3n1p/2Qp4/8/2P5/PP1PPPPP/R1KBNNBR w HAha - 2 9", 36347815, 5);
// skunk->perft_test_position("rqknnbbr/2pppp2/pp5p/6p1/1P1P4/4PP2/P1P3PP/RQKNNBBR w HAha - 0 9", 13787303, 5);
// skunk->perft_test_position("rqkn1rbb/1pp1pppp/p7/3p4/3Pn3/2P1PP2/PP4PP/RQKNNRBB w FAfa - 1 9", 8082183, 5);
// skunk->perft_test_position("bbrkqn1r/1pppppp1/5n2/p7/1PP2P1p/7N/P2PP1PP/BBRKQN1R w HChc - 1 9", 35907489, 5);
// skunk->perft_test_position("brkbqn1r/p2ppppp/7n/1p6/P1p3PP/8/1PPPPP1N/BRKBQ1NR w HBhb - 0 9", 8858385, 5);
// skunk->perft_test_position("brkq1bnr/pp1ppp1p/8/2p2np1/P7/8/1PPPPPPP/BRKQNBNR w HBhb - 0 9", 8432183, 5);
// skunk->perft_test_position("brkqnnrb/1ppppppp/8/8/p3P3/5N2/PPPP1PPP/BRKQ1NRB w GBgb - 3 9", 5489836, 5);
// skunk->perft_test_position("rbbkq1nr/1p2pppp/p1p3nB/3p4/1Q1P4/6N1/PPP1PPPP/RB1K2NR w HAha - 0 9", 47425783, 5);
// skunk->perft_test_position("rkbbq1nr/1pppp1p1/4np2/p6p/8/PP3P2/1KPPP1PP/R1BBQNNR w ha - 0 9", 10822049, 5);
// skunk->perft_test_position("r1bqn1nr/pkpppp1p/1p4pb/8/PN6/R7/1PPPPPPP/1KBQ1BNR w H - 2 9", 20919309, 5);
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

