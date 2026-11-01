#pragma once

#include <string>
#include <vector>
#include <unordered_set>

/**
 * @brief Handles text normalization, case folding, punctuation stripping, tokenization,
 *        and configurable stop-word removal.
 */
class TextProcessor {
public:
    TextProcessor();
    explicit TextProcessor(std::unordered_set<std::string> customStopWords);

    /**
     * @brief Transforms an entire string to lowercase.
     */
    std::string toLowerCase(const std::string& text) const;

    /**
     * @brief Removes punctuation symbols, replacing them with whitespace so tokens are preserved.
     */
    std::string removePunctuation(const std::string& text) const;

    /**
     * @brief Splits string by whitespace into individual tokens without punctuation removal.
     */
    std::vector<std::string> splitWhitespace(const std::string& text) const;

    /**
     * @brief Standard tokenization: lowercases, strips punctuation, and extracts non-empty tokens.
     */
    std::vector<std::string> tokenize(const std::string& text) const;

    /**
     * @brief Full text pipeline: lowercases, removes punctuation, tokenizes, and filters stop words.
     * @param rawText Input raw text string.
     * @param removeStopWords If true, filters out stop words; otherwise retains them.
     */
    std::vector<std::string> processText(const std::string& rawText, bool removeStopWords = true) const;

    /**
     * @brief Processes a search query string with stop-word removal.
     */
    std::vector<std::string> processQuery(const std::string& queryText) const;

    /**
     * @brief Generates a deterministic canonical key for a query (sorted unique tokens).
     *        Ensures "machine learning" and "learning machine" map to the exact same popularity key.
     */
    std::string canonicalizeQuery(const std::string& queryText) const;

    /**
     * @brief Counts total words in raw text (whitespace delimited).
     */
    size_t countRawWords(const std::string& rawText) const;

    // Stop-word configuration methods
    bool isStopWord(const std::string& word) const noexcept;
    void addStopWord(const std::string& word);
    void removeStopWord(const std::string& word);
    void setStopWords(std::unordered_set<std::string> words);
    const std::unordered_set<std::string>& getStopWords() const noexcept;
    bool loadStopWordsFromFile(const std::string& filepath);

private:
    std::unordered_set<std::string> stopWords_;
    void initializeDefaultStopWords();
};
