
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

    uint32_t cur_palette[4];

    const uint32_t COLORS[0x40] = {
        0x62626200, 0x002C7C00, 0x11159C00, 0x36039C00, 0x55007C00, 0x67004400, 0x67070300, 0x551C0000, 0x36320000, 0x11440000, 0x004E0000, 0x004C0300, 0x00404400, 0x00000000, 0x00000000, 0x00000000,
        0xABABAB00, 0x1260CE00, 0x3D42FA00, 0x6E29FA00, 0x991CCE00, 0xB11E8100, 0xB12F2900, 0x994A0000, 0x6E690000, 0x3D820000, 0x128F0000, 0x008D2900, 0x007C8100, 0x00000000, 0x00000000, 0x00000000,
        0xFFFFFF00, 0x60B2FF00, 0x8D92FF00, 0xC078FF00, 0xEC6AFF00, 0xFF6DD400, 0xFF7F7900, 0xEC9B2A00, 0xC0BA0000, 0x8DD40000, 0x60E22A00, 0x47E07900, 0x47CED400, 0x4E4E4E00, 0X00000000, 0x00000000,
        0XFFFFFF00, 0XBFE0FF00, 0XD1D3FF00, 0XE6C9FF00, 0XF7C3FF00, 0XFFC4EE00, 0XFFCBC900, 0XF7D7A900, 0XE6E39700, 0XD1EE9700, 0XBFF3A900, 0XB5F2C900, 0xB5EBEE00, 0xB8B8B800, 0x00000000, 0x00000000 
    };

    UI();
    UI(bool);

    sf::Color hex_to_sfcpo;

    void set_pixel(uint8_t, uint8_t, uint8_t);
    void set_palette(uint8_t, uint8_t, uint8_t, uint8_t);

    void update();
    void tick();

};