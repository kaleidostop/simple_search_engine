#include <gtest/gtest.h>
#include "lib/indexer.h"
#include "lib/search.h"

TEST(PostingElementTest, Creation) {
    PostingElement pe(1, 2);
    EXPECT_EQ(pe.docID, 1);
    EXPECT_EQ(pe.count, 1);
    EXPECT_EQ(pe.lines.size(), 1);
    EXPECT_EQ(pe.lines[0], 2);
}

TEST(PostingElementTest, AddWord) {
    PostingElement pe(1, 2);
    pe.addWord(3);
    EXPECT_EQ(pe.count, 2);
    EXPECT_EQ(pe.lines.size(), 2);
    EXPECT_EQ(pe.lines[1], 3);
}

TEST(TrieNodeTest, Creation) {
    TrieNode node;
    EXPECT_TRUE(node.children.empty());
    EXPECT_TRUE(node.postings.empty());
}

TEST(TrieTest, InsertAndSearch) {
    Trie trie;
    trie.insert("test", 1, 10);

    const auto& postings = trie.search("test");
    EXPECT_EQ(postings.size(), 1);
    EXPECT_EQ(postings[0]->docID, 1);
    EXPECT_EQ(postings[0]->count, 1);
    EXPECT_EQ(postings[0]->lines[0], 10);
}

TEST(TrieTest, SearchNonExistentWord) {
    Trie trie;
    const auto& postings = trie.search("nonexistent");
    EXPECT_TRUE(postings.empty());
}

TEST(TrieTest, InsertDuplicateWord) {
    Trie trie;
    trie.insert("test", 1, 10);
    trie.insert("test", 1, 20);

    const auto& postings = trie.search("test");
    EXPECT_EQ(postings.size(), 1);
    EXPECT_EQ(postings[0]->docID, 1);
    EXPECT_EQ(postings[0]->count, 2);
    EXPECT_EQ(postings[0]->lines.size(), 2);
    EXPECT_EQ(postings[0]->lines[0], 10);
    EXPECT_EQ(postings[0]->lines[1], 20);
}

TEST(TrieTest, InsertDifferentDocs) {
    Trie trie;
    trie.insert("test", 1, 10);
    trie.insert("test", 2, 15);

    const auto& postings = trie.search("test");
    EXPECT_EQ(postings.size(), 2);
    EXPECT_EQ(postings[0]->docID, 1);
    EXPECT_EQ(postings[0]->count, 1);
    EXPECT_EQ(postings[0]->lines[0], 10);

    EXPECT_EQ(postings[1]->docID, 2);
    EXPECT_EQ(postings[1]->count, 1);
    EXPECT_EQ(postings[1]->lines[0], 15);
}

TEST(TrieTest, InsertMultipleWords) {
    Trie trie;
    trie.insert("test", 1, 10);
    trie.insert("hello", 1, 5);
    trie.insert("test", 2, 15);

    const auto& postingsTest = trie.search("test");
    EXPECT_EQ(postingsTest.size(), 2);

    const auto& postingsHello = trie.search("hello");
    EXPECT_EQ(postingsHello.size(), 1);
    EXPECT_EQ(postingsHello[0]->docID, 1);
    EXPECT_EQ(postingsHello[0]->count, 1);
    EXPECT_EQ(postingsHello[0]->lines[0], 5);
}

TEST(TrieTest, InsertAndSearchMultipleLines) {
    Trie trie;
    trie.insert("test", 1, 10);
    trie.insert("test", 1, 20);
    trie.insert("test", 1, 30);

    const auto& postings = trie.search("test");
    EXPECT_EQ(postings.size(), 1);
    EXPECT_EQ(postings[0]->docID, 1);
    EXPECT_EQ(postings[0]->count, 3);
    EXPECT_EQ(postings[0]->lines.size(), 3);
    EXPECT_EQ(postings[0]->lines[0], 10);
    EXPECT_EQ(postings[0]->lines[1], 20);
    EXPECT_EQ(postings[0]->lines[2], 30);
}

