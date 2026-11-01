#pragma once

#include "Document.h"
#include "DocumentManager.h"
#include "InvertedIndex.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

/**
 * @brief Represents a ranked search result item containing detailed scoring diagnostics.
 */
struct SearchResult {
    uint32_t docId{0};
    std::string filename;
    double rawRelevanceScore{0.0};
    double normalizedRelevance{0.0};
    uint32_t visitCount{0};
    double normalizedPopularity{0.0};
    double finalScore{0.0};
    std::string snippet;
};

/**
 * @brief Computes TF-IDF relevance scores, query-specific popularity weighting,
 *        and produces ranked result sets.
 */
class RankingEngine {
public:
    explicit RankingEngine(double alpha = 0.8, double beta = 0.2);

    /**
     * @brief Computes normalized Term Frequency:
     *        TF(t, d) = termFrequency / max(totalDocWords, 1)
     */
    double calculateTF(uint32_t termFrequency, size_t totalDocWords) const noexcept;

    /**
     * @brief Computes smoothed Inverse Document Frequency:
     *        IDF(t) = ln(1.0 + totalDocuments / max(docFrequency, 1))
     */
    double calculateIDF(size_t docFrequency, size_t totalDocuments) const noexcept;

    /**
     * @brief Ranks a list of candidate documents using hybrid TF-IDF and query popularity.
     * @param candidateDocIds List of matching document IDs.
     * @param queryTerms List of normalized query tokens.
     * @param index Inverted index reference.
     * @param docMgr Document manager reference for word counts, filenames, and snippets.
     * @param queryVisits Map of docId -> visitCount for this specific query.
     * @return Sorted vector of SearchResult in descending order of finalScore.
     */
    std::vector<SearchResult> rankDocuments(
        const std::vector<uint32_t>& candidateDocIds,
        const std::vector<std::string>& queryTerms,
        const InvertedIndex& index,
        const DocumentManager& docMgr,
        const std::unordered_map<uint32_t, uint32_t>& queryVisits) const;

    void setWeights(double alpha, double beta);
    double getAlpha() const noexcept;
    double getBeta() const noexcept;

private:
    double alpha_{0.8}; // Weight for relevance
    double beta_{0.2};  // Weight for query popularity
};
