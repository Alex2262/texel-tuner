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

inline bool same_color (Square square_1, Square square_2) {
    return (( 9 * (square_1 ^ square_2)) & 8) == 0;
}

inline int get_manhattan_distance(Square square_1, Square square_2) {
    return abs(static_cast<int>(rank_of(square_1)) - static_cast<int>(rank_of(square_2))) +
           abs(static_cast<int>(file_of(square_1)) - static_cast<int>(file_of(square_2)));
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

    short piece_threats[6][6][2]{};

    short king_ring_attacks[2][6][2]{};
    short total_king_ring_attacks[30][2]{};

    short king_pawn_shield[5][8][2]{};
    short king_pawn_storm[6][8][2]{};

    short opp_king_tropism[6][2]{};

    short doubled_pawn_penalty[2]{};
};

template<int n>
struct KingRing {
    constexpr KingRing() : masks() {
        for (int rings = 1; rings <= n; rings++) {
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    masks[rings-1][i * 8 + j] = 0ULL;
                    for (int y = i - rings; y <= i + rings; y++){
                        for (int x = j - rings; x <= j + rings; x++) {
                            if (y < 0 || y >= 8 || x < 0 || x >= 8) continue;
                            if (y == i - rings || y == i + rings || x == j - rings || x == j + rings) {
                                masks[rings-1][i * 8 + j] |= 1ULL << (y * 8 + x);
                            }
                        }
                    }
                    //std::cout << i * 8 + j << " " << masks[rings-1][i * 8 + j] << std::endl;
                }
            }
        }
    }

    uint64_t masks[n][64]{};
};

constexpr KingRing king_ring_zone = KingRing<2>();

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
