#include <iostream>

// Forward declarations
void testAPIClient();
void testAllProviders();

int main() {
    std::cout << "Running ssmtp-mailer tests..." << std::endl;
    std::cout << "=============================" << std::endl;
    std::cout << std::endl;

    // Run all test suites
    testAPIClient();
    testAllProviders();

    std::cout << "All test suites completed!" << std::endl;
    return 0;
}
