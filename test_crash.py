"""Reproducer for pybind11 sized deallocation bug.

When __cpp_sized_deallocation is active (clang 14+ with C++14+), pybind11's
call_operator_delete passes sizeof(Base) to ::operator delete(p, s), but the
object was allocated as a Derived (much larger). This corrupts glibc allocator
metadata and crashes with "double free or corruption (out)".
"""

import gc
import crash_module


def main():
    for cycle in range(200):
        objects = [crash_module.Base() for _ in range(500)]
        del objects
        gc.collect()
        if (cycle + 1) % 50 == 0:
            print(f"Cycle {cycle + 1}/200 completed")

    print("All cycles completed without crash (bug may not have triggered)")


if __name__ == "__main__":
    main()
