#include "Combinations.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <map>
#include <numeric>

int compare_tm(std::tm tm1, std::tm tm2);
bool operator==(std::tm tm1, std::tm tm2);
bool operator!=(std::tm tm1, std::tm tm2);
int tm_diff_day(std::tm tm1, std::tm tm2);

bool Combinations::load(const std::filesystem::path & resource)
{
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(resource.c_str());
    Combination comb;
    if (!result) {
        return false;
    }
    for (const pugi::xml_node combination : doc.child("combinations")) {
        std::vector<Combination::Leg> legs;
        comb.name = combination.attribute("name").value();
        comb.cardinality = combination.child("legs").attribute("cardinality").value();
        for (const pugi::xml_node leg : combination.child("legs")) {
            Combination::Leg l;
            l.type = static_cast<InstrumentType>(leg.attribute("type").value()[0]);
            pugi::xml_attribute tmp_ratio = leg.attribute("ratio");

            if (std::string(tmp_ratio.value()).size() == 1 &&
                (tmp_ratio.value()[0] == '+' || tmp_ratio.value()[0] == '-')) {
                l.ratio = tmp_ratio.value()[0];
            }
            else {
                l.ratio = leg.attribute("ratio").as_double();
            }
            if (leg.attribute("strike") != nullptr) {
                l.strike = leg.attribute("strike").value()[0];
            }
            if (leg.attribute("strike_offset") != nullptr) {
                int sign;
                if (leg.attribute("strike_offset").value()[0] == '+') {
                    sign = 1;
                }
                else {
                    sign = -1;
                }
                l.strike_offset_count = static_cast<int>(std::string(leg.attribute("strike_offset").value()).size()) * sign;
            }
            if (leg.attribute("expiration") != nullptr) {
                l.expiration = leg.attribute("expiration").value()[0];
            }
            if (leg.attribute("expiration_offset") != nullptr) {
                if (leg.attribute("expiration_offset").value()[0] == '+') {
                    l.expiration_offset = static_cast<int>(std::string(leg.attribute("expiration_offset").value()).size());
                }
                else if (leg.attribute("expiration_offset").value()[0] == '-') {
                    l.expiration_offset = -static_cast<int>(std::string(leg.attribute("expiration_offset").value()).size());
                }
                else {
                    l.expiration_offset = leg.attribute("expiration_offset").value();
                }
            }
            legs.emplace_back(l);
        }
        comb.legs = legs;
        if (combination.child("legs").attribute("mincount") != nullptr) {
            comb.mincount = std::stoi(combination.child("legs").attribute("mincount").value());
        }
        else {
            comb.mincount = comb.legs.size();
        }
        m_combinations.emplace_back(comb);
    }
    return true;
}

