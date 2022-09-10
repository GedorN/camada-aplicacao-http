#ifndef WINDOWSPLATFORM_HPP
#define WINDOWSPLATFORM_HPP
#include "Platform.hpp"
#include <windows.h>
#include <wingdi.h>

class WindowsPlatform : public Platform {
    friend Platform;

   protected:
   private:
   public:
    STRINGTYPE get_executable_path(std::string path = {}) override;
    std::string utf16_to_utf8(const std::wstring &wstr) override;
    WindowsPlatform(/* args */){};
    ~WindowsPlatform(){};
};

#endif  // WINDOWSPLATFORM_HPP