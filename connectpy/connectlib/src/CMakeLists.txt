cmake_minimum_required(VERSION 3.4...3.18)
project(cconnectlib LANGUAGES CXX)

add_subdirectory(extern/pybind11/)
pybind11_add_module(connectlib connectlib.cpp)
