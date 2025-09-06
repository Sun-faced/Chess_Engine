#pragma once

#include <cstdint>

class RandomNumberGenerator {
public:
  explicit constexpr RandomNumberGenerator(uint64_t initialSeed = DEFAULT_SEED) 
    : currentSeed(initialSeed) {}

  constexpr uint64_t generateNext() {
    return xorShiftMultiply();
  }

private:
  static constexpr uint64_t DEFAULT_SEED = UINT64_C(1070372);
  static constexpr uint64_t MULTIPLIER = UINT64_C(2685821657736338717);

  uint64_t currentSeed;

  constexpr uint64_t xorShiftMultiply() {
    currentSeed ^= currentSeed >> 12;
    currentSeed ^= currentSeed << 25;
    currentSeed ^= currentSeed >> 27;
    currentSeed *= MULTIPLIER;
    return currentSeed;
  }
};