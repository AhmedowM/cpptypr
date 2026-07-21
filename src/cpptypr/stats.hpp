#pragma once

#include <chrono>
#include <cstdint>

namespace cpptypr {

/** @brief Snapshot of typing session statistics. */
struct SessionStats {
    std::chrono::milliseconds durationMs{0}; /**< Total session duration in milliseconds. */
    uint32_t correctKeystrokes;  /**< Number of correct keystrokes. */
    uint32_t incorrectKeystrokes;/**< Number of incorrect keystrokes. */
    uint32_t totalKeystrokes;    /**< Total keystrokes (correct + incorrect). */
    double accuracy;             /**< Accuracy percentage (0.0–100.0). */
    double wpm;                  /**< Net words-per-minute (penalises mistakes). */
    double wpmRaw;               /**< Raw words-per-minute (no penalty). */
};

}
