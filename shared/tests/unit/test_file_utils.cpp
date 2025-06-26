/**
 * Unit Tests for File Utilities Component
 *
 * Tests file operations, path manipulation, and utility functions.
 */

#include <gtest/gtest.h>
#include <filesystem>
#include "../utils/test_helpers.h"
#include "utilities/file_utils.h"

class FileUtilsTest : public ::testing::Test {
   protected:
    void SetUp() override {
        // Create temporary test directory
        test_dir = std::filesystem::temp_directory_path() / "shared_components_test";
        std::filesystem::create_directories(test_dir);
    }

    void TearDown() override {
        // Clean up test directory
        if (std::filesystem::exists(test_dir)) {
            std::filesystem::remove_all(test_dir);
        }
    }

    std::filesystem::path test_dir;
};

// ===== File Existence Tests =====

TEST_F(FileUtilsTest, FileExistsTests) {
    std::string test_file = CreateTempTestFile("test_exists.txt", "test content");
    ASSERT_FALSE(test_file.empty());

    EXPECT_TRUE(file_exists(test_file.c_str()));
    EXPECT_FALSE(file_exists("nonexistent_file.txt"));
    EXPECT_FALSE(file_exists(""));

    RemoveTempTestFile(test_file);
    EXPECT_FALSE(file_exists(test_file.c_str()));
}

TEST_F(FileUtilsTest, DirectoryExistsTests) {
    EXPECT_TRUE(dir_exists(test_dir.string().c_str()));
    EXPECT_FALSE(dir_exists("nonexistent_directory"));
    EXPECT_FALSE(dir_exists(""));
}

// ===== File Size Tests =====

TEST_F(FileUtilsTest, FileSizeTests) {
    std::string content = "Hello, World!";
    std::string test_file = CreateTempTestFile("size_test.txt", content.c_str());
    ASSERT_FALSE(test_file.empty());

    long size = file_get_size(test_file.c_str());
    EXPECT_EQ(static_cast<long>(content.length()), size);

    EXPECT_EQ(-1, file_get_size("nonexistent.txt"));

    RemoveTempTestFile(test_file);
}

// ===== Path Manipulation Tests =====

TEST_F(FileUtilsTest, FilenameExtraction) {
    char filename[MAX_FILENAME_LENGTH];

    EXPECT_TRUE(file_get_filename("/path/to/file.txt", filename, sizeof(filename)));
    EXPECT_STREQ("file.txt", filename);

    EXPECT_TRUE(file_get_filename("simple.txt", filename, sizeof(filename)));
    EXPECT_STREQ("simple.txt", filename);

    EXPECT_TRUE(file_get_filename("/path/to/directory/", filename, sizeof(filename)));
    EXPECT_STREQ("", filename);  // Directory path

    // Test buffer too small
    char small_buffer[5];
    EXPECT_FALSE(file_get_filename("very_long_filename.txt", small_buffer, sizeof(small_buffer)));
}

TEST_F(FileUtilsTest, DirectoryExtraction) {
    char directory[MAX_PATH_LENGTH];

    EXPECT_TRUE(file_get_directory("/path/to/file.txt", directory, sizeof(directory)));
    EXPECT_STREQ("/path/to", directory);

    EXPECT_TRUE(file_get_directory("simple.txt", directory, sizeof(directory)));
    EXPECT_STREQ(".", directory);  // Current directory
}

TEST_F(FileUtilsTest, ExtensionExtraction) {
    char extension[64];

    EXPECT_TRUE(file_get_extension("file.txt", extension, sizeof(extension)));
    EXPECT_STREQ("txt", extension);

    EXPECT_TRUE(file_get_extension("archive.tar.gz", extension, sizeof(extension)));
    EXPECT_STREQ("gz", extension);  // Last extension

    EXPECT_FALSE(file_get_extension("no_extension", extension, sizeof(extension)));
    EXPECT_FALSE(file_get_extension("", extension, sizeof(extension)));
}