bool fixed_classify(const Combinations::Combination & combination, const std::vector<Component> & components, std::vector<int> & order)
{
    do {
        std::map<int, double> strike_offset_distribution = {};
        std::map<char, double> strike_distribution = {};
        std::map<int, std::tm> expiration_offset_distribution = {};
        std::map<char, std::tm> expiration_distribution = {};
        bool good = true;
        for (std::size_t i = 0; i < combination.mincount; ++i) {
            auto & leg = combination.legs[i];
            const Component & comp = components[order[i]];
            if (comp.type != leg.type) {
                good = false;
                break;
            }
            if (std::holds_alternative<double>(leg.ratio) &&
                std::get<double>(leg.ratio) != comp.ratio) {
                good = false;
                break;
            }
            if (std::holds_alternative<char>(leg.ratio) &&
                ((std::get<char>(leg.ratio) == '+' && comp.ratio < 0) || (std::get<char>(leg.ratio) == '-' && comp.ratio > 0))) {
                good = false;
                break;
            }
            if (leg.strike_offset_count != 0) {
                if (strike_offset_distribution.find(leg.strike_offset_count) !=
                    strike_offset_distribution.end()) {
                    if (strike_offset_distribution.at(leg.strike_offset_count) != comp.strike) {
                        good = false;
                        break;
                    }
                }
                else {
                    if (leg.strike_offset_count > 0) {
                        if (comp.strike > strike_offset_distribution.at(leg.strike_offset_count - 1)) {
                            strike_offset_distribution.insert({leg.strike_offset_count, comp.strike});
                        }
                        else {
                            good = false;
                            break;
                        }
                    }
                    else {
                        if (comp.strike < strike_offset_distribution.at(leg.strike_offset_count + 1)) {
                            strike_offset_distribution.insert({leg.strike_offset_count, comp.strike});
                        }
                        else {
                            good = false;
                            break;
                        }
                    }
                }
            }
            else {
                strike_offset_distribution.clear();
                strike_offset_distribution.insert({0, comp.strike});
            }
            if (std::holds_alternative<int>(leg.expiration_offset)) {
                int expiration_offset_count = std::get<int>(leg.expiration_offset);
                if (expiration_offset_count != 0) {
                    if (expiration_offset_distribution.find(expiration_offset_count) != expiration_offset_distribution.end()) {
                        if (expiration_offset_distribution.at(expiration_offset_count) != comp.expiration) {
                            good = false;
                            break;
                        }
                    }
                    else {
                        int diff = expiration_offset_count > 0 ? -1 : 1;
                        if (compare_tm(comp.expiration, expiration_offset_distribution.at(expiration_offset_count + diff)) != diff) {
                            good = false;
                            break;
                        }
                    }
                }
                else {
                    expiration_offset_distribution.clear();
                }
                expiration_offset_distribution.insert({expiration_offset_count, comp.expiration});
            }
            if (std::holds_alternative<char>(leg.strike)) {
                if (strike_distribution.find(std::get<char>(leg.strike)) != strike_distribution.end()) {
                    if (strike_distribution.at(std::get<char>(leg.strike)) != comp.strike) {
                        good = false;
                        break;
                    }
                }
                else {
                    strike_distribution.insert({std::get<char>(leg.strike), comp.strike});
                }
            }
            if (std::holds_alternative<char>(leg.expiration)) {
                if (expiration_distribution.find(std::get<char>(leg.expiration)) !=
                    expiration_distribution.end()) {
                    if (compare_tm(expiration_distribution.at(std::get<char>(leg.expiration)), comp.expiration) != 0) {
                        good = false;
                        break;
                    }
                }
                else {
                    expiration_distribution.insert({std::get<char>(leg.expiration), comp.expiration});
                }
            }
            if (std::holds_alternative<std::string>(leg.expiration_offset)) {
                const std::string & number_expr = std::get<std::string>(leg.expiration_offset);
                if (number_expr.back() == 'y') {
                    if (comp.expiration.tm_year - expiration_offset_distribution[0].tm_year !=
                        std::stoi(number_expr.substr(0, number_expr.size() - 1))) {
                        good = false;
                        break;
                    }
                    else {
                        if (comp.expiration.tm_mon != expiration_offset_distribution[0].tm_mon ||
                            comp.expiration.tm_mday != expiration_offset_distribution[0].tm_mday) {
                            good = false;
                            break;
                        }
                    }
                }
                if (number_expr.back() == 'q') {
                    int mon_diff = comp.expiration.tm_mon - expiration_offset_distribution[0].tm_mon;
                    int diff;
                    if (mon_diff > 0) {
                        diff = 2;
                    }
                    else {
                        diff = -2;
                    }
                    if ((comp.expiration.tm_year - expiration_offset_distribution[0].tm_year) * 4 + (mon_diff + diff) / 3 !=
                        std::stoi(number_expr.substr(0, number_expr.size() - 1))) {
                        good = false;
                        break;
                    }
                }
                if (number_expr.back() == 'm') {
                    int day;
                    int mon_diff;
                    if ((expiration_offset_distribution[0].tm_mon + std::stoi(number_expr.substr(0, number_expr.size() - 1))) % 12 == 1) {
                        if (comp.expiration.tm_year % 4 == 0) {
                            day = expiration_offset_distribution[0].tm_mday % 29;
                        }
                        else {
                            day = expiration_offset_distribution[0].tm_mday % 28;
                        }
                        if (expiration_offset_distribution[0].tm_mday > 27) {
                            mon_diff = (comp.expiration.tm_year - expiration_offset_distribution[0].tm_year) * 12 +
                                    (comp.expiration.tm_mon - expiration_offset_distribution[0].tm_mon) - 1;
                        }
                        else {
                            mon_diff = (comp.expiration.tm_year - expiration_offset_distribution[0].tm_year) * 12 +
                                    (comp.expiration.tm_mon - expiration_offset_distribution[0].tm_mon);
                        }
                    }
                    else {
                        day = expiration_offset_distribution[0].tm_mday;
                        mon_diff = (comp.expiration.tm_year - expiration_offset_distribution[0].tm_year) * 12 +
                                (comp.expiration.tm_mon - expiration_offset_distribution[0].tm_mon);
                    }
                    if ((mon_diff != std::stoi(number_expr.substr(0, number_expr.size() - 1))) ||
                        comp.expiration.tm_mday != day) {
                        good = false;
                        break;
                    }
                }
                if (number_expr.back() == 'd') {
                    int days_count = tm_diff_day(comp.expiration, expiration_offset_distribution[0]);
                    if (days_count != std::stoi(number_expr.substr(0, number_expr.size() - 1))) {
                        good = false;
                        break;
                    }
                }
            }
        }
        if (good) {
            for (std::size_t j = 0; j < order.size(); ++j) {
                ++order[j];
            }
            return true;
        }
    } while (std::next_permutation(order.begin(), order.end()));
    order.clear();
    return false;
}

