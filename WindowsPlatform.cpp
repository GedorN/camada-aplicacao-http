#include "WindowsPlatform.hpp"

STRINGTYPE WindowsPlatform::get_executable_path(std::string path) {
    wchar_t path_buffer[300];
    auto module_name =
        GetModuleFileName(nullptr, path_buffer, _countof(path_buffer));

    return {path_buffer};
}

std::string WindowsPlatform::utf16_to_utf8(const std::wstring &wstr) {
    std::string str;
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0,
                                   nullptr, nullptr);
    if (size > 0) {
        str.resize(size);
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], size,
                            nullptr, nullptr);
    }
    return str;
}