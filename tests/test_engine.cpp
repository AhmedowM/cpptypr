#include <cpptypr.hpp>

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <thread>

static int tests_passed = 0;
static int tests_failed = 0;
static int test_count = 0;

#define TEST(name) do { \
    test_count++; \
    printf("  TEST %d: %s ... ", test_count, name); \
    fflush(stdout); \
} while(0)

#define PASS() do { \
    tests_passed++; \
    printf("PASSED\n"); \
} while(0)

#define FAIL(msg) do { \
    tests_failed++; \
    printf("FAILED: %s (line %d)\n", msg, __LINE__); \
} while(0)

#define ASSERT(cond, msg) do { \
    if (!(cond)) { FAIL(msg); return; } \
} while(0)

static cpptypr::ContentProvider sharedContent = cpptypr::ContentProvider::fromString(
    "The quick brown fox jumps over the lazy dog.");

static cpptypr::Engine makeEngine(cpptypr::EngineMode mode, uint16_t timeout = 0) {
    sharedContent.reset();
    return cpptypr::Engine(mode, sharedContent, timeout);
}

// ===== Creation & Config =====

static void test_create_destroy() {
    TEST("Engine creation and destruction");
    auto e = makeEngine(cpptypr::EngineMode::Strict, 0);
    ASSERT(e.mode() == cpptypr::EngineMode::Strict, "mode should be Strict");
    ASSERT(e.timeout() == 0, "timeout should be 0");
    ASSERT(e.isIdle(), "engine should be idle after creation");
    PASS();
}

static void test_mode() {
    TEST("Mode get/set");
    auto e = makeEngine(cpptypr::EngineMode::Strict);
    ASSERT(e.mode() == cpptypr::EngineMode::Strict, "initial mode should be Strict");
    e.setMode(cpptypr::EngineMode::Flow);
    ASSERT(e.mode() == cpptypr::EngineMode::Flow, "mode should be Flow after set");
    e.setMode(cpptypr::EngineMode::Strict);
    ASSERT(e.mode() == cpptypr::EngineMode::Strict, "mode should be Strict after set back");
    PASS();
}

static void test_timeout() {
    TEST("Timeout get/set");
    auto e = makeEngine(cpptypr::EngineMode::Strict, 30);
    ASSERT(e.timeout() == 30, "initial timeout should be 30");
    e.setTimeout(60);
    ASSERT(e.timeout() == 60, "timeout should be 60 after set");
    PASS();
}

// ===== State Transitions =====

static void test_state_transitions() {
    TEST("State transitions: start -> stop");
    auto e = makeEngine(cpptypr::EngineMode::Strict);
    ASSERT(e.isIdle(), "should be idle initially");
    e.start();
    ASSERT(e.isRunning(), "should be running after start");
    ASSERT(!e.isIdle(), "should not be idle after start");
    e.stop();
    ASSERT(!e.isIdle(), "should not be idle (has stop cause)");
    ASSERT(e.isStopped(), "should be stopped (user stop)");
    PASS();
}

static void test_pause_resume() {
    TEST("State transitions: start -> pause -> resume -> stop");
    auto e = makeEngine(cpptypr::EngineMode::Strict);
    e.start();
    ASSERT(e.isRunning(), "should be running after start");
    e.pause();
    ASSERT(e.isPaused(), "should be paused after pause");
    ASSERT(!e.isRunning(), "should not be running after pause");
    e.resume();
    ASSERT(e.isRunning(), "should be running after resume");
    e.stop();
    ASSERT(e.isStopped(), "should be stopped");
    PASS();
}

static void test_reset() {
    TEST("State transitions: start -> reset");
    auto e = makeEngine(cpptypr::EngineMode::Strict);
    e.start();
    ASSERT(e.isRunning(), "should be running after start");
    e.reset();
    ASSERT(e.isIdle(), "should be idle after reset");
    ASSERT(!e.isStopped(), "should not be stopped after reset");
    ASSERT(!e.isCompleted(), "should not be completed after reset");
    PASS();
}

// ===== Strict Mode =====

