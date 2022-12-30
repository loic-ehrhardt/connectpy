#include <pybind11/pybind11.h>
#include <iostream>

namespace py = pybind11;

int main() {
    std::cout << "Hello world" << std::endl;
    return 0;
}

int add(int i, int j) {
    return i + j;
}

PYBIND11_MODULE(connectlib, m) {
    m.doc() = "pybind11 example plugin"; // optional module docstring
    m.def("add", &add, "A function that adds two numbers");
}
