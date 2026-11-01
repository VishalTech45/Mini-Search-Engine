#include "SearchEngine.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <limits>

namespace {
    // Utility for safe line reading from stdin
    std::string readLine(const std::string& prompt = "") {
        if (!prompt.empty()) {
            std::cout << prompt;
            std::cout.flush();
        }
        std::string line;
        if (!std::getline(std::cin, line)) {
            return "";
        }
        return line;
    }

    int readInt(const std::string& prompt, int minVal, int maxVal) {
        while (true) {
            std::string input = readLine(prompt);
            if (input.empty() && std::cin.eof()) {
                return minVal;
            }
            std::istringstream iss(input);
            int val;
            if (iss >> val) {
                if (val >= minVal && val <= maxVal) {
                    return val;
                }
            }
            std::cout << "Invalid input. Please enter a number between "
                      << minVal << " and " << maxVal << ".\n";
        }
    }

    void printHeader(const std::string& title) {
        std::cout << "\n========================================================\n";
        std::cout << "  " << title << "\n";
        std::cout << "========================================================\n";
    }

    void handleSearch(SearchEngine& engine) {
        printHeader("SEARCH DOCUMENTS");
        std::string query = readLine("Enter search query: ");
        if (query.find_first_not_of(" \t\r\n") == std::string::npos) {
            std::cout << "Query cannot be empty. Returning to main menu.\n";
            return;
        }

        std::cout << "\nChoose search mode:\n";
        std::cout << "  1. AND (Documents containing ALL query terms)\n";
        std::cout << "  2. OR  (Documents containing ANY query term)\n";
        int modeChoice = readInt("Select mode (1 or 2): ", 1, 2);
        SearchMode mode = (modeChoice == 1) ? SearchMode::AND : SearchMode::OR;

        auto results = engine.search(query, mode);

        std::cout << "\n========================================================\n";
        std::cout << "SEARCH RESULTS FOR: \"" << query << "\" ("
                  << (mode == SearchMode::AND ? "AND mode" : "OR mode") << ")\n";
        std::cout << "Matches Found: " << results.size() << "\n";
        std::cout << "========================================================\n";

        if (results.empty()) {
            std::cout << "No matching documents found.\n";
            std::cout << "Tips:\n";
            std::cout << " - Try using OR mode to match documents containing any word.\n";
            std::cout << " - Check for spelling errors or try different keywords.\n";
            return;
        }

        for (size_t i = 0; i < results.size(); ++i) {
            const auto& r = results[i];
            std::cout << "\n" << (i + 1) << ". " << r.filename << "\n";
            std::cout << "   Final Score: " << std::fixed << std::setprecision(4) << r.finalScore << "\n";
            std::cout << "   Relevance:   " << std::fixed << std::setprecision(4) << r.normalizedRelevance
                      << " (Raw: " << r.rawRelevanceScore << ")\n";
            std::cout << "   Popularity:  " << std::fixed << std::setprecision(4) << r.normalizedPopularity
                      << " (Visits for this query: " << r.visitCount << ")\n";
            if (!r.snippet.empty()) {
                std::cout << "   Snippet:     " << r.snippet << "\n";
            }
        }
        std::cout << "--------------------------------------------------------\n";

        int selection = readInt("Select document number to open (1-" +
                                std::to_string(results.size()) + ") or 0 to return: ",
                                0, static_cast<int>(results.size()));

        if (selection > 0) {
            const auto& selected = results[selection - 1];
            engine.openDocument(query, selected.docId);

            const Document* doc = engine.getDocumentManager().getDocument(selected.docId);
            printHeader("DOCUMENT VIEW: " + selected.filename);
            if (doc) {
                std::cout << "Document ID: " << doc->getId() << "\n";
                std::cout << "File Size:   " << doc->getFileSize() << " bytes\n";
                std::cout << "Word Count:  " << doc->getWordCount() << " words\n";
                std::cout << "File Path:   " << doc->getFilepath() << "\n";
                std::cout << "--------------------------------------------------------\n";
                std::cout << doc->getContent() << "\n";
                std::cout << "--------------------------------------------------------\n";
            }
            std::cout << "[SUCCESS] Document opened! Popularity count for query \""
                      << query << "\" incremented to " << (selected.visitCount + 1)
                      << " and persisted.\n";
        }
    }

    void handleViewAllDocuments(const SearchEngine& engine) {
        printHeader("INDEXED DOCUMENTS REPOSITORY");
        const auto& docMgr = engine.getDocumentManager();
        auto docs = docMgr.getAllDocuments();

        if (docs.empty()) {
            std::cout << "No documents currently loaded.\n";
            return;
        }

        std::cout << std::left << std::setw(6)  << "ID"
                  << std::setw(28) << "Filename"
                  << std::setw(14) << "Words"
                  << std::setw(14) << "Size (Bytes)"
                  << "Path\n";
        std::cout << std::string(75, '-') << "\n";

        for (const auto* doc : docs) {
            std::cout << std::left << std::setw(6)  << doc->getId()
                      << std::setw(28) << doc->getFilename()
                      << std::setw(14) << doc->getWordCount()
                      << std::setw(14) << doc->getFileSize()
                      << doc->getFilepath() << "\n";
        }
        std::cout << std::string(75, '-') << "\n";
        std::cout << "Total Documents: " << docs.size() << "\n";
    }

