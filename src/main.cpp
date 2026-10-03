#include <iostream>
#include <string>
#include "../lib/game.h"

using namespace std;

// Converts "e2" style text to a square. Returns false if invalid.
bool parseSquare(char file, char rank, Square& sq)
{
    if (file < 'a' || file > 'h' || rank < '1' || rank > '8')
        return false;
    sq = makeSquare(rank - '1', file - 'a');
    return true;
}

int main()
{
    initialize_attack_tables();

    Game game;
    game.new_game();

    while (true)
    {
        printBoard(game.position());

        // ---- Game over? ----
        GameStatus status = game.status();
        if (status == Checkmate)
        {
            cout << "Checkmate! " << (game.side_to_move() == White ? "Black" : "White") << " wins.\n";
            break;
        }
        if (status == Stalemate)     { cout << "Stalemate. Draw.\n"; break; }
        if (status == FiftyMoveDraw) { cout << "Draw by the 50-move rule.\n"; break; }

        if (in_check(game.position()))
            cout << "Check!\n";

        // ---- Read a move, re-prompting until it's legal ----
        while (true)
        {
            cout << (game.side_to_move() == White ? "White" : "Black")
                 << " to move (e.g. e2e4, e7e8q, or quit): ";

            string input;
            cin >> input;

            if (input == "quit")
                return 0;

            Square from, to;
            Position::PieceType promotion = Position::PieceCount;

            bool valid = (input.size() == 4 || input.size() == 5)
                      && parseSquare(input[0], input[1], from)
                      && parseSquare(input[2], input[3], to);

            if (valid && input.size() == 5)
            {
                switch (input[4])
                {
                    case 'q': promotion = Position::Queen;  break;
                    case 'r': promotion = Position::Rook;   break;
                    case 'b': promotion = Position::Bishop; break;
                    case 'n': promotion = Position::Knight; break;
                    default:  valid = false;
                }
            }

            if (valid && game.try_move(from, to, promotion))
                break;   // legal move made

            cout << "Illegal move, try again.\n";
        }
    }

    return 0;
}