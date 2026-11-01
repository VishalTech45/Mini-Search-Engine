#include "RankingEngine.h"
#include <cmath>
#include <algorithm>

RankingEngine::RankingEngine(double alpha, double beta)
    : alpha_(alpha), beta_(beta) {}

double RankingEngine::calculateTF(uint32_t termFrequency, size_t totalDocWords) const noexcept {
    if (termFrequency == 0 || totalDocWords == 0) {
        return 0.0;
    }
    return static_cast<double>(termFrequency) / static_cast<double>(totalDocWords);
}

double RankingEngine::calculateIDF(size_t docFrequency, size_t totalDocuments) const noexcept {
    if (docFrequency == 0 || totalDocuments == 0) {
        return 0.0;
    }
    // Smoothed logarithmic IDF formulation
    return std::log(1.0 + (static_cast<double>(totalDocuments) / static_cast<double>(docFrequency)));
}

std::vector<SearchResult> RankingEngine::rankDocuments(
    const std::vector<uint32_t>& candidateDocIds,
    const std::vector<std::string>& queryTerms,
    const InvertedIndex& index,
    const DocumentManager& docMgr,
    const std::unordered_map<uint32_t, uint32_t>& queryVisits) const {

    if (candidateDocIds.empty()) {
        return {};
    }

    size_t totalDocs = index.getTotalIndexedDocuments();
    if (totalDocs == 0) {
        totalDocs = docMgr.getDocumentCount();
    }

    // Step 1: Precompute IDF values for each query term
    std::unordered_map<std::string, double> idfValues;
    for (const auto& term : queryTerms) {
        size_t df = index.getDocumentFrequency(term);
        idfValues[term] = calculateIDF(df, totalDocs);
    }

    std::vector<SearchResult> results;
    results.reserve(candidateDocIds.size());

    double maxRawScore = 0.0;
    uint32_t maxVisits = 0;

    // Step 2: Compute raw TF-IDF score and collect query-specific visits
    for (uint32_t docId : candidateDocIds) {
        const Document* doc = docMgr.getDocument(docId);
        std::string filename = doc ? doc->getFilename() : ("doc_" + std::to_string(docId));
        size_t wordCount = doc ? doc->getWordCount() : 1;

        double rawRelevance = 0.0;
        for (const auto& term : queryTerms) {
            uint32_t tfVal = index.getTermFrequency(term, docId);
            double tf = calculateTF(tfVal, wordCount);
            double idf = idfValues[term];
            rawRelevance += (tf * idf);
        }

        uint32_t visits = 0;
        auto visitIt = queryVisits.find(docId);
        if (visitIt != queryVisits.end()) {
            visits = visitIt->second;
        }

        if (rawRelevance > maxRawScore) {
            maxRawScore = rawRelevance;
        }
        if (visits > maxVisits) {
            maxVisits = visits;
        }

        std::string snippet = doc ? doc->getSnippet(queryTerms) : "";

        SearchResult item;
        item.docId = docId;
        item.filename = std::move(filename);
        item.rawRelevanceScore = rawRelevance;
        item.visitCount = visits;
        item.snippet = std::move(snippet);

        results.push_back(std::move(item));
    }

    // Step 3: Compute normalized scores and composite final score
    for (auto& item : results) {
        item.normalizedRelevance = (maxRawScore > 0.0)
            ? (item.rawRelevanceScore / maxRawScore)
            : 0.0;

        item.normalizedPopularity = (maxVisits > 0)
            ? (static_cast<double>(item.visitCount) / static_cast<double>(maxVisits))
            : 0.0;

        item.finalScore = (alpha_ * item.normalizedRelevance) +
                          (beta_ * item.normalizedPopularity);
    }

    // Step 4: Sort in descending order of final score
    std::sort(results.begin(), results.end(), [](const SearchResult& a, const SearchResult& b) {
        if (std::abs(a.finalScore - b.finalScore) > 1e-6) {
            return a.finalScore > b.finalScore;
        }
        if (std::abs(a.normalizedRelevance - b.normalizedRelevance) > 1e-6) {
            return a.normalizedRelevance > b.normalizedRelevance;
        }
        return a.docId < b.docId;
    });

    return results;
}

void RankingEngine::setWeights(double alpha, double beta) {
    alpha_ = alpha;
    beta_ = beta;
}

double RankingEngine::getAlpha() const noexcept {
    return alpha_;
}

double RankingEngine::getBeta() const noexcept {
    return beta_;
}
