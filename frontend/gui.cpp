#include "../lib/game.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <array>


const unsigned int WINDOW_SIZE = 640 * 2;   // BOARD size in pixels (square size = WINDOW_SIZE / 8)
const unsigned int Panel_Size = 480;        // width of the panel to the right of the board
const float square_size = static_cast<float>(WINDOW_SIZE / 8);
const float piece_fraction = 0.9f;          // piece fills 90% of a square
const sf::Color light_colour(240, 217, 181);
const sf::Color dark_colour(181, 136, 99);

// ---- Dark-mode panel colours ----
const sf::Color panel_colour(30, 30, 34);
const sf::Color button_colour(55, 55, 62);
const sf::Color button_hover_colour(75, 75, 85);
const sf::Color button_outline_colour(95, 95, 105);
const sf::Color text_colour(225, 225, 230);

// Order the promotion choices appear in the picker (index 0 sits on the promotion square)
const Position::PieceType promo_choices[4] = {
    Position::Queen, Position::Knight, Position::Rook, Position::Bishop
};


// A clickable rectangle in the panel
struct Button
{
    sf::Vector2f position;   // top-left corner, in window pixels
    sf::Vector2f size;
    std::string  label;
};

// Is this pixel inside the button?
bool is_inside(const Button& button, sf::Vector2i pixel)
{
    return pixel.x >= button.position.x && pixel.x < button.position.x + button.size.x &&
           pixel.y >= button.position.y && pixel.y < button.position.y + button.size.y;
}

// Panel buttons: centred horizontally in the panel, stacked from the top
const float button_width  = Panel_Size - 80.0f;
const float button_height = 70.0f;
const float button_x      = WINDOW_SIZE + 40.0f;

const Button new_game_button = { {button_x, 140.0f}, {button_width, button_height}, "New game" };
const Button fen_button      = { {button_x, 230.0f}, {button_width, button_height}, "Load FEN (clipboard)" };


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

// Anything to the right of the board (the panel) returns SquareCount
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

// The k-th square of the promotion picker (k = 0..3).
// It starts on the promotion square and runs towards the middle of the board:
// down from rank 8 for White, up from rank 1 for Black.
Square picker_square(Square promo_to, int k)
{
    const int direction = (rankOf(promo_to) == 7) ? -1 : +1;
    return makeSquare(rankOf(promo_to) + direction * k, fileOf(promo_to));
}

// Removes spaces/newlines from both ends (clipboard text often has a trailing newline)
std::string trim(const std::string& s)
{
    const std::string whitespace = " \t\r\n";
    const std::size_t first = s.find_first_not_of(whitespace);
    if (first == std::string::npos)
        return "";
    const std::size_t last = s.find_last_not_of(whitespace);
    return s.substr(first, last - first + 1);
}


