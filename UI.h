
#include <cstdint>
#include <SFML/Graphics.hpp>

struct UI {

    static const int SCALE_FACTOR = 3;

    static const int SCREEN_WIDTH = 256;
    static const int SCREEN_HEIGHT = 240;

    static const int WINDOW_WIDTH = SCREEN_WIDTH * SCALE_FACTOR;
    static const int WINDOW_HEIGHT = SCREEN_HEIGHT * SCALE_FACTOR;

    sf::Color screen_status[SCREEN_HEIGHT][SCREEN_WIDTH];

    sf::RenderWindow* window = nullptr;

    bool ui_disabled = false;

    UI();
    UI(bool);

    void set_pixel(uint8_t, uint8_t, uint8_t);

    void update();
    void tick();

};