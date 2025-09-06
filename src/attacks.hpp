#pragma once

#include <iostream>
#include <array>
#include "bitboard.hpp"
#include "enums.hpp"
#include "magics.hpp"

constexpr int NUMBER_OF_COLOR_DEPENDENT_PIECES = 1;

template<PieceType piece, Side side = Side::any>
constexpr std::array<bb, 64> generate_mask_table() {
    std::array<bb, 64> table{};
    for (int square = 0; square < 64; square++) {
        table[square] = bb(side, square, piece);
    }
    return table;
}

constexpr std::array<bb, 64> white_pawn_masks = generate_mask_table<PieceType::pawn, Side::white>();
constexpr std::array<bb, 64> black_pawn_masks = generate_mask_table<PieceType::pawn, Side::black>();
constexpr std::array<std::array<bb, 64>, 2> pawn_masks = {
  white_pawn_masks,
  black_pawn_masks
};
constexpr std::array<bb, 64> knight_masks = generate_mask_table<PieceType::knight>();
constexpr std::array<bb, 64> king_masks = generate_mask_table<PieceType::king>();
constexpr std::array<bb, 64> bishop_masks = generate_mask_table<PieceType::bishop>();
constexpr std::array<bb, 64> rook_masks = generate_mask_table<PieceType::rook>();

template<SlidingPiece piece, size_t TableSize>
constexpr std::array<std::array<bb, TableSize>, 64> generate_sliding_table() {
  std::array<std::array<bb, TableSize>, 64> table{};
  constexpr auto masks = generate_mask_table<to_normal_pieces(piece)>();
  
  for (int square = 0; square < 64; square++) {
      int blocker_combinations = 1 << masks[square].count_bits();
      for (int index = 0; index < blocker_combinations; ++index) {
          bb blockers(index, masks[square]);
          int magic_idx = magic_index(square, blockers, piece);
          table[square][magic_idx] = bb(square, blockers, piece);
      }
  }
  return table;
}

constexpr std::array<std::array<bb, 512>, 64> bishop_attacks = 
  generate_sliding_table<SlidingPiece::bishop, 512>();

constexpr std::array<std::array<bb, 4096>, 64> rook_attacks = 
  generate_sliding_table<SlidingPiece::rook, 4096>();


template <size_t AttackTableSize>
struct SliderTables {
  const std::array<bb, 64>& masks;
  const std::array<uint64_t, 64>& magic_numbers;
  const std::array<int, 64>& relevant_bits;
  const std::array<std::array<bb, AttackTableSize>, 64>& attacks;
};

constexpr SliderTables<512> bishop_tables{
  bishop_masks,
  bishop_magic_numbers,
  bishop_relevant_bits,
  bishop_attacks
};

constexpr SliderTables<4096> rook_tables{
  rook_masks,
  rook_magic_numbers,
  rook_relevant_bits,
  rook_attacks
};

template <size_t AttackTableSize>
static inline bb get_slider_attacks(int square, bb occupancy, const SliderTables<AttackTableSize>& tables)
{
  uint64_t block = occupancy.get_board();
  block &= tables.masks[square].get_board();
  block *= tables.magic_numbers[square];
  block >>= 64 - tables.relevant_bits[square];
  return tables.attacks[square][block];
}

static inline bb get_bishop_attacks(int square, bb occupancy) {
  return get_slider_attacks(square, occupancy, bishop_tables);
}

static inline bb get_rook_attacks(int square, bb occupancy) {
  return get_slider_attacks(square, occupancy, rook_tables);
}

static inline bb get_queen_attacks(int square, bb occupancy) {
  return bb(get_bishop_attacks(square, occupancy).get_board() |
            get_rook_attacks(square, occupancy).get_board());
}

