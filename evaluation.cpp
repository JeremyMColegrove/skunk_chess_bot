//
// Evaluation functions for the Skunk chess engine
//

#include "board.h"
#include <cmath>

// Mobility lookup table (6.0f * log(i + 1))
const int mobility_lookup[32] = {
    0, 4, 6, 8, 9, 10, 11, 12, 13, 13, 14, 14, 15, 15, 16, 16,
    17, 17, 17, 17, 18, 18, 18, 19, 19, 19, 19, 20, 20, 20, 20, 20
};

// Passed pawn bonus by rank (index 0 = rank 1/8, index 7 = rank 8/1)
// More advanced passed pawns are exponentially more valuable
const int passed_pawn_bonus[8] = {0, 10, 17, 30, 60, 100, 150, 0};

// Static evaluation weights
float Skunk::evaluation_weights[NUM_WEIGHTS] = {
    15.0f,  // MATERIAL_WEIGHT
    1.0f,   // PIECE_SCORE_WEIGHT
    8.0f,   // DOUBLED_PAWNS_WEIGHT (increased from 4)
    10.0f,  // ISOLATED_PAWNS_WEIGHT (increased from 4)
    1.0f,   // PASSED_PAWN_WEIGHT (now used as multiplier for rank-based bonus)
    6.0f,   // MOBILITY_WEIGHT
    5.0f,   // KING_SAFETY_WEIGHT
    1.0f,   // PAWN_SHIELD_WEIGHT
    1.0f,   // BACKWARD_PAWN_WEIGHT
    0.5f    // SPACE_WEIGHT (scaled down since count can be high)
};

//==============================================================================
// Helper Functions (DRY refactoring)
//==============================================================================

// Unified square occupancy score calculation
int Skunk::calculate_square_occupancy_score_impl(const int* tables[6]) {
    int score = 0;
    U64 bitboard;

    for (int piece = P; piece <= k; piece++) {
        bitboard = bitboards[piece];

        while (bitboard) {
            int square = __builtin_ctzll(bitboard);

            if (piece >= p) {
                score -= tables[piece % 6][mirror_score[square]] * evaluation_weights[PIECE_SCORE_WEIGHT];
            } else {
                score += tables[piece % 6][square] * evaluation_weights[PIECE_SCORE_WEIGHT];
            }

            pop_lsb(bitboard);
        }
    }

    return score;
}

// Calculate king distance from one side
int Skunk::calculate_side_king_distance(int king_square, int start_piece, int end_piece) {
    int total_distance = 0;
    int row_king = king_square >> 3;
    int column_king = king_square & 7;

    for (int piece = start_piece; piece < end_piece; piece++) {
        U64 bitboard = bitboards[piece];
        while (bitboard) {
            int square = __builtin_ctzll(bitboard);
            int row_attacker = square >> 3;
            int column_attacker = square & 7;

            int distance = 16 - (abs(row_king - row_attacker) + abs(column_king - column_attacker));
            total_distance -= distance * king_distance_heuristic[piece % 6];

            pop_lsb(bitboard);
        }
    }

    return total_distance / 10;
}

// Evaluate rooks on open/semi-open files for one side
int Skunk::evaluate_rooks_on_files(int rook_piece, U64 own_pawns, U64 enemy_pawns) {
    int score = 0;
    U64 rooks = bitboards[rook_piece];

    while (rooks) {
        int sq = __builtin_ctzll(rooks);
        int file = sq & 7;
        U64 file_mask = file_masks[MIDDLE][file];

        // Open file (no pawns)
        if (!(file_mask & (own_pawns | enemy_pawns))) {
            score += 25;
        }
        // Semi-open file (no friendly pawns)
        else if (!(file_mask & own_pawns)) {
            score += 15;
        }
        pop_lsb(rooks);
    }

    return score;
}

