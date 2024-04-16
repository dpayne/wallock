#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <array>
#include <fstream>
#include "conf/ConfigDefaultSettings.hpp"
#include "mpv/MpvFileLoader.hpp"
#include "mpv/MpvResource.hpp"
#include "mpv/MpvResourceConfig.hpp"

using mpv_cmd_arg = std::array<const char*, 4>;

TEST(MpvFileLoaderTest, test_file_load_empty_files) {
    auto config = wall::Config::get_default_config();
    config.set(wall::conf::k_file_path, "/tmp/doesnotexist");
    wall::Loop loop;
    wall::PrimaryDisplayState primary_state;

    wall::MpvResourceConfig resource_config = wall::MpvResourceConfig::build_config(config, wall::ResourceMode::Wallpaper);

    wall::MpvFileLoader loader(config, &loop, &resource_config, &primary_state, "",
                               [&](const std::string& file) { EXPECT_EQ(file, std::string{"/tmp/doesnotexist"}); });

    loader.load_next_file();
}

TEST(MpvFileLoaderTest, test_file_load_single_file) {
    auto config = wall::Config::get_default_config();
    config.set(wall::conf::k_file_path, "/tmp/test_1.png");
    std::vector<uint8_t> png_data = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52, 0x00, 0x00,
                                     0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0x15, 0xc4, 0x89, 0x00, 0x00, 0x00,
                                     0x0d, 0x49, 0x44, 0x41, 0x54, 0x08, 0x5b, 0x63, 0xf8, 0xbf, 0x94, 0xe1, 0x3f, 0x00, 0x06, 0xef, 0x02, 0xa4,
                                     0x97, 0x04, 0x3f, 0x6f, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82};
    wall::Loop loop;

    // write out png to tmp file
    std::ofstream file("/tmp/test_1.png");
    file.write(reinterpret_cast<const char*>(png_data.data()), png_data.size());

    wall::PrimaryDisplayState primary_state;

    wall::MpvResourceConfig resource_config = wall::MpvResourceConfig::build_config(config, wall::ResourceMode::Wallpaper);
    wall::MpvFileLoader loader(config, &loop, &resource_config, &primary_state, "",
                               [&](const std::string& file_loaded) { EXPECT_STREQ(file_loaded.c_str(), "/tmp/test_1.png"); });
    loader.load_options();
    loader.load_next_file();

    EXPECT_EQ(loader.get_current_file(), "/tmp/test_1.png");
    EXPECT_EQ(primary_state.m_wallpaper_files.size(), 0);

    loader.load_next_file();
    EXPECT_EQ(loader.get_current_file(), "/tmp/test_1.png");
    EXPECT_EQ(primary_state.m_wallpaper_files.size(), 0);
}

TEST(MpvFileLoaderTest, test_file_load_folder) {
    auto config = wall::Config::get_default_config();
    config.set(wall::conf::k_file_path, "/tmp/load_folder");
    config.set(wall::conf::k_file_sort_order, "alpha");
    std::vector<uint8_t> png_data = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52, 0x00, 0x00,
                                     0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0x15, 0xc4, 0x89, 0x00, 0x00, 0x00,
                                     0x0d, 0x49, 0x44, 0x41, 0x54, 0x08, 0x5b, 0x63, 0xf8, 0xbf, 0x94, 0xe1, 0x3f, 0x00, 0x06, 0xef, 0x02, 0xa4,
                                     0x97, 0x04, 0x3f, 0x6f, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82};

    std::filesystem::create_directory("/tmp/load_folder");

    // write out png to tmp file
    std::ofstream file("/tmp/load_folder/test_1.png");
    file.write(reinterpret_cast<const char*>(png_data.data()), png_data.size());
    file.close();

    std::ofstream file2("/tmp/load_folder/test_2.png");
    file.write(reinterpret_cast<const char*>(png_data.data()), png_data.size());
    file.close();

    std::ofstream file3("/tmp/load_folder/test_3.png");
    file.write(reinterpret_cast<const char*>(png_data.data()), png_data.size());
    file.close();

    wall::PrimaryDisplayState primary_state;
    wall::Loop loop;

    wall::MpvResourceConfig resource_config = wall::MpvResourceConfig::build_config(config, wall::ResourceMode::Wallpaper);
    wall::MpvFileLoader loader(config, &loop, &resource_config, &primary_state, "", [&](const std::string& /* file_loaded */) {});
    loader.load_options();
    loader.load_next_file();

    EXPECT_EQ(loader.get_current_file(), "/tmp/load_folder/test_1.png");
    EXPECT_EQ(primary_state.m_wallpaper_files.size(), 0);

    loader.load_next_file();
    EXPECT_EQ(loader.get_current_file(), "/tmp/load_folder/test_2.png");

    loader.load_next_file();
    EXPECT_EQ(loader.get_current_file(), "/tmp/load_folder/test_3.png");

    loader.load_next_file();
    EXPECT_EQ(loader.get_current_file(), "/tmp/load_folder/test_1.png");
}

