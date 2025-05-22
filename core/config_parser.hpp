#ifndef CORE_CONFIG_PARSER_HPP
#define CORE_CONFIG_PARSER_HPP

#include <string>
#include <map>

namespace Neurodeck {

class ConfigParser {
public:
    ConfigParser(); // Constructor
    // Constructor that loads a file directly.
    explicit ConfigParser(const std::string& filename); 
    
    // Loads configuration from a file. Returns true on success, false on failure (e.g., file not found).
    // Clears any previously loaded configuration.
    bool load_file(const std::string& filename);

    // Retrieves a string value for a given key.
    // Returns default_value if the key is not found.
    std::string get_string(const std::string& key, const std::string& default_value = "") const;

    // Retrieves an integer value for a given key.
    // Returns default_value if the key is not found or if the value cannot be converted to an integer.
    int get_int(const std::string& key, int default_value = 0) const;
    
    // Checks if a key exists
    bool has_key(const std::string& key) const;

private:
    std::map<std::string, std::string> data_;
    // Helper to trim whitespace
    std::string trim_whitespace(const std::string& str) const;
};

} // namespace Neurodeck

#endif // CORE_CONFIG_PARSER_HPP
