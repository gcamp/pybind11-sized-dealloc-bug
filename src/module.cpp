#include <pybind11/pybind11.h>

namespace py = pybind11;

struct Base {
    virtual ~Base() = default;
    int value = 42;
};

// Derived is significantly larger than Base.
// When pybind11 calls ::operator delete(p, sizeof(Base)) on a Derived*,
// the wrong size corrupts glibc's tcache/fastbin metadata.
struct Derived : Base {
    char payload[512]{};
};

PYBIND11_MODULE(crash_module, m) {
    m.doc() = "Reproducer for pybind11 sized deallocation bug";

    py::class_<Base>(m, "Base")
        // Factory returns a Derived* cast to Base*.
        // pybind11 takes ownership but records type_size = sizeof(Base).
        // On dealloc, it calls ::operator delete(p, sizeof(Base))
        // even though the allocation was sizeof(Derived).
        .def(py::init([]() -> Base * { return new Derived(); }))
        .def_readonly("value", &Base::value);
}
