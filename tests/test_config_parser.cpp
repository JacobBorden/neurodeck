#include "gtest/gtest.h"
#include "core/config_parser.hpp"
#include <fstream>
#include <cstdio> // For std::remove

// Helper function to create a temporary config file for tests
void create_temp_config_file(const std::string& filename, const std::string& content) {
    std::ofstream outfile(filename);
    ASSERT_TRUE(outfile.is_open()) << "Failed to create temp config file: " << filename;
    outfile << content;
    outfile.close();
}

// Test fixture for ConfigParser tests
class ConfigParserTest : public ::testing::Test {
protected:
    std::string temp_config_filename_ = "temp_test_config.ini";

    void TearDown() override {
        std::remove(temp_config_filename_.c_str());
    }
};

TEST_F(ConfigParserTest, TestLoadNonExistentFile) {
    Neurodeck::ConfigParser parser;
    EXPECT_FALSE(parser.load_file("non_existent_file.ini"));
}

TEST_F(ConfigParserTest, TestLoadValidFile) {
    const std::string content = 
        "key1 = value1\n"
        "key2 = 123\n"
        "# This is a comment\n"
        "  key3 =  spaced value  \n"
        "key4 = another value # with inline comment\n"
        "keyInt = -456\n"
        "\n" // Empty line
        "key_empty_val = \n"
        "key_with_equals_in_value = key=value\n";
    create_temp_config_file(temp_config_filename_, content);

    Neurodeck::ConfigParser parser;
    ASSERT_TRUE(parser.load_file(temp_config_filename_));

    EXPECT_EQ(parser.get_string("key1"), "value1");
    EXPECT_TRUE(parser.has_key("key1"));

    EXPECT_EQ(parser.get_int("key2"), 123);
    EXPECT_TRUE(parser.has_key("key2"));
    
    EXPECT_EQ(parser.get_string("key3"), "spaced value");
    EXPECT_TRUE(parser.has_key("key3"));

    EXPECT_EQ(parser.get_string("key4"), "another value");
    EXPECT_TRUE(parser.has_key("key4"));

    EXPECT_EQ(parser.get_int("keyInt"), -456);
    EXPECT_TRUE(parser.has_key("keyInt"));

    EXPECT_FALSE(parser.has_key("This is a comment")); // Check comment isn't a key
    EXPECT_FALSE(parser.has_key("")); // Check empty line isn't a key

    EXPECT_EQ(parser.get_string("key_empty_val"), "");
    EXPECT_TRUE(parser.has_key("key_empty_val"));
    
    EXPECT_EQ(parser.get_string("key_with_equals_in_value"), "key=value");
    EXPECT_TRUE(parser.has_key("key_with_equals_in_value"));
}

TEST_F(ConfigParserTest, TestGetStringNotFound) {
    Neurodeck::ConfigParser parser;
    EXPECT_EQ(parser.get_string("non_existent_key", "default_val"), "default_val");
    EXPECT_EQ(parser.get_string("non_existent_key"), ""); // Default default_value
}

TEST_F(ConfigParserTest, TestGetIntNotFound) {
    Neurodeck::ConfigParser parser;
    EXPECT_EQ(parser.get_int("non_existent_key", 999), 999);
    EXPECT_EQ(parser.get_int("non_existent_key"), 0); // Default default_value
}

TEST_F(ConfigParserTest, TestGetIntInvalidFormat) {
    const std::string content = "key_string = not_an_integer\nkey_float = 1.23\nkey_large = 99999999999999999999";
    create_temp_config_file(temp_config_filename_, content);
    Neurodeck::ConfigParser parser;
    ASSERT_TRUE(parser.load_file(temp_config_filename_));

    EXPECT_EQ(parser.get_int("key_string", 123), 123);
    EXPECT_EQ(parser.get_int("key_float", 456), 456);
    EXPECT_EQ(parser.get_int("key_large", 789), 789); // std::stoll might parse then check range
}

