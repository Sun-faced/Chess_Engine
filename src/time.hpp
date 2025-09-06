#pragma once

#include <cstdint>
#include <chrono>

class TimeManager {
public:
  static int64_t calculateTimeAllocation(int64_t playerTime, int64_t opponentTime, 
                                         int64_t playerIncrement, int64_t opponentIncrement, 
                                         int64_t movesToGo);

  static int64_t getCurrentTimeMilliseconds();

private:
  static constexpr int64_t DEFAULT_SEARCH_TIME = 5000;
  static constexpr int64_t MINIMUM_TIME_ALLOCATION = 100;
  static constexpr int64_t LONG_GAME_THRESHOLD = 300000;
  static constexpr int64_t MEDIUM_GAME_THRESHOLD = 60000;
  static constexpr int64_t LONG_GAME_DIVISOR = 40;
  static constexpr int64_t MEDIUM_GAME_DIVISOR = 30;
  static constexpr int64_t SHORT_GAME_DIVISOR = 20;
  static constexpr int64_t INCREMENT_MULTIPLIER = 3;
  static constexpr int64_t INCREMENT_DIVISOR = 4;
  static constexpr int64_t MAX_TIME_FRACTION = 2;
};

int64_t calculateTimeAllocation(int64_t playerTime, int64_t opponentTime, 
                                int64_t playerIncrement, int64_t opponentIncrement, 
                                int64_t movesToGo);

inline int64_t getCurrentTimeMilliseconds() {
  return TimeManager::getCurrentTimeMilliseconds();
}