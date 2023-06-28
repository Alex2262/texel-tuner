//
// Created by Alex Tian on 2/24/2023.
//

#include "altair.h"
#include <iostream>
#include <array>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>

using namespace std;
using namespace Altair;

//
// Created by Alex Tian on 9/29/2022.
//



template <typename Out>
void split(const std::string &s, char delim, Out result) {
    std::istringstream iss(s);
    std::string item;
    while (std::getline(iss, item, delim)) {
        *result++ = item;
    }
}


std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}


int piece_to_num(char piece) {

    auto it = std::find(std::begin(PIECE_MATCHER), std::end(PIECE_MATCHER), piece);
    return it - std::begin(PIECE_MATCHER);
}

bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

class Position {

public:

    Position() = default;

    PLY_TYPE set_fen(const std::string& fen_string);

    PIECE_TYPE board[120]{};
    SQUARE_TYPE piece_list_index[120] = {NO_PIECE_INDEX};

    std::vector<PIECE_TYPE> white_pieces;
    std::vector<PIECE_TYPE> black_pieces;

    SQUARE_TYPE king_positions[2]{};

    SQUARE_TYPE pawn_rank[2][10]{};

    SQUARE_TYPE castle_ability_bits = 0;
    SQUARE_TYPE ep_square = 0;
    SQUARE_TYPE side = 0;

    inline void update_piece_list_index(SQUARE_TYPE left_range, SQUARE_TYPE right_range, SQUARE_TYPE side_to_use) {
        if (!side_to_use) {
            for (int i = left_range; i < right_range; i++) {
                piece_list_index[white_pieces[i]] = i;
            }
        } else {
            for (int i = left_range; i < right_range; i++) {
                piece_list_index[black_pieces[i]] = i;
            }
        }
    }


};


PLY_TYPE Position::set_fen(const std::string& fen_string) {

    std::vector<std::string> fen_tokens = split(fen_string, ' ');

    // Setting the padding around the 8x8 board inscribed within the 10x12 board
    SQUARE_TYPE pos = 0;

    white_pieces.clear();
    black_pieces.clear();

    for (SQUARE_TYPE& i : piece_list_index) {
        i = NO_PIECE_INDEX;
    }

    for (int i = 0; i < 21; i++) {
        board[pos++] = PADDING;
    }

    // Parsing the main 8x8 board part while adding appropriate padding
    for (char c : fen_tokens[0]) {

        if (c == '/') {

            board[pos++] = PADDING;
            board[pos++] = PADDING;

        }
        else if (std::isdigit(c)) {

            for (int empty_amt = 0; empty_amt < c - '0'; empty_amt++) {
                board[pos++] = EMPTY;
            }

        }
        else if (std::isalpha(c)) {

            board[pos] = piece_to_num(c);

            if (std::isupper(c)) {
                white_pieces.push_back(pos);
                piece_list_index[pos] = white_pieces.size() - 1;
            }
            else {
                black_pieces.push_back(pos);
                piece_list_index[pos] = black_pieces.size() - 1;
            }

            if (c == 'K') king_positions[0] = pos;
            else if (c == 'k') king_positions[1] = pos;

            pos++;

        }
    }

    for (int i = 0; i < 21; i++) {
        board[pos++] = PADDING;
    }

    castle_ability_bits = 0;
    for (char c : fen_tokens[2]) {

        if (c == 'K') castle_ability_bits |= 1;
        else if (c == 'Q') castle_ability_bits |= 2;
        else if (c == 'k') castle_ability_bits |= 4;
        else if (c == 'q') castle_ability_bits |= 8;

    }

    if (fen_tokens[3].size() > 1) {
        SQUARE_TYPE square = (8 - (fen_tokens[3][1] - '0')) * 8 + fen_tokens[3][0] - 'a';
        ep_square = STANDARD_TO_MAILBOX[square];
    }
    else {
        ep_square = 0;
    }

    side = 0;
    if (fen_tokens[1] == "b") side = 1;

    if (fen_tokens.size() >= 5) {
        if (!is_number(fen_tokens[4])) return 0;
        return static_cast<PLY_TYPE>(std::stoi(fen_tokens[4]));
    }
    else return 0;
}


// ------------------------------------------- EVALUATION -----------------------------------------------------



void evaluate_king_pawn(const Position& position, Score_Struct& scores, SQUARE_TYPE file, bool is_white, Trace& trace) {
    SQUARE_TYPE col = file - 1;
    if (is_white) {
        if (position.pawn_rank[0][file] == 3) {
            scores.mid += KING_PAWN_SHIELD_OWN_PENALTIES_MID[0][col];  // Pawn moved one square
            scores.end += KING_PAWN_SHIELD_OWN_PENALTIES_END[0][col];  // Pawn moved one square
            trace.own_king_pawn_shield[0][col][WHITE_COLOR]++;
        }
        else if (position.pawn_rank[0][file] == 4) {
            scores.mid += KING_PAWN_SHIELD_OWN_PENALTIES_MID[1][col];  // Pawn moved two squares
            scores.end += KING_PAWN_SHIELD_OWN_PENALTIES_END[1][col];  // Pawn moved two squares
            trace.own_king_pawn_shield[1][col][WHITE_COLOR]++;
        }
        else if (position.pawn_rank[0][file] != 2) {
            scores.mid += KING_PAWN_SHIELD_OWN_PENALTIES_MID[2][col];  // Pawn moved > 2 squares, or it doesn't exist
            scores.end += KING_PAWN_SHIELD_OWN_PENALTIES_END[2][col];  // Pawn moved > 2 squares, or it doesn't exist
            trace.own_king_pawn_shield[2][col][WHITE_COLOR]++;
        }

        if (position.pawn_rank[1][file] == 0) {
            scores.mid += KING_PAWN_SHIELD_OPP_PENALTIES_MID[0][col];
            scores.end += KING_PAWN_SHIELD_OPP_PENALTIES_END[0][col];
            trace.opp_king_pawn_shield[0][col][WHITE_COLOR]++;
        }
        else if (position.pawn_rank[1][file] == 4) {
            scores.mid += KING_PAWN_SHIELD_OPP_PENALTIES_MID[1][col];  // Enemy pawn is on the 4th rank
            scores.end += KING_PAWN_SHIELD_OPP_PENALTIES_END[1][col];  // Enemy pawn is on the 4th rank
            trace.opp_king_pawn_shield[1][col][WHITE_COLOR]++;
        }
        else if (position.pawn_rank[1][file] == 3) {
            scores.mid += KING_PAWN_SHIELD_OPP_PENALTIES_MID[2][col];  // Enemy pawn is on the 3rd rank
            scores.end += KING_PAWN_SHIELD_OPP_PENALTIES_END[2][col];  // Enemy pawn is on the 3rd rank
            trace.opp_king_pawn_shield[2][col][WHITE_COLOR]++;
        }
    }

    else {
        if (position.pawn_rank[1][file] == 6) {
            scores.mid += KING_PAWN_SHIELD_OWN_PENALTIES_MID[0][col];  // Pawn moved one square
            scores.end += KING_PAWN_SHIELD_OWN_PENALTIES_END[0][col];  // Pawn moved one square
            trace.own_king_pawn_shield[0][col][BLACK_COLOR]++;
        }
        else if (position.pawn_rank[1][file] == 5) {
            scores.mid += KING_PAWN_SHIELD_OWN_PENALTIES_MID[1][col];  // Pawn moved two squares
            scores.end += KING_PAWN_SHIELD_OWN_PENALTIES_END[1][col];  // Pawn moved two squares
            trace.own_king_pawn_shield[1][col][BLACK_COLOR]++;
        }
        else if (position.pawn_rank[1][file] != 7) {
            scores.mid += KING_PAWN_SHIELD_OWN_PENALTIES_MID[2][col];  // Pawn moved > 2 squares, or it doesn't exist
            scores.end += KING_PAWN_SHIELD_OWN_PENALTIES_END[2][col];  // Pawn moved > 2 squares, or it doesn't exist
            trace.own_king_pawn_shield[2][col][BLACK_COLOR]++;
        }

        if (position.pawn_rank[0][file] == 9) {
            scores.mid += KING_PAWN_SHIELD_OPP_PENALTIES_MID[0][col];  // No enemy pawn on this file
            scores.end += KING_PAWN_SHIELD_OPP_PENALTIES_END[0][col];  // No enemy pawn on this file
            trace.opp_king_pawn_shield[0][col][BLACK_COLOR]++;
        }
        else if (position.pawn_rank[0][file] == 5) {
            scores.mid += KING_PAWN_SHIELD_OPP_PENALTIES_MID[1][col];  // Enemy pawn is on the 5th rank
            scores.end += KING_PAWN_SHIELD_OPP_PENALTIES_END[1][col];  // Enemy pawn is on the 5th rank
            trace.opp_king_pawn_shield[1][col][BLACK_COLOR]++;
        }
        else if (position.pawn_rank[0][file] == 6) {
            scores.mid += KING_PAWN_SHIELD_OPP_PENALTIES_MID[2][col];  // Enemy pawn is on the 6th rank
            scores.end += KING_PAWN_SHIELD_OPP_PENALTIES_END[2][col];  // Enemy pawn is on the 6th rank
            trace.opp_king_pawn_shield[2][col][BLACK_COLOR]++;
        }
    }
}


