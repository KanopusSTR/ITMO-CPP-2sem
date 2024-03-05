#include <iosfwd>
#include <istream>
#include <limits>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Digraph
{
    std::unordered_map<unsigned, std::unordered_set<unsigned>> graph;

public:
    Digraph(std::istream & hypernyms);

    std::size_t size() const;

    const std::unordered_set<unsigned> * operator[](unsigned v) const;
};

class ShortestCommonAncestor
{

    std::pair<unsigned, unsigned> ancestor_common(const std::set<unsigned> & subset_a, const std::set<unsigned> & subset_b) const;

public:
    ShortestCommonAncestor(const Digraph & G);

    // calculates length of the shortest common ancestor path from node with id 'v' to node with id 'w'
    unsigned length(unsigned v, unsigned w) const;

    // returns node id of the shortest common ancestor of nodes v and w
    unsigned ancestor(unsigned v, unsigned w) const;

    // calculates length of the shortest common ancestor path from node subset 'subset_a' to node subset 'subset_b'
    unsigned length_subset(const std::set<unsigned> & subset_a, const std::set<unsigned> & subset_b) const;

    // returns node id of shortest common ancestor of node subset 'subset_a' and node subset 'subset_b'
    unsigned ancestor_subset(const std::set<unsigned> & subset_a, const std::set<unsigned> & subset_b) const;

private:
    Digraph sca_graph;
};

class WordNet
{
    using SynsIds = std::unordered_map<std::string, std::set<unsigned>>;
    SynsIds syns_ids;
    std::unordered_map<std::size_t, std::string> glosses;
    ShortestCommonAncestor m_sca;

public:
    WordNet(std::istream & synsets, std::istream & hypernyms);

    /**
     * Simple proxy class used to enumerate nouns.
     *
     * Usage example:
     *
     * WordNet wordnet{...};
     * ...
     * for (const std::string & noun : wordnet.nouns()) {
     *     // ...
     * }
     */
    class Nouns
    {
        const WordNet & m_WordNet;

    public:
        class iterator
        {
            SynsIds::const_iterator m_internal_iterator;

        public:
            using iterator_category = std::forward_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = std::string;
            using pointer = const value_type *;
            using reference = const value_type &;

            iterator() = default;
            iterator(const SynsIds::const_iterator & iter)
                : m_internal_iterator(iter)
            {
            }

            reference operator*() const
            {
                return m_internal_iterator->first;
            }
            pointer operator->() const
            {
                return &m_internal_iterator->first;
            }

            // Prefix increment
            iterator & operator++()
            {
                ++m_internal_iterator;
                return *this;
            }

            // Postfix increment
            iterator operator++(int)
            {
                iterator tmp = *this;
                operator++();
                return tmp;
            }

            friend bool operator==(const iterator & a, const iterator & b)
            {
                return a.m_internal_iterator == b.m_internal_iterator;
            };
            friend bool operator!=(const iterator & a, const iterator & b)
            {
                return !operator==(a, b);
            };
        };

        Nouns(const WordNet & wn)
            : m_WordNet(wn)
        {
        }

        iterator begin() const
        {
            return iterator(m_WordNet.syns_ids.begin());
        }
        iterator end() const
        {
            return iterator(m_WordNet.syns_ids.end());
        }
        unsigned size()
        {
            return m_WordNet.syns_ids.size();
        }
    };

    // lists all nouns stored in WordNet
    Nouns nouns() const;

    // returns 'true' iff 'word' is stored in WordNet
    bool is_noun(const std::string & word) const;

    // returns gloss of "shortest common ancestor" of noun1 and noun2
    std::string sca(const std::string & noun1, const std::string & noun2);

    // calculates distance between noun1 and noun2
    unsigned distance(const std::string & noun1, const std::string & noun2);
};

class Outcast
{
    WordNet & m_wordnet;

public:
    explicit Outcast(WordNet & wordnet);

    // returns outcast word
    std::string outcast(const std::set<std::string> & nouns);
};