// Evaluate knight outposts for one side
int Skunk::evaluate_knight_outposts(int knight_piece, int color) {
    int score = 0;
    U64 own_pawns = (color == white) ? bitboards[P] : bitboards[p];
    U64 enemy_pawns = (color == white) ? bitboards[p] : bitboards[P];
    U64 knights = bitboards[knight_piece];

    // Outpost rank range based on color
    int min_rank = (color == white) ? 2 : 3;
    int max_rank = (color == white) ? 4 : 5;

    while (knights) {
        int sq = __builtin_ctzll(knights);
        int rank = sq >> 3;

        if (rank >= min_rank && rank <= max_rank) {
            // Check if protected by own pawn
            int pawn_attack_side = (color == white) ? black : white;
            if (pawn_masks[pawn_attack_side][sq] & own_pawns) {
                // Check if can't be attacked by enemy pawn
                if (!(pawn_attack_span_masks[color][sq] & enemy_pawns)) {
                    score += 25;
                }
            }
        }
        pop_lsb(knights);
    }

    return score;
}

// Evaluate connected rooks for one side
int Skunk::evaluate_connected_rooks(int rook_piece) {
    if (bit_count(bitboards[rook_piece]) != 2) {
        return 0;
    }

    U64 rooks = bitboards[rook_piece];
    int sq1 = __builtin_ctzll(rooks);
    pop_lsb(rooks);
    int sq2 = __builtin_ctzll(rooks);

    // Same file or same rank
    if ((sq1 & 7) == (sq2 & 7) || (sq1 >> 3) == (sq2 >> 3)) {
        return 20;
    }

    return 0;
}

//==============================================================================
// NEW: Advanced Passed Pawn Evaluation
//==============================================================================

int Skunk::evaluate_passed_pawns() {
    int score = 0;
    U64 white_pawns = bitboards[P];
    U64 black_pawns = bitboards[p];
    
    int white_king_sq = __builtin_ctzll(bitboards[K]);
    int black_king_sq = __builtin_ctzll(bitboards[k]);
    
    // Evaluate white passed pawns
    U64 pawns = white_pawns;
    while (pawns) {
        int sq = __builtin_ctzll(pawns);
        
        // Check if passed (no enemy pawns in front or on adjacent files)
        if ((pawn_attack_span_masks[white][sq] & black_pawns) == 0) {
            int rank = 7 - (sq >> 3);  // Convert to rank 0-7 from white's perspective
            
            // Rank-based bonus
            int bonus = passed_pawn_bonus[rank];
            
            // King proximity bonus: own king close = good, enemy king close = bad
            int white_king_dist = abs((white_king_sq >> 3) - (sq >> 3)) + 
                                  abs((white_king_sq & 7) - (sq & 7));
            int black_king_dist = abs((black_king_sq >> 3) - (sq >> 3)) + 
                                  abs((black_king_sq & 7) - (sq & 7));
            
            // Bonus if our king is closer in endgame
            bonus += (black_king_dist - white_king_dist) * 3;
            
            // Free path bonus: no pieces blocking the path ahead
            U64 path_ahead = 0ULL;
            for (int r = sq - 8; r >= 0; r -= 8) {
                path_ahead |= (1ULL << r);
            }
            if (!(path_ahead & (occupancies[white] | occupancies[black]))) {
                bonus += 15;
            }
            
            score += bonus;
        }
        pop_lsb(pawns);
    }
    
    // Evaluate black passed pawns
    pawns = black_pawns;
    while (pawns) {
        int sq = __builtin_ctzll(pawns);
        
        // Check if passed
        if ((pawn_attack_span_masks[black][sq] & white_pawns) == 0) {
            int rank = sq >> 3;  // Rank from black's perspective
            
            // Rank-based bonus
            int bonus = passed_pawn_bonus[rank];
            
            // King proximity
            int white_king_dist = abs((white_king_sq >> 3) - (sq >> 3)) + 
                                  abs((white_king_sq & 7) - (sq & 7));
            int black_king_dist = abs((black_king_sq >> 3) - (sq >> 3)) + 
                                  abs((black_king_sq & 7) - (sq & 7));
            
            bonus += (white_king_dist - black_king_dist) * 3;
            
            // Free path bonus
            U64 path_ahead = 0ULL;
            for (int r = sq + 8; r < 64; r += 8) {
                path_ahead |= (1ULL << r);
            }
            if (!(path_ahead & (occupancies[white] | occupancies[black]))) {
                bonus += 15;
            }
            
            score -= bonus;
        }
        pop_lsb(pawns);
    }
    
    return score * evaluation_weights[PASSED_PAWN_WEIGHT];
}

