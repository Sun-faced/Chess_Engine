#pragma once

#include <cstdint>
#include <iostream>
#include <array>
#include "enums.hpp"

static constexpr uint64_t FILE_A  = 0x0101010101010101;
static constexpr uint64_t FILE_H  = 0x8080808080808080;
static constexpr uint64_t FILE_AB = 0x0303030303030303;
static constexpr uint64_t FILE_GH = 0xC0C0C0C0C0C0C0C0;

static constexpr uint64_t NOT_FILE_A  = ~FILE_A;
static constexpr uint64_t NOT_FILE_H  = ~FILE_H;
static constexpr uint64_t NOT_FILE_AB = ~FILE_AB;
static constexpr uint64_t NOT_FILE_GH = ~FILE_GH;

static constexpr uint64_t square_bb(int square) {
    return 1ull << square;
}

template<typename T>
struct PieceAttackGenerator {};

template<>
struct PieceAttackGenerator<PieceType> {
    static constexpr uint64_t generate_mask(Side side, int square, PieceType piece);
    static constexpr uint64_t generate_attacks(int square, uint64_t blockers, PieceType piece);
};

template<>
struct PieceAttackGenerator<SlidingPiece> {
  static constexpr uint64_t generate_mask(int square, SlidingPiece piece);
  static constexpr uint64_t generate_attacks(int square, uint64_t blockers, SlidingPiece piece);
};


static constexpr uint64_t generate_pawn_attacks(Side side, int square) {
  uint64_t board = square_bb(square);
  
  if (side == Side::white) {
    return ((board >> 7) & NOT_FILE_A) | ((board >> 9) & NOT_FILE_H);
  }
  return ((board << 9) & NOT_FILE_A) | ((board << 7) & NOT_FILE_H);
}

static constexpr uint64_t generate_knight_attacks(int square) {
  uint64_t board = square_bb(square);
  uint64_t attacks = 0;

  if ((board >> 17) & NOT_FILE_H)  attacks |= board >> 17;
  if ((board >> 15) & NOT_FILE_A)  attacks |= board >> 15;
  if ((board >> 10) & NOT_FILE_GH) attacks |= board >> 10;
  if ((board >> 6)  & NOT_FILE_AB) attacks |= board >> 6;
  if ((board << 17) & NOT_FILE_A)  attacks |= board << 17;
  if ((board << 15) & NOT_FILE_H)  attacks |= board << 15;
  if ((board << 10) & NOT_FILE_AB) attacks |= board << 10;
  if ((board << 6)  & NOT_FILE_GH) attacks |= board << 6;

  return attacks;
}

static constexpr uint64_t generate_king_attacks(int square) {
  uint64_t board = square_bb(square);
  uint64_t attacks = 0;

  attacks |= board >> 8;
  attacks |= (board >> 9) & NOT_FILE_H;
  attacks |= (board >> 7) & NOT_FILE_A;
  attacks |= (board >> 1) & NOT_FILE_H;
  attacks |= board << 8;
  attacks |= (board << 9) & NOT_FILE_A;
  attacks |= (board << 7) & NOT_FILE_H;
  attacks |= (board << 1) & NOT_FILE_A;

  return attacks;
}

static constexpr uint64_t generate_bishop_mask(int square) {
  uint64_t attacks = 0;
  int rank = square / 8;
  int file = square % 8;

  constexpr int dirs[4][2] = {{1, 1}, {-1, 1}, {1, -1}, {-1, -1}};
  
  for (int i = 0; i < 4; i++) {
    int r = rank + dirs[i][0];
    int f = file + dirs[i][1];

    while (r >= 1 && r <= 6 && f >= 1 && f <= 6) {
      attacks |= square_bb(r * 8 + f);
      r += dirs[i][0];
      f += dirs[i][1];
    }
  }
  return attacks;
}