static void test_strict_correct_key() {
    TEST("Strict mode: correct key advances");
    auto e = makeEngine(cpptypr::EngineMode::Strict);
    e.start();
    ASSERT(e.isRunning(), "should be running");
    e.keyPress('T');
    auto stats = e.stats();
    ASSERT(stats.totalKeystrokes == 1, "totalKeystrokes should be 1");
    ASSERT(stats.correctKeystrokes == 1, "correctKeystrokes should be 1");
    ASSERT(fabs(stats.accuracy - 100.0) < 0.001, "accuracy should be ~100%");
    PASS();
}

static void test_strict_incorrect_key() {
    TEST("Strict mode: incorrect key does not advance");
    auto e = makeEngine(cpptypr::EngineMode::Strict);
    e.start();
    e.keyPress('X');
    auto stats = e.stats();
    ASSERT(stats.totalKeystrokes == 1, "totalKeystrokes should be 1");
    ASSERT(stats.correctKeystrokes == 0, "correctKeystrokes should be 0");
    ASSERT(stats.accuracy == 0.0, "accuracy should be 0%");
    e.keyPress('T');
    stats = e.stats();
    ASSERT(stats.totalKeystrokes == 2, "totalKeystrokes should be 2");
    ASSERT(stats.correctKeystrokes == 1, "correctKeystrokes should be 1");
    PASS();
}

static void test_strict_backspace_disabled() {
    TEST("Strict mode: backspace is disabled");
    auto e = makeEngine(cpptypr::EngineMode::Strict);
    e.start();
    e.keyPress('T');
    e.backspacePress();
    auto stats = e.stats();
    ASSERT(stats.totalKeystrokes == 1, "totalKeystrokes should still be 1");
    ASSERT(stats.correctKeystrokes == 1, "correctKeystrokes should still be 1");
    PASS();
}

// ===== Flow Mode =====

static void test_flow_correct_key() {
    TEST("Flow mode: correct key advances");
    auto e = makeEngine(cpptypr::EngineMode::Flow);
    e.start();
    e.keyPress('T');
    auto stats = e.stats();
    ASSERT(stats.totalKeystrokes == 1, "totalKeystrokes should be 1");
    ASSERT(stats.correctKeystrokes == 1, "correctKeystrokes should be 1");
    PASS();
}

static void test_flow_incorrect_key_advances() {
    TEST("Flow mode: incorrect key still advances");
    auto e = makeEngine(cpptypr::EngineMode::Flow);
    e.start();
    e.keyPress('X');
    auto stats = e.stats();
    ASSERT(stats.totalKeystrokes == 1, "totalKeystrokes should be 1");
    ASSERT(stats.correctKeystrokes == 0, "correctKeystrokes should be 0");
    e.keyPress('h');
    stats = e.stats();
    ASSERT(stats.totalKeystrokes == 2, "totalKeystrokes should be 2");
    ASSERT(stats.correctKeystrokes == 1, "correctKeystrokes should be 1");
    PASS();
}

static void test_flow_backspace_correct() {
    TEST("Flow mode: backspace over correct key");
    auto e = makeEngine(cpptypr::EngineMode::Flow);
    e.start();
    e.keyPress('T');
    e.keyPress('h');
    auto stats = e.stats();
    ASSERT(stats.totalKeystrokes == 2, "totalKeystrokes should be 2");
    ASSERT(stats.correctKeystrokes == 2, "correctKeystrokes should be 2");
    e.backspacePress();
    stats = e.stats();
    ASSERT(stats.totalKeystrokes == 2, "totalKeystrokes should still be 2");
    ASSERT(stats.correctKeystrokes == 1, "correctKeystrokes should be 1 after undoing correct char");
    PASS();
}

static void test_flow_backspace_incorrect() {
    TEST("Flow mode: backspace over incorrect key");
    auto e = makeEngine(cpptypr::EngineMode::Flow);
    e.start();
    e.keyPress('X');
    auto stats = e.stats();
    ASSERT(stats.totalKeystrokes == 1, "totalKeystrokes should be 1");
    ASSERT(stats.correctKeystrokes == 0, "correctKeystrokes should be 0");
    e.backspacePress();
    stats = e.stats();
    ASSERT(stats.totalKeystrokes == 1, "totalKeystrokes should still be 1");
    ASSERT(stats.correctKeystrokes == 0, "correctKeystrokes should still be 0");
    e.keyPress('T');
    stats = e.stats();
    ASSERT(stats.totalKeystrokes == 2, "totalKeystrokes should be 2");
    ASSERT(stats.correctKeystrokes == 1, "correctKeystrokes should be 1");
    PASS();
}

