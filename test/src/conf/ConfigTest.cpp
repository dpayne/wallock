#include <gtest/gtest.h>

#include "conf/Config.hpp"

class ConfigMock : public wall::Config {
   public:
    ConfigMock() : wall::Config(0, nullptr) {}
};

TEST(Conf, color_scheme) {
    auto conf = wall::Config::get_default_config();
    EXPECT_EQ(std::string{wall_conf_get(conf, color, background)}, std::string{"#000000"});
    EXPECT_EQ(std::string{wall_conf_get(conf, background, color)}, std::string{"#000000C0"});
    EXPECT_EQ(std::string{wall_conf_get(conf, font, color)}, std::string{"#FFFFFFFF"});
}

TEST(Conf, get_color) {
    auto conf = wall::Config::get_default_config();
    EXPECT_EQ(conf.get_color_name("{color0}"), "#000000");
    EXPECT_EQ(conf.get_color_name(" {color0} "), "#000000");
    EXPECT_EQ(conf.get_color_name("{color0}FF"), "#000000FF");
    EXPECT_EQ(conf.get_color_name("{background}C0"), "#000000C0");
    EXPECT_EQ(conf.get_color_name("{foreground}"), "#FFFFFF");
}
