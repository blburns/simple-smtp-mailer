# 🧪 Testing Framework Documentation

**Version:** 0.2.0+
**Last Updated:** December 2024
**Status:** Active

## 📋 Overview

The simple-smtp-mailer project uses Google Test (gtest) as its primary testing framework, providing comprehensive unit and integration testing across all components.

## 🏗️ Test Structure

### **Test Directory Organization**
```
tests/
├── CMakeLists.txt                    # Test build configuration
├── test_main.cpp                     # Main test runner
├── test_basic.cpp                    # Basic functionality tests
├── test_api_client.cpp               # API client tests
├── test_all_providers.cpp            # Provider integration tests
├── test_http_client.cpp              # HTTP client tests
├── test_mailgun_integration.cpp      # Mailgun-specific tests
├── test_amazon_ses_integration.cpp   # Amazon SES-specific tests
└── test_sendgrid_integration.cpp     # SendGrid-specific tests
```

### **Test Suites Overview**

| Test Suite | Tests | Description |
|------------|-------|-------------|
| **BasicFunctionalityTest** | 12 | Core data structures, email validation, priority levels |
| **APIClientTest** | 7 | API client factory, provider support, configuration |
| **ProviderIntegrationTest** | 8 | Multi-provider testing, batch sending, error handling |
| **HTTPClientTest** | 8 | HTTP methods, headers, SSL, timeouts, request structure |
| **MailgunIntegrationTest** | 7 | Mailgun API client, authentication, error handling |
| **AmazonSESIntegrationTest** | 8 | Amazon SES API client, region config, batch sending |
| **SendGridIntegrationTest** | 8 | SendGrid API client, email formats, multiple recipients |

**Total: 58 tests across 7 test suites**

## 🚀 Running Tests

### **Build Tests**
```bash
# Clean build
make clean

# Build with tests
make -j8

# Or build tests only
cd build && make simple-smtp-mailer-tests
```

### **Execute Tests**
```bash
# Run all tests
cd build && ./tests/simple-smtp-mailer-tests

# Run with verbose output
cd build && ./tests/simple-smtp-mailer-tests --gtest_verbose

# Run specific test suite
cd build && ./tests/simple-smtp-mailer-tests --gtest_filter="BasicFunctionalityTest.*"

# Run specific test
cd build && ./tests/simple-smtp-mailer-tests --gtest_filter="BasicFunctionalityTest.EmailStructure"
```

### **Test Output Example**
```
🚀 Running simple-smtp-mailer Test Suite
========================================

[==========] Running 58 tests from 7 test suites.
[----------] Global test environment set-up.
[----------] 12 tests from BasicFunctionalityTest
[ RUN      ] BasicFunctionalityTest.EmailStructure
[       OK ] BasicFunctionalityTest.EmailStructure (0 ms)
...
[----------] 12 tests from BasicFunctionalityTest (41 ms total)

[==========] 58 tests from 7 test suites ran. (8162 ms total)
[  PASSED  ] 53 tests.
[  FAILED  ] 5 tests, listed below:
[  FAILED  ] APIClientTest.EmailValidation
[  FAILED  ] ProviderIntegrationTest.ErrorHandling
[  FAILED  ] AmazonSESIntegrationTest.RequestFormatting
[  FAILED  ] AmazonSESIntegrationTest.BatchSending
[  FAILED  ] SendGridIntegrationTest.ErrorHandling

 5 FAILED TESTS
```

## 📊 Test Coverage

### **Current Coverage Areas**

#### ✅ **Fully Tested Components**
- **Email Data Structures**: Email, QueueItem, Priority, Status enums
- **HTTP Client**: Request/response handling, methods, headers, SSL
- **API Client Factory**: Provider enumeration, client creation
- **Basic Functionality**: Core data validation, structure integrity

#### 🔄 **Partially Tested Components**
- **API Integration**: Tests exist but use placeholder credentials
- **Error Handling**: Basic error scenarios covered
- **Authentication**: Framework in place, needs real credential testing

#### ❌ **Not Yet Tested**
- **Performance Testing**: Load testing, concurrent operations
- **Cross-Platform Testing**: Platform-specific functionality
- **Security Testing**: Input validation, authentication security
- **End-to-End Testing**: Complete email sending workflows

### **Test Categories**

#### **Unit Tests**
- Individual component testing
- Mock object usage
- Isolated functionality verification
- Data structure validation

