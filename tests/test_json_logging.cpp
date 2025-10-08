#include "gtest/gtest.h"
#include "simple-smtp-mailer/json_logger.hpp"
#include "simple-smtp-mailer/logger.hpp"
#include <sstream>
#include <map>
#include <vector>

namespace ssmtp_mailer {

class JsonLoggingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset JSON logger for each test
        JsonLogConfig config;
        config.enabled = true;
        config.fields = "timestamp,level,message,thread";
        config.pretty_print = false;
        JsonLogger::initialize(config);
    }

    void TearDown() override {
        // Cleanup if needed
    }
};

TEST_F(JsonLoggingTest, BasicJsonLogging) {
    JsonLogger& logger = JsonLogger::getInstance();
    
    // Capture output
    std::ostringstream captured_output;
    auto old_cout = std::cout.rdbuf();
    std::cout.rdbuf(captured_output.rdbuf());
    
    logger.logJson(LogLevel::INFO, "Test message");
    
    // Restore cout
    std::cout.rdbuf(old_cout);
    
    std::string output = captured_output.str();
    
    // Verify JSON structure
    EXPECT_TRUE(output.find("\"level\":\"INFO\"") != std::string::npos);
    EXPECT_TRUE(output.find("\"message\":\"Test message\"") != std::string::npos);
    EXPECT_TRUE(output.find("\"timestamp\":") != std::string::npos);
    EXPECT_TRUE(output.find("\"thread\":") != std::string::npos);
    EXPECT_TRUE(output.find("{") != std::string::npos);
    EXPECT_TRUE(output.find("}") != std::string::npos);
}

TEST_F(JsonLoggingTest, JsonFieldConfiguration) {
    JsonLogger& logger = JsonLogger::getInstance();
    
    // Test minimal fields
    logger.setJsonFields("level,message");
    
    std::ostringstream captured_output;
    auto old_cout = std::cout.rdbuf();
    std::cout.rdbuf(captured_output.rdbuf());
    
    logger.logJson(LogLevel::ERROR, "Error message");
    
    std::cout.rdbuf(old_cout);
    std::string output = captured_output.str();
    
    // Should only contain level and message
    EXPECT_TRUE(output.find("\"level\":\"ERROR\"") != std::string::npos);
    EXPECT_TRUE(output.find("\"message\":\"Error message\"") != std::string::npos);
    EXPECT_TRUE(output.find("\"timestamp\":") == std::string::npos);
    EXPECT_TRUE(output.find("\"thread\":") == std::string::npos);
}

TEST_F(JsonLoggingTest, CustomFields) {
    JsonLogger& logger = JsonLogger::getInstance();
    
    logger.addCustomField("service", "test-service");
    logger.addCustomField("version", "1.0.0");
    
    std::ostringstream captured_output;
    auto old_cout = std::cout.rdbuf();
    std::cout.rdbuf(captured_output.rdbuf());
    
    logger.logJson(LogLevel::INFO, "Message with custom fields");
    
    std::cout.rdbuf(old_cout);
    std::string output = captured_output.str();
    
    EXPECT_TRUE(output.find("\"service\":\"test-service\"") != std::string::npos);
    EXPECT_TRUE(output.find("\"version\":\"1.0.0\"") != std::string::npos);
}

TEST_F(JsonLoggingTest, EmailEventLogging) {
    JsonLogger& logger = JsonLogger::getInstance();
    
    std::vector<std::string> recipients = {"user1@example.com", "user2@example.com"};
    
    std::ostringstream captured_output;
    auto old_cout = std::cout.rdbuf();
    std::cout.rdbuf(captured_output.rdbuf());
    
    logger.logEmailEvent(LogLevel::INFO, "sender@example.com", recipients, 
                        "Test Subject", "sent", "msg-12345");
    
    std::cout.rdbuf(old_cout);
    std::string output = captured_output.str();
    
    EXPECT_TRUE(output.find("\"event_type\":\"email_send\"") != std::string::npos);
    EXPECT_TRUE(output.find("\"from\":\"sender@example.com\"") != std::string::npos);
    EXPECT_TRUE(output.find("\"to\":\"user1@example.com,user2@example.com\"") != std::string::npos);
    EXPECT_TRUE(output.find("\"subject\":\"Test Subject\"") != std::string::npos);
    EXPECT_TRUE(output.find("\"status\":\"sent\"") != std::string::npos);
    EXPECT_TRUE(output.find("\"message_id\":\"msg-12345\"") != std::string::npos);
}

