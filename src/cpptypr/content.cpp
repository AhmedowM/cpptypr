#include <content.h>

#include <cpptypr/content.hpp>
#include <cpptypr/detail.hpp>

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

namespace detail {

ContentMode parseContentMode(std::string_view s) {
    auto lower = cpptypr::detail::toLower(s);
    if (lower == "sentences")    return ContentMode::Sentences;
    if (lower == "commonwords")  return ContentMode::CommonWords;
    if (lower == "randomwords")  return ContentMode::RandomWords;
    throw Error(ErrorCode::InvalidMode);
}

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

ContentProvider::~ContentProvider() { if (m_impl) { ::contentProviderDestroy(m_impl); } }

ContentProvider::ContentProvider(ContentProvider&& other) noexcept : m_impl(other.m_impl) { other.m_impl = nullptr; }

ContentProvider& ContentProvider::operator=(ContentProvider&& other) noexcept {
    if (this != &other) {
        if (m_impl) { ::contentProviderDestroy(m_impl); }
        m_impl = other.m_impl;
        other.m_impl = nullptr;
    }
    return *this;
}

void ContentProvider::setMode(ContentMode mode) { CHECK_MOVED(); ::contentProviderSetMode(m_impl, static_cast<::ContentMode>(mode)); }
void ContentProvider::setMode(std::string_view mode) { CHECK_MOVED(); setMode(detail::parseContentMode(mode)); }

void ContentProvider::setContentLimit(size_t limit) { CHECK_MOVED(); ::contentProviderSetContentLimit(m_impl, limit); }

ContentChunk ContentProvider::getNext() {
    CHECK_MOVED();
    auto c = ::contentProviderGetNext(m_impl);
    if (!c.text) return ContentChunk{};
    return ContentChunk{ std::string(c.text, c.length) };
}

void ContentProvider::reset() { CHECK_MOVED(); ::contentProviderReset(m_impl); }

bool ContentProvider::isExhausted() const { CHECK_MOVED(); return ::contentProviderIsExhausted(m_impl); }

}
