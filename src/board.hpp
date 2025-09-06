#pragma once

#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include "bitboard.hpp"
#include "attacks.hpp"
#include "zobrist.hpp"
#include "enums.hpp"
#include "move.hpp"

inline const std::string empty_board = "8/8/8/8/8/8/8/8 b - - 0 1";
inline const std::string start_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ";
inline const std::string tricky_position = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 ";
inline const std::string killer_position = "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1";
inline const std::string cmk_position = "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 ";
inline const std::string repetitions = "2r3k1/R7/8/1R6/8/8/P4KPP/8 w - - 0 40 ";

class Board {
private:
  static constexpr int BOARD_SIZE = 8;
  static constexpr int TOTAL_SQUARES = 64;
  static constexpr int NUMBER_OF_PIECES = 12;
  static constexpr int NUMBER_OF_SIDES = 3;
  static constexpr int INVALID_SQUARE = -1;
  static constexpr char FEN_RANK_SEPARATOR = '/';
  static constexpr char FEN_EMPTY_SQUARES_START = '1';
  static constexpr char FEN_EMPTY_SQUARES_END = '8';
  static constexpr char RANK_ONE_CHAR = '1';
  static constexpr char FILE_A_CHAR = 'a';
  static constexpr char UPPERCASE_A = 'A';
  static constexpr int LOWERCASE_TO_BLACK_OFFSET = 6;

  std::array<bb, NUMBER_OF_PIECES> piece_location{};
  std::array<bb, NUMBER_OF_SIDES> occupancies{};
  Side side = Side::white;
  std::pair<bool, int> enpassant;
  uint32_t castling = 0;
  bb hash_key{};
  uint32_t rep_idx = 0;
  int fifty = 0;

  void reset();
  uint64_t generate_hash_key();
  void move_piece(int piexe_idx, int from_sq, int to_sq);
  void pop_piece(int sq, Side side);
  
public:
  void load_fen(const std::string& fen);
  void print() const;
  bool is_sq_attacked(int sq, Side attacking_side) const;
  void print_insides();
  bool make_move(Move move);
  Side get_side() const { return side; }
  
  const std::array<bb, NUMBER_OF_PIECES>& get_piece_locations() const { 
    return piece_location; 
  }
  
  const std::array<bb, NUMBER_OF_SIDES>& get_occupancies() const { 
    return occupancies; 
  }
  
  bb get_piece_bitboard(PieceType piece, Side side) const {
    int index = static_cast<int>(piece) + NUMBER_OF_UNIQUE_PIECES * static_cast<int>(side);
    return piece_location[index];
  }

  bb get_piece_bitboard_by_idx(int idx) const {
    return piece_location[idx];
  }
  
  bb get_occupancy(Side side) const {
    return occupancies[static_cast<int>(side)];
  }
  
  bb get_all_occupancy() const {
    return occupancies[static_cast<int>(Side::any)];
  }
  
  std::pair<bool, int> get_enpassant() const { 
    return enpassant; 
  }
  
  uint32_t get_castling_rights() const { 
    return castling; 
  }
  
  bool can_castle(CastlingRights right) const {
    return castling & right;
  }

  int get_fifty_move_counter() const { 
    return fifty; 
  }
  
  bool is_fifty_move_rule_draw() const {
    return fifty >= 100;  // 100 half-moves = 50 full moves
  }

  uint64_t get_hash_key() {
    return hash_key.get_board();
  }
  
  void set_fifty_move_counter(int value) {
    fifty = value;
  }

  void change_hash_en() {
    hash_key ^= zobrist_table.enp_keys[enpassant.second];
    enpassant.first = false;
  }

  void switch_side() {
    side = opposite_side(side);
    hash_key ^= zobrist_table.side_key;
  }
};