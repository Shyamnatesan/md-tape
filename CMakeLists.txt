cmake_minimum_required(VERSION 3.24)

project(md-tape LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_subdirectory(md-recorder)
add_subdirectory(md-replayer)
add_subdirectory(md-lib)
