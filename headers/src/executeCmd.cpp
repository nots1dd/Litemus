#include "../executeCmd.h"
#include "../exitError.h"
#include <cstdio>
#include <sstream>

std::string executeCommand(const std::string& cmd) {
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        printErrorAndExit("[ERROR] Improper pipe.");
    }

    std::string result;
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

    pclose(pipe);
    return result;
}