    void handleViewSearchHistory(const SearchEngine& engine) {
        printHeader("RECENT SEARCH HISTORY");
        const auto& history = engine.getSearchHistory();

        if (history.empty()) {
            std::cout << "Search history is currently empty.\n";
            return;
        }

        std::cout << std::left << std::setw(6)  << "No."
                  << std::setw(22) << "Timestamp"
                  << std::setw(8)  << "Mode"
                  << std::setw(10) << "Matches"
                  << std::setw(22) << "Query"
                  << "Opened Document\n";
        std::cout << std::string(80, '-') << "\n";

        for (size_t i = 0; i < history.size(); ++i) {
            const auto& item = history[i];
            std::string opened = item.selectedDocument.empty() ? "[None]" : item.selectedDocument;
            std::cout << std::left << std::setw(6)  << (i + 1)
                      << std::setw(22) << item.timestamp
                      << std::setw(8)  << item.mode
                      << std::setw(10) << item.resultCount
                      << std::setw(22) << item.query
                      << opened << "\n";
        }
    }

    void handleViewPopularDocuments(const SearchEngine& engine) {
        printHeader("MOST POPULAR DOCUMENTS (GLOBAL & QUERY-SPECIFIC)");
        const auto& popTracker = engine.getPopularityTracker();
        const auto& docMgr = engine.getDocumentManager();

        auto topDocs = popTracker.getTopGlobalDocuments(10);
        std::cout << "--- Top Documents by Global Lifetime Visits ---\n";
        if (topDocs.empty()) {
            std::cout << "No documents have been opened yet.\n";
        } else {
            for (size_t i = 0; i < topDocs.size(); ++i) {
                const Document* doc = docMgr.getDocument(topDocs[i].first);
                std::string fname = doc ? doc->getFilename() : ("doc_" + std::to_string(topDocs[i].first));
                std::cout << (i + 1) << ". " << fname
                          << " — Total Visits: " << topDocs[i].second << "\n";
            }
        }

        std::cout << "\n--- Query-Specific Popularity Breakdown ---\n";
        const auto& allData = popTracker.getAllData();
        if (allData.empty()) {
            std::cout << "No query-specific records recorded yet.\n";
        } else {
            for (const auto& [query, docMap] : allData) {
                std::cout << "Query: \"" << query << "\"\n";
                for (const auto& [docId, visits] : docMap) {
                    const Document* doc = docMgr.getDocument(docId);
                    std::string fname = doc ? doc->getFilename() : ("doc_" + std::to_string(docId));
                    std::cout << "    * " << fname << " -> " << visits << " visits\n";
                }
            }
        }
    }

    void handleViewDocumentDetails(const SearchEngine& engine) {
        printHeader("VIEW DOCUMENT DETAILS");
        const auto& docMgr = engine.getDocumentManager();
        auto docs = docMgr.getAllDocuments();
        if (docs.empty()) {
            std::cout << "No documents available.\n";
            return;
        }

        std::cout << "Available Documents:\n";
        for (const auto* doc : docs) {
            std::cout << "  " << doc->getId() << ". " << doc->getFilename() << "\n";
        }

        int choice = readInt("\nEnter Document ID to inspect (or 0 to cancel): ", 0, 10000);
        if (choice == 0) return;

        const Document* doc = docMgr.getDocument(static_cast<uint32_t>(choice));
        if (!doc) {
            std::cout << "Document ID " << choice << " not found.\n";
            return;
        }

        printHeader("DETAILS: " + doc->getFilename());
        std::cout << "ID:          " << doc->getId() << "\n";
        std::cout << "Filename:    " << doc->getFilename() << "\n";
        std::cout << "Filepath:    " << doc->getFilepath() << "\n";
        std::cout << "Word Count:  " << doc->getWordCount() << " words\n";
        std::cout << "File Size:   " << doc->getFileSize() << " bytes\n";
        std::cout << "Global Hits: " << engine.getPopularityTracker().getGlobalVisits(doc->getId()) << "\n";
        std::cout << "--------------------------------------------------------\n";
        std::cout << doc->getContent() << "\n";
        std::cout << "--------------------------------------------------------\n";
    }

    void handleRebuildIndex(SearchEngine& engine) {
        printHeader("REBUILD INVERTED INDEX");
        std::cout << "Scanning documents and rebuilding inverted index...\n";
        size_t count = engine.rebuildIndex();
        std::cout << "[SUCCESS] Inverted index rebuilt!\n";
        std::cout << "Total Indexed Documents: " << count << "\n";
        std::cout << "Vocabulary Size:         " << engine.getInvertedIndex().getVocabularySize() << " unique terms\n";
    }

