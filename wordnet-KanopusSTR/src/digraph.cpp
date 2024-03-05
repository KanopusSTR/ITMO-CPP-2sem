#include "wordnet.h"

#include <iostream>

Digraph::Digraph(std::istream & hypernyms)
{
    std::string line;
    while (getline(hypernyms, line)) {
        if (!line.empty()) {
            std::string segment;
            unsigned id;
            std::string hyp;
            std::unordered_set<unsigned> hyps;
            std::stringstream hypline(line);
            std::getline(hypline, segment, ',');
            id = std::stoi(segment);
            while (std::getline(hypline, hyp, ',')) {
                hyps.insert(std::stoi(hyp));
            }
            std::getline(hypline, segment);
            hyps.insert(std::stoi(segment));
            graph.insert({id, hyps});
        }
    }
}

std::size_t Digraph::size() const
{
    return graph.size();
}

const std::unordered_set<unsigned> * Digraph::operator[](unsigned v) const
{
    auto it = graph.find(v);
    if (it != graph.end()) {
        return &it->second;
    }
    return nullptr;
}
