#include <iostream>
#include <string>
#include <initializer_list>
#include "../lib/bitboard_utilities.h"
#include "../lib/attacks.h"

using namespace std;

// Names in the same order as the ray_directions enum
const char* direction_names[direction_count] = {
    "north_west", "north", "north_east", "east",
    "south_east", "south", "south_west", "west"
};

string squareName(Square sq)
{
    string name;
    name += static_cast<char>('a' + fileOf(sq));
    name += static_cast<char>('1' + rankOf(sq));
    return name;
}

// Builds a bitboard from a list of squares, e.g. board({a2, a3, a4})
uint64_t board(initializer_list<Square> squares)
{
    uint64_t b = 0;
    for (Square sq : squares)
        b = setSquare(b, sq);
    return b;
}

int failures = 0;

// Compares the blocked ray against the exact expected bitboard
void check(const string& description, ray_directions direction, Square square,
           uint64_t occupied, uint64_t expected)
{
    uint64_t actual = calculate_raywithblocker(direction, square, occupied);
    bool pass = (actual == expected);
    if (!pass) failures++;

    cout << (pass ? "PASS  " : "FAIL  ") << direction_names[direction]
         << " from " << squareName(square) << ": " << description << "\n";

    if (!pass)
    {
        cout << "  Occupied:";
        printBitboard(occupied);
        cout << "  Expected:";
        printBitboard(expected);
        cout << "  Got:";
        printBitboard(actual);
    }
}

// Prints a case in full, for looking at by eye
void show(const string& description, ray_directions direction, Square square, uint64_t occupied)
{
    cout << "\n=== " << direction_names[direction] << " from " << squareName(square)
         << ": " << description << " ===\n";
    cout << "Occupied:";
    printBitboard(occupied);
    cout << "Blocked ray:";
    printBitboard(calculate_raywithblocker(direction, square, occupied));
}

int main()
{
    initialize_attack_tables();

    cout << "\n--- Single blocker, positive directions ---\n";
    check("blocker on a5", north, a1,
          board({a5}), board({a2, a3, a4, a5}));
    check("blocker on b1 (adjacent)", east, a1,
          board({b1}), board({b1}));
    check("blockers on e3 and g5 (nearest wins)", north_east, c1,
          board({e3, g5}), board({d2, e3}));
    check("blocker on e4", north_west, h1,
          board({e4}), board({g2, f3, e4}));

    cout << "\n--- Single blocker, negative directions ---\n";
    check("blocker on h4", south, h8,
          board({h4}), board({h7, h6, h5, h4}));
    check("blocker on c3", south_west, f6,
          board({c3}), board({e5, d4, c3}));
    check("blocker on d5", south_east, a8,
          board({d5}), board({b7, c6, d5}));
    check("blocker on e1", west, h1,
          board({e1}), board({g1, f1, e1}));

    cout << "\n--- Multiple blockers, negative direction (nearest = highest bit) ---\n";
    check("blockers on d6 and d3", south, d8,
          board({d6, d3}), board({d7, d6}));
    check("blockers on c1 and f1", west, h1,
          board({c1, f1}), board({g1, f1}));

    cout << "\n--- No blockers ---\n";
    check("empty board", west, h1,
          0, board({g1, f1, e1, d1, c1, b1, a1}));
    check("pieces off the ray don't block", north, a1,
          board({b2, c5, h8}), board({a2, a3, a4, a5, a6, a7, a8}));

    cout << "\n--- Edge cases ---\n";
    check("blocker on the last square of the ray", east, a1,
          board({h1}), board({b1, c1, d1, e1, f1, g1, h1}));
    check("own square is occupied (it must not block itself)", north, d4,
          board({d4, d7}), board({d5, d6, d7}));
    check("ray already empty at the edge", east, h4,
          board({a5}), 0);

    // ---- Visual checks for a few interesting cases ----
    show("blockers on e3 and g5", north_east, c1, board({e3, g5}));
    show("blockers on d6 and d3", south, d8, board({d6, d3}));
    show("own square d4 plus blocker d7", north, d4, board({d4, d7}));

    cout << "\n" << (failures == 0 ? "ALL TESTS PASSED" : "SOME TESTS FAILED")
         << " (" << failures << " failures)\n";
}