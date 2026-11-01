#include "test_macros.h"
#include "InvertedIndex.h"
#include "Document.h"
#include "TextProcessor.h"
#include <vector>

TEST_CASE(testIndexBuildAndTermFrequency) {
    TextProcessor tp;
    InvertedIndex index;

    Document doc1(1, "doc1.txt", "path1", "apple orange apple banana", 4, 100, {});
    Document doc2(2, "doc2.txt", "path2", "banana grape orange orange orange", 5, 120, {});

    std::vector<const Document*> docs = { &doc1, &doc2 };
    index.buildIndex(docs, tp);

    ASSERT_EQ(index.getTotalIndexedDocuments(), 2);
    ASSERT_EQ(index.getTermFrequency("apple", 1), 2);
    ASSERT_EQ(index.getTermFrequency("apple", 2), 0);
    ASSERT_EQ(index.getTermFrequency("orange", 1), 1);
    ASSERT_EQ(index.getTermFrequency("orange", 2), 3);
    ASSERT_EQ(index.getTermFrequency("banana", 1), 1);
    ASSERT_EQ(index.getTermFrequency("banana", 2), 1);
}

TEST_CASE(testDocumentFrequency) {
    TextProcessor tp;
    InvertedIndex index;

    Document doc1(1, "doc1.txt", "p1", "graph algorithm", 2, 50, {});
    Document doc2(2, "doc2.txt", "p2", "graph tree", 2, 50, {});
    Document doc3(3, "doc3.txt", "p3", "matrix graph", 2, 50, {});

    index.buildIndex({ &doc1, &doc2, &doc3 }, tp);

    ASSERT_EQ(index.getDocumentFrequency("graph"), 3);
    ASSERT_EQ(index.getDocumentFrequency("algorithm"), 1);
    ASSERT_EQ(index.getDocumentFrequency("tree"), 1);
    ASSERT_EQ(index.getDocumentFrequency("nonexistent"), 0);
}

TEST_CASE(testCandidateRetrievalLogic) {
    TextProcessor tp;
    InvertedIndex index;

    Document doc1(1, "doc1.txt", "p1", "quick brown fox", 3, 30, {});
    Document doc2(2, "doc2.txt", "p2", "lazy dog brown", 3, 30, {});
    Document doc3(3, "doc3.txt", "p3", "quick lazy cat", 3, 30, {});

    index.buildIndex({ &doc1, &doc2, &doc3 }, tp);

    // OR mode: "quick" OR "dog" -> doc 1 (quick), doc 2 (dog), doc 3 (quick)
    auto orCandidates = index.getCandidateDocIds({"quick", "dog"}, false);
    ASSERT_EQ(orCandidates.size(), 3);

    // AND mode: "quick" AND "lazy" -> doc 3 only
    auto andCandidates = index.getCandidateDocIds({"quick", "lazy"}, true);
    ASSERT_EQ(andCandidates.size(), 1);
    ASSERT_EQ(andCandidates[0], 3);

    // AND mode with non-existent term -> empty
    auto andEmpty = index.getCandidateDocIds({"quick", "elephant"}, true);
    ASSERT_TRUE(andEmpty.empty());
}

TEST_CASE(testNoDuplicatePostingsOnReindex) {
    TextProcessor tp;
    InvertedIndex index;

    Document doc1(1, "doc1.txt", "p1", "search engine test", 3, 30, {});
    index.addDocument(doc1, tp);
    // Add again with updated content
    Document doc1Updated(1, "doc1.txt", "p1", "search engine high performance test", 5, 50, {});
    index.addDocument(doc1Updated, tp);

    const auto* postings = index.getPostings("search");
    ASSERT_TRUE(postings != nullptr);
    ASSERT_EQ(postings->size(), 1);
    ASSERT_EQ((*postings)[0].docId, 1);
}

TEST_CASE(testDocumentRemovalFromIndex) {
    TextProcessor tp;
    InvertedIndex index;

    Document doc1(10, "doc10.txt", "p10", "uniquealpha keyword", 2, 20, {});
    Document doc2(20, "doc20.txt", "p20", "keyword shared", 2, 20, {});

    index.buildIndex({ &doc1, &doc2 }, tp);
    ASSERT_EQ(index.getDocumentFrequency("uniquealpha"), 1);
    ASSERT_EQ(index.getDocumentFrequency("keyword"), 2);

    index.removeDocument(10);
    ASSERT_EQ(index.getDocumentFrequency("uniquealpha"), 0);
    ASSERT_EQ(index.getDocumentFrequency("keyword"), 1);
    ASSERT_FALSE(index.isIndexed(10));
    ASSERT_TRUE(index.isIndexed(20));
}

int main() {
    std::cout << ">>> Running InvertedIndex Tests <<<\n";
    RUN_TEST(testIndexBuildAndTermFrequency);
    RUN_TEST(testDocumentFrequency);
    RUN_TEST(testCandidateRetrievalLogic);
    RUN_TEST(testNoDuplicatePostingsOnReindex);
    RUN_TEST(testDocumentRemovalFromIndex);
    return printTestSummary();
}
