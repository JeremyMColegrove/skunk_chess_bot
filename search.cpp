//
// Search functions for the Skunk chess engine
//

#include "board.h"
#include <algorithm>
#include <climits>
#include <cmath>
#include <iostream>

//==============================================================================
// LMR Table (logarithmic reductions based on depth and move count)
//==============================================================================

int LMR_TABLE[64][64];

// Diagnostic counters for search analysis
static int max_ply_reached = 0;
static int check_extensions = 0;
static int lmp_pruned = 0;
static int futility_pruned = 0;
static int see_pruned = 0;
static int razoring_cutoffs = 0;
static int null_move_cutoffs = 0;
static int max_qs_ply = 0;

void init_lmr_table() {
    for (int depth = 1; depth < 64; depth++) {
        for (int moves = 1; moves < 64; moves++) {
            // Standard logarithmic formula (similar to Stockfish)
            LMR_TABLE[depth][moves] = std::max(1, (int)(0.5 + log(depth) * log(moves) / 2.0));
        }
    }
    LMR_TABLE[0][0] = 0;
}

//==============================================================================
// Transposition Table
//==============================================================================

void Skunk::store_transposition_table(U64 zobristKey, int16_t value, int16_t depth, int move, NodeType type) {
    TTEntry *entry = &transpositionTable[zobristKey & (HASH_SIZE - 1)];
    entry->zobristKey = zobristKey;
    entry->value = value;
    entry->depth = depth;
    entry->move = move;
    entry->type = type;
}

TTEntry *Skunk::probe_transposition_table(U64 zobristKey) {
    TTEntry *entry = &transpositionTable[zobristKey & (HASH_SIZE - 1)];
    if (entry->zobristKey == zobristKey) {
        return entry;
    }
    return nullptr;
}

//==============================================================================
// Move Ordering & Heuristics
//==============================================================================

void Skunk::init_heuristics() {
    for (int i = 0; i < MAX_PLY; ++i) {
        killer_moves[0][i] = -1;
        killer_moves[1][i] = -1;
    }
    for (int piece = P; piece <= k; ++piece) {
        for (int square = 0; square < 64; ++square) {
            history_table[piece][square] = 0;
            counter_moves[piece][square] = 0;
        }
    }
    last_move = 0;
}

void Skunk::update_heuristics(int ply, int move, int depth) {
    // Update killer moves
    if (move != killer_moves[0][ply]) {
        killer_moves[1][ply] = killer_moves[0][ply];
        killer_moves[0][ply] = move;
    }

    // Update history table with saturation to prevent overflow
    int piece = decode_piece(move);
    int destination = decode_destination(move);
    int bonus = depth * depth;
    history_table[piece][destination] += bonus;
    if (history_table[piece][destination] > 10000) {
        history_table[piece][destination] = 10000;
    }

    // Update counter move table
    if (last_move != 0) {
        int prev_piece = decode_piece(last_move);
        int prev_dest = decode_destination(last_move);
        counter_moves[prev_piece][prev_dest] = move;
    }
}

int Skunk::score_move(int move) {
    int score = 0;

    // Look for move in previous PV line
    for (int i = 0; i < previous_pv_line.cmove; ++i) {
        if (move == previous_pv_line.argmove[i]) {
            score += 20000 - (i * 100);
        }
    }

    int piece = decode_piece(move);
    int destination = decode_destination(move);
    int victim = get_piece(destination);

    if (decode_promoted(move)) {
        score += 5000;
    }

    if (is_capture(move)) {
        // Handle en passant: victim is a pawn but not on destination square
        if (victim == -1 && decode_enpassant(move)) {
            victim = (side == white) ? p : P;  // Captured pawn
        }
        if (victim >= 0 && victim < 12) {
            score += mvv_lva[piece][victim] * 100;
        }
    }

#ifdef KILLER_HISTORY
    if (move == killer_moves[0][ply]) {
        score += 9000;
    } else if (move == killer_moves[1][ply]) {
        score += 8000;
    }

    if (last_move != 0) {
        int prev_piece = decode_piece(last_move);
        int prev_dest = decode_destination(last_move);
        if (move == counter_moves[prev_piece][prev_dest]) {
            score += 7000;
        }
    }

    score += history_table[piece][decode_destination(move)];
#endif

    return score;
}

