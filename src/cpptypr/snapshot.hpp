#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

#include <snapshot.h>

#include <cpptypr/stats.hpp>

namespace cpptypr {

class Engine;

enum class EngineState {
    Idle,
    Running,
    Paused,
    Error
};

enum class StopCause {
    None,
    Timeout,
    Finished,
    User,
    Error,
    Unknown
};

[[nodiscard]] std::string_view toString(EngineState state) noexcept;
[[nodiscard]] std::string_view toString(StopCause cause) noexcept;

class Snapshot {
public:
    [[nodiscard]] std::string_view text() const noexcept;
    [[nodiscard]] size_t length() const noexcept;
    [[nodiscard]] uint32_t cursorIndex() const noexcept;
    [[nodiscard]] char expectedChar() const noexcept;
    [[nodiscard]] bool isIncorrect(size_t index) const;
    [[nodiscard]] SessionStats stats() const noexcept;
    [[nodiscard]] EngineState state() const noexcept;
    [[nodiscard]] StopCause stopCause() const noexcept;

private:
    friend Engine;
    explicit Snapshot(const ::EngineSnapshot& snap);
    ::EngineSnapshot m_snap;
};

}
