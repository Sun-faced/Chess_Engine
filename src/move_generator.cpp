#include "move_generator.hpp"

static inline void add_move(MoveArray& moves, uint32_t from, uint32_t to, 
                           PieceType piece, Side piece_side, 
                           PieceType promo_piece = PieceType::pawn, Side promo_side = Side::any,
                           MoveFlag flag = MoveFlag::NO_FLAG, bool is_capture = false) {
  moves.push(Move(from, to, piece, piece_side, promo_piece, promo_side, flag, is_capture));
}

static inline void add_promotion_moves(MoveArray& moves, int from, int to, Side side, 
                                      bool is_capture = false) {
  add_move(moves, from, to, PieceType::pawn, side, PieceType::queen, side, MoveFlag::NO_FLAG, is_capture);
  add_move(moves, from, to, PieceType::pawn, side, PieceType::rook, side, MoveFlag::NO_FLAG, is_capture);
  add_move(moves, from, to, PieceType::pawn, side, PieceType::bishop, side, MoveFlag::NO_FLAG, is_capture);
  add_move(moves, from, to, PieceType::pawn, side, PieceType::knight, side, MoveFlag::NO_FLAG, is_capture);
}

static inline void handle_pawn_push(MoveArray& moves, int from_sq, int to_sq, Side side, int rank, int promo_rank) {
  if (rank == promo_rank) {
    add_promotion_moves(moves, from_sq, to_sq, side);
    return;
  }
  
  add_move(moves, from_sq, to_sq, PieceType::pawn, side);
}

static inline void handle_double_pawn_push(MoveArray& moves, const Board& board, int from_sq, int direction, 
                                          Side side, int rank, int start_rank) {
  if (rank != start_rank) {
    return;
  }
  
  int double_push_sq = from_sq + 2 * direction;
  bb all_pieces = board.get_all_occupancy();
  if (all_pieces.get_bit(double_push_sq)) {
    return;
  }
  
  add_move(moves, from_sq, double_push_sq, PieceType::pawn, side, 
          PieceType::pawn, Side::any, MoveFlag::PAWN_START);
}

static inline void handle_pawn_captures(MoveArray& moves, int from_sq, Side side, int rank, int promo_rank, bb enemy_pieces) {
  bb pawn_attacks = pawn_masks[static_cast<int>(side)][from_sq];
  bb capture_targets = bb(pawn_attacks.get_board() & enemy_pieces.get_board());
  
  while (capture_targets.get_board() != 0) {
    int capture_sq = capture_targets.get_lsb_index();
    capture_targets.pop_bit(capture_sq);
    
    if (rank == promo_rank) {
      add_promotion_moves(moves, from_sq, capture_sq, side, true);
      continue;
    }
    add_move(moves, from_sq, capture_sq, PieceType::pawn, side, 
            PieceType::pawn, Side::any, MoveFlag::NO_FLAG, true);
  }
}

static inline void handle_en_passant(MoveArray& moves, const Board& board, int from_sq, Side side) {
  auto enpassant = board.get_enpassant();
  if (!enpassant.first) {
    return;
  }
  
  bb pawn_attacks = pawn_masks[static_cast<int>(side)][from_sq];
  bb enpassant_attacks = bb(pawn_attacks.get_board() & 1ULL << enpassant.second);
  if (enpassant_attacks.get_board() == 0) {
    return;
  }
  
  add_move(moves, from_sq, enpassant.second, PieceType::pawn, side, 
          PieceType::pawn, Side::any, MoveFlag::EN_PASSANT, true);
}

static inline void generate_pawn_moves(MoveArray& moves, const Board& board, Side side) {
  bb pawns = board.get_piece_bitboard(PieceType::pawn, side);
  bb all_pieces = board.get_all_occupancy();
  bb enemy_pieces = board.get_occupancy(opposite_side(side));
  
  int side_val = static_cast<int>(side);
  int direction = -8 + 16 * side_val;  // white=-8, black=8
  int start_rank = 6 - 5 * side_val;   // white=6, black=1
  int promo_rank = 1 + 5 * side_val;   // white=1, black=6
  
  while (pawns.get_board() != 0) {
    int from_sq = pawns.get_lsb_index();
    pawns.pop_bit(from_sq);
    
    int rank = from_sq / 8;
    int to_sq = from_sq + direction;
    
    if (!all_pieces.get_bit(to_sq)) {
      handle_pawn_push(moves, from_sq, to_sq, side, rank, promo_rank);
      handle_double_pawn_push(moves, board, from_sq, direction, side, rank, start_rank);
    }
    
    handle_pawn_captures(moves, from_sq, side, rank, promo_rank, enemy_pieces);
    
    handle_en_passant(moves, board, from_sq, side);
  }
}