void Skunk::sort_moves(int *moves, int num_moves, int tt_move) {
    std::pair<int, int> scored_moves[256];
    for (int i = 0; i < num_moves; i++) {
        int move = moves[i];
        int score = score_move(move);
        if (move == tt_move) score += 30000;
        scored_moves[i] = {score, move};
    }

    std::sort(scored_moves, scored_moves + num_moves, 
              [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
                  return a.first > b.first;
              });

    for (int i = 0; i < num_moves; i++) {
        moves[i] = scored_moves[i].second;
    }
}

//==============================================================================
// Static Exchange Evaluation
//==============================================================================

int Skunk::see(int move) {
    if (!is_capture(move)) {
        return 0;
    }

    int from = decode_source(move);
    int to = decode_destination(move);
    int attacker = get_piece(from);
    int attacker_value = abs(piece_scores[attacker]);
    int victim = get_piece(to);

    if (victim == -1) return 0;

    int victim_value = abs(piece_scores[victim]);

    if (attacker_value <= victim_value) {
        return victim_value - attacker_value;
    }

    copy_board();
    make_move(move, all_moves);

    int recapture = get_smallest_attacker(to);
    int see_value;
    if (recapture == 0) {
        see_value = victim_value;
    } else {
        see_value = victim_value - see(recapture);
    }

    restore_board();
    return see_value;
}

int Skunk::get_smallest_attacker(int to) {
    int piece_order[] = {P, N, B, R, Q, K};

    for (int i = 0; i < 6; i++) {
        int piece = piece_order[i] + (side * 6);
        U64 piece_bb = bitboards[piece];

        while (piece_bb) {
            int from = __builtin_ctzll(piece_bb);
            U64 attacks = get_attacks(piece, from, side);

            if (attacks & (1ULL << to)) {
                return encode_move(from, to, piece, 0, 0, 0);
            }
            pop_lsb(piece_bb);
        }
    }
    return 0;
}

//==============================================================================
// Null Move Pruning
//==============================================================================

bool Skunk::should_do_null_move() {
    int num_major;
    if (side == white) {
        num_major = piece_count[B] + piece_count[R] + piece_count[Q] + piece_count[N];
    } else {
        num_major = piece_count[b] + piece_count[r] + piece_count[q] + piece_count[n];
    }

    if (num_major <= 1) return false;
    return true;
}

//==============================================================================
// Quiescence Search
//==============================================================================

int Skunk::quiesence(int alpha, int beta, int qs_ply) {
    q_nodes++;
    if (force_stop) return 0;

    // Track max quiescence ply reached
    if (ply > max_qs_ply) max_qs_ply = ply;

    // Check detection for move generation context (optional, but useful if we wanted to search evasions)
    bool in_check = is_check();

    // Stand pat
    // We allow stand pat even in check because we only search captures.
    // If we don't stand pat, we implicitly assume that if we have no good captures,
    // the position is bad (alpha), which ignores the possibility of good quiet moves.
    int evaluation = evaluate();

    if (!in_check) {
        if (evaluation >= beta) {
            return evaluation;
        }

        if (evaluation > alpha) {
            alpha = evaluation;
        }
    }

    t_moves moves_list;
    generate_moves(moves_list);

    // If no legal moves
    if (moves_list.count == 0) {
        if (in_check) {
            return -CHECKMATE + ply;
        }
        return evaluation;  // Stalemate
    }

    // Extract only capture moves (even when in check, only search captures for efficiency)
    int captures[64];
    int capture_count = 0;
    for (int i = 0; i < moves_list.count; i++) {
        if (is_capture(moves_list.moves[i])) {
            captures[capture_count++] = moves_list.moves[i];
        }
    }

    // If no captures, and we're in check, we need to search at least one evasion
    // But for now, just return evaluation (risky but fast)
    if (capture_count == 0) {
        if (in_check) {
            // We're in check but no captures - search might be incomplete
            // For safety, return low score but not mate (could miss non-capture evasions)
            return evaluation;
        }
        return evaluation;
    }

    sort_moves(captures, capture_count);

    for (int i = 0; i < capture_count; i++) {
        int move = captures[i];

        // Pruning only applies when NOT in check
        if (!in_check) {
            int victim = get_piece(decode_destination(move));

            // Handle en passant
            if (victim == -1 && decode_enpassant(move)) {
                victim = (side == white) ? p : P;
            }

#ifdef FUTILITY_PRUNE
            // Delta pruning
            if (victim >= 0 && victim < 12) {
                int capture_value = abs(piece_scores[victim]);
                if ((evaluation + capture_value + DELTA_MARGIN < alpha) && decode_promoted(move) == 0) {
                    continue;
                }
            }
#endif

            // SEE pruning
            if (see(move) < 0) {
                continue;
            }
        }

        copy_board();
        make_move(move, only_captures);
        ply++;

        int test = -quiesence(-beta, -alpha, qs_ply + 1);

        ply--;
        restore_board();
        moves--;

        if (test >= beta) {
            return test;
        }

        if (test > alpha) {
            alpha = test;
        }
    }

    return alpha;
}

