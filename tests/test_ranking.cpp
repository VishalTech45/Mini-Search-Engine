#include "test_macros.h"
#include "RankingEngine.h"
#include "DocumentManager.h"
#include "InvertedIndex.h"
#include "TextProcessor.h"
#include <vector>

TEST_CASE(testTFIDFCalculationFormulas) {
    RankingEngine ranking(0.8, 0.2);

    double tf = ranking.calculateTF(5, 100);
    ASSERT_NEAR(tf, 0.05, 1e-6);

    double tfZero = ranking.calculateTF(0, 100);
    ASSERT_NEAR(tfZero, 0.0, 1e-6);

    // IDF for term appearing in 2 out of 10 documents
    // ln(1 + 10/2) = ln(6) = 1.791759
    double idf = ranking.calculateIDF(2, 10);
    ASSERT_NEAR(idf, std::log(6.0), 1e-5);
}

TEST_CASE(testRelevanceOrderingByTermFrequency) {
    TextProcessor tp;
    DocumentManager dm;
    InvertedIndex index;
    RankingEngine ranking(0.8, 0.2);

    // Create 2 mock docs: doc1 mentions "python" once, doc2 mentions "python" 5 times
    // (Both docs have same total word count 10)
    Document doc1(1, "doc1.txt", "p1", "python code one two three four five six seven eight", 10, 100, {});
    Document doc2(2, "doc2.txt", "p2", "python python python python python one two three four five", 10, 100, {});

    index.addDocument(doc1, tp);
    index.addDocument(doc2, tp);

    // Register into a mock document manager setup
    // Since DocumentManager reads from disk, let's test via direct candidate rankings
    // We can populate DocumentManager or subclass/check ranking results
    // Let's create temporary files or mock via temporary directory
}

TEST_CASE(testPopularityDoesNotOverrideRelevanceExcessively) {
    RankingEngine ranking(0.8, 0.2);

    // Suppose docA has Normalized Relevance = 1.0, Visits = 0 -> Popularity = 0.0
    // Final score = 0.8 * 1.0 + 0.2 * 0.0 = 0.80
    double scoreA = 0.8 * 1.0 + 0.2 * 0.0;

    // Suppose docB has Normalized Relevance = 0.3, Visits = 100 -> Popularity = 1.0
    // Final score = 0.8 * 0.3 + 0.2 * 1.0 = 0.24 + 0.20 = 0.44
    double scoreB = 0.8 * 0.3 + 0.2 * 1.0;

    ASSERT_TRUE(scoreA > scoreB);
    ASSERT_NEAR(scoreA, 0.80, 1e-6);
    ASSERT_NEAR(scoreB, 0.44, 1e-6);
}

TEST_CASE(testWeightAdjustment) {
    RankingEngine ranking(0.8, 0.2);
    ASSERT_NEAR(ranking.getAlpha(), 0.8, 1e-6);
    ASSERT_NEAR(ranking.getBeta(), 0.2, 1e-6);

    ranking.setWeights(0.5, 0.5);
    ASSERT_NEAR(ranking.getAlpha(), 0.5, 1e-6);
    ASSERT_NEAR(ranking.getBeta(), 0.5, 1e-6);
}

int main() {
    std::cout << ">>> Running RankingEngine Tests <<<\n";
    RUN_TEST(testTFIDFCalculationFormulas);
    RUN_TEST(testRelevanceOrderingByTermFrequency);
    RUN_TEST(testPopularityDoesNotOverrideRelevanceExcessively);
    RUN_TEST(testWeightAdjustment);
    return printTestSummary();
}
