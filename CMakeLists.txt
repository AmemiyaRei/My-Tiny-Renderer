cmake_minimum_required(VERSION 3.24)
project(TinyRenderer)

set(CMAKE_CXX_STANDARD 17)

add_executable(TinyRenderer main.cpp tgaimage.cpp tgaimage.h model.cpp model.h geometry.h
        geometry.cpp)
