#include "evaluation.hpp"
#include "enums.hpp"
#include "./nnue/nnue.h"

static inline int piece_idx_to_nnue_idx(int idx) {
  return (idx < 6) ? (6 - idx) : (18 - idx);
}

static inline int sq_to_nnue_sq(int sq) {
  return (7 - (sq / 8)) * 8 + (sq % 8);
}

int evaluate(const Board& board) {
  int pieces[33];
  int squares[33];
  int index = 2;
  
  bb white_king_bb = board.get_piece_bitboard_by_idx(5);
  pieces[0] = piece_idx_to_nnue_idx(5);
  squares[0] = sq_to_nnue_sq(white_king_bb.get_lsb_index());
  
  bb black_king_bb = board.get_piece_bitboard_by_idx(11);
  pieces[1] = piece_idx_to_nnue_idx(5);
  squares[1] = sq_to_nnue_sq(black_king_bb.get_lsb_index());
  
  for (int piece_idx = 0; piece_idx < NUMBER_OF_UNIQUE_PIECES * NUMBER_OF_SIDES; ++piece_idx) {
    if (piece_idx == 5 || piece_idx == 11) {
      continue;
    }
    
    bb piece_bb = board.get_piece_bitboard_by_idx(piece_idx);
    
    if (!piece_bb.get_board()) {
      continue;
    }
    
    const int nnue_piece = piece_idx_to_nnue_idx(piece_idx);

    while (piece_bb.get_board()) {
      const int sq = piece_bb.get_lsb_index();
      pieces[index] = nnue_piece;
      squares[index] = sq_to_nnue_sq(sq);
      index++;
      piece_bb.pop_bit(sq);
    }
  }

  pieces[index] = 0;
  squares[index] = 0;

  return nnue_evaluate(static_cast<int>(board.get_side()), pieces, squares) * (100 - board.get_fifty_move_counter()) / 100;
}