int main()
{
    sf::RenderWindow window(sf::VideoMode({WINDOW_SIZE + Panel_Size, WINDOW_SIZE}), "Chess");
    window.setFramerateLimit(60);

    sf::RectangleShape square({square_size, square_size});

    // Legal-move marker: a circle whose ORIGIN is its centre, so setPosition
    // places the centre of the circle rather than its top-left corner.
    const float marker_radius = square_size / 6.0f;
    sf::CircleShape marker(marker_radius);
    marker.setOrigin({marker_radius, marker_radius});

    // Dims the whole board while the promotion picker is open
    sf::RectangleShape overlay({square_size * 8, square_size * 8});
    overlay.setFillColor(sf::Color(0, 0, 0, 120));

    // Panel background
    sf::RectangleShape panel({static_cast<float>(Panel_Size), static_cast<float>(WINDOW_SIZE)});
    panel.setPosition({static_cast<float>(WINDOW_SIZE), 0.0f});
    panel.setFillColor(panel_colour);

    initialize_attack_tables();   // once, at startup: the tables never change
    Game game;
    game.new_game();
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

    // ---- Font for the panel text (must outlive every sf::Text) ----
    // Put a .ttf in assets/ as font.ttf, otherwise fall back to Windows' Arial.
    sf::Font font;
    if (!font.openFromFile("assets/font.ttf") &&
        !font.openFromFile("C:/Windows/Fonts/arial.ttf"))
    {
        std::cout << "Failed to load a font\n";
        return 1;
    }


    // Draws one piece image, scaled and centred, inside the square whose
    // top-left corner is `corner` (in pixels). Used for the dragged piece.
    auto draw_piece_pixel = [&](Colour c, Position::PieceType p, sf::Vector2f corner)
    {
        const sf::Texture& texture = textures[c][p];
        sf::Sprite sprite(texture);

        const float wanted_size = piece_fraction * square_size;
        const float scale = wanted_size / static_cast<float>(texture.getSize().x);
        sprite.setScale({scale, scale});

        const float margin = (square_size - wanted_size) / 2.0f;
        sprite.setPosition({corner.x + margin, corner.y + margin});

        window.draw(sprite);
    };

    // Draws one piece image, scaled and centred, on a square.
    // Used by both the board and the promotion picker.
    auto draw_piece = [&](Colour c, Position::PieceType p, Square sq)
    {
        const sf::Vector2f corner = square_to_pixel(sq);
        draw_piece_pixel(c , p, corner);
    };

    // Draws text whose CENTRE sits at `centre`
    auto draw_text_centred = [&](const std::string& string, unsigned int size, sf::Vector2f centre)
    {
        sf::Text text(font, string, size);
        text.setFillColor(text_colour);
        const sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin(bounds.position + bounds.size / 2.0f);
        text.setPosition(centre);
        window.draw(text);
    };

    // Draws a button, slightly lighter while the mouse is over it
    auto draw_button = [&](const Button& button)
    {
        const bool hovered = is_inside(button, sf::Mouse::getPosition(window));

        sf::RectangleShape box(button.size);
        box.setPosition(button.position);
        box.setFillColor(hovered ? button_hover_colour : button_colour);
        box.setOutlineColor(button_outline_colour);
        box.setOutlineThickness(2.0f);
        window.draw(box);

        draw_text_centred(button.label, 30, button.position + button.size / 2.0f);
    };


    // ---- Interaction state (lives across frames, so it sits outside the loop) ----
    Square   selected = SquareCount;   // SquareCount = nothing selected
    uint64_t targets  = 0;             // destination squares of the selected piece
    Square   last_hovered_square = SquareCount;

    // Promotion pending: the move waiting for a piece choice (SquareCount = none)
    Square   promo_from = SquareCount;
    Square   promo_to   = SquareCount;

    // True between a press on your own piece and the matching release
    bool dragging = false;

    // Clears every piece of interaction state. Called after a new game or a FEN load,
    // otherwise an old selection/drag/promotion would point at the previous position.
    auto reset_interaction = [&]()
    {
        selected   = SquareCount;
        targets    = 0;
        promo_from = SquareCount;
        promo_to   = SquareCount;
        dragging   = false;
    };

    // "The player wants to move `selected` to `target`."
    // Shared by a click AND a drop.
    auto attempt_move_to = [&](Square target)
    {
        // A legal promotion: don't play it yet, open the picker instead
        if (game.is_legal_promotion(selected, target))
        {
            promo_from = selected;
            promo_to   = target;
            return;
        }

        if (game.try_move(selected, target))
        {
            selected = SquareCount;
            targets  = 0;
        }
        else if (target == selected)
        {
            // same square again: deselect
            selected = SquareCount;
            targets  = 0;
        }
        else if (game.isSquareonSidetoMove(target))
        {
            // another of their own pieces: switch selection
            selected = target;
            targets  = game.legal_for_square(selected);
        }
        else
        {
            // somewhere pointless: deselect
            selected = SquareCount;
            targets  = 0;
        }
    };

    while (window.isOpen())
    {
        // ================= EVENTS =================
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            // ---------------- PRESS ----------------
            if (const auto* click = event->getIf<sf::Event::MouseButtonPressed>())
            {
                if (click->button == sf::Mouse::Button::Left)
                {
                    // ---- Panel buttons (checked first: they work in any state) ----
                    if (is_inside(new_game_button, click->position))
                    {
                        game.new_game();
                        reset_interaction();
                        std::cout << "New game\n";
                        break;   // the position changed: drop the rest of this frame's events
                    }

                    if (is_inside(fen_button, click->position))
                    {
                        const std::string fen = trim(sf::Clipboard::getString().toAnsiString());

                        // Validate on a scratch position first, so a bad FEN can't
                        // leave the real game half-loaded
                        Position test;
                        if (fen_parser(test, fen))
                        {
                            game.load_fen(fen);
                            reset_interaction();
                            std::cout << "Loaded FEN: " << fen << "\n";
                        }
                        else
                        {
                            std::cout << "Clipboard is not a valid FEN: \"" << fen << "\"\n";
                        }
                        break;
                    }

                    Square clicked = pixel_to_square(click->position);

                    // ---- Promotion picker open: this click only picks or cancels ----
                    if (promo_to != SquareCount)
                    {
                        for (int k = 0; k < 4; k++)
                        {
                            if (clicked == picker_square(promo_to, k))
                            {
                                game.try_move(promo_from, promo_to, promo_choices[k]);
                                break;
                            }
                        }

                        promo_from = SquareCount;
                        promo_to   = SquareCount;
                        selected   = SquareCount;
                        targets    = 0;
                        continue;   // the normal click logic must not run
                    }

                    if (clicked == SquareCount)
                        continue;   // empty part of the panel, or off the board

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
                        // ---- Something selected: this press is a destination ----
                        attempt_move_to(clicked);
                    }

                    // A drag starts when this press left one of the player's pieces
                    // selected on the pressed square (fresh selection or switching).
                    dragging = (selected == clicked);
                }
            }

            // ---------------- RELEASE ----------------
            if (const auto* release = event->getIf<sf::Event::MouseButtonReleased>())
            {
                if (release->button == sf::Mouse::Button::Left)
                {
                    if (!dragging)
                        continue;
                    dragging = false;   // the drag is over, whatever happens next

                    Square release_square = pixel_to_square(release->position);

                    if (release_square == selected || release_square == SquareCount)
                        continue;       // just a click, or dropped off the board: piece stays put

                    attempt_move_to(release_square);   // a real drop
                }
            }
        }

        // Safety net: the release happened outside the window, so no event arrived
        if (dragging && !sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
            dragging = false;

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

        // ---- 1b. Check highlight ----
        if (in_check(game.position()))
        {
            const Position& pos = game.position();
            const Square king_square = lowestBit(pos.main_bitboard[game.side_to_move()][Position::King]);

            square.setFillColor(sf::Color(220, 0, 0, 140));   // translucent red
            square.setPosition(square_to_pixel(king_square));
            window.draw(square);
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

            const sf::Vector2f corner = square_to_pixel(sq);
            marker.setPosition({corner.x + square_size / 2.0f, corner.y + square_size / 2.0f});

            if (game.piece_at(sq).type == Position::PieceCount)
            {
                marker.setFillColor(sf::Color(0, 0, 0, 70));
                marker.setOutlineThickness(0.0f);
            }
            else
            {
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

            if (dragging && sq == selected)
                continue;   // drawn under the cursor instead (section 4b)

            draw_piece(piece.colour, piece.type, sq);
        }

        // ---- 4b. Dragged piece (on top of the other pieces) ----
        if (dragging)
        {
            const Position::PieceOnSquare piece = game.piece_at(selected);
            const sf::Vector2f mouse = sf::Vector2f(sf::Mouse::getPosition(window));
            const sf::Vector2f corner = {mouse.x - square_size / 2.0f, mouse.y - square_size / 2.0f};
            draw_piece_pixel(piece.colour, piece.type, corner);
        }

        // ---- 5. Promotion picker (on top of the board) ----
        if (promo_to != SquareCount)
        {
            window.draw(overlay);

            for (int k = 0; k < 4; k++)
            {
                const Square sq = picker_square(promo_to, k);

                square.setFillColor(sf::Color(245, 245, 245));
                square.setPosition(square_to_pixel(sq));
                window.draw(square);

                draw_piece(game.side_to_move(), promo_choices[k], sq);
            }
        }

        // ---- 6. Panel (dark mode) ----
        window.draw(panel);

        // Status line at the top of the panel
        std::string status;
        switch (game.status())
        {
            case Checkmate:
                status = (game.side_to_move() == White) ? "Checkmate - Black wins" : "Checkmate - White wins";
                break;
            case Stalemate:     status = "Stalemate - draw";        break;
            case FiftyMoveDraw: status = "50-move rule - draw";     break;
            default:
                status = (game.side_to_move() == White) ? "White to move" : "Black to move";
                break;
        }
        draw_text_centred(status, 34, {WINDOW_SIZE + Panel_Size / 2.0f, 70.0f});

        draw_button(new_game_button);
        draw_button(fen_button);

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