void evaluate_pawn(const Position& position, Score_Struct& scores, SQUARE_TYPE pos, bool is_white, Trace& trace) {

    SQUARE_TYPE i = MAILBOX_TO_STANDARD[pos];
    SQUARE_TYPE row = 8 - i / 8, col = i % 8 + 1;

    if (is_white) {

        scores.mid += PAWN_PST_MID[i];
        scores.end += PAWN_PST_END[i];
        trace.pawn_pst[i][WHITE_COLOR]++;

        // Pawn Protection and Threats
        if (position.board[pos - 9] < BLACK_PAWN) {
            scores.mid += PIECE_SUPPORT_MID[WHITE_PAWN][position.board[pos - 9]];
            scores.end += PIECE_SUPPORT_END[WHITE_PAWN][position.board[pos - 9]];
            trace.piece_support[WHITE_PAWN][position.board[pos - 9]][WHITE_COLOR]++;
        } else if (position.board[pos - 9] < EMPTY) {
            scores.mid += PIECE_THREAT_MID[WHITE_PAWN][position.board[pos - 9] - BLACK_PAWN];
            scores.end += PIECE_THREAT_END[WHITE_PAWN][position.board[pos - 9] - BLACK_PAWN];
            trace.piece_threat[WHITE_PAWN][position.board[pos - 9] - BLACK_PAWN][WHITE_COLOR]++;
        }

        if (position.board[pos - 11] < BLACK_PAWN) {
            scores.mid += PIECE_SUPPORT_MID[WHITE_PAWN][position.board[pos - 11]];
            scores.end += PIECE_SUPPORT_END[WHITE_PAWN][position.board[pos - 11]];
            trace.piece_support[WHITE_PAWN][position.board[pos - 11]][WHITE_COLOR]++;
        } else if (position.board[pos - 11] < EMPTY) {
            scores.mid += PIECE_THREAT_MID[WHITE_PAWN][position.board[pos - 11] - BLACK_PAWN];
            scores.end += PIECE_THREAT_END[WHITE_PAWN][position.board[pos - 11] - BLACK_PAWN];
            trace.piece_threat[WHITE_PAWN][position.board[pos - 11] - BLACK_PAWN][WHITE_COLOR]++;
        }

        // Doubled pawns. The pawn we are checking is higher in row compared to
        // the least advanced pawn in our column.
        if (row > position.pawn_rank[0][col]) {
            scores.mid += DOUBLED_PAWN_PENALTY_MID;
            scores.end += DOUBLED_PAWN_PENALTY_END;
            trace.doubled_pawns[WHITE_COLOR]++;
        }

        // Isolated pawns. We do not have pawns on the columns next to our pawn.
        if (position.pawn_rank[0][col - 1] == 9 && position.pawn_rank[0][col + 1] == 9) {
            // If our opponent does not have a pawn in front of our pawn
            if (position.pawn_rank[1][col] == 0) {
                // The isolated pawn in the middle game is worse if the opponent
                // has the semi open file to attack it.
                scores.mid += ISOLATED_PAWN_SEMI_OPEN_FILE_PENALTY_MID;
                scores.end += ISOLATED_PAWN_SEMI_OPEN_FILE_PENALTY_END;
                trace.isolated_pawns_semi_open_file[WHITE_COLOR]++;
            }
            else {
                scores.mid += ISOLATED_PAWN_PENALTY_MID;
                scores.end += ISOLATED_PAWN_PENALTY_END;
                trace.isolated_pawns[WHITE_COLOR]++;
            }
        }

        // Backwards pawn
        else if (row < position.pawn_rank[0][col - 1] && row < position.pawn_rank[0][col + 1]) {
            // In the middle game it's worse to have a very backwards pawn
            // since then, the 'forwards' pawns won't be protected
            scores.mid += BACKWARDS_PAWN_PENALTY_MID;

            // In the end game the backwards pawn should be worse, but if it's very backwards it's not awful.
            scores.end += BACKWARDS_PAWN_PENALTY_END;

            trace.backward_pawns[WHITE_COLOR]++;

            // If there's no enemy pawn in front of our pawn then it's even worse, since
            // we allow outposts and pieces to attack us easily
            if (position.pawn_rank[1][col] == 0) {
                scores.mid += BACKWARDS_PAWN_SEMI_OPEN_FILE_PENALTY_MID;
                scores.end += BACKWARDS_PAWN_SEMI_OPEN_FILE_PENALTY_END;
                trace.backward_pawns_semi_open_file[WHITE_COLOR]++;
            }

            // Outposts
            auto outpost_pos = static_cast<SQUARE_TYPE>(pos - 10);
            if (position.board[outpost_pos - 11] == BLACK_PAWN || position.board[outpost_pos - 9] == BLACK_PAWN) {
                scores.mid += OUTPOST_PENALTY_MID;
                scores.end += OUTPOST_PENALTY_END;
                trace.outpost_penalty[WHITE_COLOR]++;

                if (position.board[outpost_pos] == BLACK_KNIGHT) {
                    scores.mid += OUTPOST_KNIGHT_PENALTY_MID;
                    scores.end += OUTPOST_KNIGHT_PENALTY_END;
                    trace.outpost_knight_penalty[WHITE_COLOR]++;
                } else if (position.board[outpost_pos] == BLACK_BISHOP) {
                    scores.mid += OUTPOST_BISHOP_PENALTY_MID;
                    scores.end += OUTPOST_BISHOP_PENALTY_END;
                    trace.outpost_bishop_penalty[WHITE_COLOR]++;
                }
            }
        }

        // Passed Pawn Bonus
        if (row >= position.pawn_rank[1][col - 1] &&
            row >= position.pawn_rank[1][col] &&
            row >= position.pawn_rank[1][col + 1]) {

            int protectors = 0;
            if (position.board[pos + 11] == WHITE_PAWN) protectors++;
            if (position.board[pos + 9] == WHITE_PAWN) protectors++;

            scores.mid += PASSED_PAWN_BONUSES_MID[protectors][row - 1];
            scores.end += PASSED_PAWN_BONUSES_END[protectors][row - 1];

            trace.passed_pawns[protectors][row - 1][WHITE_COLOR]++;

            // Blocker right in front of pawn
            if (WHITE_KING < position.board[pos - 10] && position.board[pos - 10] < EMPTY) {
                scores.mid += BLOCKER_VALUES_MID[position.board[pos - 10] - 6][row - 1];
                scores.end += BLOCKER_VALUES_END[position.board[pos - 10] - 6][row - 1];
                trace.blockers[position.board[pos - 10] - 6][row - 1][WHITE_COLOR]++;
            }

            // Blocker two squares in front of pawn
            if (row < 7 && WHITE_KING < position.board[pos - 20] && position.board[pos - 20] < EMPTY) {
                scores.mid += BLOCKER_TWO_SQUARE_VALUES_MID[position.board[pos - 20] - 6][row - 1];
                scores.end += BLOCKER_TWO_SQUARE_VALUES_END[position.board[pos - 20] - 6][row - 1];
                trace.blockers_two_squares[position.board[pos - 20] - 6][row - 1][WHITE_COLOR]++;
            }

            SQUARE_TYPE promotion_square = i % 8;
            int our_distance = get_chebyshev_distance(i, promotion_square);
            int king_distance = get_chebyshev_distance(MAILBOX_TO_STANDARD[position.king_positions[BLACK_COLOR]], promotion_square);

            if (std::min(our_distance, 5) < king_distance - (position.side == BLACK_COLOR)) {
                scores.mid += SQUARE_OF_THE_PAWN_MID;
                scores.end += SQUARE_OF_THE_PAWN_END;
                trace.square_of_the_pawn[WHITE_COLOR]++;
            }

        }

        // Pawn Phalanx
        if (position.board[pos - 1] == WHITE_PAWN) {
            scores.mid += PHALANX_PAWN_BONUS_MID[row - 1];
            scores.end += PHALANX_PAWN_BONUS_END[row - 1];
            trace.pawn_phalanx[row - 1][WHITE_COLOR]++;
        }
    }

    else {

        scores.mid += PAWN_PST_MID[i ^ 56];
        scores.end += PAWN_PST_END[i ^ 56];
        trace.pawn_pst[i ^ 56][BLACK_COLOR]++;

        // Pawn Protection and Threats
        if (position.board[pos + 9] < BLACK_PAWN) {
            scores.mid += PIECE_THREAT_MID[WHITE_PAWN][position.board[pos + 9]];
            scores.end += PIECE_THREAT_END[WHITE_PAWN][position.board[pos + 9]];
            trace.piece_threat[WHITE_PAWN][position.board[pos + 9]][BLACK_COLOR]++;
        } else if (position.board[pos + 9] < EMPTY) {
            scores.mid += PIECE_SUPPORT_MID[WHITE_PAWN][position.board[pos + 9] - BLACK_PAWN];
            scores.end += PIECE_SUPPORT_END[WHITE_PAWN][position.board[pos + 9] - BLACK_PAWN];
            trace.piece_support[WHITE_PAWN][position.board[pos + 9] - BLACK_PAWN][BLACK_COLOR]++;
        }

        if (position.board[pos + 11] < BLACK_PAWN) {
            scores.mid += PIECE_THREAT_MID[WHITE_PAWN][position.board[pos + 11]];
            scores.end += PIECE_THREAT_END[WHITE_PAWN][position.board[pos + 11]];
            trace.piece_threat[WHITE_PAWN][position.board[pos + 11]][BLACK_COLOR]++;
        } else if (position.board[pos + 11] < EMPTY) {
            scores.mid += PIECE_SUPPORT_MID[WHITE_PAWN][position.board[pos + 11] - BLACK_PAWN];
            scores.end += PIECE_SUPPORT_END[WHITE_PAWN][position.board[pos + 11] - BLACK_PAWN];
            trace.piece_support[WHITE_PAWN][position.board[pos + 11] - BLACK_PAWN][BLACK_COLOR]++;
        }

        // Doubled pawns. The pawn we are checking is higher in row compared to
        // the least advanced pawn in our column.
        if (row < position.pawn_rank[1][col]) {
            scores.mid += DOUBLED_PAWN_PENALTY_MID;
            scores.end += DOUBLED_PAWN_PENALTY_END;
            trace.doubled_pawns[BLACK_COLOR]++;
        }

        // Isolated pawns. We do not have pawns on the columns next to our pawn.
        if (position.pawn_rank[1][col - 1] == 0 && position.pawn_rank[1][col + 1] == 0) {
            // If our opponent does not have a pawn in front of our pawn
            if (position.pawn_rank[0][col] == 9) {
                // The isolated pawn in the middle game is worse if the opponent
                // has the semi open file to attack it.
                scores.mid += ISOLATED_PAWN_SEMI_OPEN_FILE_PENALTY_MID;
                scores.end += ISOLATED_PAWN_SEMI_OPEN_FILE_PENALTY_END;
                trace.isolated_pawns_semi_open_file[BLACK_COLOR]++;
            }
            else {
                scores.mid += ISOLATED_PAWN_PENALTY_MID;
                scores.end += ISOLATED_PAWN_PENALTY_END;
                trace.isolated_pawns[BLACK_COLOR]++;
            }
        }

        // Backwards pawn
        else if (row > position.pawn_rank[1][col - 1] && row > position.pawn_rank[1][col + 1]) {
            // In the middle game it's worse to have a very backwards pawn
            // since then, the 'forwards' pawns won't be protected
            scores.mid += BACKWARDS_PAWN_PENALTY_MID;

            // In the end game the backwards pawn should be worse, but if it's very backwards it's not awful.
            scores.end += BACKWARDS_PAWN_PENALTY_END;

            trace.backward_pawns[BLACK_COLOR]++;

            // If there's no enemy pawn in front of our pawn then it's even worse, since
            // we allow outposts and pieces to attack us easily
            if (position.pawn_rank[0][col] == 9) {
                scores.mid += BACKWARDS_PAWN_SEMI_OPEN_FILE_PENALTY_MID;
                scores.end += BACKWARDS_PAWN_SEMI_OPEN_FILE_PENALTY_END;
                trace.backward_pawns_semi_open_file[BLACK_COLOR]++;
            }

            // Outposts
            auto outpost_pos = static_cast<SQUARE_TYPE>(pos + 10);
            if (position.board[outpost_pos + 11] == WHITE_PAWN || position.board[outpost_pos + 9] == WHITE_PAWN) {
                scores.mid += OUTPOST_PENALTY_MID;
                scores.end += OUTPOST_PENALTY_END;
                trace.outpost_penalty[BLACK_COLOR]++;

                if (position.board[outpost_pos] == WHITE_KNIGHT) {
                    scores.mid += OUTPOST_KNIGHT_PENALTY_MID;
                    scores.end += OUTPOST_KNIGHT_PENALTY_END;
                    trace.outpost_knight_penalty[BLACK_COLOR]++;
                } else if (position.board[outpost_pos] == WHITE_BISHOP) {
                    scores.mid += OUTPOST_BISHOP_PENALTY_MID;
                    scores.end += OUTPOST_BISHOP_PENALTY_END;
                    trace.outpost_bishop_penalty[BLACK_COLOR]++;
                }
            }
        }

        // Passed Pawn Bonus
        if (row <= position.pawn_rank[0][col - 1] &&
            row <= position.pawn_rank[0][col] &&
            row <= position.pawn_rank[0][col + 1]) {

            int protectors = 0;
            if (position.board[pos - 11] == BLACK_PAWN) protectors++;
            if (position.board[pos - 9] == BLACK_PAWN) protectors++;

            scores.mid += PASSED_PAWN_BONUSES_MID[protectors][8 - row];
            scores.end += PASSED_PAWN_BONUSES_END[protectors][8 - row];

            trace.passed_pawns[protectors][8 - row][BLACK_COLOR]++;

            // Blocker right in front of pawn
            if (position.board[pos + 10] < BLACK_PAWN) {
                scores.mid += BLOCKER_VALUES_MID[position.board[pos + 10]][8 - row];
                scores.end += BLOCKER_VALUES_END[position.board[pos + 10]][8 - row];
                trace.blockers[position.board[pos + 10]][8 - row][BLACK_COLOR]++;
            }

            // Blocker two squares in front of pawn
            if (row > 2 && position.board[pos + 20] < BLACK_PAWN) {
                scores.mid += BLOCKER_TWO_SQUARE_VALUES_MID[position.board[pos + 20]][8 - row];
                scores.end += BLOCKER_TWO_SQUARE_VALUES_END[position.board[pos + 20]][8 - row];
                trace.blockers_two_squares[position.board[pos + 20]][8 - row][BLACK_COLOR]++;
            }

            SQUARE_TYPE promotion_square = (i % 8) ^ 56;
            int our_distance = get_chebyshev_distance(i, promotion_square);
            int king_distance = get_chebyshev_distance(MAILBOX_TO_STANDARD[position.king_positions[WHITE_COLOR]], promotion_square);

            if (std::min(our_distance, 5) < king_distance - (position.side == WHITE_COLOR)) {
                scores.mid += SQUARE_OF_THE_PAWN_MID;
                scores.end += SQUARE_OF_THE_PAWN_END;
                trace.square_of_the_pawn[BLACK_COLOR]++;
            }
        }

        // Pawn Phalanx
        if (position.board[pos - 1] == BLACK_PAWN) {
            scores.mid += PHALANX_PAWN_BONUS_MID[8 - row];
            scores.end += PHALANX_PAWN_BONUS_END[8 - row];
            trace.pawn_phalanx[8 - row][BLACK_COLOR]++;
        }
    }
}