//==============================================================================
// NEW: Pawn Shield Evaluation for King Safety
//==============================================================================

int Skunk::evaluate_pawn_shield() {
    int score = 0;
    U64 white_pawns = bitboards[P];
    U64 black_pawns = bitboards[p];
    
    int white_king_sq = __builtin_ctzll(bitboards[K]);
    int black_king_sq = __builtin_ctzll(bitboards[k]);
    
    int white_king_file = white_king_sq & 7;
    int black_king_file = black_king_sq & 7;
    
    // White pawn shield (king assumed castled if on g/h or a/b/c files, rank 1)
    int white_king_rank = white_king_sq >> 3;
    if (white_king_rank >= 6) {  // King on 1st or 2nd rank
        // Kingside castle position (files f, g, h)
        if (white_king_file >= 5) {
            for (int f = 5; f <= 7 && f < 8; f++) {
                U64 shield_mask = (1ULL << (48 + f)) | (1ULL << (40 + f));  // 2nd and 3rd rank
                if (white_pawns & (1ULL << (48 + f))) {
                    score += 15;  // Pawn on 2nd rank (best)
                } else if (white_pawns & (1ULL << (40 + f))) {
                    score += 8;   // Pawn on 3rd rank (okay)
                } else {
                    score -= 20;  // Missing shield pawn (bad)
                }
            }
        }
        // Queenside castle position (files a, b, c)
        else if (white_king_file <= 2) {
            for (int f = 0; f <= 2; f++) {
                if (white_pawns & (1ULL << (48 + f))) {
                    score += 15;
                } else if (white_pawns & (1ULL << (40 + f))) {
                    score += 8;
                } else {
                    score -= 20;
                }
            }
        }
    }
    
    // Black pawn shield
    int black_king_rank = black_king_sq >> 3;
    if (black_king_rank <= 1) {  // King on 8th or 7th rank
        if (black_king_file >= 5) {
            for (int f = 5; f <= 7 && f < 8; f++) {
                if (black_pawns & (1ULL << (8 + f))) {
                    score -= 15;
                } else if (black_pawns & (1ULL << (16 + f))) {
                    score -= 8;
                } else {
                    score += 20;
                }
            }
        }
        else if (black_king_file <= 2) {
            for (int f = 0; f <= 2; f++) {
                if (black_pawns & (1ULL << (8 + f))) {
                    score -= 15;
                } else if (black_pawns & (1ULL << (16 + f))) {
                    score -= 8;
                } else {
                    score += 20;
                }
            }
        }
    }
    
    return score * evaluation_weights[PAWN_SHIELD_WEIGHT];
}

//==============================================================================
// NEW: Backward Pawn Evaluation (Simplified)
// A backward pawn is one that cannot be protected by other pawns and
// whose advance is blocked by enemy pawn control
//==============================================================================