    void handleAddRemoveDocument(SearchEngine& engine) {
        printHeader("DOCUMENT MANAGEMENT (ADD / REMOVE)");
        std::cout << "1. Add New Document from Filepath\n";
        std::cout << "2. Remove Existing Document by ID\n";
        std::cout << "0. Return to Main Menu\n";

        int subChoice = readInt("Select option (0-2): ", 0, 2);
        if (subChoice == 1) {
            std::string path = readLine("Enter path to .txt file: ");
            if (path.empty()) {
                std::cout << "Path cannot be empty.\n";
                return;
            }
            if (engine.addNewDocument(path)) {
                std::cout << "[SUCCESS] Document added and indexed successfully!\n";
            } else {
                std::cout << "[ERROR] Could not add document. Ensure file exists, is valid .txt, and not duplicate.\n";
            }
        } else if (subChoice == 2) {
            int docId = readInt("Enter DocID to remove: ", 1, 100000);
            if (engine.removeExistingDocument(static_cast<uint32_t>(docId))) {
                std::cout << "[SUCCESS] Document ID " << docId << " removed and pruned from index.\n";
            } else {
                std::cout << "[ERROR] Document ID " << docId << " not found.\n";
            }
        }
    }

    void handleAdjustWeights(SearchEngine& engine) {
        printHeader("ADJUST RANKING WEIGHTS");
        auto& ranking = engine.getRankingEngine();
        std::cout << "Current Weights:\n";
        std::cout << "  Alpha (Relevance Score):  " << ranking.getAlpha() << "\n";
        std::cout << "  Beta  (Popularity Score): " << ranking.getBeta() << "\n";
        std::cout << "Formula: FinalScore = (Alpha * Relevance) + (Beta * Popularity)\n\n";

        std::string alphaStr = readLine("Enter new Alpha (0.0 to 1.0, press Enter to keep current): ");
        double newAlpha = ranking.getAlpha();
        if (!alphaStr.empty()) {
            try {
                newAlpha = std::stod(alphaStr);
            } catch (...) {
                std::cout << "Invalid number format. Keeping current alpha.\n";
            }
        }

        std::string betaStr = readLine("Enter new Beta (0.0 to 1.0, press Enter to keep current): ");
        double newBeta = ranking.getBeta();
        if (!betaStr.empty()) {
            try {
                newBeta = std::stod(betaStr);
            } catch (...) {
                std::cout << "Invalid number format. Keeping current beta.\n";
            }
        }

        ranking.setWeights(newAlpha, newBeta);
        std::cout << "[SUCCESS] Ranking weights updated to: Alpha=" << ranking.getAlpha()
                  << ", Beta=" << ranking.getBeta() << "\n";
    }
}

int main(int argc, char* argv[]) {
    std::string docDir = (argc > 1) ? argv[1] : "documents";
    std::string dataDir = (argc > 2) ? argv[2] : "data";

    SearchEngine engine;
    std::cout << "Initializing Mini Search Engine...\n";
    std::cout << "Document Directory: " << docDir << "\n";
    std::cout << "Data Directory:     " << dataDir << "\n";

    if (!engine.initialize(docDir, dataDir)) {
        std::cerr << "[ERROR] Failed to initialize Search Engine!\n";
        return 1;
    }

    bool running = true;
    while (running) {
        std::cout << "\n";
        std::cout << "========================================\n";
        std::cout << "          MINI SEARCH ENGINE            \n";
        std::cout << "========================================\n";
        std::cout << "Total Documents: " << engine.getDocumentManager().getDocumentCount() << "\n";
        std::cout << "Vocabulary Size: " << engine.getInvertedIndex().getVocabularySize() << " terms\n";
        std::cout << "Index Status:    Ready\n";
        std::cout << "----------------------------------------\n";
        std::cout << "1. Search Documents\n";
        std::cout << "2. View All Documents\n";
        std::cout << "3. View Search History\n";
        std::cout << "4. View Popular Documents\n";
        std::cout << "5. View Document Details\n";
        std::cout << "6. Rebuild Inverted Index\n";
        std::cout << "7. Add / Remove Document\n";
        std::cout << "8. Adjust Ranking Weights\n";
        std::cout << "9. Exit\n";
        std::cout << "========================================\n";

        int choice = readInt("Enter your choice (1-9): ", 1, 9);
        switch (choice) {
            case 1: handleSearch(engine); break;
            case 2: handleViewAllDocuments(engine); break;
            case 3: handleViewSearchHistory(engine); break;
            case 4: handleViewPopularDocuments(engine); break;
            case 5: handleViewDocumentDetails(engine); break;
            case 6: handleRebuildIndex(engine); break;
            case 7: handleAddRemoveDocument(engine); break;
            case 8: handleAdjustWeights(engine); break;
            case 9:
                std::cout << "Saving engine state and exiting...\n";
                engine.saveState();
                std::cout << "Goodbye!\n";
                running = false;
                break;
            default:
                break;
        }
    }

    return 0;
}
