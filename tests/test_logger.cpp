#include <cpptypr.hpp>

#include <cstdio>
#include <cstring>
#include <string>

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

static void test_create_destroy() {
    TEST("Logger: create and destroy");
    { cpptypr::Logger log(cpptypr::LogLevel::Info, false); }
    PASS();
}

static void test_default_level() {
    TEST("Logger: level defaults correctly");
    cpptypr::Logger log(cpptypr::LogLevel::Warning, false);
    ASSERT(log.level() == cpptypr::LogLevel::Warning, "level should be Warning");
    PASS();
}

static void test_set_get_level() {
    TEST("Logger: set and get level");
    cpptypr::Logger log(cpptypr::LogLevel::Debug, false);
    log.setLevel(cpptypr::LogLevel::Error);
    ASSERT(log.level() == cpptypr::LogLevel::Error, "level should be Error");
    log.setLevel(cpptypr::LogLevel::Debug);
    ASSERT(log.level() == cpptypr::LogLevel::Debug, "level should be Debug");
    PASS();
}

static void test_log_to_stdout() {
    TEST("Logger: log to stdout does not crash");
    cpptypr::Logger log(cpptypr::LogLevel::Debug, true);
    log.log(cpptypr::LogLevel::Info, "stdout smoke test");
    PASS();
}

static void test_log_to_file() {
    TEST("Logger: log to file");
    {
        cpptypr::Logger log(cpptypr::LogLevel::Debug, false);
        ASSERT(log.addFile("test_log.txt"), "addFile should succeed");
        log.log(cpptypr::LogLevel::Warning, "file test message");
    }
    FILE* f = fopen("test_log.txt", "r");
    ASSERT(f != nullptr, "log file should exist");
    char buf[256]{};
    size_t n = fread(buf, 1, sizeof(buf) - 1, f);
    buf[n] = '\0';
    fclose(f);
    remove("test_log.txt");
    ASSERT(strstr(buf, "[WARNING]") != nullptr, "output should contain [WARNING]");
    ASSERT(strstr(buf, "file test message") != nullptr, "output should contain message");
    PASS();
}

static void test_level_filtering() {
    TEST("Logger: messages below level are suppressed via file");
    {
        cpptypr::Logger log(cpptypr::LogLevel::Warning, false);
        ASSERT(log.addFile("test_filter.txt"), "add file");
        log.log(cpptypr::LogLevel::Debug, "debug msg");
        log.log(cpptypr::LogLevel::Info, "info msg");
        log.log(cpptypr::LogLevel::Warning, "warning msg");
        log.log(cpptypr::LogLevel::Error, "error msg");
    }
    FILE* f = fopen("test_filter.txt", "r");
    ASSERT(f != nullptr, "log file should exist");
    char buf[512]{};
    size_t n = fread(buf, 1, sizeof(buf) - 1, f);
    buf[n] = '\0';
    fclose(f);
    remove("test_filter.txt");
    ASSERT(strstr(buf, "debug msg") == nullptr, "Debug should be suppressed");
    ASSERT(strstr(buf, "info msg") == nullptr, "Info should be suppressed");
    ASSERT(strstr(buf, "warning msg") != nullptr, "Warning should appear");
    ASSERT(strstr(buf, "error msg") != nullptr, "Error should appear");
    PASS();
}

static void test_toggle_stdout() {
    TEST("Logger: toggle stdout on/off does not crash");
    cpptypr::Logger log(cpptypr::LogLevel::Debug, true);
    log.logToStdout(false);
    log.log(cpptypr::LogLevel::Info, "should not appear on stdout");
    log.logToStdout(true);
    log.log(cpptypr::LogLevel::Info, "should appear on stdout");
    PASS();
}

static void test_add_multiple_files() {
    TEST("Logger: add multiple files");
    {
        cpptypr::Logger log(cpptypr::LogLevel::Debug, false);
        ASSERT(log.addFile("test_log1.txt"), "add file 1");
        ASSERT(log.addFile("test_log2.txt"), "add file 2");
        log.log(cpptypr::LogLevel::Info, "multi-file test");
    }
    for (int i = 1; i <= 2; i++) {
        char name[32];
        snprintf(name, sizeof(name), "test_log%d.txt", i);
        FILE* f = fopen(name, "r");
        ASSERT(f != nullptr, "log file should exist");
        char buf[128]{};
        fread(buf, 1, sizeof(buf) - 1, f);
        fclose(f);
        remove(name);
        ASSERT(strstr(buf, "multi-file test") != nullptr, "file should contain message");
    }
    PASS();
}

static void test_max_files_limit() {
    TEST("Logger: max files limit enforced");
    {
        cpptypr::Logger log(cpptypr::LogLevel::Debug, false);
        ASSERT(log.addFile("test_mf1.txt"), "add 1");
        ASSERT(log.addFile("test_mf2.txt"), "add 2");
        ASSERT(log.addFile("test_mf3.txt"), "add 3");
        ASSERT(log.addFile("test_mf4.txt"), "add 4");
        ASSERT(!log.addFile("test_mf5.txt"), "5th add should fail");
    }
    remove("test_mf1.txt");
    remove("test_mf2.txt");
    remove("test_mf3.txt");
    remove("test_mf4.txt");
    PASS();
}

