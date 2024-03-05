#include "wordnet.h"

#include <iostream>

WordNet::WordNet(std::istream & synsets, std::istream & hypernyms) // NOLINT "error: 'hypernyms' can have 'const' qualifier [const-param]"
    : m_sca(Digraph(hypernyms))
{
    std::string line;
    while (getline(synsets, line)) {
        if (!line.empty()) {
            std::string segment;
            std::stringstream synline(line);
            std::getline(synline, segment, ',');
            unsigned id;
            id = std::stoi(segment);
            std::getline(synline, segment, ',');
            std::stringstream ss(segment);
            while (ss) {
                std::string syn;
                ss >> syn;
                if (!syn.empty()) {
                    syns_ids[syn].insert(id);
                }
            }
            std::getline(synline, segment);
            glosses.insert({id, segment});
        }
    }
}

// lists all nouns stored in WordNet
WordNet::Nouns WordNet::nouns() const
{
    return Nouns(*this);
}

// returns 'true' iff 'word' is stored in WordNet
bool WordNet::is_noun(const std::string & word) const
{
    return syns_ids.find(word) != syns_ids.end();
}

// returns gloss of "shortest common ancestor" of noun1 and noun2
std::string WordNet::sca(const std::string & noun1, const std::string & noun2)
{
    return glosses.at(m_sca.ancestor_subset(syns_ids.at(noun1), syns_ids.at(noun2)));
}

// calculates distance between noun1 and noun2
unsigned WordNet::distance(const std::string & noun1, const std::string & noun2)
{
    return m_sca.length_subset(syns_ids.at(noun1), syns_ids.at(noun2));
}
