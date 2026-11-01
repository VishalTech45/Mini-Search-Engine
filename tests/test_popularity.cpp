#include "test_macros.h"
#include "PopularityTracker.h"
#include "StorageManager.h"
#include "DocumentManager.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

TEST_CASE(testVisitIncrementAndQueryIsolation) {
    PopularityTracker tracker;

    // Record visits for "python"
    tracker.recordVisit("python", 1);
    tracker.recordVisit("python", 1);
    tracker.recordVisit("python", 2);

    // Record visits for "algorithms"
    tracker.recordVisit("algorithms", 2);
    tracker.recordVisit("algorithms", 2);
    tracker.recordVisit("algorithms", 2);

    // Query "python" counts
    ASSERT_EQ(tracker.getVisitCount("python", 1), 2);
    ASSERT_EQ(tracker.getVisitCount("python", 2), 1);
    ASSERT_EQ(tracker.getVisitCount("python", 3), 0);

    // Query "algorithms" counts
    ASSERT_EQ(tracker.getVisitCount("algorithms", 1), 0);
    ASSERT_EQ(tracker.getVisitCount("algorithms", 2), 3);

    // Global visits
    ASSERT_EQ(tracker.getGlobalVisits(1), 2);
    ASSERT_EQ(tracker.getGlobalVisits(2), 4);
    ASSERT_EQ(tracker.getGlobalVisits(3), 0);
}

TEST_CASE(testPersistenceCycle) {
    std::string testDir = "build/test_temp_pop";
    fs::create_directories(testDir);
    std::string testFile = testDir + "/popularity.json";

    TextProcessor tp;
    DocumentManager dm;
    StorageManager sm;
    PopularityTracker tracker;

    // Create a dummy document on disk so DocumentManager can resolve it
    std::string docPath = testDir + "/sample.txt";
    {
        std::ofstream out(docPath);
        out << "Sample test document";
    }
    auto docIdOpt = dm.addDocument(docPath, tp);
    ASSERT_TRUE(docIdOpt.has_value());
    uint32_t docId = docIdOpt.value();

    tracker.recordVisit("test query", docId);
    tracker.recordVisit("test query", docId);
    ASSERT_EQ(tracker.getVisitCount("test query", docId), 2);

    // Save to disk
    bool saved = sm.savePopularity(testFile, tracker.getAllData(), dm);
    ASSERT_TRUE(saved);
    ASSERT_TRUE(fs::exists(testFile));

    // Load in fresh tracker
    PopularityTracker restored;
    std::unordered_map<std::string, std::unordered_map<uint32_t, uint32_t>> loadedData;
    bool loaded = sm.loadPopularity(testFile, loadedData, dm);
    ASSERT_TRUE(loaded);
    restored.setAllData(std::move(loadedData));

    ASSERT_EQ(restored.getVisitCount("test query", docId), 2);

    // Clean up
    fs::remove_all(testDir);
}

TEST_CASE(testCorruptedPersistenceFileHandling) {
    std::string testDir = "build/test_temp_corrupt";
    fs::create_directories(testDir);
    std::string corruptFile = testDir + "/corrupt_popularity.json";

    // Write malformed JSON
    {
        std::ofstream out(corruptFile);
        out << "{ invalid json content ... }}} missing quotes and braces";
    }

    DocumentManager dm;
    StorageManager sm;
    std::unordered_map<std::string, std::unordered_map<uint32_t, uint32_t>> loadedData;

    // Should return false and not crash, backing up the corrupted file
    bool success = sm.loadPopularity(corruptFile, loadedData, dm);
    ASSERT_FALSE(success);
    ASSERT_TRUE(loadedData.empty());

    // Clean up
    fs::remove_all(testDir);
}

int main() {
    std::cout << ">>> Running PopularityTracker Tests <<<\n";
    RUN_TEST(testVisitIncrementAndQueryIsolation);
    RUN_TEST(testPersistenceCycle);
    RUN_TEST(testCorruptedPersistenceFileHandling);
    return printTestSummary();
}
