"""Reproducer for pybind11 sized deallocation bug.

Animal defines only operator delete(void*, size_t) (no unsized overload).
This forces pybind11's call_operator_delete to use the has_operator_delete_size
overload, which passes type_size as the size argument.

With a trampoline (PyAnimal), type_size = sizeof(PyAnimal) but the factory
allocates sizeof(Animal). The custom operator delete detects the mismatch.
"""

import gc
import sys
import crash_module


def main():
    print(f"Python {sys.version}", flush=True)

    obj = crash_module.Animal()
    print(f"Created Animal with value={obj.value}", flush=True)
    del obj
    gc.collect()
    print("First object deallocated — if we got here, bug was NOT triggered")

    for cycle in range(200):
        objects = [crash_module.Animal() for _ in range(100)]
        del objects
        gc.collect()

    print("All cycles completed — bug was NOT triggered")


if __name__ == "__main__":
    main()
