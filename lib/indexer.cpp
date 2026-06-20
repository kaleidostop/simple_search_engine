#include "indexer.h"


void Trie::insert(const std::string& word, size_t docID, size_t line) {
    auto currentNode = root;
    for (char c : word) {
        if (!currentNode->children.count(c)) {
            currentNode->children[c] = std::make_shared<TrieNode>();
        }
        currentNode = currentNode->children[c];
    }
    auto& postings = currentNode->postings;
    if (!postings.empty() && postings.back()->docID == docID) {
        postings.back()->addWord(line);
    } else {
        postings.push_back(std::make_shared<PostingElement>(docID, line));
    }
}

const std::vector<std::shared_ptr<PostingElement>>& Trie::search(const std::string& word) const {
    auto currentNode = root;
    for (char c : word) {
        if (!currentNode->children.count(c)) {
            return emptyPosting;
        }
        currentNode = currentNode->children[c];
    }
    return currentNode->postings;
}


void PostingElement::addWord(size_t line) {
    lines.push_back(line);
    count++;
}

void preprocess(std::string& word) {
    std::transform(word.begin(), word.end(), word.begin(),
                   [](unsigned char c) { return std::tolower(c); });
}

void InvertedIndex::addDocument(const std::string& name, const std::string& doc) {
    size_t docID = N;

    if (std::find(names.begin(), names.end(), name) != names.end()) {
        return;
    }

    std::istringstream iss(doc);
    std::string word;
    size_t docLen = 0;
    size_t line_num = 1;
    std::string line;

    while (std::getline(iss, line)) {
        std::replace_if(line.begin(), line.end(),
                    [](char c) { return std::ispunct(static_cast<unsigned char>(c)); }, ' ');

        std::istringstream lineStream(line);
        while (lineStream >> word) {
            preprocess(word);
            if (word.empty()) {
                continue;
            }
            dictionary.insert(word, docID, line_num);
            ++docLen;
        }
        ++line_num;
    }

    ++N;
    sumLen += docLen;
    docLens.push_back(docLen);
    names.push_back(name);
}

void InvertedIndex::saveIndex(const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Error. Could not open file for saving index.";
        exit(1);
    }

    outFile << N << "\n";
    outFile << sumLen << "\n";

    for (const auto& name : names) {
        outFile << name << "\n";
    }

    for (const auto& length : docLens) {
        outFile << length << "\n";
    }

    saveNode(outFile, dictionary.root);
    outFile.close();
}


void InvertedIndex::saveNode(std::ofstream& outFile, const std::shared_ptr<TrieNode>& node) {
    outFile << node->children.size() << "\n";

    for (const auto& child : node->children) {
        outFile << child.first << "\n";
        saveNode(outFile, child.second);
    }

    outFile << node->postings.size() << "\n";

    for (const auto& posting : node->postings) {
        outFile << posting->docID << " " << posting->count << " " << posting->lines.size() << " ";
        for (const auto& line : posting->lines) {
            outFile << line << " ";
        }
        outFile << "\n";
    }
}


void InvertedIndex::loadIndex(const std::string& filename) {
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        std::cerr << "Error. Could not open file for loading index.";
        exit(1);
    }

    inFile >> N;
    inFile >> sumLen;

    names.resize(N);
    for (size_t i = 0; i < N; ++i) {
        inFile >> names[i];
    }

    docLens.resize(N);
    for (size_t i = 0; i < N; ++i) {
        inFile >> docLens[i];
    }

    loadNode(inFile, dictionary.root);
    inFile.close();
}

void InvertedIndex::loadNode(std::ifstream& inFile, const std::shared_ptr<TrieNode>& node) {
    size_t numChildren;
    inFile >> numChildren;

    for (size_t i = 0; i < numChildren; ++i) {
        char c;
        inFile >> c;
        auto childNode = std::make_shared<TrieNode>();
        node->children[c] = childNode;

        loadNode(inFile, childNode);
    }

    size_t numPostings;
    inFile >> numPostings;
    for (size_t j = 0; j < numPostings; ++j) {
        size_t docID, count, numLines;
        inFile >> docID >> count >> numLines;

        std::vector<size_t> lines(numLines);
        for (size_t k = 0; k < numLines; ++k) {
            inFile >> lines[k];
        }

        auto posting = std::make_shared<PostingElement>(docID, lines);
        node->postings.push_back(posting);
    }
}



const InvertedIndex::dict_type & InvertedIndex::getIndex() const {
    return dictionary;
}

const size_t InvertedIndex::getN() const {
    return N;
}

const float InvertedIndex::getAvgDocLength() const {
    return (float)sumLen / N;
}

const size_t InvertedIndex::getDocLength(size_t docID) const {
    return docLens[docID];
}

const std::string &InvertedIndex::getName(size_t docID) const {
    return names[docID];
}

void InvertedIndex::printInfo() {
    std::cout << "N: " << N << "\n";
    std::cout << "average len: " << getAvgDocLength() << "\n";
    for (auto const& name : names) {
        std::cout << name << "\n";
    }
    for (auto const & docLen: docLens) {
        std::cout << docLen << " ";
    }
    std::cout << '\n';
}


