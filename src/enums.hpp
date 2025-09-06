#pragma once

constexpr int NUMBER_OF_SIDES = 2; // we don't count BOTH sides to be a side

enum class Side {
  white,
  black,
  any
};

inline Side opposite_side(Side side) {
  switch(side) {
    case Side::white: return Side::black;
    case Side::black: return Side::white;
    case Side::any:   return Side::any;
  }
  return Side::any;
}

constexpr int NUMBER_OF_UNIQUE_PIECES = 6;

enum class PieceType {
  pawn,
  knight,
  bishop,
  rook,
  queen,
  king
};

constexpr int NUMBER_OF_SLIDING_PIECES = 3;

enum class SlidingPiece {
  bishop,
  rook,
  queen
};

enum CastlingRights : uint32_t {
	white_king = 1,
	white_queen	= 2,
	black_king = 4,
	black_queen	= 8
};

constexpr int NUMBER_OF_MOVE_FLAGS = 4;

enum MoveFlag : uint32_t {
	NO_FLAG	   = 0,
	PAWN_START = 1,
	CASTLE	   = 2,
	EN_PASSANT = 4
};

constexpr PieceType to_normal_pieces(SlidingPiece piece) {
  switch (piece) {
    case SlidingPiece::bishop: return PieceType::bishop;
    case SlidingPiece::rook:   return PieceType::rook;
    case SlidingPiece::queen:  return PieceType::queen;
  }
}
