//
// Created by Alex Tian on 2/24/2023.
//

#ifndef TUNER_ALTAIR_H
#define TUNER_ALTAIR_H

#include "../base.h"
#include <string>
#include <vector>
#include <cstdint>
#include "types.h"
#include "bitboard.h"

class Position {

public:

    Position() = default;

    BITBOARD all_pieces{};
    BITBOARD our_pieces{};
    BITBOARD opp_pieces{};
    BITBOARD empty_squares{};

    BITBOARD pieces[12]{};

    Piece board[64]{};

    Color side = WHITE;

    uint8_t castle_ability_bits = 0;
    Square ep_square = NO_SQUARE;

    [[nodiscard]] BITBOARD get_pieces(Piece piece) const;
    [[nodiscard]] BITBOARD get_pieces(PieceType piece, Color color) const;
    [[nodiscard]] BITBOARD get_pieces(Color color) const;

    [[nodiscard]] BITBOARD get_our_pieces();
    [[nodiscard]] BITBOARD get_opp_pieces();
    [[nodiscard]] BITBOARD get_all_pieces() const;
    [[nodiscard]] BITBOARD get_empty_squares() const;

    [[nodiscard]] Square get_king_pos(Color color) const;

    void remove_piece(Piece piece, Square square);
    void place_piece(Piece piece, Square square);

    PLY_TYPE set_fen(const std::string& fen);

};

constexpr char PIECE_MATCHER[12] = {'P', 'N', 'B', 'R', 'Q', 'K', 'p', 'n', 'b', 'r', 'q', 'k'};
constexpr int GAME_PHASE_SCORES[6] = {0, 1, 1, 2, 4, 0};

struct Trace {
    int score={};

    short piece_values[6][2]{};
    short piece_square_tables[6][64][2]{};

    short passed_pawn_bonuses[3][8][2]{};
    short passed_pawn_blockers[6][8][2]{};
    short passed_pawn_blockers_2[6][8][2]{};

    short phalanx_pawn_bonuses[8][2]{};

    short isolated_pawn_penalty[2]{};

    short bishop_pair_bonus[2]{};

    short tempo_bonus[2]{};

    short mobility_values[6][2]{};

    short semi_open_file_values[6][2]{};
    short open_file_values[6][2]{};
};

namespace Altair {
    class AltairEval
    {
    public:
        constexpr static bool includes_additional_score = true;
        static parameters_t get_initial_parameters();
        static EvalResult get_fen_eval_result(const std::string& fen);
        static void print_parameters(const parameters_t& parameters);
    };
}


#endif //TUNER_ALTAIR_H
