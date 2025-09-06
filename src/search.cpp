#include "search.hpp"
#include <algorithm>
#include <vector>

namespace {
  constexpr int PIECE_VALUES[6] = {100, 200, 300, 400, 500, 600};
  constexpr int ATTACKER_PENALTIES[6] = {5, 4, 3, 2, 1, 0};
  constexpr int NO_HASH_ENTRY = 100000;
  
  int calculateMvvLvaScore(int victimPiece, int attackerPiece) {
    const int victimType = victimPiece % 6;
    const int attackerType = attackerPiece % 6;
    return PIECE_VALUES[victimType] + ATTACKER_PENALTIES[attackerType];
  }
  
  std::string squareToString(uint32_t square) {
    constexpr const char FILES[] = "abcdefgh";
    const uint32_t file = square % 8;
    const uint32_t rank = 8 - (square / 8);
    return std::string(1, FILES[file]) + std::to_string(rank);
  }
  
  std::string moveToString(const Move& move) {
    std::string result = squareToString(move.get_from_sq()) + squareToString(move.get_to_sq());
    
    if (move.is_promo()) {
      switch (move.get_prom_piece()) {
        case PieceType::queen: result += "q"; break;
        case PieceType::rook: result += "r"; break;
        case PieceType::bishop: result += "b"; break;
        case PieceType::knight: result += "n"; break;
        default: break;
      }
    }
    
    return result;
  }
}

ChessSearch::ChessSearch(int hashSizeMB) 
  : transpositionTable(hashSizeMB),
    currentPly(0),
    searchStartTime(0),
    nodesSearched(0),
    searchStopped(false),
    timeAllocated(INT64_MAX),
    repetitionIndex(0),
    scorePrincipalVariation(false),
    followPrincipalVariation(false) {
  resetRepetitionTable();
}

void ChessSearch::setTimeLimit(uint64_t timeLimit) {
  timeAllocated = timeLimit;
}

void ChessSearch::stopSearch() {
  searchStopped = true;
}

void ChessSearch::resetRepetitionTable() {
  std::fill(repetitionTable.begin(), repetitionTable.end(), 0);
  repetitionIndex = 0;
}

void ChessSearch::enablePrincipalVariationScoring(const MoveArray& moves) {
  followPrincipalVariation = false;
  
  for (size_t i = 0; i < moves.size(); ++i) {
    if (principalVariationTable[0][currentPly] == moves.get(i)) {
      scorePrincipalVariation = true;
      followPrincipalVariation = true;
      break;
    }
  }
}

int ChessSearch::calculateMoveScore(Move move, const Board& board) const {
  if (scorePrincipalVariation && principalVariationTable[0][currentPly] == move) {
    return 20000;
  }

  if (!move.is_capture()) {
    if (killerMoves[0][currentPly] == move) {
      return 9000;
    }
    if (killerMoves[1][currentPly] == move) {
      return 8000;
    }
    return historyMoves[static_cast<int>(move.get_piece())][move.get_to_sq()];
  }

  int targetPieceIndex = 0;
  const int startIndex = static_cast<int>(opposite_side(board.get_side())) * 6;
  const int endIndex = startIndex + 5;

  for (int i = startIndex; i <= endIndex; ++i) {
    if (board.get_piece_bitboard_by_idx(i).get_bit(move.get_to_sq())) {
      targetPieceIndex = i;
      break;
    }
  }

  return calculateMvvLvaScore(static_cast<int>(move.get_piece()), targetPieceIndex) + 10000;
}

void ChessSearch::sortMoves(MoveArray& moves, const Board& board) const {
  std::vector<int> moveScores(moves.size());
  
  for (size_t i = 0; i < moves.size(); ++i) {
    moveScores[i] = calculateMoveScore(moves.get(i), board);
  }

  for (size_t i = 0; i < moves.size(); ++i) {
    for (size_t j = i + 1; j < moves.size(); ++j) {
      if (moveScores[i] < moveScores[j]) {
        moves.swap(i, j);
        std::swap(moveScores[i], moveScores[j]);
      }
    }
  }
}

bool ChessSearch::isPositionRepeated(uint64_t hashKey) const {
  for (int i = 0; i < repetitionIndex; ++i) {
    if (repetitionTable[i] == hashKey) {
      return true;
    }
  }
  return false;
}

