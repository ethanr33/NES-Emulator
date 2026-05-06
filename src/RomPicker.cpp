#include "RomPicker.h"

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <vector>

namespace {

bool ends_with_nes_extension(const std::string& ext) {
    if (ext.size() != 4) {
        return false;
    }
    return std::tolower(static_cast<unsigned char>(ext[0])) == '.' &&
           std::tolower(static_cast<unsigned char>(ext[1])) == 'n' &&
           std::tolower(static_cast<unsigned char>(ext[2])) == 'e' &&
           std::tolower(static_cast<unsigned char>(ext[3])) == 's';
}

std::vector<std::string> list_nes_files(const std::filesystem::path& dir) {
    std::vector<std::string> names;
    std::error_code ec;
    if (!std::filesystem::exists(dir, ec) || !std::filesystem::is_directory(dir, ec)) {
        return names;
    }
    for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
        if (ec) {
            break;
        }
        if (!entry.is_regular_file()) {
            continue;
        }
        const std::string ext = entry.path().extension().string();
        if (!ends_with_nes_extension(ext)) {
            continue;
        }
        names.push_back(entry.path().filename().string());
    }
    std::sort(names.begin(), names.end());
    return names;
}

bool try_load_system_font(sf::Font& font) {
    if (const char* env = std::getenv("NES_FONT_PATH")) {
        if (font.loadFromFile(env)) {
            return true;
        }
    }
    static const char* candidates[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
    };
    for (const char* path : candidates) {
        if (font.loadFromFile(path)) {
            return true;
        }
    }
    return false;
}

} // namespace

std::optional<std::string> pick_rom_interactively(const std::string& rom_directory) {
    std::vector<std::string> roms = list_nes_files(rom_directory);

    sf::Font font;
    if (!try_load_system_font(font)) {
        std::cerr << "RomPicker: could not load a font. Install DejaVu or Liberation fonts, "
                     "or set NES_FONT_PATH to a .ttf file.\n";
        return std::nullopt;
    }

    constexpr unsigned win_w = 720;
    constexpr unsigned win_h = 520;
    constexpr float pad = 24.f;
    constexpr float line_h = 26.f;
    constexpr float title_h = 36.f;
    constexpr float footer_h = 28.f;
    const float list_top = pad + title_h;
    const float list_h = static_cast<float>(win_h) - list_top - pad - footer_h;
    const int visible_rows = std::max(1, static_cast<int>(list_h / line_h));

    sf::RenderWindow window(sf::VideoMode(win_w, win_h), "NES Emulator — Select a ROM");
    window.setVerticalSyncEnabled(true);

    sf::Text title("ROMs in " + rom_directory + "/", font, 20);
    title.setFillColor(sf::Color(230, 230, 230));

    sf::Text body("", font, 16);
    body.setFillColor(sf::Color(220, 220, 220));

    sf::Text footer("Up / Down: navigate     Enter: load     Esc: quit", font, 14);
    footer.setFillColor(sf::Color(160, 160, 160));

    int selected = 0;
    int scroll = 0;

    while (window.isOpen()) {
        if (!roms.empty()) {
            selected = std::clamp(selected, 0, static_cast<int>(roms.size()) - 1);
            if (selected < scroll) {
                scroll = selected;
            }
            if (selected >= scroll + visible_rows) {
                scroll = selected - visible_rows + 1;
            }
        }

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return std::nullopt;
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                    return std::nullopt;
                }
                if (!roms.empty()) {
                    if (event.key.code == sf::Keyboard::Up) {
                        selected = std::max(0, selected - 1);
                    } else if (event.key.code == sf::Keyboard::Down) {
                        selected = std::min(static_cast<int>(roms.size()) - 1, selected + 1);
                    } else if (event.key.code == sf::Keyboard::Enter ||
                               event.key.code == sf::Keyboard::Return) {
                        window.close();
                        return roms[static_cast<size_t>(selected)];
                    }
                }
            }
        }

        window.clear(sf::Color(40, 44, 52));

        title.setPosition(pad, pad);
        window.draw(title);

        if (roms.empty()) {
            body.setString("No .nes files found. Add ROMs to " + rom_directory + "/");
            body.setPosition(pad, list_top);
            window.draw(body);
        } else {
            for (int i = 0; i < visible_rows; ++i) {
                const int idx = scroll + i;
                if (idx >= static_cast<int>(roms.size())) {
                    break;
                }
                const float y = list_top + static_cast<float>(i) * line_h;
                const bool is_sel = (idx == selected);
                if (is_sel) {
                    sf::RectangleShape highlight(sf::Vector2f(static_cast<float>(win_w) - 2.f * pad, line_h - 2.f));
                    highlight.setPosition(pad, y);
                    highlight.setFillColor(sf::Color(70, 110, 170, 200));
                    window.draw(highlight);
                }
                body.setString(roms[static_cast<size_t>(idx)]);
                body.setFillColor(is_sel ? sf::Color::White : sf::Color(210, 210, 210));
                body.setPosition(pad + 8.f, y + 2.f);
                window.draw(body);
            }
        }

        footer.setPosition(pad, static_cast<float>(win_h) - pad - 6.f);
        window.draw(footer);

        window.display();
    }

    return std::nullopt;
}
