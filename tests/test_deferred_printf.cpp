#include "deferred_printf.h"
#include <iostream>
#include <vector>
#include <string>
#include <cassert>

void test_basic_logging()
{
    jrmwng::deferred_printf<> logger;

    logger("Hello %d %d", 1, 2);
    logger("Test %s", "string");

    std::vector<std::string> output;
    logger.apply([&output](char const *pcFormat, va_list args) -> int {
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), pcFormat, args);
        output.push_back(buffer);
        return 0;
    });

    assert(output.size() == 2);
    assert(output[0] == "Hello 1 2");
    assert(output[1] == "Test string");
}

void test_custom_vprintf()
{
    jrmwng::deferred_printf<> logger;

    logger("Custom %d %d", 3, 4);

    std::vector<std::string> output;
    auto custom_vprintf = [&output](char const *pcFormat, va_list args) -> int {
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), pcFormat, args);
        output.push_back(buffer);
        return 0;
    };

    logger.apply(custom_vprintf);

    assert(output.size() == 1);
    assert(output[0] == "Custom 3 4");
}

void test_logger_capacity()
{
    jrmwng::deferred_printf_logger<64> logger;

    logger.log("Entry %d", 1);
    logger.log("Entry %d", 2);

    std::vector<std::string> output;
    auto custom_vprintf = [&output](char const *pcFormat, va_list args) -> int {
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), pcFormat, args);
        output.push_back(buffer);
        return 0;
    };

    for (auto &log : logger)
    {
        log.apply(custom_vprintf);
    }

    assert(output.size() == 2);
    assert(output[0] == "Entry 1");
    assert(output[1] == "Entry 2");
}

int main()
{
    test_basic_logging();
    test_custom_vprintf();
    test_logger_capacity();

    std::cout << "All tests passed!" << std::endl;
    return 0;
}