#include "Component.h"

#include <iomanip>
#include <sstream>

Component Component::from_stream(std::istream & strm)
{
    Component component;

    char type = '\0';
    strm >> type;
    if (strm.fail()) {
        return {};
    }
    bool read_strike = false;
    component.type = static_cast<InstrumentType>(type);
    switch (component.type) {
    case InstrumentType::C: [[fallthrough]];
    case InstrumentType::O: [[fallthrough]];
    case InstrumentType::P:
        read_strike = true;
        break;
    case InstrumentType::F: [[fallthrough]];
    case InstrumentType::U:
        break;
    case InstrumentType::Unknown: [[fallthrough]];
    default:
        return {};
    }

    strm >> component.ratio;
    if (strm.fail()) {
        return {};
    }

    if (read_strike) {
        strm >> component.strike;
        if (strm.fail()) {
            return {};
        }
    }

    strm >> std::get_time(&component.expiration, "%Y-%m-%d");
    if (strm.fail()) {
        return {};
    }

    return component;
}

Component Component::from_string(const std::string & str)
{
    std::istringstream strm{str};
    return from_stream(strm);
}

int compare_tm(std::tm tm1, std::tm tm2);

bool operator==(const Component & comp1, const Component & comp2)
{
    return compare_tm(comp1.expiration, comp2.expiration) == 0 &&
            comp1.type == comp2.type && comp1.strike == comp2.strike && comp1.ratio == comp2.ratio;
}
