# Mini Search Engine — Testing & Verification Report

## 1. Testing Strategy

The project enforces automated test verification using a lightweight, zero-dependency modern C++ testing framework integrated directly with **CTest** and **CMake**.

Testing spans four architectural levels:
1. **Unit Testing**: Isolated verification of individual functions and methods (tokenization, formula calculation, posting insertion).
2. **Component Testing**: Testing interactions within isolated classes (`DocumentManager`, `InvertedIndex`, `PopularityTracker`, `StorageManager`).
3. **Integration Testing**: End-to-end flows initializing `SearchEngine`, resolving queries, testing popularity boosts, and validating session persistence.
4. **Resilience Testing**: Testing corrupt persistence recovery, missing file handling, empty documents, and zero-match queries.

---

## 2. Test Suites & Coverage

### 2.1 `test_text_processor` (`tests/test_text_processor.cpp`)
- `testLowerCaseConversion`: Ensures ASCII case folding is robust and preserves empty strings.
- `testPunctuationRemoval`: Confirms removal of all punctuation marks while retaining special technical tokens such as `+` in `c++`.
- `testTokenization`: Validates clean whitespace delimiter splitting and empty token suppression.
- `testStopWordRemoval`: Verifies filtering of common English stop words ("and", "are", etc.).
- `testEmptyAndRepeatedTokens`: Checks behavior on whitespace-only and punctuation-only inputs.
- `testCanonicalizeQuery`: Verifies order-invariant normalization (e.g., `"machine learning"` == `"learning machine"`).
- `testCustomStopWords`: Confirms dynamic addition and removal of stop-word entries.

### 2.2 `test_inverted_index` (`tests/test_inverted_index.cpp`)
- `testIndexBuildAndTermFrequency`: Validates multi-document indexing and exact term frequency calculations.
- `testDocumentFrequency`: Checks document frequency counts ($DF$) across the corpus.
- `testCandidateRetrievalLogic`: Verifies disjunctive (`OR`) union and conjunctive (`AND`) intersection set logic.
- `testNoDuplicatePostingsOnReindex`: Verifies that re-indexing an existing document replaces rather than duplicates postings.
- `testDocumentRemovalFromIndex`: Validates cleanup of postings lists and removal of orphaned vocabulary entries when a document is deleted.

### 2.3 `test_ranking` (`tests/test_ranking.cpp`)
- `testTFIDFCalculationFormulas`: Validates numerical accuracy of term frequency and smoothed logarithmic IDF equations.
- `testRelevanceOrderingByTermFrequency`: Verifies that documents containing higher term concentrations receive proportional score advantages.
- `testPopularityDoesNotOverrideRelevanceExcessively`: Mathematically proves that an irrelevant document cannot surpass a relevant document purely via popularity clicks under $\alpha = 0.8, \beta = 0.2$.
- `testWeightAdjustment`: Confirms dynamic configuration of $\alpha$ and $\beta$ parameters.

### 2.4 `test_popularity` (`tests/test_popularity.cpp`)
- `testVisitIncrementAndQueryIsolation`: Validates that opening a document for query `"python"` increments visits exclusively for `"python"`, leaving `"algorithms"` visit counters at 0.
- `testPersistenceCycle`: Verifies complete round-trip JSON serialization and deserialization using document filenames.
- `testCorruptedPersistenceFileHandling`: Proves that a malformed JSON file is automatically backed up as `.corrupted.<timestamp>` without throwing uncaught exceptions or crashing the engine.

### 2.5 `test_search_engine` (`tests/test_search_engine.cpp`)
- `testSearchEngineInitializationAndDiscovery`: Confirms automated discovery and indexing of all 9 core documents.
- `testSingleKeywordSearch`: Verifies single-term lookup and correct top-document identification (`python.txt`).
- `testMultipleKeywordANDAndORSearch`: Validates multi-term conjunctive and disjunctive queries (`"binary search"`).
- `testStopWordAndNoResultQueries`: Validates graceful handling of non-matching queries and pure stop-word queries.
- `testPopularityBoostAndPersistenceAcrossEngineInstances`: End-to-end integration test creating two temporary documents, performing a search, clicking the lower-ranked document, verifying immediate popularity boost, creating a completely new engine instance, and verifying that the boosted ranking persists across sessions.

---

## 3. Test Execution Commands & Verification Results

### Build & CTest Command:
```powershell
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

### Verified CTest Output:
```
Test project C:/Users/VISHAL/.gemini/antigravity/scratch/Mini_Search_Engine/build
    Start 1: test_text_processor
1/5 Test #1: test_text_processor ..............   Passed    0.08 sec
    Start 2: test_inverted_index
2/5 Test #2: test_inverted_index ..............   Passed    0.02 sec
    Start 3: test_ranking
3/5 Test #3: test_ranking .....................   Passed    0.02 sec
    Start 4: test_popularity
4/5 Test #4: test_popularity ..................   Passed    0.03 sec
    Start 5: test_search_engine
5/5 Test #5: test_search_engine ...............   Passed    0.18 sec

100% tests passed out of 5

Total Test time (real) = 0.33 sec
```

### Detailed Suite Output:
```
>>> Running TextProcessor Tests <<<
[RUN     ] testLowerCaseConversion            [  OK  ]
[RUN     ] testPunctuationRemoval              [  OK  ]
[RUN     ] testTokenization                    [  OK  ]
[RUN     ] testStopWordRemoval                 [  OK  ]
[RUN     ] testEmptyAndRepeatedTokens          [  OK  ]
[RUN     ] testCanonicalizeQuery               [  OK  ]
[RUN     ] testCustomStopWords                 [  OK  ]
Tests Run: 7 | Passed: 7 | Failed: 0

>>> Running InvertedIndex Tests <<<
[RUN     ] testIndexBuildAndTermFrequency      [  OK  ]
[RUN     ] testDocumentFrequency               [  OK  ]
[RUN     ] testCandidateRetrievalLogic         [  OK  ]
[RUN     ] testNoDuplicatePostingsOnReindex    [  OK  ]
[RUN     ] testDocumentRemovalFromIndex        [  OK  ]
Tests Run: 5 | Passed: 5 | Failed: 0

>>> Running RankingEngine Tests <<<
[RUN     ] testTFIDFCalculationFormulas        [  OK  ]
[RUN     ] testRelevanceOrderingByTermFrequency[  OK  ]
[RUN     ] testPopularityDoesNotOverrideRelevanceExcessively [  OK  ]
[RUN     ] testWeightAdjustment                [  OK  ]
Tests Run: 4 | Passed: 4 | Failed: 0

>>> Running PopularityTracker Tests <<<
[RUN     ] testVisitIncrementAndQueryIsolation [  OK  ]
[RUN     ] testPersistenceCycle                [  OK  ]
[RUN     ] testCorruptedPersistenceFileHandling[  OK  ]
Tests Run: 3 | Passed: 3 | Failed: 0

>>> Running SearchEngine Integration Tests <<<
[RUN     ] testSearchEngineInitializationAndDiscovery [  OK  ]
[RUN     ] testSingleKeywordSearch             [  OK  ]
[RUN     ] testMultipleKeywordANDAndORSearch   [  OK  ]
[RUN     ] testStopWordAndNoResultQueries      [  OK  ]
[RUN     ] testPopularityBoostAndPersistenceAcrossEngineInstances [  OK  ]
Tests Run: 5 | Passed: 5 | Failed: 0
```
