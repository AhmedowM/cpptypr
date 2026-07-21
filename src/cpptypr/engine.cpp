#include <engine.h>
#include <callback.h>
#include <repository.h>
#include <state.h>
#include <stats.h>
#include <snapshot.h>

#include <cpptypr/engine.hpp>
#include <cpptypr/content.hpp>
#include <cpptypr/repository.hpp>
#include <cpptypr/logger.hpp>
#include <cpptypr/detail.hpp>

#include <string>
#include <utility>

namespace cpptypr {

namespace {

void callbackTrampoline(::Engine*, void* userData) {
    auto cb = static_cast<std::function<void()>*>(userData);
    (*cb)();
}

}

std::string_view toString(EngineMode mode) noexcept {
    switch (mode) {
        case EngineMode::Strict: return "strict";
        case EngineMode::Flow:   return "flow";
    }
    return "strict";
}

namespace detail {

EngineMode parseEngineMode(std::string_view s) {
    auto lower = cpptypr::detail::toLower(s);
    if (lower == "strict") return EngineMode::Strict;
    if (lower == "flow")   return EngineMode::Flow;
    throw Error(ErrorCode::InvalidMode);
}

}

Engine::Engine(std::string_view mode, ContentProvider& provider, uint16_t timeout)
    : Engine(detail::parseEngineMode(mode), provider, timeout) {}

Engine::Engine(EngineMode mode, ContentProvider& provider, uint16_t timeout)
    : m_impl([&] {
        ::EngineConfig config{};
        config.mode = static_cast<::EngineMode>(mode);
        config.timeout = timeout;
        config.contentProvider = provider.m_impl;
        return ::engineCreate(&config);
    }()),
    m_defaultLogger(std::make_unique<Logger>(LogLevel::Warning, false)),
    m_logger(m_defaultLogger.get())
{
    ::engineSetLogger(m_impl, static_cast<::Logger*>(m_logger->m_impl));
}

Engine::~Engine() { if (m_impl) { ::engineDestroy(m_impl); } }

Engine::Engine(Engine&& other) noexcept
    : m_impl(other.m_impl),
      m_defaultLogger(std::move(other.m_defaultLogger)),
      m_logger(other.m_logger)
{
    other.m_impl = nullptr;
    other.m_logger = nullptr;
}

Engine& Engine::operator=(Engine&& other) noexcept {
    if (this != &other) {
        if (m_impl) { ::engineDestroy(m_impl); }
        m_impl = other.m_impl;
        m_defaultLogger = std::move(other.m_defaultLogger);
        m_logger = other.m_logger;
        other.m_impl = nullptr;
        other.m_logger = nullptr;
    }
    return *this;
}

void Engine::start() { CHECK_MOVED(); ::engineStart(m_impl); }
void Engine::stop() { CHECK_MOVED(); ::engineStop(m_impl); }
void Engine::pause() { CHECK_MOVED(); ::enginePause(m_impl); }
void Engine::resume() { CHECK_MOVED(); ::engineResume(m_impl); }
void Engine::reset() { CHECK_MOVED(); ::engineReset(m_impl); }

void Engine::keyPress(char key) { CHECK_MOVED(); ::engineKeyPress(m_impl, key); }
void Engine::backspacePress() { CHECK_MOVED(); ::engineBackspacePress(m_impl); }

bool Engine::isRunning() const { CHECK_MOVED(); return ::engineIsRunning(m_impl); }
bool Engine::isPaused() const { CHECK_MOVED(); return ::engineIsPaused(m_impl); }
bool Engine::isIdle() const { CHECK_MOVED(); return ::engineIsIdle(m_impl); }
bool Engine::isError() const { CHECK_MOVED(); return ::engineIsError(m_impl); }
bool Engine::isCompleted() const { CHECK_MOVED(); return ::engineIsCompleted(m_impl); }
bool Engine::isTimedOut() const { CHECK_MOVED(); return ::engineIsTimedOut(m_impl); }
bool Engine::isStopped() const { CHECK_MOVED(); return ::engineIsStopped(m_impl); }
bool Engine::wasStopped() const { CHECK_MOVED(); return ::engineWasStopped(m_impl); }

SessionStats Engine::stats() const {
    CHECK_MOVED();
    auto s = ::engineGetStats(m_impl);
    return { std::chrono::milliseconds(s.durationMs), s.correctKeystrokes, s.incorrectKeystrokes, s.totalKeystrokes, s.accuracy, s.wpm, s.wpmRaw };
}

void Engine::setMode(EngineMode mode) { CHECK_MOVED(); ::engineSetMode(m_impl, static_cast<::EngineMode>(mode)); }
void Engine::setMode(std::string_view mode) { CHECK_MOVED(); setMode(detail::parseEngineMode(mode)); }

EngineMode Engine::mode() const { CHECK_MOVED(); return static_cast<EngineMode>(::engineGetMode(m_impl)); }

void Engine::setTimeout(uint16_t seconds) { CHECK_MOVED(); ::engineSetTimeout(m_impl, seconds); }

uint16_t Engine::timeout() const { CHECK_MOVED(); return ::engineGetTimeout(m_impl); }

void Engine::setContentProvider(ContentProvider& provider) {
    CHECK_MOVED();
    ::engineSetContentProvider(m_impl, provider.m_impl);
}

void Engine::clearContentProvider() {
    CHECK_MOVED();
    ::engineSetContentProvider(m_impl, nullptr);
}

void Engine::setAutoSave(Repository& repo, bool enabled) {
    CHECK_MOVED();
    ::engineSetAutoSave(m_impl, repo.m_impl, enabled);
}

void Engine::clearAutoSave() {
    CHECK_MOVED();
    ::engineSetAutoSave(m_impl, nullptr, false);
}

void Engine::setLogger(Logger& logger) {
    CHECK_MOVED();
    m_logger = &logger;
    ::engineSetLogger(m_impl, static_cast<::Logger*>(logger.m_impl));
}

void Engine::resetLogger() {
    CHECK_MOVED();
    m_logger = m_defaultLogger.get();
    ::engineSetLogger(m_impl, static_cast<::Logger*>(m_logger->m_impl));
}

#define DEFINE_ON(name, c_event) \
CallbackHandle Engine::on##name(std::function<void()> cb) { \
    CHECK_MOVED(); \
    auto* stored = new std::function<void()>(std::move(cb)); \
    int slot = ::engineOn##name(m_impl, &callbackTrampoline, stored); \
    if (slot < 0) { delete stored; return CallbackHandle(); } \
    return CallbackHandle(m_impl, c_event, slot, stored); \
}

