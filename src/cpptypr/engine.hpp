#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <memory>
#include <string_view>

#include <cpptypr/error.hpp>
#include <cpptypr/stats.hpp>
#include <cpptypr/snapshot.hpp>

struct Engine;

namespace cpptypr {

class ContentProvider;
class Repository;
class Logger;
class Snapshot;

/** @brief Typing modes supported by the engine. */
enum class EngineMode {
    Strict, /**< Every keystroke must match the expected character. */
    Flow    /**< Allows typos and continues; measures raw vs. corrected speed. */
};

/** @brief Convert an EngineMode to its lowercase string representation.
 *  @param mode The engine mode.
 *  @return "strict" or "flow". */
[[nodiscard]] std::string_view toString(EngineMode mode) noexcept;

/** @brief Parse a case-insensitive string to an EngineMode.
 *  @param s One of "strict" or "flow".
 *  @return The matching EngineMode.
 *  @throws Error if the string does not match any valid mode. */
[[nodiscard]] EngineMode engineModeFromString(std::string_view s);

/** @brief Write an EngineMode to an output stream.
 *  @param os   The output stream.
 *  @param mode The engine mode.
 *  @return The output stream. */
std::ostream& operator<<(std::ostream& os, EngineMode mode);

/** @brief RAII handle that disconnects an event callback on destruction.
 *
 * Returned by Engine::onStarted, Engine::onFinished, etc.
 * Automatically disconnects the callback when the handle goes out of scope,
 * preventing dangling callbacks after the handle is discarded. */
class CallbackHandle {
public:
    /** @brief Construct a null (disconnected) handle. */
    CallbackHandle() = default;
    ~CallbackHandle();

    CallbackHandle(CallbackHandle&&) noexcept;
    CallbackHandle& operator=(CallbackHandle&&) noexcept;

    CallbackHandle(const CallbackHandle&) = delete;
    CallbackHandle& operator=(const CallbackHandle&) = delete;

    /** @brief Manually disconnect the callback before destruction. */
    void disconnect();

    /** @brief Check whether this handle refers to an active connection.
     *  @return true if the handle is connected to a registered callback. */
    [[nodiscard]] explicit operator bool() const noexcept { return m_cb != nullptr; }

    CallbackHandle(::Engine* engine, int event, int slotId, void* cb);

private:
    friend class Engine;

    ::Engine* m_engine = nullptr;
    int m_event = 0;
    int m_slotId = -1;
    void* m_cb = nullptr;
};

/** @brief RAII wrapper around the C typing engine.
 *
 * Manages the full lifecycle: creation, start/stop, pause/resume,
 * keystroke input, statistics, event callbacks, and cleanup.
 *
 * The Engine owns the underlying C engine instance. It must be provided
 * with a ContentProvider (and optionally a Logger and Repository for
 * auto-save) before starting.
 *
 * @note Non-copyable, movable. After a move the source is left empty. */
class Engine {
public:
    /** @brief Construct an engine in the given mode with a content provider.
     *  @param mode     The typing mode (Strict or Flow).
     *  @param provider Content provider; must outlive the engine.
     *  @param timeout  Session timeout in seconds. 0 means no timeout. */
    explicit Engine(EngineMode mode, ContentProvider& provider, uint16_t timeout = 0);

    /** @brief Construct an engine with a string-based mode.
     *  @param mode     Case-insensitive mode string ("strict" or "flow").
     *  @param provider Content provider; must outlive the engine.
     *  @param timeout  Session timeout in seconds. 0 means no timeout.
     *  @throws Error if the mode string is invalid. */
    explicit Engine(std::string_view mode, ContentProvider& provider, uint16_t timeout = 0);
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    Engine(Engine&&) noexcept;
    Engine& operator=(Engine&&) noexcept;

    /** @brief Get a point-in-time snapshot of the engine state.
     *  @return A Snapshot containing text, cursor, stats, and flags. */
    [[nodiscard]] Snapshot getSnapshot();

    // ---- Lifecycle ----

    /** @brief Start the typing session. Blocks until the session ends. */
    void start();

    /** @brief Stop a running session. */
    void stop();

    /** @brief Pause a running session. */
    void pause();

    /** @brief Resume a paused session. */
    void resume();

    /** @brief Reset the engine to its initial idle state. */
    void reset();

    // ---- Input ----

    /** @brief Feed a single character keystroke into the engine.
     *  @param key The character pressed. */
    void keyPress(char key);

    /** @brief Signal a backspace (undo the last keystroke if allowed by mode). */
    void backspacePress();

    // ---- State queries ----

    /** @brief Check if the engine is currently running.
     *  @return true if the session is active. */
    [[nodiscard]] bool isRunning() const;

    /** @brief Check if the engine is paused.
     *  @return true if the session is paused. */
    [[nodiscard]] bool isPaused() const;

    /** @brief Check if the engine is idle (not started).
     *  @return true if the session has not started or has been reset. */
    [[nodiscard]] bool isIdle() const;

    /** @brief Check if the engine is in an error state.
     *  @return true if an error occurred. */
    [[nodiscard]] bool isError() const;

