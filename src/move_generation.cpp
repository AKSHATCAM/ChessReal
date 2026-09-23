#include <iostream>
#include <cstdint>
#include "../lib/main.h"
#include "../lib/bitboard_utilities.h"

int main() {
    Position pos;

    pos.main_bitboard[Position::White][Position::Rook] |= 1ULL << a1;
    pos.main_bitboard[Position::White][Position::Knight] |= 1ULL << g1;
    pos.main_bitboard[Position::Black][Position::King] |= 1ULL << e8;

    pos.derived_bitboard();

    std::cout << "White occupancy:\n";
    printBitboard(pos.occupancyarray[Position::White]);

    std::cout << "Black occupancy:\n";
    printBitboard(pos.occupancyarray[Position::Black]);

    std::cout << "All occupied:\n";
    printBitboard(pos.all_occupied_squares);

    pos.clear();
    pos.derived_bitboard();

    std::cout << "After clear - all occupied:\n";
    printBitboard(pos.all_occupied_squares);

    return 0;
}

