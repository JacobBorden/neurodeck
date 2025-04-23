#include "tokenize.hpp"
#include <sstream>

std::vector<std::string> tokenize(const std::string& line) {
    std::istringstream ss(line);
    std::vector<std::string> out;
    std::string tok;
    while (ss >> tok) out.push_back(tok);
    return out;
}
