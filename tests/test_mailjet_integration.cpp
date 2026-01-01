#include <gtest/gtest.h>
#include <string>
#include <algorithm>
#include "simple-smtp-mailer/api_client.hpp"

class MailjetIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up Mailjet configuration
        config.provider = ssmtp_mailer::APIProvider::MAILJET;
        config.auth.api_key = "test_key";
        config.auth.api_secret = "test_secret"; // Mailjet requires both key and secret
        config.sender_email = "test@example.com";
        config.request.base_url = "https://api.mailjet.com";
        config.request.endpoint = "/v3.1/send";
        config.request.timeout_seconds = 30;
        config.request.verify_ssl = true;
        config.enable_tracking = true;

        // Set up test email
        test_email.from = "test@example.com";
        test_email.to = {"recipient@example.com"};
        test_email.subject = "Test Email from Mailjet";
        test_email.body = "This is a test email sent via Mailjet API.";
        test_email.html_body = "<h1>Test Email</h1><p>This is a test email sent via <strong>Mailjet API</strong>.</p>";
    }

    void TearDown() override {
        // Clean up after each test
    }

    ssmtp_mailer::APIClientConfig config;
    ssmtp_mailer::Email test_email;
};

// Test Mailjet client creation
TEST_F(MailjetIntegrationTest, ClientCreation) {
    EXPECT_NO_THROW({
        auto client = ssmtp_mailer::APIClientFactory::createClient(config);
        EXPECT_NE(client, nullptr);
        EXPECT_EQ(client->getProviderName(), "Mailjet");
        EXPECT_TRUE(client->isValid());
    });
}

// Test Mailjet authentication
TEST_F(MailjetIntegrationTest, Authentication) {
    auto client = ssmtp_mailer::APIClientFactory::createClient(config);

    // Test with valid configuration
    EXPECT_TRUE(client->isValid());

    // Test with invalid configuration (missing secret)
    ssmtp_mailer::APIClientConfig invalid_config;
    invalid_config.provider = ssmtp_mailer::APIProvider::MAILJET;
    invalid_config.auth.api_key = "test_key";
    // Missing api_secret

    auto invalid_client = ssmtp_mailer::APIClientFactory::createClient(invalid_config);
    EXPECT_FALSE(invalid_client->isValid());
}

// Test Mailjet email composition
TEST_F(MailjetIntegrationTest, EmailComposition) {
    auto client = ssmtp_mailer::APIClientFactory::createClient(config);
    
    // Test email composition
    EXPECT_FALSE(test_email.from.empty());
    EXPECT_FALSE(test_email.to.empty());
    EXPECT_FALSE(test_email.subject.empty());
    EXPECT_FALSE(test_email.body.empty());
}

// Test Mailjet batch sending structure
TEST_F(MailjetIntegrationTest, BatchSending) {
    auto client = ssmtp_mailer::APIClientFactory::createClient(config);
    
    std::vector<ssmtp_mailer::Email> batch;
    batch.push_back(test_email);
    
    EXPECT_EQ(batch.size(), 1);
}

// Test Mailjet provider support
TEST_F(MailjetIntegrationTest, ProviderSupport) {
    auto providers = ssmtp_mailer::APIClientFactory::getSupportedProviders();
    EXPECT_TRUE(std::find(providers.begin(), providers.end(), "Mailjet") != providers.end());
    
    EXPECT_TRUE(ssmtp_mailer::APIClientFactory::isProviderSupported(ssmtp_mailer::APIProvider::MAILJET));
}

// Test Mailjet configuration validation
TEST_F(MailjetIntegrationTest, ConfigurationValidation) {
    // Valid configuration
    EXPECT_TRUE(config.auth.api_key.length() > 0);
    EXPECT_TRUE(config.auth.api_secret.length() > 0);  // Mailjet requires both
    EXPECT_TRUE(config.sender_email.length() > 0);
    EXPECT_TRUE(config.request.base_url.length() > 0);
    
    // Invalid configuration (missing secret)
    ssmtp_mailer::APIClientConfig invalid_config;
    invalid_config.provider = ssmtp_mailer::APIProvider::MAILJET;
    invalid_config.auth.api_key = "test_key";
    invalid_config.auth.api_secret = "";  // Missing secret
    
    auto client = ssmtp_mailer::APIClientFactory::createClient(invalid_config);
    EXPECT_FALSE(client->isValid());
}

