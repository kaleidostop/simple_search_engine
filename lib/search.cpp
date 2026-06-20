
#include "search.h"

SearchEngine::SearchEngine(const InvertedIndex& idx) : index(idx) {}

double SearchEngine::calculateBM25(int N, int docFreq, int termFreq, int docLength, float avgDocLength) {
    const double k = 1.2;  // Параметр, который регулирует влияние длины документа
    const double b = 0.75;   // Параметр, который регулирует влияние средней длины документа

    if (docFreq <= 0 || termFreq <= 0 || docLength <= 0) {
        return 0.0;
    }

    double idf = log((N - docFreq + 0.5) / (docFreq + 0.5));
    double tf = (termFreq * (k + 1)) / (termFreq + k * (1 - b + b * ((float) docLength / avgDocLength)));

    return tf * idf;
}

void validateQuery(const std::vector<std::string>& tokens) {
    bool lastWasOperator = true;
    int openBrackets = 0;

    for (const auto& token : tokens) {
        if (token == "AND" || token == "OR") {
            if (lastWasOperator) {
                throw std::runtime_error("Two operators in a row");
            }
            lastWasOperator = true;
        } else if (token == "(") {
            openBrackets++;
            lastWasOperator = true;
        } else if (token == ")") {
            if (lastWasOperator) {
                throw std::runtime_error("Closing bracket after operator");
            } else if (openBrackets == 0) {
                throw std::runtime_error("Hanging closing bracket");
            }
            openBrackets--;
            lastWasOperator = false;
        } else { 
            if (!lastWasOperator) {
                throw std::runtime_error("Two words in a row");
            }
            lastWasOperator = false;
        }
    }

    if (openBrackets != 0) {
        throw std::runtime_error("Brackets are not closed");
    }
    if (lastWasOperator) {
        throw std::runtime_error("Operator at the end");
    }
}

std::vector<std::string> tokenize(const std::string& query) {
    std::vector<std::string> tokens;
    std::string current;

    for (char ch : query) {
        if (ch == '(' || ch == ')') {
            if (current != "AND" && current != "OR") {
                preprocess(current);
            }
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            tokens.emplace_back(1, ch);
        } else if (std::isspace(static_cast<unsigned char>(ch))) {
            if (current != "AND" && current != "OR") {
                preprocess(current);
            }
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        } else {
            current += ch;
        }
    }

    if (!current.empty()) {
        if (current != "AND" && current != "OR") {
            preprocess(current);
        }
        if (!current.empty()) {
            tokens.push_back(current);
            current.clear();
        }
    }

    return tokens;
}


void processSubExpression(std::stack<InvertedIndex::postinglist_type> & operands,
                          std::stack<std::string> & operators) {
    while (!operators.empty()) {
        auto operatorToken = operators.top();
        operators.pop();
        if (operatorToken == "(") {
            break;
        }

        if (operands.size() < 2) {
            throw std::runtime_error("Not enough operands for operator " + operatorToken);
        }
        auto right = operands.top();
        operands.pop();
        auto left = operands.top();
        operands.pop();

        InvertedIndex::postinglist_type result;

        if (operatorToken == "AND") {
            std::unordered_map<size_t, std::shared_ptr<PostingElement>> leftMap; 
            for (const auto& elem : left) {
                leftMap[elem->docID] = std::make_shared<PostingElement>(*elem);
            }

            for (const auto& elem : right) {
                auto it = leftMap.find(elem->docID);
                if (it != leftMap.end()) {
                    auto combinedElement = leftMap[elem->docID];
                    combinedElement->count += elem->count;
                    combinedElement->lines.insert(combinedElement->lines.end(), elem->lines.begin(), elem->lines.end());
                    result.push_back(combinedElement);
                }
            }
        } else if (operatorToken == "OR") {
            std::unordered_map<size_t, std::shared_ptr<PostingElement>> leftMap;
            for (const auto& elem : left) {
                leftMap[elem->docID] = std::make_shared<PostingElement>(*elem);
            }

            for (const auto& elem : right) {
                auto it = leftMap.find(elem->docID);
                if (it != leftMap.end()) {
                    auto combinedElement = it->second;
                    combinedElement->count += elem->count;
                    combinedElement->lines.insert(combinedElement->lines.end(), elem->lines.begin(), elem->lines.end());
                } else {
                    leftMap[elem->docID] = std::make_shared<PostingElement>(*elem);
                }
            }

            for (const auto& pair : leftMap) {
                result.push_back(std::move(pair.second));
            }
        } else {
            throw std::runtime_error("Unknown operator: " + operatorToken);
        }

        operands.push(std::move(result));
    }
}

