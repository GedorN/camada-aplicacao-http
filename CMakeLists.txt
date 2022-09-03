cmake_minimum_required(VERSION 3.14)
project(redes)
find_package( Boost REQUIRED COMPONENTS program_options regex )

set(CMAKE_CXX_STANDARD 20)

add_executable(redes server.cpp)
target_link_libraries(redes pthread ${Boost_REGEX_LIBRARY})