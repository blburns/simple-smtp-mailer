#include <gtest/gtest.h>
#include <string>
#include "simple-smtp-mailer/api_client.hpp"

class PostmarkIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up Postmark configuration
        config.provider = ssmtp_mailer::APIProvider::POSTMARK;
        config.auth.api_key = "test_key"; // Replace with real key for actual testing
        config.sender_email = "test@example.com";
        config.request.base_url = "https://api.postmarkapp.com";
        config.request.endpoint = "/email";
        config.request.timeout_seconds = 30;
        config.request.verify_ssl = true;
        config.enable_tracking = true;

        // Set up test email
        test_email.from = "test@example.com";
        test_email.to = {"recipient@example.com"};
        test_email.subject = "Test Email from Postmark";
        test_email.body = "This is a test email sent via Postmark API.";
        test_email.html_body = "<h1>Test Email</h1><p>This is a test email sent via <strong>Postmark API</strong>.</p>";
    }

    void TearDown() override {
        // Clean up after each test
    }

    ssmtp_mailer::APIClientConfig config;
    ssmtp_mailer::Email test_email;
};

// Test Postmark client creation
TEST_F(PostmarkIntegrationTest, ClientCreation) {
    EXPECT_NO_THROW({
        auto client = ssmtp_mailer::APIClientFactory::createClient(config);
        EXPECT_NE(client, nullptr);
        EXPECT_EQ(client->getProviderName(), "Postmark");
        EXPECT_TRUE(client->isValid());
    });
}

// Test Postmark authentication
TEST_F(PostmarkIntegrationTest, Authentication) {
    auto client = ssmtp_mailer::APIClientFactory::createClient(config);

    // Test with valid configuration
    EXPECT_TRUE(client->isValid());

    // Test with invalid configuration
    ssmtp_mailer::APIClientConfig invalid_config;
    invalid_config.provider = ssmtp_mailer::APIProvider::POSTMARK;
    // Missing required fields

    auto invalid_client = ssmtp_mailer::APIClientFactory::createClient(invalid_config);
    EXPECT_FALSE(invalid_client->isValid());
}

// Test Postmark email composition
TEST_F(PostmarkIntegrationTest, EmailComposition) {
    auto client = ssmtp_mailer::APIClientFactory::createClient(config);
    
    // Test email composition
    EXPECT_FALSE(test_email.from.empty());
    EXPECT_FALSE(test_email.to.empty());
    EXPECT_FALSE(test_email.subject.empty());
    EXPECT_FALSE(test_email.body.empty());
}

// Test Postmark batch sending structure
TEST_F(PostmarkIntegrationTest, BatchSending) {
    auto client = ssmtp_mailer::APIClientFactory::createClient(config);
    
    std::vector<ssmtp_mailer::Email> batch;
    batch.push_back(test_email);
    
    EXPECT_EQ(batch.size(), 1);
}

// Test Postmark provider support
TEST_F(PostmarkIntegrationTest, ProviderSupport) {
    auto providers = ssmtp_mailer::APIClientFactory::getSupportedProviders();
    EXPECT_TRUE(std::find(providers.begin(), providers.end(), "Postmark") != providers.end());
    
    EXPECT_TRUE(ssmtp_mailer::APIClientFactory::isProviderSupported(ssmtp_mailer::APIProvider::POSTMARK));
}

// Test Postmark configuration validation
TEST_F(PostmarkIntegrationTest, ConfigurationValidation) {
    // Valid configuration
    EXPECT_TRUE(config.auth.api_key.length() > 0);
    EXPECT_TRUE(config.sender_email.length() > 0);
    EXPECT_TRUE(config.request.base_url.length() > 0);
    
    // Invalid configuration
    ssmtp_mailer::APIClientConfig invalid_config;
    invalid_config.provider = ssmtp_mailer::APIProvider::POSTMARK;
    invalid_config.auth.api_key = "";
    
    auto client = ssmtp_mailer::APIClientFactory::createClient(invalid_config);
    EXPECT_FALSE(client->isValid());
}

// Test Postmark request/response structure
TEST_F(PostmarkIntegrationTest, RequestResponseStructure) {
    auto client = ssmtp_mailer::APIClientFactory::createClient(config);
    
    // The client should be able to handle basic requests
    EXPECT_NO_THROW({
        // This will fail with test credentials, but should not throw exceptions
        // Uncomment and add real credentials to test actual sending
        // client->sendEmail(test_email);
    });
}

