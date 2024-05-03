
#include <iostream>
#include <string>
#include <set>
#include <iostream>
#include <fstream>
#include "NES.h"

using std::cout;
using std::cin;
using std::getline;
using std::endl;

using std::string;

void print_cpu_state(NES nes, std::ostream& out, bool has_formatting) {
    if (has_formatting) {
        out << "CPU State:" << endl;
        out << "-------------------------------------------------" << endl;
    }
    out << " PC: " << std::hex << nes.get_cpu()->get_program_counter() << " ";
    out << "A: " << std::hex << static_cast<unsigned>(nes.get_cpu()->get_a());
    out << " X: " << std::hex << static_cast<unsigned>(nes.get_cpu()->get_x());
    out << " Y: " << std::hex << static_cast<unsigned>(nes.get_cpu()->get_y());
    out << " Flags: ";

    for (int i = 0; i < 8; i++) {
        out << nes.get_cpu()->get_flag(static_cast<flag_type>(i));
    }

    out << endl;
}

int main() {

    NES nes = NES();

    cout << "Welcome to the CPU debugger" << endl;
    cout << "First, enter the file name of the ROM to test:" << endl;

    string rom_name;

    getline(cin, rom_name);

    cout << endl;

    bool load_success = nes.load_program(rom_name);

    if (load_success) {
        cout << "Loaded rom " << rom_name << endl;
    } else {
        return 1;
    }

    cout << endl;

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
            while (breakpoints.find(nes.get_cpu()->get_program_counter()) == breakpoints.end()) {
                try {
                    print_cpu_state(nes, log_file, false);
                    nes.get_cpu()->execute_next_opcode();
                } catch(std::runtime_error& e) {
                    cout << e.what() << endl;
                    break;
                }
                
            }
        } else if (command == 's') {
            print_cpu_state(nes, log_file, false);
            nes.get_cpu()->execute_next_opcode();
        } else if (command == 'q') {
            return 0;
        } else {
            cout << "Unknown command" << endl;
        }

        cout << endl;
    }

    log_file.close();

}