TEST(TrieTest, SearchWithDifferentCasing) {
    Trie trie;
    trie.insert("Test", 1, 10);

    const auto& postings = trie.search("test");
    EXPECT_TRUE(postings.empty());

    const auto& postingsUpper = trie.search("Test");
    EXPECT_EQ(postingsUpper.size(), 1);
    EXPECT_EQ(postingsUpper[0]->docID, 1);
    EXPECT_EQ(postingsUpper[0]->count, 1);
    EXPECT_EQ(postingsUpper[0]->lines[0], 10);
}

TEST(TrieTest, InsertEmptyString) {
    Trie trie;
    trie.insert("", 1, 0);

    const auto& postings = trie.search("");
    EXPECT_EQ(postings.size(), 1);
    EXPECT_EQ(postings[0]->docID, 1);
    EXPECT_EQ(postings[0]->count, 1);
    EXPECT_EQ(postings[0]->lines[0], 0);
}

TEST(TrieTest, InsertAndSearchSingleCharacter) {
    Trie trie;
    trie.insert("a", 1, 5);
    trie.insert("a", 1, 10);

    const auto& postings = trie.search("a");
    EXPECT_EQ(postings.size(), 1);
    EXPECT_EQ(postings[0]->docID, 1);
    EXPECT_EQ(postings[0]->count, 2);
    EXPECT_EQ(postings[0]->lines.size(), 2);
    EXPECT_EQ(postings[0]->lines[0], 5);
    EXPECT_EQ(postings[0]->lines[1], 10);
}



TEST(InvertedIndexTest, AddDocumentAndGetInfo) {
    InvertedIndex index;
    index.addDocument("doc1", "this is a test");

    EXPECT_EQ(index.getN(), 1);
    EXPECT_EQ(index.getDocLength(0), 4);
    EXPECT_EQ(index.getName(0), "doc1");
}

TEST(InvertedIndexTest, SaveAndLoadIndex) {
    InvertedIndex index1;
    index1.addDocument("doc1", "this is a test");
    index1.saveIndex("index.txt");

    InvertedIndex index2;
    index2.loadIndex("index.txt");

    EXPECT_EQ(index2.getN(), 1);
    EXPECT_EQ(index2.getDocLength(0), index1.getDocLength(0));
    EXPECT_EQ(index2.getName(0), index1.getName(0));
}

TEST(InvertedIndexTest, AddMultipleDocuments) {
    InvertedIndex index;
    index.addDocument("doc1", "this is a test");
    index.addDocument("doc2", "this is another test here");
    index.addDocument("doc3", "hello world");

    EXPECT_EQ(index.getN(), 3);
    EXPECT_EQ(index.getDocLength(0), 4);
    EXPECT_EQ(index.getDocLength(1), 5);
    EXPECT_EQ(index.getDocLength(2), 2);

    EXPECT_EQ(index.getName(0), "doc1");
    EXPECT_EQ(index.getName(1), "doc2");
    EXPECT_EQ(index.getName(2), "doc3");
}

TEST(InvertedIndexTest, CheckPostings) {
    InvertedIndex index;
    index.addDocument("doc1", "this is a test");
    index.addDocument("doc2", "this is another test");

    auto postings = index.getIndex().search("test");
    EXPECT_EQ(postings.size(), 2);
    EXPECT_EQ(postings[0]->docID, 0);
    EXPECT_EQ(postings[0]->count, 1);
    EXPECT_EQ(postings[1]->docID, 1);
    EXPECT_EQ(postings[1]->count, 1);
}

TEST(InvertedIndexTest, SaveAndLoadIndex2) {
    InvertedIndex index1;
    index1.addDocument("doc1", "this is a test");
    index1.addDocument("doc2", "this is another test");
    index1.saveIndex("index.txt");

    InvertedIndex index2;
    index2.loadIndex("index.txt");

    EXPECT_EQ(index2.getN(), 2);
    EXPECT_EQ(index2.getDocLength(0), index1.getDocLength(0));
    EXPECT_EQ(index2.getDocLength(1), index1.getDocLength(1));
    EXPECT_EQ(index2.getName(0), index1.getName(0));
    EXPECT_EQ(index2.getName(1), index1.getName(1));
}

