#include "TextProcessor.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

TextProcessor::TextProcessor() {
    initializeDefaultStopWords();
}

TextProcessor::TextProcessor(std::unordered_set<std::string> customStopWords)
    : stopWords_(std::move(customStopWords)) {}

void TextProcessor::initializeDefaultStopWords() {
    stopWords_ = {
        "a", "about", "above", "after", "again", "against", "all", "am", "an", "and",
        "any", "are", "aren't", "as", "at", "be", "because", "been", "before", "being",
        "below", "between", "both", "but", "by", "can", "cannot", "could", "couldn't",
        "did", "didn't", "do", "does", "doesn't", "doing", "don't", "down", "during",
        "each", "few", "for", "from", "further", "had", "hadn't", "has", "hasn't",
        "have", "haven't", "having", "he", "her", "here", "hers", "herself", "him",
        "himself", "his", "how", "i", "if", "in", "into", "is", "isn't", "it", "its",
        "itself", "me", "more", "most", "must", "my", "myself", "no", "nor", "not",
        "of", "off", "on", "once", "only", "or", "other", "ought", "our", "ours",
        "ourselves", "out", "over", "own", "same", "she", "should", "shouldn't", "so",
        "some", "such", "than", "that", "the", "their", "theirs", "them", "themselves",
        "then", "there", "these", "they", "this", "those", "through", "to", "too",
        "under", "until", "up", "very", "was", "wasn't", "we", "were", "weren't",
        "what", "when", "where", "which", "while", "who", "whom", "why", "with",
        "would", "you", "your", "yours", "yourself", "yourselves"
    };
}

std::string TextProcessor::toLowerCase(const std::string& text) const {
    std::string result = text;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return result;
}

std::string TextProcessor::removePunctuation(const std::string& text) const {
    std::string result;
    result.reserve(text.size());

    for (size_t i = 0; i < text.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(text[i]);

        // Preserve '+' when part of "c++"
        if (c == '+') {
            bool partOfCpp = false;
            if (i > 0 && (text[i - 1] == 'c' || text[i - 1] == 'C' || text[i - 1] == '+')) {
                partOfCpp = true;
            }
            if (partOfCpp) {
                result.push_back('+');
                continue;
            }
        }

        // Standard punctuation characters become space
        if (std::ispunct(c)) {
            result.push_back(' ');
        } else {
            result.push_back(text[i]);
        }
    }
    return result;
}

std::vector<std::string> TextProcessor::splitWhitespace(const std::string& text) const {
    std::vector<std::string> tokens;
    std::istringstream stream(text);
    std::string token;
    while (stream >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

std::vector<std::string> TextProcessor::tokenize(const std::string& text) const {
    std::string cleaned = removePunctuation(toLowerCase(text));
    std::vector<std::string> tokens;
    std::istringstream stream(cleaned);
    std::string token;

    while (stream >> token) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

std::vector<std::string> TextProcessor::processText(const std::string& rawText, bool removeStopWords) const {
    std::vector<std::string> rawTokens = tokenize(rawText);
    if (!removeStopWords) {
        return rawTokens;
    }

    std::vector<std::string> filtered;
    filtered.reserve(rawTokens.size());

    for (const auto& token : rawTokens) {
        if (!isStopWord(token)) {
            filtered.push_back(token);
        }
    }
    return filtered;
}

std::vector<std::string> TextProcessor::processQuery(const std::string& queryText) const {
    // Process query with stop words removed
    std::vector<std::string> tokens = processText(queryText, true);

    // If query consisted solely of stop words (e.g. "to be or not to be"),
    // fall back to keeping them so the search engine can still attempt a match.
    if (tokens.empty()) {
        tokens = processText(queryText, false);
    }
    return tokens;
}

std::string TextProcessor::canonicalizeQuery(const std::string& queryText) const {
    std::vector<std::string> tokens = processQuery(queryText);
    if (tokens.empty()) {
        return "";
    }

    std::sort(tokens.begin(), tokens.end());
    tokens.erase(std::unique(tokens.begin(), tokens.end()), tokens.end());

    std::ostringstream canonical;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (i > 0) {
            canonical << " ";
        }
        canonical << tokens[i];
    }
    return canonical.str();
}

size_t TextProcessor::countRawWords(const std::string& rawText) const {
    std::istringstream stream(rawText);
    std::string word;
    size_t count = 0;
    while (stream >> word) {
        ++count;
    }
    return count;
}

bool TextProcessor::isStopWord(const std::string& word) const noexcept {
    return stopWords_.find(word) != stopWords_.end();
}

void TextProcessor::addStopWord(const std::string& word) {
    stopWords_.insert(toLowerCase(word));
}

void TextProcessor::removeStopWord(const std::string& word) {
    stopWords_.erase(toLowerCase(word));
}

void TextProcessor::setStopWords(std::unordered_set<std::string> words) {
    stopWords_.clear();
    for (const auto& w : words) {
        stopWords_.insert(toLowerCase(w));
    }
}

const std::unordered_set<std::string>& TextProcessor::getStopWords() const noexcept {
    return stopWords_;
}

bool TextProcessor::loadStopWordsFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false;
    }

    stopWords_.clear();
    std::string line;
    while (std::getline(file, line)) {
        std::string token;
        std::istringstream iss(line);
        while (iss >> token) {
            stopWords_.insert(toLowerCase(token));
        }
    }
    return true;
}