bool ChessSearch::isTimeExpired() const {
  return ((nodesSearched & 2047) == 0) && 
         ((getCurrentTimeMilliseconds() - searchStartTime) > timeAllocated);
}

int ChessSearch::quiescenceSearch(int alpha, int beta, Board& board) {
  if (isTimeExpired()) {
    searchStopped = true;
  }

  ++nodesSearched;

  const int standPatScore = evaluate(board);

  if (currentPly > MAX_PLY - 1) {
    return standPatScore;
  }

  if (standPatScore >= beta) {
    return beta;
  }

  if (standPatScore > alpha) {
    alpha = standPatScore;
  }

  MoveArray moves{};
  fill_move_array(moves, board);
  sortMoves(moves, board);

  for (size_t i = 0; i < moves.size(); ++i) {
    if (!moves.get(i).is_capture()) {
      continue;
    }

    const Board boardCopy = board;
    ++currentPly;
    ++repetitionIndex;
    repetitionTable[repetitionIndex] = board.get_hash_key();

    if (!board.make_move(moves.get(i))) {
      --currentPly;
      --repetitionIndex;
      continue;
    }

    const int score = -quiescenceSearch(-beta, -alpha, board);
    --currentPly;
    --repetitionIndex;
    board = boardCopy;

    if (searchStopped) {
      return 0;
    }

    if (score > alpha) {
      alpha = score;
      if (score >= beta) {
        return beta;
      }
    }
  }

  return alpha;
}

int ChessSearch::negamaxSearch(int alpha, int beta, int depth, Board& board) {
  principalVariationLengths[currentPly] = currentPly;
  int score = 0;
  HashFlag hashFlag = HashFlag::ALPHA;

  if (currentPly && (isPositionRepeated(board.get_hash_key()) || 
                     board.get_fifty_move_counter() >= 100)) {
    return 0;
  }

  const bool isPrincipalVariationNode = (beta - alpha) > 1;

  if (!isPrincipalVariationNode && currentPly != 0) {
    score = transpositionTable.probe(alpha, beta, depth, board.get_hash_key(), currentPly);
    if (score != NO_HASH_ENTRY) {
      return score;
    }
  }

  if (isTimeExpired()) {
    searchStopped = true;
  }

  if (depth == 0) {
    return quiescenceSearch(alpha, beta, board);
  }
  
  if (currentPly > MAX_PLY - 1) {
    return evaluate(board);
  }

  ++nodesSearched;

  const bool inCheck = board.is_sq_attacked(
    board.get_piece_bitboard_by_idx(5 + 6 * static_cast<int>(board.get_side())).get_lsb_index(), 
    opposite_side(board.get_side())
  );

  if (inCheck) {
    ++depth;
  }

  int legalMovesCount = 0;

  if (depth >= 3 && !inCheck && currentPly != 0) {
    const Board boardCopy = board;

    ++currentPly;
    ++repetitionIndex;
    repetitionTable[repetitionIndex] = board.get_hash_key();

    if (board.get_enpassant().first) {
      board.change_hash_en();
    }

    board.switch_side();
    score = -negamaxSearch(-beta, -beta + 1, depth - 3, board);
    --currentPly;
    --repetitionIndex;
    board = boardCopy;

    if (searchStopped) {
      return 0;
    }

    if (score >= beta) {
      return beta;
    }
  }

  MoveArray moves{};
  fill_move_array(moves, board);

  if (followPrincipalVariation) {
    enablePrincipalVariationScoring(moves);
  }

  sortMoves(moves, board);

  int movesSearched = 0;

  for (size_t i = 0; i < moves.size(); ++i) {
    const Board boardCopy = board;
    ++currentPly;
    ++repetitionIndex;
    repetitionTable[repetitionIndex] = board.get_hash_key();

    if (!board.make_move(moves.get(i))) {
      --currentPly;
      --repetitionIndex;
      continue;
    }

    ++legalMovesCount;

    if (movesSearched == 0) {
      score = -negamaxSearch(-beta, -alpha, depth - 1, board);
    } else {
      if (movesSearched >= FULL_DEPTH_MOVES && depth >= REDUCTION_LIMIT && 
          !inCheck && !moves.get(i).is_capture() && !moves.get(i).is_promo()) {
        score = -negamaxSearch(-alpha - 1, -alpha, depth - 2, board);
      } else {
        score = alpha + 1;
      }
      
      if (score > alpha) {
        score = -negamaxSearch(-alpha - 1, -alpha, depth - 1, board);
        if (score > alpha && score < beta) {
          score = -negamaxSearch(-beta, -alpha, depth - 1, board);
        }
      } 
    }
    
    --currentPly;
    --repetitionIndex;
    board = boardCopy;
    
    if (searchStopped) {
      return 0;
    }

    ++movesSearched;

    if (score > alpha) {
      hashFlag = HashFlag::EXACT;

      if (!moves.get(i).is_capture()) {
        historyMoves[static_cast<int>(moves.get(i).get_piece())][moves.get(i).get_to_sq()] += depth;
      }

      alpha = score;

      principalVariationTable[currentPly][currentPly] = moves.get(i);

      for (int nextPly = currentPly + 1; nextPly < principalVariationLengths[currentPly + 1]; ++nextPly) {
        principalVariationTable[currentPly][nextPly] = principalVariationTable[currentPly + 1][nextPly];
      }

      principalVariationLengths[currentPly] = principalVariationLengths[currentPly + 1];

      if (score >= beta) {
        transpositionTable.store(beta, depth, HashFlag::BETA, board.get_hash_key(), currentPly);

        if (!moves.get(i).is_capture()) {
          killerMoves[1][currentPly] = killerMoves[0][currentPly];
          killerMoves[0][currentPly] = moves.get(i);
        }

        return beta;
      }
    }
  }

  if (legalMovesCount == 0) {
    if (inCheck) {
      return -MATE_VALUE + currentPly;
    } else {
      return 0;
    }
  }

  transpositionTable.store(alpha, depth, hashFlag, board.get_hash_key(), currentPly);
  return alpha;
}

