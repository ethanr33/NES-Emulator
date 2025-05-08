
#include "UI.h"

void PixelMap::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    // apply the entity's transform -- combine it with the one that was passed by the caller
    states.transform *= getTransform(); // getTransform() is defined by sf::Transformable

    // you may also override states.shader or states.blendMode if you want

    // draw the vertex array
    target.draw(vertices, states);
}

void PixelMap::load(const vector<vector<sf::Color>>& screen_status, const int scale_factor) {
    vertices.setPrimitiveType(sf::Triangles);
    vertices.resize(SCREEN_WIDTH * SCREEN_HEIGHT * 6);

    for (int i = 0; i < SCREEN_HEIGHT; i++) {
        for (int j = 0; j < SCREEN_WIDTH; j++) {
            sf::Color pixel_color = screen_status.at(i).at(j);

            // 2 triangles per pixel, so we skip 6 vertices
            sf::Vertex* triangles = &vertices[(i * SCREEN_WIDTH + j) * 6];

            triangles[0].position = sf::Vector2f(j * scale_factor, i * scale_factor);
            triangles[1].position = sf::Vector2f((j + 1) * scale_factor, i * scale_factor);
            triangles[2].position = sf::Vector2f(j * scale_factor, (i + 1) * scale_factor);
            triangles[3].position = sf::Vector2f((j + 1) * scale_factor, i * scale_factor);
            triangles[4].position = sf::Vector2f(j * scale_factor, (i + 1) * scale_factor);
            triangles[5].position = sf::Vector2f((j + 1) * scale_factor, (i + 1) * scale_factor);

            triangles[0].color = pixel_color;
            triangles[1].color = pixel_color;
            triangles[2].color = pixel_color;
            triangles[3].color = pixel_color;
            triangles[4].color = pixel_color;
            triangles[5].color = pixel_color;

        }
    }
}

UI::UI() {
    window = new sf::RenderWindow(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "NES Emulator");
    screen_status = vector<vector<sf::Color>>(SCREEN_HEIGHT, vector<sf::Color>(SCREEN_WIDTH));
} 
    
UI::UI(bool disable_ui) {
    if (!disable_ui) {
        window = new sf::RenderWindow(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "NES Emulator");
    } else {
        window = nullptr;
    }

    screen_status = vector<vector<sf::Color>>(SCREEN_HEIGHT, vector<sf::Color>(SCREEN_WIDTH));
}

void UI::set_pixel(uint16_t row, uint16_t col, uint8_t color_index, bool using_background_palette) {
    if (col >= 256 || row >= 240) {
        throw std::runtime_error("Attempted to draw pixel out of bounds at position " + std::to_string(col) + ", " + std::to_string(row));
    }

    if (color_index > 3) {
        throw std::runtime_error("Unknown pixel index color " + std::to_string(color_index));
    }

    if (using_background_palette) {
        screen_status.at(row).at(col) = sf::Color(cur_background_palette[color_index]);
    } else {
        screen_status.at(row).at(col) = sf::Color(cur_sprite_palette[color_index]);
    }
}

void UI::set_pixel_color(uint16_t row, uint16_t col, uint8_t color_index) {
    if (col >= 256 || row >= 240) {
        throw std::runtime_error("Attempted to draw pixel out of bounds at position " + std::to_string(col) + ", " + std::to_string(row));
    }

    // 
    screen_status.at(row).at(col) = sf::Color(COLORS[color_index]);
}

void UI::set_background_palette(uint8_t color0, uint8_t color1, uint8_t color2, uint8_t color3) {
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

    cur_background_palette[0] = COLORS[color0];
    cur_background_palette[1] = COLORS[color1];
    cur_background_palette[2] = COLORS[color2];
    cur_background_palette[3] = COLORS[color3];
}

void UI::set_sprite_palette(uint8_t color0, uint8_t color1, uint8_t color2, uint8_t color3) {
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

    cur_sprite_palette[0] = COLORS[color0];
    cur_sprite_palette[1] = COLORS[color1];
    cur_sprite_palette[2] = COLORS[color2];
    cur_sprite_palette[3] = COLORS[color3];
}

void UI::update() {
    PixelMap pixels;
    pixels.load(screen_status, SCALE_FACTOR); 
    window->draw(pixels);
}

void UI::tick() {
    if (window != nullptr) {
        window->clear();
        update();
        window->display();
    }
}