//==============================================================================
// Negamax Search
//==============================================================================

int Skunk::negamax(int alpha, int beta, int depth, int verify, int do_null, t_line *pline) {
    int best_move = 0, current_move, best_score = -INT_MAX, current_score, null_move_score;
    bool fail_high = false, check = false;
    t_line line = {.cmove = 0};

    nodes++;

    // Track max ply reached
    if (ply > max_ply_reached) max_ply_reached = ply;

    if ((nodes % time_check_node_interval) == 0) {
        communicate();
    }

    if (force_stop) return 0;

    if (depth < 1) {
        if (pline != nullptr) pline->cmove = 0;
        return quiesence(alpha, beta);
    }

    if (ply && is_repetition()) {
        // Contempt factor: deter draws when ahead, encourage when behind
        // Return negative of static evaluation
        return -evaluate();
    }

    // Transposition table lookup
    TTEntry *entry = probe_transposition_table(zobrist);
    if (entry != nullptr && !verify) {
        if (entry->depth >= depth) {
            if (entry->type == EXACT) {
                cache_hit++;
                if (ply == 0 && pline != nullptr) {
                    pline->argmove[0] = entry->move;
                    pline->cmove = 1;
                }
                return entry->value;
            } else if (entry->type == LOWER_BOUND) {
                alpha = std::max(alpha, (entry->value));
            } else if (entry->type == UPPER_BOUND) {
                beta = std::min(beta, (entry->value));
            }
            if (alpha >= beta) {
                cache_hit++;
                return entry->value;
            }
        }
    }

    t_moves moves_list;
    generate_moves(moves_list);

    check = is_check();

    if (moves_list.count == 0) {
        return check ? (-CHECKMATE) + ply : 0;
    }

    // Check extensions (always extend in check)
    if (check) {
        depth++;
        check_extensions++;
    }

    int tt_move = (entry != nullptr) ? entry->move : 0;
    bool is_pv_node = (beta - alpha > 1);

    // Static evaluation (cached for multiple pruning techniques)
    int static_eval = evaluate();

    // Razoring: at low depths, if static eval is far below alpha, verify with qsearch
    if (!check && !is_pv_node && depth <= 3 && !verify) {
        if (static_eval + RAZORING_MARGIN[depth] < alpha) {
            int razor_score = quiesence(alpha, beta);
            if (razor_score < alpha) {
                razoring_cutoffs++;
                return razor_score;
            }
        }
    }

    // Reverse futility pruning (static null move pruning)
    if (!check && !is_pv_node && depth <= 4 && !verify) {
        int margin = 100 * depth;
        if (static_eval - margin >= beta) {
            return static_eval - margin;
        }
    }

    // Internal Iterative Deepening
    if (tt_move == 0 && depth >= 4 && is_pv_node) {
        t_line iid_line = {.cmove = 0};
        negamax(alpha, beta, depth - 2, verify, DO_NULL, &iid_line);
        if (iid_line.cmove > 0) {
            tt_move = iid_line.argmove[0];
        }
    }

    sort_moves(moves_list.moves, moves_list.count, tt_move);

    // Null move pruning with adaptive reduction
    if (!check && do_null == DO_NULL && (!verify || depth > 1) && should_do_null_move()) {
        copy_board();
        side ^= 1;
        zobrist ^= side_key;
        if (enpassant != no_square) {
            zobrist ^= enpassant_keys[enpassant];
        }
        enpassant = no_square;

        // Adaptive null move reduction: R = 3 + depth/6, capped at depth/2 for deep searches
        int null_r = 3 + depth / 6;
        if (depth > 6) null_r = std::min(null_r, depth / 2);

        ply++;
        null_move_score = -negamax(-beta, -beta + 1, depth - 1 - null_r, verify, NO_NULL, nullptr);
        ply--;
        restore_board();

        if (null_move_score >= beta) {
            if (verify) {
                depth--;
                verify = false;
                fail_high = true;
            } else {
                null_move_cutoffs++;
                return null_move_score;
            }
        }
    }

    copy_board();
    int searched_moves = 0;
    bool is_quiet_move;

    for (int i = 0; i < moves_list.count; i++) {
        current_move = moves_list.moves[i];
        is_quiet_move = !is_capture(current_move) && decode_promoted(current_move) == 0;

        // Late Move Pruning
        if (!check && !is_pv_node && depth <= 7 && searched_moves >= LMP_THRESHOLD[depth] && is_quiet_move) {
            lmp_pruned++;
            continue;
        }

        // Futility Pruning for quiet moves at low depths
        if (!check && !is_pv_node && depth <= 4 && is_quiet_move && searched_moves > 0) {
            if (static_eval + FUTILITY_MARGIN[depth] <= alpha) {
                futility_pruned++;
                continue;
            }
        }

        // SEE pruning with depth-dependent threshold (more aggressive at higher depths)
        if (is_capture(current_move) && searched_moves > 0 && !is_pv_node && depth <= 6) {
            int see_threshold = -20 * depth * depth;  // Deeper = allow worse captures
            if (see(current_move) < see_threshold) {
                see_pruned++;
                continue;
            }
        }

        int prev_last_move = last_move;
        last_move = current_move;

        make_move(current_move, all_moves);
        ply++;
        repitition.table[repitition.count++] = zobrist;

    re_search:
        if (searched_moves == 0) {
            current_score = -negamax(-beta, -alpha, depth - 1, verify, DO_NULL, &line);
        } else {
            if (searched_moves >= LMR_DEPTH && depth >= LMR_MIN_DEPTH && !check && 
                !is_capture(current_move) && decode_promoted(current_move) == 0) {
                // Get base reduction from precomputed LMR table
                int reduction = LMR_TABLE[std::min(depth, 63)][std::min(searched_moves, 63)];

                // Adjust based on history: reduce more for moves with bad history
                int piece = decode_piece(current_move);
                int dest = decode_destination(current_move);
                int history = history_table[piece][dest];
                if (history < 0) reduction++;
                if (history > 5000) reduction = std::max(0, reduction - 1);

                // Don't reduce into qsearch, ensure at least 1 ply remains
                reduction = std::min(reduction, depth - 2);
                if (reduction < 1) reduction = 1;

                current_score = -negamax(-alpha - 1, -alpha, depth - 1 - reduction, verify, NO_NULL, &line);

                if (current_score > alpha && current_score < beta) {
                    current_score = -negamax(-beta, -alpha, depth - 1, verify, DO_NULL, &line);
                }
            } else {
                current_score = -negamax(-alpha - 1, -alpha, depth - 1, verify, DO_NULL, &line);

                if (current_score > alpha && current_score < beta) {
                    current_score = -negamax(-beta, -alpha, depth - 1, verify, DO_NULL, &line);
                }
            }
        }

        if (current_score > best_score) {
            best_score = current_score;
            best_move = current_move;
        }

        if (fail_high && current_score < beta) {
            depth++;
            fail_high = false;
            verify = true;
            goto re_search;
        }

        restore_board();
        last_move = prev_last_move;
        ply--;
        repitition.count--;
        moves--;
        searched_moves++;

        if (best_score > alpha) {
            alpha = best_score;
            if (pline != nullptr) {
                pline->argmove[0] = best_move;
                memcpy(pline->argmove + 1, line.argmove, line.cmove * sizeof(int));
                pline->cmove = line.cmove + 1;
            }

            if (alpha >= beta) {
#ifdef KILLER_HISTORY
                if (!is_capture(current_move) && ply < MAX_PLY) {
                    update_heuristics(ply, current_move, depth);
                }
#endif

#ifdef TRANSPOSITION_TABLE
                store_transposition_table(zobrist, beta, depth, best_move, LOWER_BOUND);
#endif
                return beta;
            }
        }
    }

#ifdef TRANSPOSITION_TABLE
    NodeType type;
    if (best_score <= alpha) {
        type = UPPER_BOUND;
    } else if (best_score >= beta) {
        type = LOWER_BOUND;
    } else {
        type = EXACT;
    }
    store_transposition_table(zobrist, best_score, depth, best_move, type);
#endif

    return best_score;
}

