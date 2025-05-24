/**
 * @file config_parser.hpp
 * @brief Defines the ConfigParser class for loading and querying key-value configuration settings.
 *
 * This file provides the declaration of the ConfigParser class, which is used
 * throughout the Neurodeck application to manage configuration parameters
 * loaded from files. It supports basic operations like reading string and integer
 * values, checking for key existence, and loading configuration from a specified file.
 */
#ifndef CORE_CONFIG_PARSER_HPP
#define CORE_CONFIG_PARSER_HPP

#include <string>
#include <map>

namespace Neurodeck {

/**
 * @brief Parses and stores key-value configuration settings from a file.
 *
 * The ConfigParser class handles reading configuration files where each line
 * is expected to be in a 'key=value' format. It provides methods to retrieve
 * these values by key, with support for default values if a key is not found.
 */
class ConfigParser {
public:
    /**
     * @brief Default constructor.
     * Creates an empty ConfigParser object.
     */
    ConfigParser();

    /**
     * @brief Constructor that loads configuration from a file.
     * @param filename The path to the configuration file to load.
     *                 The file should contain key-value pairs, one per line, separated by an equals sign (=).
     *                 Lines starting with '#' or ';' are considered comments and ignored.
     *                 Whitespace around keys and values is trimmed.
     */
    explicit ConfigParser(const std::string& filename); 
    
    /**
     * @brief Loads configuration settings from a specified file.
     * 
     * This method reads key-value pairs from the given file. Any existing configuration
     * in the object is cleared before loading the new file.
     * Lines in the file are expected to be in 'key=value' format.
     * Lines starting with '#' or ';' are treated as comments and are ignored.
     * Whitespace around keys and values is automatically trimmed.
     * 
     * @param filename The path to the configuration file.
     * @return true if the file was successfully loaded and parsed, false otherwise
     *         (e.g., if the file cannot be opened).
     */
    bool load_file(const std::string& filename);

    /**
     * @brief Retrieves a string value associated with a given key.
     * @param key The key whose value is to be retrieved.
     * @param default_value The value to return if the key is not found in the configuration.
     *                      Defaults to an empty string.
     * @return The string value for the key if found; otherwise, the default_value.
     */
    std::string get_string(const std::string& key, const std::string& default_value = "") const;

    /**
     * @brief Retrieves an integer value associated with a given key.
     * 
     * If the key is found, the method attempts to convert its string value to an integer.
     * If the conversion fails, or if the key is not found, the default_value is returned.
     * 
     * @param key The key whose integer value is to be retrieved.
     * @param default_value The value to return if the key is not found or if the value
     *                      cannot be converted to an integer. Defaults to 0.
     * @return The integer value for the key if found and convertible; otherwise, the default_value.
     */
    int get_int(const std::string& key, int default_value = 0) const;
    
    /**
     * @brief Checks if a specific key exists in the configuration.
     * @param key The key to check for.
     * @return true if the key exists, false otherwise.
     */
    bool has_key(const std::string& key) const;

private:
    /// @brief Internal storage for the key-value configuration data.
    std::map<std::string, std::string> data_; 
    
    /**
     * @brief Helper function to trim leading and trailing whitespace from a string.
     * @param str The string to trim.
     * @return The trimmed string.
     */
    std::string trim_whitespace(const std::string& str) const;
};

} // namespace Neurodeck

#endif // CORE_CONFIG_PARSER_HPP
