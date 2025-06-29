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

## License

MIT
