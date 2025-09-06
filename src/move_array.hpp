#pragma once

#include <iostream>
#include "move.hpp"

class MoveArray {
private:
  static constexpr int MAX_MOVES = 218;
  size_t arr_size = 0;
  std::array<Move, MAX_MOVES> moves{};
public:
  inline size_t size() const {
    return arr_size;
  }

  inline void push(const Move move) {
    moves[arr_size++] = move;
  }

  inline Move get(const int i) const {
    return moves[i];
  }

  inline void swap(const int i1, const int i2) {
    std::swap(moves[i1], moves[i2]);
  }
};