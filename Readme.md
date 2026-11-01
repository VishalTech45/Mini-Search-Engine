# Mini Search Engine

A console-based search engine written in C++17. It reads a folder of local `.txt` files, builds an inverted index in memory and returns ranked results for keyword queries. Ranking combines TF-IDF relevance with a popularity score that is tracked separately for each query.

Built as a B.Tech project at IIIT Bhubaneswar by Vishal Kumar Pandit.

## Features

- Loads every `.txt` file from the `documents/` folder and skips duplicate paths.
- Text processing: lowercase conversion, punctuation removal (`c++` stays one token), whitespace tokenisation and removal of 138 common English stop words.
- Inverted index: `unordered_map<string, vector<Posting>>`, where each posting holds a DocID and a term frequency. Posting lists are kept sorted by DocID.
- AND mode (all words must appear) and OR mode (any word may appear) for multi-word queries.
- TF-IDF ranking with a query-specific popularity boost.
- Result snippets centred on the first matching query term.
- Popularity tracking per (query, document) pair. Opening a result updates the count for that exact query only.
- Persistence: popularity data and search history are saved as JSON in `data/`. Files are written to a temporary file first and then renamed. A corrupted file is backed up as `.corrupted.<timestamp>`.
- Menu options to rebuild the index, add or remove a document, and adjust the ranking weights.

## Project Structure

```
Mini_Search_Engine/
|-- CMakeLists.txt
|-- README.md
|-- documents/      18 sample .txt files
|-- include/        Document, DocumentManager, InvertedIndex, PopularityTracker,
|                   RankingEngine, SearchEngine, StorageManager, TextProcessor
|-- src/            matching .cpp files and main.cpp (console menu)
|-- tests/          5 test programs and test_macros.h
|-- data/           popularity.json, search_history.json (created by the program)
`-- docs/           ARCHITECTURE.md, ALGORITHMS.md, TESTING.md
```

## Classes

| Class | Responsibility |
|---|---|
| `Document` | Stores one file's id, name, path, text, word count, size and modified time. Builds snippets. |
| `DocumentManager` | Scans the folder, loads files, assigns DocIDs, adds, removes and reloads changed files. |
| `TextProcessor` | Lowercasing, punctuation handling, tokenisation, stop words, query key (sorted unique tokens). |
| `InvertedIndex` | Term to posting-list mapping and AND/OR candidate retrieval. |
| `RankingEngine` | TF, IDF, normalisation, final score and sorting. |
| `PopularityTracker` | Nested map `query -> docId -> visits` plus global visit counts. |
| `StorageManager` | JSON save and load for popularity data and search history. |
| `SearchEngine` | Facade that connects all of the above and is used by `main.cpp`. |

## Requirements

- A C++17 compiler (developed with GCC from MSYS2 UCRT64 on Windows 11)
- CMake 3.15 or newer
- Ninja (or any other CMake generator)

## Build and Run

Run these from the project folder:

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

Start the program from the project folder, because it looks for `documents/` and `data/` relative to the current directory:

```bash
# Windows
build\MiniSearchEngine.exe

# Linux / macOS
./build/MiniSearchEngine
```

You can pass other folders as optional arguments: `MiniSearchEngine <documents_dir> <data_dir>`.

## Using the Program

The main menu has nine options:

1. Search documents (choose AND or OR, then optionally open a result)
2. View all documents
3. View search history
4. View popular documents (global and per query)
5. View document details
6. Rebuild the inverted index
7. Add or remove a document
8. Adjust ranking weights
9. Exit (saves state)

Example: searching `python` in OR mode returns `python.txt` first. Opening it records one visit for the query `python`, and the next search for `python` shows that visit in the popularity score.

## How Ranking Works

For each candidate document and query term:

- `TF = occurrences of the term / words in the document`
- `IDF = ln(1 + N / df)`, where N is the number of indexed documents and df the number of documents containing the term
- Relevance is the sum of `TF x IDF` over the query terms, divided by the best score in the result set.
- Popularity is the document's visit count for this query, divided by the highest visit count in the result set.

```
final score = 0.8 x relevance + 0.2 x popularity
```

The weights can be changed from the menu (option 8) but are not saved between runs. Query keys ignore word order and stop words, so `machine learning` and `learning machine` share visit counts.

## Tests

Five test programs (24 tests in total) are built with the project and run through CTest:

```bash
ctest --test-dir build --output-on-failure
```

| Suite | Tests |
|---|---|
| `test_text_processor` | 7 |
| `test_inverted_index` | 5 |
| `test_ranking` | 4 |
| `test_popularity` | 3 |
| `test_search_engine` | 5 |

## Limitations

- Only local `.txt` files in one folder are searched. There is no web crawling.
- Words are matched exactly. There is no stemming, so `algorithm` and `algorithms` give different results.
- AND mode checks that all words appear in a file, not that they appear as a phrase.
- The index and document text are kept in memory and the index is rebuilt at every start.
- Popularity is stored by file name, so renaming a file loses its history.
- Console interface only. The stop-word list is English and text handling is aimed at ASCII.

## Future Scope

Support for PDF and DOCX files, phrase search, search suggestions, better ranking, a graphical interface, a larger document collection, search statistics and faster indexing for large datasets.