void evaluate_knight(const Position& position, Score_Struct& scores, SQUARE_TYPE pos, bool is_white, Trace& trace) {
    SQUARE_TYPE i = MAILBOX_TO_STANDARD[pos];
    SCORE_TYPE mobility = 0;
    int king_ring_attacks[2]{};

    if (is_white) {
        scores.mid += KNIGHT_PST_MID[i];
        scores.end += KNIGHT_PST_END[i];
        trace.knight_pst[i][WHITE_COLOR]++;

        // Calculate Mobility
        for (short increment : WHITE_INCREMENTS[WHITE_KNIGHT]) {
            SQUARE_TYPE new_pos = pos + increment;
            PIECE_TYPE occupied = position.board[new_pos];

            if (occupied == PADDING) continue;

            king_ring_attacks[0] += bool((1ULL << MAILBOX_TO_STANDARD[new_pos]) &
                                         king_ring_zone.masks[0][MAILBOX_TO_STANDARD[position.king_positions[BLACK_COLOR]]]);
            king_ring_attacks[1] += bool((1ULL << MAILBOX_TO_STANDARD[new_pos]) &
                                         king_ring_zone.masks[1][MAILBOX_TO_STANDARD[position.king_positions[BLACK_COLOR]]]);

            // If there is an enemy pawn controlling this square then we deduct 2.
            if (position.board[new_pos - 11] == BLACK_PAWN || position.board[new_pos - 9] == BLACK_PAWN)
                mobility--;

            mobility++;

            scores.mid += KNIGHT_CONTROL_MID[MAILBOX_TO_STANDARD[new_pos]];
            scores.end += KNIGHT_CONTROL_END[MAILBOX_TO_STANDARD[new_pos]];
            trace.knight_control[MAILBOX_TO_STANDARD[new_pos]][WHITE_COLOR]++;

            // If we hit a piece of ours, we still add 1 to mobility because
            // that means we are protecting a piece of ours.
            if (occupied < BLACK_PAWN) {
                scores.mid += PIECE_SUPPORT_MID[WHITE_KNIGHT][occupied];
                scores.end += PIECE_SUPPORT_END[WHITE_KNIGHT][occupied];
                trace.piece_support[WHITE_KNIGHT][occupied][WHITE_COLOR]++;
                continue;
            }

            // Attacking pieces means more pressure which is good
            if (occupied < EMPTY) {
                scores.mid += PIECE_THREAT_MID[WHITE_KNIGHT][occupied - BLACK_PAWN];
                scores.end += PIECE_THREAT_END[WHITE_KNIGHT][occupied - BLACK_PAWN];
                trace.piece_threat[WHITE_KNIGHT][occupied - BLACK_PAWN][WHITE_COLOR]++;
                continue;
            }
        }
    }
    else {
        scores.mid += KNIGHT_PST_MID[i ^ 56];
        scores.end += KNIGHT_PST_END[i ^ 56];
        trace.knight_pst[i ^ 56][BLACK_COLOR]++;

        // Calculate Mobility
        for (short increment : BLACK_INCREMENTS[WHITE_KNIGHT]) {
            SQUARE_TYPE new_pos = pos + increment;
            PIECE_TYPE occupied = position.board[new_pos];

            if (occupied == PADDING) continue;

            king_ring_attacks[0] += bool((1ULL << MAILBOX_TO_STANDARD[new_pos]) &
                                         king_ring_zone.masks[0][MAILBOX_TO_STANDARD[position.king_positions[WHITE_COLOR]]]);
            king_ring_attacks[1] += bool((1ULL << MAILBOX_TO_STANDARD[new_pos]) &
                                         king_ring_zone.masks[1][MAILBOX_TO_STANDARD[position.king_positions[WHITE_COLOR]]]);

            // If there is an enemy pawn controlling this square then we deduct 2.
            if (position.board[new_pos + 11] == WHITE_PAWN || position.board[new_pos + 9] == WHITE_PAWN)
                mobility--;

            mobility++;

            scores.mid += KNIGHT_CONTROL_MID[MAILBOX_TO_STANDARD[new_pos] ^ 56];
            scores.end += KNIGHT_CONTROL_END[MAILBOX_TO_STANDARD[new_pos] ^ 56];
            trace.knight_control[MAILBOX_TO_STANDARD[new_pos] ^ 56][BLACK_COLOR]++;

            // If we hit a piece of ours, we still add 1 to mobility because
            // that means we are protecting a piece of ours.
            if (WHITE_KING < occupied && occupied < EMPTY) {
                scores.mid += PIECE_SUPPORT_MID[WHITE_KNIGHT][occupied - BLACK_PAWN];
                scores.end += PIECE_SUPPORT_END[WHITE_KNIGHT][occupied - BLACK_PAWN];
                trace.piece_support[WHITE_KNIGHT][occupied - BLACK_PAWN][BLACK_COLOR]++;
                continue;
            }

            // Attacking pieces means more pressure which is good
            if (occupied < BLACK_PAWN) {
                scores.mid += PIECE_THREAT_MID[WHITE_KNIGHT][occupied];
                scores.end += PIECE_THREAT_END[WHITE_KNIGHT][occupied];
                trace.piece_threat[WHITE_KNIGHT][occupied][BLACK_COLOR]++;
                continue;
            }
        }
    }

    // Knights are good protectors for the king
    SCORE_TYPE distance_to_our_king = get_distance(i, MAILBOX_TO_STANDARD[position.king_positions[(!is_white)]]);
    scores.mid += static_cast<SCORE_TYPE>(OWN_KING_DISTANCE_COEFFICIENTS_MID[WHITE_KNIGHT] * distance_to_our_king);
    scores.end += static_cast<SCORE_TYPE>(OWN_KING_DISTANCE_COEFFICIENTS_END[WHITE_KNIGHT] * distance_to_our_king);
    trace.own_king_tropism[WHITE_KNIGHT][!is_white] += distance_to_our_king;

    // Knights are also very good at attacking the opponents king
    SCORE_TYPE distance_to_opp_king = get_distance(i, MAILBOX_TO_STANDARD[position.king_positions[(is_white)]]);
    scores.mid += static_cast<SCORE_TYPE>(OPP_KING_DISTANCE_COEFFICIENTS_MID[WHITE_KNIGHT] * distance_to_opp_king);
    scores.end += static_cast<SCORE_TYPE>(OPP_KING_DISTANCE_COEFFICIENTS_END[WHITE_KNIGHT] * distance_to_opp_king);
    trace.opp_king_tropism[WHITE_KNIGHT][!is_white] += distance_to_opp_king;

    scores.mid += KNIGHT_MOBILITY_MID[mobility];
    scores.end += KNIGHT_MOBILITY_END[mobility];
    trace.knight_mobility[mobility][!is_white]++;

    scores.mid += king_ring_attacks[0] * KING_RING_ATTACKS_MID[0][WHITE_KNIGHT];
    scores.end += king_ring_attacks[0] * KING_RING_ATTACKS_END[0][WHITE_KNIGHT];
    trace.king_ring_attacks[0][WHITE_KNIGHT][!is_white] += king_ring_attacks[0];

    scores.mid += king_ring_attacks[1] * KING_RING_ATTACKS_MID[1][WHITE_KNIGHT];
    scores.end += king_ring_attacks[1] * KING_RING_ATTACKS_END[1][WHITE_KNIGHT];
    trace.king_ring_attacks[1][WHITE_KNIGHT][!is_white] += king_ring_attacks[1];

    // std::cout << "KNIGHT MOBILITY: " << mobility << std::endl;
}


