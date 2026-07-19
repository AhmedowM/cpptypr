#include <repository.h>

#include <cpptypr/repository.hpp>

#include <chrono>
#include <cstring>
#include <cstdlib>
#include <string>

namespace cpptypr {

#define CHECK_MOVED() do { if (!m_impl) throw Error(ErrorCode::State); } while(0)

Repository::Repository(std::string_view dbPath) : m_impl(::repositoryCreate(std::string(dbPath).c_str())) {}

Repository::~Repository() { if (m_impl) { ::repositoryDestroy(m_impl); } }

Repository::Repository(Repository&& other) noexcept : m_impl(other.m_impl) { other.m_impl = nullptr; }

Repository& Repository::operator=(Repository&& other) noexcept {
    if (this != &other) {
        if (m_impl) { ::repositoryDestroy(m_impl); }
        m_impl = other.m_impl;
        other.m_impl = nullptr;
    }
    return *this;
}

int64_t Repository::saveSession(const SessionData& data) {
    CHECK_MOVED();
    ::SessionData cd{};
    cd.id = data.id;
    std::strncpy(cd.timestamp, data.timestamp.c_str(), sizeof(cd.timestamp) - 1);
    cd.timestamp[sizeof(cd.timestamp) - 1] = '\0';
    std::strncpy(cd.mode, data.mode.c_str(), sizeof(cd.mode) - 1);
    cd.mode[sizeof(cd.mode) - 1] = '\0';
    cd.totalChars = data.totalChars;
    cd.correctChars = data.correctChars;
    cd.durationMs = data.durationMs.count();
    cd.wpm = data.wpm;
    cd.wpmRaw = data.wpmRaw;
    cd.accuracy = data.accuracy;
    auto id = ::repositorySaveSession(m_impl, &cd);
    invalidateCache();
    return id;
}

std::optional<SessionData> Repository::getSession(int64_t id) {
    CHECK_MOVED();
    auto cd = ::repositoryGetSession(m_impl, id);
    if (cd.id == 0) { return std::nullopt; }
    return SessionData{ cd.id, cd.timestamp, cd.mode,
        cd.totalChars, cd.correctChars, std::chrono::milliseconds(cd.durationMs),
        cd.wpm, cd.wpmRaw, cd.accuracy };
}

std::vector<SessionData> Repository::getAll() {
    CHECK_MOVED();
    size_t count;
    auto* arr = ::repositoryGetAll(m_impl, &count);
    if (!arr) { return {}; }
    std::vector<SessionData> result;
    result.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        result.push_back({ arr[i].id, arr[i].timestamp, arr[i].mode,
            arr[i].totalChars, arr[i].correctChars, std::chrono::milliseconds(arr[i].durationMs),
            arr[i].wpm, arr[i].wpmRaw, arr[i].accuracy });
    }
    ::free(arr);
    return result;
}

std::vector<SessionData> Repository::getRecent(int64_t limit) {
    CHECK_MOVED();
    size_t count;
    auto* arr = ::repositoryGetRecent(m_impl, limit, &count);
    if (!arr) { return {}; }
    std::vector<SessionData> result;
    result.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        result.push_back({ arr[i].id, arr[i].timestamp, arr[i].mode,
            arr[i].totalChars, arr[i].correctChars, std::chrono::milliseconds(arr[i].durationMs),
            arr[i].wpm, arr[i].wpmRaw, arr[i].accuracy });
    }
    ::free(arr);
    return result;
}

int64_t Repository::count() { CHECK_MOVED(); return ::repositoryGetCount(m_impl); }

bool Repository::deleteSession(int64_t id) {
    CHECK_MOVED();
    invalidateCache();
    return ::repositoryDeleteSession(m_impl, id);
}

void Repository::clearAll() {
    CHECK_MOVED();
    invalidateCache();
    ::repositoryClearAll(m_impl);
}

std::optional<SessionData> Repository::bestWpm() {
    CHECK_MOVED();
    auto cd = ::repositoryGetBestWpm(m_impl);
    if (cd.id == 0) { return std::nullopt; }
    return SessionData{ cd.id, cd.timestamp, cd.mode,
        cd.totalChars, cd.correctChars, std::chrono::milliseconds(cd.durationMs),
        cd.wpm, cd.wpmRaw, cd.accuracy };
}

std::optional<SessionData> Repository::bestRawWpm() {
    CHECK_MOVED();
    auto cd = ::repositoryGetBestRawWpm(m_impl);
    if (cd.id == 0) { return std::nullopt; }
    return SessionData{ cd.id, cd.timestamp, cd.mode,
        cd.totalChars, cd.correctChars, std::chrono::milliseconds(cd.durationMs),
        cd.wpm, cd.wpmRaw, cd.accuracy };
}

double Repository::averageWpm() { CHECK_MOVED(); return ::repositoryGetAverageWpm(m_impl); }

void Repository::ensureCache() const {
    if (!m_cacheValid) {
        const_cast<Repository*>(this)->m_cache = const_cast<Repository*>(this)->getAll();
        const_cast<Repository*>(this)->m_cacheValid = true;
    }
}

void Repository::invalidateCache() {
    m_cacheValid = false;
    m_cache.clear();
}

std::vector<SessionData>::iterator Repository::begin() { ensureCache(); return m_cache.begin(); }
std::vector<SessionData>::iterator Repository::end() { ensureCache(); return m_cache.end(); }
std::vector<SessionData>::const_iterator Repository::begin() const { ensureCache(); return m_cache.begin(); }
std::vector<SessionData>::const_iterator Repository::end() const { ensureCache(); return m_cache.end(); }

}
