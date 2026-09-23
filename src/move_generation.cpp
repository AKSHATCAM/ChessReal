#include <iostream>
#include "../lib/main.h"
#include <cstdint>


using namespace std;



int main() {
    Position pos;

    // Place a few pieces by hand
    pos.main_bitboard[Position::White][Position::Rook]   |= 1ULL << 0;   // a1
    pos.main_bitboard[Position::White][Position::Knight] |= 1ULL << 6;   // g1
    pos.main_bitboard[Position::Black][Position::King]   |= 1ULL << 60;  // e8

    // Round 1: derive and print
    pos.derived_bitboard();
    std::cout << "White occupancy:";
    printBitboard(pos.occupancyarray[Position::White]);
    std::cout << "Black occupancy:";
    printBitboard(pos.occupancyarray[Position::Black]);
    std::cout << "All occupied:";
    printBitboard(pos.all_occupied_squares);

    // Round 2: clear, re-derive, print again
    pos.clear();
    pos.derived_bitboard();
    std::cout << "After clear - all occupied:";
    printBitboard(pos.all_occupied_squares);
    std::cout << "After clear - white rook board:";
    printBitboard(pos.main_bitboard[Position::White][Position::Rook]);
}

