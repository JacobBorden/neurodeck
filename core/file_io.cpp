#include "file_io.hpp"
#include <fstream>
#include <sstream> // For read_file_to_string
#include <sys/stat.h> // For stat()
#include <unistd.h>   // For S_ISREG on some systems, though sys/stat.h usually has it

namespace CoreFileIO {

bool read_file_to_string(const std::string& filename, std::string& contents) {
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        contents.clear(); // Ensure contents is empty on failure
        return false;
    }

    std::ostringstream sstr;
    sstr << file.rdbuf(); // Read entire buffer
    
    if (file.bad()) { // I/O error during read
        contents.clear();
        return false;
    }
    // No need to check fail() explicitly here if we just read the whole buffer, 
    // as eof is expected. bad() covers stream errors.
    
    contents = sstr.str();
    file.close(); // Close the file
    return true;
}

bool write_string_to_file(const std::string& filename, const std::string& contents) {
    // Open with std::ios::trunc to overwrite if file exists
    std::ofstream file(filename, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }
    file << contents;
    if (file.fail()) { // Check for write errors after writing
        file.close(); // Attempt to close even on failure
        return false;
    }
    file.close(); // Ensure data is flushed and file is closed
    return !file.fail(); // Final check after close to ensure all operations succeeded
}

bool file_exists(const std::string& filename) {
    struct stat buffer;
    if (stat(filename.c_str(), &buffer) != 0) {
        return false; // stat failed (e.g., file not found, permission issue)
    }
    return S_ISREG(buffer.st_mode); // Check if it's a regular file
}

long long get_file_size(const std::string& filename) {
    struct stat buffer;
    if (stat(filename.c_str(), &buffer) != 0) {
        return -1; // stat failed (file not found, or other error)
    }
    if (!S_ISREG(buffer.st_mode)) {
        return -1; // Not a regular file
    }
    return buffer.st_size;
}

} // namespace CoreFileIO
