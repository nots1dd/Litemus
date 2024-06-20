#ifndef EXIT_ERROR_H
#define EXIT_ERROR_H

#include <iostream>
#include <cstdlib>
#include <string>

const std::string RED_EXIT = "\033[31m";
const std::string BOLD_EXIT = "\033[1m";

inline void printErrorAndExit(const std::string& msg) {
    std::cerr << RED_EXIT << BOLD_EXIT << msg << std::endl;
    std::exit(EXIT_FAILURE);
}

#endif // EXIT_ERROR_H