static void test_log_output_format() {
    TEST("Logger: output format is [LEVEL] message");
    {
        cpptypr::Logger log(cpptypr::LogLevel::Debug, false);
        ASSERT(log.addFile("test_fmt.txt"), "add file");
        log.log(cpptypr::LogLevel::Error, "error!");
    }
    FILE* f = fopen("test_fmt.txt", "r");
    ASSERT(f != nullptr, "log file should exist");
    char buf[256]{};
    size_t n = fread(buf, 1, sizeof(buf) - 1, f);
    buf[n] = '\0';
    fclose(f);
    remove("test_fmt.txt");
    ASSERT(strcmp(buf, "[ERROR] error!\n") == 0, "format should match exactly");
    PASS();
}

static void test_output_format_all_levels() {
    TEST("Logger: output format for all levels");
    const char* levels[] = {"DEBUG", "INFO", "WARNING", "ERROR"};
    cpptypr::LogLevel lvls[] = {cpptypr::LogLevel::Debug, cpptypr::LogLevel::Info, cpptypr::LogLevel::Warning, cpptypr::LogLevel::Error};

    for (int i = 0; i < 4; i++) {
        char fname[32];
        snprintf(fname, sizeof(fname), "test_lvl_%d.txt", i);
        {
            cpptypr::Logger log(cpptypr::LogLevel::Debug, false);
            ASSERT(log.addFile(fname), "add file");
            log.log(lvls[i], "msg");
        }
        FILE* f = fopen(fname, "r");
        ASSERT(f != nullptr, "log file should exist");
        char expected[64];
        snprintf(expected, sizeof(expected), "[%s] msg\n", levels[i]);
        char buf[64]{};
        fread(buf, 1, sizeof(buf) - 1, f);
        fclose(f);
        remove(fname);
        ASSERT(strcmp(buf, expected) == 0, "format should match");
    }
    PASS();
}

static void test_log_level_none_suppresses_all() {
    TEST("Logger: LogLevel::None suppresses all messages");
    {
        cpptypr::Logger log(cpptypr::LogLevel::None, false);
        ASSERT(log.addFile("test_none.txt"), "add file");
        log.log(cpptypr::LogLevel::Debug, "debug");
        log.log(cpptypr::LogLevel::Info, "info");
        log.log(cpptypr::LogLevel::Warning, "warning");
        log.log(cpptypr::LogLevel::Error, "error");
    }
    FILE* f = fopen("test_none.txt", "r");
    ASSERT(f != nullptr, "log file should exist");
    char buf[256]{};
    fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);
    remove("test_none.txt");
    ASSERT(buf[0] == '\0', "all messages should be suppressed when level is None");
    PASS();
}

static void test_stdout_enabled_with_file() {
    TEST("Logger: stdout + file both receive messages");
    {
        cpptypr::Logger log(cpptypr::LogLevel::Debug, true);
        ASSERT(log.addFile("test_dual.txt"), "add file");
        log.log(cpptypr::LogLevel::Info, "dual output");
    }
    FILE* f = fopen("test_dual.txt", "r");
    ASSERT(f != nullptr, "log file should exist");
    char buf[128]{};
    fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);
    remove("test_dual.txt");
    ASSERT(strstr(buf, "[INFO]") != nullptr, "file should receive output");
    ASSERT(strstr(buf, "dual output") != nullptr, "file should contain message");
    PASS();
}

static void test_logger_add_file_failure() {
    TEST("Logger: addFile returns false on invalid path");
    cpptypr::Logger log(cpptypr::LogLevel::Debug, false);
#ifdef _WIN32
    bool ok = log.addFile("\\\\invalid_path\\test.log");
#else
    bool ok = log.addFile("/nonexistent_dir_xyz/test.log");
#endif
    ASSERT(!ok, "addFile should fail for invalid path");
    PASS();
}

static void test_move_semantics() {
    TEST("Logger: move semantics");
    cpptypr::Logger log1(cpptypr::LogLevel::Debug, false);
    cpptypr::Logger log2(std::move(log1));
    ASSERT(log2.level() == cpptypr::LogLevel::Debug, "moved logger should retain level");

    cpptypr::Logger log3(cpptypr::LogLevel::Error, false);
    log3 = std::move(log2);
    ASSERT(log3.level() == cpptypr::LogLevel::Debug, "assigned logger should get level from moved");
    PASS();
}

int main() {
    for (int i = 0; i < 4; i++) { char n[32]; snprintf(n, sizeof(n), "test_lvl_%d.txt", i); remove(n); }
    remove("test_log.txt"); remove("test_filter.txt"); remove("test_none.txt");
    remove("test_fmt.txt"); remove("test_log1.txt"); remove("test_log2.txt");
    remove("test_mf1.txt"); remove("test_mf2.txt"); remove("test_mf3.txt");
    remove("test_mf4.txt"); remove("test_dual.txt"); remove("test_default.db");

    printf("=== cpptypr Logger Test Suite ===\n\n");

    test_create_destroy();
    test_default_level();
    test_set_get_level();
    test_log_to_stdout();
    test_log_to_file();
    test_level_filtering();
    test_toggle_stdout();
    test_add_multiple_files();
    test_max_files_limit();
    test_log_output_format();
    test_output_format_all_levels();
    test_log_level_none_suppresses_all();
    test_stdout_enabled_with_file();
    test_logger_add_file_failure();
    test_move_semantics();

    printf("\n=== Results: %d passed, %d failed, %d total ===\n",
           tests_passed, tests_failed, test_count);

    return tests_failed > 0 ? 1 : 0;
}
