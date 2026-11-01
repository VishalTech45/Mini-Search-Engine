#include "StorageManager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <cctype>

namespace fs = std::filesystem;

std::string StorageManager::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm_buf, &now_c);
#else
    localtime_r(&now_c, &tm_buf);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string StorageManager::escapeJsonString(const std::string& str) {
    std::string escaped;
    escaped.reserve(str.size() + 16);
    for (char c : str) {
        switch (c) {
            case '"':  escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\b': escaped += "\\b";  break;
            case '\f': escaped += "\\f";  break;
            case '\n': escaped += "\\n";  break;
            case '\r': escaped += "\\r";  break;
            case '\t': escaped += "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(c));
                    escaped += buf;
                } else {
                    escaped += c;
                }
                break;
        }
    }
    return escaped;
}

void StorageManager::skipWhitespace(const std::string& content, size_t& pos) {
    while (pos < content.size() && std::isspace(static_cast<unsigned char>(content[pos]))) {
        ++pos;
    }
}

bool StorageManager::parseString(const std::string& content, size_t& pos, std::string& outStr) {
    skipWhitespace(content, pos);
    if (pos >= content.size() || content[pos] != '"') {
        return false;
    }
    ++pos; // skip opening quote

    outStr.clear();
    while (pos < content.size() && content[pos] != '"') {
        if (content[pos] == '\\') {
            ++pos;
            if (pos >= content.size()) return false;
            switch (content[pos]) {
                case '"':  outStr.push_back('"');  break;
                case '\\': outStr.push_back('\\'); break;
                case '/':  outStr.push_back('/');  break;
                case 'b':  outStr.push_back('\b'); break;
                case 'f':  outStr.push_back('\f'); break;
                case 'n':  outStr.push_back('\n'); break;
                case 'r':  outStr.push_back('\r'); break;
                case 't':  outStr.push_back('\t'); break;
                default:   outStr.push_back(content[pos]); break;
            }
        } else {
            outStr.push_back(content[pos]);
        }
        ++pos;
    }

    if (pos < content.size() && content[pos] == '"') {
        ++pos; // skip closing quote
        return true;
    }
    return false;
}

bool StorageManager::parseNumber(const std::string& content, size_t& pos, uint64_t& outNum) {
    skipWhitespace(content, pos);
    if (pos >= content.size() || !std::isdigit(static_cast<unsigned char>(content[pos]))) {
        return false;
    }
    outNum = 0;
    while (pos < content.size() && std::isdigit(static_cast<unsigned char>(content[pos]))) {
        outNum = outNum * 10 + (content[pos] - '0');
        ++pos;
    }
    return true;
}

bool StorageManager::savePopularity(
    const std::string& filepath,
    const std::unordered_map<std::string, std::unordered_map<uint32_t, uint32_t>>& data,
    const DocumentManager& docMgr) {

    fs::path targetPath(filepath);
    std::error_code ec;
    if (targetPath.has_parent_path()) {
        fs::create_directories(targetPath.parent_path(), ec);
    }

    std::string tmpPath = filepath + ".tmp";
    std::ofstream out(tmpPath, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        return false;
    }

    out << "{\n  \"popularity\": {\n";
    size_t qIdx = 0;
    for (const auto& [query, docMap] : data) {
        out << "    \"" << escapeJsonString(query) << "\": {\n";
        size_t dIdx = 0;
        for (const auto& [docId, visits] : docMap) {
            const Document* doc = docMgr.getDocument(docId);
            std::string docKey = doc ? doc->getFilename() : ("doc_" + std::to_string(docId));
            out << "      \"" << escapeJsonString(docKey) << "\": " << visits;
            if (++dIdx < docMap.size()) {
                out << ",";
            }
            out << "\n";
        }
        out << "    }";
        if (++qIdx < data.size()) {
            out << ",";
        }
        out << "\n";
    }
    out << "  }\n}\n";

    out.flush();
    out.close();

    // Atomic replace
    if (fs::exists(targetPath, ec)) {
        fs::remove(targetPath, ec);
    }
    fs::rename(tmpPath, targetPath, ec);
    return !ec;
}

