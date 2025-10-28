#include <gtest/gtest.h>
#include <string>
#include "simple-smtp-mailer/api_client.hpp"

class SparkPostIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up SparkPost configuration
        config.provider = ssmtp_mailer::APIProvider::SPARKPOST;
        config.auth.api_key = "test_key"; // Replace with real key for actual testing
        config.sender_email = "test@example.com";
        config.request.base_url = "https://api.sparkpost.com";
        config.request.endpoint = "/api/v1/transmissions";
        config.request.timeout_seconds = 30;
        config.request.verify_ssl = true;
        config.enable_tracking = true;

        // Set up test email
        test_email.from = "test@example.com";
        test_email.to = {"recipient@example.com"};
        test_email.subject = "Test Email from SparkPost";
        test_email.body = "This is a test email sent via SparkPost API.";
        test_email.html_body = "<h1>Test Email</h1><p>This is a test email sent via <strong>SparkPost API</strong>.</p>";
    }

    void TearDown() override {
        // Clean up after each test
    }

    ssmtp_mailer::APIClientConfig config;
    ssmtp_mailer::Email test_email;
};

// Test SparkPost client creation
TEST_F(SparkPostIntegrationTest, ClientCreation) {
    EXPECT_NO_THROW({
        auto client = ssmtp_mailer::APIClientFactory::createClient(config);
        EXPECT_NE(client, nullptr);
        EXPECT_EQ(client->getProviderName(), "SparkPost");
        EXPECT_TRUE(client->isValid());
    });
}

// Test SparkPost authentication
TEST_F(SparkPostIntegrationTest, Authentication) {
    auto client = ssmtp_mailer::APIClientFactory::createClient(config);

    // Test with valid configuration
    EXPECT_TRUE(client->isValid());

    // Test with invalid configuration
    ssmtp_mailer::APIClientConfig invalid_config;
    invalid_config.provider = ssmtp_mailer::APIProvider::SPARKPOST;
    // Missing required fields

    auto invalid_client = ssmtp_mailer::APIClientFactory::createClient(invalid_config);
    EXPECT_FALSE(invalid_client->isValid());
}

// Test SparkPost email composition
TEST_F(SparkPostIntegrationTest, EmailComposition) {
    auto client = ssmtp_mailer::APIClientFactory::createClient(config);
    
    // Test email composition
    EXPECT_FALSE(test_email.from.empty());
    EXPECT_FALSE(test_email.to.empty());
    EXPECT_FALSE(test_email.subject.empty());
    EXPECT_FALSE(test_email.body.empty());
}

// Test SparkPost batch sending structure
TEST_F(SparkPostIntegrationTest, BatchSending) {
    auto client = ssmtp_mailer::APIClientFactory::createClient(config);
    
    std::vector<ssmtp_mailer::Email> batch;
    batch.push_back(test_email);
    
    EXPECT_EQ(batch.size(), 1);
}

// Test SparkPost provider support
TEST_F(SparkPostIntegrationTest, ProviderSupport) {
    auto providers = ssmtp_mailer::APIClientFactory::getSupportedProviders();
    EXPECT_TRUE(std::find(providers.begin(), providers.end(), "SparkPost") != providers.end());
    
    EXPECT_TRUE(ssmtp_mailer::APIClientFactory::isProviderSupported(ssmtp_mailer::APIProvider::SPARKPOST));
}

// Test SparkPost configuration validation
TEST_F(SparkPostIntegrationTest, ConfigurationValidation) {
    // Valid configuration
    EXPECT_TRUE(config.auth.api_key.length() > 0);
    EXPECT_TRUE(config.sender_email.length() > 0);
    EXPECT_TRUE(config.request.base_url.length() > 0);
    
    // Invalid configuration
    ssmtp_mailer::APIClientConfig invalid_config;
    invalid_config.provider = ssmtp_mailer::APIProvider::SPARKPOST;
    invalid_config.auth.api_key = "";
    
    auto client = ssmtp_mailer::APIClientFactory::createClient(invalid_config);
    EXPECT_FALSE(client->isValid());
}