//==============================================================================
// Top-level Search
//==============================================================================

int Skunk::search(int maxDepth) {
    t_line pline = {.cmove = 0};

    start_time = std::chrono::steady_clock::now();
    force_stop = 0;

    int score = 0, result;
    cache_hit = 0;
    q_nodes = 0;
    nodes = 0;
    null_move_pruned = 0;

    int alpha = -INT_MAX, beta = INT_MAX;

    init_heuristics();

    for (int depth = 0; depth < maxDepth; depth++) {
        ply = 0;
        pline.cmove = 0;

        // Reset diagnostic counters for this iteration
        max_ply_reached = 0;
        max_qs_ply = 0;
        check_extensions = 0;
        lmp_pruned = 0;
        futility_pruned = 0;
        see_pruned = 0;
        razoring_cutoffs = 0;
        null_move_cutoffs = 0;

        // Aspiration windows
        if (depth >= 4) {
            alpha = score - ASPIRATION_WINDOW;
            beta = score + ASPIRATION_WINDOW;
        }

        score = negamax(alpha, beta, depth + 1, 1, DO_NULL, &pline);

        // Re-search with full window on fail
        if (!force_stop && (score <= alpha || score >= beta)) {
            alpha = -INT_MAX;
            beta = INT_MAX;
            score = negamax(alpha, beta, depth + 1, 1, DO_NULL, &pline);
        }

        if (force_stop) break;

        // Print info
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_time).count();
        long nps = (elapsed > 0) ? (nodes * 1000 / elapsed) : nodes;

        if (score < -CHECKMATE + 2000) {
            int mate_in = (score + CHECKMATE + 1) / 2;
            std::cout << "info depth " << depth + 1 << " score mate " << -mate_in
                      << " nodes " << nodes << " nps " << nps << " time " << elapsed << " pv ";
        } else if (score > CHECKMATE - 2000) {
            int mate_in = (CHECKMATE - score + 1) / 2;
            std::cout << "info depth " << depth + 1 << " score mate " << mate_in
                      << " nodes " << nodes << " nps " << nps << " time " << elapsed << " pv ";
        } else {
            std::cout << "info depth " << depth + 1 << " score cp " << score
                      << " nodes " << nodes << " nps " << nps << " time " << elapsed << " pv ";
        }

        memcpy(&previous_pv_line, &pline, sizeof(t_line));

        for (int i = 0; i < previous_pv_line.cmove; i++) {
            print_move(previous_pv_line.argmove[i]);
            printf(" ");
        }
        std::cout << std::endl;

        // Print diagnostic info as UCI info string
        std::cout << "info string diag: maxPly=" << max_ply_reached 
                  << " maxQsPly=" << max_qs_ply
                  << " qNodes=" << q_nodes
                  << " chkExt=" << check_extensions
                  << " nullCut=" << null_move_cutoffs
                  << " lmpPrune=" << lmp_pruned
                  << " futPrune=" << futility_pruned
                  << " seePrune=" << see_pruned
                  << " razorCut=" << razoring_cutoffs
                  << " ttHit=" << cache_hit
                  << std::endl;
        std::cout.flush();
    }

    if (previous_pv_line.cmove > 0) {
        printf("bestmove ");
        print_move(previous_pv_line.argmove[0]);
        printf("\n");
    } else {
        printf("bestmove 0000\n");
    }
    fflush(stdout);

    return pline.argmove[0];
}