TEST(InvertedIndexTest, SaveAndLoadIndex3) {
    InvertedIndex index1;
    index1.addDocument("doc1", "apple\n"
                               "apricot\n"
                               "avocado - the plural is avocados though you may see avocadoes (less frequently).\n"
                               "banana\n"
                               "blackberry\n"
                               "blackcurrant\n"
                               "blueberry\n"
                               "boysenberry - is a cross between a raspberry and a blackberry\n"
                               "cherry\n"
                               "coconut\n"
                               "fig\n"
                               "grape\n"
                               "grapefruit\n"
                               "kiwifruit - sometimes written as two words kiwi fruit. It has the same form in singular and plural kiwifruit.\n"
                               "lemon\n"
                               "lime\n"
                               "lychee - sometimes called litchi in US English\n"
                               "mandarin\n"
                               "mango - the plural of mango can be either mangos or mangoes.\n"
                               "melon - the generic name for most types of melon\n"
                               "nectarine - the same a peach but without fur on its skin\n"
                               "orange\n"
                               "papaya - In some countries it is called pawpaw.\n"
                               "passion fruit - In United States it is written as two words while in some countries it is written as one word: passionfruit. The plural of passion fruit is either passion fruit or passion fruits. See our notes about the plural of fruit above.\n"
                               "peach - same as a nectarine but with a slight fur on its skin\n"
                               "pear\n"
                               "pineapple\n"
                               "plum\n"
                               "pomegranate\n"
                               "quince\n"
                               "raspberry\n"
                               "strawberry\n"
                               "watermelon");
    index1.addDocument("doc2", "To be, or not to be, that is the question:\n"
                               "Whether 'tis nobler in the mind to suffer\n"
                               "The slings and arrows of outrageous fortune,\n"
                               "Or to take arms against a sea of troubles\n"
                               "And by opposing end them. To die—to sleep,\n"
                               "No more; and by a sleep to say we end\n"
                               "The heart-ache and the thousand natural shocks\n"
                               "That flesh is heir to: 'tis a consummation\n"
                               "Devoutly to be wish'd. To die, to sleep;\n"
                               "To sleep, perchance to dream—ay, there's the rub:\n"
                               "For in that sleep of death what dreams may come,\n"
                               "When we have shuffled off this mortal coil,\n"
                               "Must give us pause—there's the respect\n"
                               "That makes calamity of so long life.\n"
                               "For who would bear the whips and scorns of time,\n"
                               "Th'oppressor's wrong, the proud man's contumely,\n"
                               "The pangs of dispriz'd love, the law's delay,\n"
                               "The insolence of office, and the spurns\n"
                               "That patient merit of th'unworthy takes,\n"
                               "When he himself might his quietus make\n"
                               "With a bare bodkin? Who would fardels bear,\n"
                               "To grunt and sweat under a weary life,\n"
                               "But that the dread of something after death,\n"
                               "The undiscovere'd country, from whose bourn\n"
                               "No traveller returns, puzzles the will,\n"
                               "And makes us rather bear those ills we have\n"
                               "Than fly to others that we know not of?\n"
                               "Thus conscience doth make cowards of us all,\n"
                               "And thus the native hue of resolution\n"
                               "Is sicklied o'er with the pale cast of thought,\n"
                               "And enterprises of great pith and moment\n"
                               "With this regard their currents turn awry\n"
                               "And lose the name of action.");
    index1.saveIndex("index.txt");

    InvertedIndex index2;
    index2.loadIndex("index.txt");

    EXPECT_EQ(index2.getN(), 2);
    EXPECT_EQ(index2.getDocLength(0), index1.getDocLength(0));
    EXPECT_EQ(index2.getDocLength(1), index1.getDocLength(1));
    EXPECT_EQ(index2.getName(0), index1.getName(0));
    EXPECT_EQ(index2.getName(1), index1.getName(1));
    EXPECT_EQ(index2.getIndex().search("lemon").size(), index1.getIndex().search("lemon").size());
}

