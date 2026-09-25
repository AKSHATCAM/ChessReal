#pragma once
#include <cstdint>
#include <iostream>
#include <cassert>

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

enum Colour {
    White,
    Black,
    ColourCount
};

namespace Masks {
    constexpr uint64_t h_file = 0x8080808080808080ULL;
    constexpr uint64_t not_h_file = ~h_file;
        
    constexpr uint64_t g_file = 0x4040404040404040ULL;
    constexpr uint64_t not_g_file = ~g_file;

    constexpr uint64_t a_file = 0x0101010101010101ULL;
    constexpr uint64_t not_a_file = ~a_file;

    constexpr uint64_t b_file = 0x0202020202020202ULL;
    constexpr uint64_t not_b_file = ~b_file;


    constexpr uint64_t notGHFile = not_g_file & not_h_file;
    constexpr uint64_t notABFile = not_a_file & not_b_file; 

    constexpr uint64_t full_board = 0xFFFFFFFFFFFFFFFFULL;

}

inline Square makeSquare(int rank, int file) {
    assert(rank >= 0 && rank < 8);
    assert(file >= 0 && file < 8);
    return static_cast<Square>(rank * 8 + file);
}

inline int rankOf(Square sq) {
    return sq / 8;
}

inline int fileOf(Square sq) {
    return sq % 8;
}


inline uint64_t setSquare(uint64_t bitboard, Square sq)
{
    assert(sq >= 0 && sq < 64);
    return bitboard | (1ULL << sq);
}

inline uint64_t clearSquare(uint64_t bitboard, Square sq)
{
    assert(sq >= 0 && sq < 64);
    return bitboard & ~(1ULL << sq);
}

inline bool testSquare(uint64_t bitboard, Square sq)
{
    assert(sq >= 0 && sq < 64);
    return bitboard & (1ULL << sq);
}

inline int population_count(uint64_t bitboard){

    int result = 0;
    for(int i = 0; i < SquareCount; ++i)
    {
        if (testSquare(bitboard, static_cast<Square>(i)))
            result ++;
    }

    return result;
}

inline Square lowestBit(uint64_t bitboard)
{
    assert(bitboard != 0);

    for (int i = 0; i < SquareCount; ++i)
    {
        if (testSquare(bitboard, static_cast<Square>(i)))
            return static_cast<Square>(i);
    }

    return SquareCount; // unreachable if the assert holds
}

inline Square highestBit(uint64_t bitboard)
{
    assert(bitboard != 0);
    Square current_square = a1;

    for (int i = 0; i < SquareCount; ++i)
    {
        if (testSquare(bitboard, static_cast<Square>(i)))
            current_square = static_cast<Square>(i);
    }

    return current_square;
}



inline Square poplowestBit(uint64_t& bitboard)
{
    assert(bitboard != 0);

    for (int i = 0; i < SquareCount; ++i)
    {
        if (testSquare(bitboard, static_cast<Square>(i))){
            bitboard = clearSquare(bitboard, static_cast<Square>(i));
            return static_cast<Square>(i);
        }
            
    }

    return SquareCount; // unreachable if the assert holds
}

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



