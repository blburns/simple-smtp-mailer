#include <iostream>
#include <string>
#include "simple-smtp-mailer/api_client.hpp"
#include "simple-smtp-mailer/http_client.hpp"

void testMailgunIntegration() {
    std::cout << "🧪 Testing Mailgun API Integration" << std::endl;
    std::cout << "===================================" << std::endl;

    try {
        // Configure Mailgun client
        ssmtp_mailer::APIClientConfig config;
        config.provider = ssmtp_mailer::APIProvider::MAILGUN;
        config.auth.api_key = "test_key"; // Replace with real API key for testing
        config.sender_email = "test@example.com";
        config.request.base_url = "https://api.mailgun.net/v3";
        config.request.endpoint = "/messages";
        config.request.custom_headers["domain"] = "example.com"; // Required for Mailgun

        auto client = ssmtp_mailer::APIClientFactory::createClient(config);

        std::cout << "✓ Mailgun client created successfully" << std::endl;
        std::cout << "  Provider: " << client->getProviderName() << std::endl;
        std::cout << "  Is valid: " << (client->isValid() ? "Yes" : "No") << std::endl;

        // Test connection (this will fail with test credentials, but shows the flow)
        std::cout << "\n🔗 Testing connection..." << std::endl;
        bool connection_ok = client->testConnection();
        std::cout << "  Connection test: " << (connection_ok ? "✓ Success" : "✗ Failed (expected with test credentials)") << std::endl;

        // Test email composition
        std::cout << "\n📧 Testing email composition..." << std::endl;
        ssmtp_mailer::Email email;
        email.from = "test@example.com";
        email.to = {"recipient@example.com"};
        email.subject = "Test Email from Mailgun";
        email.body = "This is a test email sent via Mailgun API.";
        email.html_body = "<h1>Test Email</h1><p>This is a test email sent via <strong>Mailgun API</strong>.</p>";

        std::cout << "  From: " << email.from << std::endl;
        std::cout << "  To: " << email.to[0] << std::endl;
        std::cout << "  Subject: " << email.subject << std::endl;
        std::cout << "  Body length: " << email.body.length() << " characters" << std::endl;
        std::cout << "  HTML body length: " << email.html_body.length() << " characters" << std::endl;

        // Note: Actual sending would require real credentials
        std::cout << "\n⚠️  Note: Actual email sending requires valid Mailgun API key and verified domain" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "✗ Mailgun test failed: " << e.what() << std::endl;
    }

    std::cout << std::endl;
}

void testAmazonSESIntegration() {
    std::cout << "🧪 Testing Amazon SES API Integration" << std::endl;
    std::cout << "=====================================" << std::endl;

    try {
        // Configure Amazon SES client
        ssmtp_mailer::APIClientConfig config;
        config.provider = ssmtp_mailer::APIProvider::AMAZON_SES;
        config.auth.api_key = "test_access_key"; // Replace with real AWS access key
        config.auth.api_secret = "test_secret_key"; // Replace with real AWS secret key
        config.sender_email = "test@example.com";
        config.request.base_url = "https://email.us-east-1.amazonaws.com";
        config.request.endpoint = "/v2/email";
        config.request.custom_headers["region"] = "us-east-1";

        auto client = ssmtp_mailer::APIClientFactory::createClient(config);

        std::cout << "✓ Amazon SES client created successfully" << std::endl;
        std::cout << "  Provider: " << client->getProviderName() << std::endl;
        std::cout << "  Is valid: " << (client->isValid() ? "Yes" : "No") << std::endl;

        // Test connection (this will fail with test credentials, but shows the flow)
        std::cout << "\n🔗 Testing connection..." << std::endl;
        bool connection_ok = client->testConnection();
        std::cout << "  Connection test: " << (connection_ok ? "✓ Success" : "✗ Failed (expected with test credentials)") << std::endl;

        // Test email composition
        std::cout << "\n📧 Testing email composition..." << std::endl;
        ssmtp_mailer::Email email;
        email.from = "test@example.com";
        email.to = {"recipient@example.com"};
        email.cc = {"cc@example.com"};
        email.bcc = {"bcc@example.com"};
        email.subject = "Test Email from Amazon SES";
        email.body = "This is a test email sent via Amazon SES API.";
        email.html_body = "<h1>Test Email</h1><p>This is a test email sent via <strong>Amazon SES API</strong>.</p>";

        std::cout << "  From: " << email.from << std::endl;
        std::cout << "  To: " << email.to[0] << std::endl;
        std::cout << "  CC: " << email.cc[0] << std::endl;
        std::cout << "  BCC: " << email.bcc[0] << std::endl;
        std::cout << "  Subject: " << email.subject << std::endl;
        std::cout << "  Body length: " << email.body.length() << " characters" << std::endl;
        std::cout << "  HTML body length: " << email.html_body.length() << " characters" << std::endl;

        // Note: Actual sending would require real credentials
        std::cout << "\n⚠️  Note: Actual email sending requires valid AWS credentials and verified sender email" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "✗ Amazon SES test failed: " << e.what() << std::endl;
    }

    std::cout << std::endl;
}

void testErrorHandling() {
    std::cout << "🧪 Testing Error Handling" << std::endl;
    std::cout << "=========================" << std::endl;

    try {
        // Test with invalid configuration
        ssmtp_mailer::APIClientConfig config;
        config.provider = ssmtp_mailer::APIProvider::MAILGUN;
        // Missing required fields

        auto client = ssmtp_mailer::APIClientFactory::createClient(config);

        std::cout << "✓ Client created with invalid config" << std::endl;
        std::cout << "  Is valid: " << (client->isValid() ? "Yes" : "No") << std::endl;

        // Test email sending with invalid config
        ssmtp_mailer::Email email;
        email.from = "test@example.com";
        email.to = {"recipient@example.com"};
        email.subject = "Test Email";
        email.body = "This is a test email.";

        std::cout << "\n📧 Testing email sending with invalid config..." << std::endl;
        ssmtp_mailer::APIResponse response = client->sendEmail(email);

        std::cout << "  Success: " << (response.success ? "Yes" : "No") << std::endl;
        std::cout << "  HTTP Code: " << response.http_code << std::endl;
        std::cout << "  Error Message: " << response.error_message << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "✗ Error handling test failed: " << e.what() << std::endl;
    }

    std::cout << std::endl;
}

int main() {
    std::cout << "🚀 API Integration Test Suite" << std::endl;
    std::cout << "==============================" << std::endl;
    std::cout << std::endl;

    testMailgunIntegration();
    testAmazonSESIntegration();
    testErrorHandling();

    std::cout << "✅ All API integration tests completed!" << std::endl;
    std::cout << std::endl;
    std::cout << "📋 Next Steps:" << std::endl;
    std::cout << "1. Replace test credentials with real API keys" << std::endl;
    std::cout << "2. Verify sender email addresses with providers" << std::endl;
    std::cout << "3. Test actual email sending" << std::endl;
    std::cout << "4. Monitor delivery and error rates" << std::endl;

    return 0;
}
