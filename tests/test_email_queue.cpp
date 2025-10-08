#include <gtest/gtest.h>
#include <string>
#include <thread>
#include <chrono>
#include "simple-smtp-mailer/queue_types.hpp"

class EmailQueueTest : public ::testing::Test {
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

// Test email creation
TEST_F(EmailQueueTest, EmailCreation) {
    EXPECT_FALSE(test_email.from.empty());
    EXPECT_FALSE(test_email.to.empty());
    EXPECT_FALSE(test_email.subject.empty());
    EXPECT_FALSE(test_email.body.empty());

    EXPECT_EQ(test_email.from, "test@example.com");
    EXPECT_EQ(test_email.to[0], "recipient@example.com");
    EXPECT_EQ(test_email.subject, "Test Subject");
    EXPECT_EQ(test_email.body, "Test Body");
}

// Test email priority
TEST_F(EmailQueueTest, EmailPriority) {
    ssmtp_mailer::QueueItem item;
    item.email = test_email;
    item.priority = ssmtp_mailer::EmailPriority::HIGH;
    item.retry_count = 0;
    item.max_retries = 3;

    EXPECT_EQ(item.priority, ssmtp_mailer::EmailPriority::HIGH);
    EXPECT_EQ(item.retry_count, 0);
    EXPECT_EQ(item.max_retries, 3);
}

// Test different priority levels
TEST_F(EmailQueueTest, PriorityLevels) {
    ssmtp_mailer::QueueItem low_priority;
    low_priority.email = test_email;
    low_priority.priority = ssmtp_mailer::EmailPriority::LOW;

    ssmtp_mailer::QueueItem normal_priority;
    normal_priority.email = test_email;
    normal_priority.priority = ssmtp_mailer::EmailPriority::NORMAL;

    ssmtp_mailer::QueueItem high_priority;
    high_priority.email = test_email;
    high_priority.priority = ssmtp_mailer::EmailPriority::HIGH;

    EXPECT_EQ(low_priority.priority, ssmtp_mailer::EmailPriority::LOW);
    EXPECT_EQ(normal_priority.priority, ssmtp_mailer::EmailPriority::NORMAL);
    EXPECT_EQ(high_priority.priority, ssmtp_mailer::EmailPriority::HIGH);
}

// Test retry mechanism
TEST_F(EmailQueueTest, RetryMechanism) {
    ssmtp_mailer::QueueItem item;
    item.email = test_email;
    item.retry_count = 0;
    item.max_retries = 3;

    // Test initial state
    EXPECT_EQ(item.retry_count, 0);
    EXPECT_EQ(item.max_retries, 3);
    EXPECT_FALSE(item.shouldRetry());

    // Simulate retries
    item.retry_count = 1;
    EXPECT_TRUE(item.shouldRetry());

    item.retry_count = 2;
    EXPECT_TRUE(item.shouldRetry());

    item.retry_count = 3;
    EXPECT_FALSE(item.shouldRetry());
}

// Test email with attachments (if supported)
TEST_F(EmailQueueTest, EmailWithAttachments) {
    // Add some test attachments
    test_email.attachments = {"test1.txt", "test2.pdf"};

    EXPECT_EQ(test_email.attachments.size(), 2);
    EXPECT_EQ(test_email.attachments[0], "test1.txt");
    EXPECT_EQ(test_email.attachments[1], "test2.pdf");
}

// Test email with HTML content
TEST_F(EmailQueueTest, EmailWithHTML) {
    test_email.html_body = "<h1>Test</h1><p>This is a test email.</p>";

    EXPECT_FALSE(test_email.html_body.empty());
    EXPECT_TRUE(test_email.html_body.find("<h1>") != std::string::npos);
    EXPECT_TRUE(test_email.html_body.find("<p>") != std::string::npos);
}
