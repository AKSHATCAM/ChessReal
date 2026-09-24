#pragma once
#include "bitboard_utilities.h"

// Structs -----------------------------------------------------------------------
struct ShiftDir {
    int shift;
    uint64_t mask;   // applied to the result after shifting
};

//mappings -----------------------------------------------------------------------------

constexpr ShiftDir northKingDirs[4] = {
    {7, Masks::not_h_file}, //noWe
    {8, Masks::full_board}, //No
    {9, Masks::not_a_file}, //noEa
    {1, Masks::not_a_file}, //Ea
};

constexpr ShiftDir southKingDirs[4] = {
    {7, Masks::not_a_file}, //soEa
    {8, Masks::full_board}, //So
    {9, Masks::not_h_file}, //soWe
    {1, Masks::not_h_file}, //We
};


constexpr ShiftDir white_north_Dirs[2] = {
    {9, Masks::not_a_file },   // noEa
    {7, Masks::not_h_file },   // noWe
};

constexpr ShiftDir black_south_Dirs[2] = {
    {9, Masks::not_h_file },   // soWe
    {7, Masks::not_a_file },   // soEa
};

constexpr ShiftDir northDirs[4] = {
    { 6, Masks::notGHFile},   // noWeWe
    {15, Masks::not_h_file },   // noNoWe
    {17, Masks::not_a_file },   // noNoEa
    {10, Masks::notABFile},   // noEaEa
};

constexpr ShiftDir southDirs[4] = {
    {10, Masks::notGHFile},   // soWeWe
    {17, Masks::not_h_file},   // soSoWe
    {15, Masks::not_a_file },   // soSoEa
    { 6, Masks::notABFile},   // soEaEa
};


// Tables ---------------------------------------------------------------------------

inline uint64_t king_table[SquareCount]{};
inline uint64_t knight_table[SquareCount]{};
inline uint64_t pawn_table[ColourCount][SquareCount]{};

// Functions ------------------------------------------------------------------------

inline uint64_t calculate_King(Square square){
    uint64_t initial_Kings = setSquare(0, square);
    uint64_t attacks = 0; 

    for(const auto& d : northKingDirs)
        attacks |= (initial_Kings << d.shift) & d.mask;
    for(const auto& d: southKingDirs)
        attacks |= (initial_Kings >> d.shift) & d.mask;
    
    return attacks;
}


inline uint64_t calculate_knight(Square square){
    uint64_t initial_knights = setSquare(0, square);
    uint64_t attacks = 0; 

    for(const auto& d : northDirs)
        attacks |= (initial_knights << d.shift) & d.mask;
    for(const auto& d: southDirs)
        attacks |= (initial_knights >> d.shift) & d.mask;
    
    return attacks;
}






inline uint64_t calculate_Pawn_White(Square square){
    uint64_t initial_Pawns = setSquare(0, square);
    uint64_t attacks_white = 0; 

    for(const auto& d : white_north_Dirs)
        attacks_white |= (initial_Pawns << d.shift) & d.mask;

    return attacks_white;
}

inline uint64_t calculate_Pawn_Black(Square square){
    uint64_t initial_Pawns = setSquare(0, square);
    uint64_t attacks_black = 0; 

    for(const auto& d: black_south_Dirs)
        attacks_black |= (initial_Pawns >> d.shift) & d.mask;
    
    return attacks_black;
}

inline void initialize_attack_tables()
{
    static bool initialized = false;
    if (initialized)
        return;

    for (int i = 0; i < SquareCount; ++i)
    {
        Square square = static_cast<Square>(i);
        king_table[i] = calculate_King(square);
        knight_table[i] = calculate_knight(square);
        pawn_table[White][i] = calculate_Pawn_White(square);
        pawn_table[Black][i] = calculate_Pawn_Black(square);
    }

    initialized = true;
}