int Skunk::evaluate_backward_pawns() {
    int score = 0;
    U64 white_pawns = bitboards[P];
    U64 black_pawns = bitboards[p];
    
    // For white pawns: check if it's the most backward on its file cluster
    // and if advancing it would put it under attack
    U64 pawns = white_pawns;
    while (pawns) {
        int sq = __builtin_ctzll(pawns);
        int file = sq & 7;
        int rank = sq >> 3;
        
        // Only check pawns not too advanced (ranks 6,5,4 = indices 2,3,4 from top)
        if (rank > 2 && rank < 6) {
            // Check if any friendly pawn on adjacent files is behind or at same rank
            bool is_backward = true;
            
            // Check left adjacent file
            if (file > 0) {
                U64 left_pawns = file_masks[MIDDLE][file - 1] & white_pawns;
                while (left_pawns) {
                    int lsq = __builtin_ctzll(left_pawns);
                    if ((lsq >> 3) >= rank) {  // Same rank or behind (higher rank index = closer to rank 1)
                        is_backward = false;
                        break;
                    }
                    pop_lsb(left_pawns);
                }
            }
            
            // Check right adjacent file  
            if (is_backward && file < 7) {
                U64 right_pawns = file_masks[MIDDLE][file + 1] & white_pawns;
                while (right_pawns) {
                    int rsq = __builtin_ctzll(right_pawns);
                    if ((rsq >> 3) >= rank) {
                        is_backward = false;
                        break;
                    }
                    pop_lsb(right_pawns);
                }
            }
            
            // If backward and stop square is controlled by enemy pawn
            if (is_backward) {
                int stop_sq = sq - 8;
                // Check if enemy pawn attacks the stop square
                // pawn_masks[black] gives squares a black pawn on that square attacks
                // We want to check if any black pawn attacks stop_sq
                if (stop_sq >= 0) {
                    // Check if black pawns on adjacent files can attack the stop square
                    bool stop_controlled = false;
                    if (file > 0 && (black_pawns & (1ULL << (stop_sq - 1 - 8)))) stop_controlled = true;
                    if (file < 7 && (black_pawns & (1ULL << (stop_sq + 1 - 8)))) stop_controlled = true;
                    if (stop_controlled) {
                        score -= 8;
                    }
                }
            }
        }
        pop_lsb(pawns);
    }
    
    // For black pawns (mirror logic)
    pawns = black_pawns;
    while (pawns) {
        int sq = __builtin_ctzll(pawns);
        int file = sq & 7;
        int rank = sq >> 3;
        
        if (rank > 1 && rank < 5) {
            bool is_backward = true;
            
            if (file > 0) {
                U64 left_pawns = file_masks[MIDDLE][file - 1] & black_pawns;
                while (left_pawns) {
                    int lsq = __builtin_ctzll(left_pawns);
                    if ((lsq >> 3) <= rank) {
                        is_backward = false;
                        break;
                    }
                    pop_lsb(left_pawns);
                }
            }
            
            if (is_backward && file < 7) {
                U64 right_pawns = file_masks[MIDDLE][file + 1] & black_pawns;
                while (right_pawns) {
                    int rsq = __builtin_ctzll(right_pawns);
                    if ((rsq >> 3) <= rank) {
                        is_backward = false;
                        break;
                    }
                    pop_lsb(right_pawns);
                }
            }
            
            if (is_backward) {
                int stop_sq = sq + 8;
                if (stop_sq < 64) {
                    bool stop_controlled = false;
                    if (file > 0 && (white_pawns & (1ULL << (stop_sq - 1 + 8)))) stop_controlled = true;
                    if (file < 7 && (white_pawns & (1ULL << (stop_sq + 1 + 8)))) stop_controlled = true;
                    if (stop_controlled) {
                        score += 8;  // Penalty for black = bonus for white
                    }
                }
            }
        }
        pop_lsb(pawns);
    }
    
    return score * evaluation_weights[BACKWARD_PAWN_WEIGHT];
}


//==============================================================================
// NEW: Rook on 7th Rank Evaluation
//==============================================================================

