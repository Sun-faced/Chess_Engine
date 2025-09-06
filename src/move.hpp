#pragma once

#include <iostream>
#include <string>
#include "enums.hpp"

class Move {
private:
  uint32_t body = 0;

public:
  // Constructors
  constexpr Move() noexcept : body(0) {}
  
  constexpr Move(uint32_t from_sq, uint32_t to_sq,
                 PieceType piece, Side side_of_piece,
                 PieceType prom_pce, Side side_of_prom,
                 MoveFlag flag, bool is_capture = false) noexcept
    : body((from_sq & 0x3F)
         | ((to_sq & 0x3F) << 6)
         | ((static_cast<uint32_t>(piece) & 0x7) << 12)
         | ((static_cast<uint32_t>(side_of_piece) & 0x3) << 15)
         | ((static_cast<uint32_t>(prom_pce) & 0x7) << 17)
         | ((static_cast<uint32_t>(side_of_prom) & 0x3) << 20)
         | ((static_cast<uint32_t>(flag) & 0x7) << 22)
         | ((is_capture ? 1U : 0U) << 25)) {}

  // Getters
  constexpr uint32_t get_from_sq() const noexcept {
    return body & 0x3F;
  }

  constexpr uint32_t get_to_sq() const noexcept {
    return (body >> 6) & 0x3F;
  }

  constexpr PieceType get_piece() const noexcept {
    return static_cast<PieceType>((body >> 12) & 0x7);
  }

  constexpr Side get_side_of_piece() const noexcept {
    return static_cast<Side>((body >> 15) & 0x3);
  }

  constexpr bool is_promo() const noexcept {
    return static_cast<Side>((body >> 20) & 0x3) != Side::any;
  }

  constexpr PieceType get_prom_piece() const noexcept {
    return static_cast<PieceType>((body >> 17) & 0x7);
  }

  constexpr Side get_prom_side() const noexcept {
    return static_cast<Side>((body >> 20) & 0x3);
  }

  // Flag checks
  constexpr bool is_double_pawn() const noexcept {
    return ((body >> 22) & 0x7) == MoveFlag::PAWN_START;
  }

  constexpr bool is_castle() const noexcept {
    return ((body >> 22) & 0x7) == MoveFlag::CASTLE;
  }

  constexpr bool is_enpassant() const noexcept {
    return ((body >> 22) & 0x7) == MoveFlag::EN_PASSANT;
  }

  constexpr bool is_capture() const noexcept {
    return (body >> 25) & 0x1;
  }

  // Utility functions
  constexpr uint32_t get_body() const noexcept {
    return body;
  }

  constexpr bool operator==(const Move& other) const noexcept {
    return body == other.body;
  }

  constexpr bool operator!=(const Move& other) const noexcept {
    return body != other.body;
  }

  // Debug printer
  inline void print() const {
    static constexpr const char files[] = "abcdefgh";
    static constexpr const char* piece_names[] = {"pawn", "knight", "bishop", "rook", "queen", "king"};
    
    auto square_to_string = [](uint32_t sq) -> std::string {
      uint32_t file = sq % 8;
      uint32_t rank = 8 - (sq / 8);
      return std::string(1, files[file]) + std::to_string(rank);
    };
    
    auto piece_to_string = [](PieceType piece, Side side) -> std::string {
      std::string side_str;
      switch (side) {
        case Side::white: side_str = "white "; break;
        case Side::black: side_str = "black "; break;
        default: side_str = ""; break;
      }
      return side_str + piece_names[static_cast<int>(piece)];
    };
    
    auto flag_to_string = [](MoveFlag flag) -> const char* {
      switch (flag) {
        case MoveFlag::NO_FLAG: return "NO_FLAG";
        case MoveFlag::PAWN_START: return "PAWN_START";
        case MoveFlag::CASTLE: return "CASTLE";
        case MoveFlag::EN_PASSANT: return "EN_PASSANT";
        default: return "UNKNOWN";
      }
    };

    MoveFlag flag = static_cast<MoveFlag>((body >> 22) & 0x7);

    std::cout << "Move("
              << square_to_string(get_from_sq())
              << " -> "
              << square_to_string(get_to_sq())
              << ", piece=" << piece_to_string(get_piece(), get_side_of_piece());

    if (is_promo()) {
      std::cout << ", promo=" << piece_to_string(get_prom_piece(), get_prom_side());
    }

    if (is_capture()) {
      std::cout << ", capture=true";
    }

    std::cout << ", flag=" << flag_to_string(flag)
              << ")" << std::endl;
  }
};