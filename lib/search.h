#pragma once

#include <cmath>
#include <stack>
#include <unordered_set>

#include "indexer.h"

class SearchEngine {
public:
    SearchEngine(const InvertedIndex& index);
    std::vector<std::string> search(const std::string& query);

private:
    const InvertedIndex & index;
    double calculateBM25(int N, int docFreq, int termFreq, int docLength, float avgDocLength);
};


// for debugging

void printPostingList(const InvertedIndex & index, const std::string & word);
