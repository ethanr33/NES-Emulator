
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

    static const int PALETTE_SIZE = 0x40;

    const uint32_t COLORS[PALETTE_SIZE] = {
        0x626262FF, 0x002C7CFF, 0x11159CFF, 0x36039CFF, 0x55007CFF, 0x670044FF, 0x670703FF, 0x551C00FF, 0x363200FF, 0x114400FF, 0x004E00FF, 0x004C03FF, 0x004044FF, 0x000000FF, 0x000000FF, 0x000000FF,
        0xABABABFF, 0x1260CEFF, 0x3D42FAFF, 0x6E29FAFF, 0x991CCEFF, 0xB11E81FF, 0xB12F29FF, 0x994A00FF, 0x6E6900FF, 0x3D8200FF, 0x128F00FF, 0x008D29FF, 0x007C81FF, 0x000000FF, 0x000000FF, 0x000000FF,
        0xFFFFFFFF, 0x60B2FFFF, 0x8D92FFFF, 0xC078FFFF, 0xEC6AFFFF, 0xFF6DD4FF, 0xFF7F79FF, 0xEC9B2AFF, 0xC0BA00FF, 0x8DD400FF, 0x60E22AFF, 0x47E079FF, 0x47CED4FF, 0x4E4E4EFF, 0X000000FF, 0x000000FF,
        0XFFFFFFFF, 0XBFE0FFFF, 0XD1D3FFFF, 0XE6C9FFFF, 0XF7C3FFFF, 0XFFC4EEFF, 0XFFCBC9FF, 0XF7D7A9FF, 0XE6E397FF, 0XD1EE97FF, 0XBFF3A9FF, 0XB5F2C9FF, 0xB5EBEEFF, 0xB8B8B8FF, 0x000000FF, 0x000000FF 
    };

    uint32_t cur_palette[4];

    UI();
    UI(bool);

    sf::Color hex_to_sfcpo;

    void set_pixel(uint8_t, uint8_t, uint8_t);
    void set_palette(uint8_t, uint8_t, uint8_t, uint8_t);

    void update();
    void tick();

};