// ===== Session Completion =====

static void test_session_complete_strict() {
    TEST("Strict mode: session completion");
    auto e = makeEngine(cpptypr::EngineMode::Strict);
    bool finished = false, stopped = false;
    auto h1 = e.onFinished([&] { finished = true; });
    auto h2 = e.onStopped([&] { stopped = true; });
    e.start();
    const char* text = "The quick brown fox jumps over the lazy dog.";
    for (const char* p = text; *p; p++) { e.keyPress(*p); }
    ASSERT(!e.isIdle(), "should not be idle (has finish cause)");
    ASSERT(e.isCompleted(), "should be completed");
    ASSERT(finished, "Finished callback should have fired");
    ASSERT(stopped, "Stopped callback should have fired");
    PASS();
}

static void test_session_complete_flow() {
    TEST("Flow mode: session completion");
    auto e = makeEngine(cpptypr::EngineMode::Flow);
    bool finished = false, stopped = false;
    auto h1 = e.onFinished([&] { finished = true; });
    auto h2 = e.onStopped([&] { stopped = true; });
    e.start();
    const char* text = "The quick brown fox jumps over the lazy dog.";
    for (const char* p = text; *p; p++) { e.keyPress(*p); }
    ASSERT(!e.isIdle(), "should not be idle (has finish cause)");
    ASSERT(e.isCompleted(), "should be completed");
    ASSERT(finished, "Finished callback should have fired");
    ASSERT(stopped, "Stopped callback should have fired");
    PASS();
}

// ===== Callbacks =====

static void test_callbacks() {
    TEST("Callback system: all lifecycle events fire");
    auto e = makeEngine(cpptypr::EngineMode::Strict);
    bool started = false, stopped = false, paused = false, resumed = false;
    bool correct = false, incorrect = false, backspace = false;
    auto h1 = e.onStarted([&] { started = true; });
    auto h2 = e.onStopped([&] { stopped = true; });
    auto h3 = e.onPaused([&] { paused = true; });
    auto h4 = e.onResumed([&] { resumed = true; });
    auto h5 = e.onCorrectKeystroke([&] { correct = true; });
    auto h6 = e.onIncorrectKeystroke([&] { incorrect = true; });
    auto h7 = e.onBackspace([&] { backspace = true; });

    e.start();
    ASSERT(started, "Started callback should fire");

    e.keyPress('T');
    ASSERT(correct, "CorrectKeystroke callback should fire");

    e.keyPress('X');
    ASSERT(incorrect, "IncorrectKeystroke callback should fire");

    e.pause();
    ASSERT(paused, "Paused callback should fire");

    e.resume();
    ASSERT(resumed, "Resumed callback should fire");

    e.stop();
    ASSERT(stopped, "Stopped callback should fire");
    PASS();
}

static void test_multiple_callbacks() {
    TEST("Multiple callbacks for same event");
    auto e = makeEngine(cpptypr::EngineMode::Strict);
    int c1 = 0, c2 = 0;
    auto h1 = e.onStarted([&] { c1++; });
    auto h2 = e.onStarted([&] { c2++; });
    e.start();
    ASSERT(c1 == 1, "first callback should fire once");
    ASSERT(c2 == 1, "second callback should fire once");
    PASS();
}

static void test_signal_disconnect() {
    TEST("Signal: disconnect removes callback");
    auto e = makeEngine(cpptypr::EngineMode::Strict);
    int count = 0;
    cpptypr::CallbackHandle handle;
    {
        auto h = e.onStarted([&] { count++; });
        handle = std::move(h);
    }
    handle.disconnect();
    e.start();
    ASSERT(count == 0, "callback should NOT fire after disconnect");
    PASS();
}

static void test_callback_raii() {
    TEST("CallbackHandle: RAII disconnect on destruction");
    auto e = makeEngine(cpptypr::EngineMode::Strict);
    int count = 0;
    {
        auto h = e.onStarted([&] { count++; });
        ASSERT(h, "handle should be valid");
    }
    e.start();
    ASSERT(count == 0, "callback should NOT fire after handle destruction");
    PASS();
}

