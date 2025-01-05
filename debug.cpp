
#include <iostream>
#include <string>
#include <iostream>
#include <queue>
#include <iomanip>
#include <sstream>

#include "Bus.h"
#include "Helpers.h"

using std::cout;
using std::cin;
using std::getline;
using std::endl;
using std::string;
using std::to_string;

sf::RenderWindow cpu_window;

sf::Sprite step_button;
sf::Sprite step_scanline_button;
sf::Sprite step_frame_button;

int elapsed_cpu_cycles = 0;

// Snapshot struct represents the status of the emulator (PC, registers, etc.) at a given moment in time
struct Snapshot {
    uint16_t instruction; // opcode of currently executing instruction
    uint16_t program_counter; // current value of program counter
    uint8_t stack_pointer;

    uint8_t A;
    uint8_t X;
    uint8_t Y;
    uint8_t flags;

    uint16_t scanline;
    uint16_t dot;
    
    // constructor
    Snapshot(uint16_t instr, uint16_t pc, uint8_t sp, uint8_t a, uint8_t x, uint8_t y, uint8_t flag, uint16_t line, uint16_t cur_dot) {
        instruction = instr;
        program_counter = pc;
        stack_pointer = sp;
        A = a;
        X = x;
        Y = y;
        flags = flag;
        scanline = line;
        dot = cur_dot;
    }

    // returns a formatted string containing all data from snapshot
    string to_string() {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(4) << std::hex;
        ss << program_counter << "   ";
        ss << std::setw(2) << instruction << "          ";
        ss << "A: "<< std::setfill('0') << std::setw(2) << std::hex << (int) A << "  ";
        ss << "X: "<< std::setfill('0') << std::setw(2) << std::hex << (int) X << "  ";
        ss << "Y: "<< std::setfill('0') << std::setw(2) << std::hex << (int) Y << "  ";
        ss << "SP: "<< std::setfill('0') << std::setw(2) << std::hex << (int) stack_pointer << "  ";
        ss << "F: ";

        for (int i = 7; i >= 0; i--) {
            if (is_bit_set(i, flags)) {
                ss << "1";
            } else {
                ss << "0";
            }
        }

        ss << "  ";

        ss << "SL: "<< std::dec << (int) scanline << "  ";
        ss << "D: "<< std::dec << (int) dot << "  ";

        return ss.str();
    }
};

// A queue containing the 10 most recent snapshots of the status of the CPU
// To maintain the size of the queue, if a new snapshot is pushed to a full queue the oldest snapshot is popped off
std::vector<Snapshot> recent_instructions;

