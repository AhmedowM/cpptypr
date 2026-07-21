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
    auto lower = cpptypr::detail::toLower(s);
    if (lower == "sentences")    return ContentMode::Sentences;
    if (lower == "commonwords")  return ContentMode::CommonWords;
    if (lower == "randomwords")  return ContentMode::RandomWords;
    throw Error(ErrorCode::InvalidMode);
}

std::ostream& operator<<(std::ostream& os, ContentMode mode) {
    return os << toString(mode);
}

ContentProvider ContentProvider::fromString(std::string_view text) {
    return ContentProvider(::contentProviderFromString(std::string(text).c_str()));
}

ContentProvider ContentProvider::fromFile(std::string_view path) {
    return ContentProvider(::contentProviderFromFile(std::string(path).c_str()));
}

ContentProvider ContentProvider::fromDatabase(std::string_view path) {
    return ContentProvider(::contentProviderFromDatabase(std::string(path).c_str()));
}

ContentProvider ContentProvider::fromWeb(std::string_view url) {
    return ContentProvider(::contentProviderFromWeb(std::string(url).c_str()));
}

ContentProvider::ContentProvider(::ContentProvider* p) : m_impl(p) {}

ContentProvider::~ContentProvider() = default;

ContentProvider::ContentProvider(ContentProvider&&) noexcept = default;

ContentProvider& ContentProvider::operator=(ContentProvider&&) noexcept = default;

void ContentProvider::setMode(ContentMode mode) { CHECK_MOVED(); ::contentProviderSetMode(m_impl.get(), static_cast<::ContentMode>(mode)); }
void ContentProvider::setMode(std::string_view mode) { CHECK_MOVED(); setMode(contentModeFromString(mode)); }

void ContentProvider::setContentLimit(size_t limit) { CHECK_MOVED(); ::contentProviderSetContentLimit(m_impl.get(), limit); }

ContentChunk ContentProvider::getNext() {
    CHECK_MOVED();
    auto c = ::contentProviderGetNext(m_impl.get());
    if (c.length == 0) return ContentChunk{};
    return ContentChunk{ std::string(c.text, c.length) };
}

void ContentProvider::reset() { CHECK_MOVED(); ::contentProviderReset(m_impl.get()); }

bool ContentProvider::isExhausted() const { CHECK_MOVED(); return ::contentProviderIsExhausted(m_impl.get()); }

}
