#include "db_helpers.hpp"
#include <sqlite3.h>
#include <cstdio>
#include <cstring>
#include <string>

static int execSql(sqlite3* db, const char* sql) {
    char* err = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &err);
    if (rc != SQLITE_OK && err) {
        printf("SQL error: %s\n", err);
        sqlite3_free(err);
    }
    return rc;
}

void createTestWordDb() {
    const char* TEST_DB = "test_content_words.db";
    remove(TEST_DB);
    sqlite3* db = nullptr;
    if (sqlite3_open(TEST_DB, &db) != SQLITE_OK) return;
    execSql(db, "CREATE TABLE IF NOT EXISTS common_words ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, word TEXT NOT NULL UNIQUE,"
        "word_length INTEGER NOT NULL, frequency_rank INTEGER NOT NULL)");
    execSql(db, "CREATE TABLE IF NOT EXISTS random_words ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, word TEXT NOT NULL UNIQUE,"
        "word_length INTEGER NOT NULL, difficulty_rating REAL DEFAULT 1.0)");
    execSql(db, "INSERT INTO common_words (word, word_length, frequency_rank) VALUES ('the', 3, 1)");
    execSql(db, "INSERT INTO common_words (word, word_length, frequency_rank) VALUES ('quick', 5, 2)");
    execSql(db, "INSERT INTO common_words (word, word_length, frequency_rank) VALUES ('brown', 5, 3)");
    execSql(db, "INSERT INTO common_words (word, word_length, frequency_rank) VALUES ('fox', 3, 4)");
    execSql(db, "INSERT INTO random_words (word, word_length) VALUES ('jumps', 5)");
    execSql(db, "INSERT INTO random_words (word, word_length) VALUES ('over', 4)");
    execSql(db, "INSERT INTO random_words (word, word_length) VALUES ('lazy', 4)");
    execSql(db, "INSERT INTO random_words (word, word_length) VALUES ('dog', 3)");
    sqlite3_close(db);
}

void createTestSentenceDb() {
    const char* TEST_SENTENCE_DB = "test_content_sentences.db";
    remove(TEST_SENTENCE_DB);
    sqlite3* db = nullptr;
    if (sqlite3_open(TEST_SENTENCE_DB, &db) != SQLITE_OK) return;
    execSql(db, "CREATE TABLE IF NOT EXISTS typing_sentences ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, text_content TEXT NOT NULL,"
        "char_count INTEGER NOT NULL, word_count INTEGER NOT NULL,"
        "source_title TEXT NOT NULL, source_author TEXT DEFAULT 'Unknown',"
        "difficulty_category TEXT DEFAULT 'Normal')");
    execSql(db, "INSERT INTO typing_sentences (text_content, char_count, word_count, source_title) "
        "VALUES ('The quick brown fox.', 20, 4, 'Test')");
    execSql(db, "INSERT INTO typing_sentences (text_content, char_count, word_count, source_title) "
        "VALUES ('Jumps over the lazy dog.', 24, 5, 'Test')");
    sqlite3_close(db);
}

void createEmptyWordDb() {
    const char* path = "test_content_words.db";
    remove(path);
    sqlite3* db = nullptr;
    if (sqlite3_open(path, &db) != SQLITE_OK) return;
    execSql(db, "CREATE TABLE common_words ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, word TEXT NOT NULL UNIQUE,"
        "word_length INTEGER NOT NULL, frequency_rank INTEGER NOT NULL)");
    sqlite3_close(db);
}

void cleanupTestDb(const char* path) {
    remove(path);
    std::string base = path;
    remove((base + "-journal").c_str());
    remove((base + "-shm").c_str());
    remove((base + "-wal").c_str());
}
