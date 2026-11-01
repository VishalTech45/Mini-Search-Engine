#pragma once

#include "Document.h"
#include "TextProcessor.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <filesystem>

/**
 * @brief Manages the document collection, handles file I/O, auto-discovery,
 *        deduplication, dynamic additions/removals, and modification checks.
 */
class DocumentManager {
public:
    DocumentManager() = default;

    /**
     * @brief Discovers and loads all .txt documents from the specified directory.
     * @param dirPath Directory path to scan.
     * @param processor TextProcessor used to calculate initial word counts.
     * @return True if directory was successfully accessed and scanned.
     */
    bool loadFromDirectory(const std::string& dirPath, const TextProcessor& processor);

    /**
     * @brief Adds or loads a single document by its file path.
     * @param filepath Path to the document.
     * @param processor TextProcessor used to compute word counts.
     * @return Assigned DocID if added, or std::nullopt if failed or duplicate.
     */
    std::optional<uint32_t> addDocument(const std::string& filepath, const TextProcessor& processor);

    /**
     * @brief Removes a document by its DocID.
     * @return True if document was found and removed.
     */
    bool removeDocument(uint32_t docId);

    /**
     * @brief Inspects all loaded documents against filesystem modification timestamps
     *        and reloads any files that have changed on disk.
     * @param processor TextProcessor used to recompute word counts.
     * @return List of docIds that were reloaded.
     */
    std::vector<uint32_t> reloadModifiedDocuments(const TextProcessor& processor);

    /**
     * @brief Retrieves a document pointer by DocID.
     * @return Const pointer to Document, or nullptr if not found.
     */
    const Document* getDocument(uint32_t docId) const;

    /**
     * @brief Retrieves a document pointer by filename (e.g. "algorithms.txt").
     */
    const Document* getDocumentByFilename(const std::string& filename) const;

    /**
     * @brief Returns pointers to all loaded documents sorted by DocID.
     */
    std::vector<const Document*> getAllDocuments() const;

    size_t getDocumentCount() const noexcept;
    const std::string& getDocumentDirectory() const noexcept;
    void clear();

private:
    std::string documentDir_;
    std::unordered_map<uint32_t, Document> documents_;
    std::unordered_map<std::string, uint32_t> pathToDocId_;
    std::unordered_map<std::string, uint32_t> filenameToDocId_;
    uint32_t nextDocId_{1};

    // Internal helper to read file contents into a string safely
    bool readFileContents(const std::filesystem::path& path, std::string& outContent, size_t& outSize) const;
};
