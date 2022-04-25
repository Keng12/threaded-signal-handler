cmake_minimum_required(VERSION 3.5)

project(sth LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(sth src/main.cpp)
target_sources(sth PRIVATE src/sighand.cpp)
