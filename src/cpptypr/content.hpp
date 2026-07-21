#pragma once

#include <iosfwd>
#include <memory>
#include <string>
#include <string_view>

#include <cpptypr/error.hpp>

struct ContentProvider;
struct ContentProviderDeleter { void operator()(::ContentProvider* p) const noexcept; };

namespace cpptypr {

class Logger;
class Engine;

/** @brief A single chunk of content returned by the provider. */
struct ContentChunk {
    std::string text; /**< The content text. */
};

/** @brief Determines how content is selected by the provider. */
enum class ContentMode {
    Sentences,   /**< Full sentences as practice content. */
    CommonWords, /**< Common English words. */
    RandomWords  /**< Completely random words. */
};

/** @brief Convert a ContentMode to its lowercase string representation.
 *  @param mode The content mode.
 *  @return "sentences", "commonwords", or "randomwords". */
[[nodiscard]] std::string_view toString(ContentMode mode) noexcept;

/** @brief Parse a case-insensitive string to a ContentMode.
 *  @param s One of "sentences", "commonwords", or "randomwords".
 *  @return The matching ContentMode.
 *  @throws Error if the string does not match any valid mode. */
[[nodiscard]] ContentMode contentModeFromString(std::string_view s);

/** @brief Write a ContentMode to an output stream.
 *  @param os  The output stream.
 *  @param mode The content mode.
 *  @return The output stream. */
std::ostream& operator<<(std::ostream& os, ContentMode mode);

/** @brief RAII wrapper that provides practice content to an Engine.
 *
 * Use the static factory methods (fromString, fromFile, etc.) to create
 * a provider backed by a specific content source. The underlying C
 * ContentProvider is owned by this wrapper.
 *
 * @note ContentProvider is non-copyable but movable. */
class ContentProvider {
public:
    /** @brief Create a provider that reads from a text string.
     *  @param text The source text to use as content. */
    static ContentProvider fromString(std::string_view text);

    /** @brief Create a provider that reads from a file.
     *  @param path Path to the file containing practice content. */
    static ContentProvider fromFile(std::string_view path);

    /** @brief Create a provider that reads sessions from a database.
     *  @param path Path to the SQLite database file. */
    static ContentProvider fromDatabase(std::string_view path);

    /** @brief Create a provider that fetches content from a URL.
     *  @param url The URL to fetch content from. */
    static ContentProvider fromWeb(std::string_view url);
    ~ContentProvider();

    ContentProvider(const ContentProvider&) = delete;
    ContentProvider& operator=(const ContentProvider&) = delete;

    ContentProvider(ContentProvider&&) noexcept;
    ContentProvider& operator=(ContentProvider&&) noexcept;

    /** @brief Set how content is selected.
     *  @param mode The content selection mode. */
    void setMode(ContentMode mode);

    /** @brief Set how content is selected using a case-insensitive string.
     *  @param mode "sentences", "commonwords", or "randomwords".
     *  @throws Error if the string is invalid. */
    void setMode(std::string_view mode);

    /** @brief Limit the total amount of content the provider will return.
     *  @param limit Maximum number of content units. */
    void setContentLimit(size_t limit);

    /** @brief Retrieve the next chunk of content.
     *  @return The next ContentChunk, or an empty chunk when exhausted. */
    [[nodiscard]] ContentChunk getNext();

    /** @brief Reset the provider, allowing content to be replayed from the beginning. */
    void reset();

    /** @brief Check if all content has been consumed.
     *  @return true if no more content is available. */
    [[nodiscard]] bool isExhausted() const;

private:
    friend class Engine;
    explicit ContentProvider(::ContentProvider* p);
    std::unique_ptr<::ContentProvider, ContentProviderDeleter> m_impl;
};

}
