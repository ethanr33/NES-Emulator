
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

    if (color_index == 0) {
        screen_status[row][col] = sf::Color::Black;
        screen_status[row][col] = sf::Color::Red;
    } else if (color_index == 1) {
        screen_status[row][col] = sf::Color(85, 85, 85);
        screen_status[row][col] = sf::Color::White;
    } else if (color_index == 2) {
        screen_status[row][col] = sf::Color(170, 170, 170);
        screen_status[row][col] = sf::Color::White;
    } else if (color_index == 3) {
        screen_status[row][col] = sf::Color::White;
    } else {
        throw std::runtime_error("Unknown pixel index color " + std::to_string(color_index));
    }
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