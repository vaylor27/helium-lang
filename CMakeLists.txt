cmake_minimum_required(VERSION 3.20)

project(helium)

set(CMAKE_CXX_STANDARD 20)

add_executable(helium src/Main.cpp
        src/Tokenization.cpp
        src/Parser.cpp
        src/Generation.cpp
        src/Arena.cpp
)