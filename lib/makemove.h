#pragma once
#include <iostream>
#include <cassert>
#include "attacks.h"
#include "position.h"
#include "bitboard_utilities.h"
#include "movegen.h"

// enum PieceType{Pawn, Knight, Bishop, Rook, Queen, King, PieceCount};

// Every square attacked by `attacker` (including squares with its own pieces on them)
inline uint64_t attacked_squares(const Position& pos, Colour attacker)
{
    uint64_t occupied = pos.all_occupied_squares;
    uint64_t attacks = 0;

    uint64_t pawns = pos.main_bitboard[attacker][Position::Pawn];
    while (pawns)
        attacks |= pawn_table[attacker][poplowestBit(pawns)];

    uint64_t knights = pos.main_bitboard[attacker][Position::Knight];
    while (knights)
        attacks |= knight_table[poplowestBit(knights)];

    uint64_t bishops = pos.main_bitboard[attacker][Position::Bishop];
    while (bishops)
        attacks |= bishop_attacks(poplowestBit(bishops), occupied);

    uint64_t rooks = pos.main_bitboard[attacker][Position::Rook];
    while (rooks)
        attacks |= rook_attacks(poplowestBit(rooks), occupied);

    uint64_t queens = pos.main_bitboard[attacker][Position::Queen];
    while (queens)
        attacks |= queen_attacks(poplowestBit(queens), occupied);

    uint64_t king = pos.main_bitboard[attacker][Position::King];
    while (king)
        attacks |= king_table[poplowestBit(king)];

    return attacks;
}

inline bool is_square_attacked(const Position& pos, Square square, Colour attacker)
{
    Colour defender  = (attacker == White) ? Black : White;
    uint64_t occupied = pos.all_occupied_squares;

    // Knights: a knight on `square` would reach exactly the squares
    // from which an enemy knight attacks `square`
    if (knight_table[square] & pos.main_bitboard[attacker][Position::Knight])
        return true;

    // King: same symmetry
    if (king_table[square] & pos.main_bitboard[attacker][Position::King])
        return true;

    // Pawns: use the DEFENDER's pawn pattern from `square`.
    // Attacking pawns sit diagonally in front of the square from the defender's side.
    if (pawn_table[defender][square] & pos.main_bitboard[attacker][Position::Pawn])
        return true;

    // Diagonals: bishops and queens
    uint64_t diagonal_attackers = pos.main_bitboard[attacker][Position::Bishop]
                                | pos.main_bitboard[attacker][Position::Queen];
    if (bishop_attacks(square, occupied) & diagonal_attackers)
        return true;

    // Straight lines: rooks and queens
    uint64_t straight_attackers = pos.main_bitboard[attacker][Position::Rook]
                                | pos.main_bitboard[attacker][Position::Queen];
    if (rook_attacks(square, occupied) & straight_attackers)
        return true;

    return false;
}

