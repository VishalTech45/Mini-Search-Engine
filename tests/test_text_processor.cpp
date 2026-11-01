#include "test_macros.h"
#include "TextProcessor.h"
#include <vector>

TEST_CASE(testLowerCaseConversion) {
    TextProcessor tp;
    ASSERT_EQ(tp.toLowerCase("HELLO WORLD!"), "hello world!");
    ASSERT_EQ(tp.toLowerCase("C++ Programming"), "c++ programming");
    ASSERT_EQ(tp.toLowerCase(""), "");
}

TEST_CASE(testPunctuationRemoval) {
    TextProcessor tp;
    std::string text = "Data Structures, Algorithms... and OOP!";
    std::string noPunct = tp.removePunctuation(text);
    // Should preserve 'c++' if present
    ASSERT_TRUE(noPunct.find(',') == std::string::npos);
    ASSERT_TRUE(noPunct.find('.') == std::string::npos);
    ASSERT_TRUE(noPunct.find('!') == std::string::npos);

    std::string cppTest = tp.removePunctuation("C++ code");
    ASSERT_TRUE(cppTest.find("++") != std::string::npos);
}

TEST_CASE(testTokenization) {
    TextProcessor tp;
    std::string input = "  Binary Search   Trees and   Graphs! ";
    std::vector<std::string> tokens = tp.tokenize(input);
    ASSERT_EQ(tokens.size(), 5);
    ASSERT_EQ(tokens[0], "binary");
    ASSERT_EQ(tokens[1], "search");
    ASSERT_EQ(tokens[2], "trees");
    ASSERT_EQ(tokens[3], "and");
    ASSERT_EQ(tokens[4], "graphs");
}

TEST_CASE(testStopWordRemoval) {
    TextProcessor tp;
    std::string input = "Data Structures and Algorithms are Important!";
    std::vector<std::string> processed = tp.processText(input, true);
    // "and" and "are" are default stop words
    ASSERT_EQ(processed.size(), 4);
    ASSERT_EQ(processed[0], "data");
    ASSERT_EQ(processed[1], "structures");
    ASSERT_EQ(processed[2], "algorithms");
    ASSERT_EQ(processed[3], "important");
}

TEST_CASE(testEmptyAndRepeatedTokens) {
    TextProcessor tp;
    ASSERT_TRUE(tp.tokenize("").empty());
    ASSERT_TRUE(tp.tokenize("   , , ; : !   ").empty());

    std::string repeated = "apple apple apple banana banana";
    auto tokens = tp.tokenize(repeated);
    ASSERT_EQ(tokens.size(), 5);
}

TEST_CASE(testCanonicalizeQuery) {
    TextProcessor tp;
    std::string q1 = "machine learning";
    std::string q2 = "learning machine";
    std::string q3 = "learning  and  machine"; // stop word "and" filtered
    std::string c1 = tp.canonicalizeQuery(q1);
    std::string c2 = tp.canonicalizeQuery(q2);
    std::string c3 = tp.canonicalizeQuery(q3);

    ASSERT_EQ(c1, "learning machine");
    ASSERT_EQ(c1, c2);
    ASSERT_EQ(c1, c3);
}

TEST_CASE(testCustomStopWords) {
    TextProcessor tp;
    ASSERT_TRUE(tp.isStopWord("the"));
    tp.removeStopWord("the");
    ASSERT_FALSE(tp.isStopWord("the"));

    tp.addStopWord("specialtoken");
    ASSERT_TRUE(tp.isStopWord("specialtoken"));
}

int main() {
    std::cout << ">>> Running TextProcessor Tests <<<\n";
    RUN_TEST(testLowerCaseConversion);
    RUN_TEST(testPunctuationRemoval);
    RUN_TEST(testTokenization);
    RUN_TEST(testStopWordRemoval);
    RUN_TEST(testEmptyAndRepeatedTokens);
    RUN_TEST(testCanonicalizeQuery);
    RUN_TEST(testCustomStopWords);
    return printTestSummary();
}
