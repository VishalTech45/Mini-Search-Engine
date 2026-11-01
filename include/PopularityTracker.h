#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

/**
 * @brief Manages query-specific document popularity counters and global visit statistics.
 *        Maintains isolation between different queries so visits for "python" do not
 *        affect queries for "algorithms".
 */
class PopularityTracker {
public:
    PopularityTracker() = default;

    /**
     * @brief Records a visit (opening) of a document specifically for a canonical search query.
     * @param canonicalQuery Normalized, canonical query key.
     * @param docId Document identifier opened.
     */
    void recordVisit(const std::string& canonicalQuery, uint32_t docId);

    /**
     * @brief Retrieves the visit count for a specific document under a given query.
     */
    uint32_t getVisitCount(const std::string& canonicalQuery, uint32_t docId) const;

    /**
     * @brief Returns a map of all (docId -> visitCount) recorded for a given query.
     */
    std::unordered_map<uint32_t, uint32_t> getVisitsForQuery(const std::string& canonicalQuery) const;

    /**
     * @brief Returns the total aggregate visits for a document across all queries.
     */
    uint32_t getGlobalVisits(uint32_t docId) const;

    /**
     * @brief Retrieves the top globally visited documents.
     * @return Pairs of (docId, totalVisits) sorted descending by visits.
     */
    std::vector<std::pair<uint32_t, uint32_t>> getTopGlobalDocuments(size_t limit = 10) const;

    /**
     * @brief Returns raw access to the nested popularity map for serialization.
     */
    const std::unordered_map<std::string, std::unordered_map<uint32_t, uint32_t>>& getAllData() const noexcept;

    /**
     * @brief Restores popularity data from loaded persistence state.
     */
    void setAllData(std::unordered_map<std::string, std::unordered_map<uint32_t, uint32_t>> data);

    void clear();

private:
    // query -> (docId -> visitCount)
    std::unordered_map<std::string, std::unordered_map<uint32_t, uint32_t>> queryDocVisits_;
    // docId -> total visits across all queries
    std::unordered_map<uint32_t, uint32_t> globalDocVisits_;
};