template<PieceType piece_type>
static inline void generate_piece_moves(MoveArray& moves, const Board& board, Side side) {
  bb pieces = board.get_piece_bitboard(piece_type, side);
  bb own_pieces = board.get_occupancy(side);
  bb enemy_pieces = board.get_occupancy(opposite_side(side));
  
  while (pieces.get_board() != 0) {
    int from_sq = pieces.get_lsb_index();
    pieces.pop_bit(from_sq);
    
    bb attacks;
    if constexpr (piece_type == PieceType::knight) {
      attacks = knight_masks[from_sq];
    } else if constexpr (piece_type == PieceType::king) {
      attacks = king_masks[from_sq];
    } else if constexpr (piece_type == PieceType::bishop) {
      attacks = get_bishop_attacks(from_sq, board.get_all_occupancy());
    } else if constexpr (piece_type == PieceType::rook) {
      attacks = get_rook_attacks(from_sq, board.get_all_occupancy());
    } else if constexpr (piece_type == PieceType::queen) {
      attacks = get_queen_attacks(from_sq, board.get_all_occupancy());
    }
    
    attacks &= bb(~own_pieces.get_board());
    
    while (attacks.get_board() != 0) {
      int to_sq = attacks.get_lsb_index();
      attacks.pop_bit(to_sq);
      
      bool is_capture = enemy_pieces.get_bit(to_sq);
      add_move(moves, from_sq, to_sq, piece_type, side, 
              PieceType::pawn, Side::any, MoveFlag::NO_FLAG, is_capture);
    }
  }
}

static inline bool can_castle_side(const Board& board, Side side, bool is_kingside, bb all_pieces, Side enemy_side) {
  int side_val = static_cast<int>(side);
  
  CastlingRights required_right = static_cast<CastlingRights>(
    (is_kingside ? CastlingRights::white_king : CastlingRights::white_queen) << (side_val * 2)
  );
  
  if (!board.can_castle(required_right)) {
    return false;
  }
  
  int base_king_sq = 60 - 56 * side_val;
  int king_sq = base_king_sq;
  
  if (is_kingside) {
    int f_sq = base_king_sq + 1;
    int g_sq = base_king_sq + 2;
    
    if (all_pieces.get_bit(f_sq) || all_pieces.get_bit(g_sq)) return false;
    return !board.is_sq_attacked(king_sq, enemy_side) && !board.is_sq_attacked(f_sq, enemy_side);
  }
  int d_sq = base_king_sq - 1;
  int c_sq = base_king_sq - 2;
  int b_sq = base_king_sq - 3;
  
  if (all_pieces.get_bit(d_sq) || all_pieces.get_bit(c_sq) || all_pieces.get_bit(b_sq)) return false;
  return !board.is_sq_attacked(king_sq, enemy_side) && !board.is_sq_attacked(d_sq, enemy_side);
}

static inline void generate_castling_moves(MoveArray& moves, const Board& board, Side side) {
  bb all_pieces = board.get_all_occupancy();
  Side enemy_side = opposite_side(side);
  int side_val = static_cast<int>(side);
  
  int from_sq = 60 - 56 * side_val;
  
  // Kingside castling
  if (can_castle_side(board, side, true, all_pieces, enemy_side)) {
    int to_sq = from_sq + 2;
    add_move(moves, from_sq, to_sq, PieceType::king, side, 
            PieceType::pawn, Side::any, MoveFlag::CASTLE);
  }
  
  // Queenside castling
  if (can_castle_side(board, side, false, all_pieces, enemy_side)) {
    int to_sq = from_sq - 2;
    add_move(moves, from_sq, to_sq, PieceType::king, side, 
            PieceType::pawn, Side::any, MoveFlag::CASTLE);
  }
}

void fill_move_array(MoveArray& moves, const Board& board) {
  Side side_to_move = board.get_side();
  
  generate_pawn_moves(moves, board, side_to_move);
  generate_piece_moves<PieceType::knight>(moves, board, side_to_move);
  generate_piece_moves<PieceType::bishop>(moves, board, side_to_move);
  generate_piece_moves<PieceType::rook>(moves, board, side_to_move);
  generate_piece_moves<PieceType::queen>(moves, board, side_to_move);
  generate_piece_moves<PieceType::king>(moves, board, side_to_move);
  generate_castling_moves(moves, board, side_to_move);
}