DEFINE_ON(Started, ENGINE_EVENT_STARTED)
DEFINE_ON(Stopped, ENGINE_EVENT_STOPPED)
DEFINE_ON(Finished, ENGINE_EVENT_FINISHED)
DEFINE_ON(Paused, ENGINE_EVENT_PAUSED)
DEFINE_ON(Resumed, ENGINE_EVENT_RESUMED)
DEFINE_ON(Timeout, ENGINE_EVENT_TIMEOUT)
DEFINE_ON(CorrectKeystroke, ENGINE_EVENT_CORRECT_KEYSTROKE)
DEFINE_ON(IncorrectKeystroke, ENGINE_EVENT_INCORRECT_KEYSTROKE)
DEFINE_ON(Backspace, ENGINE_EVENT_BACKSPACE)
DEFINE_ON(SegmentCompleted, ENGINE_EVENT_SEGMENT_COMPLETED)
DEFINE_ON(Error, ENGINE_EVENT_ERROR)

#undef DEFINE_ON

CallbackHandle::CallbackHandle(::Engine* engine, int event, int slotId, void* cb)
    : m_engine(engine), m_event(event), m_slotId(slotId), m_cb(cb) {}

CallbackHandle::~CallbackHandle() { disconnect(); }

CallbackHandle::CallbackHandle(CallbackHandle&& other) noexcept
    : m_engine(other.m_engine), m_event(other.m_event), m_slotId(other.m_slotId), m_cb(other.m_cb)
{
    other.m_engine = nullptr;
    other.m_event = 0;
    other.m_slotId = -1;
    other.m_cb = nullptr;
}

CallbackHandle& CallbackHandle::operator=(CallbackHandle&& other) noexcept {
    if (this != &other) {
        disconnect();
        m_engine = other.m_engine;
        m_event = other.m_event;
        m_slotId = other.m_slotId;
        m_cb = other.m_cb;
        other.m_engine = nullptr;
        other.m_event = 0;
        other.m_slotId = -1;
        other.m_cb = nullptr;
    }
    return *this;
}

void CallbackHandle::disconnect() {
    if (m_cb) {
        if (m_engine) { ::engineDisconnect(m_engine, static_cast<::EngineEvent>(m_event), m_slotId); }
        delete static_cast<std::function<void()>*>(m_cb);
        m_cb = nullptr;
    }
    m_engine = nullptr;
    m_event = 0;
    m_slotId = -1;
}

Snapshot Engine::getSnapshot() {
    CHECK_MOVED();
    return Snapshot(::engineGetSnapshot(m_impl));
}

}
