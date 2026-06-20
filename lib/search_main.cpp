#include "search.h"

int main() {
    InvertedIndex index;
    index.loadIndex("inverted_index.txt");
    SearchEngine searcher = SearchEngine(index);

    //index.printInfo();

    std::cout << "Welcome to the search engine! Type your search query and press Enter.\n";
    std::cout << "Type 'q' to quit the program.\n";

    std::string query;
    while (true) {
        std::cout << "Enter search query: ";
        std::getline(std::cin, query);

        if (query == "q") {
            break;
        }

        try {
            std::vector<std::string> results = searcher.search(query);

            if (results.empty()) {
                std::cout << "No results found.\n";
            } else {
                std::cout << "Results found:\n";
                for (const auto &res : results) {
                    std::cout << res << "\n";
                }
            }
        } catch (const std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }

    return 0;
}
