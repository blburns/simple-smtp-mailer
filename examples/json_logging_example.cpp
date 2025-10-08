#include "simple-smtp-mailer/json_logger.hpp"
#include "simple-smtp-mailer/logger.hpp"
#include <iostream>
#include <map>
#include <vector>
#include <thread>
#include <chrono>

using namespace ssmtp_mailer;

int main() {
    std::cout << "=== JSON Logging Example ===" << std::endl;
    
    // Configure JSON logging
    JsonLogConfig config;
    config.enabled = true;
    config.fields = "timestamp,level,message,thread,component,event_type";
    config.pretty_print = false;
    config.include_stack_trace = false;
    config.timestamp_format = "%Y-%m-%dT%H:%M:%S.%fZ";
    
    // Add custom fields
    config.custom_fields["service"] = "simple-smtp-mailer";
    config.custom_fields["version"] = "0.2.0";
    config.custom_fields["environment"] = "development";
    
    // Initialize JSON logger
    JsonLogger::initialize(config);
    JsonLogger& json_logger = JsonLogger::getInstance();
    
    // Add additional custom fields
    json_logger.addCustomField("hostname", "mail-server-01");
    json_logger.addCustomField("region", "us-west-2");
    
    std::cout << "\n1. Basic JSON Logging:" << std::endl;
    json_logger.logJson(LogLevel::INFO, "Application started");
    json_logger.logJson(LogLevel::DEBUG, "Configuration loaded successfully");
    json_logger.logJson(LogLevel::WARNING, "Rate limit approaching threshold");
    json_logger.logJson(LogLevel::ERROR, "Failed to connect to SMTP server");
    
    std::cout << "\n2. Structured Data Logging:" << std::endl;
    std::map<std::string, std::string> config_data = {
        {"smtp_server", "smtp.example.com"},
        {"smtp_port", "587"},
        {"auth_method", "oauth2"},
        {"use_ssl", "true"}
    };
    json_logger.logStructured(LogLevel::INFO, config_data);
    
    std::cout << "\n3. Email Event Logging:" << std::endl;
    std::vector<std::string> recipients = {"user1@example.com", "user2@example.com"};
    json_logger.logEmailEvent(LogLevel::INFO, "sender@example.com", recipients, 
                             "Welcome to our service", "sent", "msg-12345");
    
    json_logger.logEmailEvent(LogLevel::ERROR, "sender@example.com", {"invalid@email"}, 
                             "Failed email", "failed", "", "Invalid email address");
    
    std::cout << "\n4. API Request Logging:" << std::endl;
    json_logger.logApiRequest(LogLevel::INFO, "SendGrid", "/v3/mail/send", "POST", 
                             200, 150, "");
    
    json_logger.logApiRequest(LogLevel::ERROR, "Mailgun", "/v3/domain/messages", "POST", 
                             401, 2000, "Invalid API key");
    
    std::cout << "\n5. Threaded Logging (simulating concurrent operations):" << std::endl;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([&json_logger, i]() {
            for (int j = 0; j < 2; ++j) {
                std::map<std::string, std::string> thread_data = {
                    {"worker_id", std::to_string(i)},
                    {"task_id", std::to_string(j)},
                    {"status", "processing"}
                };
                json_logger.logStructured(LogLevel::DEBUG, thread_data);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << "\n6. Different JSON Field Configurations:" << std::endl;
    
    // Minimal fields
    json_logger.setJsonFields("timestamp,level,message");
    json_logger.logJson(LogLevel::INFO, "Minimal JSON logging");
    
    // Extended fields
    json_logger.setJsonFields("timestamp,level,message,thread,component,event_type,service,version");
    json_logger.logJson(LogLevel::INFO, "Extended JSON logging");
    
    std::cout << "\n7. Pretty Print Example:" << std::endl;
    json_logger.enablePrettyPrint(true);
    json_logger.logJson(LogLevel::INFO, "Pretty printed JSON log");
    json_logger.enablePrettyPrint(false);
    
    std::cout << "\n8. Mixed Traditional and JSON Logging:" << std::endl;
    
    // Initialize traditional logger
    Logger::initialize("", LogLevel::INFO);
    Logger& traditional_logger = Logger::getInstance();
    
    traditional_logger.info("Traditional log message");
    json_logger.logJson(LogLevel::INFO, "JSON log message");
    
    std::cout << "\n=== JSON Logging Example Complete ===" << std::endl;
    
    return 0;
}
