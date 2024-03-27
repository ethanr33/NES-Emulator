
#include <iostream>
#include <string>
#include <fstream>
#include "CPU.h"

using std::cout;
using std::cin;
using std::getline;
using std::endl;

using std::string;

int main() {

    cout << "Welcome to the CPU debugger" << endl;
    cout << "First, enter the file name of the ROM to test:" << endl;

    string rom_name;

    getline(cin, rom_name);

    while (true) {
        cout << "Please enter a command (type h for list of commands): " << endl;

        char command;
        cin >> command;
    }

}