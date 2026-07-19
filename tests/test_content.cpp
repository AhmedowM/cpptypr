#include "db_helpers.hpp"
#include <cpptypr.hpp>

#include <cstdio>
#include <cstring>
#include <string>

static int tests_passed = 0;
static int tests_failed = 0;
static int test_count = 0;

#define TEST(name) do { \
    test_count++; \
    fprintf(stderr, "  TEST %d: %s ... ", test_count, name); \
    fflush(stderr); \
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

// ===== String provider =====

static void test_string_provider() {
    TEST("String provider: create and get content");
    auto cp = cpptypr::ContentProvider::fromString("Hello World");
    auto chunk = cp.getNext();
    ASSERT(chunk.text == "Hello World", "text should match");
    ASSERT(chunk.text.size() == 11, "length should be 11");
    PASS();
}

static void test_string_provider_empty() {
    TEST("String provider: empty string");
    auto cp = cpptypr::ContentProvider::fromString("");
    auto chunk = cp.getNext();
    ASSERT(chunk.text.empty(), "text should be empty");
    PASS();
}

static void test_string_provider_reset() {
    TEST("String provider: reset");
    auto cp = cpptypr::ContentProvider::fromString("Test");
    cp.reset();
    ASSERT(!cp.isExhausted(), "should not be exhausted after reset");
    PASS();
}

// ===== File provider =====

static void test_file_provider() {
    TEST("File provider: create and read file");
    FILE* f = fopen("test_content.txt", "wb");
    if (!f) {
        printf("SKIPPED (cannot create test file)\n");
        return;
    }
    const char* testText = "File content test";
    fwrite(testText, 1, strlen(testText), f);
    fclose(f);

    auto cp = cpptypr::ContentProvider::fromFile("test_content.txt");
    auto chunk = cp.getNext();
    ASSERT(chunk.text == "File content test", "text should match file content");
    ASSERT(chunk.text.size() == 17, "length should be 17");

    remove("test_content.txt");
    PASS();
}

static void test_file_provider_missing() {
    TEST("File provider: missing file returns empty");
    auto cp = cpptypr::ContentProvider::fromFile("nonexistent_file.txt");
    auto chunk = cp.getNext();
    ASSERT(chunk.text.empty(), "length should be 0 for missing file");
    PASS();
}

// ===== Web provider =====

static void test_web_provider() {
    TEST("Web provider: fallback to default string");
    auto cp = cpptypr::ContentProvider::fromWeb("http://example.com");
    auto chunk = cp.getNext();
    ASSERT(chunk.text == "The quick brown fox jumps over the lazy dog.",
           "web provider should fallback to default string");
    PASS();
}

// ===== Database provider =====

static void test_database_common_words() {
    TEST("Database provider: common words mode");
    createTestWordDb();
    auto cp = cpptypr::ContentProvider::fromDatabase("test_content_words.db");
    cp.setMode(cpptypr::ContentMode::CommonWords);
    cp.setContentLimit(10);
    auto chunk = cp.getNext();
    ASSERT(!chunk.text.empty(), "should have content");
    ASSERT(chunk.text.find("the") != std::string::npos, "should contain 'the'");
    ASSERT(chunk.text.find("quick") != std::string::npos, "should contain 'quick'");
    cleanupTestDb("test_content_words.db");
    PASS();
}

static void test_database_random_words() {
    TEST("Database provider: random words mode");
    createTestWordDb();
    auto cp = cpptypr::ContentProvider::fromDatabase("test_content_words.db");
    cp.setMode(cpptypr::ContentMode::RandomWords);
    cp.setContentLimit(10);
    auto chunk = cp.getNext();
    ASSERT(!chunk.text.empty(), "should have content");
    ASSERT(chunk.text.find("jumps") != std::string::npos, "should contain 'jumps'");
    cleanupTestDb("test_content_words.db");
    PASS();
}

static void test_database_sentences() {
    TEST("Database provider: sentences mode");
    createTestSentenceDb();
    auto cp = cpptypr::ContentProvider::fromDatabase("test_content_sentences.db");
    cp.setMode(cpptypr::ContentMode::Sentences);
    cp.setContentLimit(10);
    auto chunk = cp.getNext();
    ASSERT(!chunk.text.empty(), "should have content");
    ASSERT(chunk.text.find("quick brown fox") != std::string::npos, "should contain sentence text");
    cleanupTestDb("test_content_sentences.db");
    PASS();
}

