#include "wordnet.h"

#include <iostream>

Outcast::Outcast(WordNet & wordnet)
    : m_wordnet(wordnet)
{
}

// returns outcast word
std::string Outcast::outcast(const std::set<std::string> & nouns)
{
    std::vector<unsigned> lengths(nouns.size(), 0);
    unsigned max_length = 0;
    std::string final_word;
    unsigned index1 = 0;
    unsigned max = std::numeric_limits<unsigned>::max();
    for (auto it1 = nouns.begin(); it1 != nouns.end(); it1++, index1++) {
        unsigned index2 = index1 + 1;
        for (auto it2 = std::next(it1); it2 != nouns.end(); it2++, index2++) {
            unsigned sum = m_wordnet.distance(*it1, *it2);
            if (sum != max) {
                lengths[index1] += sum;
                lengths[index2] += sum;
            }
        }
        if (lengths[index1] == max_length) {
            max_length = 0;
            final_word.clear();
        }
        else {
            if (lengths[index1] > max_length) {
                max_length = lengths[index1];
                final_word = *it1;
            }
        }
    }
    return *&final_word;
}