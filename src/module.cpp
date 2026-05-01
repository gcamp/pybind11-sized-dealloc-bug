#include <pybind11/pybind11.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unordered_map>

namespace py = pybind11;

static std::unordered_map<void *, size_t> alloc_tracker;

struct Animal {
    virtual ~Animal() = default;
    virtual std::string speak() const { return "..."; }
    int value = 42;

    static void *operator new(size_t s) {
        void *p = ::operator new(s);
        alloc_tracker[p] = s;
        return p;
    }

    // Defining ONLY sized operator delete suppresses the implicit unsized one.
    // This forces pybind11's call_operator_delete to select the
    // has_operator_delete_size<T> overload, which forwards type_size as `s`.
    static void operator delete(void *p, size_t s) {
        auto it = alloc_tracker.find(p);
        if (it != alloc_tracker.end()) {
            size_t allocated = it->second;
            alloc_tracker.erase(it);
            if (allocated != s) {
                std::fprintf(stderr,
                             "\n*** BUG CONFIRMED ***\n"
                             "operator delete called with s=%zu but allocation was %zu bytes\n"
                             "pybind11 passed sizeof(trampoline) instead of sizeof(actual type)\n",
                             s, allocated);
                std::fflush(stderr);
                std::abort();
            }
        }
        ::operator delete(p);
    }
};

// Trampoline — significantly larger than Animal.
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

    py::class_<Animal, PyAnimal>(m, "Animal")
        .def(py::init([]() -> Animal * { return new Animal(); }))
        .def("speak", &Animal::speak)
        .def_readwrite("value", &Animal::value);
}
