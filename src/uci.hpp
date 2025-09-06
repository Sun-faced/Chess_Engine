#pragma once

#include "board.hpp"
#include "time.hpp"
#include "search.hpp"
#include <string>
#include <vector>

class UciInterface {
public:
  UciInterface();
  
  void runGameLoop();

private:
  static constexpr const char* ENGINE_NAME = "Misha Osipov Prime";
  static constexpr const char* AUTHOR_NAME = "Ruslan Sharafetdinov";
  static constexpr int DEFAULT_SEARCH_DEPTH = 255;
  static constexpr int64_t MINIMUM_TIME_ALLOCATION = 100;
  static constexpr int64_t EMERGENCY_TIME_DIVISOR = 10;
  static constexpr int64_t MAXIMUM_EMERGENCY_TIME = 5000;
  
  Board chessBoard;
  ChessSearch searchEngine;
  
  std::vector<std::string> splitString(const std::string& input, char delimiter) const;
  
  void handleUciCommand();
  void handleIsReadyCommand();
  void handleNewGameCommand();
  void handlePositionCommand(const std::vector<std::string>& tokens);
  void handleGoCommand(const std::vector<std::string>& tokens);
  void handleStopCommand();
  
  void parseAndMakeMove(const std::string& moveString);
  std::string moveToString(const Move& move) const;
  std::string squareToString(uint32_t square) const;
  int64_t calculateTimeAllocation(int64_t playerTime, int64_t opponentTime, 
                                  int64_t playerIncrement, int64_t opponentIncrement, 
                                  int64_t movesToGo, bool infiniteSearch) const;
};