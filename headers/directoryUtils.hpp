// directory_utils.hpp
#ifndef DIRECTORY_UTILS_HPP
#define DIRECTORY_UTILS_HPP

#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include "exitError.h"  // Assuming this header contains the declaration of printErrorAndExit function

using namespace std;

// ANSI Escape Codes for Colors
extern const string RESET;
extern const string GREEN;
extern const string YELLOW;
extern const string BOLD;

// Function to create a directory if it doesn't exist
void createDirectory(const string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
        if (mkdir(path.c_str(), 0755) == 0) {
            cout << GREEN << "[SUCCESS] Directory created successfully: " << path << RESET << endl;
        } else {
            cerr << YELLOW << BOLD << "[ERROR] Failed to create directory: " << path << " (" << strerror(errno) << ")" << RESET << endl;
        }
    }
}

// Function to change the current working directory
void changeDirectory(const string& path) {
    if (chdir(path.c_str()) != 0) {
        printErrorAndExit("[ERROR] Failed to change directory.");
    }
    cout << GREEN << "[SUCCESS] Directory changed successfully." << RESET << endl;
}

#endif // DIRECTORY_UTILS_HPP
