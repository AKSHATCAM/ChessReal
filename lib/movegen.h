#pragma once
#include <iostream>
#include <cassert>
#include "attacks.h"
#include "position.h"
#include "bitboard_utilities.h"


// Each flag is its own bit, so flags can be combined (e.g. Capture | EnPassant).
// A normal move has no flags set: move_flags == 0.
enum flag_types {
    Capture    = 1,   // 0001
    DoublePush = 2,   // 0010
    EnPassant  = 4,   // 0100
    Castling   = 8    // 1000
};

// Signed shift: positive amounts shift up, negative amounts shift down.
// (Could live in bitboard_utilities.h, since it's a general bitboard tool.)

struct Move {
    Square from = a1;
    Square to = a1;
    Position::PieceType promotion_piece = Position::PieceCount;   // PieceCount = no promotion
    uint8_t move_flags = 0;                                        // 0 = normal move

    
};

constexpr Position::PieceType promotion_pieces[] = {
        Position::Queen, Position::Rook, Position::Bishop, Position::Knight
};


// ---- Accessors: the rest of the engine uses these, never the fields directly ----

inline Square from_square(const Move& move)
{
    return move.from;
}

inline Square to_square(const Move& move)
{
    return move.to;
}

inline Position::PieceType promotion_of(const Move& move)
{
    return move.promotion_piece;
}

inline bool is_capture(const Move& move)
{
    return (move.move_flags & Capture) != 0;
}

inline bool is_double_push(const Move& move)
{
    return (move.move_flags & DoublePush) != 0;
}

inline bool is_en_passant(const Move& move)
{
    return (move.move_flags & EnPassant) != 0;
}

inline bool is_castling(const Move& move)
{
    return (move.move_flags & Castling) != 0;
}

inline bool is_promotion(const Move& move)
{
    return promotion_of(move) != Position::PieceCount;
}


// ---- Move list ----

struct MoveList {
    static constexpr int size_array = 256;
    Move move_array[size_array]{};
    int count = 0;
    void clear(){
        count = 0;
    }
};

inline void move_adder(Move move_to_add, MoveList& list)
{
    assert(list.count < list.size_array);

    list.move_array[list.count] = move_to_add;
    list.count++;
}

// Adds a pawn move with the given flags. If it lands on the promotion rank,
// it is added four times, once for each promotion piece.
inline void add_pawn_move(Move move_to_add, uint64_t promotion_rank, MoveList& list, uint8_t flag)
{


    move_to_add.move_flags = flag;

    if (testSquare(promotion_rank, move_to_add.to))
    {
        for (Position::PieceType piece : promotion_pieces)
        {
            move_to_add.promotion_piece = piece;
            move_adder(move_to_add, list);
        }
    }
    else
    {
        move_adder(move_to_add, list);
    }
}


// ---- Printing ----

inline void print_move(Move move)
{
    char file_from = 'a' + fileOf(from_square(move));
    int  rank_from = rankOf(from_square(move)) + 1;
    char file_to   = 'a' + fileOf(to_square(move));
    int  rank_to   = rankOf(to_square(move)) + 1;
    const char mapping[Position::PieceCount] = {'p', 'n', 'b', 'r', 'q', 'k'};

    std::cout << file_from << rank_from << file_to << rank_to;
    if (is_promotion(move))
        std::cout << mapping[promotion_of(move)];
}

inline void print_move_list(const MoveList& list)
{
    std::cout << list.count << " moves:";

    for (int i = 0; i < list.count; ++i)
    {
        std::cout << " ";
        print_move(list.move_array[i]);
    }

    std::cout << "\n";
}


// ---- Pawn moves ----

