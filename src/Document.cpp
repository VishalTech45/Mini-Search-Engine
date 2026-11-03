#include "Document.h"
#include <algorithm>
#include <cctype>

namespace {
    // Helper function to search for substring case-insensitively
    size_t findCaseInsensitive(const std::string& text, const std::string& term) {
        if (term.empty() || text.empty() || term.size() > text.size()) {
            return std::string::npos;
        }

        auto it = std::search(
            text.begin(), text.end(),
            term.begin(), term.end(),
            [](char ch1, char ch2) {
                return std::tolower(static_cast<unsigned char>(ch1)) ==
                       std::tolower(static_cast<unsigned char>(ch2));
            }
        );

        if (it != text.end()) {
            return static_cast<size_t>(std::distance(text.begin(), it));
        }
        return std::string::npos;
    }
}

Document::Document(uint32_t id,std::string filename,std::string filepath,std::string content, size_t wordCount,
                   size_t fileSize,
                   std::filesystem::file_time_type lastModified)
    : id_(id),
      filename_(std::move(filename)),
      filepath_(std::move(filepath)),
      rawContent_(std::move(content)),
      wordCount_(wordCount),
      fileSize_(fileSize),
      lastModified_(lastModified) {}

uint32_t Document::getId() const noexcept {
    return id_;
}

const std::string& Document::getFilename() const noexcept {
    return filename_;
}

const std::string& Document::getFilepath() const noexcept {
    return filepath_;
}

const std::string& Document::getContent() const noexcept {
    return rawContent_;
}

size_t Document::getWordCount() const noexcept {
    return wordCount_;
}

size_t Document::getFileSize() const noexcept {
    return fileSize_;
}

std::filesystem::file_time_type Document::getLastModified() const noexcept {
    return lastModified_;
}

bool Document::isEmpty() const noexcept {
    return rawContent_.empty() || wordCount_ == 0;
}

void Document::setContent(std::string content) {
    rawContent_ = std::move(content);
}

void Document::setWordCount(size_t count) noexcept {
    wordCount_ = count;
}

void Document::setFileSize(size_t size) noexcept {
    fileSize_ = size;
}

void Document::setLastModified(std::filesystem::file_time_type time) noexcept {
    lastModified_ = time;
}

std::string Document::getSnippet(const std::vector<std::string>& queryTerms, size_t maxChars) const {
    if (rawContent_.empty()) {
        return "[Empty Document]";
    }

    if (rawContent_.size() <= maxChars) {
        return rawContent_;
    }

    // Locate the first occurrence of any query term
    size_t earliestPos = std::string::npos;

    for (const auto& term : queryTerms) {
        if (term.empty()) continue;
        size_t pos = findCaseInsensitive(rawContent_, term);
        if (pos != std::string::npos && pos < earliestPos) {
            earliestPos = pos;
        }
    }

    size_t startPos = 0;
    if (earliestPos != std::string::npos) {
        // Center the matched term within the snippet window
        size_t leadContext = maxChars / 3;
        if (earliestPos > leadContext) {
            startPos = earliestPos - leadContext;
            // Advance to the nearest space boundary to avoid cutting words
            while (startPos < earliestPos && !std::isspace(static_cast<unsigned char>(rawContent_[startPos]))) {
                ++startPos;
            }
            if (startPos < earliestPos && std::isspace(static_cast<unsigned char>(rawContent_[startPos]))) {
                ++startPos;
            }
        }
    }

    size_t snippetLen = std::min(maxChars, rawContent_.size() - startPos);
    std::string snippet = rawContent_.substr(startPos, snippetLen);

    // Replace linebreaks with spaces for clean snippet display
    for (char& ch : snippet) {
        if (ch == '\r' || ch == '\n' || ch == '\t') {
            ch = ' ';
        }
    }

    std::string result;
    if (startPos > 0) {
        result += "... ";
    }
    result += snippet;
    if (startPos + snippetLen < rawContent_.size()) {
        result += " ...";
    }

    return result;
}
