#include "PopularityTracker.h"
#include <algorithm>

void PopularityTracker::recordVisit(const std::string& canonicalQuery, uint32_t docId) {
    if (canonicalQuery.empty() || docId == 0) {
        return;
    }
    queryDocVisits_[canonicalQuery][docId]++;
    globalDocVisits_[docId]++;
}

uint32_t PopularityTracker::getVisitCount(const std::string& canonicalQuery, uint32_t docId) const {
    auto qIt = queryDocVisits_.find(canonicalQuery);
    if (qIt == queryDocVisits_.end()) {
        return 0;
    }
    auto dIt = qIt->second.find(docId);
    if (dIt == qIt->second.end()) {
        return 0;
    }
    return dIt->second;
}

std::unordered_map<uint32_t, uint32_t> PopularityTracker::getVisitsForQuery(const std::string& canonicalQuery) const {
    auto qIt = queryDocVisits_.find(canonicalQuery);
    if (qIt != queryDocVisits_.end()) {
        return qIt->second;
    }
    return {};
}

uint32_t PopularityTracker::getGlobalVisits(uint32_t docId) const {
    auto it = globalDocVisits_.find(docId);
    if (it != globalDocVisits_.end()) {
        return it->second;
    }
    return 0;
}

std::vector<std::pair<uint32_t, uint32_t>> PopularityTracker::getTopGlobalDocuments(size_t limit) const {
    std::vector<std::pair<uint32_t, uint32_t>> list(globalDocVisits_.begin(), globalDocVisits_.end());
    std::sort(list.begin(), list.end(), [](const auto& a, const auto& b) {
        if (a.second != b.second) {
            return a.second > b.second; // descending by visits
        }
        return a.first < b.first;
    });

    if (list.size() > limit) {
        list.resize(limit);
    }
    return list;
}

const std::unordered_map<std::string, std::unordered_map<uint32_t, uint32_t>>& PopularityTracker::getAllData() const noexcept {
    return queryDocVisits_;
}

void PopularityTracker::setAllData(std::unordered_map<std::string, std::unordered_map<uint32_t, uint32_t>> data) {
    queryDocVisits_ = std::move(data);
    globalDocVisits_.clear();
    for (const auto& [query, docMap] : queryDocVisits_) {
        for (const auto& [docId, count] : docMap) {
            globalDocVisits_[docId] += count;
        }
    }
}

void PopularityTracker::clear() {
    queryDocVisits_.clear();
    globalDocVisits_.clear();
}
