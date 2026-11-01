#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <cstdint>

/**
 * @brief Represents an individual indexed text document with its metadata and raw content.
 */
class Document {
public:
    Document() = default;
    Document(uint32_t id,
             std::string filename,
             std::string filepath,
             std::string content,
             size_t wordCount,
             size_t fileSize,
             std::filesystem::file_time_type lastModified);

    // Getters
    uint32_t getId() const noexcept;
    const std::string& getFilename() const noexcept;
    const std::string& getFilepath() const noexcept;
    const std::string& getContent() const noexcept;
    size_t getWordCount() const noexcept;
    size_t getFileSize() const noexcept;
    std::filesystem::file_time_type getLastModified() const noexcept;
    bool isEmpty() const noexcept;

    // Setters
    void setContent(std::string content);
    void setWordCount(size_t count) noexcept;
    void setFileSize(size_t size) noexcept;
    void setLastModified(std::filesystem::file_time_type time) noexcept;

    /**
     * @brief Generates an excerpt snippet centered around matching query terms.
     * @param queryTerms List of search terms to locate in the text.
     * @param maxChars Maximum character length of snippet.
     * @return Formatted snippet string with ellipsis.
     */
    std::string getSnippet(const std::vector<std::string>& queryTerms, size_t maxChars = 160) const;

private:
    uint32_t id_{0};
    std::string filename_;
    std::string filepath_;
    std::string rawContent_;
    size_t wordCount_{0};
    size_t fileSize_{0};
    std::filesystem::file_time_type lastModified_{};
};