    /** @brief Check if the session has completed successfully.
     *  @return true if all content was typed. */
    [[nodiscard]] bool isCompleted() const;

    /** @brief Check if the session timed out.
     *  @return true if the timeout was reached. */
    [[nodiscard]] bool isTimedOut() const;

    /** @brief Check if the session was explicitly stopped.
     *  @return true if stop() was called. */
    [[nodiscard]] bool isStopped() const;

    /** @brief Check if the session was stopped by the user.
     *  @return true if stop() was called (as opposed to completion or timeout). */
    [[nodiscard]] bool wasStopped() const;

    // ---- Statistics ----

    /** @brief Get a snapshot of the current session statistics.
     *  @return SessionStats with the latest metrics. */
    [[nodiscard]] SessionStats stats() const;

    // ---- Configuration ----

    /** @brief Change the engine mode.
     *  @param mode The new engine mode. Only valid when idle. */
    void setMode(EngineMode mode);

    /** @brief Change the engine mode using a case-insensitive string.
     *  @param mode "strict" or "flow".
     *  @throws Error if the string is invalid. */
    void setMode(std::string_view mode);

    /** @brief Get the current engine mode.
     *  @return The active EngineMode. */
    [[nodiscard]] EngineMode mode() const;

    /** @brief Set the session timeout.
     *  @param seconds Timeout in seconds. 0 disables the timeout. */
    void setTimeout(uint16_t seconds);

    /** @brief Get the current session timeout.
     *  @return Timeout in seconds. 0 means no timeout. */
    [[nodiscard]] uint16_t timeout() const;

    /** @brief Set the content provider.
     *  @param provider The content source; must outlive the engine. */
    void setContentProvider(ContentProvider& provider);

    /** @brief Remove the content provider; the engine goes idle. */
    void clearContentProvider();

    /** @brief Enable or disable automatic session saving.
     *  @param repo    The repository to save to.
     *  @param enabled true to enable auto-save. */
    void setAutoSave(Repository& repo, bool enabled);

    /** @brief Disable auto-save. */
    void clearAutoSave();

    /** @brief Attach a logger.
     *  @param logger The logger instance; must outlive the engine. */
    void setLogger(Logger& logger);

    /** @brief Remove the attached logger (falls back to default logging). */
    void resetLogger();

    // ---- Event callbacks ----
    // Each returns a CallbackHandle that disconnects on destruction.
    // The callback is invoked with no arguments when the event fires.

    /** @brief Register a callback for when the session starts.
     *  @param cb Callback to invoke.
     *  @return A CallbackHandle that disconnects on destruction. */
    [[nodiscard]] CallbackHandle onStarted(std::function<void()> cb);

    /** @brief Register a callback for when the session stops.
     *  @param cb Callback to invoke.
     *  @return A CallbackHandle that disconnects on destruction. */
    [[nodiscard]] CallbackHandle onStopped(std::function<void()> cb);

    /** @brief Register a callback for when the session finishes.
     *  @param cb Callback to invoke.
     *  @return A CallbackHandle that disconnects on destruction. */
    [[nodiscard]] CallbackHandle onFinished(std::function<void()> cb);

    /** @brief Register a callback for when the session is paused.
     *  @param cb Callback to invoke.
     *  @return A CallbackHandle that disconnects on destruction. */
    [[nodiscard]] CallbackHandle onPaused(std::function<void()> cb);

    /** @brief Register a callback for when the session resumes.
     *  @param cb Callback to invoke.
     *  @return A CallbackHandle that disconnects on destruction. */
    [[nodiscard]] CallbackHandle onResumed(std::function<void()> cb);

    /** @brief Register a callback for when the session times out.
     *  @param cb Callback to invoke.
     *  @return A CallbackHandle that disconnects on destruction. */
    [[nodiscard]] CallbackHandle onTimeout(std::function<void()> cb);

    /** @brief Register a callback for a correct keystroke.
     *  @param cb Callback to invoke.
     *  @return A CallbackHandle that disconnects on destruction. */
    [[nodiscard]] CallbackHandle onCorrectKeystroke(std::function<void()> cb);

    /** @brief Register a callback for an incorrect keystroke.
     *  @param cb Callback to invoke.
     *  @return A CallbackHandle that disconnects on destruction. */
    [[nodiscard]] CallbackHandle onIncorrectKeystroke(std::function<void()> cb);

    /** @brief Register a callback for a backspace press.
     *  @param cb Callback to invoke.
     *  @return A CallbackHandle that disconnects on destruction. */
    [[nodiscard]] CallbackHandle onBackspace(std::function<void()> cb);

    /** @brief Register a callback for segment completion.
     *  @param cb Callback to invoke.
     *  @return A CallbackHandle that disconnects on destruction. */
    [[nodiscard]] CallbackHandle onSegmentCompleted(std::function<void()> cb);

    /** @brief Register a callback for an error event.
     *  @param cb Callback to invoke.
     *  @return A CallbackHandle that disconnects on destruction. */
    [[nodiscard]] CallbackHandle onError(std::function<void()> cb);

private:
    ::Engine* m_impl;
    std::unique_ptr<class Logger> m_defaultLogger;
    class Logger* m_logger;
};

}
