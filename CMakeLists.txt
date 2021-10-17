cmake_minimum_required(VERSION 3.20)
project(AudioVisualizer2)

set(CMAKE_CXX_STANDARD 14)

add_executable(AudioVisualizer2 main.cpp)

set(SFML_DIR C:/Libs/SFML_MINGW/lib/cmake/SFML)
find_package(SFML COMPONENTS system window graphics audio REQUIRED)
include_directories(C:/Libs/SFML_MINGW/include)
target_link_libraries(AudioVisualizer2 sfml-system sfml-window sfml-graphics sfml-audio)
