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

typedef unsigned __int128 uint128_t;

inline int centerDistance(int square)
{
    return (int)(2 * ((0xFFFFC3C3C3C3FFFF >> square) & 1) + ((0xFF81BDA5A5BD81FF >> square) & 1));
}

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

struct Trace {
    int score={};

    short piece_values[6][2]{};
    short piece_square_tables[6][64][2]{};

    short bishop_pair[2]{};
    // short mobility[4][2]{};

    short tempo[2];
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
