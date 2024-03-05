#pragma once

#include "Component.h"
#include "pugixml.hpp"

#include <filesystem>
#include <string>
#include <variant>
#include <vector>

struct Component;

class Combinations
{
public:
    struct Combination
    {
        std::string name;
        std::string cardinality;
        std::size_t mincount;

        struct Leg
        {
            Leg() = default;

            InstrumentType type{InstrumentType::Unknown};
            std::variant<double, char> ratio;
            std::variant<int, char> strike = 0;
            int strike_offset_count = 0;
            std::variant<bool, char> expiration = false;
            std::variant<int, std::string> expiration_offset = false;
        };

        std::vector<Leg> legs;
    };

    Combinations() = default;

    bool load(const std::filesystem::path & resource);

    std::string classify(const std::vector<Component> & components, std::vector<int> & order) const;

private:
    std::vector<Combination> m_combinations;
};
