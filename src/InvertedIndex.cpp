#include "InvertedIndex.h"
#include <algorithm>

void InvertedIndex::buildIndex(const std::vector<const Document*>& documents, const TextProcessor& processor) {
    clear();
    for (const auto* doc : documents) {
        if (doc) {
            addDocument(*doc, processor);
        }
    }
}

void InvertedIndex::addDocument(const Document& doc, const TextProcessor& processor) {
    if (doc.isEmpty()) {
        indexedDocIds_.insert(doc.getId());
        return;
    }

    // Remove prior postings for this docId if already indexed
    if (indexedDocIds_.find(doc.getId()) != indexedDocIds_.end()) {
        removeDocument(doc.getId());
    }

    std::vector<std::string> tokens = processor.processText(doc.getContent(), true);
    if (tokens.empty()) {
        indexedDocIds_.insert(doc.getId());
        return;
    }

    // Compute local term frequencies for this document
    std::unordered_map<std::string, uint32_t> localFreq;
    for (const auto& token : tokens) {
        localFreq[token]++;
    }

    uint32_t docId = doc.getId();
    for (const auto& [term, freq] : localFreq) {
        auto& postings = index_[term];
        // Insert maintaining sorted docId order
        auto it = std::lower_bound(postings.begin(), postings.end(), docId,
                                   [](const Posting& p, uint32_t id) {
                                       return p.docId < id;
                                   });
        if (it != postings.end() && it->docId == docId) {
            it->termFrequency = freq; // Update
        } else {
            postings.insert(it, Posting{docId, freq});
        }
    }

    indexedDocIds_.insert(docId);
}

void InvertedIndex::removeDocument(uint32_t docId) {
    if (indexedDocIds_.find(docId) == indexedDocIds_.end()) {
        return;
    }

    std::vector<std::string> emptyTerms;
    for (auto& [term, postings] : index_) {
        postings.erase(
            std::remove_if(postings.begin(), postings.end(),
                           [docId](const Posting& p) { return p.docId == docId; }),
            postings.end()
        );
        if (postings.empty()) {
            emptyTerms.push_back(term);
        }
    }

    for (const auto& term : emptyTerms) {
        index_.erase(term);
    }

    indexedDocIds_.erase(docId);
}

void InvertedIndex::clear() {
    index_.clear();
    indexedDocIds_.clear();
}

const std::vector<Posting>* InvertedIndex::getPostings(const std::string& term) const {
    auto it = index_.find(term);
    if (it != index_.end()) {
        return &it->second;
    }
    return nullptr;
}

uint32_t InvertedIndex::getTermFrequency(const std::string& term, uint32_t docId) const {
    const auto* postings = getPostings(term);
    if (!postings) {
        return 0;
    }

    auto it = std::lower_bound(postings->begin(), postings->end(), docId,
                               [](const Posting& p, uint32_t id) {
                                   return p.docId < id;
                               });
    if (it != postings->end() && it->docId == docId) {
        return it->termFrequency;
    }
    return 0;
}

size_t InvertedIndex::getDocumentFrequency(const std::string& term) const {
    const auto* postings = getPostings(term);
    return postings ? postings->size() : 0;
}

std::vector<uint32_t> InvertedIndex::getCandidateDocIds(const std::vector<std::string>& queryTerms, bool conjunctive) const {
    if (queryTerms.empty()) {
        return {};
    }

    if (!conjunctive) {
        // Disjunctive OR query: union of all matching doc IDs
        std::unordered_set<uint32_t> docSet;
        for (const auto& term : queryTerms) {
            const auto* postings = getPostings(term);
            if (postings) {
                for (const auto& p : *postings) {
                    docSet.insert(p.docId);
                }
            }
        }
        std::vector<uint32_t> results(docSet.begin(), docSet.end());
        std::sort(results.begin(), results.end());
        return results;
    }

    // Conjunctive AND query: intersection of doc IDs across all query terms
    std::vector<uint32_t> candidates;
    bool first = true;

    for (const auto& term : queryTerms) {
        const auto* postings = getPostings(term);
        if (!postings || postings->empty()) {
            return {}; // If any term has zero postings, AND intersection is empty
        }

        std::vector<uint32_t> termDocIds;
        termDocIds.reserve(postings->size());
        for (const auto& p : *postings) {
            termDocIds.push_back(p.docId);
        }

        if (first) {
            candidates = std::move(termDocIds);
            first = false;
        } else {
            std::vector<uint32_t> intersection;
            std::set_intersection(
                candidates.begin(), candidates.end(),
                termDocIds.begin(), termDocIds.end(),
                std::back_inserter(intersection)
            );
            candidates = std::move(intersection);
            if (candidates.empty()) {
                return {};
            }
        }
    }

    return candidates;
}

size_t InvertedIndex::getVocabularySize() const noexcept {
    return index_.size();
}

size_t InvertedIndex::getTotalIndexedDocuments() const noexcept {
    return indexedDocIds_.size();
}

std::vector<std::string> InvertedIndex::getVocabulary() const {
    std::vector<std::string> vocab;
    vocab.reserve(index_.size());
    for (const auto& [term, _] : index_) {
        vocab.push_back(term);
    }
    std::sort(vocab.begin(), vocab.end());
    return vocab;
}

bool InvertedIndex::isIndexed(uint32_t docId) const noexcept {
    return indexedDocIds_.find(docId) != indexedDocIds_.end();
}
