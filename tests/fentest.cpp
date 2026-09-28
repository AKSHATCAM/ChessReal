#include <string>
#include <sstream>
#include <cctype>
#include "../lib/position.h"
#include "../lib/bitboard_utilities.h"


int main()
{
    Position pos;

    std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    std::string fen1 = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    std::string fen2 = "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3";
    std::string fen3 = "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1";
    std::string fen4 = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
    std::string fen5 = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";


    if (!fen_parser(pos, fen))
        std::cout << "Invalid FEN\n";

    printBoard(pos);
    if (!fen_parser(pos, fen1))
        std::cout << "Invalid FEN\n";

    printBoard(pos);
    if (!fen_parser(pos, fen2))
        std::cout << "Invalid FEN\n";

    printBoard(pos);
    if (!fen_parser(pos, fen3))
        std::cout << "Invalid FEN\n";

    printBoard(pos);
    if (!fen_parser(pos, fen4))
        std::cout << "Invalid FEN\n";

    printBoard(pos);
    if (!fen_parser(pos, fen5))
        std::cout << "Invalid FEN\n";

    printBoard(pos);
}


