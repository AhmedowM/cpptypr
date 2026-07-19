#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <cpptypr/error.hpp>

struct Repository;

namespace cpptypr {

class Logger;
class Engine;

/** @brief A single saved typing session. */
struct SessionData {
    int64_t id;            /**< Unique session identifier. */
    std::string timestamp; /**< ISO-formatted creation timestamp. */
    std::string mode;      /**< Engine mode used during the session. */
    int64_t totalChars;    /**< Total characters typed. */
    int64_t correctChars;  /**< Correctly typed characters. */
    int64_t durationMs;    /**< Session duration in milliseconds. */
    double wpm;            /**< Net words-per-minute. */
    double wpmRaw;         /**< Raw words-per-minute. */
    double accuracy;       /**< Accuracy ratio (0.0–1.0). */
};

/** @brief RAII wrapper that persists session data to a SQLite database.
 *
 * Provides CRUD operations for typing sessions and convenience queries
 * for best scores and averages. The underlying ::Repository is owned
 * by this wrapper.
 *
 * @note Non-copyable but movable. */
class Repository {
public:
    /** @brief Open (or create) a database at the given file path.
     *  @param dbPath Path to the SQLite database file. */
    explicit Repository(std::string_view dbPath);
    ~Repository();

    Repository(const Repository&) = delete;
    Repository& operator=(const Repository&) = delete;

    Repository(Repository&&) noexcept;
    Repository& operator=(Repository&&) noexcept;

    /** @brief Persist a session and return its assigned ID.
     *  @param data The session data to save.
     *  @return The auto-generated session ID. */
    int64_t saveSession(const SessionData& data);

    /** @brief Retrieve a session by ID.
     *  @param id The session identifier.
     *  @return The SessionData, or std::nullopt if not found. */
    std::optional<SessionData> getSession(int64_t id);

    /** @brief Get all sessions in the database.
     *  @return A vector of all saved sessions. */
    std::vector<SessionData> getAll();

    /** @brief Get the most recent sessions.
     *  @param limit Maximum number of sessions to return.
     *  @return A vector of up to `limit` sessions, most recent first. */
    std::vector<SessionData> getRecent(int64_t limit);

    /** @brief Get the total number of stored sessions.
     *  @return Session count. */
    int64_t count();

    /** @brief Delete a session by ID.
     *  @param id The session identifier to remove.
     *  @return true if a session was deleted, false if not found. */
    bool deleteSession(int64_t id);

    /** @brief Remove all sessions from the database. */
    void clearAll();

    /** @brief Get the session with the highest net WPM.
     *  @return The best session, or std::nullopt if the database is empty. */
    std::optional<SessionData> bestWpm();

    /** @brief Get the session with the highest raw WPM.
     *  @return The best raw session, or std::nullopt if the database is empty. */
    std::optional<SessionData> bestRawWpm();

    /** @brief Get the average net WPM across all sessions.
     *  @return Average WPM, or 0.0 if there are no sessions. */
    double averageWpm();

private:
    friend class Engine;
    ::Repository* m_impl;
};

}
