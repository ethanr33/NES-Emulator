
#include <iostream>
#include <string>
#include "Bus.h"

int main(int argc, char* argv[])
{

    if (argc != 2) {
        std::cout << "Incorrect usage. ./chr_dump [rom name]" << std::endl;
        return 1;
    }

    Bus nes(true); // Hide default UI

    Cartridge* game = new Cartridge(argv[1]);

    nes.insert_cartridge(game);
    nes.reset();

    // create the window
    sf::RenderWindow window(sf::VideoMode(1024,512), "CHR Dump");

    // run the program as long as the window is open
    while (window.isOpen())
    {
        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while (window.pollEvent(event))
        {
            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::Black);

        for (int row = 0; row < 16; row++) {
            for (int col = 0; col < 16; col++) {
                for (int pixel_row = 0; pixel_row < 8; pixel_row++) {
                    uint8_t plane_0 = nes.cartridge->CHR_ROM[256 * row + 16 * col + pixel_row];
                    uint8_t plane_1 = nes.cartridge->CHR_ROM[256 * row + 16 * col + pixel_row + 8];

                    for (int pixel_col = 0; pixel_col < 8; pixel_col++) {
                        uint8_t low_bit = is_bit_set(pixel_col, plane_0);
                        uint8_t high_bit = is_bit_set(pixel_col, plane_1);

                        int color = low_bit | (high_bit << 1);
                    
                        sf::RectangleShape square(sf::Vector2f(4, 4));

                        if (color == 0) {
                            square.setFillColor(sf::Color::Black);
                        } else if (color == 1) {
                            square.setFillColor(sf::Color(85, 85, 85));
                        } else if (color == 2) {
                            square.setFillColor(sf::Color(170, 170, 170));
                        } else if (color == 3) {
                            square.setFillColor(sf::Color::White);
                        } else {
                            //throw std::runtime_error("Unknown color");
                        }

                        square.setPosition(4 * (8 * col + (7 - pixel_col)), 4 * (8 * row + pixel_row));
                        window.draw(square);
                    }

                }
            }
        }

        for (int row = 0; row < 16; row++) {
            for (int col = 0; col < 16; col++) {
                for (int pixel_row = 0; pixel_row < 8; pixel_row++) {
                    uint8_t plane_0 = nes.cartridge->CHR_ROM[0x1000 + 256 * row + 16 * col + pixel_row];
                    uint8_t plane_1 = nes.cartridge->CHR_ROM[0x1000 + 256 * row + 16 * col + pixel_row + 8];

                    for (int pixel_col = 7; pixel_col >= 0; pixel_col--) {
                        uint8_t low_bit = is_bit_set(pixel_col, plane_0);
                        uint8_t high_bit = is_bit_set(pixel_col, plane_1);

                        int color = low_bit | (high_bit << 1);
                    
                        sf::RectangleShape square(sf::Vector2f(4, 4));

                        if (color == 0) {
                            square.setFillColor(sf::Color::Black);
                        } else if (color == 1) {
                            square.setFillColor(sf::Color(85, 85, 85));
                        } else if (color == 2) {
                            square.setFillColor(sf::Color(170, 170, 170));
                        } else if (color == 3) {
                            square.setFillColor(sf::Color::White);
                        } else {
                            //throw std::runtime_error("Unknown color");
                        }

                        square.setPosition(512 + 4 * (8 * col + (7 - pixel_col)), 4 * (8 * row + pixel_row));
                        window.draw(square);
                    }

                }
            }
        }

        window.display();
    }

    return 0;
}
