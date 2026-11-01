#include "DocumentManager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace fs = std::filesystem;

bool DocumentManager::readFileContents(const fs::path& path, std::string& outContent, size_t& outSize) const {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    file.seekg(0, std::ios::end);
    auto size = file.tellg();
    if (size < 0) {
        return false;
    }
    outSize = static_cast<size_t>(size);

    file.seekg(0, std::ios::beg);
    outContent.resize(outSize);
    if (outSize > 0) {
        file.read(&outContent[0], static_cast<std::streamsize>(outSize));
    }
    return true;
}

bool DocumentManager::loadFromDirectory(const std::string& dirPath, const TextProcessor& processor) {
    documentDir_ = dirPath;
    fs::path p(dirPath);

    std::error_code ec;
    if (!fs::exists(p, ec) || !fs::is_directory(p, ec)) {
        return false;
    }

    for (const auto& entry : fs::directory_iterator(p, ec)) {
        if (ec) break;
        if (entry.is_regular_file(ec) && entry.path().extension() == ".txt") {
            addDocument(entry.path().string(), processor);
        }
    }
    return true;
}

std::optional<uint32_t> DocumentManager::addDocument(const std::string& filepath, const TextProcessor& processor) {
    fs::path p(filepath);
    std::error_code ec;

    if (!fs::exists(p, ec) || !fs::is_regular_file(p, ec)) {
        return std::nullopt;
    }

    fs::path canonicalP = fs::weakly_canonical(p, ec);
    std::string canonicalStr = ec ? p.string() : canonicalP.string();

    // Check for duplicate loading by canonical path
    if (pathToDocId_.find(canonicalStr) != pathToDocId_.end()) {
        return std::nullopt;
    }

    std::string content;
    size_t fileSize = 0;
    if (!readFileContents(p, content, fileSize)) {
        return std::nullopt;
    }

    auto lastMod = fs::last_write_time(p, ec);
    if (ec) {
        lastMod = fs::file_time_type{};
    }

    uint32_t docId = nextDocId_++;
    std::string filename = p.filename().string();
    size_t wordCount = processor.countRawWords(content);

    Document doc(docId, filename, p.string(), std::move(content), wordCount, fileSize, lastMod);
    documents_.emplace(docId, std::move(doc));
    pathToDocId_[canonicalStr] = docId;
    filenameToDocId_[filename] = docId;

    return docId;
}

bool DocumentManager::removeDocument(uint32_t docId) {
    auto it = documents_.find(docId);
    if (it == documents_.end()) {
        return false;
    }

    const std::string& path = it->second.getFilepath();
    const std::string& filename = it->second.getFilename();

    std::error_code ec;
    fs::path canonicalP = fs::weakly_canonical(fs::path(path), ec);
    std::string canonicalStr = ec ? path : canonicalP.string();

    pathToDocId_.erase(canonicalStr);
    filenameToDocId_.erase(filename);
    documents_.erase(it);

    return true;
}

std::vector<uint32_t> DocumentManager::reloadModifiedDocuments(const TextProcessor& processor) {
    std::vector<uint32_t> reloadedDocIds;
    std::error_code ec;

    for (auto& [docId, doc] : documents_) {
        fs::path p(doc.getFilepath());
        if (!fs::exists(p, ec)) {
            continue;
        }

        auto currentMod = fs::last_write_time(p, ec);
        if (!ec && currentMod != doc.getLastModified()) {
            std::string content;
            size_t fileSize = 0;
            if (readFileContents(p, content, fileSize)) {
                size_t wordCount = processor.countRawWords(content);
                doc.setContent(std::move(content));
                doc.setFileSize(fileSize);
                doc.setWordCount(wordCount);
                doc.setLastModified(currentMod);
                reloadedDocIds.push_back(docId);
            }
        }
    }
    return reloadedDocIds;
}

const Document* DocumentManager::getDocument(uint32_t docId) const {
    auto it = documents_.find(docId);
    if (it != documents_.end()) {
        return &it->second;
    }
    return nullptr;
}

const Document* DocumentManager::getDocumentByFilename(const std::string& filename) const {
    auto it = filenameToDocId_.find(filename);
    if (it != filenameToDocId_.end()) {
        return getDocument(it->second);
    }
    return nullptr;
}

std::vector<const Document*> DocumentManager::getAllDocuments() const {
    std::vector<const Document*> list;
    list.reserve(documents_.size());
    for (const auto& [id, doc] : documents_) {
        list.push_back(&doc);
    }
    std::sort(list.begin(), list.end(), [](const Document* a, const Document* b) {
        return a->getId() < b->getId();
    });
    return list;
}

size_t DocumentManager::getDocumentCount() const noexcept {
    return documents_.size();
}

const std::string& DocumentManager::getDocumentDirectory() const noexcept {
    return documentDir_;
}

void DocumentManager::clear() {
    documents_.clear();
    pathToDocId_.clear();
    filenameToDocId_.clear();
    nextDocId_ = 1;
}
