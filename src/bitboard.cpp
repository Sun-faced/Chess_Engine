#include "bitboard.hpp"

bb::bb(uint64_t val) : board(val) {}

void bb::print_bb() const {
    std::cout << '\n';
    for (int rank = 0; rank < 8; ++rank) {
        std::cout << "  " << 8 - rank << " ";
        for (int file = 0; file < 8; ++file) {
            int square = rank * 8 + file;
            std::cout << ' ' << (get_bit(square) ? '1' : '0');
        }
        std::cout << '\n';
    }
    
    std::cout << "\n     a b c d e f g h\n\n";
    std::cout << "     Numeric: " << board << "\n\n";
}