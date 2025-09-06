#include "uci.hpp"
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include "./nnue/nnue.h"

UciInterface::UciInterface() : searchEngine(64) {
  nnue_init("nn-62ef826d1a6d.nnue");
}

std::vector<std::string> UciInterface::splitString(const std::string& input, char delimiter) const {
  std::vector<std::string> tokens;
  std::stringstream stream(input);
  std::string token;

  while (std::getline(stream, token, delimiter)) {
    if (!token.empty()) {
      tokens.push_back(token);
    }
  }
  
  return tokens;
}

std::string UciInterface::squareToString(uint32_t square) const {
  constexpr const char FILES[] = "abcdefgh";
  const uint32_t file = square % 8;
  const uint32_t rank = 8 - (square / 8);
  return std::string(1, FILES[file]) + std::to_string(rank);
}

std::string UciInterface::moveToString(const Move& move) const {
  std::string result = squareToString(move.get_from_sq()) + squareToString(move.get_to_sq());
  
  if (move.is_promo()) {
    constexpr const char PROMOTION_PIECES[] = {'q', 'r', 'b', 'n'};
    const int pieceIndex = static_cast<int>(move.get_prom_piece()) - 1;
    if (pieceIndex >= 0 && pieceIndex < 4) {
      result += PROMOTION_PIECES[pieceIndex];
    }
  }
  
  return result;
}

int64_t UciInterface::calculateTimeAllocation(int64_t playerTime, int64_t opponentTime, 
                                              int64_t playerIncrement, int64_t opponentIncrement, 
                                              int64_t movesToGo, bool infiniteSearch) const {
  if (infiniteSearch || playerTime == 0) {
    return INT64_MAX;
  }
  
  int64_t timeAllocation = ::calculateTimeAllocation(playerTime, opponentTime, 
                                                     playerIncrement, opponentIncrement, movesToGo);
  
  if (timeAllocation < MINIMUM_TIME_ALLOCATION) {
    timeAllocation = std::min(playerTime / EMERGENCY_TIME_DIVISOR, MAXIMUM_EMERGENCY_TIME);
  }
  
  return timeAllocation;
}

void UciInterface::parseAndMakeMove(const std::string& moveString) {
  if (moveString.length() < 4) {
    return;
  }
  
  MoveArray legalMoves;
  fill_move_array(legalMoves, chessBoard);
  
  for (size_t i = 0; i < legalMoves.size(); ++i) {
    const Move currentMove = legalMoves.get(i);
    const std::string currentMoveString = moveToString(currentMove);
    
    if (currentMoveString == moveString) {
      chessBoard.make_move(currentMove);
      break;
    }
  }
}

void UciInterface::handleUciCommand() {
  std::cout << "id name " << ENGINE_NAME << std::endl;
  std::cout << "id author " << AUTHOR_NAME << std::endl;
  std::cout << "uciok" << std::endl;
}

void UciInterface::handleIsReadyCommand() {
  std::cout << "readyok" << std::endl;
}

void UciInterface::handleNewGameCommand() {
  chessBoard.load_fen(start_position);
  searchEngine.resetRepetitionTable();
}

void UciInterface::handlePositionCommand(const std::vector<std::string>& tokens) {
  if (tokens.size() < 2) {
    return;
  }
  
  size_t moveTokenIndex = 0;
  
  if (tokens[1] == "startpos") {
    chessBoard.load_fen(start_position);
    searchEngine.resetRepetitionTable();
    moveTokenIndex = 3;
  } else if (tokens[1] == "fen" && tokens.size() >= 8) {
    std::string fenString;
    for (size_t i = 2; i < 8 && i < tokens.size(); ++i) {
      if (i > 2) fenString += " ";
      fenString += tokens[i];
    }
    chessBoard.load_fen(fenString);
    searchEngine.resetRepetitionTable();
    moveTokenIndex = 9;
  }
  
  auto movesIterator = std::find(tokens.begin(), tokens.end(), "moves");
  if (movesIterator != tokens.end()) {
    moveTokenIndex = std::distance(tokens.begin(), movesIterator) + 1;
    
    for (size_t i = moveTokenIndex; i < tokens.size(); ++i) {
      parseAndMakeMove(tokens[i]);
    }
  }
  
  chessBoard.print();
}

void UciInterface::handleGoCommand(const std::vector<std::string>& tokens) {
  int64_t whiteTime = 0;
  int64_t blackTime = 0;
  int64_t whiteIncrement = 0;
  int64_t blackIncrement = 0;
  int64_t movesToGo = 0;
  int searchDepth = DEFAULT_SEARCH_DEPTH;
  bool infiniteSearch = false;
  
  for (size_t i = 1; i < tokens.size(); ++i) {
    const std::string& token = tokens[i];
    
    if (token == "infinite") {
      infiniteSearch = true;
    } else if (i + 1 < tokens.size()) {
      const std::string& value = tokens[i + 1];
      
      if (token == "wtime") {
        whiteTime = std::stoll(value);
      } else if (token == "btime") {
        blackTime = std::stoll(value);
      } else if (token == "winc") {
        whiteIncrement = std::stoll(value);
      } else if (token == "binc") {
        blackIncrement = std::stoll(value);
      } else if (token == "movestogo") {
        movesToGo = std::stoll(value);
      } else if (token == "depth") {
        searchDepth = std::stoi(value);
      }
      
      if (token == "wtime" || token == "btime" || token == "winc" || 
          token == "binc" || token == "movestogo" || token == "depth") {
        ++i;
      }
    }
  }
  
  const bool isWhiteToMove = (chessBoard.get_side() == Side::white);
  const int64_t playerTime = isWhiteToMove ? whiteTime : blackTime;
  const int64_t opponentTime = isWhiteToMove ? blackTime : whiteTime;
  const int64_t playerIncrement = isWhiteToMove ? whiteIncrement : blackIncrement;
  const int64_t opponentIncrement = isWhiteToMove ? blackIncrement : whiteIncrement;
  
  const int64_t timeAllocation = calculateTimeAllocation(playerTime, opponentTime, 
                                                        playerIncrement, opponentIncrement, 
                                                        movesToGo, infiniteSearch);
  
  searchEngine.setTimeLimit(timeAllocation);
  std::cout << "time_alloted: " << timeAllocation << "\n";
  
  searchEngine.findBestMove(chessBoard, searchDepth);
  chessBoard.print();
}

void UciInterface::handleStopCommand() {
  searchEngine.stopSearch();
}

void UciInterface::runGameLoop() {
  std::string inputLine;
  
  while (std::getline(std::cin, inputLine)) {
    const std::vector<std::string> tokens = splitString(inputLine, ' ');
    
    if (tokens.empty()) {
      continue;
    }
    
    const std::string& command = tokens[0];
    
    if (command == "quit") {
      break;
    } else if (command == "uci") {
      handleUciCommand();
    } else if (command == "isready") {
      handleIsReadyCommand();
    } else if (command == "ucinewgame") {
      handleNewGameCommand();
    } else if (command == "position") {
      handlePositionCommand(tokens);
    } else if (command == "go") {
      handleGoCommand(tokens);
    } else if (command == "stop") {
      handleStopCommand();
    } else {
      std::cout << "Unknown command: " << inputLine << std::endl;
    }
  }
}