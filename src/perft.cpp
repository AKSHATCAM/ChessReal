#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <chrono>
#include "../lib/bitboard_utilities.h"
#include "../lib/attacks.h"
#include "../lib/position.h"
#include "../lib/movegen.h"
#include "../lib/makemove.h"
#include "../lib/perft.h"

using namespace std;

int failures = 0;

// Column widths
const int W_DEPTH = 7, W_EXPECT = 12, W_GOT = 12, W_TIME = 12, W_STATUS = 8;

void printLine()
{
    cout << "+" << string(W_DEPTH + 2, '-')
         << "+" << string(W_EXPECT + 2, '-')
         << "+" << string(W_GOT + 2, '-')
         << "+" << string(W_TIME + 2, '-')
         << "+" << string(W_STATUS + 2, '-') << "+\n";
}

// Prints the position, then a table with one row per depth.
// expected[0] is the count for depth 1, expected[1] for depth 2, and so on.
void runPosition(const string& name, const string& fen, const vector<uint64_t>& expected)
{
    Position pos;
    fen_parser(pos, fen);

    cout << "\n\n==================================================\n";
    cout << name << "\n";
    cout << "==================================================";
    printBoard(pos);

    // Table header
    printLine();
    cout << "| " << right << setw(W_DEPTH)  << "Depth"
         << " | " << setw(W_EXPECT) << "Expected"
         << " | " << setw(W_GOT)    << "My code"
         << " | " << setw(W_TIME)   << "Time (ms)"
         << " | " << left  << setw(W_STATUS) << "Status"
         << " |\n";
    printLine();

    // One row per depth
    for (size_t i = 0; i < expected.size(); ++i)
    {
        int depth = static_cast<int>(i) + 1;

        auto start = chrono::steady_clock::now();
        uint64_t result = perft(pos, depth);
        auto end = chrono::steady_clock::now();

        double ms = chrono::duration<double, milli>(end - start).count();

        bool pass = (result == expected[i]);
        if (!pass) failures++;

        cout << "| " << right << setw(W_DEPTH)  << depth
             << " | " << setw(W_EXPECT) << expected[i]
             << " | " << setw(W_GOT)    << result
             << " | " << setw(W_TIME)   << fixed << setprecision(2) << ms
             << " | " << left  << setw(W_STATUS) << (pass ? "PASS" : "FAIL")
             << " |\n";
    }

    printLine();
}

int main()
{
    initialize_attack_tables();

    runPosition("Starting position",
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            { 20, 400, 8902, 197281, 4865609, 119060324 });
    
    runPosition("Position 5",
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
            { 44, 1486, 62379, 2103487, 89941194 });

    cout << "\n" << (failures == 0 ? "ALL PERFT TESTS PASSED" : "SOME PERFT TESTS FAILED")
         << " (" << failures << " failures)\n";

    // ---- Divide, for debugging ----
    // If a row fails, uncomment a line below and compare with Stockfish:
    //     position fen <fen>
    //     go perft <depth>
    //
    // Position dbg;
    // fen_parser(dbg, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    // divide(dbg, 3);
}

