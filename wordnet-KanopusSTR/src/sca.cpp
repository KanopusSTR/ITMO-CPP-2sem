#include "wordnet.h"

#include <iostream>

std::pair<unsigned, unsigned> ShortestCommonAncestor::ancestor_common(const std::set<unsigned> & subset_a, const std::set<unsigned> & subset_b) const
{
    static const auto max = std::numeric_limits<unsigned>::max();
    std::unordered_map<unsigned, std::pair<unsigned, unsigned>> lengths;
    std::queue<std::pair<unsigned, bool>> queue;
    for (unsigned edge_a : subset_a) {
        lengths.emplace(edge_a, std::pair<unsigned, unsigned>{0, max});
        queue.emplace(edge_a, false);
    }
    for (unsigned edge_b : subset_b) {
        if (!lengths.emplace(edge_b, std::pair<unsigned, unsigned>{max, 0}).second) {
            return {0, edge_b};
        }
        queue.emplace(edge_b, true);
    }
    std::pair<unsigned, unsigned> min_length{max, 0};
    while (!queue.empty()) {
        std::pair<unsigned, bool> queue_elem = queue.front();
        queue.pop();

        if (!queue_elem.second) {
            if (sca_graph[queue_elem.first] == nullptr) {
                continue;
            }
            for (unsigned hypernym : *sca_graph[queue_elem.first]) {
                if (hypernym != queue_elem.first) {
                    queue.emplace(hypernym, false);
                }
                unsigned local_length = lengths[queue_elem.first].first;
                if (lengths.find(hypernym) == lengths.end()) {
                    lengths.emplace(hypernym, std::pair<unsigned, unsigned>{local_length + 1, max});
                }
                else {
                    lengths[hypernym].first = std::min(lengths[hypernym].first, local_length + 1);
                    if (lengths[hypernym].second != max) {
                        if (lengths[hypernym].first + lengths[hypernym].second < min_length.first) {
                            min_length = {lengths[hypernym].first + lengths[hypernym].second, hypernym};
                        }
                    }
                }
            }
        }
        else {
            if (sca_graph[queue_elem.first] == nullptr) {
                continue;
            }
            for (unsigned hypernym : *sca_graph[queue_elem.first]) {
                if (hypernym != queue_elem.first) {
                    queue.emplace(hypernym, true);
                }
                unsigned local_length = lengths[queue_elem.first].second;
                if (lengths.find(hypernym) == lengths.end()) {
                    lengths.emplace(hypernym, std::pair<unsigned, unsigned>{max, local_length + 1});
                }
                else {
                    lengths[hypernym].second = std::min(lengths[hypernym].second, local_length + 1);
                    if (lengths[hypernym].first != max) {
                        if (lengths[hypernym].first + lengths[hypernym].second < min_length.first) {
                            min_length = {lengths[hypernym].first + lengths[hypernym].second, hypernym};
                        }
                    }
                }
            }
        }
    }
    return min_length;
}

ShortestCommonAncestor::ShortestCommonAncestor(const Digraph & G)
    : sca_graph(G)
{
}

// calculates length of the shortest common ancestor path from node with id 'v' to node with id 'w'
unsigned ShortestCommonAncestor::length(unsigned v, unsigned w) const
{
    return ancestor_common({v}, {w}).first;
}

// returns node id of the shortest common ancestor of nodes v and w
unsigned ShortestCommonAncestor::ancestor(unsigned v, unsigned w) const
{
    return ancestor_common({v}, {w}).second;
}

// calculates length of the shortest common ancestor path from node subset 'subset_a' to node subset 'subset_b'
unsigned ShortestCommonAncestor::length_subset(const std::set<unsigned> & subset_a, const std::set<unsigned> & subset_b) const
{
    return ancestor_common(subset_a, subset_b).first;
}

// returns node id of shortest common ancestor of node subset 'subset_a' and node subset 'subset_b'
unsigned ShortestCommonAncestor::ancestor_subset(const std::set<unsigned> & subset_a, const std::set<unsigned> & subset_b) const
{
    return ancestor_common(subset_a, subset_b).second;
}