// https://www.codewars.com/kata/58e24788e24ddee28e000053/train/cpp

#include <unordered_map>
#include <vector>
#include <string>
#include <variant>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using reg_t = char;
using constant = int;

struct mov
{
    reg_t reg;
    std::variant<register_t, constant> value;
};

struct inc
{
    reg_t reg;
};

struct dec
{
    reg_t reg;
};

struct jnz
{
    std::variant<register_t, constant> x; //non zero?
    std::variant<register_t, constant> y;
};
bool operator==(const jnz &lhs, const jnz &rhs)
{
    return lhs.x == rhs.x and lhs.y == rhs.y;
}

bool operator==(const dec &lhs, const dec &rhs)
{
    return lhs.reg == rhs.reg;
}
bool operator==(const inc &lhs, const inc &rhs)
{
    return lhs.reg == rhs.reg;
}
bool operator==(const mov &lhs, const mov &rhs)
{
    return lhs.reg == rhs.reg and lhs.value == rhs.value;
}

using result = std::unordered_map<std::string, int>;
using input = std::vector<std::string>;
using operation = std::variant<mov, inc, dec, jnz>;
using ops = std::vector<operation>;

operation parse(const std::string &line)
{
    if (line.substr(0, 3) == "inc")
    {
        return inc{line.at(4)};
    }
    return {};
}

ops parse(const input &program)
{
    ops result{};
    for (const auto &line : program)
    {
        result.push_back(parse(line));
    }
    return result;
}

result assembler(const input &)
{
    return {{"a", 0}};
}

TEST_CASE("kata_test", "[tag_foo]")
{
    input program{"mov a 5", "inc a", "dec a", "dec a",
                  "jnz a -1", "inc a"};
    result out{{"a", 1}};
    // REQUIRE(assembler(program) == out);
    REQUIRE(assembler(program)["a"] == 1);
}

TEST_CASE("parser_test", "")
{
    REQUIRE(parse(input{"inc a"}) == ops{inc{'a'}});
}