void evaluate_bishop(const Position& position, Score_Struct& scores, SQUARE_TYPE pos, bool is_white, Trace& trace) {
    SQUARE_TYPE i = MAILBOX_TO_STANDARD[pos];
    SCORE_TYPE mobility = 0;
    int king_ring_attacks[2]{};

    if (is_white) {
        scores.mid += BISHOP_PST_MID[i];
        scores.end += BISHOP_PST_END[i];
        trace.bishop_pst[i][WHITE_COLOR]++;

        // Calculate Mobility
        for (short increment : WHITE_INCREMENTS[WHITE_BISHOP]) {
            SQUARE_TYPE new_pos = pos;
            if (!increment) break;

            while (true) {
                new_pos += increment;
                PIECE_TYPE occupied = position.board[new_pos];

                if (occupied == PADDING) break;

                king_ring_attacks[0] += bool((1ULL << MAILBOX_TO_STANDARD[new_pos]) &
                                             king_ring_zone.masks[0][MAILBOX_TO_STANDARD[position.king_positions[BLACK_COLOR]]]);
                king_ring_attacks[1] += bool((1ULL << MAILBOX_TO_STANDARD[new_pos]) &
                                             king_ring_zone.masks[1][MAILBOX_TO_STANDARD[position.king_positions[BLACK_COLOR]]]);

                // If there is an enemy pawn controlling this square then we deduct 2.
                if (position.board[new_pos - 11] == BLACK_PAWN || position.board[new_pos - 9] == BLACK_PAWN)
                    mobility--;

                mobility++;

                scores.mid += BISHOP_CONTROL_MID[MAILBOX_TO_STANDARD[new_pos]];
                scores.end += BISHOP_CONTROL_END[MAILBOX_TO_STANDARD[new_pos]];
                trace.bishop_control[MAILBOX_TO_STANDARD[new_pos]][WHITE_COLOR]++;

                // If we hit a piece of ours, we still add 1 to mobility because
                // that means we are protecting a piece of ours.
                if (occupied < BLACK_PAWN) {
                    scores.mid += PIECE_SUPPORT_MID[WHITE_BISHOP][occupied];
                    scores.end += PIECE_SUPPORT_END[WHITE_BISHOP][occupied];
                    trace.piece_support[WHITE_BISHOP][occupied][WHITE_COLOR]++;
                    break;
                }

                // Attacking pieces means more pressure which is good
                if (occupied < EMPTY) {
                    scores.mid += PIECE_THREAT_MID[WHITE_BISHOP][occupied - BLACK_PAWN];
                    scores.end += PIECE_THREAT_END[WHITE_BISHOP][occupied - BLACK_PAWN];
                    trace.piece_threat[WHITE_BISHOP][occupied - BLACK_PAWN][WHITE_COLOR]++;
                    break;
                }
            }
        }
    }
    else {
        scores.mid += BISHOP_PST_MID[i ^ 56];
        scores.end += BISHOP_PST_END[i ^ 56];
        trace.bishop_pst[i ^ 56][BLACK_COLOR]++;

        // Calculate Mobility
        for (short increment : BLACK_INCREMENTS[WHITE_BISHOP]) {
            SQUARE_TYPE new_pos = pos;
            if (!increment) break;

            while (true) {
                new_pos += increment;
                PIECE_TYPE occupied = position.board[new_pos];

                if (occupied == PADDING) break;

                king_ring_attacks[0] += bool((1ULL << MAILBOX_TO_STANDARD[new_pos]) &
                                             king_ring_zone.masks[0][MAILBOX_TO_STANDARD[position.king_positions[WHITE_COLOR]]]);
                king_ring_attacks[1] += bool((1ULL << MAILBOX_TO_STANDARD[new_pos]) &
                                             king_ring_zone.masks[1][MAILBOX_TO_STANDARD[position.king_positions[WHITE_COLOR]]]);

                // If there is an enemy pawn controlling this square then we deduct 2.
                if (position.board[new_pos + 11] == WHITE_PAWN || position.board[new_pos + 9] == WHITE_PAWN)
                    mobility--;

                mobility++;

                scores.mid += BISHOP_CONTROL_MID[MAILBOX_TO_STANDARD[new_pos] ^ 56];
                scores.end += BISHOP_CONTROL_END[MAILBOX_TO_STANDARD[new_pos] ^ 56];
                trace.bishop_control[MAILBOX_TO_STANDARD[new_pos] ^ 56][BLACK_COLOR]++;

                // If we hit a piece of ours, we still add 1 to mobility because
                // that means we are protecting a piece of ours.
                if (WHITE_KING < occupied && occupied < EMPTY) {
                    scores.mid += PIECE_SUPPORT_MID[WHITE_BISHOP][occupied - BLACK_PAWN];
                    scores.end += PIECE_SUPPORT_END[WHITE_BISHOP][occupied - BLACK_PAWN];
                    trace.piece_support[WHITE_BISHOP][occupied - BLACK_PAWN][BLACK_COLOR]++;
                    break;
                }

                // Attacking pieces means more pressure which is good
                if (occupied < BLACK_PAWN) {
                    scores.mid += PIECE_THREAT_MID[WHITE_BISHOP][occupied];
                    scores.end += PIECE_THREAT_END[WHITE_BISHOP][occupied];
                    trace.piece_threat[WHITE_BISHOP][occupied][BLACK_COLOR]++;
                    break;
                }
            }
        }
    }

    SCORE_TYPE distance_to_opp_king = get_distance(i, MAILBOX_TO_STANDARD[position.king_positions[(is_white)]]);
    scores.mid += static_cast<SCORE_TYPE>(OPP_KING_DISTANCE_COEFFICIENTS_MID[WHITE_BISHOP] * distance_to_opp_king);
    scores.end += static_cast<SCORE_TYPE>(OPP_KING_DISTANCE_COEFFICIENTS_END[WHITE_BISHOP] * distance_to_opp_king);
    trace.opp_king_tropism[WHITE_BISHOP][!is_white] += distance_to_opp_king;

    scores.mid += BISHOP_MOBILITY_MID[mobility];
    scores.end += BISHOP_MOBILITY_END[mobility];
    trace.bishop_mobility[mobility][!is_white]++;

    scores.mid += king_ring_attacks[0] * KING_RING_ATTACKS_MID[0][WHITE_BISHOP];
    scores.end += king_ring_attacks[0] * KING_RING_ATTACKS_END[0][WHITE_BISHOP];
    trace.king_ring_attacks[0][WHITE_BISHOP][!is_white] += king_ring_attacks[0];

    scores.mid += king_ring_attacks[1] * KING_RING_ATTACKS_MID[1][WHITE_BISHOP];
    scores.end += king_ring_attacks[1] * KING_RING_ATTACKS_END[1][WHITE_BISHOP];
    trace.king_ring_attacks[1][WHITE_BISHOP][!is_white] += king_ring_attacks[1];
    // std::cout << "BISHOP MOBILITY: " << mobility << std::endl;
}