TEST_F(FileUtilsTest, ExtensionChange) {
    char output[MAX_PATH_LENGTH];

    EXPECT_TRUE(file_change_extension("file.txt", "bak", output, sizeof(output)));
    EXPECT_STREQ("file.bak", output);

    EXPECT_TRUE(file_change_extension("path/to/file.old", "new", output, sizeof(output)));
    EXPECT_STREQ("path/to/file.new", output);

    EXPECT_TRUE(file_change_extension("no_extension", "txt", output, sizeof(output)));
    EXPECT_STREQ("no_extension.txt", output);
}

TEST_F(FileUtilsTest, PathJoining) {
    char output[MAX_PATH_LENGTH];

    EXPECT_TRUE(file_join_path("/path/to", "file.txt", output, sizeof(output)));
    EXPECT_TRUE(strstr(output, "file.txt") != nullptr);

    EXPECT_TRUE(file_join_path("relative", "file.txt", output, sizeof(output)));
    EXPECT_TRUE(strstr(output, "file.txt") != nullptr);
}

// ===== File Operations Tests =====

TEST_F(FileUtilsTest, FileCopy) {
    std::string src_content = "Source file content";
    std::string src_file = CreateTempTestFile("source.txt", src_content.c_str());
    ASSERT_FALSE(src_file.empty());

    std::filesystem::path dest_path = test_dir / "destination.txt";
    std::string dest_file = dest_path.string();

    EXPECT_TRUE(file_copy(src_file.c_str(), dest_file.c_str()));
    EXPECT_TRUE(file_exists(dest_file.c_str()));

    // Verify content
    EXPECT_EQ(file_get_size(src_file.c_str()), file_get_size(dest_file.c_str()));

    RemoveTempTestFile(src_file);
}

TEST_F(FileUtilsTest, FileBackup) {
    std::string content = "Original content";
    std::string test_file = CreateTempTestFile("backup_test.txt", content.c_str());
    ASSERT_FALSE(test_file.empty());

    EXPECT_TRUE(file_create_backup(test_file.c_str()));

    std::string backup_file = test_file + ".bak";
    EXPECT_TRUE(file_exists(backup_file.c_str()));

    RemoveTempTestFile(test_file);
    RemoveTempTestFile(backup_file);
}

// ===== Validation Tests =====

TEST_F(FileUtilsTest, FilenameValidation) {
    EXPECT_TRUE(file_is_valid_filename("valid_file.txt"));
    EXPECT_TRUE(file_is_valid_filename("file123.ext"));
    EXPECT_TRUE(file_is_valid_filename("my-file_name.txt"));

    // Invalid characters (platform dependent)
    EXPECT_FALSE(file_is_valid_filename("file|with|pipes.txt"));
    EXPECT_FALSE(file_is_valid_filename("file<with>brackets.txt"));
    EXPECT_FALSE(file_is_valid_filename(""));
}

TEST_F(FileUtilsTest, FilenameSanitization) {
    char output[MAX_FILENAME_LENGTH];

    EXPECT_TRUE(file_sanitize_filename("valid_file.txt", output, sizeof(output)));
    EXPECT_STREQ("valid_file.txt", output);

    EXPECT_TRUE(file_sanitize_filename("file|with|invalid<chars>.txt", output, sizeof(output)));
    // Should replace invalid characters
    EXPECT_TRUE(strlen(output) > 0);
    EXPECT_FALSE(strstr(output, "|"));
    EXPECT_FALSE(strstr(output, "<"));
    EXPECT_FALSE(strstr(output, ">"));
}

// ===== Error Handling Tests =====

TEST_F(FileUtilsTest, NullPointerHandling) {
    EXPECT_FALSE(file_exists(nullptr));
    EXPECT_FALSE(dir_exists(nullptr));
    EXPECT_EQ(-1, file_get_size(nullptr));
    EXPECT_FALSE(file_get_filename(nullptr, nullptr, 0));
    EXPECT_FALSE(file_copy(nullptr, nullptr));
}

TEST_F(FileUtilsTest, InvalidOperations) {
    EXPECT_FALSE(file_copy("nonexistent.txt", "destination.txt"));
    EXPECT_FALSE(file_create_backup("nonexistent.txt"));
}