TEST(MpvFileLoaderTest, global_order) {
    auto config = wall::Config::get_default_config();
    config.set(wall::conf::k_file_path, "/tmp/load_folder");
    config.set(wall::conf::k_file_sort_order, "alpha");
    config.set(wall::conf::k_file_keep_same_order, true);
    std::vector<uint8_t> png_data = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52, 0x00, 0x00,
                                     0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0x15, 0xc4, 0x89, 0x00, 0x00, 0x00,
                                     0x0d, 0x49, 0x44, 0x41, 0x54, 0x08, 0x5b, 0x63, 0xf8, 0xbf, 0x94, 0xe1, 0x3f, 0x00, 0x06, 0xef, 0x02, 0xa4,
                                     0x97, 0x04, 0x3f, 0x6f, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82};

    std::filesystem::create_directory("/tmp/load_folder");

    // write out png to tmp file
    std::ofstream file("/tmp/load_folder/test_1.png");
    file.write(reinterpret_cast<const char*>(png_data.data()), png_data.size());
    file.close();

    std::ofstream file2("/tmp/load_folder/test_2.png");
    file.write(reinterpret_cast<const char*>(png_data.data()), png_data.size());
    file.close();

    std::ofstream file3("/tmp/load_folder/test_3.png");
    file.write(reinterpret_cast<const char*>(png_data.data()), png_data.size());
    file.close();

    wall::PrimaryDisplayState primary_state;
    wall::Loop loop;

    wall::MpvResourceConfig resource_config = wall::MpvResourceConfig::build_config(config, wall::ResourceMode::Wallpaper);
    wall::MpvFileLoader loader(config, &loop, &resource_config, &primary_state, "", [&](const std::string& /* file_loaded */) {});
    loader.load_options();

    ASSERT_EQ(primary_state.m_wallpaper_files.size(), 3);
    EXPECT_EQ(primary_state.m_wallpaper_files[0], "/tmp/load_folder/test_1.png");
    EXPECT_EQ(primary_state.m_wallpaper_files[1], "/tmp/load_folder/test_2.png");
    EXPECT_EQ(primary_state.m_wallpaper_files[2], "/tmp/load_folder/test_3.png");
}

