#include <snapshot.h>

#include <cpptypr/snapshot.hpp>

namespace cpptypr {

std::string_view toString(EngineState state) noexcept {
    switch (state) {
        case EngineState::Idle:    return "idle";
        case EngineState::Running: return "running";
        case EngineState::Paused:  return "paused";
        case EngineState::Error:   return "error";
    }
    return "idle";
}

std::string_view toString(StopCause cause) noexcept {
    switch (cause) {
        case StopCause::None:     return "none";
        case StopCause::Timeout:  return "timeout";
        case StopCause::Finished: return "finished";
        case StopCause::User:     return "user";
        case StopCause::Error:    return "error";
        case StopCause::Unknown:  return "unknown";
    }
    return "none";
}

Snapshot::Snapshot(const ::EngineSnapshot& snap) : m_snap(snap) {}

std::string_view Snapshot::text() const noexcept { return std::string_view(m_snap.text, m_snap.length); }

size_t Snapshot::length() const noexcept { return m_snap.length; }

uint32_t Snapshot::cursorIndex() const noexcept { return m_snap.cursorIndex; }

char Snapshot::expectedChar() const noexcept { return m_snap.expectedChar; }

bool Snapshot::isIncorrect(size_t index) const {
    return index < m_snap.length && m_snap.incorrectFlags[index];
}

SessionStats Snapshot::stats() const noexcept {
    auto& s = m_snap.stats;
    return { std::chrono::milliseconds(s.durationMs), s.correctKeystrokes, s.incorrectKeystrokes, s.totalKeystrokes, s.accuracy, s.wpm, s.wpmRaw };
}

EngineState Snapshot::state() const noexcept {
    switch (m_snap.state) {
        case ::ENGINE_IDLE:    return EngineState::Idle;
        case ::ENGINE_RUNNING: return EngineState::Running;
        case ::ENGINE_PAUSED:  return EngineState::Paused;
        case ::ENGINE_ERROR:   return EngineState::Error;
    }
    return EngineState::Idle;
}

StopCause Snapshot::stopCause() const noexcept {
    switch (m_snap.stopCause) {
        case ::ENGINE_STOP_CAUSE_NONE:     return StopCause::None;
        case ::ENGINE_STOP_CAUSE_TIMEOUT:  return StopCause::Timeout;
        case ::ENGINE_STOP_CAUSE_FINISHED: return StopCause::Finished;
        case ::ENGINE_STOP_CAUSE_USER:     return StopCause::User;
        case ::ENGINE_STOP_CAUSE_ERROR:    return StopCause::Error;
        case ::ENGINE_STOP_CAUSE_UNKNOWN:  return StopCause::Unknown;
    }
    return StopCause::None;
}

}
