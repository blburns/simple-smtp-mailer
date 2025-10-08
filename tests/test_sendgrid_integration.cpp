#include <gtest/gtest.h>
#include <string>
#include "simple-smtp-mailer/api_client.hpp"

class SendGridIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up SendGrid configuration
        config.provider = ssmtp_mailer::APIProvider::SENDGRID;
        config.auth.api_key = "test_key"; // Replace with real key for actual testing
        config.sender_email = "test@example.com";
        config.request.base_url = "https://api.sendgrid.com";
        config.request.endpoint = "/v3/mail/send";

        // Set up test email
        test_email.from = "test@example.com";
        test_email.to = {"recipient@example.com"};
        test_email.subject = "Test Email from SendGrid";
        test_email.body = "This is a test email sent via SendGrid API.";
        test_email.html_body = "<h1>Test Email</h1><p>This is a test email sent via <strong>SendGrid API</strong>.</p>";
    }

    void TearDown() override {
        // Clean up after each test
    }

    ssmtp_mailer::APIClientConfig config;
    ssmtp_mailer::Email test_email;
};

// Test SendGrid client creation
TEST_F(SendGridIntegrationTest, ClientCreation) {
    EXPECT_NO_THROW({
        auto client = ssmtp_mailer::APIClientFactory::createClient(config);
        EXPECT_NE(client, nullptr);
        EXPECT_EQ(client->getProviderName(), "SendGrid");
        EXPECT_TRUE(client->isValid());
    });
}

// Test SendGrid authentication
TEST_F(SendGridIntegrationTest, Authentication) {
    auto client = ssmtp_mailer::APIClientFactory::createClient(config);

    // Test with valid configuration
    EXPECT_TRUE(client->isValid());

    // Test with invalid configuration
    ssmtp_mailer::APIClientConfig invalid_config;
    invalid_config.provider = ssmtp_mailer::APIProvider::SENDGRID;
    // Missing required fields

    auto invalid_client = ssmtp_mailer::APIClientFactory::createClient(invalid_config);
    EXPECT_FALSE(invalid_client->isValid());
}

// Test SendGrid request formatting
TEST_F(SendGridIntegrationTest, RequestFormatting) {
    auto client = ssmtp_mailer::APIClientFactory::createClient(config);

    // Test email sending (will fail with test credentials, but tests formatting)
    EXPECT_NO_THROW({
        auto response = client->sendEmail(test_email);
        // Expected to fail due to test credentials
        EXPECT_FALSE(response.success);
        EXPECT_EQ(response.http_code, 401); // Unauthorized with test key
    });
}

// Test SendGrid response parsing
TEST_F(SendGridIntegrationTest, ResponseParsing) {
    auto client = ssmtp_mailer::APIClientFactory::createClient(config);

    // Test connection (will fail with test credentials, but tests parsing)
    EXPECT_NO_THROW({
        bool connected = client->testConnection();
        // Expected to fail due to test credentials
        EXPECT_FALSE(connected);
    });
}

// Test SendGrid error handling
TEST_F(SendGridIntegrationTest, ErrorHandling) {
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

// Test SendGrid batch sending
TEST_F(SendGridIntegrationTest, BatchSending) {
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

// Test SendGrid with different email formats
TEST_F(SendGridIntegrationTest, EmailFormats) {
    auto client = ssmtp_mailer::APIClientFactory::createClient(config);

    // Test with text only
    ssmtp_mailer::Email text_email = test_email;
    text_email.html_body = "";

    EXPECT_NO_THROW({
        auto response = client->sendEmail(text_email);
        EXPECT_FALSE(response.success); // Expected to fail with test credentials
    });

    // Test with HTML only
    ssmtp_mailer::Email html_email = test_email;
    html_email.body = "";

    EXPECT_NO_THROW({
        auto response = client->sendEmail(html_email);
        EXPECT_FALSE(response.success); // Expected to fail with test credentials
    });

    // Test with both text and HTML
    EXPECT_NO_THROW({
        auto response = client->sendEmail(test_email);
        EXPECT_FALSE(response.success); // Expected to fail with test credentials
    });
}

// Test SendGrid with multiple recipients
TEST_F(SendGridIntegrationTest, MultipleRecipients) {
    auto client = ssmtp_mailer::APIClientFactory::createClient(config);

    ssmtp_mailer::Email multi_email = test_email;
    multi_email.to = {"recipient1@example.com", "recipient2@example.com", "recipient3@example.com"};
    multi_email.cc = {"cc1@example.com", "cc2@example.com"};
    multi_email.bcc = {"bcc1@example.com", "bcc2@example.com"};

    EXPECT_NO_THROW({
        auto response = client->sendEmail(multi_email);
        EXPECT_FALSE(response.success); // Expected to fail with test credentials
    });
}
