#include <gtest/gtest.h>
#include <iostream>

int main(int argc, char **argv) {
    std::cout << "🚀 Running simple-smtp-mailer Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // Initialize Google Test
    ::testing::InitGoogleTest(&argc, argv);

    // Run all tests
    int result = RUN_ALL_TESTS();

    std::cout << std::endl;
    if (result == 0) {
        std::cout << "✅ All tests passed!" << std::endl;
    } else {
        std::cout << "❌ Some tests failed!" << std::endl;
    }

    return result;
}
