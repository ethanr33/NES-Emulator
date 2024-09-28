
#include <iostream>
#include <string>
#include <set>
#include <iostream>
#include <fstream>
#include "NES.h"
#include "Helpers.h"

using std::cout;
using std::cin;
using std::getline;
using std::endl;

using std::string;

void print_cpu_state(Bus nes, std::ostream& out, bool has_formatting) {
    if (has_formatting) {
        out << "CPU State:" << endl;
        out << "-------------------------------------------------" << endl;
    }
    out << " PC: " << std::hex << nes.cpu->program_counter << " ";
    out << "Opcode: " << std::hex << static_cast<unsigned>(nes.read_cpu(nes.cpu->program_counter))<< " ";
    out << "A: " << std::hex << static_cast<unsigned>(nes.cpu->A);
    out << " X: " << std::hex << static_cast<unsigned>(nes.cpu->X);
    out << " Y: " << std::hex << static_cast<unsigned>(nes.cpu->Y);
    out << " SP: " << std::hex << static_cast<unsigned>(nes.cpu->stack_pointer)<< " ";
    out << " Cycles: " << std::dec << nes.cpu->num_clock_cycles << " ";

    for (int i = 0; i < 8; i++) {
        out << nes.cpu->get_flag(static_cast<flag_type>(i));
    }

    out << endl;
}

int main() {

    Bus nes = Bus();

    cout << "Welcome to the CPU debugger" << endl;
    //cout << "First, enter the file name of the ROM to test:" << endl;

    string rom_name = "donkeykong.nes";

    //getline(cin, rom_name);

    //cout << endl;

    Cartridge* cartridge = new Cartridge(rom_name);
    nes.insert_cartridge(cartridge);
    nes.reset();

    std::ofstream log_file("debug.log");

    if (!log_file.is_open()) {
        cout << "Failed to open log file" << endl;
        return 1;
    }

    std::set<uint16_t> breakpoints;

    while (true) {
        cout << "Please enter a command (type h for list of commands): " << endl;

        char command;
        cin >> command;

        cout << endl;

        if (command == 'h') {
            cout << "b - Make breakpoint" << endl;
            cout << "c - Compare to expected output" << endl;
            cout << "h - Print help menu" << endl;
            cout << "p - Print CPU status" << endl;
            cout << "r - Run" << endl;
            cout << "s - Step" << endl;
            cout << "q - Quit debugger" << endl;
            cout << endl;
        } else if (command == 'b') {
            cout << "What address do you want to set a breakpoint at?" << endl;

            uint16_t breakpoint_address;
            cin >> std::hex >> breakpoint_address;

            breakpoints.insert(breakpoint_address);

            cout << "Breakpoint set at " << std::hex << breakpoint_address;
        } else if (command == 'p') {
            print_cpu_state(nes, cout, true);
            cout << endl;
        } else if (command == 'r') {
            while (breakpoints.find(nes.cpu->program_counter) == breakpoints.end()) {
                try {
                    print_cpu_state(nes, log_file, false);
                    nes.cpu->execute_next_opcode();
                } catch(std::runtime_error& e) {
                    cout << e.what() << endl;
                    break;
                }
                
            }
        } else if (command == 's') {
            print_cpu_state(nes, log_file, false);
            nes.cpu->execute_next_opcode();
        } else if (command == 'c') {
            std::ifstream expected_output("nestest.log");

            bool matching = true;

            while (matching) {
                string line;
                string expected_pc;
                string expected_a;
                int expected_cyc;

                expected_output >> expected_pc;
                expected_output.ignore();
                std::getline(expected_output, line);

                size_t a_pos = line.find(':');
                expected_a = line.substr(a_pos + 1, 2);

                size_t cyc_pos = line.find("CYC:");
                expected_cyc = stoi(line.substr(cyc_pos));

                string actual_pc = get_hex_string(nes.cpu->program_counter, 4);
                string actual_A = get_hex_string(nes.cpu->A, 2);
                int actual_cycles = nes.cpu->num_clock_cycles;

                bool pc_matches = (expected_pc == actual_pc);
                bool A_matches = (expected_a == actual_A);
                bool cycles_match (expected_cyc == actual_cycles);


                matching = pc_matches && A_matches && cycles_match;

                print_cpu_state(nes, log_file, false);

                if (!pc_matches) {
                    log_file << "PC does not match!" << endl;
                }

                if (!A_matches) {
                    log_file << "A does not match!" << endl;
                }

                if (!cycles_match) {
                    log_file << "Cycles do not match!" << endl;
                }

                nes.cpu->execute_next_opcode();
            }

        } else if (command == 'q') {
            return 0;
        } else {
            cout << "Unknown command" << endl;
        }

        cout << endl;
    }

    log_file.close();

}