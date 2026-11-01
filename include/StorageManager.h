#pragma once

#include "DocumentManager.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

/**
 * @brief Records a historical user query for inspection in the search history interface.
 */
struct SearchHistoryEntry {
    std::string query;
    std::string mode;
    std::string timestamp;
    size_t resultCount{0};
    std::string selectedDocument;
};

/**
 * @brief Handles data persistence (JSON format) for query-specific popularity data
 *        and search history with atomic file replacement and corruption recovery.
 */
class StorageManager {
public:
    StorageManager() = default;

    /**
     * @brief Saves query-specific popularity map to a JSON file safely.
     *        Uses document filenames for durable cross-session identity.
     */
    bool savePopularity(
        const std::string& filepath,
        const std::unordered_map<std::string, std::unordered_map<uint32_t, uint32_t>>& data,
        const DocumentManager& docMgr);

    /**
     * @brief Loads query-specific popularity map from JSON file.
     *        Translates document filenames to current active DocIDs.
     *        Recovers safely if file is missing or corrupted.
     */
    bool loadPopularity(
        const std::string& filepath,
        std::unordered_map<std::string, std::unordered_map<uint32_t, uint32_t>>& outData,
        const DocumentManager& docMgr);

    /**
     * @brief Saves search history to a JSON file.
     */
    bool saveHistory(const std::string& filepath, const std::vector<SearchHistoryEntry>& history);

    /**
     * @brief Loads search history from a JSON file.
     */
    bool loadHistory(const std::string& filepath, std::vector<SearchHistoryEntry>& outHistory);

    /**
     * @brief Helper to generate current UTC/local timestamp string.
     */
    static std::string getCurrentTimestamp();

private:
    // Helper to escape string for JSON serialization
    static std::string escapeJsonString(const std::string& str);

    // Simple JSON lexer / parser helpers
    static void skipWhitespace(const std::string& content, size_t& pos);
    static bool parseString(const std::string& content, size_t& pos, std::string& outStr);
    static bool parseNumber(const std::string& content, size_t& pos, uint64_t& outNum);
};
