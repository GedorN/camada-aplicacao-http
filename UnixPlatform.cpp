#include "UnixPlatform.hpp"

std::string UnixPlatform::get_executable_path(std::string path) {
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    const char *c_path;
    if (count != -1) {
        c_path = dirname(result);
    }

    return {c_path};
}