TEST(InvertedIndexTest, AddSameDocumentMultipleTimes) {
    InvertedIndex index;
    index.addDocument("doc1", "this is a test");
    index.addDocument("doc1", "this is a test");

    EXPECT_EQ(index.getN(), 1);
    EXPECT_EQ(index.getDocLength(0), 4);
}

TEST(InvertedIndexTest, AddDocument_InsertsWordsCorrectly) {
    InvertedIndex index;

    std::string docName = "doc1";
    std::string docContent = "hello world hello";

    index.addDocument(docName, docContent);

    const auto& postingsHello = index.getIndex().search("hello");
    const auto& postingsWorld = index.getIndex().search("world");

    ASSERT_EQ(postingsHello.size(), 1);
    ASSERT_EQ(postingsHello[0]->docID, 0);
    ASSERT_EQ(postingsHello[0]->count, 2);
    ASSERT_EQ(postingsHello[0]->lines.size(), 2);

    ASSERT_EQ(postingsWorld.size(), 1);
    ASSERT_EQ(postingsWorld[0]->docID, 0);
    ASSERT_EQ(postingsWorld[0]->count, 1);
    ASSERT_EQ(postingsWorld[0]->lines.size(), 1);
    ASSERT_EQ(postingsWorld[0]->lines[0], 1);
}

TEST(InvertedIndexTest, MultipleDocuments) {
    InvertedIndex index;

    index.addDocument("doc1", "the quick brown fox");
    index.addDocument("doc2", "the quick red fox");

    const auto& postingsThe = index.getIndex().search("the");
    const auto& postingsQuick = index.getIndex().search("quick");
    const auto& postingsFox = index.getIndex().search("fox");

    ASSERT_EQ(postingsThe.size(), 2);
    ASSERT_EQ(postingsQuick.size(), 2);
    ASSERT_EQ(postingsFox.size(), 2);

    ASSERT_EQ(postingsThe[0]->docID, 0);
    ASSERT_EQ(postingsThe[1]->docID, 1);
    ASSERT_EQ(postingsQuick[0]->docID, 0);
    ASSERT_EQ(postingsQuick[1]->docID, 1);
    ASSERT_EQ(postingsFox[0]->docID, 0);
    ASSERT_EQ(postingsFox[1]->docID, 1);
}