TEST_F(ConfigParserTest, TestCommentsAndEmptyLines) {
    const std::string content =
        "# Full comment line 1\n"
        "key1 = value1\n"
        "\n"
        "  # Indented comment line\n"
        "key2 = value2\n";
    create_temp_config_file(temp_config_filename_, content);
    Neurodeck::ConfigParser parser;
    ASSERT_TRUE(parser.load_file(temp_config_filename_));

    EXPECT_TRUE(parser.has_key("key1"));
    EXPECT_EQ(parser.get_string("key1"), "value1");
    EXPECT_TRUE(parser.has_key("key2"));
    EXPECT_EQ(parser.get_string("key2"), "value2");
    EXPECT_FALSE(parser.has_key("# Full comment line 1"));
    EXPECT_FALSE(parser.has_key("  # Indented comment line"));
}

TEST_F(ConfigParserTest, TestInlineComments) {
    const std::string content = 
        "setting1 = true # enable feature X\n"
        "setting2 = false    # disable feature Y   \n"
        "path = /usr/local/bin # installation path\n"
        "equals_in_value = first_part=second_part # comment after equals in value";
    create_temp_config_file(temp_config_filename_, content);
    Neurodeck::ConfigParser parser;
    ASSERT_TRUE(parser.load_file(temp_config_filename_));

    EXPECT_EQ(parser.get_string("setting1"), "true");
    EXPECT_EQ(parser.get_string("setting2"), "false");
    EXPECT_EQ(parser.get_string("path"), "/usr/local/bin");
    EXPECT_EQ(parser.get_string("equals_in_value"), "first_part=second_part");
}

TEST_F(ConfigParserTest, TestKeyOverwriting) {
    const std::string content =
        "mykey = original_value\n"
        "mykey = new_value\n"
        "mykey = final_value";
    create_temp_config_file(temp_config_filename_, content);
    Neurodeck::ConfigParser parser;
    ASSERT_TRUE(parser.load_file(temp_config_filename_));

    EXPECT_EQ(parser.get_string("mykey"), "final_value");
}

TEST_F(ConfigParserTest, TestCaseSensitivity) {
    const std::string content =
        "Key = Value1\n"
        "key = value2";
    create_temp_config_file(temp_config_filename_, content);
    Neurodeck::ConfigParser parser;
    ASSERT_TRUE(parser.load_file(temp_config_filename_));

    EXPECT_EQ(parser.get_string("Key"), "Value1");
    EXPECT_EQ(parser.get_string("key"), "value2");
    EXPECT_TRUE(parser.has_key("Key"));
    EXPECT_TRUE(parser.has_key("key"));
}

TEST_F(ConfigParserTest, TestWhitespaceHandling) {
    const std::string content =
        "  spaced_key  =  value_with_spaces  \n"
        "key_no_spaces=value_no_spaces\n"
        "tab_key\t=\tvalue_with_tabs\t\n"
        "trail_space_val = val_with_trail #comment";
    create_temp_config_file(temp_config_filename_, content);
    Neurodeck::ConfigParser parser;
    ASSERT_TRUE(parser.load_file(temp_config_filename_));

    EXPECT_TRUE(parser.has_key("spaced_key"));
    EXPECT_EQ(parser.get_string("spaced_key"), "value_with_spaces");

    EXPECT_TRUE(parser.has_key("key_no_spaces"));
    EXPECT_EQ(parser.get_string("key_no_spaces"), "value_no_spaces");
    
    EXPECT_TRUE(parser.has_key("tab_key"));
    EXPECT_EQ(parser.get_string("tab_key"), "value_with_tabs");

    EXPECT_TRUE(parser.has_key("trail_space_val"));
    EXPECT_EQ(parser.get_string("trail_space_val"), "val_with_trail");
}

TEST_F(ConfigParserTest, TestConstructorLoad) {
    const std::string content = "test_key = constructor_loaded";
    create_temp_config_file(temp_config_filename_, content);
    
    Neurodeck::ConfigParser parser(temp_config_filename_);
    EXPECT_TRUE(parser.has_key("test_key"));
    EXPECT_EQ(parser.get_string("test_key"), "constructor_loaded");

    // Also test constructor with non-existent file
    Neurodeck::ConfigParser parser_non_existent("no_such_file.ini");
    EXPECT_FALSE(parser_non_existent.has_key("any_key"));
}

TEST_F(ConfigParserTest, TestEmptyFile) {
    create_temp_config_file(temp_config_filename_, "");
    Neurodeck::ConfigParser parser;
    EXPECT_TRUE(parser.load_file(temp_config_filename_)); // Should succeed
    EXPECT_FALSE(parser.has_key("any_key")); // No keys should be present
}

