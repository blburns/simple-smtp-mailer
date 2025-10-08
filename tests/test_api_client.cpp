#include <gtest/gtest.h>
#include <string>
#include "simple-smtp-mailer/api_client.hpp"
#include "simple-smtp-mailer/http_client.hpp"

class APIClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test configuration
        config.provider = ssmtp_mailer::APIProvider::SENDGRID;
        config.auth.api_key = "test_key";
        config.sender_email = "test@example.com";
        config.request.base_url = "https://api.sendgrid.com";
        config.request.endpoint = "/v3/mail/send";
    }

    void TearDown() override {
        // Clean up after each test
    }

    ssmtp_mailer::APIClientConfig config;
};

// Test HTTP client creation
TEST_F(APIClientTest, HTTPClientCreation) {
    EXPECT_NO_THROW({
        auto http_client = ssmtp_mailer::HTTPClientFactory::createClient();
        EXPECT_NE(http_client, nullptr);
    });
}

// Test available backends
TEST_F(APIClientTest, AvailableBackends) {
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

// Test API client factory
TEST_F(APIClientTest, APIClientFactory) {
    auto providers = ssmtp_mailer::APIClientFactory::getSupportedProviders();
    EXPECT_FALSE(providers.empty());

    // Check for expected providers
    bool has_sendgrid = false;
    bool has_mailgun = false;
    bool has_amazon_ses = false;

    for (const auto& provider : providers) {
        if (provider == "SendGrid") has_sendgrid = true;
        if (provider == "Mailgun") has_mailgun = true;
        if (provider == "Amazon SES") has_amazon_ses = true;
    }

    EXPECT_TRUE(has_sendgrid);
    EXPECT_TRUE(has_mailgun);
    EXPECT_TRUE(has_amazon_ses);
}

// Test provider support checking
TEST_F(APIClientTest, ProviderSupportCheck) {
    EXPECT_TRUE(ssmtp_mailer::APIClientFactory::isProviderSupported(ssmtp_mailer::APIProvider::SENDGRID));
    EXPECT_TRUE(ssmtp_mailer::APIClientFactory::isProviderSupported(ssmtp_mailer::APIProvider::MAILGUN));
    EXPECT_TRUE(ssmtp_mailer::APIClientFactory::isProviderSupported(ssmtp_mailer::APIProvider::AMAZON_SES));
    EXPECT_FALSE(ssmtp_mailer::APIClientFactory::isProviderSupported(ssmtp_mailer::APIProvider::POSTMARK));
}

// Test SendGrid client creation
TEST_F(APIClientTest, SendGridClientCreation) {
    EXPECT_NO_THROW({
        auto client = ssmtp_mailer::APIClientFactory::createClient(config);
        EXPECT_NE(client, nullptr);
        EXPECT_EQ(client->getProviderName(), "SendGrid");
        EXPECT_TRUE(client->isValid());
    });
}

// Test invalid configuration
TEST_F(APIClientTest, InvalidConfiguration) {
    ssmtp_mailer::APIClientConfig invalid_config;
    invalid_config.provider = ssmtp_mailer::APIProvider::SENDGRID;
    // Missing required fields

    EXPECT_NO_THROW({
        auto client = ssmtp_mailer::APIClientFactory::createClient(invalid_config);
        EXPECT_NE(client, nullptr);
        EXPECT_FALSE(client->isValid());
    });
}

// Test email validation
TEST_F(APIClientTest, EmailValidation) {
    auto client = ssmtp_mailer::APIClientFactory::createClient(config);

    // Test with valid email
    ssmtp_mailer::Email valid_email;
    valid_email.from = "test@example.com";
    valid_email.to = {"recipient@example.com"};
    valid_email.subject = "Test Subject";
    valid_email.body = "Test Body";

    // This should not throw (but may fail due to invalid credentials)
    EXPECT_NO_THROW({
        auto response = client->sendEmail(valid_email);
        // We expect this to fail due to test credentials, but it shouldn't crash
    });

    // Test with invalid email (missing recipient)
    ssmtp_mailer::Email invalid_email;
    invalid_email.from = "test@example.com";
    // Missing to field

    EXPECT_NO_THROW({
        auto response = client->sendEmail(invalid_email);
        EXPECT_FALSE(response.success);
        EXPECT_EQ(response.http_code, 400);
    });
}
