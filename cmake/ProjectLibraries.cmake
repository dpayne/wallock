CPMAddPackage("gh:TheLartians/Format.cmake@${cmake_format_version}")
CPMAddPackage("gh:gabime/spdlog@${spdlog_version}")

CPMAddPackage(
  GITHUB_REPOSITORY jarro2783/cxxopts
  VERSION ${cxxopts_version}
  OPTIONS "CXXOPTS_BUILD_EXAMPLES NO" "CXXOPTS_BUILD_TESTS NO" "CXXOPTS_ENABLE_INSTALL NO"
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(xkbcommon REQUIRED IMPORTED_TARGET xkbcommon)
pkg_check_modules(mpv REQUIRED IMPORTED_TARGET mpv)
pkg_check_modules(drm REQUIRED IMPORTED_TARGET libdrm)
pkg_check_modules(udev REQUIRED IMPORTED_TARGET libudev)
pkg_check_modules(cairo REQUIRED IMPORTED_TARGET cairo fontconfig)
pkg_check_modules(gdk-pixbuf REQUIRED IMPORTED_TARGET gdk-pixbuf-2.0)

find_package(PAM REQUIRED)

set(PROJECT_HEADER_LIBRARIES spdlog)
set(PROJECT_LIBRARIES
    cxxopts
    PkgConfig::xkbcommon
    PkgConfig::mpv
    PkgConfig::drm
    PkgConfig::cairo
    PkgConfig::udev
    PkgConfig::gdk-pixbuf
    ${PAM_LIBRARIES}
)