TEST(InvertedIndexTest, Words) {
    InvertedIndex index;

    index.addDocument("doc1", "apple\n"
                              "apricot\n"
                              "avocado - the plural is avocados though you may see avocadoes (less frequently).\n"
                              "banana\n"
                              "blackberry\n"
                              "blackcurrant\n"
                              "blueberry\n"
                              "boysenberry - is a cross between a raspberry and a blackberry\n"
                              "cherry\n"
                              "coconut\n"
                              "fig\n"
                              "grape\n"
                              "grapefruit\n"
                              "kiwifruit - sometimes written as two words kiwi fruit. It has the same form in singular and plural kiwifruit.\n"
                              "lemon\n"
                              "lime\n"
                              "lychee - sometimes called litchi in US English\n"
                              "mandarin\n"
                              "mango - the plural of mango can be either mangos or mangoes.\n"
                              "melon - the generic name for most types of melon\n"
                              "nectarine - the same a peach but without fur on its skin\n"
                              "orange\n"
                              "papaya - In some countries it is called pawpaw.\n"
                              "passion fruit - In United States it is written as two words while in some countries it is written as one word: passionfruit. The plural of passion fruit is either passion fruit or passion fruits. See our notes about the plural of fruit above.\n"
                              "peach - same as a nectarine but with a slight fur on its skin\n"
                              "pear\n"
                              "pineapple\n"
                              "plum\n"
                              "pomegranate\n"
                              "quince\n"
                              "raspberry\n"
                              "strawberry\n"
                              "watermelon");
    index.addDocument("doc2", "To be, or not to be, that is the question:\n"
                              "Whether 'tis nobler in the mind to suffer\n"
                              "The slings and arrows of outrageous fortune,\n"
                              "Or to take arms against a sea of troubles\n"
                              "And by opposing end them. To die—to sleep,\n"
                              "No more; and by a sleep to say we end\n"
                              "The heart-ache and the thousand natural shocks\n"
                              "That flesh is heir to: 'tis a consummation\n"
                              "Devoutly to be wish'd. To die, to sleep;\n"
                              "To sleep, perchance to dream—ay, there's the rub:\n"
                              "For in that sleep of death what dreams may come,\n"
                              "When we have shuffled off this mortal coil,\n"
                              "Must give us pause—there's the respect\n"
                              "That makes calamity of so long life.\n"
                              "For who would bear the whips and scorns of time,\n"
                              "Th'oppressor's wrong, the proud man's contumely,\n"
                              "The pangs of dispriz'd love, the law's delay,\n"
                              "The insolence of office, and the spurns\n"
                              "That patient merit of th'unworthy takes,\n"
                              "When he himself might his quietus make\n"
                              "With a bare bodkin? Who would fardels bear,\n"
                              "To grunt and sweat under a weary life,\n"
                              "But that the dread of something after death,\n"
                              "The undiscovere'd country, from whose bourn\n"
                              "No traveller returns, puzzles the will,\n"
                              "And makes us rather bear those ills we have\n"
                              "Than fly to others that we know not of?\n"
                              "Thus conscience doth make cowards of us all,\n"
                              "And thus the native hue of resolution\n"
                              "Is sicklied o'er with the pale cast of thought,\n"
                              "And enterprises of great pith and moment\n"
                              "With this regard their currents turn awry\n"
                              "And lose the name of action.");

    const auto& postingsLemon = index.getIndex().search("lemon");
    const auto& postingsLime = index.getIndex().search("lime");

    ASSERT_EQ(postingsLemon[0]->count, 1);
    ASSERT_EQ(postingsLime.size(), 1);
}

TEST(InvertedIndexTest, AverageDocumentLength) {
    InvertedIndex index;

    index.addDocument("doc1", "one two three");
    index.addDocument("doc2", "four five");

    ASSERT_EQ(index.getN(), 2);
    ASSERT_FLOAT_EQ(index.getAvgDocLength(), 2.5f);
}

TEST(InvertedIndexTest, DocumentLength) {
    InvertedIndex index;

    index.addDocument("doc1", "line one two");
    index.addDocument("doc2", "another line three four");

    ASSERT_EQ(index.getDocLength(0), 3);
    ASSERT_EQ(index.getDocLength(1), 4);
}

TEST(InvertedIndexTest, GetDocumentName) {
    InvertedIndex index;

    index.addDocument("doc1", "sample text");
    index.addDocument("doc2", "another text");

    ASSERT_EQ(index.getName(0), "doc1");
    ASSERT_EQ(index.getName(1), "doc2");
}



class SearchEngineTest : public ::testing::Test {
protected:
    InvertedIndex index;
    SearchEngine searchEngine{index};

    void SetUp() {
        index.addDocument("doc1", "lemon lime limon");
        index.addDocument("doc2", "cat dog dog cat");
        index.addDocument("doc3", "lemon cat lime dog some thing");
        index.addDocument("doc5", "hello world");
        index.addDocument("doc6", "hello everyone");
        index.addDocument("doc7", "some words");
        index.addDocument("doc8", "some test of lemon");
        index.addDocument("doc9", "just some TEST");
        index.addDocument("doc10", "so many tests");
    }
};

TEST_F(SearchEngineTest, SearchSingleTerm) {
    auto results = searchEngine.search("cat");

    EXPECT_EQ(results.size(), 2);

    EXPECT_NE(results[0].find("doc2"), std::string::npos);
    EXPECT_NE(results[1].find("doc3"), std::string::npos);
}

