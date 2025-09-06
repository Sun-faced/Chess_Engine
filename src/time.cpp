#include "time.hpp"
#include <algorithm>

int64_t TimeManager::calculateTimeAllocation(int64_t playerTime, int64_t opponentTime, 
                                              int64_t playerIncrement, int64_t opponentIncrement, 
                                              int64_t movesToGo) {
  if (playerTime <= 0) {
    return DEFAULT_SEARCH_TIME;
  }

  int64_t timeAllocation;
  
  if (movesToGo > 0) {
    timeAllocation = (playerTime / movesToGo) + 
                     (playerIncrement * INCREMENT_MULTIPLIER / INCREMENT_DIVISOR);
  } else {
    if (playerTime > LONG_GAME_THRESHOLD) {
      timeAllocation = playerTime / LONG_GAME_DIVISOR + playerIncrement;
    } else if (playerTime > MEDIUM_GAME_THRESHOLD) {
      timeAllocation = playerTime / MEDIUM_GAME_DIVISOR + playerIncrement;
    } else {
      timeAllocation = playerTime / SHORT_GAME_DIVISOR + playerIncrement;
    }
  }
  
  timeAllocation = std::max(timeAllocation, MINIMUM_TIME_ALLOCATION);
  timeAllocation = std::min(timeAllocation, playerTime / MAX_TIME_FRACTION);
  
  return timeAllocation;
}

int64_t TimeManager::getCurrentTimeMilliseconds() {
  return std::chrono::duration_cast<std::chrono::milliseconds>
    (std::chrono::steady_clock::now().time_since_epoch()).count();
}

int64_t calculateTimeAllocation(int64_t playerTime, int64_t opponentTime, 
                                int64_t playerIncrement, int64_t opponentIncrement, 
                                int64_t movesToGo) {
  return TimeManager::calculateTimeAllocation(playerTime, opponentTime, 
                                              playerIncrement, opponentIncrement, movesToGo);
}