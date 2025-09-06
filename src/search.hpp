#pragma once

#include <iostream>
#include <array>
#include "board.hpp"
#include "transposition_table.hpp"
#include "evaluation.hpp"
#include "move_array.hpp"
#include "move_generator.hpp"
#include "time.hpp"

class ChessSearch {
public:
  explicit ChessSearch(int hashSizeMB = 64);
  
  void setTimeLimit(uint64_t timeLimit);
  void stopSearch();
  void resetRepetitionTable();
  void findBestMove(Board& board, int maxDepth);

private:
  static constexpr int MATE_SCORE = 48000;
  static constexpr int MATE_VALUE = 49000;
  static constexpr int INFINITY_VALUE = 500000;
  static constexpr int MAX_PLY = 64;
  static constexpr int FULL_DEPTH_MOVES = 4;
  static constexpr int REDUCTION_LIMIT = 3;
  
  TranspositionTable transpositionTable;
  
  int currentPly;
  uint64_t searchStartTime;
  uint32_t nodesSearched;
  bool searchStopped;
  uint64_t timeAllocated;
  
  std::array<std::array<Move, MAX_PLY>, 2> killerMoves;
  std::array<std::array<int, 64>, 12> historyMoves;
  std::array<uint64_t, 1024> repetitionTable;
  int repetitionIndex;
  
  bool scorePrincipalVariation;
  bool followPrincipalVariation;
  std::array<int, MAX_PLY> principalVariationLengths;
  std::array<std::array<Move, MAX_PLY>, MAX_PLY> principalVariationTable;
  
  int calculateMoveScore(Move move, const Board& board) const;
  void sortMoves(MoveArray& moves, const Board& board) const;
  bool isPositionRepeated(uint64_t hashKey) const;
  bool isTimeExpired() const;
  void enablePrincipalVariationScoring(const MoveArray& moves);
  
  int quiescenceSearch(int alpha, int beta, Board& board);
  int negamaxSearch(int alpha, int beta, int depth, Board& board);
};