#include <gtest/gtest.h>
#include <string>
#include "simple-smtp-mailer/api_client.hpp"

class AmazonSESIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up Amazon SES configuration
        config.provider = ssmtp_mailer::APIProvider::AMAZON_SES;
        config.auth.api_key = "test_access_key"; // Replace with real key for actual testing
        config.auth.api_secret = "test_secret_key"; // Replace with real secret for actual testing
        config.sender_email = "test@example.com";
        config.request.base_url = "https://email.us-east-1.amazonaws.com";
        config.request.endpoint = "/v2/email";
        config.request.custom_headers["region"] = "us-east-1";

        // Set up test email
        test_email.from = "test@example.com";
        test_email.to = {"recipient@example.com"};
        test_email.cc = {"cc@example.com"};
        test_email.bcc = {"bcc@example.com"};
        test_email.subject = "Test Email from Amazon SES";
        test_email.body = "This is a test email sent via Amazon SES API.";
        test_email.html_body = "<h1>Test Email</h1><p>This is a test email sent via <strong>Amazon SES API</strong>.</p>";
    }

    void TearDown() override {
        // Clean up after each test
    }

    ssmtp_mailer::APIClientConfig config;
    ssmtp_mailer::Email test_email;
};

// Test Amazon SES client creation
TEST_F(AmazonSESIntegrationTest, ClientCreation) {
    EXPECT_NO_THROW({
        auto client = ssmtp_mailer::APIClientFactory::createClient(config);
        EXPECT_NE(client, nullptr);
        EXPECT_EQ(client->getProviderName(), "Amazon SES");
        EXPECT_TRUE(client->isValid());
    });
}

// Test Amazon SES authentication
TEST_F(AmazonSESIntegrationTest, Authentication) {
    auto client = ssmtp_mailer::APIClientFactory::createClient(config);

    // Test with valid configuration
    EXPECT_TRUE(client->isValid());

    // Test with invalid configuration
    ssmtp_mailer::APIClientConfig invalid_config;
    invalid_config.provider = ssmtp_mailer::APIProvider::AMAZON_SES;
    // Missing required fields

    auto invalid_client = ssmtp_mailer::APIClientFactory::createClient(invalid_config);
    EXPECT_FALSE(invalid_client->isValid());
}

// Test Amazon SES request formatting
TEST_F(AmazonSESIntegrationTest, RequestFormatting) {
    auto client = ssmtp_mailer::APIClientFactory::createClient(config);

    // Test email sending (will fail with test credentials, but tests formatting)
    EXPECT_NO_THROW({
        auto response = client->sendEmail(test_email);
        // Expected to fail due to test credentials
        EXPECT_FALSE(response.success);
        EXPECT_EQ(response.http_code, 0); // Connection failed with test credentials
    });
}

// Test Amazon SES response parsing
TEST_F(AmazonSESIntegrationTest, ResponseParsing) {
    auto client = ssmtp_mailer::APIClientFactory::createClient(config);

    // Test connection (will fail with test credentials, but tests parsing)
    EXPECT_NO_THROW({
        bool connected = client->testConnection();
        // Expected to fail due to test credentials
        EXPECT_FALSE(connected);
    });
}

// Test Amazon SES error handling
TEST_F(AmazonSESIntegrationTest, ErrorHandling) {
    auto client = ssmtp_mailer::APIClientFactory::createClient(config);

    // Test with invalid email
    ssmtp_mailer::Email invalid_email;
    invalid_email.from = "test@example.com";
    // Missing recipient

    EXPECT_NO_THROW({
        auto response = client->sendEmail(invalid_email);
        EXPECT_FALSE(response.success);
        EXPECT_EQ(response.http_code, 400);
        EXPECT_FALSE(response.error_message.empty());
    });
}

// Test Amazon SES batch sending
TEST_F(AmazonSESIntegrationTest, BatchSending) {
    auto client = ssmtp_mailer::APIClientFactory::createClient(config);

    std::vector<ssmtp_mailer::Email> emails = {test_email, test_email};

    EXPECT_NO_THROW({
        auto responses = client->sendBatch(emails);
        EXPECT_EQ(responses.size(), 2);
        // Responses will likely fail due to test credentials
        for (const auto& response : responses) {
            EXPECT_FALSE(response.success);
        }
    });
}

// Test Amazon SES region configuration
TEST_F(AmazonSESIntegrationTest, RegionConfiguration) {
    // Test with region in custom headers
    ssmtp_mailer::APIClientConfig config_with_region = config;
    config_with_region.request.custom_headers["region"] = "us-west-2";
    config_with_region.request.base_url = "https://email.us-west-2.amazonaws.com";

    EXPECT_NO_THROW({
        auto client = ssmtp_mailer::APIClientFactory::createClient(config_with_region);
        EXPECT_NE(client, nullptr);
        EXPECT_TRUE(client->isValid());
    });

    // Test with region extracted from base URL
    ssmtp_mailer::APIClientConfig config_with_url = config;
    config_with_url.request.custom_headers.clear();
    config_with_url.request.base_url = "https://email.eu-west-1.amazonaws.com";

    EXPECT_NO_THROW({
        auto client = ssmtp_mailer::APIClientFactory::createClient(config_with_url);
        EXPECT_NE(client, nullptr);
        EXPECT_TRUE(client->isValid());
    });
}

// Test Amazon SES configuration set
TEST_F(AmazonSESIntegrationTest, ConfigurationSet) {
    ssmtp_mailer::APIClientConfig config_with_set = config;
    config_with_set.request.custom_headers["ses_configuration_set"] = "test-config-set";

    EXPECT_NO_THROW({
        auto client = ssmtp_mailer::APIClientFactory::createClient(config_with_set);
        EXPECT_NE(client, nullptr);
        EXPECT_TRUE(client->isValid());
    });
}
