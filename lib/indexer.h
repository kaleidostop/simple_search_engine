#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <stack>
#include <algorithm>

struct PostingElement {
    size_t docID;
    size_t count;
    std::vector<size_t> lines;

    PostingElement(size_t doc, size_t line): docID(doc), count(1), lines({line}) {};

    PostingElement(size_t doc, const std::vector<size_t> & lines): docID(doc), count(lines.size()), lines(lines) {};

    void addWord(size_t line);
};

class TrieNode {
public:
    std::unordered_map<char, std::shared_ptr<TrieNode>> children;
    std::vector<std::shared_ptr<PostingElement>> postings;

    TrieNode() = default;
};


class Trie {
public:
    std::shared_ptr<TrieNode> root;

    Trie() : root(std::make_shared<TrieNode>()) {}

    void insert(const std::string& word, size_t docID, size_t line);

    const std::vector<std::shared_ptr<PostingElement>>& search(const std::string& word) const;

private:
    std::vector<std::shared_ptr<PostingElement>> emptyPosting;
};


class InvertedIndex {
public:
    using postinglist_type = std::vector<std::shared_ptr<PostingElement>>;
    using dict_type = Trie;

    void addDocument(const std::string& name, const std::string& doc);
    void saveIndex(const std::string& filename);
    void loadIndex(const std::string& filename);
    const dict_type & getIndex() const;
    const size_t getN() const;
    const float getAvgDocLength() const;
    const size_t getDocLength(size_t docID) const;
    const std::string & getName(size_t docID) const;

    //for debugging
    void printInfo();

private:
    dict_type dictionary;
    size_t N = 0;
    size_t sumLen = 0;
    std::vector<size_t> docLens;
    std::vector<std::string> names;

    void saveNode(std::ofstream& outFile, const std::shared_ptr<TrieNode>& node);
    void loadNode(std::ifstream& inFile, const std::shared_ptr<TrieNode>& node);
};

void preprocess(std::string& word);