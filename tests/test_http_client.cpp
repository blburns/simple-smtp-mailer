#include <gtest/gtest.h>
#include <string>
#include "simple-smtp-mailer/http_client.hpp"

class HTTPClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test request
        test_request.method = ssmtp_mailer::HTTPMethod::GET;
        test_request.url = "https://httpbin.org/get";
        test_request.timeout_seconds = 30;
        test_request.verify_ssl = true;
    }

    void TearDown() override {
        // Clean up after each test
    }

    ssmtp_mailer::HTTPRequest test_request;
};

// Test HTTP client creation
TEST_F(HTTPClientTest, ClientCreation) {
    EXPECT_NO_THROW({
        auto client = ssmtp_mailer::HTTPClientFactory::createClient();
        EXPECT_NE(client, nullptr);
    });
}

// Test available backends
TEST_F(HTTPClientTest, AvailableBackends) {
    auto backends = ssmtp_mailer::HTTPClientFactory::getAvailableBackends();
    EXPECT_FALSE(backends.empty());

    // Should have curl backend
    bool has_curl = false;
    for (const auto& backend : backends) {
        if (backend == "curl") {
            has_curl = true;
            break;
        }
    }
    EXPECT_TRUE(has_curl);
}

// Test HTTP request structure
TEST_F(HTTPClientTest, RequestStructure) {
    EXPECT_EQ(test_request.method, ssmtp_mailer::HTTPMethod::GET);
    EXPECT_FALSE(test_request.url.empty());
    EXPECT_EQ(test_request.timeout_seconds, 30);
    EXPECT_TRUE(test_request.verify_ssl);
}

// Test different HTTP methods
TEST_F(HTTPClientTest, HTTPMethods) {
    ssmtp_mailer::HTTPRequest get_request;
    get_request.method = ssmtp_mailer::HTTPMethod::GET;

    ssmtp_mailer::HTTPRequest post_request;
    post_request.method = ssmtp_mailer::HTTPMethod::POST;

    ssmtp_mailer::HTTPRequest put_request;
    put_request.method = ssmtp_mailer::HTTPMethod::PUT;

    ssmtp_mailer::HTTPRequest delete_request;
    delete_request.method = ssmtp_mailer::HTTPMethod::DELETE;

    EXPECT_EQ(get_request.method, ssmtp_mailer::HTTPMethod::GET);
    EXPECT_EQ(post_request.method, ssmtp_mailer::HTTPMethod::POST);
    EXPECT_EQ(put_request.method, ssmtp_mailer::HTTPMethod::PUT);
    EXPECT_EQ(delete_request.method, ssmtp_mailer::HTTPMethod::DELETE);
}

// Test request headers
TEST_F(HTTPClientTest, RequestHeaders) {
    test_request.headers["User-Agent"] = "test-client/1.0";
    test_request.headers["Content-Type"] = "application/json";

    EXPECT_EQ(test_request.headers["User-Agent"], "test-client/1.0");
    EXPECT_EQ(test_request.headers["Content-Type"], "application/json");
    EXPECT_EQ(test_request.headers.size(), 2);
}

// Test request body
TEST_F(HTTPClientTest, RequestBody) {
    test_request.method = ssmtp_mailer::HTTPMethod::POST;
    test_request.body = "{\"test\": \"data\"}";

    EXPECT_EQ(test_request.method, ssmtp_mailer::HTTPMethod::POST);
    EXPECT_EQ(test_request.body, "{\"test\": \"data\"}");
}

// Test SSL verification
TEST_F(HTTPClientTest, SSLVerification) {
    test_request.verify_ssl = true;
    EXPECT_TRUE(test_request.verify_ssl);

    test_request.verify_ssl = false;
    EXPECT_FALSE(test_request.verify_ssl);
}

// Test timeout settings
TEST_F(HTTPClientTest, TimeoutSettings) {
    test_request.timeout_seconds = 60;
    EXPECT_EQ(test_request.timeout_seconds, 60);

    test_request.timeout_seconds = 0;
    EXPECT_EQ(test_request.timeout_seconds, 0);
}
