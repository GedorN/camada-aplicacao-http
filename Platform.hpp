// Header Guard
#ifndef PLATFORM_HPP
#define PLATFORM_HPP
#define UNICODE

#include <string>
#include <memory>

#if defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)
#define WINDOWS
#define STRINGTYPE std::wstring
#define COUT std::wcout
#elif defined(UNIX) || defined(__unix__) || defined(__unix) || \
    (defined(__APPLE__) && defined(__MACH__))
#define UNIX
#define STRINGTYPE std::string
#define COUT std::cout
#else
#define Unknown
#endif

enum class PlatformType { Windows, Unix, Mac, Unknown };

class Platform {
   protected:
    Platform(/* args */);
    inline static PlatformType type{};
    std::string bin_path{};

   public:
    virtual ~Platform() = default;
    void setup();
    virtual STRINGTYPE get_executable_path(std::string path = {}) = 0;
    virtual std::string utf16_to_utf8(const STRINGTYPE &wstr) { return {}; }
    virtual std::wstring utf8_to_utf16(const std::string &wstr) { return {}; }
    virtual std::string get_platform_string(const STRINGTYPE &str) {
        if (type == PlatformType::Windows) {
            return utf16_to_utf8(str);
        } else {
            return {reinterpret_cast<const char *>(str.c_str())};
        }
    }
    static std::shared_ptr<Platform> create_platform();
    static PlatformType get_platform_type() { return type; }
    const std::string &get_bin_path() const { return bin_path; }
};

#endif  // PLATFORM_HPP