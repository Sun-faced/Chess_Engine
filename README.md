
# Chess engine - Misha Osipov Prime

Named after a [viral meme](https://www.youtube.com/watch?v=c7BVtGnlxT8), this C++ chess engine implements several advanced techniques for performance optimization. The most notable features include [NNUE](https://www.chessprogramming.org/NNUE)-based evaluation and the Negamax search algorithm, which provides a more efficient alternative to the traditional Minimax approach.

## Installation

You can build the engine using `cmake`.

### Build with CMake
```bash
git clone https://github.com/Sun-faced/Chess_Engine.git
cd Chess_Engine
mkdir build && cd build
cmake ..
make
```
## Features of engie:

### Protocols
- Partial support of [UCI](https://en.wikipedia.org/wiki/Universal_Chess_Interface) protocol

### Board representation

- Bitboard representation of pieces locations
- Magic numbers implementation of slider pieces
- Precompilation of attacks tables
- Move as an integer representation

### Evaluation

- NNUE evaluation

### Search

- Negamax with alpha-beta pruning
- Quiescence search
- Transposition table
- Iterative deepening
- Principal valuation search
- Static null move pruning
- Late move reduciton
- Move ordering (PV/MVV-LVA/killer/history)
- Razoring
