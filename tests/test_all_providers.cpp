#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "simple-smtp-mailer/api_client.hpp"
#include "simple-smtp-mailer/http_client.hpp"

class ProviderIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test email
        test_email.from = "sender@example.com";
        test_email.to = {"recipient1@example.com", "recipient2@example.com"};
        test_email.cc = {"cc@example.com"};
        test_email.bcc = {"bcc@example.com"};
        test_email.subject = "Test Email";
        test_email.body = "This is a test email body.";
        test_email.html_body = "<h1>Test Email</h1><p>This is a test email with <strong>HTML</strong> content.</p>";
    }

    void TearDown() override {
        // Clean up after each test
    }

    ssmtp_mailer::Email test_email;
};

// Test HTTP client infrastructure
TEST_F(ProviderIntegrationTest, HTTPClientInfrastructure) {
    EXPECT_NO_THROW({
        auto http_client = ssmtp_mailer::HTTPClientFactory::createClient();
        EXPECT_NE(http_client, nullptr);

        auto backends = ssmtp_mailer::HTTPClientFactory::getAvailableBackends();
        EXPECT_FALSE(backends.empty());
    });
}

// Test API client factory
TEST_F(ProviderIntegrationTest, APIClientFactory) {
    auto providers = ssmtp_mailer::APIClientFactory::getSupportedProviders();
    EXPECT_FALSE(providers.empty());

    // Test provider support checking
    EXPECT_TRUE(ssmtp_mailer::APIClientFactory::isProviderSupported(ssmtp_mailer::APIProvider::SENDGRID));
    EXPECT_TRUE(ssmtp_mailer::APIClientFactory::isProviderSupported(ssmtp_mailer::APIProvider::MAILGUN));
    EXPECT_TRUE(ssmtp_mailer::APIClientFactory::isProviderSupported(ssmtp_mailer::APIProvider::AMAZON_SES));
    EXPECT_FALSE(ssmtp_mailer::APIClientFactory::isProviderSupported(ssmtp_mailer::APIProvider::POSTMARK));
}

// Test SendGrid client
TEST_F(ProviderIntegrationTest, SendGridClient) {
    ssmtp_mailer::APIClientConfig config;
    config.provider = ssmtp_mailer::APIProvider::SENDGRID;
    config.auth.api_key = "test_key";
    config.sender_email = "test@example.com";
    config.request.base_url = "https://api.sendgrid.com";
    config.request.endpoint = "/v3/mail/send";

    EXPECT_NO_THROW({
        auto client = ssmtp_mailer::APIClientFactory::createClient(config);
        EXPECT_NE(client, nullptr);
        EXPECT_EQ(client->getProviderName(), "SendGrid");
        EXPECT_TRUE(client->isValid());
    });
}

// Test Mailgun client
TEST_F(ProviderIntegrationTest, MailgunClient) {
    ssmtp_mailer::APIClientConfig config;
    config.provider = ssmtp_mailer::APIProvider::MAILGUN;
    config.auth.api_key = "test_key";
    config.sender_email = "test@example.com";
    config.request.base_url = "https://api.mailgun.net/v3";
    config.request.endpoint = "/messages";
    config.request.custom_headers["domain"] = "example.com";

    EXPECT_NO_THROW({
        auto client = ssmtp_mailer::APIClientFactory::createClient(config);
        EXPECT_NE(client, nullptr);
        EXPECT_EQ(client->getProviderName(), "Mailgun");
        EXPECT_TRUE(client->isValid());
    });
}

// Test Amazon SES client
TEST_F(ProviderIntegrationTest, AmazonSESClient) {
    ssmtp_mailer::APIClientConfig config;
    config.provider = ssmtp_mailer::APIProvider::AMAZON_SES;
    config.auth.api_key = "test_access_key";
    config.auth.api_secret = "test_secret_key";
    config.sender_email = "test@example.com";
    config.request.base_url = "https://email.us-east-1.amazonaws.com";
    config.request.endpoint = "/v2/email";
    config.request.custom_headers["region"] = "us-east-1";

    EXPECT_NO_THROW({
        auto client = ssmtp_mailer::APIClientFactory::createClient(config);
        EXPECT_NE(client, nullptr);
        EXPECT_EQ(client->getProviderName(), "Amazon SES");
        EXPECT_TRUE(client->isValid());
    });
}

// Test email composition
TEST_F(ProviderIntegrationTest, EmailComposition) {
    EXPECT_FALSE(test_email.from.empty());
    EXPECT_FALSE(test_email.to.empty());
    EXPECT_FALSE(test_email.subject.empty());
    EXPECT_FALSE(test_email.body.empty());
    EXPECT_FALSE(test_email.html_body.empty());

    EXPECT_EQ(test_email.to.size(), 2);
    EXPECT_EQ(test_email.cc.size(), 1);
    EXPECT_EQ(test_email.bcc.size(), 1);

    EXPECT_EQ(test_email.from, "sender@example.com");
    EXPECT_EQ(test_email.to[0], "recipient1@example.com");
    EXPECT_EQ(test_email.to[1], "recipient2@example.com");
    EXPECT_EQ(test_email.cc[0], "cc@example.com");
    EXPECT_EQ(test_email.bcc[0], "bcc@example.com");
    EXPECT_EQ(test_email.subject, "Test Email");
}

// Test error handling with invalid configuration
TEST_F(ProviderIntegrationTest, ErrorHandling) {
    ssmtp_mailer::APIClientConfig invalid_config;
    invalid_config.provider = ssmtp_mailer::APIProvider::SENDGRID;
    // Missing required fields

    EXPECT_NO_THROW({
        auto client = ssmtp_mailer::APIClientFactory::createClient(invalid_config);
        EXPECT_NE(client, nullptr);
        EXPECT_FALSE(client->isValid());

        auto response = client->sendEmail(test_email);
        EXPECT_FALSE(response.success);
        EXPECT_EQ(response.http_code, 400);
        EXPECT_FALSE(response.error_message.empty());
    });
}

// Test batch sending
TEST_F(ProviderIntegrationTest, BatchSending) {
    ssmtp_mailer::APIClientConfig config;
    config.provider = ssmtp_mailer::APIProvider::SENDGRID;
    config.auth.api_key = "test_key";
    config.sender_email = "test@example.com";
    config.request.base_url = "https://api.sendgrid.com";
    config.request.endpoint = "/v3/mail/send";

    auto client = ssmtp_mailer::APIClientFactory::createClient(config);

    std::vector<ssmtp_mailer::Email> emails = {test_email, test_email};

    EXPECT_NO_THROW({
        auto responses = client->sendBatch(emails);
        EXPECT_EQ(responses.size(), 2);
        // Responses will likely fail due to test credentials, but shouldn't crash
    });
}
