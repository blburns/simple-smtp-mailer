#include <gtest/gtest.h>
#include <string>
#include "simple-smtp-mailer/api_client.hpp"

class MailgunIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up Mailgun configuration
        config.provider = ssmtp_mailer::APIProvider::MAILGUN;
        config.auth.api_key = "test_key"; // Replace with real key for actual testing
        config.sender_email = "test@example.com";
        config.request.base_url = "https://api.mailgun.net/v3";
        config.request.endpoint = "/messages";
        config.request.custom_headers["domain"] = "example.com";

        // Set up test email
        test_email.from = "test@example.com";
        test_email.to = {"recipient@example.com"};
        test_email.subject = "Test Email from Mailgun";
        test_email.body = "This is a test email sent via Mailgun API.";
        test_email.html_body = "<h1>Test Email</h1><p>This is a test email sent via <strong>Mailgun API</strong>.</p>";
    }

    void TearDown() override {
        // Clean up after each test
    }

    ssmtp_mailer::APIClientConfig config;
    ssmtp_mailer::Email test_email;
};

// Test Mailgun client creation
TEST_F(MailgunIntegrationTest, ClientCreation) {
    EXPECT_NO_THROW({
        auto client = ssmtp_mailer::APIClientFactory::createClient(config);
        EXPECT_NE(client, nullptr);
        EXPECT_EQ(client->getProviderName(), "Mailgun");
        EXPECT_TRUE(client->isValid());
    });
}

// Test Mailgun authentication
TEST_F(MailgunIntegrationTest, Authentication) {
    auto client = ssmtp_mailer::APIClientFactory::createClient(config);

    // Test with valid configuration
    EXPECT_TRUE(client->isValid());

    // Test with invalid configuration
    ssmtp_mailer::APIClientConfig invalid_config;
    invalid_config.provider = ssmtp_mailer::APIProvider::MAILGUN;
    // Missing required fields

    auto invalid_client = ssmtp_mailer::APIClientFactory::createClient(invalid_config);
    EXPECT_FALSE(invalid_client->isValid());
}

// Test Mailgun request formatting
TEST_F(MailgunIntegrationTest, RequestFormatting) {
    auto client = ssmtp_mailer::APIClientFactory::createClient(config);

    // Test email sending (will fail with test credentials, but tests formatting)
    EXPECT_NO_THROW({
        auto response = client->sendEmail(test_email);
        // Expected to fail due to test credentials
        EXPECT_FALSE(response.success);
        EXPECT_EQ(response.http_code, 401); // Unauthorized with test key
    });
}

// Test Mailgun response parsing
TEST_F(MailgunIntegrationTest, ResponseParsing) {
    auto client = ssmtp_mailer::APIClientFactory::createClient(config);

    // Test connection (will fail with test credentials, but tests parsing)
    EXPECT_NO_THROW({
        bool connected = client->testConnection();
        // Expected to fail due to test credentials
        EXPECT_FALSE(connected);
    });
}

// Test Mailgun error handling
TEST_F(MailgunIntegrationTest, ErrorHandling) {
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

// Test Mailgun batch sending
TEST_F(MailgunIntegrationTest, BatchSending) {
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

// Test Mailgun domain configuration
TEST_F(MailgunIntegrationTest, DomainConfiguration) {
    // Test with domain in custom headers
    ssmtp_mailer::APIClientConfig config_with_domain = config;
    config_with_domain.request.custom_headers["domain"] = "test-domain.com";

    EXPECT_NO_THROW({
        auto client = ssmtp_mailer::APIClientFactory::createClient(config_with_domain);
        EXPECT_NE(client, nullptr);
        EXPECT_TRUE(client->isValid());
    });

    // Test with domain extracted from sender email
    ssmtp_mailer::APIClientConfig config_with_sender = config;
    config_with_sender.request.custom_headers.clear();
    config_with_sender.sender_email = "test@example-domain.com";

    EXPECT_NO_THROW({
        auto client = ssmtp_mailer::APIClientFactory::createClient(config_with_sender);
        EXPECT_NE(client, nullptr);
        EXPECT_TRUE(client->isValid());
    });
}