inline void pawn_generator(MoveList& list, const Position& current_position)
{
    Colour side_to_move = current_position.side_to_move;
    bool white = (side_to_move == White);
    Colour enemy = white ? Black : White;

    uint64_t pawns         = current_position.main_bitboard[side_to_move][Position::Pawn];
    uint64_t empty_squares = ~current_position.all_occupied_squares;
    uint64_t enemy_pieces  = current_position.occupancyarray[enemy];

    // Colour-dependent values, chosen once
    uint64_t promotion_rank   = white ? Masks::rank_8 : Masks::rank_1;
    int      direction        = white ? 8 : -8;
    uint64_t double_push_rank = white ? Masks::rank_3 : Masks::rank_6;

    // ---- Pushes ----
    uint64_t pushed_by_one = shift(pawns, direction) & empty_squares;
    uint64_t pushed_by_two = shift(pushed_by_one & double_push_rank, direction) & empty_squares;

    while (pushed_by_one)
    {
        Move temp;
        temp.to   = poplowestBit(pushed_by_one);
        temp.from = static_cast<Square>(temp.to - direction);
        add_pawn_move(temp, promotion_rank, list, 0);
    }

    while (pushed_by_two)
    {
        Move temp;
        temp.to   = poplowestBit(pushed_by_two);
        temp.from = static_cast<Square>(temp.to - direction * 2);
        add_pawn_move(temp, promotion_rank, list, DoublePush);
    }

    // ---- Captures and en passant ----
    uint64_t remaining_pawns = pawns;   // copy, so we can pop from it

    while (remaining_pawns)
    {
        Square from = poplowestBit(remaining_pawns);

        // Geometry for this square, filtered by the position
        uint64_t attacks  = pawn_table[side_to_move][from];
        uint64_t captures = attacks & enemy_pieces;

        while (captures)
        {
            Move temp;
            temp.from = from;
            temp.to   = poplowestBit(captures);
            add_pawn_move(temp, promotion_rank, list, Capture);
        }

        // En passant: the target square is empty, so it isn't in enemy_pieces
        if (attacks & current_position.enpassant_bitboard)
        {
            Move temp;
            temp.from = from;
            temp.to   = lowestBit(current_position.enpassant_bitboard);
            add_pawn_move(temp, promotion_rank, list, Capture | EnPassant);
        }
    }
}


// ---- Knight and king moves ----

inline void knight_generator(MoveList& list, const Position& current_position)
{
    Colour side_to_move = current_position.side_to_move;
    Colour enemy = (side_to_move == White) ? Black : White;

    uint64_t knights      = current_position.main_bitboard[side_to_move][Position::Knight];
    uint64_t own_pieces   = current_position.occupancyarray[side_to_move];
    uint64_t enemy_pieces = current_position.occupancyarray[enemy];

    while (knights)
    {
        Move temp;
        temp.from = poplowestBit(knights);

        // Every attacked square except those holding our own pieces
        uint64_t targets = knight_table[temp.from] & ~own_pieces;

        while (targets)
        {
            temp.to = poplowestBit(targets);
            temp.move_flags = testSquare(enemy_pieces, temp.to) ? Capture : 0;
            move_adder(temp, list);
        }
    }
}

inline void king_generator(MoveList& list, const Position& current_position)
{
    Colour side_to_move = current_position.side_to_move;
    Colour enemy = (side_to_move == White) ? Black : White;

    uint64_t king         = current_position.main_bitboard[side_to_move][Position::King];
    uint64_t own_pieces   = current_position.occupancyarray[side_to_move];
    uint64_t enemy_pieces = current_position.occupancyarray[enemy];

    while (king)
    {
        Move temp;
        temp.from = poplowestBit(king);

        uint64_t targets = king_table[temp.from] & ~own_pieces;

        while (targets)
        {
            temp.to = poplowestBit(targets);
            temp.move_flags = testSquare(enemy_pieces, temp.to) ? Capture : 0;
            move_adder(temp, list);
        }
    }
}

// ---- Slider moves (bishops, rooks, queens) ----

inline void slider_generator(MoveList& list, const Position& current_position)
{
    Colour side_to_move = current_position.side_to_move;
    Colour enemy = (side_to_move == White) ? Black : White;

    uint64_t own_pieces   = current_position.occupancyarray[side_to_move];
    uint64_t enemy_pieces = current_position.occupancyarray[enemy];
    uint64_t occupied     = current_position.all_occupied_squares;

    constexpr Position::PieceType slider_types[] = {
        Position::Bishop, Position::Rook, Position::Queen
    };

    for (Position::PieceType type : slider_types)
    {
        uint64_t pieces = current_position.main_bitboard[side_to_move][type];

        while (pieces)
        {
            Move temp;
            temp.from = poplowestBit(pieces);

            uint64_t attacks = 0;
            switch (type)
            {
                case Position::Bishop: 
                    attacks = bishop_attacks(temp.from, occupied);
                    break;
                case Position::Rook:
                    attacks = rook_attacks(temp.from, occupied);
                    break;
                case Position::Queen:
                    attacks = queen_attacks(temp.from, occupied);
                    break;
                default: 
                    break;
            }

            // Every attacked square except those holding our own pieces
            uint64_t targets = attacks & ~own_pieces;

            while (targets)
            {
                temp.to = poplowestBit(targets);
                temp.move_flags = testSquare(enemy_pieces, temp.to) ? Capture : 0;
                move_adder(temp, list);
            }
        }
    }
}