inline uint8_t rights_lost_on(Square sq)
{
    switch (sq)
    {
        case e1: return Position::white_kingside | Position::white_queenside;
        case h1: return Position::white_kingside;
        case a1: return Position::white_queenside;
        case e8: return Position::black_kingside | Position::black_queenside;
        case h8: return Position::black_kingside;
        case a8: return Position::black_queenside;
        default: return 0;
    }
}
 
 
// Applies a move to the position. To keep the original, copy it first:
//     Position after = pos;
//     make_move(after, move);
inline void make_move(Position& pos, const Move& move)
{
    Square from  = from_square(move);
    Square to    = to_square(move);
    Colour side  = pos.side_to_move;
    Colour enemy = (side == White) ? Black : White;
    bool   white = (side == White);
 
    // ---- Find the moving piece ----
    Position::PieceOnSquare mover = pos.pieceAt(from);
    assert(mover.type != Position::PieceCount);   // there must be a piece on the from-square
    assert(mover.colour == side);                 // and it must belong to the side to move
    Position::PieceType type = mover.type;
 
 
    // ================================================================
    // Part 1: remove a captured piece
    // ================================================================
    if (is_en_passant(move))
    {
        // The captured pawn is one rank BEHIND the to-square, from the mover's point of view
        Square captured_sq = static_cast<Square>(white ? to - 8 : to + 8);
        uint64_t& enemy_pawns = pos.main_bitboard[enemy][Position::Pawn];
        enemy_pawns = clearSquare(enemy_pawns, captured_sq);
    }
    else if (is_capture(move))
    {
        // Must be done BEFORE our piece moves onto the square
        Position::PieceOnSquare captured = pos.pieceAt(to);
        assert(captured.type != Position::PieceCount && captured.colour == enemy);
 
        uint64_t& enemy_board = pos.main_bitboard[enemy][captured.type];
        enemy_board = clearSquare(enemy_board, to);
    }
 
 
    // ================================================================
    // Part 1 (continued): move our piece
    // ================================================================
    uint64_t& mover_board = pos.main_bitboard[side][type];
    mover_board = clearSquare(mover_board, from);
    mover_board = setSquare(mover_board, to);
 
 
    // ================================================================
    // Part 2: special moves
    // ================================================================
 
    // Promotion: the pawn now on the to-square becomes the promotion piece
    if (is_promotion(move))
    {
        uint64_t& pawns = pos.main_bitboard[side][Position::Pawn];
        pawns = clearSquare(pawns, to);
 
        uint64_t& promoted = pos.main_bitboard[side][promotion_of(move)];
        promoted = setSquare(promoted, to);
    }
 
    // Castling: the king has moved, now move the rook
    if (is_castling(move))
    {
        Square rook_from = a1;
        Square rook_to   = a1;
 
        switch (to)
        {
            case g1: rook_from = h1; rook_to = f1; break;
            case c1: rook_from = a1; rook_to = d1; break;
            case g8: rook_from = h8; rook_to = f8; break;
            case c8: rook_from = a8; rook_to = d8; break;
            default: assert(false && "castling move with unexpected destination");
        }
 
        uint64_t& rooks = pos.main_bitboard[side][Position::Rook];
        rooks = clearSquare(rooks, rook_from);
        rooks = setSquare(rooks, rook_to);
    }
 
 
    // ================================================================
    // Part 3: state updates
    // ================================================================
 
    // En passant square: cleared after every move, set only after a double push
    pos.enpassant_bitboard = 0;
    if (is_double_push(move))
    {
        // The skipped square is halfway between from and to
        Square skipped = static_cast<Square>((from + to) / 2);
        pos.enpassant_bitboard = setSquare(0, skipped);
    }
 
    // Castling rights: lost if a move starts or ends on a king or rook square
    pos.CastlingRightStatus &= static_cast<uint8_t>(~(rights_lost_on(from) | rights_lost_on(to)));
 
    // Clocks
    if (type == Position::Pawn || is_capture(move))
        pos.halfmove_clock = 0;
    else
        pos.halfmove_clock++;
 
    if (side == Black)
        pos.fullmove_number++;
 
    // Side to move: switched LAST, since everything above needs to know who moved
    pos.side_to_move = enemy;
 
    // Occupancy boards must match the new piece boards
    pos.derived_bitboard();
}


inline bool is_castling_legal(const Move& move, const Position& pos)
{
    assert(is_castling(move));

    Colour side  = pos.side_to_move;
    Colour enemy = (side == White) ? Black : White;
    Square king_square = (side == White) ? e1 : e8;

    // Can't castle out of check
    if (is_square_attacked(pos, king_square, enemy))
        return false;

    // Can't pass through (or land on) an attacked square
    switch (to_square(move))
    {
        case g1:
            return !(is_square_attacked(pos, f1, enemy) || is_square_attacked(pos, g1, enemy));
        case c1:
            return !(is_square_attacked(pos, d1, enemy) || is_square_attacked(pos, c1, enemy));
        case g8:
            return !(is_square_attacked(pos, f8, enemy) || is_square_attacked(pos, g8, enemy));
        case c8:
            return !(is_square_attacked(pos, d8, enemy) || is_square_attacked(pos, c8, enemy));
        default:
            assert(false && "castling move with unexpected destination");
            return false;
    }
}

inline void generate_legal_moves(MoveList& list, const Position& pos)
{
    list.clear();

    MoveList pseudo_legal;
    generate_moves_total(pseudo_legal, pos);

    Colour mover = pos.side_to_move;   // read BEFORE any move is made

    for(int i = 0; i < pseudo_legal.count; ++i)
    {
        const Move& move = pseudo_legal.move_array[i];

        if (is_castling(move))
        {
            // Castling rules are checked on the original position
            if (is_castling_legal(move, pos))
                move_adder(move, list);
        }
        else
        {
            Position position_after = pos;
            make_move(position_after, move);

            // The mover's king, after the move (it may have moved)
            Square king_square = lowestBit(position_after.main_bitboard[mover][Position::King]);

            // The opponent is now the side to move in position_after
            if (!is_square_attacked(position_after, king_square, position_after.side_to_move))
                move_adder(move, list);
        }
    }
}