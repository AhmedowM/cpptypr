# cpptypr

C++ RAII wrappers over [ctypr](https://github.com/AhmedowM/ctypr) — a small C library that simulates typing sessions and measures your speed and accuracy.

## Why this exists

ctypr gives you a clean C API for typing logic, but C code needs manual resource management, null checks, and tends to spread boilerplate across every caller. cpptypr wraps every ctypr object (Engine, Repository, Logger, ContentProvider) in a move-only RAII class with `std::chrono` integration, `std::function`-based callbacks, and exceptions for error handling — so you can focus on the UI in idiomatic C++.

- **RAII wrappers** — Engine, Repository, Logger, ContentProvider, Snapshot; all non-copyable, movable, with automatic cleanup
- **Two typing modes** — strict mode locks you until you hit the right key; flow mode keeps advancing and lets you backspace
- **WPM, accuracy, and timing** — `std::chrono::milliseconds` durations, raw and adjusted WPM
- **Multiple content sources** — strings, files, SQLite databases, or URLs, with configurable content mode
- **SQLite persistence** — range-based for iteration, best/average queries, auto-save on completion
- **Event callbacks** — `std::function`-based signals with RAII `CallbackHandle` that auto-disconnects on destruction
- **Snapshot API** — `Engine::getSnapshot()` returns an atomic `Snapshot` with text, cursor, incorrect flags, stats, and engine state in one call
- **String-based enums** — all modes and levels can be set via case-insensitive strings; `toString()` and `fromString()` for every enum
- **Stream output** — `operator<<` for Error, SessionData, SessionStats, and all enums
- **Version API** — `cpptypr::Version::Major` / `::Minor` / `::Patch` and `Version()()` for the string; `ctypr::Version` mirrors the same pattern without exposing C headers
- **No extra dependencies** — cpptypr links to ctypr, which pulls in SQLite via CMake; nothing else required
- **CMake install + CPack** — `find_package(cpptypr)` support, TGZ/ZIP packaging

## Prerequisites

- **CMake >= 3.25**
- A **C++23 compiler** (GCC 14+, Clang 18+, MSVC 2022 17.12+)
- Windows, macOS, or Linux
- An internet connection on first build (CMake fetches ctypr and SQLite automatically)

## Building

```sh
git clone https://github.com/AhmedowM/cpptypr.git
cd cpptypr
cmake -B build
cmake --build build
```

Tests are built by default:

```sh
cmake -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## A minimal example

```cpp
#include <cpptypr.hpp>
#include <iostream>
#include <string>

int main() {
    auto cp = cpptypr::ContentProvider::fromString(
        "The quick brown fox jumps over the lazy dog."
    );

    cpptypr::Engine e(cpptypr::EngineMode::Flow, cp, 60);
    e.start();

    std::string text = "The quick brown fox jumps over the lazy dog.";
    for (char c : text) {
        e.keyPress(c);
        if (e.isCompleted()) break;
    }

    auto snap = e.getSnapshot();
    std::cout << "WPM: " << snap.stats().wpm
              << "  Accuracy: " << snap.stats().accuracy << "%\n";
    return 0;
}
```

## Current status

cpptypr is functionally complete for basic typing sessions. Every ctypr object has a corresponding RAII wrapper. All wrappers include move semantics, `CHECK_MOVED` guards, and string-based enum constructors/setters. The library is tested across 4 suites (80+ tests) on Linux, macOS, and Windows (MinGW, MSVC).

## API reference

### Version

```cpp
#include <cpptypr/version.hpp>

auto major = cpptypr::Version::Major; // static const uint32_t
auto minor = cpptypr::Version::Minor;
auto patch = cpptypr::Version::Patch;
auto str   = cpptypr::Version()();    // static operator() returns std::string_view

auto cMajor = ctypr::Version::Major;  // same pattern for ctypr version
auto cStr   = ctypr::Version()();
```

### Engine lifecycle

```cpp
cpptypr::Engine e(cpptypr::EngineMode::Flow, contentProvider, timeout);
cpptypr::Engine e("flow", contentProvider, timeout); // string-based mode

e.setLogger(logger);
e.setContentProvider(provider);
e.clearContentProvider();
e.setAutoSave(repo, true);
e.clearAutoSave();
e.resetLogger();
```

### Mode and timeout

```cpp
e.setMode(cpptypr::EngineMode::Strict);
e.setMode("strict");
auto mode = e.mode();           // EngineMode
e.setTimeout(60);
auto t = e.timeout();           // uint16_t seconds
```

### Session control

```cpp
e.start();
e.stop();
e.pause();
e.resume();
e.reset();
```

### State queries

```cpp
e.isRunning();
e.isPaused();
e.isIdle();
e.isError();
e.isCompleted();
e.isTimedOut();
e.isStopped();
e.wasStopped();
```

### Keystroke processing

```cpp
e.keyPress('T');
e.backspacePress();        // flow mode only
```

### Statistics

```cpp
auto s = e.stats();
// s.durationMs, s.correctKeystrokes, s.incorrectKeystrokes,
// s.totalKeystrokes, s.accuracy, s.wpm, s.wpmRaw
```

### Snapshot

```cpp
auto snap = e.getSnapshot();
snap.text();              // std::string_view
snap.length();            // size_t
snap.cursorIndex();       // uint32_t
snap.expectedChar();      // char
snap.isIncorrect(i);      // bool
snap.stats();             // SessionStats
snap.state();             // EngineState
snap.stopCause();         // StopCause

// Enums:
//   EngineState: Idle, Running, Paused, Error
//   StopCause:   None, Timeout, Finished, User, Error, Unknown
//   toString() for both
```

### Events (callbacks)

```cpp
auto h = e.onStarted([] { std::cout << "started\n"; });
// onStopped, onFinished, onPaused, onResumed, onTimeout
// onCorrectKeystroke, onIncorrectKeystroke, onBackspace
// onSegmentCompleted, onError

h.disconnect();           // manual disconnect
// or let h go out of scope (RAII auto-disconnect)
```

### ContentProvider

```cpp
auto cp = cpptypr::ContentProvider::fromString(text);
auto cp = cpptypr::ContentProvider::fromFile(path);
auto cp = cpptypr::ContentProvider::fromDatabase(path);
auto cp = cpptypr::ContentProvider::fromWeb(url);

cp.setMode(cpptypr::ContentMode::Sentences);
cp.setMode("commonwords");

cp.setContentLimit(100);
auto chunk = cp.getNext();   // ContentChunk{ text }
bool done = cp.isExhausted();
cp.reset();

// ContentMode: Sentences, CommonWords, RandomWords
// contentModeFromString(s), toString(mode)
```

### Repository

```cpp
cpptypr::Repository repo("sessions.db");

auto id = repo.saveSession(data);
auto opt = repo.getSession(id);       // std::optional<SessionData>
auto all = repo.getAll();
auto recent = repo.getRecent(10);
auto count = repo.count();
repo.deleteSession(id);
repo.clearAll();

auto best = repo.bestWpm();           // std::optional<SessionData>
auto bestRaw = repo.bestRawWpm();
auto avg = repo.averageWpm();

// Range-based for:
for (const auto& s : repo) { /* ... */ }
```

### Logger

```cpp
cpptypr::Logger log(cpptypr::LogLevel::Warning, true);
cpptypr::Logger log("info", true);   // string-based level

log.setLevel(cpptypr::LogLevel::Debug);
log.setLevel("warning");
auto level = log.level();
log.logToStdout(true);
log.addFile("log.txt");
log.log(cpptypr::LogLevel::Info, "hello");
log.log("error", "something broke");

// LogLevel: Debug, Info, Warning, Error, None
// logLevelFromString(s), toString(level)
```

### Error handling

```cpp
try {
    e.start();
} catch (const cpptypr::Error& err) {
    auto code = err.code();      // ErrorCode
    std::cout << err.what() << "\n";  // human-readable message
}

// ErrorCode: None, InvalidMode, InvalidTimeout, AlreadyRunning,
//            NotRunning, Config, Content, State, Provider, File, Unknown
// operator<< for Error and ErrorCode
```

## Project structure

```
src/
  cpptypr/
    engine.{hpp,cpp}      Engine, CallbackHandle
    content.{hpp,cpp}     ContentProvider, ContentMode, ContentChunk
    repository.{hpp,cpp}  Repository, SessionData
    logger.{hpp,cpp}      Logger, LogLevel
    snapshot.{hpp,cpp}    Snapshot, EngineState, StopCause
    stats.hpp             SessionStats
    error.{hpp,cpp}       Error, ErrorCode
    version.hpp.in        Version structs (cpptypr::Version via CMake, ctypr::Version declaration)
    version.cpp           ctypr::Version out-of-line definitions (includes ctypr version.h)
    detail.hpp            internal helpers (CHECK_MOVED, toLower)
  cpptypr.hpp             umbrella header
tests/
  test_engine.cpp         40 tests: lifecycle, state, keystrokes, callbacks, stats, snapshot, version
  test_logger.cpp         15 tests: level, stdout, file, filtering, move
  test_content.cpp        16 tests: string/file/db/web providers, mode, limit, move
  test_repository.cpp     12 tests: CRUD, best/average, move, range-for
  db_helpers.cpp          test database utilities
```

## Running the tests

```sh
ctest --test-dir build --output-on-failure
```

Or run a specific suite:

```sh
./build/tests/cpptypr_test_engine
./build/tests/cpptypr_test_logger
./build/tests/cpptypr_test_content
./build/tests/cpptypr_test_repository
```
