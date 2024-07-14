
#include <chrono>

#include "Bus.h"

int main() {

    Bus nes;
    Cartridge* game = new Cartridge("nestest.nes");

    nes.insert_cartridge(game);

    nes.reset();

    const int MICROS_PER_PPU_TICK = 86;

    auto start = std::chrono::steady_clock::now();

    while (true) {
        auto curr_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(curr_time - start).count();

        if (elapsed >= MICROS_PER_PPU_TICK) {
            nes.tick();
        }

    }

    return 0;
}