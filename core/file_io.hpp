/**
 * @file file_io.hpp
 * @brief Defines utility functions for file input/output operations.
 *
 * This file provides a namespace `CoreFileIO` containing a set of standalone functions
 * for common file operations such as reading from and writing to files,
 * checking file existence, and retrieving file sizes. These utilities are
 * intended for general-purpose file handling within the Neurodeck application.
 */
#ifndef CORE_FILE_IO_HPP
#define CORE_FILE_IO_HPP

#include <string>
#include <vector> // Potentially for future read_file_lines

/**
 * @brief Namespace containing utility functions for core file I/O operations.
 *
 * This namespace groups functions that perform basic file operations like
 * reading, writing, checking existence, and getting file size. These are
 * fundamental operations needed by various parts of an application that
 * interact with the file system.
 */
namespace CoreFileIO {

/**
 * @brief Reads the entire content of a file into a string.
 * @param filename The path to the file to be read.
 * @param[out] contents A reference to a string where the file's content will be stored.
 *                      If the function fails, the content of this string is undefined.
 * @return true if the file was successfully read and its content stored in `contents`.
 * @return false if the file cannot be opened, read, or another I/O error occurs.
 */
bool read_file_to_string(const std::string& filename, std::string& contents);

/**
 * @brief Writes a string to a file.
 * 
 * This function writes the provided string `contents` to the specified `filename`.
 * If the file already exists, its content will be overwritten. If the file does not
 * exist, it will be created.
 * 
 * @param filename The path to the file where the string will be written.
 * @param contents The string content to write to the file.
 * @return true if the string was successfully written to the file.
 * @return false if the file cannot be opened for writing or another I/O error occurs.
 */
bool write_string_to_file(const std::string& filename, const std::string& contents);

/**
 * @brief Checks if a file exists and is a regular file.
 * @param filename The path to the file to check.
 * @return true if the file exists at the specified path and is a regular file.
 * @return false if the file does not exist, is not a regular file (e.g., it's a directory),
 *         or an error occurs during the check.
 */
bool file_exists(const std::string& filename);

/**
 * @brief Gets the size of a file in bytes.
 * @param filename The path to the file whose size is to be determined.
 * @return The size of the file in bytes if successful.
 * @return -1 if the file does not exist, is not a regular file (e.g., it's a directory),
 *         or if an error occurs while trying to determine the file size.
 */
long long get_file_size(const std::string& filename);

} // namespace CoreFileIO

#endif // CORE_FILE_IO_HPP
