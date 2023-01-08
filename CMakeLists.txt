cmake_minimum_required(VERSION 3.4...3.18)
project(connectlib LANGUAGES CXX)


add_subdirectory(extern/pybind11/)
pybind11_add_module(connectlib connectpy/connectlib.cpp)

set_target_properties(connectlib PROPERTIES
                                 LIBRARY_OUTPUT_DIRECTORY connectpy)