void evaluate_rook(const Position& position, Score_Struct& scores, SQUARE_TYPE pos, bool is_white, Trace& trace) {
    SQUARE_TYPE i = MAILBOX_TO_STANDARD[pos];
    SQUARE_TYPE col = i % 8 + 1;
    SCORE_TYPE mobility = 0;
    int king_ring_attacks[2]{};

    if (is_white) {
        scores.mid += ROOK_PST_MID[i];
        scores.end += ROOK_PST_END[i];
        trace.rook_pst[i][WHITE_COLOR]++;

        if (position.pawn_rank[0][col] == 9) {
            if (position.pawn_rank[1][col] == 0) {
                scores.mid += ROOK_OPEN_FILE_BONUS_MID;
                scores.end += ROOK_OPEN_FILE_BONUS_END;
                trace.rook_open[WHITE_COLOR]++;
            }
            else {
                scores.mid += ROOK_SEMI_OPEN_FILE_BONUS_MID;
                scores.end += ROOK_SEMI_OPEN_FILE_BONUS_END;
                trace.rook_semi_open[WHITE_COLOR]++;
            }
        }

        // Calculate Mobility
        for (short increment : WHITE_INCREMENTS[WHITE_ROOK]) {
            SQUARE_TYPE new_pos = pos;
            if (!increment) break;

            while (true) {
                new_pos += increment;
                PIECE_TYPE occupied = position.board[new_pos];

                if (occupied == PADDING) break;

                king_ring_attacks[0] += bool((1ULL << MAILBOX_TO_STANDARD[new_pos]) &
                                             king_ring_zone.masks[0][MAILBOX_TO_STANDARD[position.king_positions[BLACK_COLOR]]]);
                king_ring_attacks[1] += bool((1ULL << MAILBOX_TO_STANDARD[new_pos]) &
                                             king_ring_zone.masks[1][MAILBOX_TO_STANDARD[position.king_positions[BLACK_COLOR]]]);

                // If there is an enemy pawn controlling this square then we deduct 2.
                if (position.board[new_pos - 11] == BLACK_PAWN || position.board[new_pos - 9] == BLACK_PAWN)
                    mobility--;

                mobility++;

                scores.mid += ROOK_CONTROL_MID[MAILBOX_TO_STANDARD[new_pos]];
                scores.end += ROOK_CONTROL_END[MAILBOX_TO_STANDARD[new_pos]];
                trace.rook_control[MAILBOX_TO_STANDARD[new_pos]][WHITE_COLOR]++;

                // If we hit a piece of ours, we still add 1 to mobility because
                // that means we are protecting a piece of ours.
                if (occupied < BLACK_PAWN) {
                    scores.mid += PIECE_SUPPORT_MID[WHITE_ROOK][occupied];
                    scores.end += PIECE_SUPPORT_END[WHITE_ROOK][occupied];
                    trace.piece_support[WHITE_ROOK][occupied][WHITE_COLOR]++;
                    break;
                }

                // If we hit an enemy piece, get a score of 2.
                // An empty square may be even better, so you get a score 3.
                if (occupied < EMPTY) {
                    scores.mid += PIECE_THREAT_MID[WHITE_ROOK][occupied - BLACK_PAWN];
                    scores.end += PIECE_THREAT_END[WHITE_ROOK][occupied - BLACK_PAWN];
                    trace.piece_threat[WHITE_ROOK][occupied - BLACK_PAWN][WHITE_COLOR]++;
                    break;
                }
            }
        }
    }
    else {
        scores.mid += ROOK_PST_MID[i ^ 56];
        scores.end += ROOK_PST_END[i ^ 56];
        trace.rook_pst[i ^ 56][BLACK_COLOR]++;

        if (position.pawn_rank[1][col] == 0) {
            if (position.pawn_rank[0][col] == 9) {
                scores.mid += ROOK_OPEN_FILE_BONUS_MID;
                scores.end += ROOK_OPEN_FILE_BONUS_END;
                trace.rook_open[BLACK_COLOR]++;
            }
            else {
                scores.mid += ROOK_SEMI_OPEN_FILE_BONUS_MID;
                scores.end += ROOK_SEMI_OPEN_FILE_BONUS_END;
                trace.rook_semi_open[BLACK_COLOR]++;
            }
        }

        // Calculate Mobility
        for (short increment : BLACK_INCREMENTS[WHITE_ROOK]) {
            SQUARE_TYPE new_pos = pos;
            if (!increment) break;

            while (true) {
                new_pos += increment;
                PIECE_TYPE occupied = position.board[new_pos];

                if (occupied == PADDING) break;

                king_ring_attacks[0] += bool((1ULL << MAILBOX_TO_STANDARD[new_pos]) &
                                             king_ring_zone.masks[0][MAILBOX_TO_STANDARD[position.king_positions[WHITE_COLOR]]]);
                king_ring_attacks[1] += bool((1ULL << MAILBOX_TO_STANDARD[new_pos]) &
                                             king_ring_zone.masks[1][MAILBOX_TO_STANDARD[position.king_positions[WHITE_COLOR]]]);

                // If there is an enemy pawn controlling this square then we deduct 2.
                if (position.board[new_pos + 11] == WHITE_PAWN || position.board[new_pos + 9] == WHITE_PAWN)
                    mobility--;

                mobility++;

                scores.mid += ROOK_CONTROL_MID[MAILBOX_TO_STANDARD[new_pos] ^ 56];
                scores.end += ROOK_CONTROL_END[MAILBOX_TO_STANDARD[new_pos] ^ 56];
                trace.rook_control[MAILBOX_TO_STANDARD[new_pos] ^ 56][BLACK_COLOR]++;

                // If we hit a piece of ours, we still add 1 to mobility because
                // that means we are protecting a piece of ours.
                if (WHITE_KING < occupied && occupied < EMPTY) {
                    scores.mid += PIECE_SUPPORT_MID[WHITE_ROOK][occupied - BLACK_PAWN];
                    scores.end += PIECE_SUPPORT_END[WHITE_ROOK][occupied - BLACK_PAWN];
                    trace.piece_support[WHITE_ROOK][occupied - BLACK_PAWN][BLACK_COLOR]++;
                    break;
                }

                // If we hit an enemy piece, get a score of 2.
                // An empty square may be even better, so you get a score 3.
                if (occupied < BLACK_PAWN) {
                    scores.mid += PIECE_THREAT_MID[WHITE_ROOK][occupied];
                    scores.end += PIECE_THREAT_END[WHITE_ROOK][occupied];
                    trace.piece_threat[WHITE_ROOK][occupied][BLACK_COLOR]++;
                    break;
                }
            }
        }
    }

    SCORE_TYPE distance_to_opp_king = get_distance(i, MAILBOX_TO_STANDARD[position.king_positions[(is_white)]]);
    scores.mid += static_cast<SCORE_TYPE>(OPP_KING_DISTANCE_COEFFICIENTS_MID[WHITE_ROOK] * distance_to_opp_king);
    scores.end += static_cast<SCORE_TYPE>(OPP_KING_DISTANCE_COEFFICIENTS_END[WHITE_ROOK] * distance_to_opp_king);
    trace.opp_king_tropism[WHITE_ROOK][!is_white] += distance_to_opp_king;

    scores.mid += ROOK_MOBILITY_MID[mobility];
    scores.end += ROOK_MOBILITY_END[mobility];
    trace.rook_mobility[mobility][!is_white]++;

    scores.mid += king_ring_attacks[0] * KING_RING_ATTACKS_MID[0][WHITE_ROOK];
    scores.end += king_ring_attacks[0] * KING_RING_ATTACKS_END[0][WHITE_ROOK];
    trace.king_ring_attacks[0][WHITE_ROOK][!is_white] += king_ring_attacks[0];

    scores.mid += king_ring_attacks[1] * KING_RING_ATTACKS_MID[1][WHITE_ROOK];
    scores.end += king_ring_attacks[1] * KING_RING_ATTACKS_END[1][WHITE_ROOK];
    trace.king_ring_attacks[1][WHITE_ROOK][!is_white] += king_ring_attacks[1];

    // std::cout << "ROOK MOBILITY: " << mobility << std::endl;
}


void evaluate_queen(const Position& position, Score_Struct& scores, SQUARE_TYPE pos, bool is_white, Trace& trace) {
    SQUARE_TYPE i = MAILBOX_TO_STANDARD[pos];
    SQUARE_TYPE col = i % 8 + 1;
    SCORE_TYPE mobility = 0;
    int king_ring_attacks[2]{};

    if (is_white) {
        scores.mid += QUEEN_PST_MID[i];
        scores.end += QUEEN_PST_END[i];
        trace.queen_pst[i][WHITE_COLOR]++;

        if (position.pawn_rank[0][col] == 9) {
            if (position.pawn_rank[1][col] == 0) {
                scores.mid += QUEEN_OPEN_FILE_BONUS_MID;
                scores.end += QUEEN_OPEN_FILE_BONUS_END;
                trace.queen_open[WHITE_COLOR]++;
            }
            else {
                scores.mid += QUEEN_SEMI_OPEN_FILE_BONUS_MID;
                scores.end += QUEEN_SEMI_OPEN_FILE_BONUS_END;
                trace.queen_semi_open[WHITE_COLOR]++;
            }
        }

        // Calculate Mobility
        for (short increment : WHITE_INCREMENTS[WHITE_QUEEN]) {
            SQUARE_TYPE new_pos = pos;

            while (true) {
                new_pos += increment;
                PIECE_TYPE occupied = position.board[new_pos];

                if (occupied == PADDING) break;

                king_ring_attacks[0] += bool((1ULL << MAILBOX_TO_STANDARD[new_pos]) &
                                             king_ring_zone.masks[0][MAILBOX_TO_STANDARD[position.king_positions[BLACK_COLOR]]]);
                king_ring_attacks[1] += bool((1ULL << MAILBOX_TO_STANDARD[new_pos]) &
                                             king_ring_zone.masks[1][MAILBOX_TO_STANDARD[position.king_positions[BLACK_COLOR]]]);

                // If there is an enemy pawn controlling this square then we deduct 2.
                if (position.board[new_pos - 11] == BLACK_PAWN || position.board[new_pos - 9] == BLACK_PAWN)
                    mobility--;

                mobility++;

                scores.mid += QUEEN_CONTROL_MID[MAILBOX_TO_STANDARD[new_pos]];
                scores.end += QUEEN_CONTROL_END[MAILBOX_TO_STANDARD[new_pos]];
                trace.queen_control[MAILBOX_TO_STANDARD[new_pos]][WHITE_COLOR]++;

                // If we hit a piece of ours, we still add 1 to mobility because
                // that means we are protecting a piece of ours.
                if (occupied < BLACK_PAWN) {
                    scores.mid += PIECE_SUPPORT_MID[WHITE_QUEEN][occupied];
                    scores.end += PIECE_SUPPORT_END[WHITE_QUEEN][occupied];
                    trace.piece_support[WHITE_QUEEN][occupied][WHITE_COLOR]++;
                    break;
                }

                // If we hit an enemy piece, get a score of 2.
                // An empty square may be even better, so you get a score 3.
                if (occupied < EMPTY) {
                    scores.mid += PIECE_THREAT_MID[WHITE_QUEEN][occupied - BLACK_PAWN];
                    scores.end += PIECE_THREAT_END[WHITE_QUEEN][occupied - BLACK_PAWN];
                    trace.piece_threat[WHITE_QUEEN][occupied - BLACK_PAWN][WHITE_COLOR]++;
                    break;
                }
            }
        }
    }
    else {
        scores.mid += QUEEN_PST_MID[i ^ 56];
        scores.end += QUEEN_PST_END[i ^ 56];
        trace.queen_pst[i ^ 56][BLACK_COLOR]++;

        if (position.pawn_rank[1][col] == 0) {
            if (position.pawn_rank[0][col] == 9) {
                scores.mid += QUEEN_OPEN_FILE_BONUS_MID;
                scores.end += QUEEN_OPEN_FILE_BONUS_END;
                trace.queen_open[BLACK_COLOR]++;
            }
            else {
                scores.mid += QUEEN_SEMI_OPEN_FILE_BONUS_MID;
                scores.end += QUEEN_SEMI_OPEN_FILE_BONUS_END;
                trace.queen_semi_open[BLACK_COLOR]++;
            }
        }

        // Calculate Mobility
        for (short increment : BLACK_INCREMENTS[WHITE_QUEEN]) {
            SQUARE_TYPE new_pos = pos;

            while (true) {
                new_pos += increment;
                PIECE_TYPE occupied = position.board[new_pos];

                if (occupied == PADDING) break;

                king_ring_attacks[0] += bool((1ULL << MAILBOX_TO_STANDARD[new_pos]) &
                                             king_ring_zone.masks[0][MAILBOX_TO_STANDARD[position.king_positions[WHITE_COLOR]]]);
                king_ring_attacks[1] += bool((1ULL << MAILBOX_TO_STANDARD[new_pos]) &
                                             king_ring_zone.masks[1][MAILBOX_TO_STANDARD[position.king_positions[WHITE_COLOR]]]);

                // If there is an enemy pawn controlling this square then we deduct 2.
                if (position.board[new_pos + 11] == WHITE_PAWN || position.board[new_pos + 9] == WHITE_PAWN)
                    mobility--;

                mobility++;

                scores.mid += QUEEN_CONTROL_MID[MAILBOX_TO_STANDARD[new_pos] ^ 56];
                scores.end += QUEEN_CONTROL_END[MAILBOX_TO_STANDARD[new_pos] ^ 56];
                trace.queen_control[MAILBOX_TO_STANDARD[new_pos] ^ 56][BLACK_COLOR]++;

                // If we hit a piece of ours, we still add 1 to mobility because
                // that means we are protecting a piece of ours.
                if (WHITE_KING < occupied && occupied < EMPTY) {
                    scores.mid += PIECE_SUPPORT_MID[WHITE_QUEEN][occupied - BLACK_PAWN];
                    scores.end += PIECE_SUPPORT_END[WHITE_QUEEN][occupied - BLACK_PAWN];
                    trace.piece_support[WHITE_QUEEN][occupied - BLACK_PAWN][BLACK_COLOR]++;
                    break;
                }

                // If we hit an enemy piece, get a score of 2.
                // An empty square may be even better, so you get a score 3.
                if (occupied < BLACK_PAWN) {
                    scores.mid += PIECE_THREAT_MID[WHITE_QUEEN][occupied];
                    scores.end += PIECE_THREAT_END[WHITE_QUEEN][occupied];
                    trace.piece_threat[WHITE_QUEEN][occupied][BLACK_COLOR]++;
                    break;
                }
            }
        }
    }

    SCORE_TYPE distance_to_opp_king = get_distance(i, MAILBOX_TO_STANDARD[position.king_positions[(is_white)]]);
    scores.mid += static_cast<SCORE_TYPE>(OPP_KING_DISTANCE_COEFFICIENTS_MID[WHITE_QUEEN] * distance_to_opp_king);
    scores.end += static_cast<SCORE_TYPE>(OPP_KING_DISTANCE_COEFFICIENTS_END[WHITE_QUEEN] * distance_to_opp_king);
    trace.opp_king_tropism[WHITE_QUEEN][!is_white] += distance_to_opp_king;

    scores.mid += QUEEN_MOBILITY_MID[mobility];
    scores.end += QUEEN_MOBILITY_END[mobility];
    trace.queen_mobility[mobility][!is_white]++;

    scores.mid += king_ring_attacks[0] * KING_RING_ATTACKS_MID[0][WHITE_QUEEN];
    scores.end += king_ring_attacks[0] * KING_RING_ATTACKS_END[0][WHITE_QUEEN];
    trace.king_ring_attacks[0][WHITE_QUEEN][!is_white] += king_ring_attacks[0];

    scores.mid += king_ring_attacks[1] * KING_RING_ATTACKS_MID[1][WHITE_QUEEN];
    scores.end += king_ring_attacks[1] * KING_RING_ATTACKS_END[1][WHITE_QUEEN];
    trace.king_ring_attacks[1][WHITE_QUEEN][!is_white] += king_ring_attacks[1];

    // std::cout << "QUEEN MOBILITY: " << mobility << std::endl;
}

