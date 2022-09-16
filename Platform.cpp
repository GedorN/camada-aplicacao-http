#include "Platform.hpp"
#include <iostream>

#if defined(WINDOWS)
#include "WindowsPlatform.hpp"
#elif defined(UNIX)
#include "UnixPlatform.hpp"
#endif

Platform::Platform(/* args */) { std::cout << "Platform Created" << std::endl; }

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