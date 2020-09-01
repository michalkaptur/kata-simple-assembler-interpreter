// https://www.codewars.com/kata/58e24788e24ddee28e000053/train/cpp

#include <unordered_map>
#include <vector>
#include <string>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

enum class ops
{
    mov,
    inc,
    dec,
    jnz
};

using result = std::unordered_map<std::string, int>;
using input = std::vector<std::string>;

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