TEST(MpvFileLoaderTest, global_order_reshuffle) {
    auto config = wall::Config::get_default_config();
    config.set(wall::conf::k_file_path, "/tmp/load_folder");
    config.set(wall::conf::k_file_sort_order, "random");
    config.set(wall::conf::k_file_keep_same_order, true);
    std::vector<uint8_t> png_data = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52, 0x00, 0x00,
                                     0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0x15, 0xc4, 0x89, 0x00, 0x00, 0x00,
                                     0x0d, 0x49, 0x44, 0x41, 0x54, 0x08, 0x5b, 0x63, 0xf8, 0xbf, 0x94, 0xe1, 0x3f, 0x00, 0x06, 0xef, 0x02, 0xa4,
                                     0x97, 0x04, 0x3f, 0x6f, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82};

    std::filesystem::create_directory("/tmp/load_folder");

    // write out png to tmp file
    std::ofstream file("/tmp/load_folder/test_1.png");
    file.write(reinterpret_cast<const char*>(png_data.data()), png_data.size());
    file.close();

    std::ofstream file2("/tmp/load_folder/test_2.png");
    file.write(reinterpret_cast<const char*>(png_data.data()), png_data.size());
    file.close();

    std::ofstream file3("/tmp/load_folder/test_3.png");
    file.write(reinterpret_cast<const char*>(png_data.data()), png_data.size());
    file.close();

    wall::PrimaryDisplayState primary_state;
    wall::Loop loop;

    wall::MpvResourceConfig resource_config = wall::MpvResourceConfig::build_config(config, wall::ResourceMode::Wallpaper);
    wall::MpvFileLoader loader(config, &loop, &resource_config, &primary_state, "", [&](const std::string& /* file_loaded */) {});
    loader.load_options();

    ASSERT_EQ(primary_state.m_wallpaper_files.size(), 3);

    loader.load_next_file();
    loader.load_next_file();
    loader.load_next_file();
    loader.load_next_file();
    ASSERT_EQ(primary_state.m_wallpaper_files.size(), 3);
}

TEST(MpvFileLoaderTest, timer) {
    auto config = wall::Config::get_default_config();
    config.set(wall::conf::k_file_path, "/tmp/load_folder");
    config.set(wall::conf::k_file_sort_order, "random");
    config.set(wall::conf::k_file_keep_same_order, true);
    std::vector<uint8_t> png_data = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52, 0x00, 0x00,
                                     0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0x15, 0xc4, 0x89, 0x00, 0x00, 0x00,
                                     0x0d, 0x49, 0x44, 0x41, 0x54, 0x08, 0x5b, 0x63, 0xf8, 0xbf, 0x94, 0xe1, 0x3f, 0x00, 0x06, 0xef, 0x02, 0xa4,
                                     0x97, 0x04, 0x3f, 0x6f, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82};

    std::filesystem::create_directory("/tmp/load_folder");

    // write out png to tmp file
    std::ofstream file("/tmp/load_folder/test_1.png");
    file.write(reinterpret_cast<const char*>(png_data.data()), png_data.size());
    file.close();

    std::ofstream file2("/tmp/load_folder/test_2.png");
    file.write(reinterpret_cast<const char*>(png_data.data()), png_data.size());
    file.close();

    wall::PrimaryDisplayState primary_state;
    wall::Loop loop;
    wall::MpvFileLoader* loader_ptr = nullptr;
    bool timer_called = false;

    wall::MpvResourceConfig resource_config = wall::MpvResourceConfig::build_config(config, wall::ResourceMode::Wallpaper);
    wall::MpvFileLoader loader(config, &loop, &resource_config, &primary_state, "", [&](const std::string&) {
        loader_ptr->stop();
        timer_called = true;
    });
    loader_ptr = &loader;
    loader.load_options();
    loader.load_next_file();

    loader.setup_load_next_file_timer(1.0);

    loop.run();

    EXPECT_TRUE(timer_called);
}

TEST(MpvFileLoaderTest, test_calculate_timer_delay) {
    auto config = wall::Config::get_default_config();
    wall::MpvResourceConfig resource_config = wall::MpvResourceConfig::build_config(config, wall::ResourceMode::Wallpaper);
    resource_config.m_image_change_interval_secs = std::chrono::seconds{10};
    resource_config.m_video_max_change_interval_secs = std::chrono::seconds{20};
    resource_config.m_video_preload_secs = std::chrono::seconds{5};

    auto timer_delay = wall::MpvFileLoader::calculate_timer_delay(0.0, resource_config);
    EXPECT_EQ(timer_delay, std::chrono::seconds{5});

    timer_delay = wall::MpvFileLoader::calculate_timer_delay(15.0, resource_config);
    EXPECT_EQ(timer_delay, std::chrono::seconds{10});

    timer_delay = wall::MpvFileLoader::calculate_timer_delay(25.0, resource_config);
    EXPECT_EQ(timer_delay, std::chrono::seconds{15});

    timer_delay = wall::MpvFileLoader::calculate_timer_delay(3.0, resource_config);
    EXPECT_EQ(timer_delay, std::chrono::seconds{3});
}
