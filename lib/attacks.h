#pragma once
#include "bitboard_utilities.h"
#include <iostream>
// Structs -----------------------------------------------------------------------
struct ShiftDir {
    int shift;
    uint64_t mask;   // applied to the result after shifting
};

enum ray_directions{north_west , north , north_east , east , south_east , south , south_west , west , direction_count };

inline uint64_t full_ray_table[direction_count][SquareCount];
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





inline uint64_t calculate_ray(ray_directions direction, Square square)
{
    assert(direction >= 0 && direction < direction_count);

    // Relies on northKingDirs and southKingDirs listing directions
    // in the same order as the ray_directions enum.
    bool positive = direction < south_east;
    ShiftDir d = positive ? northKingDirs[direction]
                          : southKingDirs[direction - south_east];

    uint64_t current = setSquare(0, square);   // start from the square itself
    uint64_t ray = 0;

    while (true)
    {
        if (positive)
            current = (current << d.shift) & d.mask;
        else
            current = (current >> d.shift) & d.mask;

        if (current == 0)
            break;   // the ray has left the board

        ray |= current;
    }

    return ray;
}


inline uint64_t calculate_raywithblocker(ray_directions direction, Square square, uint64_t occupied_bitboard)
{
    assert(direction >= 0 && direction < direction_count);

    bool positive = direction < south_east;

    uint64_t full_ray = full_ray_table[direction][square];
    uint64_t blockers = occupied_bitboard & full_ray;

    if (blockers == 0)
        return full_ray;   // nothing in the way

    Square blocker = positive ? lowestBit(blockers)
                              : highestBit(blockers);

    // Remove everything beyond the blocker. The ray from the blocker
    // in the same direction is exactly those squares, and it doesn't
    // include the blocker itself, so the blocker stays in the result.
    return full_ray & ~full_ray_table[direction][blocker];
}


inline uint64_t queen_attacks(Square square, uint64_t occupied){
    
    uint64_t attacks = 0;

    for (int j = 0; j < direction_count; ++j)
    {
        ray_directions d = static_cast<ray_directions>(j);
        attacks |= calculate_raywithblocker(d, square, occupied);
    }

    return attacks;
}

inline uint64_t rook_attacks(Square square, uint64_t occupied){
    
    uint64_t attacks = 0;
    constexpr ray_directions rook_directions[] = { north, east, south, west };

    for (ray_directions d : rook_directions)
    {
        
        attacks |= calculate_raywithblocker(d, square, occupied);
    }

    return attacks;
}

inline uint64_t bishop_attacks(Square square, uint64_t occupied){
    
    uint64_t attacks = 0;
    constexpr ray_directions bishop_directions[] = { north_east, south_east, south_west, north_west };

    for (ray_directions d : bishop_directions)
    {
        
        attacks |= calculate_raywithblocker(d, square, occupied);
    }

    return attacks;
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
        for (int j = 0; j < direction_count; j++)
            full_ray_table[j][i] = calculate_ray(static_cast<ray_directions>(j), square);
    }

    initialized = true;
}





