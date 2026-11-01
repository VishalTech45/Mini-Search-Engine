#include "test_macros.h"
#include "SearchEngine.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

TEST_CASE(testSearchEngineInitializationAndDiscovery) {
    SearchEngine engine;
    bool ok = engine.initialize("documents", "data");
    ASSERT_TRUE(ok);
    ASSERT_TRUE(engine.getDocumentManager().getDocumentCount() >= 9);
    ASSERT_TRUE(engine.getInvertedIndex().getVocabularySize() > 0);
}

TEST_CASE(testSingleKeywordSearch) {
    SearchEngine engine;
    engine.initialize("documents", "data");

    auto results = engine.search("python", SearchMode::OR);
    ASSERT_FALSE(results.empty());

    // python.txt should be the top result for "python"
    ASSERT_EQ(results[0].filename, "python.txt");
    ASSERT_TRUE(results[0].finalScore > 0.0);
}

TEST_CASE(testMultipleKeywordANDAndORSearch) {
    SearchEngine engine;
    engine.initialize("documents", "data");

    // Search "binary search" in AND mode
    auto andResults = engine.search("binary search", SearchMode::AND);
    ASSERT_FALSE(andResults.empty());

    for (const auto& r : andResults) {
        // Each matched doc must contain both "binary" and "search"
        uint32_t tf1 = engine.getInvertedIndex().getTermFrequency("binary", r.docId);
        uint32_t tf2 = engine.getInvertedIndex().getTermFrequency("search", r.docId);
        ASSERT_TRUE(tf1 > 0);
        ASSERT_TRUE(tf2 > 0);
    }

    // Search in OR mode
    auto orResults = engine.search("binary search", SearchMode::OR);
    ASSERT_TRUE(orResults.size() >= andResults.size());
}

TEST_CASE(testStopWordAndNoResultQueries) {
    SearchEngine engine;
    engine.initialize("documents", "data");

    // Query with non-existent terms in OR mode
    auto noResults = engine.search("xylophonic extraterrestrial nonsenseqwerty", SearchMode::OR);
    ASSERT_TRUE(noResults.empty());

    // Query in AND mode where one term does not exist
    auto noResultsAnd = engine.search("python nonexistentqwertyxyz", SearchMode::AND);
    ASSERT_TRUE(noResultsAnd.empty());

    // Query with stop words e.g. "the and of"
    auto stopResults = engine.search("the and of", SearchMode::OR);
    // Should handle gracefully without crashing
}

TEST_CASE(testPopularityBoostAndPersistenceAcrossEngineInstances) {
    std::string testDocDir = "build/test_integration_docs";
    std::string testDataDir = "build/test_integration_data";
    fs::create_directories(testDocDir);
    fs::create_directories(testDataDir);

    // Create 2 test docs
    {
        std::ofstream f1(testDocDir + "/doc_a.txt");
        f1 << "matrix mathematics algebra vectors computing";
    }
    {
        std::ofstream f2(testDocDir + "/doc_b.txt");
        f2 << "matrix mathematics geometry tensors computing";
    }

    // Session 1:
    {
        SearchEngine engine1;
        engine1.initialize(testDocDir, testDataDir);

        auto resultsBefore = engine1.search("matrix", SearchMode::OR);
        ASSERT_EQ(resultsBefore.size(), 2);
        ASSERT_EQ(resultsBefore[0].visitCount, 0);
        ASSERT_EQ(resultsBefore[1].visitCount, 0);

        // Open doc_b (which is initially ranked second or tied)
        uint32_t docBId = 0;
        for (const auto& r : resultsBefore) {
            if (r.filename == "doc_b.txt") docBId = r.docId;
        }
        ASSERT_TRUE(docBId != 0);

        engine1.openDocument("matrix", docBId);
        engine1.openDocument("matrix", docBId);
        engine1.saveState();

        auto resultsAfter = engine1.search("matrix", SearchMode::OR);
        ASSERT_EQ(resultsAfter.size(), 2);

        // doc_b should now have visitCount == 2 and be boosted
        for (const auto& r : resultsAfter) {
            if (r.filename == "doc_b.txt") {
                ASSERT_EQ(r.visitCount, 2);
                ASSERT_TRUE(r.normalizedPopularity > 0.0);
            }
        }
    }

    // Session 2: Fresh instance restoring from testDataDir
    {
        SearchEngine engine2;
        engine2.initialize(testDocDir, testDataDir);

        auto resultsRestored = engine2.search("matrix", SearchMode::OR);
        ASSERT_EQ(resultsRestored.size(), 2);

        for (const auto& r : resultsRestored) {
            if (r.filename == "doc_b.txt") {
                ASSERT_EQ(r.visitCount, 2);
                ASSERT_TRUE(r.normalizedPopularity > 0.0);
            }
        }
    }

    // Cleanup
    fs::remove_all(testDocDir);
    fs::remove_all(testDataDir);
}

int main() {
    std::cout << ">>> Running SearchEngine Integration Tests <<<\n";
    RUN_TEST(testSearchEngineInitializationAndDiscovery);
    RUN_TEST(testSingleKeywordSearch);
    RUN_TEST(testMultipleKeywordANDAndORSearch);
    RUN_TEST(testStopWordAndNoResultQueries);
    RUN_TEST(testPopularityBoostAndPersistenceAcrossEngineInstances);
    return printTestSummary();
}
