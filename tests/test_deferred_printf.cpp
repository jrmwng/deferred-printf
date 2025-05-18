#include "deferred_printf.h"
#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <cstdio>
#include <fstream>

#ifdef _MSC_VER
#pragma warning(disable : 4996) // Suppress warning: 'fopen' is deprecated
#endif

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
    jrmwng::deferred_printf<64> logger;

    logger("Entry %d", 1);
    logger("Entry %d", 2);

    std::vector<std::string> output;
    auto custom_vprintf = [&output](char const *pcFormat, va_list args) -> int {
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), pcFormat, args);
        output.push_back(buffer);
        return 0;
    };

    logger.apply(custom_vprintf);

    assert(output.size() == 2);
    assert(output[0] == "Entry 1");
    assert(output[1] == "Entry 2");
}

void test_various_format_specifiers()
{
    jrmwng::deferred_printf<> logger;

    logger("Integer: %d", 42);
    logger("Hex: %x", 255);
    logger("Octal: %o", 64);
    logger("Float: %.2f", 3.14159);
    logger("Char: %c", 'A');
    logger("String: %s", "test");

    std::vector<std::string> output;
    logger.apply([&output](char const *pcFormat, va_list args) -> int {
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), pcFormat, args);
        output.push_back(buffer);
        return 0;
    });

    assert(output.size() == 6);
    assert(output[0] == "Integer: 42");
    assert(output[1] == "Hex: ff");
    assert(output[2] == "Octal: 100");
    assert(output[3] == "Float: 3.14");
    assert(output[4] == "Char: A");
    assert(output[5] == "String: test");
}

void test_multiple_format_specifiers()
{
    jrmwng::deferred_printf<> logger;

    logger("Int: %d, Hex: %x, Oct: %o", 42, 255, 64);
    logger("Float: %.2f, Char: %c, String: %s", 3.14159, 'A', "test");

    std::vector<std::string> output;
    logger.apply([&output](char const *pcFormat, va_list args) -> int {
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), pcFormat, args);
        output.push_back(buffer);
        return 0;
    });

    assert(output.size() == 2);
    assert(output[0] == "Int: 42, Hex: ff, Oct: 100");
    assert(output[1] == "Float: 3.14, Char: A, String: test");
}

void test_multiple_data_types()
{
    jrmwng::deferred_printf<> logger;

    logger("Int: %d, Long: %ld, Short: %hd", 42, 123456789L, (short)123);
    logger("Unsigned: %u, Unsigned Long: %lu, Unsigned Short: %hu", 42U, 123456789UL, (unsigned short)123);
    logger("Float: %.2f, Double: %.4lf, Long Double: %.6Lf", 3.14159f, 2.7182818284, 1.618033988749895L);
    logger("Char: %c, String: %s", 'A', "test");

    std::vector<std::string> output;
    logger.apply([&output](char const *pcFormat, va_list args) -> int {
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), pcFormat, args);
        output.push_back(buffer);
        return 0;
    });

    assert(output.size() == 4);
    assert(output[0] == "Int: 42, Long: 123456789, Short: 123");
    assert(output[1] == "Unsigned: 42, Unsigned Long: 123456789, Unsigned Short: 123");
    assert(output[2] == "Float: 3.14, Double: 2.7183, Long Double: 1.618034");
    assert(output[3] == "Char: A, String: test");
}

void test_large_logger()
{
    jrmwng::deferred_printf<1024 * 1024> logger;

    for (int i = 0; i < 1000; ++i)
    {
        logger("Log entry %d", i);
    }

    std::vector<std::string> output;
    logger.apply([&output](char const *pcFormat, va_list args) -> int {
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), pcFormat, args);
        output.push_back(buffer);
        return 0;
    });

    assert(output.size() == 1000);
    for (int i = 0; i < 1000; ++i)
    {
        assert(output[i] == "Log entry " + std::to_string(i));
    }
}

void test_fprintf()
{
    jrmwng::deferred_printf<> logger;

    logger("File %s, Line %d\n", "test.cpp", 42);
    logger("Error: %s\n", "Something went wrong");

    std::vector<std::string> output;
    FILE *file = fopen("tempfile.txt", "w+");
    if (!file) {
        std::cerr << "Failed to open temporary file" << std::endl;
        return;
    }
    logger.apply<FILE*>(&vfprintf, file);

    fseek(file, 0, SEEK_SET);
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), file))
    {
        output.push_back(buffer);
    }
    fclose(file);
    remove("tempfile.txt");

    assert(output.size() == 2);
    assert(output[0] == std::string("File test.cpp, Line 42\n"));
    assert(output[1] == std::string("Error: Something went wrong\n"));
}

void test_dynamic_buffer_allocation()
{
    jrmwng::deferred_printf<> logger;

    logger("Dynamic buffer %d %d", 5, 6);

    // Obtain the required buffer size
    int size = logger.apply<char*, size_t>(&vsnprintf, nullptr, 0);

    // Allocate a buffer with the required size
    std::vector<char> buffer(size + 1);

    // Fill the buffer from the logger
    logger.apply<char *>(&vsprintf, buffer.data());

    assert(std::string(buffer.data()) == "Dynamic buffer 5 6");
}

int main()
{
    test_basic_logging();
    test_custom_vprintf();
    test_logger_capacity();
    test_various_format_specifiers();
    test_multiple_format_specifiers();
    test_multiple_data_types();
    test_large_logger();
    test_fprintf();
    test_dynamic_buffer_allocation();

    std::cout << "All tests passed!" << std::endl;
    return 0;
}