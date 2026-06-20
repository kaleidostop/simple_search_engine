#include "indexer.h"

#include <filesystem>

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_index>\n";
        exit(1);
    }   

    fs::path dirPath = argv[1];  

    if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
        std::cerr << "Error: Directory " << dirPath << " is not a valid directory.\n";
        exit(1);
    }

    InvertedIndex invertedIndex;

    for (const auto& entry : fs::recursive_directory_iterator(dirPath)) {
        if (entry.is_regular_file()) {
              std::ifstream ifs(entry.path());
            if (!ifs.is_open()) {
                std::cerr << "Error opening file " << entry.path() << std::endl;
                continue;
            }
            std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
            invertedIndex.addDocument(entry.path().generic_string(), content);
            std::cout << "Document " << entry.path() << " added to inverted index\n";
        }
    }

    invertedIndex.saveIndex("inverted_index.txt");
    return 0;
}