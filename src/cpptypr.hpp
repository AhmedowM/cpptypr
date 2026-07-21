/** @file cpptypr.hpp
 *  @brief Umbrella header for the cpptypr library.
 *
 *  Include this single header to gain access to all public types:
 *    - cpptypr::Engine, cpptypr::EngineMode, cpptypr::SessionStats
 *    - cpptypr::ContentProvider, cpptypr::ContentMode, cpptypr::ContentChunk
 *    - cpptypr::Repository, cpptypr::SessionData
 *    - cpptypr::Logger, cpptypr::LogLevel
 *    - cpptypr::Error, cpptypr::ErrorCode, cpptypr::CallbackHandle
 *    - cpptypr::Snapshot, cpptypr::EngineState, cpptypr::StopCause */
#pragma once

#include <cpptypr/error.hpp>
#include <cpptypr/logger.hpp>
#include <cpptypr/stats.hpp>
#include <cpptypr/content.hpp>
#include <cpptypr/snapshot.hpp>
#include <cpptypr/version.hpp>
#include <cpptypr/repository.hpp>
#include <cpptypr/engine.hpp>
