#include <pybind11/pybind11.h>
#include <cstdio>
#include <string>

namespace py = pybind11;

struct Animal {
    virtual ~Animal() = default;
    virtual std::string speak() const { return "..."; }
    int id = 0;
};

// Trampoline (alias) class — significantly larger than Animal.
// pybind11 sets type_size = sizeof(PyAnimal) when has_alias=true.
struct PyAnimal : Animal {
    using Animal::Animal;
    std::string speak() const override {
        PYBIND11_OVERRIDE(std::string, Animal, speak);
    }
    char _padding[512]{};
};

PYBIND11_MODULE(crash_module, m) {
    std::fprintf(stderr, "sizeof(Animal)=%zu  sizeof(PyAnimal)=%zu\n",
                 sizeof(Animal), sizeof(PyAnimal));

    // has_alias=true → type_size = sizeof(PyAnimal)
    py::class_<Animal, PyAnimal>(m, "Animal")
        // Factory returns Animal* allocated with new Animal() → sizeof(Animal) bytes.
        // Holder is NOT constructed for raw-pointer factories.
        // dealloc's else-branch calls:
        //   call_operator_delete(p, sizeof(PyAnimal), alignof(PyAnimal))
        // With __cpp_sized_deallocation this becomes:
        //   ::operator delete(p, sizeof(PyAnimal))
        // but the allocation was only sizeof(Animal) → size mismatch → glibc crash.
        .def(py::init([]() -> Animal * { return new Animal(); }))
        .def("speak", &Animal::speak)
        .def_readwrite("id", &Animal::id);
}
