cmake_minimum_required(VERSION 3.20)

project(HamArc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(lib)
add_subdirectory(app)
add_subdirectory(bin)

enable_testing()
add_subdirectory(tests)

cmake_minimum_required(VERSION 3.20)
project(HamArc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