void draw_cpu_state(CPU* cpu, PPU* ppu) {
    sf::Texture step_arrow;
    sf::Texture step_scanline;
    sf::Texture step_frame;
    sf::Font font;

    // Load resources
    step_arrow.loadFromFile("assets/icons/stepinto.png");
    step_scanline.loadFromFile("assets/icons/nextscanline.png");
    step_frame.loadFromFile("assets/icons/nextframe.png");
    font.loadFromFile("assets/fonts/spacemono.ttf");

    // Draw buttons
    step_button.setTexture(step_arrow);
    step_button.setScale(sf::Vector2f(2, 2));
    step_button.setPosition(sf::Vector2f(20, 10));

    step_scanline_button.setTexture(step_scanline);
    step_scanline_button.setScale(sf::Vector2f(2, 2));
    step_scanline_button.setPosition(sf::Vector2f(60, 10));

    step_frame_button.setTexture(step_frame);
    step_frame_button.setScale(sf::Vector2f(2, 2));
    step_frame_button.setPosition(sf::Vector2f(100, 10));


    cpu_window.draw(step_button);
    cpu_window.draw(step_scanline_button);
    cpu_window.draw(step_frame_button);

    // Display CPU trace log
    for (int i = 0; i < recent_instructions.size(); i++) {
        Snapshot s = recent_instructions.at(i);

        sf::Text instruction;

        instruction.setFont(font);
        instruction.setString(s.to_string());
        instruction.setPosition(sf::Vector2f(20, 50 + 25 * i));
        instruction.setFillColor(sf::Color::Black);
        instruction.setCharacterSize(20);

        cpu_window.draw(instruction);
    }

    // Draw PPUCTRL flags

    const string ppu_ctrl_header_text = "PPUCTRL:";
    sf::Text ppu_ctrl_header;

    sf::Text nmi_enable_text;
    sf::Text ppu_ms_text;
    sf::Text sprite_height_text;
    sf::Text background_tile_select_text;
    sf::Text sprite_tile_select_text;
    sf::Text increment_mode_text;
    sf::Text nametable_select_text;

    ppu_ctrl_header.setString(ppu_ctrl_header_text);
    ppu_ctrl_header.setFont(font);
    ppu_ctrl_header.setPosition(sf::Vector2f(20, 310));
    ppu_ctrl_header.setFillColor(sf::Color::Black);
    ppu_ctrl_header.setCharacterSize(25);

    cpu_window.draw(ppu_ctrl_header);

    nmi_enable_text.setString("NMI Enable: " + to_string(ppu->ppuctrl.nmi_enable));
    nmi_enable_text.setFont(font);
    nmi_enable_text.setPosition(sf::Vector2f(20, 340));
    nmi_enable_text.setFillColor(sf::Color::Black);
    nmi_enable_text.setCharacterSize(20);

    ppu_ms_text.setString("PPU M/S: " + to_string(ppu->ppuctrl.ppu_ms));
    ppu_ms_text.setFont(font);
    ppu_ms_text.setPosition(sf::Vector2f(20, 360));
    ppu_ms_text.setFillColor(sf::Color::Black);
    ppu_ms_text.setCharacterSize(20);

    sprite_height_text.setString("Sprite Height: " + to_string(ppu->ppuctrl.sprite_height));
    sprite_height_text.setFont(font);
    sprite_height_text.setPosition(sf::Vector2f(20, 380));
    sprite_height_text.setFillColor(sf::Color::Black);
    sprite_height_text.setCharacterSize(20);

    background_tile_select_text.setString("BG Tile Select: " + to_string(ppu->ppuctrl.background_tile_select));
    background_tile_select_text.setFont(font);
    background_tile_select_text.setPosition(sf::Vector2f(20, 400));
    background_tile_select_text.setFillColor(sf::Color::Black);
    background_tile_select_text.setCharacterSize(20);

    sprite_tile_select_text.setString("Sprite Tile Select: " + to_string(ppu->ppuctrl.sprite_tile_select));
    sprite_tile_select_text.setFont(font);
    sprite_tile_select_text.setPosition(sf::Vector2f(20, 420));
    sprite_tile_select_text.setFillColor(sf::Color::Black);
    sprite_tile_select_text.setCharacterSize(20);

    increment_mode_text.setString("Increment Mode: " + to_string(ppu->ppuctrl.increment_mode));
    increment_mode_text.setFont(font);
    increment_mode_text.setPosition(sf::Vector2f(20, 440));
    increment_mode_text.setFillColor(sf::Color::Black);
    increment_mode_text.setCharacterSize(20);

    nametable_select_text.setString("Nametable Select: " + to_string(ppu->ppuctrl.nametable_select));
    nametable_select_text.setFont(font);
    nametable_select_text.setPosition(sf::Vector2f(20, 460));
    nametable_select_text.setFillColor(sf::Color::Black);
    nametable_select_text.setCharacterSize(20);


    cpu_window.draw(nmi_enable_text);
    cpu_window.draw(ppu_ms_text);
    cpu_window.draw(sprite_height_text);
    cpu_window.draw(background_tile_select_text);
    cpu_window.draw(sprite_tile_select_text);
    cpu_window.draw(increment_mode_text);
    cpu_window.draw(nametable_select_text);

    // Draw PPUSTATUS flags

    const string ppu_status_header_text = "PPUSTATUS:";
    sf::Text ppu_status_header;

    sf::Text vblank_text;
    sf::Text sprite0_hit_text;
    sf::Text sprite_overflow_text;

    ppu_status_header.setString(ppu_status_header_text);
    ppu_status_header.setFont(font);
    ppu_status_header.setPosition(sf::Vector2f(300, 310));
    ppu_status_header.setFillColor(sf::Color::Black);
    ppu_status_header.setCharacterSize(25);

    cpu_window.draw(ppu_status_header);

    vblank_text.setString("VBlank: " + to_string(ppu->ppustatus.vblank));
    vblank_text.setFont(font);
    vblank_text.setPosition(sf::Vector2f(300, 340));
    vblank_text.setFillColor(sf::Color::Black);
    vblank_text.setCharacterSize(20);

    sprite0_hit_text.setString("Sprite Hit: " + to_string(ppu->ppustatus.sprite_hit));
    sprite0_hit_text.setFont(font);
    sprite0_hit_text.setPosition(sf::Vector2f(300, 360));
    sprite0_hit_text.setFillColor(sf::Color::Black);
    sprite0_hit_text.setCharacterSize(20);

    sprite_overflow_text.setString("Sprite Overflow: " + to_string(ppu->ppustatus.sprite_overflow));
    sprite_overflow_text.setFont(font);
    sprite_overflow_text.setPosition(sf::Vector2f(300, 380));
    sprite_overflow_text.setFillColor(sf::Color::Black);
    sprite_overflow_text.setCharacterSize(20);

    cpu_window.draw(vblank_text);
    cpu_window.draw(sprite0_hit_text);
    cpu_window.draw(sprite_overflow_text);

    // Draw nametable

    const string nametable_header_text = "Nametable:";
    sf::Text nametable_header;

    nametable_header.setString(nametable_header_text);
    nametable_header.setFont(font);
    nametable_header.setPosition(sf::Vector2f(20, 500));
    nametable_header.setFillColor(sf::Color::Black);
    nametable_header.setCharacterSize(30);

    cpu_window.draw(nametable_header);

    for (int i = 0; i < 30; i++) {
        for (int j = 0; j < 32; j++) {
            sf::Text nametable_cell_text;


            nametable_cell_text.setString(to_string(ppu->VRAM[0x2000 + 32 * i + j]));
            nametable_cell_text.setFont(font);
            nametable_cell_text.setPosition(sf::Vector2f(20 + 20 * j, 550 + 20 * i));
            nametable_cell_text.setFillColor(sf::Color::Black);
            nametable_cell_text.setCharacterSize(10);

            cpu_window.draw(nametable_cell_text);
        }
    }

}

