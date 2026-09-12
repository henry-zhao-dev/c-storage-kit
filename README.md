# c-storage-kit

c-storage-kit is a lightweight C library for generic storage abstractions. Its
current focus is `calist`, a dynamically resizable array with explicit value
ownership and a reusable type interface.

The project grew out of ideas I first encountered in **CS 136: Elementary
Algorithm Design and Data Abstraction** at the University of Waterloo. That
course introduced me to the discipline of designing abstract data types in C:
separating an interface from its implementation, defining invariants, and
being deliberate about memory. c-storage-kit is an ongoing application of
those ideas in a small library intended to be useful beyond a single course
exercise.

## Current scope

The library currently provides two closely related components:

- `ctype` describes a value type through its size and callbacks for copying,
  destruction, comparison, and printing. Built-in descriptors are provided for
  common C types, including integers, floating-point values, booleans, and
  strings.
- `calist` is a generic dynamic array. It supports indexed access, insertion,
  batch operations, replacement, removal, filtering, slicing, deduplication,
  sorting, searching, copying, and equality checks.

The implementation has no third-party runtime dependencies and is built as a
small static library. The public API is deliberately centered on ordinary C
values and pointers, so it can be included in another CMake project without
introducing a large framework.

## Ownership and API behavior

`calist` copies every value through the associated `ctype` when that value is
inserted. The list owns the copy and destroys it when the value is removed or
when the list itself is destroyed. The caller remains responsible for the
original object passed to the API.

Pointers returned by `calist_get` and `calist_get_mutable` are borrowed. They
must not be freed by the caller and should not be used after a structural
change to the list, such as insertion, removal, or reallocation.

The current API treats invalid use as a programming error:

- Index-based access, replacement, and removal require an index within the
  current list bounds. Insertion accepts an index from `0` through the current
  size, inclusive.
- Null pointers and incompatible type descriptors violate API preconditions.
- Precondition violations call `abort()` with a diagnostic.
- Allocation and duplication failures terminate the process with failure.

This behavior is intentional in the current design. The API does not expose
recoverable error codes for programmer errors or allocation failures, which
keeps normal container operations compact while making misuse fail immediately.

## A small example

```c
#include <assert.h>

#include "calist.h"

int main(void) {
  calist *numbers = calist_create(ctype_int());

  calist_append(numbers, WRAP_INT(30));
  calist_append(numbers, WRAP_INT(10));
  calist_append(numbers, WRAP_INT(20));

  calist_qsort(numbers);
  assert(*(const int *)calist_get(numbers, 0) == 10);

  calist_destroy(numbers);
  return 0;
}
```

The list copies each integer when it is added, so the temporary values are
safe to use and the list cleans up its own storage when it is destroyed.

## Build and test

The standalone project uses CMake 3.21 or newer:

```sh
cmake -S . -B build-cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build build-cmake
ctest --test-dir build-cmake --output-on-failure
```

For a standalone checkout, the same build-and-test workflow is available as
the `tests` target:

```sh
cmake --build build-cmake --target tests
```

By default, tests are enabled for a standalone checkout and disabled when the
project is added as a subdirectory. They can be controlled explicitly with
`CSTORAGE_KIT_BUILD_TESTS`. Examples are opt-in with
`CSTORAGE_KIT_BUILD_EXAMPLES`.

For AddressSanitizer and UndefinedBehaviorSanitizer checks, use a separate
build directory:

```sh
cmake -S . -B build-sanitize \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCSTORAGE_KIT_ENABLE_SANITIZERS=ON
cmake --build build-sanitize
ctest --test-dir build-sanitize --output-on-failure
```

On platforms where AddressSanitizer leak detection is unavailable, run the
tests with `ASAN_OPTIONS=detect_leaks=0`.

## Use as a submodule

c-storage-kit is designed to be easy to include in another CMake project:

```cmake
add_subdirectory(external/c-storage-kit cstorage_kit EXCLUDE_FROM_ALL)
target_link_libraries(my_app PRIVATE CStorageKit::Core)
```

When included this way, the parent project gets the library target without
pulling the test executables into its normal build.

The library can also be installed and consumed through its generated CMake
export:

```sh
cmake -S . -B build-cmake
cmake --build build-cmake
cmake --install build-cmake --prefix /path/to/install
```

## Future direction

I’m continuing to explore a heterogeneous list that can hold values of
different types, inspired by dynamically typed languages such as Python. I
also want to build more structures—including linked lists, dictionaries,
stacks, queues, and trees—with consistent interfaces, generic typing, and
careful memory management.

c-storage-kit is still evolving, but its direction is clear: apply the
abstraction and data-structure principles I learned in CS 136 to a practical,
modern, and lightweight toolkit for C.
