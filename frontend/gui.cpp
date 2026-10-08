#include "../lib/game.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <array>


const unsigned int WINDOW_SIZE = 640;   // square size will be WINDOW_SIZE / 8
const float square_size = static_cast<float>(WINDOW_SIZE / 8);
const float piece_fraction = 0.9f;      // piece fills 90% of a square
const sf::Color light_colour(240, 217, 181);
const sf::Color dark_colour(181, 136, 99);


// Screen row i (0 = top = rank 8) and column j (0 = left = file a).
// a8 is a light square, so an even (i + j) is light.
Colour colour(int i, int j)
{
    if ((i + j) % 2 == 0)
        return White;

    return Black;
}


sf::Vector2f square_to_pixel(Square square)
{
    float x_pos = fileOf(square) * square_size;
    float y_pos = (7 - rankOf(square)) * square_size;
    return {x_pos, y_pos};
}

Square pixel_to_square(sf::Vector2i pixel)
{
    if (pixel.x < 0 || pixel.y < 0 ||
        pixel.x >= static_cast<int>(WINDOW_SIZE) ||
        pixel.y >= static_cast<int>(WINDOW_SIZE))
        return SquareCount;   // off the board

    int file = static_cast<int>(pixel.x / square_size);
    int rank = 7 - static_cast<int>(pixel.y / square_size);
    return makeSquare(rank, file);
}


int main()
{
    sf::RenderWindow window(sf::VideoMode({WINDOW_SIZE, WINDOW_SIZE}), "Chess");
    window.setFramerateLimit(60);

    sf::RectangleShape square({square_size, square_size});

    // Legal-move marker: a circle whose ORIGIN is its centre, so setPosition
    // places the centre of the circle rather than its top-left corner.
    const float marker_radius = square_size / 6.0f;
    sf::CircleShape marker(marker_radius);
    marker.setOrigin({marker_radius, marker_radius});

    initialize_attack_tables();
    Game game;
    game.new_game();   // swap in load_fen(...) when you want a specific test position
    std::cout << "Legal moves: " << game.legal_moves().count << "\n";

    // ---- Load the 12 piece textures once, before the loop ----
    sf::Texture textures[ColourCount][Position::PieceCount];
    const std::string piece_letters = "PNBRQK";   // same order as PieceType

    for (int c = 0; c < ColourCount; c++)
    {
        for (int p = 0; p < Position::PieceCount; p++)
        {
            std::string path = "assets/";
            path += (c == White ? 'w' : 'b');
            path += piece_letters[p];
            path += ".png";

            if (!textures[c][p].loadFromFile(path))
            {
                std::cout << "Failed to load " << path << "\n";
                return 1;
            }
        }
    }

    // ---- Interaction state (lives across frames, so it sits outside the loop) ----
    Square   selected = SquareCount;   // SquareCount = nothing selected
    uint64_t targets  = 0;             // destination squares of the selected piece
    Square   last_hovered_square = SquareCount;

    while (window.isOpen())
    {
        // ================= EVENTS =================
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto* click = event->getIf<sf::Event::MouseButtonPressed>())
            {
                if (click->button == sf::Mouse::Button::Left)
                {
                    Square clicked = pixel_to_square(click->position);

                    // `continue` moves on to the next event; `break` would abandon
                    // the whole event loop for this frame.
                    if (clicked == SquareCount)
                        continue;

                    if (selected == SquareCount)
                    {
                        // ---- Nothing selected yet ----
                        if (game.isSquareonSidetoMove(clicked))
                        {
                            selected = clicked;
                            targets  = game.legal_for_square(selected);
                        }
                    }
                    else
                    {
                        // ---- Something is selected: this click is a destination ----
                        // A pawn reaching the last rank needs a promotion piece
                        // (auto-queen for now; milestone 17 adds the popup).
                        const bool is_pawn = game.piece_at(selected).type == Position::Pawn;
                        const bool last_rank = (rankOf(clicked) == 0 || rankOf(clicked) == 7);

                        bool move_happened = false;
                        if (is_pawn && last_rank)
                            move_happened = game.try_move(selected, clicked, Position::Queen);
                        else
                            move_happened = game.try_move(selected, clicked);

                        if (move_happened)
                        {
                            selected = SquareCount;
                            targets  = 0;
                        }
                        else if (clicked == selected)
                        {
                            // clicked the same square again: deselect
                            selected = SquareCount;
                            targets  = 0;
                        }
                        else if (game.isSquareonSidetoMove(clicked))
                        {
                            // clicked another of their own pieces: switch selection
                            selected = clicked;
                            targets  = game.legal_for_square(selected);
                        }
                        else
                        {
                            // clicked somewhere pointless: deselect
                            selected = SquareCount;
                            targets  = 0;
                        }
                    }
                }
            }
        }

        // ================= DRAW =================
        window.clear(sf::Color(40, 40, 40));

        // ---- 1. Board squares (drawn first, so everything sits on top) ----
        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                const bool is_light = (colour(i, j) == White);
                square.setFillColor(is_light ? light_colour : dark_colour);
                square.setPosition({j * square_size, i * square_size});
                window.draw(square);
            }
        }

        // ---- 2. Selection highlight ----
        if (selected != SquareCount)
        {
            square.setFillColor(sf::Color(255, 255, 0, 120));   // translucent yellow
            square.setPosition(square_to_pixel(selected));
            window.draw(square);
        }

        // ---- 3. Legal-move markers ----
        for (int s = 0; s < 64; s++)
        {
            const Square sq = static_cast<Square>(s);
            if (!testSquare(targets, sq))
                continue;

            // centre of the square = its top-left corner + half a square each way
            const sf::Vector2f corner = square_to_pixel(sq);
            marker.setPosition({corner.x + square_size / 2.0f, corner.y + square_size / 2.0f});

            if (game.piece_at(sq).type == Position::PieceCount)
            {
                // quiet move: solid dot
                marker.setFillColor(sf::Color(0, 0, 0, 70));
                marker.setOutlineThickness(0.0f);
            }
            else
            {
                // capture: hollow ring
                marker.setFillColor(sf::Color::Transparent);
                marker.setOutlineColor(sf::Color(200, 0, 0, 160));
                marker.setOutlineThickness(4.0f);
            }
            window.draw(marker);
        }

        // ---- 4. Pieces ----
        for (int s = 0; s < 64; s++)
        {
            const Square sq = static_cast<Square>(s);
            const Position::PieceOnSquare piece = game.piece_at(sq);

            if (piece.type == Position::PieceCount)
                continue;   // empty square: nothing to draw

            const sf::Texture& texture = textures[piece.colour][piece.type];
            sf::Sprite sprite(texture);

            const float wanted_size = piece_fraction * square_size;
            const float scale = wanted_size / static_cast<float>(texture.getSize().x);
            sprite.setScale({scale, scale});

            const float margin = (square_size - wanted_size) / 2.0f;
            const sf::Vector2f corner = square_to_pixel(sq);
            sprite.setPosition({corner.x + margin, corner.y + margin});

            window.draw(sprite);
        }

        // ---- Hover readout (debug) ----
        Square current_mouse_square = pixel_to_square(sf::Mouse::getPosition(window));
        if (current_mouse_square != last_hovered_square && current_mouse_square != SquareCount)
        {
            std::cout << static_cast<char>('a' + fileOf(current_mouse_square))
                      << rankOf(current_mouse_square) + 1 << "\n";
        }
        last_hovered_square = current_mouse_square;

        window.display();
    }
    return 0;
}