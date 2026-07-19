#include <cpptypr.hpp>

#include <cmath>
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

static std::string TEST_DB_PATH = "test_repository.db";

static void cleanupTestDb() {
    remove(TEST_DB_PATH.c_str());
    remove("test_repository.db-journal");
    remove("test_repository.db-shm");
    remove("test_repository.db-wal");
}

static cpptypr::SessionData makeSession(const char* mode = "strict") {
    cpptypr::SessionData s{};
    s.id = 0;
    s.timestamp = "2026-07-09T12:00:00+0000";
    s.mode = mode;
    s.totalChars = 100;
    s.correctChars = 95;
    s.durationMs = 60000;
    s.wpm = 78.5;
    s.wpmRaw = 83.2;
    s.accuracy = 95.0;
    return s;
}

// ===== Save & Get =====

static void test_save_and_get_session() {
    TEST("Repository: saveSession and getSession");
    cleanupTestDb();
    {
        cpptypr::Repository repo(TEST_DB_PATH);
        auto saved = makeSession();
        int64_t id = repo.saveSession(saved);
        ASSERT(id > 0, "saveSession should return positive ID");

        auto retrieved = repo.getSession(id);
        ASSERT(retrieved.has_value(), "getSession should return value");
        ASSERT(retrieved->id == id, "retrieved ID should match");
        ASSERT(retrieved->mode == "strict", "mode should match");
        ASSERT(retrieved->totalChars == 100, "totalChars should match");
        ASSERT(retrieved->correctChars == 95, "correctChars should match");
        ASSERT(retrieved->durationMs == 60000, "durationMs should match");
        ASSERT(fabs(retrieved->wpm - 78.5) < 0.001, "wpm should match");
        ASSERT(fabs(retrieved->wpmRaw - 83.2) < 0.001, "wpmRaw should match");
        ASSERT(fabs(retrieved->accuracy - 95.0) < 0.001, "accuracy should match");
    }
    cleanupTestDb();
    PASS();
}

static void test_get_session_nonexistent() {
    TEST("Repository: getSession returns nullopt for nonexistent ID");
    cleanupTestDb();
    {
        cpptypr::Repository repo(TEST_DB_PATH);
        auto result = repo.getSession(999);
        ASSERT(!result.has_value(), "nonexistent session should return nullopt");
    }
    cleanupTestDb();
    PASS();
}

// ===== GetAll =====

static void test_get_all_sessions() {
    TEST("Repository: getAll returns sessions in descending order");
    cleanupTestDb();
    {
        cpptypr::Repository repo(TEST_DB_PATH);
        auto id1 = repo.saveSession(makeSession("strict"));
        auto id2 = repo.saveSession(makeSession("flow"));
        auto id3 = repo.saveSession(makeSession("strict"));

        auto all = repo.getAll();
        ASSERT(all.size() == 3, "should have 3 sessions");
        ASSERT(all[0].id == id3, "first should be newest");
        ASSERT(all[1].id == id2, "second should be middle");
        ASSERT(all[2].id == id1, "third should be oldest");
    }
    cleanupTestDb();
    PASS();
}

// ===== GetRecent =====

static void test_get_recent_sessions() {
    TEST("Repository: getRecent returns limited results");
    cleanupTestDb();
    {
        cpptypr::Repository repo(TEST_DB_PATH);
        for (int i = 0; i < 5; i++) {
            repo.saveSession(makeSession("strict"));
        }
        auto recent = repo.getRecent(3);
        ASSERT(recent.size() == 3, "should return 3 sessions");
    }
    cleanupTestDb();
    PASS();
}

// ===== Count =====

static void test_get_count() {
    TEST("Repository: count returns correct count");
    cleanupTestDb();
    {
        cpptypr::Repository repo(TEST_DB_PATH);
        ASSERT(repo.count() == 0, "count should be 0 initially");
        repo.saveSession(makeSession());
        ASSERT(repo.count() == 1, "count should be 1");
        repo.saveSession(makeSession());
        ASSERT(repo.count() == 2, "count should be 2");
    }
    cleanupTestDb();
    PASS();
}

// ===== Delete =====

