#pragma once

#include <iostream>
#include <vector>
#include <stdexcept>

enum class HashFlag {
  EXACT,
  ALPHA,
  BETA
};

struct TranspositionEntry {
  uint64_t hashKey;
  int depth;
  HashFlag flag;
  int score;
};

class TranspositionTable {
private:
  std::vector<TranspositionEntry> entries;
  size_t tableSize;
  
  static constexpr int MATE_SCORE = 48000;
  static constexpr int NO_HASH_FOUND = 100000;
  static constexpr int BYTES_PER_MB = 0x100000;

public:
  explicit TranspositionTable(int sizeInMB) {
    initialize(sizeInMB);
  }

  void initialize(int sizeInMB) {
    const size_t totalBytes = BYTES_PER_MB * sizeInMB;
    tableSize = totalBytes / sizeof(TranspositionEntry);
    
    try {
      entries.resize(tableSize);
      clear();
      
      std::cout << "Transposition table initialized with " << tableSize 
                << " entries (" << sizeInMB << "MB)" << std::endl;
    }
    catch (const std::bad_alloc& e) {
      if (sizeInMB > 1) {
        const int halfSize = sizeInMB / 2;
        std::cout << "Memory allocation failed, retrying with " 
                  << halfSize << "MB..." << std::endl;
        initialize(halfSize);
      } else {
        std::cout << "Failed to allocate minimum transposition table size!" << std::endl;
        throw;
      }
    }
  }

  void clear() {
    for (auto& entry : entries) {
      entry.hashKey = 0;
      entry.depth = 0;
      entry.flag = HashFlag::EXACT;
      entry.score = 0;
    }
  }

  int probe(int alpha, int beta, int depth, uint64_t hashKey, int ply) const {
    const TranspositionEntry& entry = entries[hashKey % tableSize];
    
    if (entry.hashKey != hashKey || entry.depth < depth) {
      return NO_HASH_FOUND;
    }

    int adjustedScore = adjustScoreFromTable(entry.score, ply);
    
    switch (entry.flag) {
      case HashFlag::EXACT:
        return adjustedScore;
      case HashFlag::ALPHA:
        return (adjustedScore <= alpha) ? alpha : NO_HASH_FOUND;
      case HashFlag::BETA:
        return (adjustedScore >= beta) ? beta : NO_HASH_FOUND;
    }
    
  return NO_HASH_FOUND;
  }

  void store(int score, int depth, HashFlag flag, uint64_t hashKey, int ply) {
    TranspositionEntry& entry = entries[hashKey % tableSize];
    
    entry.hashKey = hashKey;
    entry.score = adjustScoreForTable(score, ply);
    entry.flag = flag;
    entry.depth = depth;
  }

  size_t size() const {
    return tableSize;
  }

private:
  int adjustScoreFromTable(int score, int ply) const {
    if (score < -MATE_SCORE) {
      return score + ply;
    }
    if (score > MATE_SCORE) {
      return score - ply;
    }
    return score;
  }

  int adjustScoreForTable(int score, int ply) const {
    if (score < -MATE_SCORE) {
      return score - ply;
    }
    if (score > MATE_SCORE) {
      return score + ply;
    }
    return score;
  }
};