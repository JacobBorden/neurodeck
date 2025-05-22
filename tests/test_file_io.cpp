#include "gtest/gtest.h"
#include "../core/file_io.hpp"
#include <fstream> // For manual file creation/verification if needed
#include <cstdio>  // For std::remove

// Test fixture for FileIO tests
class FileIOTest : public ::testing::Test {
protected:
    const std::string temp_filename_ = "temp_test_file.txt";
    const std::string non_existent_filename_ = "non_existent_temp_file.txt";

    // Helper to ensure files are cleaned up before each test, if they somehow persist
    void SetUp() override {
        std::remove(temp_filename_.c_str());
        std::remove(non_existent_filename_.c_str()); // Should not exist anyway
    }

    void TearDown() override {
        std::remove(temp_filename_.c_str());
    }
};

TEST_F(FileIOTest, TestReadFileToString_Success) {
    // Create a temporary file with known content
    std::ofstream outfile(temp_filename_);
    ASSERT_TRUE(outfile.is_open());
    const std::string expected_content = "Hello, World!\nThis is a test file.";
    outfile << expected_content;
    outfile.close();

    std::string actual_content;
    EXPECT_TRUE(CoreFileIO::read_file_to_string(temp_filename_, actual_content));
    EXPECT_EQ(actual_content, expected_content);
}

TEST_F(FileIOTest, TestReadFileToString_NonExistentFile) {
    std::string actual_content = "pre_existing_value"; // To check if it gets cleared
    EXPECT_FALSE(CoreFileIO::read_file_to_string(non_existent_filename_, actual_content));
    EXPECT_TRUE(actual_content.empty()) << "Content should be cleared on failure.";
}

TEST_F(FileIOTest, TestReadFileToString_EmptyFile) {
    std::ofstream outfile(temp_filename_); // Creates an empty file
    ASSERT_TRUE(outfile.is_open());
    outfile.close();

    std::string actual_content;
    EXPECT_TRUE(CoreFileIO::read_file_to_string(temp_filename_, actual_content));
    EXPECT_TRUE(actual_content.empty());
}

TEST_F(FileIOTest, TestWriteStringToFile_NewFile) {
    const std::string content_to_write = "Writing to a new file.\nLine 2.";
    EXPECT_TRUE(CoreFileIO::write_string_to_file(temp_filename_, content_to_write));

    std::string content_read_back;
    ASSERT_TRUE(CoreFileIO::read_file_to_string(temp_filename_, content_read_back));
    EXPECT_EQ(content_read_back, content_to_write);
}

TEST_F(FileIOTest, TestWriteStringToFile_OverwriteExisting) {
    // Create an initial file
    const std::string initial_content = "Initial content.";
    ASSERT_TRUE(CoreFileIO::write_string_to_file(temp_filename_, initial_content));

    // Overwrite it
    const std::string new_content = "Overwritten content.";
    EXPECT_TRUE(CoreFileIO::write_string_to_file(temp_filename_, new_content));

    std::string content_read_back;
    ASSERT_TRUE(CoreFileIO::read_file_to_string(temp_filename_, content_read_back));
    EXPECT_EQ(content_read_back, new_content);
}

TEST_F(FileIOTest, TestWriteStringToFile_EmptyString) {
    const std::string empty_content = "";
    EXPECT_TRUE(CoreFileIO::write_string_to_file(temp_filename_, empty_content));

    std::string content_read_back;
    ASSERT_TRUE(CoreFileIO::read_file_to_string(temp_filename_, content_read_back));
    EXPECT_TRUE(content_read_back.empty());
    EXPECT_EQ(CoreFileIO::get_file_size(temp_filename_), 0);
}

TEST_F(FileIOTest, TestWriteStringToFile_CannotWrite) {
    // This test is tricky without changing permissions or using invalid paths.
    // An invalid path (e.g., a directory as filename) might fail.
    // For now, let's try writing to a path that is likely a directory.
    // Note: This behavior can be OS-dependent.
    // On Unix-like systems, writing to a directory name with ofstream usually fails.
    // On Windows, this might also fail or have different behavior.
    // A more robust test would involve mocking the filesystem or setting up specific permissions.
    const std::string directory_path = "."; // Current directory
    EXPECT_FALSE(CoreFileIO::write_string_to_file(directory_path, "test content"));
}


TEST_F(FileIOTest, TestFileExists) {
    EXPECT_FALSE(CoreFileIO::file_exists(temp_filename_)); // Should not exist initially

    std::ofstream outfile(temp_filename_);
    ASSERT_TRUE(outfile.is_open());
    outfile.close();
    EXPECT_TRUE(CoreFileIO::file_exists(temp_filename_));

    std::remove(temp_filename_.c_str());
    EXPECT_FALSE(CoreFileIO::file_exists(temp_filename_));
    
    // Check that a directory is not reported as a file by file_exists
    // (S_ISREG check in implementation)
    const std::string current_dir = "."; 
    EXPECT_FALSE(CoreFileIO::file_exists(current_dir));
}

TEST_F(FileIOTest, TestGetFileSize) {
    const std::string content = "File with known size."; // 21 bytes
    ASSERT_TRUE(CoreFileIO::write_string_to_file(temp_filename_, content));
    EXPECT_EQ(CoreFileIO::get_file_size(temp_filename_), static_cast<long long>(content.length()));

    // Empty file
    ASSERT_TRUE(CoreFileIO::write_string_to_file(temp_filename_, ""));
    EXPECT_EQ(CoreFileIO::get_file_size(temp_filename_), 0);

    // Non-existent file
    EXPECT_EQ(CoreFileIO::get_file_size(non_existent_filename_), -1);
    
    // Directory (should also return -1 as it's not a regular file)
    const std::string current_dir = ".";
    EXPECT_EQ(CoreFileIO::get_file_size(current_dir), -1);
}

TEST_F(FileIOTest, TestReadNonExistentFileClearsContent) {
    std::string content = "Initial value that should be cleared";
    ASSERT_FALSE(CoreFileIO::read_file_to_string(non_existent_filename_, content));
    EXPECT_TRUE(content.empty());
}
