#include <gtest/gtest.h>
#include <string>
#include "simple-smtp-mailer/api_client.hpp"
#include "simple-smtp-mailer/http_client.hpp"
#include "simple-smtp-mailer/mailer.hpp"
#include "simple-smtp-mailer/queue_types.hpp"

class BasicFunctionalityTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test email
        test_email.from = "test@example.com";
        test_email.to = {"recipient@example.com"};
        test_email.subject = "Test Subject";
        test_email.body = "Test Body";
    }

    void TearDown() override {
        // Clean up after each test
    }

    ssmtp_mailer::Email test_email;
};

// Test Email structure
TEST_F(BasicFunctionalityTest, EmailStructure) {
    EXPECT_FALSE(test_email.from.empty());
    EXPECT_FALSE(test_email.to.empty());
    EXPECT_FALSE(test_email.subject.empty());
    EXPECT_FALSE(test_email.body.empty());

    EXPECT_EQ(test_email.from, "test@example.com");
    EXPECT_EQ(test_email.to[0], "recipient@example.com");
    EXPECT_EQ(test_email.subject, "Test Subject");
    EXPECT_EQ(test_email.body, "Test Body");
}

// Test HTTP client creation
TEST_F(BasicFunctionalityTest, HTTPClientCreation) {
    EXPECT_NO_THROW({
        auto http_client = ssmtp_mailer::HTTPClientFactory::createClient();
        EXPECT_NE(http_client, nullptr);
    });
}

// Test available backends
TEST_F(BasicFunctionalityTest, AvailableBackends) {
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
TEST_F(BasicFunctionalityTest, APIClientFactory) {
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
TEST_F(BasicFunctionalityTest, ProviderSupportCheck) {
    EXPECT_TRUE(ssmtp_mailer::APIClientFactory::isProviderSupported(ssmtp_mailer::APIProvider::SENDGRID));
    EXPECT_TRUE(ssmtp_mailer::APIClientFactory::isProviderSupported(ssmtp_mailer::APIProvider::MAILGUN));
    EXPECT_TRUE(ssmtp_mailer::APIClientFactory::isProviderSupported(ssmtp_mailer::APIProvider::AMAZON_SES));
    EXPECT_FALSE(ssmtp_mailer::APIClientFactory::isProviderSupported(ssmtp_mailer::APIProvider::POSTMARK));
}

// Test SendGrid client creation
TEST_F(BasicFunctionalityTest, SendGridClientCreation) {
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

// Test Mailgun client creation
TEST_F(BasicFunctionalityTest, MailgunClientCreation) {
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

// Test Amazon SES client creation
TEST_F(BasicFunctionalityTest, AmazonSESClientCreation) {
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

// Test email validation
TEST_F(BasicFunctionalityTest, EmailValidation) {
    // Test with valid email
    EXPECT_FALSE(test_email.from.empty());
    EXPECT_FALSE(test_email.to.empty());
    EXPECT_FALSE(test_email.subject.empty());
    EXPECT_FALSE(test_email.body.empty());

    // Test with invalid email
    ssmtp_mailer::Email invalid_email;
    EXPECT_TRUE(invalid_email.from.empty());
    EXPECT_TRUE(invalid_email.to.empty());
}

// Test QueueItem structure
TEST_F(BasicFunctionalityTest, QueueItemStructure) {
    ssmtp_mailer::QueueItem item;
    item.from_address = "test@example.com";
    item.to_addresses = {"recipient@example.com"};
    item.subject = "Test Subject";
    item.body = "Test Body";
    item.priority = ssmtp_mailer::EmailPriority::NORMAL;
    item.status = ssmtp_mailer::EmailStatus::PENDING;
    item.retry_count = 0;
    item.max_retries = 3;

    EXPECT_EQ(item.from_address, "test@example.com");
    EXPECT_EQ(item.to_addresses[0], "recipient@example.com");
    EXPECT_EQ(item.subject, "Test Subject");
    EXPECT_EQ(item.body, "Test Body");
    EXPECT_EQ(item.priority, ssmtp_mailer::EmailPriority::NORMAL);
    EXPECT_EQ(item.status, ssmtp_mailer::EmailStatus::PENDING);
    EXPECT_EQ(item.retry_count, 0);
    EXPECT_EQ(item.max_retries, 3);
}

// Test priority levels
TEST_F(BasicFunctionalityTest, PriorityLevels) {
    EXPECT_EQ(static_cast<int>(ssmtp_mailer::EmailPriority::LOW), 0);
    EXPECT_EQ(static_cast<int>(ssmtp_mailer::EmailPriority::NORMAL), 1);
    EXPECT_EQ(static_cast<int>(ssmtp_mailer::EmailPriority::HIGH), 2);
    EXPECT_EQ(static_cast<int>(ssmtp_mailer::EmailPriority::URGENT), 3);
}

// Test status levels
TEST_F(BasicFunctionalityTest, StatusLevels) {
    EXPECT_EQ(static_cast<int>(ssmtp_mailer::EmailStatus::PENDING), 0);
    EXPECT_EQ(static_cast<int>(ssmtp_mailer::EmailStatus::PROCESSING), 1);
    EXPECT_EQ(static_cast<int>(ssmtp_mailer::EmailStatus::SENT), 2);
    EXPECT_EQ(static_cast<int>(ssmtp_mailer::EmailStatus::FAILED), 3);
    EXPECT_EQ(static_cast<int>(ssmtp_mailer::EmailStatus::RETRY), 4);
    EXPECT_EQ(static_cast<int>(ssmtp_mailer::EmailStatus::CANCELLED), 5);
}