void evaluate_king(const Position& position, Score_Struct& scores, SQUARE_TYPE pos, bool is_white, Trace& trace) {
    SQUARE_TYPE i = MAILBOX_TO_STANDARD[pos];
    SQUARE_TYPE col = i % 8 + 1;

    if (is_white) {
        scores.mid += KING_PST_MID[i];
        scores.end += KING_PST_END[i];
        trace.king_pst[i][WHITE_COLOR]++;

        if (col < 4) {  // Queen side
            evaluate_king_pawn(position, scores, 1, true, trace);
            evaluate_king_pawn(position, scores, 2, true, trace);
            evaluate_king_pawn(position, scores, 3, true, trace);
        }
        else if (col > 5) {
            evaluate_king_pawn(position, scores, 8, true, trace);
            evaluate_king_pawn(position, scores, 7, true, trace);
            evaluate_king_pawn(position, scores, 6, true, trace);
        }
        else {
            for (SQUARE_TYPE pawn_file = col - 1; pawn_file < col + 2; pawn_file++) {
                if (position.pawn_rank[0][pawn_file] == 9) {
                    scores.mid += KING_SEMI_OPEN_FILE_PENALTY_MID;
                    scores.end += KING_SEMI_OPEN_FILE_PENALTY_END;
                    trace.king_semi_open[WHITE_COLOR]++;
                    if (position.pawn_rank[1][pawn_file] == 0) {
                        scores.mid += KING_OPEN_FILE_PENALTY_MID;
                        scores.end += KING_OPEN_FILE_PENALTY_END;
                        trace.king_open[WHITE_COLOR]++;
                    }
                }
            }
        }
    }
    else {
        scores.mid += KING_PST_MID[i ^ 56];
        scores.end += KING_PST_END[i ^ 56];
        trace.king_pst[i ^ 56][BLACK_COLOR]++;

        if (col < 4) {  // Queen side
            evaluate_king_pawn(position, scores, 1, false, trace); // A file pawn
            evaluate_king_pawn(position, scores, 2, false, trace);
            evaluate_king_pawn(position, scores, 3, false, trace); // C file pawn
        }
        else if (col > 5) {
            evaluate_king_pawn(position, scores, 8, false, trace); // H file pawn
            evaluate_king_pawn(position, scores, 7, false, trace);
            evaluate_king_pawn(position, scores, 6, false, trace); // F file pawn
        }
        else {
            for (SQUARE_TYPE pawn_file = col - 1; pawn_file < col + 2; pawn_file++) {
                if (position.pawn_rank[1][pawn_file] == 0) {
                    scores.mid += KING_SEMI_OPEN_FILE_PENALTY_MID;
                    scores.end += KING_SEMI_OPEN_FILE_PENALTY_END;
                    trace.king_semi_open[BLACK_COLOR]++;
                    if (position.pawn_rank[0][pawn_file] == 9) {
                        scores.mid += KING_OPEN_FILE_PENALTY_MID;
                        scores.end += KING_OPEN_FILE_PENALTY_END;
                        trace.king_open[BLACK_COLOR]++;
                    }
                }
            }
        }
    }
}


double evaluate_drawishness(const int white_piece_amounts[6], const int black_piece_amounts[6],
                            SCORE_TYPE white_material, SCORE_TYPE black_material, bool opp_colored_bishops) {

    // return a decimal from 0.0 - 1.0 that will multiply the eval by

    if (white_piece_amounts[WHITE_QUEEN] + black_piece_amounts[WHITE_QUEEN] > 0) return 1.0;
    if (white_piece_amounts[WHITE_ROOK] + black_piece_amounts[WHITE_ROOK] >= 3) return 1.0;
    if (white_piece_amounts[WHITE_PAWN] + black_piece_amounts[WHITE_PAWN] >= 1) {
        if (white_piece_amounts[WHITE_PAWN] + black_piece_amounts[WHITE_PAWN] == 1) {
            if (white_material <= PIECE_VALUES_MID[WHITE_PAWN] && black_material <= PIECE_VALUES_MID[WHITE_BISHOP])
                return 0.1;
            if (black_material <= PIECE_VALUES_MID[WHITE_PAWN] && white_material <= PIECE_VALUES_MID[WHITE_BISHOP])
                return 0.1;
        }

        if (white_piece_amounts[WHITE_KNIGHT] + white_piece_amounts[WHITE_ROOK] +
            black_piece_amounts[WHITE_KNIGHT] + black_piece_amounts[WHITE_ROOK] >= 1) return 1.0;

        if (white_piece_amounts[2] == 1 && black_piece_amounts[2] == 1 && opp_colored_bishops) {
            double pawn_difference = static_cast<double>(std::max(white_piece_amounts[0], black_piece_amounts[0])) /
                                     std::max(1, std::min(white_piece_amounts[0], black_piece_amounts[0]));

            return std::min(0.05 + pawn_difference *
                                   pawn_difference * 0.12, 1.0);
        }

        return 1.0;
    }
    if (white_material <= MAX_MINOR_PIECE_VALUE_MID && black_material <= MAX_MINOR_PIECE_VALUE_MID)
        return 0.0;
    if (white_material <= 2 * MAX_MINOR_PIECE_VALUE_MID && black_material <= 2 * MAX_MINOR_PIECE_VALUE_MID) {

        // With only 2 knights, it's impossible to checkmate
        if (white_piece_amounts[WHITE_KNIGHT] == 2 || black_piece_amounts[WHITE_KNIGHT] == 2)
            return 0.0;

        // If one of them has 0 pieces we know that due to the check above, at least one of has more than
        // a bishop's worth of material, or else it would have returned 0.0, and thus it would be a win.
        // Either the player that doesn't have 0 material has two bishops, a bishop and knight, or a rook.
        // All of these are wins.
        if (white_material == 0 || black_material == 0) {
            return 1.0;
        }

        // Here we know they both do not have 0 material, and they cannot have pawns or queens,
        // this means they either have a rook, and the other player has a minor piece,
        // or this means one player has two minor pieces, and the other players has one minor piece.

        return 0.14;
    }

    if (white_material <= PIECE_VALUES_MID[WHITE_ROOK] + MAX_MINOR_PIECE_VALUE_MID &&
        black_material == PIECE_VALUES_MID[WHITE_ROOK]) return 0.27;

    if (black_material <= PIECE_VALUES_MID[WHITE_ROOK] + MAX_MINOR_PIECE_VALUE_MID &&
        white_material == PIECE_VALUES_MID[WHITE_ROOK]) return 0.27;

    return 1.0;

}


