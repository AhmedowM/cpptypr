#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>

#include <cpptypr/stats.hpp>

struct EngineSnapshot;

namespace cpptypr {

class Engine;

/** @brief Engine lifecycle states. */
enum class EngineState {
    Idle,    /**< Engine has been created but not started, or has been reset. */
    Running, /**< A typing session is currently active. */
    Paused,  /**< The session has been paused and can be resumed. */
    Error    /**< An error occurred during the session. */
};

/** @brief Reasons a typing session may have stopped. */
enum class StopCause {
    None,     /**< The session has not stopped (still running or not started). */
    Timeout,  /**< The session reached its configured timeout. */
    Finished, /**< All content was typed successfully. */
    User,     /**< The session was explicitly stopped by the user. */
    Error,    /**< An error caused the session to stop. */
    Unknown   /**< The stop reason could not be determined. */
};

/** @brief Convert an EngineState to its lowercase string representation.
 *  @param state The engine state.
 *  @return "idle", "running", "paused", or "error". */
[[nodiscard]] std::string_view toString(EngineState state) noexcept;

/** @brief Convert a StopCause to its lowercase string representation.
 *  @param cause The stop cause.
 *  @return "none", "timeout", "finished", "user", "error", or "unknown". */
[[nodiscard]] std::string_view toString(StopCause cause) noexcept;

/** @brief Atomic point-in-time snapshot of the engine state.
 *
 *  Captures the full session state (text, cursor position, incorrect-keystroke
 *  bitmap, stats, stop cause) in a single call, safe for lock-free UI rendering
 *  or logging. Obtained via Engine::getSnapshot(). */
class Snapshot {
public:
    /** @brief The current content text at the time of the snapshot.
     *  @return View into the session's content string. */
    [[nodiscard]] std::string_view text() const noexcept;

    /** @brief Total length of the content text.
     *  @return Character count of the content. */
    [[nodiscard]] size_t length() const noexcept;

    /** @brief Current cursor position (0-based index into text).
     *  @return The position the next keystroke will be compared against. */
    [[nodiscard]] uint32_t cursorIndex() const noexcept;

    /** @brief The character expected at the current cursor position.
     *  @return The expected character, or '\\0' if at end of content. */
    [[nodiscard]] char expectedChar() const noexcept;

    /** @brief Check whether the keystroke at the given index was incorrect.
     *  @param index 0-based index into the session's keystroke history.
     *  @return true if that position had an incorrect keystroke.
     *  @note Only meaningful in Flow mode; Strict mode never records incorrect
     *        keystrokes since it does not advance past them. */
    [[nodiscard]] bool isIncorrect(size_t index) const;

    /** @brief Session statistics at the time of the snapshot.
     *  @return A SessionStats struct with WPM, accuracy, keystroke counts. */
    [[nodiscard]] SessionStats stats() const noexcept;

    /** @brief The engine state at the time of the snapshot.
     *  @return Idle, Running, Paused, or Error. */
    [[nodiscard]] EngineState state() const noexcept;

    /** @brief The reason the session stopped (if applicable).
     *  @return StopCause indicating None, Timeout, Finished, User, Error, or Unknown. */
    [[nodiscard]] StopCause stopCause() const noexcept;

    ~Snapshot();
    Snapshot(Snapshot&&) noexcept;
    Snapshot& operator=(Snapshot&&) noexcept;

private:
    friend Engine;
    explicit Snapshot(const ::EngineSnapshot& snap);
    std::unique_ptr<::EngineSnapshot> m_snap;
};

}
