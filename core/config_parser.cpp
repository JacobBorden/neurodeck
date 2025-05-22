#include "config_parser.hpp"
#include <fstream>
#include <sstream>
#include <algorithm> // For std::remove_if for trim
#include <limits>

namespace Neurodeck {

// Helper to trim whitespace from both ends of a string
std::string ConfigParser::trim_whitespace(const std::string& str) const {
    const std::string whitespace = " \t\n\r\f\v";
    size_t start = str.find_first_not_of(whitespace);
    if (start == std::string::npos) {
        return ""; // String is all whitespace
    }
    size_t end = str.find_last_not_of(whitespace);
    return str.substr(start, end - start + 1);
}

ConfigParser::ConfigParser() {}

ConfigParser::ConfigParser(const std::string& filename) {
    load_file(filename);
}

bool ConfigParser::load_file(const std::string& filename) {
    data_.clear(); // Clear previous configuration

    std::ifstream file(filename);
    if (!file.is_open()) {
        return false; // Failed to open file
    }

    std::string line;
    while (std::getline(file, line)) {
        // First, handle and remove inline comments for the entire line
        // This is a simplified approach: take everything before the first '#'
        // A more robust parser might need to be context-aware (e.g. '#' in quoted strings)
        size_t comment_char_pos = line.find('#');
        if (comment_char_pos != std::string::npos) {
            line = line.substr(0, comment_char_pos);
        }
        
        line = trim_whitespace(line);

        if (line.empty()) { // Skip lines that are now empty (were whitespace or full-line comments)
            continue; 
        }
        
        size_t equals_pos = line.find('=');
        if (equals_pos == std::string::npos) {
            continue; // Skip lines without an '=' separator
        }

        std::string key = line.substr(0, equals_pos);
        std::string value = line.substr(equals_pos + 1);

        key = trim_whitespace(key);
        value = trim_whitespace(value);

        if (!key.empty()) { // Ensure key is not empty after trimming
            data_[key] = value;
        }
    }
    return true;
}

std::string ConfigParser::get_string(const std::string& key, const std::string& default_value) const {
    auto it = data_.find(key);
    if (it != data_.end()) {
        return it->second;
    }
    return default_value;
}

int ConfigParser::get_int(const std::string& key, int default_value) const {
    auto it = data_.find(key);
    if (it != data_.end()) {
        try {
            // Use std::stoll for wider range then check if it fits in int
            long long long_val = std::stoll(it->second);
            if (long_val >= std::numeric_limits<int>::min() && long_val <= std::numeric_limits<int>::max()) {
                return static_cast<int>(long_val);
            }
            // If out of int range, treat as conversion failure
        } catch (const std::invalid_argument& ia) {
            // Not a valid integer string
        } catch (const std::out_of_range& oor) {
            // Integer out of range for long long
        }
    }
    return default_value;
}

bool ConfigParser::has_key(const std::string& key) const {
    return data_.count(key) > 0;
}

} // namespace Neurodeck