TEST_F(ConfigParserTest, TestMalformedLines) {
    const std::string content = 
        "key_valid = value_valid\n"
        "malformed_line_no_equals\n"
        "= malformed_starts_with_equals\n"
        "another_valid = stuff";
    create_temp_config_file(temp_config_filename_, content);
    Neurodeck::ConfigParser parser;
    ASSERT_TRUE(parser.load_file(temp_config_filename_));

    EXPECT_TRUE(parser.has_key("key_valid"));
    EXPECT_EQ(parser.get_string("key_valid"), "value_valid");
    EXPECT_TRUE(parser.has_key("another_valid"));
    EXPECT_EQ(parser.get_string("another_valid"), "stuff");

    EXPECT_FALSE(parser.has_key("malformed_line_no_equals"));
    EXPECT_FALSE(parser.has_key("")); // Key from "= malformed" would be empty after trim
}

TEST_F(ConfigParserTest, HandlesKeyOnlyLinesGracefully) {
    const std::string content =
        "key1=value1\n"
        "justkeyonly\n"
        "key2=value2";
    create_temp_config_file(temp_config_filename_, content);
    Neurodeck::ConfigParser parser;
    ASSERT_TRUE(parser.load_file(temp_config_filename_));

    EXPECT_TRUE(parser.has_key("key1"));
    EXPECT_EQ(parser.get_string("key1"), "value1");
    EXPECT_TRUE(parser.has_key("key2"));
    EXPECT_EQ(parser.get_string("key2"), "value2");
    EXPECT_FALSE(parser.has_key("justkeyonly")); // This line should be ignored
}


TEST_F(ConfigParserTest, TestSpecialCharactersInValues) {
    const std::string content = "special_chars = !@$%^&*()[]{};':\",.<>/?\\|`~";
    create_temp_config_file(temp_config_filename_, content);
    Neurodeck::ConfigParser parser;
    ASSERT_TRUE(parser.load_file(temp_config_filename_));
    EXPECT_EQ(parser.get_string("special_chars"), "!@$%^&*()[]{};':\",.<>/?\\|`~");
}

// Renamed and adjusted from TestIntegerOverflowAndUnderflow to match specific requirements
TEST_F(ConfigParserTest, GetIntHandlesOutOfIntRangeValues) { 
    // std::stoll is used internally by ConfigParser, which can parse long long.
    // This test verifies that if the parsed long long is outside int range, default is returned.
    long long ll_max_plus_1 = static_cast<long long>(std::numeric_limits<int>::max()) + 1LL;
    long long ll_min_minus_1 = static_cast<long long>(std::numeric_limits<int>::min()) - 1LL;

    std::string too_large_val_str = std::to_string(ll_max_plus_1);
    std::string too_small_val_str = std::to_string(ll_min_minus_1);
    const int default_int_val = 42;

    std::string content =
        "key_too_large=" + too_large_val_str + "\n"
        "key_too_small=" + too_small_val_str + "\n"
        "key_valid_int=123\n"
        "key_max_int=" + std::to_string(std::numeric_limits<int>::max()) + "\n"
        "key_min_int=" + std::to_string(std::numeric_limits<int>::min()) + "\n";

    create_temp_config_file(temp_config_filename_, content);
    Neurodeck::ConfigParser parser;
    ASSERT_TRUE(parser.load_file(temp_config_filename_));

    EXPECT_EQ(parser.get_int("key_too_large", default_int_val), default_int_val);
    EXPECT_EQ(parser.get_int("key_too_small", default_int_val), default_int_val);
    EXPECT_EQ(parser.get_int("key_valid_int", default_int_val), 123);
    EXPECT_EQ(parser.get_int("key_max_int", default_int_val), std::numeric_limits<int>::max());
    EXPECT_EQ(parser.get_int("key_min_int", default_int_val), std::numeric_limits<int>::min());
    
    // Test with a different default value to be sure
    const int other_default_val = -99;
    EXPECT_EQ(parser.get_int("key_too_large", other_default_val), other_default_val);
    EXPECT_EQ(parser.get_int("key_too_small", other_default_val), other_default_val);

}