static void test_database_empty_table() {
    TEST("Database provider: empty table returns empty");
    createEmptyWordDb();
    auto cp = cpptypr::ContentProvider::fromDatabase("test_content_words.db");
    auto chunk = cp.getNext();
    ASSERT(chunk.text.empty(), "should be empty for empty table");
    cleanupTestDb("test_content_words.db");
    PASS();
}

static void test_database_missing_file() {
    TEST("Database provider: missing file returns empty");
    cleanupTestDb("nonexistent_test.db");
    auto cp = cpptypr::ContentProvider::fromDatabase("nonexistent_test.db");
    auto chunk = cp.getNext();
    ASSERT(chunk.text.empty(), "should be empty for missing file");
    PASS();
}

static void test_database_reset() {
    TEST("Database provider: reset allows re-fetch");
    createTestWordDb();
    auto cp = cpptypr::ContentProvider::fromDatabase("test_content_words.db");
    cp.setMode(cpptypr::ContentMode::CommonWords);
    cp.setContentLimit(10);
    auto chunk1 = cp.getNext();
    ASSERT(!chunk1.text.empty(), "first fetch should have content");
    auto chunk2 = cp.getNext();
    ASSERT(chunk2.text.empty(), "second fetch should return empty (exhausted)");
    cp.reset();
    auto chunk3 = cp.getNext();
    ASSERT(!chunk3.text.empty(), "after reset should have content again");
    ASSERT(chunk3.text == chunk1.text, "content should match after reset");
    cleanupTestDb("test_content_words.db");
    PASS();
}

static void test_database_content_limit() {
    TEST("Database provider: content limit respected");
    createTestWordDb();
    auto cp = cpptypr::ContentProvider::fromDatabase("test_content_words.db");
    cp.setMode(cpptypr::ContentMode::CommonWords);
    cp.setContentLimit(2);
    auto chunk = cp.getNext();
    ASSERT(!chunk.text.empty(), "should have content");
    int wordCount = 0;
    for (size_t i = 0; i < chunk.text.size(); i++) {
        if (chunk.text[i] == ' ') wordCount++;
    }
    wordCount++;
    ASSERT(wordCount <= 2, "should have at most 2 words");
    cleanupTestDb("test_content_words.db");
    PASS();
}

// ===== Provider lifecycle =====

static void test_provider_reset() {
    TEST("Provider: reset clears state");
    auto cp = cpptypr::ContentProvider::fromString("Reset test");
    auto chunk1 = cp.getNext();
    ASSERT(!chunk1.text.empty(), "should have content");
    cp.reset();
    auto chunk2 = cp.getNext();
    ASSERT(chunk2.text.size() == chunk1.text.size(), "length should match after reset");
    ASSERT(chunk2.text == chunk1.text, "text should match after reset");
    PASS();
}

static void test_provider_move() {
    TEST("Provider: move semantics");
    auto cp1 = cpptypr::ContentProvider::fromString("Move test");
    auto cp2 = std::move(cp1);
    auto chunk = cp2.getNext();
    ASSERT(chunk.text == "Move test", "moved provider should work");
    PASS();
}

static void test_database_default_mode() {
    TEST("Database provider: default mode is common words");
    auto cp = cpptypr::ContentProvider::fromDatabase("test_default.db");
    auto chunk = cp.getNext();
    ASSERT(chunk.text.empty(), "empty DB means empty content");
    remove("test_default.db");
    PASS();
}

int main() {
    printf("=== cpptypr Content Provider Test Suite ===\n\n");

    test_string_provider();
    test_string_provider_empty();
    test_string_provider_reset();

    test_file_provider();
    test_file_provider_missing();

    test_web_provider();

    test_database_common_words();
    test_database_random_words();
    test_database_sentences();
    test_database_empty_table();
    test_database_missing_file();
    test_database_reset();
    test_database_content_limit();
    test_database_default_mode();

    test_provider_reset();
    test_provider_move();

    fprintf(stderr, "\n=== Results: %d passed, %d failed, %d total ===\n",
           tests_passed, tests_failed, test_count);

    return tests_failed > 0 ? 1 : 0;
}
