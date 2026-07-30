#include <content.h>

#include <cpptypr/content.hpp>
#include <cpptypr/detail.hpp>

void ContentProviderDeleter::operator()(::ContentProvider* p) const noexcept { ::contentProviderDestroy(p); }

#include <ostream>
#include <string>

namespace cpptypr {

std::string_view toString(ContentMode mode) noexcept {
    switch (mode) {
        case ContentMode::Sentences:   return "sentences";
        case ContentMode::CommonWords: return "commonwords";
        case ContentMode::RandomWords: return "randomwords";
    }
    return "sentences";
}

ContentMode contentModeFromString(std::string_view s) {
    if (detail::icaseEqual(s, "sentences"))    return ContentMode::Sentences;
    if (detail::icaseEqual(s, "commonwords"))  return ContentMode::CommonWords;
    if (detail::icaseEqual(s, "randomwords"))  return ContentMode::RandomWords;
    throw Error(ErrorCode::InvalidMode);
}

std::ostream& operator<<(std::ostream& os, ContentMode mode) {
    return os << toString(mode);
}

ContentProvider ContentProvider::fromString(std::string_view text) {
    char buf[4096];
    return ContentProvider(::contentProviderFromString(detail::nullTerminal(buf, text)));
}

ContentProvider ContentProvider::fromFile(std::string_view path) {
    char buf[260];
    return ContentProvider(::contentProviderFromFile(detail::nullTerminal(buf, path)));
}

ContentProvider ContentProvider::fromDatabase(std::string_view path) {
    char buf[260];
    return ContentProvider(::contentProviderFromDatabase(detail::nullTerminal(buf, path)));
}

ContentProvider ContentProvider::fromWeb(std::string_view url) {
    char buf[260];
    return ContentProvider(::contentProviderFromWeb(detail::nullTerminal(buf, url)));
}

ContentProvider::ContentProvider(::ContentProvider* p) : m_impl(p) {}

ContentProvider::~ContentProvider() = default;

ContentProvider::ContentProvider(ContentProvider&&) noexcept = default;

ContentProvider& ContentProvider::operator=(ContentProvider&&) noexcept = default;

void ContentProvider::setMode(ContentMode mode) { CHECK_MOVED(); ::contentProviderSetMode(m_impl.get(), static_cast<::ContentMode>(mode)); }
void ContentProvider::setMode(std::string_view mode) { CHECK_MOVED(); setMode(contentModeFromString(mode)); }

void ContentProvider::setDifficultyFilter(std::string_view difficulty) {
    CHECK_MOVED();
    if (difficulty.empty()) {
        ::contentProviderSetDifficultyFilter(m_impl.get(), nullptr);
        return;
    }
    char buf[32];
    ::contentProviderSetDifficultyFilter(m_impl.get(), detail::nullTerminal(buf, difficulty));
}

void ContentProvider::setWordLengthRange(size_t minLen, size_t maxLen) {
    CHECK_MOVED();
    ::contentProviderSetWordLengthRange(m_impl.get(), minLen, maxLen);
}

void ContentProvider::setContentLimit(size_t limit) { CHECK_MOVED(); ::contentProviderSetContentLimit(m_impl.get(), limit); }

ContentChunk ContentProvider::getNext() {
    CHECK_MOVED();
    auto c = ::contentProviderGetNext(m_impl.get());
    if (!c.text || c.length == 0) return ContentChunk{};
    return ContentChunk{ std::string(c.text, c.length) };
}

void ContentProvider::reset() { CHECK_MOVED(); ::contentProviderReset(m_impl.get()); }

bool ContentProvider::isExhausted() const { CHECK_MOVED(); return ::contentProviderIsExhausted(m_impl.get()); }

}