static void test_callback_move() {
    TEST("CallbackHandle: move semantics");
    auto e = makeEngine(cpptypr::EngineMode::Strict);
    int count = 0;
    auto h1 = e.onStarted([&] { count++; });
    auto h2 = std::move(h1);
    ASSERT(h2, "moved-to handle should be valid");
    ASSERT(!h1, "moved-from handle should be invalid");
    e.start();
    ASSERT(count == 1, "callback should fire via moved handle");
    PASS();
}

// ===== Stats =====

static void test_stats_accuracy() {
    TEST("Stats: accuracy calculation");
    auto e = makeEngine(cpptypr::EngineMode::Strict);
    e.start();
    e.keyPress('T');
    e.keyPress('X');
    e.keyPress('h');
    e.keyPress('e');
    auto stats = e.stats();
    ASSERT(stats.totalKeystrokes == 4, "totalKeystrokes should be 4");
    ASSERT(stats.correctKeystrokes == 3, "correctKeystrokes should be 3");
    ASSERT(stats.accuracy == 75.0, "accuracy should be 75.0");
    PASS();
}

static void test_stats_before_start() {
    TEST("Stats: get stats before starting");
    auto e = makeEngine(cpptypr::EngineMode::Strict);
    auto stats = e.stats();
    ASSERT(stats.totalKeystrokes == 0, "totalKeystrokes should be 0");
    ASSERT(stats.correctKeystrokes == 0, "correctKeystrokes should be 0");
    ASSERT(stats.wpm == 0.0, "wpm should be 0.0");
    ASSERT(stats.wpmRaw == 0.0, "wpmRaw should be 0.0");
    ASSERT(stats.durationMs.count() == 0, "durationMs should be 0");
    PASS();
}

// ===== Error / Edge Cases =====

static void test_error_already_running() {
    TEST("Error handling: start when already running does not crash");
    auto e = makeEngine(cpptypr::EngineMode::Strict);
    e.start();
    e.start(); // should not crash
    PASS();
}

static void test_error_not_running() {
    TEST("Error handling: stop when not running does not crash");
    auto e = makeEngine(cpptypr::EngineMode::Strict);
    e.stop(); // should not crash
    PASS();
}

static void test_ctypr_error() {
    TEST("Error: Error produces correct what() string");
    try {
        throw cpptypr::Error(cpptypr::ErrorCode::AlreadyRunning);
    } catch (const cpptypr::Error& e) {
        ASSERT(strcmp(e.what(), "Already Running") == 0, "error string should match");
    }
    try {
        throw cpptypr::Error(cpptypr::ErrorCode::None);
    } catch (const cpptypr::Error& e) {
        ASSERT(strcmp(e.what(), "No Error") == 0, "error string should match");
    }
    try {
        throw cpptypr::Error(cpptypr::ErrorCode::InvalidMode);
    } catch (const cpptypr::Error& e) {
        ASSERT(strcmp(e.what(), "Invalid Mode") == 0, "error string should match");
    }
    try {
        throw cpptypr::Error(cpptypr::ErrorCode::InvalidTimeout);
    } catch (const cpptypr::Error& e) {
        ASSERT(strcmp(e.what(), "Invalid Timeout") == 0, "error string should match");
    }
    try {
        throw cpptypr::Error(cpptypr::ErrorCode::Content);
    } catch (const cpptypr::Error& e) {
        ASSERT(strcmp(e.what(), "Content Error") == 0, "error string should match");
    }
    try {
        throw cpptypr::Error(cpptypr::ErrorCode::Config);
    } catch (const cpptypr::Error& e) {
        ASSERT(strcmp(e.what(), "Config Error") == 0, "error string should match");
    }
    try {
        throw cpptypr::Error(cpptypr::ErrorCode::File);
    } catch (const cpptypr::Error& e) {
        ASSERT(strcmp(e.what(), "File Error") == 0, "error string should match");
    }
    try {
        throw cpptypr::Error(cpptypr::ErrorCode::Unknown);
    } catch (const cpptypr::Error& e) {
        ASSERT(strcmp(e.what(), "Unknown Error") == 0, "error string should match");
    }
    PASS();
}

