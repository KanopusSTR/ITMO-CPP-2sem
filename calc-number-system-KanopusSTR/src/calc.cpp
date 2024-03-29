#include "calc.h"

#include <cctype>   // for std::isspace
#include <cmath>    // various math functions
#include <iostream> // for error reporting via std::cerr

namespace {

const std::size_t max_decimal_digits = 10;

enum class Op
{
    ERR,
    SET,
    ADD,
    SUB,
    MUL,
    DIV,
    REM,
    NEG,
    POW,
    SQRT,
};

std::size_t arity(const Op op)
{
    switch (op) {
    // error
    case Op::ERR:
        return 0;
        // unary
    case Op::NEG:
    case Op::SQRT:
        return 1;
        // binary
    case Op::SET:
    case Op::ADD:
    case Op::SUB:
    case Op::MUL:
    case Op::DIV:
    case Op::REM:
    case Op::POW: return 2;
    }
    return 0;
}

Op parse_op(const std::string & line, std::size_t & i)
{
    const auto rollback = [&i, &line](const std::size_t n) {
        i -= n;
        std::cerr << "Unknown operation " << line << std::endl;
        return Op::ERR;
    };
    switch (line[i++]) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        --i; // a first digit is a part of op's argument
        return Op::SET;
    case '+':
        return Op::ADD;
    case '-':
        return Op::SUB;
    case '*':
        return Op::MUL;
    case '/':
        return Op::DIV;
    case '%':
        return Op::REM;
    case '_':
        return Op::NEG;
    case '^':
        return Op::POW;
    case 'S':
        switch (line[i++]) {
        case 'Q':
            switch (line[i++]) {
            case 'R':
                switch (line[i++]) {
                case 'T':
                    return Op::SQRT;
                default:
                    return rollback(4);
                }
            default:
                return rollback(3);
            }
        default:
            return rollback(2);
        }
    default:
        return rollback(1);
    }
}

int char_to_number(char letter)
{
    switch (std::tolower(static_cast<unsigned char>(letter))) {
    case ('0'):
        return 0;
    case ('1'):
        return 1;
    case ('2'):
        return 2;
    case ('3'):
        return 3;
    case ('4'):
        return 4;
    case ('5'):
        return 5;
    case ('6'):
        return 6;
    case ('7'):
        return 7;
    case ('8'):
        return 8;
    case ('9'):
        return 9;
    case ('a'):
        return 10;
    case ('b'):
        return 11;
    case ('c'):
        return 12;
    case ('d'):
        return 13;
    case ('e'):
        return 14;
    case ('f'):
        return 15;
    default:
        return 999;
    }
}

std::size_t skip_ws(const std::string & line, std::size_t i)
{
    while (i < line.size() && std::isspace(line[i])) {
        ++i;
    }
    return i;
}

double parse_arg(const std::string & line, std::size_t & i)
{
    double res = 0;
    std::size_t count = 0;
    bool good = true;
    bool integer = true;
    double fraction = 1;
    int num_system = 10;
    switch (line[i]) {
    case '0':
        switch (std::tolower(line[++i])) {
        case 'x':
            num_system = 16;
            ++i;
            break;
        case 'b':
            ++i;
            num_system = 2;
            break;
        case '.':
            i--;
            break;
        default:
            if (i == line.size()) {
                return 0;
            }
            num_system = 8;
        }
    }
    if (i == line.size() && num_system != 10) {
        i--;
    }
    if (line[i] == '.') {
        good = false;
    }
    else {
        while (good && i < line.size() && count < max_decimal_digits) {
            switch (line[i]) {
            case '.':
                if (!integer) {
                    good = false;
                    break;
                }
                integer = false;
                ++i;
                break;
            default:
                if (char_to_number(line[i]) < num_system) {
                    if (integer) {
                        double add = char_to_number(line[i]);
                        res *= num_system;
                        res += add;
                    }
                    else {
                        fraction /= num_system;
                        res += char_to_number(line[i]) * fraction;
                    }
                    ++i;
                    ++count;
                }
                else {
                    good = false;
                    break;
                }
            }
        }
    }
    if (!good) {
        std::cerr << "Argument parsing error at " << i << ": '" << line.substr(i) << "'" << std::endl;
    }
    else if (i < line.size()) {
        std::cerr << "Argument isn't fully parsed, suffix left: '" << line.substr(i) << "'" << std::endl;
    }
    return res;
}

double unary(const double current, const Op op)
{
    switch (op) {
    case Op::NEG:
        return -current;
    case Op::SQRT:
        if (current > 0) {
            return std::sqrt(current);
        }
        else {
            std::cerr << "Bad argument for SQRT: " << current << std::endl;
            [[fallthrough]];
        }
    default:
        return current;
    }
}

double binary(const Op op, const double left, const double right)
{
    switch (op) {
    case Op::SET:
        return right;
    case Op::ADD:
        return left + right;
    case Op::SUB:
        return left - right;
    case Op::MUL:
        return left * right;
    case Op::DIV:
        if (right != 0) {
            return left / right;
        }
        else {
            std::cerr << "Bad right argument for division: " << right << std::endl;
            return left;
        }
    case Op::REM:
        if (right != 0) {
            return std::fmod(left, right);
        }
        else {
            std::cerr << "Bad right argument for remainder: " << right << std::endl;
            return left;
        }
    case Op::POW:
        return std::pow(left, right);
    default:
        return left;
    }
}

} // anonymous namespace

double process_line(const double current, const std::string & line)
{
    std::size_t i = 0;
    const auto op = parse_op(line, i);
    switch (arity(op)) {
    case 2: {
        i = skip_ws(line, i);
        const auto old_i = i;
        const auto arg = parse_arg(line, i);
        if (i == old_i) {
            std::cerr << "No argument for a binary operation" << std::endl;
            break;
        }
        else if (i < line.size()) {
            break;
        }
        return binary(op, current, arg);
    }
    case 1: {
        if (i < line.size()) {
            std::cerr << "Unexpected suffix for a unary operation: '" << line.substr(i) << "'" << std::endl;
            break;
        }
        return unary(current, op);
    }
    default: break;
    }
    return current;
}