#### **Integration Tests**
- Multi-component interaction testing
- API provider integration
- Configuration system testing
- Error propagation testing

#### **API Provider Tests**
- Provider-specific functionality
- Authentication mechanisms
- Request/response handling
- Error parsing and handling

## 🔧 Test Development Guidelines

### **Writing New Tests**

#### **Test Structure**
```cpp
#include "gtest/gtest.h"
#include "simple-smtp-mailer/component.hpp"

namespace ssmtp_mailer {

class ComponentTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Test setup
    }

    void TearDown() override {
        // Test cleanup
    }
};

TEST_F(ComponentTest, TestName) {
    // Test implementation
    EXPECT_EQ(expected, actual);
    ASSERT_TRUE(condition);
}

} // namespace ssmtp_mailer
```

#### **Best Practices**
1. **Descriptive Test Names**: Use clear, descriptive test names
2. **Single Responsibility**: Each test should verify one specific behavior
3. **Arrange-Act-Assert**: Structure tests with clear setup, execution, and verification
4. **Mock External Dependencies**: Use mocks for external services
5. **Test Edge Cases**: Include boundary conditions and error scenarios
6. **Clean Up Resources**: Ensure proper cleanup in TearDown()

### **Test Data Management**

#### **Test Credentials**
- Use placeholder credentials for API tests
- Document required environment variables for real testing
- Implement credential validation in test setup

#### **Mock Data**
- Create realistic test data
- Use consistent test email addresses
- Include various email formats and edge cases

## 🐛 Debugging Tests

### **Common Issues**

#### **Test Failures with Placeholder Credentials**
```
Expected: false
Actual: true
```
**Solution**: These are expected when using test credentials. Real credentials are needed for full validation.

#### **Compilation Errors**
```
fatal error: 'gtest/gtest.h' file not found
```
**Solution**: Ensure Google Test is installed and CMake can find it.

#### **Linking Errors**
```
ld: library not found for -lgtest
```
**Solution**: Check that Google Test libraries are properly linked in CMakeLists.txt.

### **Debug Commands**
```bash
# Verbose test output
./tests/simple-smtp-mailer-tests --gtest_verbose

# Run tests with debug symbols
gdb ./tests/simple-smtp-mailer-tests

# Check test dependencies
ldd ./tests/simple-smtp-mailer-tests
```

## 📈 Performance Testing

### **Current Performance Metrics**
- **Total Test Runtime**: ~8 seconds
- **Test Compilation**: ~2-3 seconds (with -j8)
- **Memory Usage**: Minimal (tests run in sequence)

### **Future Performance Testing**
- [ ] Concurrent test execution
- [ ] Load testing for API clients
- [ ] Memory usage profiling
- [ ] Performance regression testing

## 🔄 Continuous Integration

### **Current Status**
- Tests run locally during development
- Manual test execution required
- No automated CI/CD pipeline yet

### **Planned CI/CD Integration**
- [ ] GitHub Actions workflow
- [ ] Automated test execution on commits
- [ ] Cross-platform testing
- [ ] Test result reporting
- [ ] Performance regression detection

## 📚 Test Documentation

### **Test Documentation Standards**
- Each test file should have a header comment explaining its purpose
- Complex test logic should be documented
- Test data sources should be documented
- Expected behavior should be clearly stated

### **Test Maintenance**
- Regular review of test coverage
- Update tests when APIs change
- Remove obsolete tests
- Add tests for new features

## 🎯 Future Enhancements

### **Planned Improvements**
- [ ] Test coverage reporting
- [ ] Performance benchmarking
- [ ] Automated test data generation
- [ ] Parallel test execution
- [ ] Test result visualization
- [ ] Integration with code quality tools

### **Advanced Testing Features**
- [ ] Property-based testing
- [ ] Fuzz testing for input validation
- [ ] Chaos engineering tests
- [ ] Security penetration testing
- [ ] Load testing with realistic data

---

## 📞 Support

### **Getting Help**
- **Documentation**: Check this file and inline code comments
- **Issues**: Report test-related issues on GitHub
- **Discussions**: Use GitHub Discussions for test strategy questions

### **Contributing Tests**
- Follow existing test patterns
- Add tests for new features
- Update tests when fixing bugs
- Ensure tests pass before submitting PRs

---

**Last Updated**: December 2024
**Next Review**: January 2025
**Maintainer**: blburns

*This testing framework documentation is updated as the testing infrastructure evolves.*
