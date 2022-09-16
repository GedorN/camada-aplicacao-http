cmake_minimum_required(VERSION 3.14)
project(redes)

# find_package(Boost REQUIRED COMPONENTS program_options regex)
set(CMAKE_CXX_STANDARD 20)

if(WIN32 OR MSVC)
    set(PlatformCPP
        WindowsPlatform.cpp
    )
endif()

if(UNIX)
    find_package(Boost 1.80 REQUIRED COMPONENTS program_options regex json)
    set(PlatformCPP
        UnixPlatform.cpp
    )
endif()

add_executable(redes server.cpp Platform.cpp ${PlatformCPP} RequestHandler.cpp)

if(UNIX)
    target_link_libraries(redes pthread ${Boost_LIBRARIES})
endif()

# target_link_libraries(redes pthread ${Boost_REGEX_LIBRARY})