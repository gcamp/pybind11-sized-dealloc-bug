# pybind11 Sized Deallocation Bug Reproducer

Reproducer for a bug in pybind11's `call_operator_delete` where sized deallocation (`::operator delete(p, s)`) receives the wrong size, corrupting glibc's allocator metadata.

## The Bug

When `__cpp_sized_deallocation` is defined (clang 14+ with C++14+), pybind11 calls `::operator delete(p, sizeof(RegisteredType))`. If the object was actually allocated as a larger derived class via a factory, the size is wrong and glibc crashes with **"double free or corruption (out)"**.

**Workaround:** build with `-fno-sized-deallocation`.

See: `src/module.cpp` for the minimal reproducer and `.github/workflows/reproduce.yml` for CI.
