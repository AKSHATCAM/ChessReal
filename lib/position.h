#pragma once
#include <iostream>
#include <cstdint>
#include "bitboard_utilities.h"



class Position{
    public:

    enum PieceType {Pawn, Knight, Bishop, Rook, Queen, King, PieceCount};

    uint64_t main_bitboard[ColourCount][PieceType::PieceCount]{};

    Colour side_to_move = White;

    enum CastlingRight{white_kingside = 1, white_queenside = 2, black_kingside = 4, black_queenside = 8};
    
    uint8_t CastlingRightStatus = 0;

    uint64_t enpassant_bitboard = 0 ; //stores the one square where the capture is. Always on rank 3 or 6. 

    uint64_t occupancyarray[ColourCount]{};

    uint64_t all_occupied_squares = 0;

    struct PieceOnSquare {
        Colour colour;
        PieceType type;
    };

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

    PieceOnSquare pieceAt(Square sq) const
    {
        for (int c = 0; c < ColourCount; ++c)
        {
            for (int p = 0; p < PieceCount; ++p)
            {
                if (testSquare(main_bitboard[c][p], sq))
                    return { static_cast<Colour>(c), static_cast<PieceType>(p) };
            }
        }

        return { ColourCount, PieceCount }; // empty square
    }
    
};


inline void printBoard(const Position& pos)
{
    // Row 0 is White (uppercase), row 1 is Black (lowercase), matching the Colour enum
    const char mapping[ColourCount][Position::PieceCount] = {
        {'P', 'N', 'B', 'R', 'Q', 'K'},
        {'p', 'n', 'b', 'r', 'q', 'k'}
    };

    std::cout << "\n\n";

    for (int i = 0; i < 8; ++i)
    {
        std::cout << 8 - i << "    ";
        for (int j = 0; j < 8; ++j)
        {
            // Row i on screen is rank 8 - i, so the square index is (7 - i) * 8 + j
            Square square = static_cast<Square>((7 - i) * 8 + j);
            Position::PieceOnSquare current_piece = pos.pieceAt(square);

            if (current_piece.type == Position::PieceCount)
                std::cout << ". ";   // empty square
            else
                std::cout << mapping[current_piece.colour][current_piece.type] << " ";
        }
        std::cout << "\n";
    }

    std::cout << "\n     ";
    for (char file = 'a'; file <= 'h'; ++file)
        std::cout << file << " ";

    std::cout << "\n\n";

    // ---- Side to move ----
    std::cout << "Side to move: "
              << (pos.side_to_move == White ? "White" : "Black") << "\n";

    // ---- Castling rights, in FEN order (KQkq), or "-" if none ----
    std::string castling;
    if (pos.CastlingRightStatus & Position::white_kingside)  castling += 'K';
    if (pos.CastlingRightStatus & Position::white_queenside) castling += 'Q';
    if (pos.CastlingRightStatus & Position::black_kingside)  castling += 'k';
    if (pos.CastlingRightStatus & Position::black_queenside) castling += 'q';
    if (castling.empty()) castling = "-";
    std::cout << "Castling:     " << castling << "\n";

    // ---- En passant square, as a name like "e3", or "-" if none ----
    std::cout << "En passant:   ";
    if (pos.enpassant_bitboard == 0)
    {
        std::cout << "-";
    }
    else
    {
        Square ep = lowestBit(pos.enpassant_bitboard);
        std::cout << static_cast<char>('a' + fileOf(ep))
                  << static_cast<char>('1' + rankOf(ep));
    }
    std::cout << "\n\n\n";
}

inline bool fen_parser(Position& pos, const std::string& fen)
{
    pos.clear();   // wipe any previous position first

    // ---- Split the FEN into its fields ----
    // The stream splits on whitespace, so extra spaces between fields are fine.
    std::istringstream stream(fen);
    std::string placement, side, castling, enpassant;
    stream >> placement >> side >> castling >> enpassant;

    if (placement.empty() || side.empty() || castling.empty() || enpassant.empty())
        return false;   // a required field is missing

    const std::string fen_type = "pnbrqk";   // same order as PieceType

    int count = 0;        // squares visited so far, counting from a8 across then down
    int rank_count = 0;   // squares visited in the current rank

    // ---- Field 1: piece placement ----
    for (char ch : placement)
    {
        if (ch == '/')
        {
            if (rank_count != 8)
                return false;   // the rank just finished didn't have exactly 8 squares
            rank_count = 0;
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(ch)))
        {
            int empty = ch - '0';
            if (empty < 1 || empty > 8)
                return false;   // only 1-8 are valid empty-square counts

            count += empty;
            rank_count += empty;

            if (rank_count > 8)
                return false;   // too many squares in this rank
            continue;
        }

        // Find the index of this letter in fen_type
        char lower = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        std::size_t index = fen_type.find(lower);
        if (index == std::string::npos)
            return false;   // unknown character

        if (rank_count >= 8)
            return false;   // a piece would be a 9th square in this rank

        Position::PieceType current_piece = static_cast<Position::PieceType>(index);

        // Uppercase is white, lowercase is black
        Colour current_colour =
            std::isupper(static_cast<unsigned char>(ch)) ? White : Black;

        // count runs a8, b8, ... h8, a7, ... but the bits start at a1,
        // so convert count into a rank and file first
        int rank = 7 - count / 8;
        int file = count % 8;
        Square sq = makeSquare(rank, file);

        pos.main_bitboard[current_colour][current_piece] =
            setSquare(pos.main_bitboard[current_colour][current_piece], sq);

        count++;
        rank_count++;
    }

    // The last rank has no '/' after it, so check it here,
    // then check the whole board covers exactly 64 squares (8 ranks of 8).
    if (rank_count != 8 || count != 64)
        return false;

    // ---- Field 2: side to move ----
    if (side == "w")      pos.side_to_move = White;
    else if (side == "b") pos.side_to_move = Black;
    else return false;

    // ---- Field 3: castling rights ----
    if (castling != "-")
    {
        for (char ch : castling)
        {
            switch (ch)
            {
                case 'K': pos.CastlingRightStatus |= Position::white_kingside;  break;
                case 'Q': pos.CastlingRightStatus |= Position::white_queenside; break;
                case 'k': pos.CastlingRightStatus |= Position::black_kingside;  break;
                case 'q': pos.CastlingRightStatus |= Position::black_queenside; break;
                default:  return false;
            }
        }
    }

    // ---- Field 4: en passant square ----
    if (enpassant != "-")
    {
        if (enpassant.size() != 2)
            return false;

        int epFile = enpassant[0] - 'a';
        int epRank = enpassant[1] - '1';

        if (epFile < 0 || epFile > 7 || epRank < 0 || epRank > 7)
            return false;

        pos.enpassant_bitboard = setSquare(0, makeSquare(epRank, epFile));
    }

    // Fields 5 and 6 (clocks) are ignored for now.

    pos.derived_bitboard();   // rebuild occupancy from the finished piece boards
    return true;
}



