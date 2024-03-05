#include "subset.h"

#include "randomized_queue.h"

void subset(unsigned long k, std::istream & in, std::ostream & out)
{
    std::string line;
    randomized_queue<std::string> queue;
    while (std::getline(in, line)) {
        queue.enqueue(line);
    }
    auto iter = queue.begin();
    while (iter != queue.end() && k > 0) {
        out << *iter++ << std::endl;
        --k;
    }
}
