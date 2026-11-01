#pragma once

#include <iostream>
#include <string>
#include <cmath>
#include <cstdlib>

// Lightweight modern C++ test framework
static int g_testsRun = 0;
static int g_testsFailed = 0;

#define TEST_CASE(name) void name()

#define RUN_TEST(name) \
    do { \
        ++g_testsRun; \
        std::cout << "[RUN     ] " << #name << "\n"; \
        try { \
            name(); \
            std::cout << "[      OK] " << #name << "\n"; \
        } catch (const std::exception& ex) { \
            ++g_testsFailed; \
            std::cout << "[  FAILED] " << #name << " (Exception: " << ex.what() << ")\n"; \
        } catch (...) { \
            ++g_testsFailed; \
            std::cout << "[  FAILED] " << #name << " (Unknown exception)\n"; \
        } \
    } while (0)

#define ASSERT_TRUE(cond) \
    do { \
        if (!(cond)) { \
            std::cerr << "Assertion failed: (" #cond ") at " << __FILE__ << ":" << __LINE__ << "\n"; \
            throw std::runtime_error("Assertion failed: " #cond); \
        } \
    } while (0)

#define ASSERT_FALSE(cond) \
    do { \
        if (cond) { \
            std::cerr << "Assertion failed: !(" #cond ") at " << __FILE__ << ":" << __LINE__ << "\n"; \
            throw std::runtime_error("Assertion failed: !(" #cond ")"); \
        } \
    } while (0)

#define ASSERT_EQ(val1, val2) \
    do { \
        if (!((val1) == (val2))) { \
            std::cerr << "Assertion failed: " #val1 " == " #val2 " (" \
                      << (val1) << " vs " << (val2) << ") at " << __FILE__ << ":" << __LINE__ << "\n"; \
            throw std::runtime_error("Assertion failed: equality"); \
        } \
    } while (0)

#define ASSERT_NEAR(val1, val2, eps) \
    do { \
        if (std::abs((val1) - (val2)) > (eps)) { \
            std::cerr << "Assertion failed: |" #val1 " - " #val2 "| <= " #eps " (" \
                      << (val1) << " vs " << (val2) << ") at " << __FILE__ << ":" << __LINE__ << "\n"; \
            throw std::runtime_error("Assertion failed: proximity"); \
        } \
    } while (0)

inline int printTestSummary() {
    std::cout << "========================================================\n";
    std::cout << "Tests Run: " << g_testsRun << " | Passed: " << (g_testsRun - g_testsFailed)
              << " | Failed: " << g_testsFailed << "\n";
    std::cout << "========================================================\n";
    return (g_testsFailed == 0) ? 0 : 1;
}
