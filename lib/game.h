#pragma once
#include <string>
#include "makemove.h"

const std::string starting_pos_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";


// ---- Status helpers ----

// True if the side to move's king is attacked
inline bool in_check(const Position& pos)
{
    Square king_square = lowestBit(pos.main_bitboard[pos.side_to_move][Position::King]);
    Colour attacker = (pos.side_to_move == White) ? Black : White;
    return is_square_attacked(pos, king_square, attacker);
}

enum GameStatus { Ongoing, Checkmate, Stalemate, FiftyMoveDraw };

// `legal` must be the legal moves for `pos` (passed in so they aren't generated twice)
inline GameStatus get_game_status(const Position& pos, const MoveList& legal)
{
    if (legal.count == 0)
        return in_check(pos) ? Checkmate : Stalemate;

    if (pos.halfmove_clock >= 100)
        return FiftyMoveDraw;

    return Ongoing;
}

// Convenience version that generates the legal moves itself
inline GameStatus get_game_status(const Position& pos)
{
    MoveList legal;
    generate_legal_moves(legal, pos);
    return get_game_status(pos, legal);
}


// ---- Engine interface for the frontend ----

class Game {
public:
    // Start from the standard starting position
    bool new_game()
    {
        return load_fen(starting_pos_fen);
    }

    // Load any position; returns false if the FEN is invalid
    bool load_fen(const std::string& fen)
    {
        if (!fen_parser(current_position, fen))
            return false;

        update_legal_moves();
        return true;
    }

    // The legal moves in the current position
    const MoveList& legal_moves() const
    {
        return current_legal_moves;
    }

    // Finds the legal move matching from/to/promotion and makes it.
    // For non-promotions, leave promotion as the default (PieceCount).
    // Returns false (and changes nothing) if no legal move matches.
    bool try_move(Square from, Square to, Position::PieceType promotion = Position::PieceCount)
    {
        for (int i = 0; i < current_legal_moves.count; ++i)
        {
            const Move& move = current_legal_moves.move_array[i];

            if (from_square(move) == from &&
                to_square(move) == to &&
                promotion_of(move) == promotion)
            {
                make_move(current_position, move);
                update_legal_moves();
                return true;
            }
        }
        return false;
    }

    GameStatus status() const
    {
        return get_game_status(current_position, current_legal_moves);
    }

    Colour side_to_move() const
    {
        return current_position.side_to_move;
    }

    // Colour and type of the piece on a square ({ColourCount, PieceCount} if empty)
    Position::PieceOnSquare piece_at(Square sq) const
    {
        return current_position.pieceAt(sq);
    }

    // Read-only access, e.g. for printing or highlighting checks
    const Position& position() const
    {
        return current_position;
    }

    bool isSquareonSidetoMove(Square sq) const
    {
        return current_position.pieceAt(sq).colour == current_position.side_to_move;
    }

    uint64_t legal_for_square(Square sq) const
    {
        uint64_t targets = 0;
        for (int i = 0; i < current_legal_moves.count; i++)
        {
            const Move& move = current_legal_moves.move_array[i];
            if (move.from == sq)
                targets = setSquare(targets, move.to);
        }
        return targets;
    }

private:
    Position current_position{};
    MoveList current_legal_moves{};

    // Must be called after every change to the position
    void update_legal_moves()
    {
        generate_legal_moves(current_legal_moves, current_position);
    }
};