static void test_delete_session() {
    TEST("Repository: deleteSession removes session");
    cleanupTestDb();
    {
        cpptypr::Repository repo(TEST_DB_PATH);
        int64_t id = repo.saveSession(makeSession());
        ASSERT(repo.count() == 1, "count should be 1");
        ASSERT(repo.deleteSession(id), "delete should succeed");
        ASSERT(repo.count() == 0, "count should be 0 after delete");
        ASSERT(!repo.deleteSession(id), "deleting nonexistent should return false");
    }
    cleanupTestDb();
    PASS();
}

// ===== ClearAll =====

static void test_clear_all() {
    TEST("Repository: clearAll removes all sessions");
    cleanupTestDb();
    {
        cpptypr::Repository repo(TEST_DB_PATH);
        repo.saveSession(makeSession());
        repo.saveSession(makeSession());
        repo.saveSession(makeSession());
        ASSERT(repo.count() == 3, "count should be 3");
        repo.clearAll();
        ASSERT(repo.count() == 0, "count should be 0 after clear");
    }
    cleanupTestDb();
    PASS();
}

// ===== Best WPM =====

static void test_get_best_wpm() {
    TEST("Repository: bestWpm returns highest WPM session");
    cleanupTestDb();
    {
        cpptypr::Repository repo(TEST_DB_PATH);
        auto s1 = makeSession("strict");
        s1.wpm = 60.0;
        repo.saveSession(s1);
        auto s2 = makeSession("flow");
        s2.wpm = 90.0;
        repo.saveSession(s2);
        auto best = repo.bestWpm();
        ASSERT(best.has_value(), "bestWpm should return a session");
        ASSERT(fabs(best->wpm - 90.0) < 0.001, "best WPM should be 90.0");
    }
    cleanupTestDb();
    PASS();
}

static void test_get_best_raw_wpm() {
    TEST("Repository: bestRawWpm returns highest raw WPM session");
    cleanupTestDb();
    {
        cpptypr::Repository repo(TEST_DB_PATH);
        auto s1 = makeSession("strict");
        s1.wpmRaw = 70.0;
        repo.saveSession(s1);
        auto s2 = makeSession("flow");
        s2.wpmRaw = 95.0;
        repo.saveSession(s2);
        auto best = repo.bestRawWpm();
        ASSERT(best.has_value(), "bestRawWpm should return a session");
        ASSERT(fabs(best->wpmRaw - 95.0) < 0.001, "best raw WPM should be 95.0");
    }
    cleanupTestDb();
    PASS();
}

// ===== Average WPM =====

static void test_get_average_wpm() {
    TEST("Repository: averageWpm calculates correctly");
    cleanupTestDb();
    {
        cpptypr::Repository repo(TEST_DB_PATH);
        ASSERT(repo.averageWpm() == 0.0, "avg should be 0.0 for empty DB");
        auto avg1 = makeSession("strict");
        avg1.wpm = 60.0;
        repo.saveSession(avg1);
        auto avg2 = makeSession("flow");
        avg2.wpm = 80.0;
        repo.saveSession(avg2);
        double avg = repo.averageWpm();
        ASSERT(fabs(avg - 70.0) < 0.001, "avg should be 70.0");
    }
    cleanupTestDb();
    PASS();
}

// ===== Move semantics =====

static void test_repository_move() {
    TEST("Repository: move semantics");
    cleanupTestDb();
    {
        cpptypr::Repository repo1(TEST_DB_PATH);
        repo1.saveSession(makeSession());
        cpptypr::Repository repo2(std::move(repo1));
        ASSERT(repo2.count() == 1, "moved repo should retain data");
    }
    cleanupTestDb();
    PASS();
}

int main() {
    printf("=== cpptypr Repository Test Suite ===\n\n");

    test_save_and_get_session();
    test_get_session_nonexistent();
    test_get_all_sessions();
    test_get_recent_sessions();
    test_get_count();
    test_delete_session();
    test_clear_all();
    test_get_best_wpm();
    test_get_best_raw_wpm();
    test_get_average_wpm();
    test_repository_move();

    printf("\n=== Results: %d passed, %d failed, %d total ===\n",
           tests_passed, tests_failed, test_count);

    return tests_failed > 0 ? 1 : 0;
}