std::string Combinations::classify(const std::vector<Component> & components, std::vector<int> & order) const
{
    for (auto & combination : m_combinations) {
        order.clear();
        order.resize(components.size());

        if (combination.cardinality == "fixed") {
            if (combination.mincount != components.size()) {
                continue;
            }
            std::iota(order.begin(), order.end(), 0);
            if (fixed_classify(combination, components, order)) {
                return combination.name;
            }
        }
        else if (combination.cardinality == "multiple") {
            if (components.size() % combination.mincount != 0) {
                continue;
            }
            std::vector<Component> uniq_components;
            std::vector<std::vector<std::size_t>> components_distribution;
            for (std::size_t j = 0; j < components.size(); ++j) {
                bool is_matches = false;
                for (std::size_t k = 0; k < uniq_components.size(); ++k) {
                    if (uniq_components[k] == components[j]) {
                        components_distribution[k].emplace_back(j);
                        is_matches = true;
                    }
                }
                if (!is_matches) {
                    uniq_components.push_back(components[j]);
                    components_distribution.push_back({j});
                }
            }
            if (uniq_components.size() != combination.legs.size()) {
                continue;
            }
            std::vector<int> tmp_order;
            tmp_order.resize(uniq_components.size());
            std::iota(tmp_order.begin(), tmp_order.end(), 0);
            if (fixed_classify(combination, uniq_components, tmp_order)) {
                for (std::size_t i = 0; i < components_distribution.size(); ++i) {
                    int shift = 0;
                    for (const auto comp : components_distribution[i]) {
                        order[comp] = tmp_order[i] + shift;
                        shift += static_cast<int>(combination.mincount);
                    }
                }
                return combination.name;
            }
        }
        else if (combination.cardinality == "more") {
            if (components.size() < combination.mincount) {
                continue;
            }
            bool good = true;
            std::iota(order.begin(), order.end(), 1);
            auto & leg = combination.legs[0];
            for (std::size_t i = 0; i < components.size(); ++i) {
                const Component & comp = components[i];
                if (!(comp.type == leg.type ||
                      (static_cast<char>(leg.type) == 'O' &&
                       (static_cast<char>(comp.type) != 'P' || static_cast<char>(comp.type) != 'C')))) {
                    good = false;
                    break;
                }
                if (std::holds_alternative<double>(leg.ratio) &&
                    std::get<double>(leg.ratio) != comp.ratio) {
                    good = false;
                    break;
                }
                if (std::holds_alternative<char>(leg.ratio) &&
                    ((std::get<char>(leg.ratio) == '+' && comp.ratio < 0) || (std::get<char>(leg.ratio) == '-' && comp.ratio > 0))) {
                    good = false;
                    break;
                }
            }
            if (good) {
                return combination.name;
            }
        }
    }
    order.clear();
    return "Unclassified";
}