bool StorageManager::loadPopularity(
    const std::string& filepath,
    std::unordered_map<std::string, std::unordered_map<uint32_t, uint32_t>>& outData,
    const DocumentManager& docMgr) {

    outData.clear();
    fs::path targetPath(filepath);
    std::error_code ec;

    if (!fs::exists(targetPath, ec)) {
        return true; // Clean initial state
    }

    std::ifstream in(filepath, std::ios::in);
    if (!in.is_open()) {
        return false;
    }

    std::string content((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    in.close();

    if (content.empty()) {
        return true;
    }

    // Handcrafted robust parser for popularity JSON
    size_t pos = 0;
    skipWhitespace(content, pos);

    auto handleCorrupted = [&]() {
        std::string corruptedBackup = filepath + ".corrupted." +
            std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
        fs::rename(targetPath, corruptedBackup, ec);
        outData.clear();
        return false;
    };

    if (pos >= content.size() || content[pos] != '{') {
        return handleCorrupted();
    }
    ++pos;

    while (pos < content.size()) {
        skipWhitespace(content, pos);
        if (pos < content.size() && content[pos] == '}') {
            ++pos;
            break;
        }

        std::string key;
        if (!parseString(content, pos, key)) return handleCorrupted();
        skipWhitespace(content, pos);
        if (pos >= content.size() || content[pos] != ':') return handleCorrupted();
        ++pos;

        if (key == "popularity") {
            skipWhitespace(content, pos);
            if (pos >= content.size() || content[pos] != '{') return handleCorrupted();
            ++pos;

            while (pos < content.size()) {
                skipWhitespace(content, pos);
                if (pos < content.size() && content[pos] == '}') {
                    ++pos;
                    break;
                }

                std::string queryKey;
                if (!parseString(content, pos, queryKey)) return handleCorrupted();
                skipWhitespace(content, pos);
                if (pos >= content.size() || content[pos] != ':') return handleCorrupted();
                ++pos;
                skipWhitespace(content, pos);
                if (pos >= content.size() || content[pos] != '{') return handleCorrupted();
                ++pos;

                while (pos < content.size()) {
                    skipWhitespace(content, pos);
                    if (pos < content.size() && content[pos] == '}') {
                        ++pos;
                        break;
                    }

                    std::string docKey;
                    if (!parseString(content, pos, docKey)) return handleCorrupted();
                    skipWhitespace(content, pos);
                    if (pos >= content.size() || content[pos] != ':') return handleCorrupted();
                    ++pos;

                    uint64_t visits = 0;
                    if (!parseNumber(content, pos, visits)) return handleCorrupted();

                    // Map docKey (filename or id) to current docId
                    const Document* doc = docMgr.getDocumentByFilename(docKey);
                    if (doc) {
                        outData[queryKey][doc->getId()] = static_cast<uint32_t>(visits);
                    } else if (docKey.rfind("doc_", 0) == 0) {
                        try {
                            uint32_t id = static_cast<uint32_t>(std::stoul(docKey.substr(4)));
                            outData[queryKey][id] = static_cast<uint32_t>(visits);
                        } catch (...) {}
                    }

                    skipWhitespace(content, pos);
                    if (pos < content.size() && content[pos] == ',') {
                        ++pos;
                    }
                }

                skipWhitespace(content, pos);
                if (pos < content.size() && content[pos] == ',') {
                    ++pos;
                }
            }
        } else {
            // Skip unknown property value
            while (pos < content.size() && content[pos] != ',' && content[pos] != '}') {
                ++pos;
            }
        }

        skipWhitespace(content, pos);
        if (pos < content.size() && content[pos] == ',') {
            ++pos;
        }
    }

    return true;
}

bool StorageManager::saveHistory(const std::string& filepath, const std::vector<SearchHistoryEntry>& history) {
    fs::path targetPath(filepath);
    std::error_code ec;
    if (targetPath.has_parent_path()) {
        fs::create_directories(targetPath.parent_path(), ec);
    }

    std::string tmpPath = filepath + ".tmp";
    std::ofstream out(tmpPath, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        return false;
    }

    out << "{\n  \"history\": [\n";
    for (size_t i = 0; i < history.size(); ++i) {
        const auto& entry = history[i];
        out << "    {\n";
        out << "      \"query\": \"" << escapeJsonString(entry.query) << "\",\n";
        out << "      \"mode\": \"" << escapeJsonString(entry.mode) << "\",\n";
        out << "      \"timestamp\": \"" << escapeJsonString(entry.timestamp) << "\",\n";
        out << "      \"resultCount\": " << entry.resultCount << ",\n";
        out << "      \"selectedDocument\": \"" << escapeJsonString(entry.selectedDocument) << "\"\n";
        out << "    }";
        if (i + 1 < history.size()) {
            out << ",";
        }
        out << "\n";
    }
    out << "  ]\n}\n";

    out.flush();
    out.close();

    if (fs::exists(targetPath, ec)) {
        fs::remove(targetPath, ec);
    }
    fs::rename(tmpPath, targetPath, ec);
    return !ec;
}

bool StorageManager::loadHistory(const std::string& filepath, std::vector<SearchHistoryEntry>& outHistory) {
    outHistory.clear();
    fs::path targetPath(filepath);
    std::error_code ec;

    if (!fs::exists(targetPath, ec)) {
        return true;
    }

    std::ifstream in(filepath, std::ios::in);
    if (!in.is_open()) {
        return false;
    }

    std::string content((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    in.close();

    if (content.empty()) {
        return true;
    }

    size_t pos = 0;
    skipWhitespace(content, pos);
    if (pos >= content.size() || content[pos] != '{') {
        return false;
    }
    ++pos;

    while (pos < content.size()) {
        skipWhitespace(content, pos);
        if (pos < content.size() && content[pos] == '}') {
            ++pos;
            break;
        }

        std::string key;
        if (!parseString(content, pos, key)) return false;
        skipWhitespace(content, pos);
        if (pos >= content.size() || content[pos] != ':') return false;
        ++pos;

        if (key == "history") {
            skipWhitespace(content, pos);
            if (pos >= content.size() || content[pos] != '[') return false;
            ++pos;

            while (pos < content.size()) {
                skipWhitespace(content, pos);
                if (pos < content.size() && content[pos] == ']') {
                    ++pos;
                    break;
                }

                if (content[pos] != '{') return false;
                ++pos;

                SearchHistoryEntry entry;
                while (pos < content.size()) {
                    skipWhitespace(content, pos);
                    if (pos < content.size() && content[pos] == '}') {
                        ++pos;
                        break;
                    }

                    std::string fKey;
                    if (!parseString(content, pos, fKey)) return false;
                    skipWhitespace(content, pos);
                    if (pos >= content.size() || content[pos] != ':') return false;
                    ++pos;

                    if (fKey == "query") {
                        if (!parseString(content, pos, entry.query)) return false;
                    } else if (fKey == "mode") {
                        if (!parseString(content, pos, entry.mode)) return false;
                    } else if (fKey == "timestamp") {
                        if (!parseString(content, pos, entry.timestamp)) return false;
                    } else if (fKey == "resultCount") {
                        uint64_t n = 0;
                        if (!parseNumber(content, pos, n)) return false;
                        entry.resultCount = static_cast<size_t>(n);
                    } else if (fKey == "selectedDocument") {
                        if (!parseString(content, pos, entry.selectedDocument)) return false;
                    } else {
                        // Skip field
                        while (pos < content.size() && content[pos] != ',' && content[pos] != '}') {
                            ++pos;
                        }
                    }

                    skipWhitespace(content, pos);
                    if (pos < content.size() && content[pos] == ',') {
                        ++pos;
                    }
                }

                outHistory.push_back(std::move(entry));

                skipWhitespace(content, pos);
                if (pos < content.size() && content[pos] == ',') {
                    ++pos;
                }
            }
        }

        skipWhitespace(content, pos);
        if (pos < content.size() && content[pos] == ',') {
            ++pos;
        }
    }

    return true;
}
