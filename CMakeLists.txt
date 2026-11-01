cmake_minimum_required(VERSION 3.15)
project(MiniSearchEngine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable compiler warnings
if(MSVC)
    add_compile_options(/W4 /EHsc)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include_directories(include)

# Core library
set(CORE_SOURCES
    src/Document.cpp
    src/DocumentManager.cpp
    src/TextProcessor.cpp
    src/InvertedIndex.cpp
    src/RankingEngine.cpp
    src/PopularityTracker.cpp
    src/StorageManager.cpp
    src/SearchEngine.cpp
)

add_library(search_engine_core STATIC ${CORE_SOURCES})
target_include_directories(search_engine_core PUBLIC include)

# Main interactive console application
add_executable(MiniSearchEngine src/main.cpp)
target_link_libraries(MiniSearchEngine PRIVATE search_engine_core)

# Automated testing suite
enable_testing()
add_subdirectory(tests)
