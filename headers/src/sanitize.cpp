// sanitize.cpp

#include "../sanitize.h"

std::string sanitizeString(const std::string& input) {
    std::string sanitized = input;
    for (char& c : sanitized) {
        if (c == '/' || c == '\'' || std::isspace(c) || c == '.') {
            c = '_';
        }
    }
    return sanitized;
}
