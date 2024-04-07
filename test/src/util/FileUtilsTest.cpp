#include <gtest/gtest.h>
#include <cstdlib>
#include <fstream>
#include <optional>
#include "util/FileUtils.hpp"

TEST(FileUtilsTest, expand_home) {
    setenv("HOME", "/home/wall", 1);
    std::string path = "~/test";
    auto result = wall::FileUtils::expand_path(path);
    std::string expected = "/home/wall/test";
    EXPECT_EQ(result, expected);
}

TEST(FileUtilsTest, expand_runtime) {
    setenv("HOME", "/home/wall", 1);
    setenv("XDG_RUNTIME_DIR", "/run/user/1000/", 1);
    auto result = wall::FileUtils::get_default_runtime_dir();
    std::string expected = "/run/user/1000/wallock";
    EXPECT_EQ(result, expected);

    auto result1 = wall::FileUtils::expand_path("$XDG_RUNTIME_DIR/wallock1");
    expected = "/run/user/1000/wallock1";
    EXPECT_EQ(result1, expected);

    unsetenv("XDG_RUNTIME_DIR");
    result = wall::FileUtils::get_default_runtime_dir();
    expected = "/tmp/wallock";
    EXPECT_EQ(result, expected);
}

TEST(FileUtilsTest, expand_data_dir) {
    setenv("HOME", "/home/wall", 1);
    setenv("XDG_DATA_HOME", "/home/wall/test", 1);
    auto result = wall::FileUtils::get_default_data_dir();
    std::string expected = "/home/wall/test/wallock";
    EXPECT_EQ(result, expected);

    unsetenv("XDG_DATA_HOME");
    expected = "/home/wall/.local/share/wallock";
    result = wall::FileUtils::get_default_data_dir();
    EXPECT_EQ(result, expected);

    unsetenv("HOME");
    unsetenv("XDG_DATA_HOME");
    result = wall::FileUtils::get_default_data_dir();
    expected = "/tmp/wallock";
    EXPECT_EQ(result, expected);
}

static auto fileutilstest_touch(const std::string& path) -> void {
    std::ofstream file(path);
    file << "test";
    file.close();
}

TEST(FileUtilsTest, expand_config_not_found) {
    setenv("HOME", "/tmp/wall_test/not_found", 1);
    setenv("XDG_CONFIG_HOME", "/tmp/wall_test/not_found/config", 1);

    std::filesystem::create_directories("/tmp/wall_test/not_found/config");
    std::filesystem::create_directories("/tmp/wall_test/not_found/.config");

    std::string file = "config";
    std::string expected = "/tmp/wall_test/config/config";

    std::error_code err_code;
    std::filesystem::remove(expected, err_code);

    auto result = wall::FileUtils::get_expansion_config(expected);
    EXPECT_EQ(result, expected) << "Expected: " << expected << " Got: " << result.value_or("");

    result = wall::FileUtils::get_expansion_config(file);
    EXPECT_FALSE(result.has_value());

    unsetenv("XDG_CONFIG_HOME");
    result = wall::FileUtils::get_expansion_config(file);
    EXPECT_FALSE(result.has_value());

    unsetenv("HOME");
    result = wall::FileUtils::get_expansion_config(file);
    EXPECT_FALSE(result.has_value());
}

TEST(FileUtilsTest, expand_config) {
    setenv("HOME", "/tmp/wall_test", 1);
    setenv("XDG_CONFIG_HOME", "/tmp/wall_test/config", 1);

    std::filesystem::create_directories("/tmp/wall_test/config/wallock");
    std::filesystem::create_directories("/tmp/wall_test/.config/wallock");

    std::string file = "config";
    std::string expected = "/tmp/wall_test/test_config";
    fileutilstest_touch(expected);

    auto result = wall::FileUtils::get_expansion_config(expected);
    EXPECT_EQ(result, expected) << "Expected: " << expected << " Got: " << result.value_or("");

    expected = "/tmp/wall_test/.config/wallock/config";
    fileutilstest_touch(expected);
    result = wall::FileUtils::get_expansion_config(file);
    EXPECT_EQ(result, expected) << "Expected: " << expected << " Got: " << result.value_or("");

    unsetenv("XDG_CONFIG_HOME");
    expected = "/tmp/wall_test/.config/wallock/config";
    result = wall::FileUtils::get_expansion_config(file);
    EXPECT_EQ(result, expected) << "Expected: " << expected << " Got: " << result.value_or("");
}

TEST(FileUtilsTest, expand_data) {
    setenv("HOME", "/tmp/wall_test", 1);
    setenv("XDG_DATA_HOME", "/tmp/wall_test/data", 1);

    std::filesystem::create_directories("/tmp/wall_test/data/wallock");
    std::filesystem::create_directories("/tmp/wall_test/.local/share/wallock");

    std::string file = "some_file";
    std::string expected = "/tmp/wall_test/data1";
    fileutilstest_touch(expected);

    auto result = wall::FileUtils::get_expansion_data(expected);
    EXPECT_EQ(result, expected) << "Expected: " << expected << " Got: " << result.value_or("");

    expected = "/tmp/wall_test/data/wallock/some_file";
    fileutilstest_touch(expected);
    result = wall::FileUtils::get_expansion_data(file);
    EXPECT_EQ(result, expected) << "Expected: " << expected << " Got: " << result.value_or("");

    unsetenv("XDG_DATA_HOME");
    expected = "/tmp/wall_test/.local/share/wallock/some_file";
    result = wall::FileUtils::get_expansion_data(file);
    EXPECT_EQ(result, expected) << "Expected: " << expected << " Got: " << result.value_or("");
}

TEST(FileUtilsTest, all_files) {
    auto result = wall::FileUtils::get_all_files("/tmp/wall_test/does_not_exist");
    EXPECT_TRUE(result.empty());

    const auto base_dir = std::string{"/tmp/wall_test_all_files"};
    std::filesystem::remove_all(base_dir);
    std::filesystem::create_directories(base_dir);
    result = wall::FileUtils::get_all_files(base_dir);
    EXPECT_TRUE(result.empty());

    result = wall::FileUtils::get_all_files("$SOME_UNKNOWN_ENV_VAR/some_dir");
    EXPECT_TRUE(result.empty());

    std::string file = base_dir + "/some_file";
    fileutilstest_touch(file);

    result = wall::FileUtils::get_all_files(file);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], file);

    result = wall::FileUtils::get_all_files(base_dir);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], file);

    std::filesystem::create_directories(base_dir + "/some_dir");
    fileutilstest_touch(base_dir + "/some_dir/some_file");
    result = wall::FileUtils::get_all_files(base_dir);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], file);

    std::string file2 = base_dir + "/some_other_file";
    fileutilstest_touch(file2);
    result = wall::FileUtils::get_all_files(base_dir);
    ASSERT_EQ(result.size(), 2);
    std::sort(result.begin(), result.end());
    EXPECT_EQ(result[0], file);
    EXPECT_EQ(result[1], file2);

    std::string file3 = base_dir + "/file.txt";
    fileutilstest_touch(file3);
    result = wall::FileUtils::get_all_files(base_dir, {".txt"});
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], file3);
}