void step_forward(Bus nes) {
    // Retrieve CPU status
    Snapshot cur_status = Snapshot(
        nes.read_cpu(nes.cpu->program_counter),
        nes.cpu->program_counter,
        nes.cpu->stack_pointer,
        nes.cpu->A,
        nes.cpu->X,
        nes.cpu->Y,
        nes.cpu->get_byte_from_flags(),
        nes.ppu->scanline,
        nes.ppu->cur_dot
    );

    // Update queue
    recent_instructions.push_back(cur_status);

    // If the queue has more than 10 recent instructions, shrink it
    if (recent_instructions.size() > 10) {
        recent_instructions.erase(recent_instructions.begin());
    }

    // Simulate ticking the PPU forward when we step the CPU forward
    // 3 PPU cycles = 1 CPU cycle
    std::cout << nes.cpu->num_clock_cycles << " " << elapsed_cpu_cycles << std::endl;
    for (int i = 0; i < 3 * (nes.cpu->num_clock_cycles - elapsed_cpu_cycles); i++) {
        nes.ppu->tick();
    }

    elapsed_cpu_cycles = nes.cpu->num_clock_cycles;

    // Step forward one CPU opcode
    nes.cpu->execute_next_opcode();
}

int main() {

    Bus nes = Bus(true);

    string rom_name = "nestest.nes";

    Cartridge* cartridge = new Cartridge(rom_name);
    nes.insert_cartridge(cartridge);
    nes.reset();

    cpu_window.create(sf::VideoMode(1600, 1600), "Tracer");

    while (cpu_window.isOpen()) {
        sf::Event event;

        while (cpu_window.pollEvent(event))
        {
            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
                cpu_window.close();

            // Check if the left mouse button is pressed
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i mouse_pos = sf::Mouse::getPosition(cpu_window);

                if (step_button.getGlobalBounds().contains((sf::Vector2f) mouse_pos)) {
                    // Step forward button was pressed
                    step_forward(nes);
                } else if (step_scanline_button.getGlobalBounds().contains((sf::Vector2f) mouse_pos)) {
                    int cur_scanline = nes.ppu->scanline;
                    while (nes.ppu->scanline == cur_scanline) {
                        step_forward(nes);
                    }
                } else if (step_frame_button.getGlobalBounds().contains((sf::Vector2f) mouse_pos)) {
                    int cur_scanline = nes.ppu->scanline;
                    while (nes.ppu->scanline == cur_scanline) {
                        step_forward(nes);
                    }

                    while (nes.ppu->scanline != cur_scanline) {
                        step_forward(nes);
                    }
                }
            }   
        }

        cpu_window.clear(sf::Color::White);
        draw_cpu_state(nes.cpu, nes.ppu);
        cpu_window.display();
    }

}