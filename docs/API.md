# API Reference

Full API documentation for cpptypr.

## Version

```cpp
#include <cpptypr/version.hpp>

auto major = cpptypr::Version::Major;   // static const uint32_t
auto minor = cpptypr::Version::Minor;
auto patch = cpptypr::Version::Patch;
auto str   = cpptypr::Version()();      // returns std::string_view

auto cMajor = ctypr::Version::Major;    // same pattern for ctypr
auto cStr   = ctypr::Version()();
```

## Engine

### Lifecycle

```cpp
cpptypr::Engine e(cpptypr::EngineMode::Flow, contentProvider, timeout);
cpptypr::Engine e("flow", contentProvider, timeout);  // string mode

e.setLogger(logger);
e.setContentProvider(provider);
e.clearContentProvider();
e.setAutoSave(repo, true);
e.clearAutoSave();
e.resetLogger();
```

### Mode & timeout

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

### Keystrokes

```cpp
e.keyPress('T');
e.backspacePress();   // flow mode only
```

### Statistics

```cpp
auto s = e.stats();
// s.durationMs, s.correctKeystrokes, s.incorrectKeystrokes
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

// Enums with toString():
//   EngineState: Idle, Running, Paused, Error
//   StopCause:   None, Timeout, Finished, User, Error, Unknown
```

### Events (callbacks)

```cpp
auto h = e.onStarted([] { std::cout << "started\n"; });
// onStopped, onFinished, onPaused, onResumed, onTimeout
// onCorrectKeystroke, onIncorrectKeystroke, onBackspace
// onSegmentCompleted, onError

h.disconnect();           // manual
// or let h go out of scope -- RAII auto-disconnect
```

## ContentProvider

```cpp
auto cp = cpptypr::ContentProvider::fromString(text);
auto cp = cpptypr::ContentProvider::fromFile(path);
auto cp = cpptypr::ContentProvider::fromDatabase(path);
auto cp = cpptypr::ContentProvider::fromWeb(url);

cp.setMode(cpptypr::ContentMode::Sentences);
cp.setMode("commonwords");

cp.setContentLimit(100);

// Content filters (database provider only):
cp.setDifficultyFilter("Easy");         // "Easy"|"Normal"|"Hard"|"Expert"; empty to clear
cp.setWordLengthRange(4, 8);            // min/max length; 0,0 to clear

auto chunk = cp.getNext();      // ContentChunk{ text }
bool done = cp.isExhausted();
cp.reset();

// ContentMode: Sentences, CommonWords, RandomWords
// contentModeFromString(s), toString(mode)
```

## Repository

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

auto strict = repo.getSessionsByMode("strict");  // filter by mode

// Range-based for:
for (const auto& s : repo) { /* ... */ }
```

## Logger

```cpp
cpptypr::Logger log(cpptypr::LogLevel::Warning, true);
cpptypr::Logger log("info", true);   // string level

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

## Error handling

```cpp
try {
    e.start();
} catch (const cpptypr::Error& err) {
    auto code = err.code();          // ErrorCode
    std::cout << err.what() << "\n"; // human-readable message
}

// ErrorCode: None, InvalidMode, InvalidTimeout, AlreadyRunning,
//            NotRunning, Config, Content, State, Provider, File, Unknown
// operator<< for Error and ErrorCode
```
