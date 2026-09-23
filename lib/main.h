#pragma once
#include <iostream>
#include <cstdint>

class Position{
    public:

    enum PieceType {Pawn, Knight, Bishop, Rook, Queen, King, PieceCount};

    enum Colour {White, Black, ColourCount};

    uint64_t main_bitboard[Colour::ColourCount][PieceType::PieceCount]{};

    Colour side_to_move = White;

    enum CastlingRight{white_kingside = 1, white_queenside = 2, black_kingside = 4, black_queenside = 8};
    
    uint8_t CastlingRightStatus = 0;

    uint64_t enpassant_bitboard = 0 ; //stores the one square where the capture is. Always on rank 3 or 6. 

    uint64_t occupancyarray[ColourCount]{};

    uint64_t all_occupied_squares = 0;

    void derived_bitboard(){
        for(int i = 0; i < ColourCount; ++i)
        {
            occupancyarray[i] = 0;
            for(int j=0; j<PieceCount;++j)
            {
                occupancyarray[i] |= main_bitboard[i][j];
            }
        }
        all_occupied_squares = occupancyarray[White] | occupancyarray[Black];
    }

    void clear(){
        
        CastlingRightStatus = 0;
        all_occupied_squares = 0;
        enpassant_bitboard = 0;
        side_to_move = White;

        for(int i = 0; i < ColourCount; ++i)
        {   
            for(int j=0; j<PieceCount;++j)
            {
                main_bitboard[i][j] = 0;

            }
            occupancyarray[i] = 0;
        }
        
    }
};

