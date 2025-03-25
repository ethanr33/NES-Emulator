
#include <iostream>
#include <fstream>

using std::string;

int main(int argc, char** argv) {

    if (argc != 3) {
        std::cout << "Usage: ./logcompare <correct log> <emulator log>" << std::endl;
        return 1;
    }

    string original_log_file_name = argv[1];
    string emulator_log_file_name = argv[2];

    std::ifstream original_log_file(original_log_file_name);

    if (!original_log_file) {
        std::cout << "Error opening correct log file" << std::endl;
        return 1;
    }

    std::ifstream emulator_log_file(emulator_log_file_name);

    if (!emulator_log_file) {
        std::cout << "Error opening emulator log file" << std::endl;
        return 1;
    }

    string cur_orignal_log_line;
    string cur_emulator_log_line;

    int cur_line = 1;

    while (getline(original_log_file, cur_orignal_log_line) && getline(emulator_log_file, cur_emulator_log_line)) {
        if (cur_orignal_log_line != cur_emulator_log_line) {
            std::cout << "Diff at " << cur_line << std::endl;
            return 0;
        }
        cur_line++;
    }

    std::cout << "Finished reading one or both files, no diff found" << std::endl;
    return 0;


}