static constexpr uint64_t generate_bishop_attacks(int square, uint64_t blockers) {
  uint64_t attacks = 0;
  int rank = square / 8;
  int file = square % 8;

  constexpr int dirs[4][2] = {{1, 1}, {-1, 1}, {1, -1}, {-1, -1}};
  
  for (int i = 0; i < 4; i++) {
    int r = rank + dirs[i][0];
    int f = file + dirs[i][1];

    while (r >= 0 && r <= 7 && f >= 0 && f <= 7) {
      uint64_t bit = square_bb(r * 8 + f);
      attacks |= bit;
      if (bit & blockers) break;
      r += dirs[i][0];
      f += dirs[i][1];
    }
  }
  return attacks;
}

static constexpr uint64_t generate_rook_attacks(int square, uint64_t blockers) {
  uint64_t attacks = 0;
  int rank = square / 8;
  int file = square % 8;

  constexpr int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
  
  for (int i = 0; i < 4; i++) {
    int r = rank + dirs[i][0];
    int f = file + dirs[i][1];

    while (r >= 0 && r <= 7 && f >= 0 && f <= 7) {
      uint64_t bit = square_bb(r * 8 + f);
      attacks |= bit;
      if (bit & blockers) break;
      r += dirs[i][0];
      f += dirs[i][1];
    }
  }
  return attacks;
}

static constexpr uint64_t generate_rook_mask(int square) {
    uint64_t attacks = 0;
    int rank = square / 8;
    int file = square % 8;
    
    for (int r = 1; r < 7; r++) {
        if (r != rank) {
          attacks |= square_bb(r * 8 + file);
        }
    }
    
    for (int f = 1; f < 7; f++) {
        if (f != file) {
          attacks |= square_bb(rank * 8 + f);
        }
    }
    return attacks;
}

constexpr uint64_t PieceAttackGenerator<PieceType>::generate_mask(Side side, int square, PieceType piece) {
  switch (piece) {
    case PieceType::pawn:   return generate_pawn_attacks(side, square);
    case PieceType::knight: return generate_knight_attacks(square);
    case PieceType::bishop: return generate_bishop_mask(square);
    case PieceType::rook:   return generate_rook_mask(square);
    case PieceType::king:   return generate_king_attacks(square);
    case PieceType::queen:  return generate_bishop_mask(square) | generate_rook_mask(square);
  }
  return 0;
}

constexpr uint64_t PieceAttackGenerator<SlidingPiece>::generate_attacks(int square, uint64_t blockers, SlidingPiece piece) {
  switch (piece) {
    case SlidingPiece::bishop: return generate_bishop_attacks(square, blockers);
    case SlidingPiece::rook:   return generate_rook_attacks(square, blockers);
    case SlidingPiece::queen:  return generate_bishop_attacks(square, blockers) | generate_rook_attacks(square, blockers);
  }
  return 0;
}

class bb {
private:
  uint64_t board;

public:
  constexpr bb() : board(0) {}
  explicit bb(uint64_t val);
  
  constexpr bb(Side side, int square, PieceType piece) 
    : board(PieceAttackGenerator<PieceType>::generate_mask(side, square, piece)) {}

  constexpr bb(int index, const bb& mask) : board(0) {
    int bit_count = mask.count_bits();
    bb temp_mask = mask;

    for (int bit_index = 0; bit_index < bit_count; bit_index++) {
      int square = temp_mask.get_lsb_index();
      temp_mask.pop_bit(square);
      if (index & (1 << bit_index)) {
        board |= square_bb(square);
      }
    }
  }

  constexpr bb(int square, const bb& blockers, SlidingPiece piece) 
    : board(PieceAttackGenerator<SlidingPiece>::generate_attacks(square, blockers.get_board(), piece)) {}

  constexpr uint64_t get_board() const { return board; }
  
  constexpr void set_bit(int square) { board |= square_bb(square); }
  constexpr void pop_bit(int square) { board &= ~square_bb(square); }
  constexpr bool get_bit(int square) const { return (board >> square) & 1; }
  constexpr int count_bits() const { return __builtin_popcountll(board); }
  constexpr int get_lsb_index() const { return __builtin_ctzll(board); }

  bb& operator^=(uint64_t other) {
    board ^= other;
    return *this;
  }

  bb& operator|=(bb other) {
    board |= other.board;
    return *this;
  }

  bb& operator&=(bb other) {
    board &= other.board;
    return *this;
  }

  void print_bb() const;
};