SCORE_TYPE evaluate(Position& position, Trace& trace) {

    Score_Struct white_material{};
    Score_Struct white_scores{};

    Score_Struct black_material{};
    Score_Struct black_scores{};

    int game_phase = 0;
    int white_piece_amounts[6] = {0};
    int black_piece_amounts[6] = {0};
    int bishop_colors[2] = {0};

    // We make a 10 size array for each side, and eight of them are used for storing
    // the least advanced pawn. Storing this allows us to check for passed pawns,
    // backwards pawns, isolated pawns and whatnot.
    // Having a ten element array gives padding on the side to prevent out of bounds exceptions.

    for (int i = 0; i < 10; i++) {
        position.pawn_rank[0][i] = 9;
        position.pawn_rank[1][i] = 0;
    }

    for (SQUARE_TYPE pos : position.white_pieces) {
        PIECE_TYPE piece = position.board[pos];
        SQUARE_TYPE i = MAILBOX_TO_STANDARD[pos];
        SQUARE_TYPE row = 8 - i / 8, col = i % 8 + 1;

        if (piece == WHITE_PAWN && row < position.pawn_rank[0][col]) position.pawn_rank[0][col] = row;
    }

    for (SQUARE_TYPE pos : position.black_pieces) {
        PIECE_TYPE piece = position.board[pos];
        SQUARE_TYPE i = MAILBOX_TO_STANDARD[pos];
        SQUARE_TYPE row = 8 - i / 8, col = i % 8 + 1;

        if (piece == BLACK_PAWN && row > position.pawn_rank[1][col]) position.pawn_rank[1][col] = row;
    }

    for (SQUARE_TYPE pos : position.white_pieces) {
        PIECE_TYPE piece = position.board[pos];
        SQUARE_TYPE standard_pos = MAILBOX_TO_STANDARD[pos];

        game_phase += GAME_PHASE_SCORES[piece];
        white_piece_amounts[piece]++;

        white_material.mid += PIECE_VALUES_MID[piece];
        white_material.end += PIECE_VALUES_END[piece];
        trace.material[piece][0]++;
        // std::cout << piece << std::endl;

        // std::cout << white_scores.mid << " " << white_scores.end << std::endl;
        if (piece == WHITE_PAWN) evaluate_pawn(position, white_scores, pos, true, trace);
        else if (piece == WHITE_KNIGHT) evaluate_knight(position, white_scores, pos, true, trace);
        else if (piece == WHITE_BISHOP) {
            bishop_colors[0] = SQUARE_COLOR[standard_pos];
            evaluate_bishop(position, white_scores, pos, true, trace);
        }
        else if (piece == WHITE_ROOK) evaluate_rook(position, white_scores, pos, true, trace);
        else if (piece == WHITE_QUEEN) evaluate_queen(position, white_scores, pos, true, trace);
        else if (piece == WHITE_KING) evaluate_king(position, white_scores, pos, true, trace);

        // std::cout << white_scores.mid << " " << white_scores.end << std::endl;
    }

    for (SQUARE_TYPE pos : position.black_pieces) {
        PIECE_TYPE piece = position.board[pos];
        SQUARE_TYPE standard_pos = MAILBOX_TO_STANDARD[pos];

        game_phase += GAME_PHASE_SCORES[piece-6];
        black_piece_amounts[piece-6]++;

        black_material.mid += PIECE_VALUES_MID[piece - BLACK_PAWN];
        black_material.end += PIECE_VALUES_END[piece - BLACK_PAWN];
        trace.material[piece - BLACK_PAWN][1]++;
        // std::cout << piece << std::endl;

        // std::cout << black_scores.mid << " " << black_scores.end << std::endl;
        if (piece == BLACK_PAWN) evaluate_pawn(position, black_scores, pos, false, trace);
        else if (piece == BLACK_KNIGHT) evaluate_knight(position, black_scores, pos, false, trace);
        else if (piece == BLACK_BISHOP) {
            bishop_colors[1] = SQUARE_COLOR[standard_pos];
            evaluate_bishop(position, black_scores, pos, false, trace);
        }
        else if (piece == BLACK_ROOK) evaluate_rook(position, black_scores, pos, false, trace);
        else if (piece == BLACK_QUEEN) evaluate_queen(position, black_scores, pos, false, trace);
        else if (piece == BLACK_KING) evaluate_king(position, black_scores, pos, false, trace);

        // std::cout << black_scores.mid << " " << black_scores.end << std::endl;
    }

    if (white_piece_amounts[WHITE_BISHOP] >= 2) {
        white_scores.mid += BISHOP_PAIR_BONUS_MID;
        white_scores.end += BISHOP_PAIR_BONUS_END;
        trace.bishop_bonus[0]++;
    }

    if (black_piece_amounts[WHITE_BISHOP] >= 2) {
        black_scores.mid += BISHOP_PAIR_BONUS_MID;
        black_scores.end += BISHOP_PAIR_BONUS_END;
        trace.bishop_bonus[1]++;
    }

    white_scores.mid += white_material.mid;
    white_scores.end += white_material.end;

    black_scores.mid += black_material.mid;
    black_scores.end += black_material.end;

    if (position.side == WHITE_COLOR) {
        white_scores.mid += TEMPO_BONUS_MID;
        white_scores.end += TEMPO_BONUS_END;
        trace.tempo_bonus[WHITE_COLOR]++;
    } else {
        black_scores.mid += TEMPO_BONUS_MID;
        black_scores.end += TEMPO_BONUS_END;
        trace.tempo_bonus[BLACK_COLOR]++;
    }

    if (game_phase > 24) game_phase = 24; // In case of early promotions
    SCORE_TYPE white_score = (white_scores.mid * game_phase +
                              (24 - game_phase) * white_scores.end) / 24;

    SCORE_TYPE black_score = (black_scores.mid * game_phase +
                              (24 - game_phase) * black_scores.end) / 24;

    double drawishness = evaluate_drawishness(white_piece_amounts, black_piece_amounts,
                                              white_material.mid, black_material.mid,
                                              bishop_colors[0] != bishop_colors[1]);

    white_score *= drawishness;
    black_score *= drawishness;
    // std::cout << white_score << " " << black_score << std::endl;

    return (white_score - black_score);
}


static void print_parameter(std::stringstream& ss, const tune_t parameter, bool decimal)
{
    if (decimal) {
        const auto test_param = static_cast<SCORE_TYPE>(parameter + 0.5 - (parameter < 0.0));
        auto param_size = std::to_string(test_param).size();
        for (int i = 0; i < 7 - param_size; i++) {
            ss << " ";
        }
        ss << std::setprecision(3) << parameter;
    } else {
        const auto param = static_cast<SCORE_TYPE>(parameter + 0.5 - (parameter < 0.0));
        auto param_size = std::to_string(param).size();
        for (int i = 0; i < 4 - param_size; i++) {
            ss << " ";
        }
        ss << param;
    }
}

static void print_single(std::stringstream& ss, const parameters_t& parameters, int& index, const std::string& name, bool decimal)
{
    string type = decimal ? " double " : " SCORE_TYPE ";
    ss << "constexpr" << type << name << "_MID = ";
    print_parameter(ss, parameters[index][0], decimal);
    ss << ";" << endl;

    ss << "constexpr" << type << name << "_END = ";
    print_parameter(ss, parameters[index][1], decimal);
    ss << ";\n" << endl;

    index++;
}

static void print_array(std::stringstream& ss, const parameters_t& parameters, int& index, const std::string& name, int count, bool decimal)
{
    for (int c = 0; c < 2; c++) {

        string type = decimal ? " double " : " SCORE_TYPE ";
        if (c == 0) {
            ss << "constexpr" << type << name << "_MID[" << std::to_string(count) << "] = {";
        } else {
            ss << "constexpr" << type << name << "_END[" << std::to_string(count) << "] = {";
        }

        if (count == 64) ss << "\n\t";

        for (auto i = 0; i < count; i++)
        {
            print_parameter(ss, parameters[index][c], decimal);
            index++;

            if (i != count - 1)
            {
                ss << ",";
            }

            if (count == 64 && (i + 1) % 8 == 0 && i != count - 1) {
                ss << "\n\t";
            }
        }
        if (count == 64) ss << "\n";
        ss << "}; \n" << endl;

        if (c == 0) index -= count;
    }

}


static void print_array_2d(std::stringstream& ss, const parameters_t& parameters, int& index, const std::string& name, int count1, int count2, bool decimal)
{
    for (int c = 0; c < 2; c++) {
        string type = decimal ? " double " : " SCORE_TYPE ";
        if (c == 0) {
            ss << "constexpr" << type << name << "_MID[" << std::to_string(count1) << "][" << std::to_string(count2) << "] = {\n";
        } else {
            ss << "constexpr" << type << name << "_END[" << std::to_string(count1) << "][" << std::to_string(count2) << "] = {\n";
        }

        for (auto i = 0; i < count1; i++)
        {
            ss << "\t{";
            for (auto j = 0; j < count2; j++) {
                print_parameter(ss, parameters[index][c], decimal);
                index++;

                if (j != count2 - 1)
                {
                    ss << ",";
                }

                if (count2 == 64 && (j + 1) % 8 == 0 && j != count2 - 1) {
                    ss << "\n\t ";
                }
            }

            ss << "}";
            if (i != count1 - 1)
            {
                ss << ",";
            }
            ss << "\n";
        }
        ss << "}; \n" << endl;

        if (c == 0) index -= count1 * count2;
    }
}


static coefficients_t get_coefficients(const Trace& trace)
{
    coefficients_t coefficients;
    get_coefficient_array(coefficients, trace.material, 6);

    get_coefficient_array(coefficients, trace.pawn_pst, 64);
    get_coefficient_array(coefficients, trace.knight_pst, 64);
    get_coefficient_array(coefficients, trace.bishop_pst, 64);
    get_coefficient_array(coefficients, trace.rook_pst, 64);
    get_coefficient_array(coefficients, trace.queen_pst, 64);
    get_coefficient_array(coefficients, trace.king_pst, 64);

    get_coefficient_array_2d(coefficients, trace.passed_pawns, 3, 8);

    get_coefficient_single(coefficients, trace.isolated_pawns);
    get_coefficient_single(coefficients, trace.isolated_pawns_semi_open_file);

    get_coefficient_single(coefficients, trace.doubled_pawns);

    get_coefficient_single(coefficients, trace.backward_pawns);
    get_coefficient_single(coefficients, trace.backward_pawns_semi_open_file);

    get_coefficient_array(coefficients, trace.pawn_phalanx, 8);

    get_coefficient_array_2d(coefficients, trace.piece_support, 6, 6);
    get_coefficient_array_2d(coefficients, trace.piece_threat, 6, 6);

    get_coefficient_array_2d(coefficients, trace.blockers, 6, 8);
    get_coefficient_array_2d(coefficients, trace.blockers_two_squares, 6, 8);

    get_coefficient_single(coefficients, trace.rook_semi_open);
    get_coefficient_single(coefficients, trace.rook_open);

    get_coefficient_single(coefficients, trace.queen_semi_open);
    get_coefficient_single(coefficients, trace.queen_open);

    get_coefficient_single(coefficients, trace.king_semi_open);
    get_coefficient_single(coefficients, trace.king_open);

    get_coefficient_single(coefficients, trace.bishop_bonus);

    get_coefficient_single(coefficients, trace.tempo_bonus);

    get_coefficient_array(coefficients, trace.knight_mobility, 9);
    get_coefficient_array(coefficients, trace.bishop_mobility, 14);
    get_coefficient_array(coefficients, trace.rook_mobility, 15);
    get_coefficient_array(coefficients, trace.queen_mobility, 28);

    get_coefficient_array(coefficients, trace.knight_control, 64);
    get_coefficient_array(coefficients, trace.bishop_control, 64);
    get_coefficient_array(coefficients, trace.rook_control, 64);
    get_coefficient_array(coefficients, trace.queen_control, 64);

    get_coefficient_array(coefficients, trace.own_king_tropism, 6);
    get_coefficient_array(coefficients, trace.opp_king_tropism, 6);

    get_coefficient_array_2d(coefficients, trace.own_king_pawn_shield, 3, 8);
    get_coefficient_array_2d(coefficients, trace.opp_king_pawn_shield, 3, 8);

    get_coefficient_array_2d(coefficients, trace.king_ring_attacks, 2, 6);

    get_coefficient_single(coefficients, trace.outpost_penalty);
    get_coefficient_single(coefficients, trace.outpost_knight_penalty);
    get_coefficient_single(coefficients, trace.outpost_bishop_penalty);

    get_coefficient_single(coefficients, trace.square_of_the_pawn);

    return coefficients;
}


