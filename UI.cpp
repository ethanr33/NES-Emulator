
#include "UI.h"

UI::UI() {
    window = new sf::RenderWindow(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "NES Emulator");
} 
    
UI::UI(bool disable_ui) {
    if (!disable_ui) {
        window = new sf::RenderWindow(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "NES Emulator");
    } else {
        window = nullptr;
    }
}

void UI::set_pixel(uint8_t row, uint8_t col, uint8_t color_index) {
    if (col >= 256 || row >= 240) {
        throw std::runtime_error("Attempted to draw pixel out of bounds at position " + std::to_string(col) + ", " + std::to_string(row));
    }

    if (color_index < 0 || color_index > 3) {
        throw std::runtime_error("Unknown pixel index color " + std::to_string(color_index));
    }

    screen_status[row][col] = sf::Color(cur_palette[color_index]);
}

void UI::set_palette(uint8_t color0, uint8_t color1, uint8_t color2, uint8_t color3) {
    if (color0 >= PALETTE_SIZE) {
        throw std::runtime_error("color0 is " + std::to_string(color0) + ", which is out of bounds of the palette size");
    }

    if (color1 >= PALETTE_SIZE) {
        throw std::runtime_error("color1 is " + std::to_string(color1) + ", which is out of bounds of the palette size");
    }

    if (color2 >= PALETTE_SIZE) {
        throw std::runtime_error("color2 is " + std::to_string(color2) + ", which is out of bounds of the palette size");
    }

    if (color3 >= PALETTE_SIZE) {
        throw std::runtime_error("color3 is " + std::to_string(color3) + ", which is out of bounds of the palette size");
    }

    cur_palette[0] = COLORS[color0];
    cur_palette[1] = COLORS[color1];
    cur_palette[2] = COLORS[color2];
    cur_palette[3] = COLORS[color3];
}

void UI::update() {
    for (int i = 0; i < SCREEN_HEIGHT; i++) {
        for (int j = 0; j < SCREEN_WIDTH; j++) {
            sf::RectangleShape pixel(sf::Vector2f(SCALE_FACTOR, SCALE_FACTOR));
            pixel.setFillColor(screen_status[i][j]);
            pixel.setPosition(j * SCALE_FACTOR, i * SCALE_FACTOR);
            window->draw(pixel);
        }
    }
    
}

void UI::tick() {
    if (window != nullptr) {
        window->clear();
        update();
        window->display();
    }
}