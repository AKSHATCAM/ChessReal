#include <iostream>
#include <string>
#include <initializer_list>
#include "../lib/bitboard_utilities.h"
#include "../lib/attacks.h"
#include "../lib/position.h"

using namespace std;

// ============================================================================
// Test helpers
// ============================================================================

int failures = 0;

// Builds a bitboard from a list of squares, e.g. board({a2, a3, a4})
uint64_t board(initializer_list<Square> squares)
{
    uint64_t b = 0;
    for (Square sq : squares)
        b = setSquare(b, sq);
    return b;
}

// Returns the attacks for the given slider letter: 'R', 'B' or 'Q'
uint64_t sliderAttacks(char slider, Square square, uint64_t occupied)
{
    if (slider == 'R') return rook_attacks(square, occupied);
    if (slider == 'B') return bishop_attacks(square, occupied);
    return queen_attacks(square, occupied);
}

void printHeader(const string& title)
{
    cout << "\n==================================================\n";
    cout << title << "\n";
    cout << "==================================================\n";
}

// Loads a FEN, prints the position and the slider's attacks,
// then checks the attacks against the exact expected squares.
void runTest(const string& title, const string& fen, Square square,
             char slider, uint64_t expected)
{
    Position pos;
    fen_parser(pos, fen);

    uint64_t attacks = sliderAttacks(slider, square, pos.all_occupied_squares);

    printHeader(title);

    cout << "Position:";
    printBoard(pos);

    cout << "Attacks:";
    printBitboard(attacks);

    bool pass = (attacks == expected);
    if (!pass)
    {
        failures++;
        cout << "Expected:";
        printBitboard(expected);
    }

    cout << (pass ? "PASS" : "FAIL") << "  (" << population_count(attacks)
         << " squares attacked, expected " << population_count(expected) << ")\n";
}

// Checks a count over every square on an empty board, without printing boards.
void runCountOverAllSquares(const string& title, char slider, int expectedEach, int expectedTotal)
{
    printHeader(title);

    int total = 0;
    int wrong = 0;

    for (int i = 0; i < SquareCount; ++i)
    {
        Square sq = static_cast<Square>(i);
        int count = population_count(sliderAttacks(slider, sq, 0));
        total += count;

        if (expectedEach >= 0 && count != expectedEach)
        {
            wrong++;
            cout << "  wrong count on square " << i << ": " << count << "\n";
        }
    }

    bool pass = (total == expectedTotal) && (wrong == 0);
    if (!pass) failures++;

    cout << (pass ? "PASS" : "FAIL") << "  (total " << total
         << ", expected " << expectedTotal << ")\n";
}

// ============================================================================
// Tests
// ============================================================================

int main()
{
    initialize_attack_tables();

    const string START = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    const string KIWI  = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";

    // ---- Every square on an empty board ----
    runCountOverAllSquares("Rook: 14 from every square, total 896",  'R', 14, 896);
    runCountOverAllSquares("Bishop: total over all squares 560",     'B', -1, 560);
    runCountOverAllSquares("Queen: total over all squares 1456",     'Q', -1, 1456);

    // ---- Empty board ----
    runTest("Rook on d4, empty board", "8/8/8/8/3R4/8/8/8 w - - 0 1", d4, 'R',
            board({d1, d2, d3, d5, d6, d7, d8, a4, b4, c4, e4, f4, g4, h4}));

    runTest("Bishop on d4, empty board", "8/8/8/8/3B4/8/8/8 w - - 0 1", d4, 'B',
            board({e5, f6, g7, h8, c5, b6, a7, e3, f2, g1, c3, b2, a1}));

    runTest("Queen on d4, empty board", "8/8/8/8/3Q4/8/8/8 w - - 0 1", d4, 'Q',
            board({d1, d2, d3, d5, d6, d7, d8, a4, b4, c4, e4, f4, g4, h4,
                   e5, f6, g7, h8, c5, b6, a7, e3, f2, g1, c3, b2, a1}));

    runTest("Bishop on a1 (corner), empty board", "8/8/8/8/8/8/8/B7 w - - 0 1", a1, 'B',
            board({b2, c3, d4, e5, f6, g7, h8}));

    // ---- Blockers ----
    runTest("Rook on a1, blocker on a5", "8/8/8/p7/8/8/8/R7 w - - 0 1", a1, 'R',
            board({a2, a3, a4, a5, b1, c1, d1, e1, f1, g1, h1}));

    runTest("Bishop on d4, blockers f6 and b2", "8/8/5p2/8/3B4/8/1p6/8 w - - 0 1", d4, 'B',
            board({e5, f6, c3, b2, c5, b6, a7, e3, f2, g1}));

    runTest("Queen on d4, blockers d6 f4 b6 (d8 and h4 hidden behind them)",
            "3p4/8/1p1p4/8/3Q1p1p/8/8/8 w - - 0 1", d4, 'Q',
            board({d5, d6, d3, d2, d1, e4, f4, c4, b4, a4,
                   e5, f6, g7, h8, c5, b6, e3, f2, g1, c3, b2, a1}));

    runTest("Bishop on d4, boxed in on all four diagonals",
            "8/8/8/2p1p3/3B4/2p1p3/8/8 w - - 0 1", d4, 'B',
            board({c5, e5, c3, e3}));

    // ---- Starting position ----
    runTest("Start: white rook a1",   START, a1, 'R', board({a2, b1}));
    runTest("Start: white queen d1",  START, d1, 'Q', board({c1, e1, c2, d2, e2}));
    runTest("Start: black bishop f8", START, f8, 'B', board({e7, g7}));
    runTest("Start: black rook h8",   START, h8, 'R', board({h7, g8}));

    // ---- Kiwipete ----
    runTest("Kiwipete: white queen f3", KIWI, f3, 'Q',
            board({f4, f5, f6, f2, g3, h3, e3, d3, c3, g4, h5, e4, g2, e2}));

    runTest("Kiwipete: black bishop a6", KIWI, a6, 'B',
            board({b7, c8, b5, c4, d3, e2}));

    runTest("Kiwipete: black rook h8", KIWI, h8, 'R',
            board({g8, f8, e8, h7, h6, h5, h4, h3}));

    cout << "\n" << (failures == 0 ? "ALL TESTS PASSED" : "SOME TESTS FAILED")
         << " (" << failures << " failures)\n";
}