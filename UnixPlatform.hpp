#ifndef UNIXPLATFORM_HPP
#define UNIXPLATFORM_HPP

#include "Platform.hpp"
#include <libgen.h>
#include <unistd.h>
#include <linux/limits.h>

class UnixPlatform : public Platform {
    friend Platform;

   protected:
   private:
   public:
    std::string get_executable_path(std::string path = {}) override;
    UnixPlatform(/* args */){};
    ~UnixPlatform(){};
};

#endif  // UNIXPLATFORM_HPP