parameters_t AltairEval::get_initial_parameters() {
    parameters_t parameters;
    get_initial_parameter_array_double(parameters, PIECE_VALUES_MID, PIECE_VALUES_END, 6);

    get_initial_parameter_array_double(parameters, PAWN_PST_MID, PAWN_PST_END, 64);
    get_initial_parameter_array_double(parameters, KNIGHT_PST_MID, KNIGHT_PST_END, 64);
    get_initial_parameter_array_double(parameters, BISHOP_PST_MID, BISHOP_PST_END, 64);
    get_initial_parameter_array_double(parameters, ROOK_PST_MID, ROOK_PST_END, 64);
    get_initial_parameter_array_double(parameters, QUEEN_PST_MID, QUEEN_PST_END, 64);
    get_initial_parameter_array_double(parameters, KING_PST_MID, KING_PST_END, 64);

    get_initial_parameter_array_2d_double(parameters, PASSED_PAWN_BONUSES_MID, PASSED_PAWN_BONUSES_END, 3, 8);

    get_initial_parameter_single_double(parameters, ISOLATED_PAWN_PENALTY_MID, ISOLATED_PAWN_PENALTY_END);
    get_initial_parameter_single_double(parameters, ISOLATED_PAWN_SEMI_OPEN_FILE_PENALTY_MID, ISOLATED_PAWN_SEMI_OPEN_FILE_PENALTY_END);

    get_initial_parameter_single_double(parameters, DOUBLED_PAWN_PENALTY_MID, DOUBLED_PAWN_PENALTY_END);

    get_initial_parameter_single_double(parameters, BACKWARDS_PAWN_PENALTY_MID, BACKWARDS_PAWN_PENALTY_END);
    get_initial_parameter_single_double(parameters, BACKWARDS_PAWN_SEMI_OPEN_FILE_PENALTY_MID, BACKWARDS_PAWN_SEMI_OPEN_FILE_PENALTY_END);

    get_initial_parameter_array_double(parameters, PHALANX_PAWN_BONUS_MID, PHALANX_PAWN_BONUS_END, 8);

    get_initial_parameter_array_2d_double(parameters, PIECE_SUPPORT_MID, PIECE_SUPPORT_END, 6, 6);
    get_initial_parameter_array_2d_double(parameters, PIECE_THREAT_MID, PIECE_THREAT_END, 6, 6);

    get_initial_parameter_array_2d_double(parameters, BLOCKER_VALUES_MID, BLOCKER_VALUES_END, 6, 8);
    get_initial_parameter_array_2d_double(parameters, BLOCKER_TWO_SQUARE_VALUES_MID, BLOCKER_TWO_SQUARE_VALUES_END, 6, 8);

    get_initial_parameter_single_double(parameters, ROOK_SEMI_OPEN_FILE_BONUS_MID, ROOK_SEMI_OPEN_FILE_BONUS_END);
    get_initial_parameter_single_double(parameters, ROOK_OPEN_FILE_BONUS_MID, ROOK_OPEN_FILE_BONUS_END);

    get_initial_parameter_single_double(parameters, QUEEN_SEMI_OPEN_FILE_BONUS_MID, QUEEN_SEMI_OPEN_FILE_BONUS_END);
    get_initial_parameter_single_double(parameters, QUEEN_OPEN_FILE_BONUS_MID, QUEEN_OPEN_FILE_BONUS_END);

    get_initial_parameter_single_double(parameters, KING_SEMI_OPEN_FILE_PENALTY_MID, KING_SEMI_OPEN_FILE_PENALTY_END);
    get_initial_parameter_single_double(parameters, KING_OPEN_FILE_PENALTY_MID, KING_OPEN_FILE_PENALTY_END);

    get_initial_parameter_single_double(parameters, BISHOP_PAIR_BONUS_MID, BISHOP_PAIR_BONUS_END);

    get_initial_parameter_single_double(parameters, TEMPO_BONUS_MID, TEMPO_BONUS_END);

    get_initial_parameter_array_double(parameters, KNIGHT_MOBILITY_MID, KNIGHT_MOBILITY_END, 9);
    get_initial_parameter_array_double(parameters, BISHOP_MOBILITY_MID, BISHOP_MOBILITY_END, 14);
    get_initial_parameter_array_double(parameters, ROOK_MOBILITY_MID, ROOK_MOBILITY_END, 15);
    get_initial_parameter_array_double(parameters, QUEEN_MOBILITY_MID, QUEEN_MOBILITY_END, 28);

    get_initial_parameter_array_double(parameters, KNIGHT_CONTROL_MID, KNIGHT_CONTROL_END, 64);
    get_initial_parameter_array_double(parameters, BISHOP_CONTROL_MID, BISHOP_CONTROL_END, 64);
    get_initial_parameter_array_double(parameters, ROOK_CONTROL_MID, ROOK_CONTROL_END, 64);
    get_initial_parameter_array_double(parameters, QUEEN_CONTROL_MID, QUEEN_CONTROL_END, 64);

    get_initial_parameter_array_double(parameters, OWN_KING_DISTANCE_COEFFICIENTS_MID, OWN_KING_DISTANCE_COEFFICIENTS_END, 6);
    get_initial_parameter_array_double(parameters, OPP_KING_DISTANCE_COEFFICIENTS_MID, OPP_KING_DISTANCE_COEFFICIENTS_END, 6);

    get_initial_parameter_array_2d_double(parameters, KING_PAWN_SHIELD_OWN_PENALTIES_MID, KING_PAWN_SHIELD_OWN_PENALTIES_END, 3, 8);
    get_initial_parameter_array_2d_double(parameters, KING_PAWN_SHIELD_OPP_PENALTIES_MID, KING_PAWN_SHIELD_OPP_PENALTIES_END, 3, 8);

    get_initial_parameter_array_2d_double(parameters, KING_RING_ATTACKS_MID, KING_RING_ATTACKS_END, 2, 6);

    get_initial_parameter_single_double(parameters, OUTPOST_PENALTY_MID, OUTPOST_PENALTY_END);
    get_initial_parameter_single_double(parameters, OUTPOST_KNIGHT_PENALTY_MID, OUTPOST_KNIGHT_PENALTY_END);
    get_initial_parameter_single_double(parameters, OUTPOST_BISHOP_PENALTY_MID, OUTPOST_BISHOP_PENALTY_END);

    get_initial_parameter_single_double(parameters, SQUARE_OF_THE_PAWN_MID, SQUARE_OF_THE_PAWN_END);

    return parameters;
}


EvalResult AltairEval::get_fen_eval_result(const string &fen) {
    Position position;
    position.set_fen(fen);

    Trace trace{};
    trace.score = evaluate(position, trace);

    EvalResult result;
    result.coefficients = get_coefficients(trace);
    result.score = trace.score;

    return result;
}


void AltairEval::print_parameters(const parameters_t &parameters) {
    parameters_t parameters_copy = parameters;

    int index = 0;
    stringstream ss;

    print_array(ss, parameters_copy, index, "PIECE_VALUES", 6, false);

    print_array(ss, parameters_copy, index, "PAWN_PST", 64, false);
    print_array(ss, parameters_copy, index, "KNIGHT_PST", 64, false);
    print_array(ss, parameters_copy, index, "BISHOP_PST", 64, false);
    print_array(ss, parameters_copy, index, "ROOK_PST", 64, false);
    print_array(ss, parameters_copy, index, "QUEEN_PST", 64, false);
    print_array(ss, parameters_copy, index, "KING_PST", 64, false);

    print_array_2d(ss, parameters_copy, index, "PASSED_PAWN_BONUSES", 3, 8, false);

    print_single(ss, parameters_copy, index, "ISOLATED_PAWN_PENALTY", false);
    print_single(ss, parameters_copy, index, "ISOLATED_PAWN_SEMI_OPEN_FILE_PENALTY", false);

    print_single(ss, parameters_copy, index, "DOUBLED_PAWN_PENALTY", false);

    print_single(ss, parameters_copy, index, "BACKWARDS_PAWN_PENALTY", false);
    print_single(ss, parameters_copy, index, "BACKWARDS_PAWN_SEMI_OPEN_FILE_PENALTY", false);

    print_array(ss, parameters_copy, index, "PHALANX_PAWN_BONUS", 8, false);

    print_array_2d(ss, parameters_copy, index, "PIECE_SUPPORT", 6, 6, false);
    print_array_2d(ss, parameters_copy, index, "PIECE_THREAT", 6, 6, false);

    print_array_2d(ss, parameters_copy, index, "BLOCKER_VALUES", 6, 8, false);
    print_array_2d(ss, parameters_copy, index, "BLOCKER_TWO_SQUARE_VALUES", 6, 8, false);

    print_single(ss, parameters_copy, index, "ROOK_SEMI_OPEN_FILE_BONUS", false);
    print_single(ss, parameters_copy, index, "ROOK_OPEN_FILE_BONUS", false);

    print_single(ss, parameters_copy, index, "QUEEN_SEMI_OPEN_FILE_BONUS", false);
    print_single(ss, parameters_copy, index, "QUEEN_OPEN_FILE_BONUS", false);

    print_single(ss, parameters_copy, index, "KING_SEMI_OPEN_FILE_PENALTY", false);
    print_single(ss, parameters_copy, index, "KING_OPEN_FILE_PENALTY", false);

    print_single(ss, parameters_copy, index, "BISHOP_PAIR_BONUS", false);

    print_single(ss, parameters_copy, index, "TEMPO_BONUS", false);

    print_array(ss, parameters_copy, index, "KNIGHT_MOBILITY", 9, false);
    print_array(ss, parameters_copy, index, "BISHOP_MOBILITY", 14, false);
    print_array(ss, parameters_copy, index, "ROOK_MOBILITY", 15, false);
    print_array(ss, parameters_copy, index, "QUEEN_MOBILITY", 28, false);

    print_array(ss, parameters_copy, index, "KNIGHT_CONTROL", 64, false);
    print_array(ss, parameters_copy, index, "BISHOP_CONTROL", 64, false);
    print_array(ss, parameters_copy, index, "ROOK_CONTROL", 64, false);
    print_array(ss, parameters_copy, index, "QUEEN_CONTROL", 64, false);

    print_array(ss, parameters_copy, index, "OWN_KING_DISTANCE_COEFFICIENTS", 6, true);
    print_array(ss, parameters_copy, index, "OPP_KING_DISTANCE_COEFFICIENTS", 6, true);

    print_array_2d(ss, parameters_copy, index, "KING_PAWN_SHIELD_OWN_PENALTIES", 3, 8, false);
    print_array_2d(ss, parameters_copy, index, "KING_PAWN_SHIELD_OPP_PENALTIES", 3, 8, false);

    print_array_2d(ss, parameters_copy, index, "KING_RING_ATTACKS", 2, 6, false);

    print_single(ss, parameters_copy, index, "OUTPOST_PENALTY", false);
    print_single(ss, parameters_copy, index, "OUTPOST_KNIGHT_PENALTY", false);
    print_single(ss, parameters_copy, index, "OUTPOST_BISHOP_PENALTY", false);

    print_single(ss, parameters_copy, index, "SQUARE_OF_THE_PAWN", false);

    std::cout << ss.str() << "\n";
}
