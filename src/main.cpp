// https://www.codewars.com/kata/58e24788e24ddee28e000053/train/cpp

#include <algorithm>
#include <cctype>
#include <iterator>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using reg_t = char;
using constant = int;

struct mov {
    reg_t reg;
    std::variant<reg_t, constant> value;
};

struct inc {
    reg_t reg;
};

struct dec {
    reg_t reg;
};

struct jnz {
    std::variant<reg_t, constant> x; // non zero?
    std::variant<reg_t, constant> y;
};
bool operator==(const jnz& lhs, const jnz& rhs)
{
    return lhs.x == rhs.x and lhs.y == rhs.y;
}

bool operator==(const dec& lhs, const dec& rhs) { return lhs.reg == rhs.reg; }
bool operator==(const inc& lhs, const inc& rhs) { return lhs.reg == rhs.reg; }
bool operator==(const mov& lhs, const mov& rhs)
{
    return lhs.reg == rhs.reg and lhs.value == rhs.value;
}

using result = std::unordered_map<std::string, int>;
using input = std::vector<std::string>;
using operation = std::variant<mov, inc, dec, jnz>;
using ops = std::vector<operation>;
using flow_control_t = constant;

std::variant<reg_t, constant> parse_arg(const std::string& arg)
{
    if (std::isalpha(arg.at(0))) {
        return reg_t { arg.at(0) };
    }
    return constant { std::stoi(arg) };
}

operation parse(const std::string& line)
{
    constexpr static auto separator = ' '; //assumed whitespace sanitized input
    const auto& op_name = line.substr(0, 3);
    if (op_name == "inc") {
        return inc { line.at(4) };
    }
    if (op_name == "dec") {
        return dec { line.at(4) };
    }
    if (op_name == "mov") {
        const reg_t register_to_set = line.at(4);
        const auto second_arg_pos = line.rfind(separator) + 1;
        const std::string& second_arg = line.substr(second_arg_pos, line.length() - second_arg_pos);
        return mov { register_to_set, parse_arg(second_arg) };
    }
    if (op_name == "jnz") {
        const auto first_arg_pos = line.find(separator) + 1;
        const auto second_arg_pos = line.rfind(separator) + 1;
        const std::string& first_arg = line.substr(first_arg_pos, second_arg_pos - first_arg_pos); //string_view?
        const std::string& second_arg = line.substr(second_arg_pos, line.length() - second_arg_pos);
        return jnz { parse_arg(first_arg), parse_arg(second_arg) };
    }
    std::terminate(); // not implemented
}

ops parse(const input& program)
{
    ops result {};
    for (const auto& line : program) {
        result.push_back(parse(line));
    }
    return result;
}

using memory_t = std::unordered_map<char, int>;

struct value_accessor {
    constant operator()(constant c) const { return c; }
    constant operator()(reg_t r) const { return mem.at(r); }
    const memory_t& mem;
};

struct operation_visitor {
    constexpr static flow_control_t default_flow_control { 1 };
    flow_control_t operator()(const inc& op)
    {
        mem[op.reg]++;
        return default_flow_control;
    }
    flow_control_t operator()(const dec& op)
    {
        mem[op.reg]--;
        return default_flow_control;
    }
    flow_control_t operator()(const mov& op)
    {
        mem[op.reg] = std::visit(value_accessor { mem }, op.value);
        return default_flow_control;
    }
    flow_control_t operator()(const jnz& op)
    {
        const value_accessor accessor { mem };
        if (std::visit(accessor, op.x) == 0) {
            return default_flow_control;
        }
        return std::visit(accessor, op.y);
    }
    memory_t& mem;
};

flow_control_t process_operation(const operation& op, memory_t& memory)
{
    return std::visit(operation_visitor { memory }, op);
}

result memory_to_result(const memory_t& mem)
{
    result out {};
    std::transform(
        std::cbegin(mem), std::cend(mem), std::inserter(out, out.end()),
        [](const auto& el) {
            return result::value_type { std::string(1, el.first), el.second };
        });
    return out;
}

result assembler(const input& in)
{
    const auto operations { parse(in) };
    memory_t memory;
    unsigned i { 0 };
    while (i < operations.size()) {
        const auto flow_control = process_operation(operations.at(i), memory);
        i = i + flow_control;
    }
    return memory_to_result(memory); // don't like the register name stored as string:/
}

TEST_CASE("kata_test", "[tag_foo]")
{
    input program { "mov a 5", "inc a", "dec a", "dec a", "jnz a -1", "inc a" };
    result out { { "a", 1 } };
    REQUIRE(assembler(program) == out);
    REQUIRE(assembler(program)["a"] == 1);
}

TEST_CASE("parser_test", "")
{
    REQUIRE(parse(input { "inc a" }) == ops { inc { 'a' } });
    REQUIRE(parse(input { "dec r" }) == ops { dec { 'r' } });
    REQUIRE(parse(input { "mov r 3" }) == ops { mov { 'r', 3 } });
    REQUIRE(parse(input { "mov r -3" }) == ops { mov { 'r', -3 } });
    REQUIRE(parse(input { "mov r -367" }) == ops { mov { 'r', -367 } });
    REQUIRE(parse(input { "mov r g" }) == ops { mov { 'r', 'g' } });
    REQUIRE(parse(input { "jnz 0 0" }) == ops { jnz { 0, 0 } });
    REQUIRE(parse(input { "jnz r 0" }) == ops { jnz { 'r', 0 } });
    REQUIRE(parse(input { "jnz r g" }) == ops { jnz { 'r', 'g' } });
    REQUIRE(parse(input { "jnz -145 z" }) == ops { jnz { -145, 'z' } });
}

TEST_CASE("simple_inc_and_dec_test", "")
{
    input program { "mov a 5", "inc a", "dec a", "dec a" };
    result out { { "a", 4 } };
    REQUIRE(assembler(program) == out);
}

TEST_CASE("negative_constant_in_mov", "")
{
    input program { "mov x -5" };
    result out { { "x", -5 } };
    REQUIRE(assembler(program) == out);
}
TEST_CASE("multi_digit_constant_in_mov", "")
{
    input program { "mov r 4567" };
    result out { { "r", 4567 } };
    REQUIRE(assembler(program) == out);
}

TEST_CASE("mov_set_by_another_register_value", "")
{
    input program { "mov q 10", "mov w q", "inc q" };
    result out { { "q", 11 }, { "w", 10 } };
    REQUIRE(assembler(program) == out);
}

TEST_CASE("jnz_no_operation", "")
{
    const result out {};
    const input program1 { "jnz 0 0" };
    REQUIRE(assembler(program1) == out);
    const input program2 { "jnz 10 1" };
    REQUIRE(assembler(program2) == out);
    const input program3 { "jnz -10 1" };
    REQUIRE(assembler(program3) == out);
    // const input program4 { "jnz 9 0" }; //infinite loop
}

TEST_CASE("jnz_move_forward", "")
{
    const input program { "jnz 6 2", "mov a 1", "mov b 10" };
    const result out { { "b", 10 } };
    REQUIRE(assembler(program) == out);
}