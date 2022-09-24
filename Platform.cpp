#include "Platform.hpp"
#include <boost/algorithm/string.hpp>
#include <iostream>

#if defined(WINDOWS)
#include "WindowsPlatform.hpp"
#elif defined(UNIX)
#include "UnixPlatform.hpp"
#endif

Platform::Platform(/* args */) {}

std::shared_ptr<Platform> Platform::create_platform() {
#if defined(WINDOWS)
    type = PlatformType::Windows;
    return std::make_shared<WindowsPlatform>();
#elif defined(UNIX)
    type = PlatformType::Unix;
    return std::make_shared<UnixPlatform>();
#else
    return nullptr;
#endif
}

void Platform::setup() {
    auto bin_path = get_executable_path();
    std::string utf8_bin_path = get_platform_string(bin_path);
    boost::trim(utf8_bin_path);
    this->bin_path = utf8_bin_path.data();

    std::cout << "Setup Completes. - Process: " << this->bin_path << std::endl;
}