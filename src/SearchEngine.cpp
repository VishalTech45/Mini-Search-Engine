#include "SearchEngine.h"
#include <iostream>

SearchEngine::SearchEngine()
    : rankingEngine_(0.8, 0.2) {}

bool SearchEngine::initialize(const std::string& docDir, const std::string& dataDir) {
    docDir_ = docDir;
    dataDir_ = dataDir;
    popularityFilePath_ = dataDir_ + "/popularity.json";
    historyFilePath_ = dataDir_ + "/search_history.json";

    docManager_.clear();
    invertedIndex_.clear();
    popularityTracker_.clear();
    searchHistory_.clear();

    // 1. Ingest document repository
    docManager_.loadFromDirectory(docDir_, textProcessor_);

    // 2. Build inverted index
    invertedIndex_.buildIndex(docManager_.getAllDocuments(), textProcessor_);

    // 3. Restore persisted popularity and search history
    loadState();

    return true;
}

std::vector<SearchResult> SearchEngine::search(const std::string& query, SearchMode mode) {
    std::vector<std::string> queryTokens = textProcessor_.processQuery(query);
    if (queryTokens.empty()) {
        return {};
    }

    bool conjunctive = (mode == SearchMode::AND);
    std::vector<uint32_t> candidateDocIds = invertedIndex_.getCandidateDocIds(queryTokens, conjunctive);

    std::string canonicalKey = textProcessor_.canonicalizeQuery(query);
    auto queryVisits = popularityTracker_.getVisitsForQuery(canonicalKey);

    std::vector<SearchResult> results = rankingEngine_.rankDocuments(
        candidateDocIds,
        queryTokens,
        invertedIndex_,
        docManager_,
        queryVisits
    );

    // Record this query in search history
    SearchHistoryEntry entry;
    entry.query = query;
    entry.mode = (mode == SearchMode::AND) ? "AND" : "OR";
    entry.timestamp = StorageManager::getCurrentTimestamp();
    entry.resultCount = results.size();
    entry.selectedDocument = "";
    searchHistory_.push_back(std::move(entry));

    // Save updated history
    storageManager_.saveHistory(historyFilePath_, searchHistory_);

    return results;
}

void SearchEngine::openDocument(const std::string& query, uint32_t docId) {
    if (docId == 0) return;

    std::string canonicalKey = textProcessor_.canonicalizeQuery(query);
    popularityTracker_.recordVisit(canonicalKey, docId);

    const Document* doc = docManager_.getDocument(docId);
    std::string docName = doc ? doc->getFilename() : ("doc_" + std::to_string(docId));

    if (!searchHistory_.empty()) {
        searchHistory_.back().selectedDocument = docName;
    }

    saveState();
}

size_t SearchEngine::rebuildIndex() {
    docManager_.reloadModifiedDocuments(textProcessor_);
    invertedIndex_.buildIndex(docManager_.getAllDocuments(), textProcessor_);
    return invertedIndex_.getTotalIndexedDocuments();
}

bool SearchEngine::addNewDocument(const std::string& filepath) {
    auto docIdOpt = docManager_.addDocument(filepath, textProcessor_);
    if (!docIdOpt.has_value()) {
        return false;
    }

    const Document* doc = docManager_.getDocument(docIdOpt.value());
    if (doc) {
        invertedIndex_.addDocument(*doc, textProcessor_);
    }
    return true;
}

bool SearchEngine::removeExistingDocument(uint32_t docId) {
    if (!docManager_.getDocument(docId)) {
        return false;
    }
    invertedIndex_.removeDocument(docId);
    return docManager_.removeDocument(docId);
}

bool SearchEngine::saveState() {
    bool popSaved = storageManager_.savePopularity(
        popularityFilePath_,
        popularityTracker_.getAllData(),
        docManager_
    );
    bool histSaved = storageManager_.saveHistory(historyFilePath_, searchHistory_);
    return popSaved && histSaved;
}

bool SearchEngine::loadState() {
    std::unordered_map<std::string, std::unordered_map<uint32_t, uint32_t>> popData;
    storageManager_.loadPopularity(popularityFilePath_, popData, docManager_);
    popularityTracker_.setAllData(std::move(popData));

    storageManager_.loadHistory(historyFilePath_, searchHistory_);
    return true;
}

const DocumentManager& SearchEngine::getDocumentManager() const noexcept {
    return docManager_;
}

DocumentManager& SearchEngine::getDocumentManager() noexcept {
    return docManager_;
}

const InvertedIndex& SearchEngine::getInvertedIndex() const noexcept {
    return invertedIndex_;
}

const PopularityTracker& SearchEngine::getPopularityTracker() const noexcept {
    return popularityTracker_;
}

const RankingEngine& SearchEngine::getRankingEngine() const noexcept {
    return rankingEngine_;
}

RankingEngine& SearchEngine::getRankingEngine() noexcept {
    return rankingEngine_;
}

const TextProcessor& SearchEngine::getTextProcessor() const noexcept {
    return textProcessor_;
}

const std::vector<SearchHistoryEntry>& SearchEngine::getSearchHistory() const noexcept {
    return searchHistory_;
}

const std::string& SearchEngine::getDocumentsDirectory() const noexcept {
    return docDir_;
}

const std::string& SearchEngine::getDataDirectory() const noexcept {
    return dataDir_;
}
