#ifndef CORE_FILE_IO_HPP
#define CORE_FILE_IO_HPP

#include <string>
#include <vector> // Potentially for future read_file_lines

namespace CoreFileIO {

// Reads the entire content of a file into a string.
// Returns true on success, false if the file cannot be opened or read.
// contents parameter will hold the file content if successful.
bool read_file_to_string(const std::string& filename, std::string& contents);

// Writes a string to a file.
// Returns true on success, false if the file cannot be opened or written.
// This will overwrite the file if it already exists.
bool write_string_to_file(const std::string& filename, const std::string& contents);

// Checks if a file exists and is a regular file.
// Returns true if the file exists and is a regular file, false otherwise.
bool file_exists(const std::string& filename);

// Gets the size of a file in bytes.
// Returns the file size if successful.
// Returns -1 if the file does not exist, is not a regular file, or an error occurs.
long long get_file_size(const std::string& filename);

} // namespace CoreFileIO

#endif // CORE_FILE_IO_HPP
