cmake_minimum_required(VERSION 3.16)
project(ecosystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(ecosystem_core src/Config.cpp src/Species.cpp src/World.cpp)
target_include_directories(ecosystem_core PUBLIC src third_party)
if(MSVC)
  target_compile_options(ecosystem_core PRIVATE /W4)
else()
  target_compile_options(ecosystem_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(ecosystem src/main.cpp)
target_link_libraries(ecosystem PRIVATE ecosystem_core)

enable_testing()
add_executable(test_world tests/test_world.cpp)
target_link_libraries(test_world PRIVATE ecosystem_core)
add_test(NAME test_world COMMAND test_world)