int Skunk::evaluate_rook_on_7th() {
    int score = 0;
    
    // Board layout: a8=0, h8=7, a1=56, h1=63
    // 7th rank for white = 2nd rank from top = a7-h7 = squares 8-15
    // 2nd rank for black = 7th rank from top = a2-h2 = squares 48-55
    
    // White rooks on 7th rank (a7-h7, squares 8-15)
    U64 rank_7 = 0x000000000000FF00ULL;
    U64 white_rooks_7th = bitboards[R] & rank_7;
    int white_count = bit_count(white_rooks_7th);
    if (white_count >= 1) {
        // Additional bonus if enemy king or pawns still on 8th rank
        U64 rank_8 = 0x00000000000000FFULL;
        if ((bitboards[k] & rank_8) || (bitboards[p] & rank_7)) {
            score += (white_count == 1) ? 30 : 50;
        } else {
            score += (white_count == 1) ? 15 : 25;  // Reduced if no targets
        }
    }
    
    // Black rooks on 2nd rank (a2-h2, squares 48-55)  
    U64 rank_2 = 0x00FF000000000000ULL;
    U64 black_rooks_2nd = bitboards[r] & rank_2;
    int black_count = bit_count(black_rooks_2nd);
    if (black_count >= 1) {
        U64 rank_1 = 0xFF00000000000000ULL;
        if ((bitboards[K] & rank_1) || (bitboards[P] & rank_2)) {
            score -= (black_count == 1) ? 30 : 50;
        } else {
            score -= (black_count == 1) ? 15 : 25;
        }
    }
    
    return score;
}

//==============================================================================
// NEW: Bishop Outpost Evaluation  
// (Simplified - only give bonus for bishops on strong central squares)
//==============================================================================

int Skunk::evaluate_bishop_outposts() {
    int score = 0;
    
    // Central outpost squares (d4, e4, d5, e5 and neighbors)
    // These are squares 27, 28, 35, 36 (d4, e4, d5, e5)
    U64 central_outposts = 0x0000001818000000ULL;  // d4,e4,d5,e5
    
    // White bishops on strong central squares
    U64 white_bishops_central = bitboards[B] & central_outposts;
    if (white_bishops_central) {
        // Check if protected by pawn and safe from enemy pawns
        while (white_bishops_central) {
            int sq = __builtin_ctzll(white_bishops_central);
            // Only bonus if can't easily be kicked by enemy pawn
            if (!(pawn_attack_span_masks[white][sq] & bitboards[p])) {
                score += 12;
            }
            pop_lsb(white_bishops_central);
        }
    }
    
    // Black bishops on strong central squares
    U64 black_bishops_central = bitboards[b] & central_outposts;
    if (black_bishops_central) {
        while (black_bishops_central) {
            int sq = __builtin_ctzll(black_bishops_central);
            if (!(pawn_attack_span_masks[black][sq] & bitboards[P])) {
                score -= 12;
            }
            pop_lsb(black_bishops_central);
        }
    }
    
    return score;
}

//==============================================================================
// NEW: Space Control Evaluation
//==============================================================================

int Skunk::evaluate_space() {
    int score = 0;
    
    // Count safe squares in enemy territory
    // White controls squares on ranks 5-7 (indices 1-3)
    U64 white_territory = 0x00000000FFFFFF00ULL;  // Ranks 3-6 from white's view
    U64 black_territory = 0x00FFFFFF00000000ULL;  // Ranks 3-6 from black's view
    
    // Get attack masks for all pieces
    U64 white_attacks = 0ULL;
    U64 black_attacks = 0ULL;
    
    // Pawn attacks
    U64 wp = bitboards[P];
    white_attacks |= ((wp & ~0x0101010101010101ULL) >> 9);  // Left attacks
    white_attacks |= ((wp & ~0x8080808080808080ULL) >> 7);  // Right attacks
    
    U64 bp = bitboards[p];
    black_attacks |= ((bp & ~0x8080808080808080ULL) << 9);
    black_attacks |= ((bp & ~0x0101010101010101ULL) << 7);
    
    // Count controlled squares in enemy territory
    int white_space = bit_count(white_attacks & white_territory & ~black_attacks);
    int black_space = bit_count(black_attacks & black_territory & ~white_attacks);
    
    // Bonus for advanced protected pawns (central pawns on 4th-5th rank)
    U64 central_files = 0x3C3C3C3C3C3C3C3CULL;  // Files c-f
    U64 white_advanced = bitboards[P] & central_files & 0x00000000FF000000ULL;  // 4th-5th rank
    U64 black_advanced = bitboards[p] & central_files & 0x000000FF00000000ULL;
    
    white_space += bit_count(white_advanced) * 3;
    black_space += bit_count(black_advanced) * 3;
    
    score = (white_space - black_space);
    
    return score * evaluation_weights[SPACE_WEIGHT];
}

