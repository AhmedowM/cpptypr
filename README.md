# cpptypr

[![CI](https://github.com/AhmedowM/cpptypr/actions/workflows/ci.yml/badge.svg)](https://github.com/AhmedowM/cpptypr/actions/workflows/ci.yml)
[![Release](https://img.shields.io/github/v/release/AhmedowM/cpptypr)](https://github.com/AhmedowM/cpptypr/releases)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-linux%20|%20macOS%20|%20windows-blue)]()
[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue)]()

C++23 RAII wrappers for [ctypr](https://github.com/AhmedowM/ctypr), the typing test engine. cpptypr manages resources automatically, gives you `std::chrono` timings and `std::function` callbacks, and throws exceptions instead of returning error codes.

## Table of Contents

- [Quickstart](#quickstart)
- [Build & install](#build--install)
- [Features](#features)
- [Minimal example](#minimal-example)
- [API reference](#api-reference)
- [Project structure](#project-structure)
- [Running tests](#running-tests)
- [Contributing](#contributing)
- [License](#license)

## Quickstart

```sh
git clone https://github.com/AhmedowM/cpptypr.git
cd cpptypr
cmake -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Requires CMake 3.25+, a C++23 compiler (GCC 14+, Clang 18+, MSVC 2022 17.12+), and an internet connection on first build (ctypr and SQLite are fetched automatically).

## Build & install

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build --prefix /usr/local
```

Then use from another project:

```cmake
find_package(cpptypr REQUIRED)
target_link_libraries(my_app PRIVATE cpptypr::cpptypr)
```

For packaging: `cpack` produces TGZ and ZIP archives.

## Features

- RAII wrappers for Engine, Repository, Logger, ContentProvider, Snapshot
- Strict and flow typing modes
- Content from strings, files, SQLite databases, or URLs
- SQLite persistence with range-based iteration and best/average queries
- `std::function` callbacks with RAII disconnect handles
- Snapshot API for lock-free UI rendering
- String-based enums with `toString()` / `fromString()` for all types
- Stream output (`operator<<`) for all types and enums
- Version API exposing both cpptypr and ctypr versions
- Zero extra dependencies beyond ctypr and its SQLite

## Minimal example

```cpp
#include <cpptypr.hpp>
#include <iostream>

int main() {
    auto cp = cpptypr::ContentProvider::fromString(
        "The quick brown fox jumps over the lazy dog."
    );

    cpptypr::Engine e(cpptypr::EngineMode::Flow, cp, 60);
    e.start();

    for (char c : std::string{"The quick brown fox jumps over the lazy dog."}) {
        e.keyPress(c);
        if (e.isCompleted()) break;
    }

    auto snap = e.getSnapshot();
    std::cout << "WPM: " << snap.stats().wpm
              << "  Accuracy: " << snap.stats().accuracy << "%\n";
    return 0;
}
```

## API reference

The full API reference (Engine, ContentProvider, Repository, Logger, error handling) is in [`docs/API.md`](docs/API.md). Headers are documented with Doxygen.

## Project structure

```
src/
  cpptypr/
    engine.{hpp,cpp}       Engine, CallbackHandle
    content.{hpp,cpp}      ContentProvider, ContentMode, ContentChunk
    repository.{hpp,cpp}   Repository, SessionData
    logger.{hpp,cpp}       Logger, LogLevel
    snapshot.{hpp,cpp}     Snapshot, EngineState, StopCause
    stats.hpp              SessionStats
    error.{hpp,cpp}        Error, ErrorCode
    version.hpp.in         Version structs (cpptypr via CMake, ctypr declared)
    version.cpp            ctypr::Version definitions
    detail.hpp             internal helpers
  cpptypr.hpp              umbrella header
tests/
  test_engine.cpp          40 tests
  test_logger.cpp          15 tests
  test_content.cpp         16 tests
  test_repository.cpp      12 tests
  db_helpers.cpp           utilities
```

## Running tests

```sh
cmake -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Run individual suites:

```sh
./build/tests/cpptypr_test_engine
./build/tests/cpptypr_test_logger
./build/tests/cpptypr_test_content
./build/tests/cpptypr_test_repository
```

## Contributing

Bug reports and pull requests are welcome. Before submitting, make sure tests pass on your platform:

```sh
cmake -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Keep commits conventional-commit style (e.g., `feat:`, `fix:`, `chore:`). This project follows [SemVer](https://semver.org/).

## License

MIT. See [LICENSE](LICENSE).
