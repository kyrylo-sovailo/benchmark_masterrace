### Proving that C++ is not slow, it's just a skill issue

This is a daughter repository of [benchmark](https://github.com/kyrylo-sovailo/benchmark). It has several goals:
 - Finding out which parts of C and C++ are faster relative to each other.
 - Crafting the fastest program that solves the program from the mother repository specifically

### Strategy

The strategy is to write a macros-heavy piece of code and to measure the performance of all plausible macros options. The options (and their names):
 - Compiler:
    - `gcc` (CMP_GCC)
    - `clang` (CMP_LLVM)
    - `g++` (CMP_GPP)
    - `clang++` (CMP_LLVMPP)
 - Compiler options (TODO: combine fastest options):
    - `-O3 -DNDEBUG` (OPT_DEFAULT)
    - `-O3 -DNDEBUG -march=native` (OPT_NATIVE)
    - `-O3 -DNDEBUG -fno-rtti` (OPT_RTTI)
    - `-O3 -DNDEBUG -flto` (OPT_LTO)
    - `-O3 -DNDEBUG -fomit-frame-pointer` (OPT_FRAME)
    - `-m32 -O3 -DNDEBUG` (OPT_DEFAULT32)
    - `-m32 -O3 -DNDEBUG -march=native` (OPT_NATIVE32)
    - `-m32 -O3 -DNDEBUG -fno-rtti` (OPT_RTTI32)
    - `-m32 -O3 -DNDEBUG -flto` (OPT_LTO32)
    - `-m32 -O3 -DNDEBUG -fomit-frame-pointer` (OPT_FRAME32)
 - Vectors:
    - `std::vector` (VEC_CPP)
    - `struct { void *p; void *end; void *end_capacity; }` (VEC_POINTER)
    - `struct { void *p; size_t length; size_t end_capacity; }` (VEC_SIZE_T)
    - `struct { void *p; unsigned int length; unsigned int capacity; }` (VEC_UINT)
    - `struct { void *p; void *end; }` (VEC_POINTER_NO_CAPACITY)
    - `struct { void *p; size_t length; }` (VEC_SIZE_T_NO_CAPACITY)
    - `struct { unsigned int offset; unsigned int length; }` (VEC_UINT_OFFSET)
    - `struct { char p[N]; size_t length; }` (VEC_STATIC)
 - Allocator:
      - `std::allocator` (ALC_ALLOCATOR)
      - `new+delete` (ALC_NEW)
      - `new` (ALC_NEW_NO_DELETE)
      - `malloc+free` (ALC_MALLOC)
      - `malloc+realloc+free` (ALC_REALLOC)
      - `malloc` (ALC_MALLOC_NO_FREE)
      - Custom `mmap`-based allocation (ALC_MMAP)
      - Static allocation (ALC_STATIC)
      - Multithreading:
      - `std::thread` (MLT_NO)
      - `std::thread` (MLT_CPP)
      - `C11 thread` (MLT_C)
      - `pthread` (MLT_PTHREAD)
      - `clone` (MLT_CLONE)
 - I/O:
    - `std::ifstream` + `std::sstream`, line-by-line parsing (IO_CPP_LINE)
    - `std::ifstream` + error handling (IO_CPP_ERROR)
    - `std::ifstream` + state machine (IO_CPP_STATE)
    - `fread` + `sscanf`, line-by-line parsing (IO_C_LINE)
    - `fscanf` + error handling (IO_C_ERROR)
    - `fread` + state machine (IO_C_STATE)
    - `read` + state machine (IO_READ_STATE)
    - `mmap` + `sscanf` (IO_MMAP_ERROR)
    - `mmap` + state machine (IO_MMAP_STATE)
  - `libc`/`libc++` usage:
    - Allowed (LC_YES)
    - Not allowed (LC_NO)
 - Prior knowledge about the problem:
    - Allowed (PK_YES)
    - Not allowed (PK_NO)

### Hardware

The measurements were performed on Intel Pentium 4415U. To reduce noise, the kernel was booted with `isolcpus=1,3`, where 1 and 3 are logical cores that correspond to physical core 1.

### Findings

All of the above mentioned options are not implemented yet, but some conclusions can be made:
 - `struct { T *p; size_t length; size_t capacity; }` are the fastest
 - `-march=native` marginally decreases performance
 - `-fno-rtti` improves performance
 - `realloc` doesn't grant any advantage over `malloc` or `new`
 - The I/O of `C++` if faster
 - 32-bit programs are slower than 64-bit

### Software
```
GCC      14.2.1
```