static void test_pause_error_path() {
    TEST("Error handling: pause when not running");
    auto e = makeEngine(cpptypr::EngineMode::Strict);
    ASSERT(e.isIdle(), "should be idle");
    e.pause();
    ASSERT(e.isIdle(), "should still be idle after pause on non-running engine");
    PASS();
}

static void test_resume_error_path() {
    TEST("Error handling: resume when not paused");
    auto e = makeEngine(cpptypr::EngineMode::Strict);
    e.start();
    ASSERT(e.isRunning(), "should be running");
    e.resume();
    ASSERT(e.isRunning(), "should still be running after resume on non-paused engine");
    e.stop();
    PASS();
}

// ===== Timeout =====

static void test_timeout_zero_disabled() {
    TEST("Timeout: zero timeout means no timeout");
    auto e = makeEngine(cpptypr::EngineMode::Strict, 0);
    e.start();
    e.keyPress('T');
    e.keyPress('h');
    e.keyPress('e');
    ASSERT(e.isRunning(), "should still be running (no timeout)");
    e.stop();
    PASS();
}

static void test_timeout_triggers() {
    TEST("Timeout: triggers after configured duration");
    auto e = makeEngine(cpptypr::EngineMode::Strict, 1);
    bool timedOut = false;
    auto h = e.onTimeout([&] { timedOut = true; });
    e.start();
    ASSERT(e.isRunning(), "should be running");
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    e.keyPress('T');
    ASSERT(e.isTimedOut(), "should be timed out");
    ASSERT(timedOut, "Timeout callback should have fired");
    auto stats = e.stats();
    ASSERT(stats.totalKeystrokes == 0, "key should not have been processed after timeout");
    PASS();
}

static void test_timeout_pause_does_not_accumulate() {
    TEST("Timeout: paused time does not count toward timeout");
    auto e = makeEngine(cpptypr::EngineMode::Strict, 1);
    e.start();
    e.keyPress('T');
    e.pause();
    ASSERT(e.isPaused(), "should be paused");
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    e.resume();
    ASSERT(e.isRunning(), "should still be running (paused time not counted)");
    e.keyPress('h');
    ASSERT(e.isRunning(), "should still be running after resume + key");
    e.stop();
    PASS();
}

// ===== Auto-save =====

static void test_auto_save_session() {
    TEST("Auto-save: saves session on completion");
    cpptypr::Repository repo("test_autosave.db");
    sharedContent.reset();
    cpptypr::Engine e(cpptypr::EngineMode::Flow, sharedContent, 0);
    e.setAutoSave(repo, true);
    e.start();
    const char* text = "The quick brown fox jumps over the lazy dog.";
    for (const char* p = text; *p; p++) { e.keyPress(*p); }
    ASSERT(e.isCompleted(), "should be completed");
    auto all = repo.getAll();
    ASSERT(all.size() == 1, "should have 1 saved session");
    ASSERT(all[0].totalChars == 44, "totalChars should be 44");
    ASSERT(all[0].correctChars == 44, "correctChars should be 44");
    ASSERT(fabs(all[0].accuracy - 100.0) < 0.001, "accuracy should be ~100.0");
    remove("test_autosave.db");
    PASS();
}

int main() {
    printf("=== cpptypr Engine Test Suite ===\n\n");

    test_create_destroy();
    test_mode();
    test_timeout();
    test_state_transitions();
    test_pause_resume();
    test_reset();
    test_strict_correct_key();
    test_strict_incorrect_key();
    test_strict_backspace_disabled();
    test_flow_correct_key();
    test_flow_incorrect_key_advances();
    test_flow_backspace_correct();
    test_flow_backspace_incorrect();
    test_session_complete_strict();
    test_session_complete_flow();
    test_callbacks();
    test_multiple_callbacks();
    test_signal_disconnect();
    test_callback_raii();
    test_callback_move();
    test_stats_accuracy();
    test_stats_before_start();
    test_error_already_running();
    test_error_not_running();
    test_ctypr_error();
    test_pause_error_path();
    test_resume_error_path();
    test_timeout_zero_disabled();
    test_timeout_triggers();
    test_timeout_pause_does_not_accumulate();
    test_auto_save_session();

    printf("\n=== Results: %d passed, %d failed, %d total ===\n",
           tests_passed, tests_failed, test_count);

    remove("test_autosave.db");

    return tests_failed > 0 ? 1 : 0;
}
