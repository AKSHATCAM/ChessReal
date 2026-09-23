#pragma once
#include <cstdint>
#include <iostream>

enum Square {
    a1, b1, c1, d1, e1, f1, g1, h1,   //  0 -  7
    a2, b2, c2, d2, e2, f2, g2, h2,   //  8 - 15
    a3, b3, c3, d3, e3, f3, g3, h3,   // 16 - 23
    a4, b4, c4, d4, e4, f4, g4, h4,   // 24 - 31
    a5, b5, c5, d5, e5, f5, g5, h5,   // 32 - 39
    a6, b6, c6, d6, e6, f6, g6, h6,   // 40 - 47
    a7, b7, c7, d7, e7, f7, g7, h7,   // 48 - 55
    a8, b8, c8, d8, e8, f8, g8, h8,   // 56 - 63
    SquareCount                       // 64
};

inline void printBitboard(uint64_t bitboard)
{
    std::cout << "\n\n";
    for (int i = 0; i < 8; ++i)
    {
        std::cout << 8 - i << "    ";
        for (int j = 0; j < 8; ++j)
        {
            uint64_t bit = 1ULL << ((7 - i) * 8 + j);
            bool temp = bit & bitboard;
            std::cout << temp << " ";
        }
        std::cout << "\n";
    }

    std::cout << "\n     ";
    for (char file = 'a'; file <= 'h'; ++file)
        std::cout << file << " ";

    std::cout << "\n\n\n";
}
