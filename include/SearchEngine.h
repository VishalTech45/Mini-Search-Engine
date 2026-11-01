#pragma once

#include "DocumentManager.h"
#include "TextProcessor.h"
#include "InvertedIndex.h"
#include "RankingEngine.h"
#include "PopularityTracker.h"
#include "StorageManager.h"
#include <string>
#include <vector>
#include <memory>

/**
 * @brief Search conjunction mode: AND requires all query terms, OR requires at least one.
 */
enum class SearchMode {
    AND,
    OR
};

/**
 * @brief Central facade coordinator that integrates DocumentManager, TextProcessor,
 *        InvertedIndex, RankingEngine, PopularityTracker, and StorageManager.
 */
class SearchEngine {
public:
    SearchEngine();

    /**
     * @brief Initializes the search engine by loading documents, constructing the
     *        inverted index, and restoring persistent popularity and history data.
     * @param docDir Documents directory path (default: "documents").
     * @param dataDir Data directory path for persistence (default: "data").
     * @return True if initialized successfully.
     */
    bool initialize(const std::string& docDir = "documents", const std::string& dataDir = "data");

    /**
     * @brief Executes a search query using either AND or OR mode.
     * @param query Raw query string entered by user.
     * @param mode SearchMode::AND or SearchMode::OR.
     * @return Vector of SearchResult ordered by descending composite score.
     */
    std::vector<SearchResult> search(const std::string& query, SearchMode mode = SearchMode::OR);

    /**
     * @brief Simulates opening a document from a search result, updating query-specific
     *        popularity counters and persisting the new state.
     * @param query Search query under which the document was selected.
     * @param docId ID of the document opened.
     */
    void openDocument(const std::string& query, uint32_t docId);

    /**
     * @brief Re-checks filesystem for modified documents and completely rebuilds the index.
     * @return Number of documents currently indexed.
     */
    size_t rebuildIndex();

    /**
     * @brief Dynamically adds a new document file and incrementally updates the index.
     */
    bool addNewDocument(const std::string& filepath);

    /**
     * @brief Dynamically removes a document by ID and cleans the index.
     */
    bool removeExistingDocument(uint32_t docId);

    // Persistence triggers
    bool saveState();
    bool loadState();

    // Accessors for UI and diagnostics
    const DocumentManager& getDocumentManager() const noexcept;
    DocumentManager& getDocumentManager() noexcept;
    const InvertedIndex& getInvertedIndex() const noexcept;
    const PopularityTracker& getPopularityTracker() const noexcept;
    const RankingEngine& getRankingEngine() const noexcept;
    RankingEngine& getRankingEngine() noexcept;
    const TextProcessor& getTextProcessor() const noexcept;
    const std::vector<SearchHistoryEntry>& getSearchHistory() const noexcept;

    const std::string& getDocumentsDirectory() const noexcept;
    const std::string& getDataDirectory() const noexcept;

private:
    std::string docDir_{"documents"};
    std::string dataDir_{"data"};
    std::string popularityFilePath_{"data/popularity.json"};
    std::string historyFilePath_{"data/search_history.json"};

    DocumentManager docManager_;
    TextProcessor textProcessor_;
    InvertedIndex invertedIndex_;
    RankingEngine rankingEngine_;
    PopularityTracker popularityTracker_;
    StorageManager storageManager_;
    std::vector<SearchHistoryEntry> searchHistory_;
};
