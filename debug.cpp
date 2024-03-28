
#include <iostream>
#include <string>
#include "NES.h"

using std::cout;
using std::cin;
using std::getline;
using std::endl;

using std::string;

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

    while (true) {
        cout << "Please enter a command (type h for list of commands): " << endl;

        char command;
        cin >> command;

        cout << endl;

        if (command == 'h') {
            cout << "h - Print help menu" << endl;
            cout << "p - Print CPU status" << endl;
            cout << "q - Quit debugger" << endl;
            cout << endl;
        } else if (command == 'p') {
            cout << "CPU State:" << endl;
            cout << "-------------------------------------------------" << endl;
            cout << "A: " << std::hex << static_cast<unsigned>(nes.get_cpu().get_a());
            cout << " X: " << static_cast<unsigned>(nes.get_cpu().get_x());
            cout << " Y: " << static_cast<unsigned>(nes.get_cpu().get_y());
            cout << " PC: " << nes.get_cpu().get_program_counter();
            cout << " Flags: ";

            for (int i = 0; i < 8; i++) {
                cout << nes.get_cpu().get_flag(static_cast<flag_type>(i));
            }

            cout << endl;
        } else if (command == 'q') {
            return 0;
        } else {
            cout << "Unknown command" << endl;
        }

        cout << endl;
    }

}