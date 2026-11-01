#pragma once

#include "Document.h"
#include "TextProcessor.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>

/**
 * @brief Represents a single document entry in a term's postings list.
 */
struct Posting {
    uint32_t docId{0};
    uint32_t termFrequency{0};

    bool operator==(const Posting& other) const noexcept {
        return docId == other.docId && termFrequency == other.termFrequency;
    }
};

/**
 * @brief Manages the inverted index data structure, mapping terms to document postings,
 *        computing term frequencies, and resolving candidate document sets for queries.
 */
class InvertedIndex {
public:
    InvertedIndex() = default;

    /**
     * @brief Builds the entire inverted index from the provided document list.
     */
    void buildIndex(const std::vector<const Document*>& documents, const TextProcessor& processor);

    /**
     * @brief Adds or updates a single document in the inverted index.
     */
    void addDocument(const Document& doc, const TextProcessor& processor);

    /**
     * @brief Removes a document's postings from the inverted index.
     */
    void removeDocument(uint32_t docId);

    /**
     * @brief Clears the entire index.
     */
    void clear();

    /**
     * @brief Retrieves the postings list for a specific normalized term.
     * @return Pointer to vector of Postings, or nullptr if term is not in vocabulary.
     */
    const std::vector<Posting>* getPostings(const std::string& term) const;

    /**
     * @brief Retrieves the term frequency of a word in a specific document.
     */
    uint32_t getTermFrequency(const std::string& term, uint32_t docId) const;

    /**
     * @brief Returns the number of documents containing the term (Document Frequency - DF).
     */
    size_t getDocumentFrequency(const std::string& term) const;

    /**
     * @brief Resolves candidate document IDs matching a query under AND or OR logic.
     * @param queryTerms List of normalized query tokens.
     * @param conjunctive If true, computes AND (intersection); if false, computes OR (union).
     * @return List of matching Document IDs.
     */
    std::vector<uint32_t> getCandidateDocIds(const std::vector<std::string>& queryTerms, bool conjunctive) const;

    size_t getVocabularySize() const noexcept;
    size_t getTotalIndexedDocuments() const noexcept;
    std::vector<std::string> getVocabulary() const;
    bool isIndexed(uint32_t docId) const noexcept;

private:
    // Core inverted index: term -> list of postings (sorted by docId)
    std::unordered_map<std::string, std::vector<Posting>> index_;
    std::unordered_set<uint32_t> indexedDocIds_;
};
