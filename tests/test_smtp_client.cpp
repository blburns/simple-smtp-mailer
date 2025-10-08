#include <gtest/gtest.h>
#include <string>
#include "simple-smtp-mailer/smtp_client.hpp"
#include "simple-smtp-mailer/mailer.hpp"

class SMTPClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test configuration
        config.host = "smtp.gmail.com";
        config.port = 587;
        config.username = "test@example.com";
        config.password = "test_password";
        config.use_tls = true;
        config.verify_cert = true;

        // Set up test email
        test_email.from = "test@example.com";
        test_email.to = {"recipient@example.com"};
        test_email.subject = "Test Subject";
        test_email.body = "Test Body";
    }

    void TearDown() override {
        // Clean up after each test
    }

    ssmtp_mailer::SMTPConfig config;
    ssmtp_mailer::Email test_email;
};

// Test SMTP client creation
TEST_F(SMTPClientTest, ClientCreation) {
    EXPECT_NO_THROW({
        auto client = std::make_unique<ssmtp_mailer::SMTPClient>(config);
        EXPECT_NE(client, nullptr);
    });
}

// Test configuration validation
TEST_F(SMTPClientTest, ConfigurationValidation) {
    auto client = std::make_unique<ssmtp_mailer::SMTPClient>(config);

    // Test valid configuration
    EXPECT_TRUE(client->isConfigured());

    // Test invalid configuration
    ssmtp_mailer::SMTPConfig invalid_config;
    auto invalid_client = std::make_unique<ssmtp_mailer::SMTPClient>(invalid_config);
    EXPECT_FALSE(invalid_client->isConfigured());
}

// Test email validation
TEST_F(SMTPClientTest, EmailValidation) {
    auto client = std::make_unique<ssmtp_mailer::SMTPClient>(config);

    // Test valid email
    EXPECT_NO_THROW({
        auto result = client->sendEmail(test_email);
        // This will likely fail due to test credentials, but shouldn't crash
        EXPECT_NE(result.success, true); // Expected to fail with test credentials
    });

    // Test invalid email (missing recipient)
    ssmtp_mailer::Email invalid_email;
    invalid_email.from = "test@example.com";
    // Missing to field

    EXPECT_NO_THROW({
        auto result = client->sendEmail(invalid_email);
        EXPECT_FALSE(result.success);
        EXPECT_FALSE(result.error_message.empty());
    });
}

// Test connection testing
TEST_F(SMTPClientTest, ConnectionTest) {
    auto client = std::make_unique<ssmtp_mailer::SMTPClient>(config);

    EXPECT_NO_THROW({
        bool connected = client->testConnection();
        // This will likely fail due to test credentials, but shouldn't crash
        EXPECT_FALSE(connected); // Expected to fail with test credentials
    });
}

// Test different SMTP configurations
TEST_F(SMTPClientTest, DifferentConfigurations) {
    // Test Gmail configuration
    ssmtp_mailer::SMTPConfig gmail_config;
    gmail_config.host = "smtp.gmail.com";
    gmail_config.port = 587;
    gmail_config.use_tls = true;

    EXPECT_NO_THROW({
        auto client = std::make_unique<ssmtp_mailer::SMTPClient>(gmail_config);
        EXPECT_NE(client, nullptr);
    });

    // Test Office 365 configuration
    ssmtp_mailer::SMTPConfig office365_config;
    office365_config.host = "smtp.office365.com";
    office365_config.port = 587;
    office365_config.use_tls = true;

    EXPECT_NO_THROW({
        auto client = std::make_unique<ssmtp_mailer::SMTPClient>(office365_config);
        EXPECT_NE(client, nullptr);
    });

    // Test custom SMTP configuration
    ssmtp_mailer::SMTPConfig custom_config;
    custom_config.host = "smtp.example.com";
    custom_config.port = 25;
    custom_config.use_tls = false;

    EXPECT_NO_THROW({
        auto client = std::make_unique<ssmtp_mailer::SMTPClient>(custom_config);
        EXPECT_NE(client, nullptr);
    });
}
