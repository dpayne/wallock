#pragma once

#define wall_conf_key__(name, setting_name, default_value, description) \
    static constexpr auto k_##name = setting_name;                      \
    static constexpr auto k_default_##name = default_value;             \
    static constexpr auto k_description_##name = description;

#define wall_conf_key_(group, name, setting_name, default_value, description) wall_conf_key__(group##name, setting_name, default_value, description)

#define wall_conf_key(group, name, default_value, description) wall_conf_key_(group, _##name, #group "_" #name, default_value, description)

#define wall_conf_key_nd(group, name, default_value) wall_conf_key_(group, _##name, #group "_" #name, default_value, "")

#define wall_conf_get__(settings, name) (settings).get(::wall::conf::k_##name, ::wall::conf::k_default_##name)

#define wall_conf_get_(settings, group, name) wall_conf_get__(settings, group##name)

#define wall_conf_get(settings, group, name) wall_conf_get_(settings, group, _##name)

#define wall_conf_get_with_fallback__(settings, name, name2) \
    (settings).get_with_fallback(::wall::conf::k_##name, ::wall::conf::k_##name2, ::wall::conf::k_default_##name)

#define wall_conf_get_with_fallback_(settings, group, name, group2, name1) wall_conf_get_with_fallback__(settings, group##name, group2##name1)

#define wall_conf_get_with_fallback(settings, group, name, group2, name2) wall_conf_get_with_fallback_(settings, group, _##name, group2, _##name2)

#define wall_conf_set__(name)                                                                                                                  \
    ::wall::conf::g_default_settings.emplace_back(std::string{::wall::conf::k_##name}, SettingsVariantType{(::wall::conf::k_default_##name)}); \
    ::wall::conf::g_description_settings[std::string{::wall::conf::k_##name}] = (::wall::conf::k_description_##name);

#define wall_conf_set_(group, name) wall_conf_set__(group##name)

#define wall_conf_set(group, name) wall_conf_set_(group, _##name)
