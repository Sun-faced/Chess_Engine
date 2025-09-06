#pragma once

#include <cstdint>
#include <array>
#include "random.hpp"

class Zobrist {
public:
  static constexpr int NUM_PIECES = 12;  // 6 piece types * 2 colors
  static constexpr int NUM_SQUARES = 64; // 8x8 board
  static constexpr int NUM_CASTLE_STATES = 16; // 2^4 possible castle combinations

  uint64_t side_key;
  std::array<uint64_t, NUM_SQUARES> enp_keys;
  std::array<uint64_t, NUM_CASTLE_STATES> castle_keys;
  std::array<std::array<uint64_t, NUM_SQUARES>, NUM_PIECES> piece_keys;

  constexpr Zobrist() : side_key{}, enp_keys{}, castle_keys{}, piece_keys{} {
    RandomNumberGenerator rand{UINT64_C(1070372)};
    
    side_key = rand.generateNext();

    for (auto& key : enp_keys) {
      key = rand.generateNext();
    }

    for (auto& key : castle_keys) {
      key = rand.generateNext();
    }

    for (auto& piece_array : piece_keys) {
      for (auto& key : piece_array) {
        key = rand.generateNext();
      }
    }
  }
};

constexpr Zobrist zobrist_table{};