TEST_F(SearchEngineTest, SearchSingleTermParenthesis) {
    auto results = searchEngine.search("(cat)");

    EXPECT_EQ(results.size(), 2);

    EXPECT_NE(results[0].find("doc2"), std::string::npos);
    EXPECT_NE(results[1].find("doc3"), std::string::npos);
}

TEST_F(SearchEngineTest, SearchAND) {
    auto results = searchEngine.search("lime AND lemon");

    EXPECT_EQ(results.size(), 2);
    EXPECT_NE(results[0].find("doc1"), std::string::npos);
    EXPECT_NE(results[1].find("doc3"), std::string::npos);
}

TEST_F(SearchEngineTest, SearchOR) {
    auto results = searchEngine.search("lemon OR dog");

    EXPECT_EQ(results.size(), 4);
    EXPECT_NE(results[0].find("doc2"), std::string::npos);
    EXPECT_NE(results[1].find("doc3"), std::string::npos);
    EXPECT_NE(results[2].find("doc1"), std::string::npos);
    EXPECT_NE(results[3].find("doc8"), std::string::npos);
}

TEST_F(SearchEngineTest, SearchANDCommutativity) {
    auto results1 = searchEngine.search("lime AND lemon");
    auto results2 = searchEngine.search("lemon AND lime");

    EXPECT_EQ(results1.size(), results2.size());
    EXPECT_EQ(results1[0].find("doc1"), results2[0].find("doc1"));
    EXPECT_EQ(results1[1].find("doc3"), results2[1].find("doc3"));
}

TEST_F(SearchEngineTest, SearchORCommutativity) {
    auto results1 = searchEngine.search("lemon OR dog");
    auto results2 = searchEngine.search("dog OR lemon");

    EXPECT_EQ(results1.size(), results2.size());
    EXPECT_EQ(results1[0].find("doc2"), results2[0].find("doc2"));
    EXPECT_EQ(results1[1].find("doc3"), results2[1].find("doc3"));
    EXPECT_EQ(results1[2].find("doc1"), results2[2].find("doc1"));
    EXPECT_EQ(results1[3].find("doc8"), results2[3].find("doc8"));
}


TEST_F(SearchEngineTest, SearchWithParentheses) {
    auto results = searchEngine.search("some AND (test OR lemon)");

    EXPECT_EQ(results.size(), 3);
    EXPECT_NE(results[0].find("doc8"), std::string::npos);
    EXPECT_NE(results[1].find("doc9"), std::string::npos);
    EXPECT_NE(results[2].find("doc3"), std::string::npos);
}

TEST_F(SearchEngineTest, SearchWithParenthesesCommutativity) {
    auto results1 = searchEngine.search("some AND (test OR lemon)");
    auto results2 = searchEngine.search("(test OR lemon) AND some");

    EXPECT_EQ(results1.size(), results2.size());
    EXPECT_EQ(results1[0].find("doc8"), results2[0].find("doc8"));
    EXPECT_EQ(results1[1].find("doc9"), results2[1].find("doc9"));
    EXPECT_EQ(results1[2].find("doc3"), results2[2].find("doc3"));
}

TEST_F(SearchEngineTest, SearchWithParenthesesDistributivity) {
    auto results1 = searchEngine.search("some AND (test OR lemon)");
    auto results2 = searchEngine.search("(some AND test) OR (some AND lemon)");

    EXPECT_EQ(results1.size(), results2.size());
    EXPECT_EQ(results1[0].find("doc8"), results2[0].find("doc8"));
    EXPECT_EQ(results1[1].find("doc9"), results2[1].find("doc9"));
    EXPECT_EQ(results1[2].find("doc3"), results2[2].find("doc3"));
}

TEST_F(SearchEngineTest, SearchNonExistentTerm) {
    auto results = searchEngine.search("unicorn");

    EXPECT_EQ(results.size(), 0);
}