//==============================================================================
// Main Evaluation Functions
//==============================================================================

int Skunk::evaluate() {
    int material_score = 0;
    int white_material_score = 0;
    int black_material_score = 0;
    int square_occupancy_score = 0;
    int square_occupancy_score_endgame = 0;
    int pawn_structure_score = 0;
    int mobility_score = 0;
    int king_safety_score = 0;
    int positional_score = 0;
    float game_phase;

    // Get total material for each side
    for (int piece = P; piece <= K; piece++) {
        white_material_score += bit_count(bitboards[piece]) * piece_scores[piece];
    }
    for (int piece = p; piece <= k; piece++) {
        black_material_score += bit_count(bitboards[piece]) * piece_scores[piece];
    }

    // Calculate core scores
    material_score = calculate_material_score();
    square_occupancy_score = calculate_square_occupancy_score();
    square_occupancy_score_endgame = calculate_square_occupancy_score_endgame();
    pawn_structure_score = calculate_pawn_structure_score();
    mobility_score = calculate_mobility_score();
    king_safety_score = calculate_king_safety_score();

    // ========== POSITIONAL FEATURES ==========

    U64 white_pawns = bitboards[P];
    U64 black_pawns = bitboards[p];

    // Bishop pair bonus (~50 centipawns)
    if (bit_count(bitboards[B]) >= 2) positional_score += 50;
    if (bit_count(bitboards[b]) >= 2) positional_score -= 50;

    // Rook on open/semi-open files (using helper)
    positional_score += evaluate_rooks_on_files(R, white_pawns, black_pawns);
    positional_score -= evaluate_rooks_on_files(r, black_pawns, white_pawns);

    // Connected rooks (using helper)
    positional_score += evaluate_connected_rooks(R);
    positional_score -= evaluate_connected_rooks(r);

    // Knight outposts (using helper)
    positional_score += evaluate_knight_outposts(N, white);
    positional_score -= evaluate_knight_outposts(n, black);

    // NEW: Advanced passed pawn evaluation
    positional_score += evaluate_passed_pawns();
    
    // NEW: Rook on 7th rank (fixed)
    positional_score += evaluate_rook_on_7th();
    
    // NEW: Bishop outposts (fixed - simplified)
    positional_score += evaluate_bishop_outposts();
    
    // NEW: Backward pawns (fixed - simplified)
    positional_score += evaluate_backward_pawns();

    // ========== END POSITIONAL FEATURES ==========

    // Determine the game phase
    game_phase = calculate_game_phase(white_material_score + -black_material_score);

    // NEW: Pawn shield (scaled by game phase - more important in middlegame)
    int pawn_shield_score = evaluate_pawn_shield() * game_phase;
    
    // NEW: Space control (only in opening/middlegame)
    int space_score = evaluate_space() * game_phase;

    // Combine scores considering game phase
    int opening_score = material_score + pawn_structure_score + mobility_score + 
                        king_safety_score + square_occupancy_score + positional_score +
                        pawn_shield_score + space_score;
    int endgame_score = material_score + pawn_structure_score + square_occupancy_score_endgame +
                        positional_score;  // Keep passed pawn etc in endgame

    int score = opening_score * game_phase + endgame_score * (1.0f - game_phase);

    return (side == white ? score : -score);
}

float Skunk::calculate_game_phase(int total_material) {
    int max_material = 2 * (piece_scores[Q] + 2 * piece_scores[R] + 2 * piece_scores[B] + 
                            2 * piece_scores[N] + 8 * piece_scores[P] + piece_scores[K]);
    return (float)total_material / max_material;
}