void ChessSearch::findBestMove(Board& board, int maxDepth) {
  searchStartTime = getCurrentTimeMilliseconds();
  std::string bestMoveString = "none";

  int score = 0;
  nodesSearched = 0;
  searchStopped = false;
  followPrincipalVariation = false;
  scorePrincipalVariation = false;

  for (auto& killerArray : killerMoves) {
    std::fill(killerArray.begin(), killerArray.end(), Move{});
  }
  for (auto& historyArray : historyMoves) {
    std::fill(historyArray.begin(), historyArray.end(), 0);
  }
  for (auto& pvArray : principalVariationTable) {
    std::fill(pvArray.begin(), pvArray.end(), Move{});
  }
  std::fill(principalVariationLengths.begin(), principalVariationLengths.end(), 0);

  int alpha = -INFINITY_VALUE;
  int beta = +INFINITY_VALUE;

  for (int depth = 1; depth <= maxDepth; ++depth) {
    if (searchStopped) {
      break;
    }

    followPrincipalVariation = true;
    score = negamaxSearch(alpha, beta, depth, board);
    
    if (score <= alpha || score >= beta) {
      alpha = -INFINITY_VALUE;
      beta = +INFINITY_VALUE;
      continue;
    }

    alpha = score - 50;
    beta = score + 50;

    if (principalVariationLengths[0] > 0) {
      const Move firstMove = principalVariationTable[0][0];
      bestMoveString = moveToString(firstMove);

      if (score > -MATE_VALUE && score < -MATE_SCORE) {
        std::cout << "info score mate " << -(score + MATE_VALUE) / 2 - 1 
                  << " depth " << depth << " nodes " << nodesSearched;
      } else if (score > MATE_SCORE && score < MATE_VALUE) {
        std::cout << "info score mate " << (MATE_VALUE - score) / 2 + 1 
                  << " depth " << depth << " nodes " << nodesSearched;
      } else {
        std::cout << "info score cp " << score 
                  << " depth " << depth << " nodes " << nodesSearched;
      }

      std::cout << " pv ";
      for (int i = 0; i < principalVariationLengths[0]; ++i) {
        std::cout << moveToString(principalVariationTable[0][i]) << " ";
      }
      std::cout << "\n";
    }
  }

  std::cout << "bestmove " << bestMoveString << std::endl;
}