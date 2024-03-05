#pragma once

#include <algorithm>
#include <random>

template <class T>
class randomized_queue
{
private:
    std::vector<T> m_queue_elements; // раньше std::vector назывался vector.
    mutable std::random_device rand_dev = std::random_device{};
    mutable std::mt19937 m_rand_engine = std::mt19937(std::random_device{}());

    template <bool is_const>
    class randomQueueIterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = std::conditional_t<is_const, const T, T>;
        using pointer = value_type *;
        using reference = value_type &;

        randomQueueIterator() = default;

        randomQueueIterator(pointer start_vec, std::size_t size, bool is_start, std::random_device & rand_engine)
            : m_index(is_start ? 0 : size)
            , m_start_ptr(start_vec)
            , m_ptr(m_start_ptr + size)
            , m_permutation(size)
        {
            if (m_index != m_permutation.size()) {
                std::mt19937 rand_mt = std::mt19937(rand_engine());
                std::iota(m_permutation.begin(), m_permutation.end(), 0);
                std::shuffle(m_permutation.begin(), m_permutation.end(), rand_mt);
                m_ptr = m_start_ptr + m_permutation[0];
            }
        }

        reference operator*() const
        {
            return *m_ptr;
        }
        pointer operator->() const
        {
            return m_ptr;
        }

        // Prefix increment
        randomQueueIterator & operator++()
        {
            if (++m_index == m_permutation.size()) {
                m_ptr = m_start_ptr + m_permutation.size();
            }
            else {
                m_ptr = m_start_ptr + m_permutation[m_index];
            }
            return *this;
        }

        // Postfix increment
        randomQueueIterator operator++(int)
        {
            randomQueueIterator tmp = *this;
            operator++();
            return tmp;
        }

        friend bool operator==(const randomQueueIterator & a, const randomQueueIterator & b)
        {
            return a.m_ptr == b.m_ptr;
        };
        friend bool operator!=(const randomQueueIterator & a, const randomQueueIterator & b)
        {
            return !operator==(a, b);
        };

    private:
        std::size_t m_index;
        pointer m_start_ptr;
        pointer m_ptr;
        std::vector<std::size_t> m_permutation; // раньше std::vector назывался m_numbers.
    };

public:
    using iterator = randomQueueIterator<false>;
    using const_iterator = randomQueueIterator<true>;

    size_t get_rand(size_t from, size_t to) const
    {
        std::uniform_int_distribution distribution(from, to);
        return distribution(m_rand_engine);
    }

    bool empty() const
    {
        return m_queue_elements.empty();
    }

    size_t size() const
    {
        return m_queue_elements.size();
    }

    template <class P>
    void enqueue(P && value)
    {
        m_queue_elements.emplace_back(std::forward<P>(value));
    }

    T const & sample() const
    {
        size_t index = get_rand(0, m_queue_elements.size() - 1);
        return m_queue_elements[index];
    }

    T dequeue()
    {
        size_t index = get_rand(0, m_queue_elements.size() - 1);
        std::swap(m_queue_elements[index], m_queue_elements.back());
        auto ans = std::move(m_queue_elements.back());
        m_queue_elements.pop_back();
        return ans;
    }

    iterator begin()
    {
        return {m_queue_elements.data(), m_queue_elements.size(), true, rand_dev};
    }

    iterator end()
    {
        return {m_queue_elements.data(), m_queue_elements.size(), false, rand_dev};
    }

    const_iterator cbegin() const
    {
        return {m_queue_elements.data(), m_queue_elements.size(), true, rand_dev};
    }

    const_iterator cend() const
    {
        return {m_queue_elements.data(), m_queue_elements.size(), false, rand_dev};
    }

    const_iterator begin() const
    {
        return cbegin();
    }

    const_iterator end() const
    {
        return cend();
    }
};