int Skunk::calculate_material_score() {
    int material_score = 0;

    for (int piece = P; piece <= k; piece++) {
        material_score += piece_count[piece] * piece_scores[piece];
    }

    return material_score;
}

int Skunk::calculate_square_occupancy_score() {
    return calculate_square_occupancy_score_impl(square_scores);
}

int Skunk::calculate_square_occupancy_score_endgame() {
    return calculate_square_occupancy_score_impl(eg_tables);
}

int Skunk::calculate_pawn_structure_score() {
    int pawn_structure_score = 0;
    U64 white_pawns = bitboards[P];
    U64 black_pawns = bitboards[p];

    for (int file = 0; file < 8; file++) {
        U64 white_pawns_on_file = file_masks[MIDDLE][file] & white_pawns;
        U64 black_pawns_on_file = file_masks[MIDDLE][file] & black_pawns;

        // Evaluate doubled pawns
        if (bit_count(white_pawns_on_file) > 1) {
            pawn_structure_score -= evaluation_weights[DOUBLED_PAWNS_WEIGHT] * 
                                    (float)(bit_count(white_pawns_on_file) - 1);
        }
        if (bit_count(black_pawns_on_file) > 1) {
            pawn_structure_score += evaluation_weights[DOUBLED_PAWNS_WEIGHT] * 
                                    (float)(bit_count(black_pawns_on_file) - 1);
        }

        // Evaluate isolated pawns
        bool white_pawns_on_left_file = file > 0 && (file_masks[MIDDLE][file - 1] & bitboards[P]);
        bool white_pawns_on_right_file = file < 7 && (file_masks[MIDDLE][file + 1] & bitboards[P]);
        bool black_pawns_on_left_file = file > 0 && (file_masks[MIDDLE][file - 1] & bitboards[p]);
        bool black_pawns_on_right_file = file < 7 && (file_masks[MIDDLE][file + 1] & bitboards[p]);

        if (white_pawns_on_file && !white_pawns_on_left_file && !white_pawns_on_right_file) {
            pawn_structure_score -= evaluation_weights[ISOLATED_PAWNS_WEIGHT] * bit_count(white_pawns_on_file);
        }
        if (black_pawns_on_file && !black_pawns_on_left_file && !black_pawns_on_right_file) {
            pawn_structure_score += evaluation_weights[ISOLATED_PAWNS_WEIGHT] * bit_count(black_pawns_on_file);
        }
    }

    // Note: Passed pawn evaluation moved to evaluate_passed_pawns() for advanced scoring

    return pawn_structure_score;
}

int Skunk::calculate_mobility_score() {
    int mobility_score = 0;
    int pieces[] = {N, R, B, n, r, b};

    for (int i = 0; i < 6; i++) {
        int piece = pieces[i];
        U64 bitboard = bitboards[piece];
        while (bitboard) {
            int square = __builtin_ctzll(bitboard);

            if (piece >= n) {
                int count = bit_count(get_attacks(piece, square, black));
                if (count > 31) count = 31;
                mobility_score -= mobility_lookup[count];
            } else {
                int count = bit_count(get_attacks(piece, square, white));
                if (count > 31) count = 31;
                mobility_score += mobility_lookup[count];
            }
            pop_lsb(bitboard);
        }
    }

    return mobility_score;
}

int Skunk::calculate_king_safety_score() {
    int king_square_white = __builtin_ctzll(bitboards[K]);
    int king_square_black = __builtin_ctzll(bitboards[k]);

    // Use helper function for DRY (white is attacked by black pieces, vice versa)
    int white_king_safety = calculate_side_king_distance(king_square_white, p, k);
    int black_king_safety = calculate_side_king_distance(king_square_black, P, K);

    return (white_king_safety - black_king_safety) * evaluation_weights[KING_SAFETY_WEIGHT];
}