std::vector<std::string> SearchEngine::search(const std::string& query) {
    // токенизация
    std::vector<std::string> tokens = tokenize(query);
    if (tokens.empty()) {
        throw std::runtime_error("Empty query");
    }

    // сохранение термов запроса (без операторов и скобок) для BM25
    std::vector<std::string> queryTerms;
    for (const auto& token : tokens) {
        if (token != "AND" && token != "OR" && token != "(" && token != ")") {
            queryTerms.push_back(token);
        }
    }
    if (queryTerms.empty()) {
        throw std::runtime_error("No search terms in query");
    }

    // валидация синтаксиса
    validateQuery(tokens);   

    // вычисление логического выражения
    std::stack<InvertedIndex::postinglist_type> operands;
    std::stack<std::string> operators;

    for (const auto& t : tokens) {
        if (t == "AND" || t == "OR") {
            operators.push(t);
        } else if (t == "(") {
            operators.push(t);
        } else if (t == ")") {
            processSubExpression(operands, operators);
        } else {
            const auto& postingList = index.getIndex().search(t);
            InvertedIndex::postinglist_type list(postingList.begin(), postingList.end());
            operands.push(std::move(list));
        }
    }

    processSubExpression(operands, operators);

    if (operands.size() != 1) {
        throw std::runtime_error("Invalid query: incorrect number of operands");
    }

    auto results = std::move(operands.top()); 

    // ранжирование документов по BM25
    const size_t N = index.getN();
    const float avgDocLength = index.getAvgDocLength();

    std::unordered_map<size_t, double> docScores;
    std::unordered_map<size_t, std::unordered_set<size_t>> docLinesSet; 

    for (const auto& posting : results) {
        size_t docID = posting->docID;
        for (const auto& term : queryTerms) {
            const auto& termPostingList = index.getIndex().search(term);
            for (const auto& termPosting : termPostingList) {
                if (termPosting->docID == docID) {
                    size_t termFreq = termPosting->count;
                    size_t docFreq = termPostingList.size(); 
                    size_t docLength = index.getDocLength(docID);
                    double bm25 = calculateBM25(N, docFreq, termFreq, docLength, avgDocLength);
                    docScores[docID] += bm25;

                    for (size_t line : termPosting->lines) {
                        docLinesSet[docID].insert(line);
                    }
                    break;
                }
            }
        }
    }

    std::vector<std::pair<size_t, double>> sortedDocs(docScores.begin(), docScores.end());
    std::sort(sortedDocs.begin(), sortedDocs.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    std::vector<std::string> output;
    size_t rank = 1;
    for (const auto& [docID, score] : sortedDocs) {
        std::string entry = std::to_string(rank) + ". Document: " + index.getName(docID) +
                            "\nScore: " + std::to_string(score);
        entry += "\nin lines: (";
        std::vector<size_t> linesVec(docLinesSet[docID].begin(), docLinesSet[docID].end());
        std::sort(linesVec.begin(), linesVec.end());
        for (size_t i = 0; i < linesVec.size(); ++i) {
            entry += std::to_string(linesVec[i]);
            if (i != linesVec.size() - 1) entry += ", ";
        }
        entry += ")\n";
        output.push_back(entry);
        ++rank;
    }

    return output;
}

// for debugging

void printPostingList(const InvertedIndex & index, const std::string & word) {
    if (!index.getIndex().search(word).empty()) {
        for (const auto &p: index.getIndex().search(word)) {
            size_t d = p->docID;
            size_t c = p->count;
            std::cout << "(" << d << ", " << c << ") ";
        }
        std::cout << "\n";
    }
}

