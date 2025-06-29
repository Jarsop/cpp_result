# cpp-result

This header-only library provides a minimal and efficient Rust-like `Result<T, E>` for C++17. It has no dependencies and offers a simple, ergonomic API inspired by Rust.

See [`examples/usage.cpp`](examples/usage.cpp) or [documentation](https://jarsop.github.io/cpp_result) for full usages.

## Building

### CMake

```bash
cmake -S . -B build
cmake --build build
```

### Meson

```bash
meson setup builddir
meson compile -C builddir
```

## Tests

_googletest required_

### CMake

```bash
ctest --test-dir build
```

### Meson

```bash
meson test -C builddir
```

## Benchmarks

_google-benchmark required_

Benchmarks are provided to compare the performance of exceptions, error code returns, and Result<T, E>.
This is a straightforward approach that provides a quick overview of the different methods.

### CMake

```bash
cmake --build build --target benchmark
```

### Meson

```bash
meson test -C builddir --benchmark
```

## Feature Control

cpp_result allows you to enable or disable groups of API features at compile time to reduce binary size or limit API surface.

- **Global switch:**  
  `CPP_RESULT_FEATURE_ALL` (default: enabled)  
  If enabled, all features are active unless overridden.

- **Individual features:**  
  - `CPP_RESULT_FEATURE_UNWRAP`   : Unwrap/expect helpers (unwrap, unwrap_err, expect, etc.)
  - `CPP_RESULT_FEATURE_MAP`      : Map/map_err/map_or/map_or_else
  - `CPP_RESULT_FEATURE_ANDOR`    : and_, and_then, or_, or_else
  - `CPP_RESULT_FEATURE_INSPECT`  : inspect, inspect_err
  - `CPP_RESULT_FEATURE_CONTAINS` : contains, contains_err
  - `CPP_RESULT_FEATURE_FLATTEN`  : flatten
  - `CPP_RESULT_FEATURE_OPTIONAL` : ok(), err() as std::optional

You can set these macros before including the header, or use the build system options (`-DCPP_RESULT_FEATURE_*` for CMake, `-DCPP_RESULT_FEATURE_*` for Meson).

Example (disable all except unwrap):

```cpp
#define CPP_RESULT_FEATURE_ALL 0
#define CPP_RESULT_FEATURE_UNWRAP 1
#include <result.hpp>
```

See the header and API docs for details.

## License

MIT