TEST_F(JsonLoggingTest, ApiRequestLogging) {
    JsonLogger& logger = JsonLogger::getInstance();
    
    std::ostringstream captured_output;
    auto old_cout = std::cout.rdbuf();
    std::cout.rdbuf(captured_output.rdbuf());
    
    logger.logApiRequest(LogLevel::INFO, "SendGrid", "/v3/mail/send", "POST", 
                         200, 150, "");
    
    std::cout.rdbuf(old_cout);
    std::string output = captured_output.str();
    
    EXPECT_TRUE(output.find("\"event_type\":\"api_request\"") != std::string::npos);
    EXPECT_TRUE(output.find("\"provider\":\"SendGrid\"") != std::string::npos);
    EXPECT_TRUE(output.find("\"endpoint\":\"/v3/mail/send\"") != std::string::npos);
    EXPECT_TRUE(output.find("\"method\":\"POST\"") != std::string::npos);
    EXPECT_TRUE(output.find("\"status_code\":\"200\"") != std::string::npos);
    EXPECT_TRUE(output.find("\"response_time_ms\":\"150\"") != std::string::npos);
}

TEST_F(JsonLoggingTest, StructuredDataLogging) {
    JsonLogger& logger = JsonLogger::getInstance();
    
    std::map<std::string, std::string> data = {
        {"component", "smtp_client"},
        {"action", "connect"},
        {"server", "smtp.example.com"},
        {"port", "587"}
    };
    
    std::ostringstream captured_output;
    auto old_cout = std::cout.rdbuf();
    std::cout.rdbuf(captured_output.rdbuf());
    
    logger.logStructured(LogLevel::DEBUG, data);
    
    std::cout.rdbuf(old_cout);
    std::string output = captured_output.str();
    
    EXPECT_TRUE(output.find("\"component\":\"smtp_client\"") != std::string::npos);
    EXPECT_TRUE(output.find("\"action\":\"connect\"") != std::string::npos);
    EXPECT_TRUE(output.find("\"server\":\"smtp.example.com\"") != std::string::npos);
    EXPECT_TRUE(output.find("\"port\":\"587\"") != std::string::npos);
}

TEST_F(JsonLoggingTest, JsonStringEscaping) {
    JsonLogger& logger = JsonLogger::getInstance();
    
    std::ostringstream captured_output;
    auto old_cout = std::cout.rdbuf();
    std::cout.rdbuf(captured_output.rdbuf());
    
    // Test message with special characters
    logger.logJson(LogLevel::INFO, "Message with \"quotes\" and \n newlines and \t tabs");
    
    std::cout.rdbuf(old_cout);
    std::string output = captured_output.str();
    
    // Should be properly escaped
    EXPECT_TRUE(output.find("\\\"quotes\\\"") != std::string::npos);
    EXPECT_TRUE(output.find("\\n") != std::string::npos);
    EXPECT_TRUE(output.find("\\t") != std::string::npos);
}

TEST_F(JsonLoggingTest, DisableJsonLogging) {
    JsonLogger& logger = JsonLogger::getInstance();
    
    logger.enableJsonLogging(false);
    
    std::ostringstream captured_output;
    auto old_cout = std::cout.rdbuf();
    std::cout.rdbuf(captured_output.rdbuf());
    
    logger.logJson(LogLevel::INFO, "This should not appear");
    
    std::cout.rdbuf(old_cout);
    std::string output = captured_output.str();
    
    // Should be empty when JSON logging is disabled
    EXPECT_TRUE(output.empty());
}

TEST_F(JsonLoggingTest, LogLevels) {
    JsonLogger& logger = JsonLogger::getInstance();
    
    std::vector<LogLevel> levels = {
        LogLevel::DEBUG, LogLevel::INFO, LogLevel::WARNING, 
        LogLevel::ERROR, LogLevel::CRITICAL
    };
    
    std::vector<std::string> expected_levels = {
        "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"
    };
    
    for (size_t i = 0; i < levels.size(); ++i) {
        std::ostringstream captured_output;
        auto old_cout = std::cout.rdbuf();
        std::cout.rdbuf(captured_output.rdbuf());
        
        logger.logJson(levels[i], "Test message");
        
        std::cout.rdbuf(old_cout);
        std::string output = captured_output.str();
        
        EXPECT_TRUE(output.find("\"level\":\"" + expected_levels[i] + "\"") != std::string::npos);
    }
}

TEST_F(JsonLoggingTest, TimestampFormat) {
    JsonLogger& logger = JsonLogger::getInstance();
    
    std::ostringstream captured_output;
    auto old_cout = std::cout.rdbuf();
    std::cout.rdbuf(captured_output.rdbuf());
    
    logger.logJson(LogLevel::INFO, "Timestamp test");
    
    std::cout.rdbuf(old_cout);
    std::string output = captured_output.str();
    
    // Should contain ISO timestamp format
    EXPECT_TRUE(output.find("\"timestamp\":") != std::string::npos);
    // Should contain 'T' and 'Z' for ISO format
    EXPECT_TRUE(output.find("T") != std::string::npos);
    EXPECT_TRUE(output.find("Z") != std::string::npos);
}

} // namespace ssmtp_mailer
