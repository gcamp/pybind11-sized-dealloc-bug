"""Reproducer for pybind11 sized deallocation bug.

When __cpp_sized_deallocation is active, pybind11's call_operator_delete passes
sizeof(PyAnimal) (the trampoline) to ::operator delete(p, s), but the object was
allocated as Animal (much smaller). This corrupts glibc tcache metadata.
"""

import gc
import sys
import crash_module


def main():
    print(f"Python {sys.version}", flush=True)
    for cycle in range(500):
        objects = [crash_module.Animal() for _ in range(200)]
        for i, o in enumerate(objects):
            o.id = i
        del objects
        gc.collect()
        if (cycle + 1) % 100 == 0:
            print(f"Cycle {cycle + 1}/500 completed", flush=True)

    print("All cycles completed without crash (bug may not have triggered)")


if __name__ == "__main__":
    main()
