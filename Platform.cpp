#include "Platform.hpp"

#if defined(WINDOWS)
#include "WindowsPlatform.hpp"
#elif defined(UNIX)
#include "UnixPlatform.hpp"
#endif

std::unique_ptr<Platform> Platform::create_platform() {
#if defined(WINDOWS)
    return std::make_unique<WindowsPlatform>();
#elif defined(UNIX)
    return std::make_unique<UnixPlatform>();
#else
    return nullptr;
#endif
}