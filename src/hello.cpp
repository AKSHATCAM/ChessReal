#include <iostream>
#include <string>
#include "../lib/bitboard_utilities.h"
#include "../lib/attacks.h"
#include "../lib/position.h"
#include "../lib/movegen.h"

using namespace std;

int failures = 0;

// Loads a FEN, prints the board and every generated move, checks the count.
void runTest(const string& title, const string& fen, int expected)
{
    Position pos;
    fen_parser(pos, fen);

    MoveList list;
    generate_moves_total(list, pos);

    cout << "\n==================================================\n";
    cout << title << "\n";
    cout << "==================================================\n";

    printBoard(pos);
    print_move_list(list);

    bool pass = (list.count == expected);
    if (!pass) failures++;

    cout << (pass ? "PASS" : "FAIL") << "  (got " << list.count
         << ", expected " << expected << ")\n";
}

int main()
{
    initialize_attack_tables();

    const string START = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    const string KIWI  = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";

    runTest("Starting position, white: 16 pawn + 4 knight",
            START, 20);

    runTest("Starting position, black to move",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1", 20);

    runTest("After 1. e4, black to move",
            "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1", 20);

    runTest("Rooks and king with castling: 5 king + 2 castling + 10 + 9 rook",
            "4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1", 26);

    runTest("Kiwipete: 8 pawn + 11 knight + 25 slider + 2 king + 2 castling",
            KIWI, 48);

    // ---- Reuse: the same list must be cleared between calls ----
    {
        Position pos;
        MoveList list;

        fen_parser(pos, START);
        generate_moves_total(list, pos);

        fen_parser(pos, KIWI);
        generate_moves_total(list, pos);   // same list again

        bool pass = (list.count == 48);
        if (!pass) failures++;

        cout << "\n==================================================\n";
        cout << "Reusing one list: start, then Kiwipete\n";
        cout << "==================================================\n";
        cout << (pass ? "PASS" : "FAIL") << "  (got " << list.count
             << ", expected 48, not 68)\n";
    }

    cout << "\n" << (failures == 0 ? "ALL TESTS PASSED" : "SOME TESTS FAILED")
         << " (" << failures << " failures)\n";
}