inline void castling_generator(MoveList& list, const Position& current_position)
{
    uint8_t castling_status = current_position.CastlingRightStatus;
    uint64_t occupied = current_position.all_occupied_squares;
    Colour side_to_move = current_position.side_to_move;

    Move temp;
    temp.move_flags = Castling;

    if (side_to_move == White)
    {
        // Kingside: right set, and f1, g1 empty
        if ((castling_status & Position::white_kingside) &&
            (occupied & Masks::white_kingside_mask) == 0)
        {
            temp.from = e1;
            temp.to = g1;
            move_adder(temp, list);
        }

        // Queenside: right set, and b1, c1, d1 empty
        if ((castling_status & Position::white_queenside) &&
            (occupied & Masks::white_queenside_mask) == 0)
        {
            temp.from = e1;
            temp.to = c1;
            move_adder(temp, list);
        }
    }
    else
    {
        // Kingside: right set, and f8, g8 empty
        if ((castling_status & Position::black_kingside) &&
            (occupied & Masks::black_kingside_mask) == 0)
        {
            temp.from = e8;
            temp.to = g8;
            move_adder(temp, list);
        }

        // Queenside: right set, and b8, c8, d8 empty
        if ((castling_status & Position::black_queenside) &&
            (occupied & Masks::black_queenside_mask) == 0)
        {
            temp.from = e8;
            temp.to = c8;
            move_adder(temp, list);
        }
    }
}


inline void generate_moves_total(MoveList& list, const Position& current_position){


    list.clear();

    pawn_generator(list, current_position);
    knight_generator(list, current_position);
    slider_generator(list, current_position);
    king_generator(list, current_position);
    castling_generator(list, current_position);

}


inline MoveList sanity_check(const MoveList& list, const Position& pos)
{
    MoveList bad_moves;

    // ---- Setup ----
    Colour side  = pos.side_to_move;
    Colour enemy = (side == White) ? Black : White;

    uint64_t own          = pos.occupancyarray[side];
    uint64_t enemy_pieces = pos.occupancyarray[enemy];
    uint64_t occupied     = pos.all_occupied_squares;
    uint64_t last_rank    = (side == White) ? Masks::rank_8 : Masks::rank_1;
    Square   king_start   = (side == White) ? e1 : e8;


    // ---- 1. Duplicates (seen table) ----
    constexpr int table_length = 64 * 64 * 5;
    bool table[table_length]{};

    for (int i = 0; i < list.count; ++i)
    {
        const Move& move = list.move_array[i];

        int promotion_index = is_promotion(move) ? promotion_of(move) : 0;
        int key = (from_square(move) * 64 + to_square(move)) * 5 + promotion_index;

        if (table[key])
        {
            std::cout << "duplicate: ";
            print_move(move);
            std::cout << "\n";
            move_adder(move, bad_moves);
        }
        else
        {
            table[key] = true;
        }
    }


    // ---- 2. Per-move checks ----
    for (int i = 0; i < list.count; ++i)
    {
        const Move& move = list.move_array[i];

        Square from = from_square(move);
        Square to   = to_square(move);
        Position::PieceOnSquare mover = pos.pieceAt(from);

        bool ok = true;

        // Starts on one of our own pieces
        if (!testSquare(own, from))
        {
            std::cout << "doesn't start on own piece: ";
            ok = false;
        }

        // Doesn't land on one of our own pieces
        if (testSquare(own, to))
        {
            std::cout << "lands on own piece: ";
            ok = false;
        }

        // Capture flag is consistent
        if (is_en_passant(move))
        {
            if (!testSquare(pos.enpassant_bitboard, to))
            {
                std::cout << "en passant not onto en passant square: ";
                ok = false;
            }
        }
        else if (is_capture(move))
        {
            if (!testSquare(enemy_pieces, to))
            {
                std::cout << "capture doesn't land on enemy piece: ";
                ok = false;
            }
        }
        else
        {
            if (testSquare(occupied, to))
            {
                std::cout << "non-capture lands on occupied square: ";
                ok = false;
            }
        }

        // Promotions: made by pawns, landing on the last rank
        if (is_promotion(move))
        {
            if (mover.type != Position::Pawn)
            {
                std::cout << "promotion not made by a pawn: ";
                ok = false;
            }
            if (!testSquare(last_rank, to))
            {
                std::cout << "promotion not onto last rank: ";
                ok = false;
            }
        }

        // Castling starts on e1 (white) or e8 (black)
        if (is_castling(move) && from != king_start)
        {
            std::cout << "castling not from king's starting square: ";
            ok = false;
        }

        if (!ok)
        {
            print_move(move);
            std::cout << "\n";
            move_adder(move, bad_moves);
        }
    }

    return bad_moves;
}




