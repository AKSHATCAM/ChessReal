#pragma once
#include <iostream>
#include <cassert>
#include "attacks.h"
#include "position.h"
#include "bitboard_utilities.h"
#include "movegen.h"
#include "makemove.h"


inline uint64_t perft(const Position& pos, int depth)
{
    if (depth == 0)
        return 1;

    MoveList list;
    generate_legal_moves(list, pos);

    // Shortcut: at depth 1, the count is just the number of legal moves
    if (depth == 1)
        return list.count;

    uint64_t total = 0;

    for (int i = 0; i < list.count; ++i)
    {
        Position position_after = pos;
        make_move(position_after, list.move_array[i]);
        total += perft(position_after, depth - 1);
    }

    return total;
}


// Prints each legal first move with the number of positions reachable after it,
// then the total. Used to find perft bugs by comparing with Stockfish's "go perft".
inline uint64_t divide(const Position& pos, int depth)
{
    MoveList list;
    generate_legal_moves(list, pos);

    uint64_t total = 0;

    for (int i = 0; i < list.count; ++i)
    {
        const Move& move = list.move_array[i];

        Position position_after = pos;
        make_move(position_after, move);

        uint64_t count = perft(position_after, depth - 1);

        print_move(move);
        std::cout << ": " << count << "\n";

        total += count;
    }

    std::cout << "\nTotal: " << total << "\n";
    return total;
}


