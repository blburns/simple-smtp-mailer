#pragma once

#include "logger.hpp"
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <mutex>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <thread>

namespace ssmtp_mailer {

/**
 * @brief JSON logging configuration
 */
struct JsonLogConfig {
    bool enabled = false;
    std::string fields = "timestamp,level,message,thread";
    std::map<std::string, std::string> custom_fields;
    bool pretty_print = false;
    bool include_stack_trace = false;
    std::string timestamp_format = "%Y-%m-%dT%H:%M:%S.%fZ";
    
    JsonLogConfig() = default;
};

/**
 * @brief Enhanced logger with JSON support
 */
class JsonLogger {
public:
    /**
     * @brief Initialize JSON logger
     * @param config JSON logging configuration
     */
    static void initialize(const JsonLogConfig& config = JsonLogConfig());
    
    /**
     * @brief Get JSON logger instance
     * @return Reference to JSON logger instance
     */
    static JsonLogger& getInstance();
    
    /**
     * @brief Enable/disable JSON logging
     * @param enable Whether to enable JSON logging
     */
    void enableJsonLogging(bool enable);
    
    /**
     * @brief Set JSON logging fields
     * @param fields Comma-separated list of fields
     */
    void setJsonFields(const std::string& fields);
    
    /**
     * @brief Add custom field to JSON output
     * @param key Field key
     * @param value Field value
     */
    void addCustomField(const std::string& key, const std::string& value);
    
    /**
     * @brief Remove custom field from JSON output
     * @param key Field key to remove
     */
    void removeCustomField(const std::string& key);
    
    /**
     * @brief Set timestamp format for JSON output
     * @param format Timestamp format string
     */
    void setTimestampFormat(const std::string& format);
    
    /**
     * @brief Enable/disable pretty printing
     * @param enable Whether to enable pretty printing
     */
    void enablePrettyPrint(bool enable);
    
    /**
     * @brief Enable/disable stack trace in JSON output
     * @param enable Whether to include stack trace
     */
    void enableStackTrace(bool enable);
    
    /**
     * @brief Log message in JSON format
     * @param level Log level
     * @param message Message to log
     * @param additional_fields Additional fields to include
     */
    void logJson(LogLevel level, const std::string& message, 
                 const std::map<std::string, std::string>& additional_fields = {});
    
    /**
     * @brief Log structured data in JSON format
     * @param level Log level
     * @param data Structured data to log
     */
    void logStructured(LogLevel level, const std::map<std::string, std::string>& data);
    
    /**
     * @brief Log email sending event in JSON format
     * @param level Log level
     * @param from From email address
     * @param to To email addresses
     * @param subject Email subject
     * @param status Send status
     * @param message_id Message ID (optional)
     * @param error_message Error message (optional)
     */
    void logEmailEvent(LogLevel level, const std::string& from, 
                      const std::vector<std::string>& to, const std::string& subject,
                      const std::string& status, const std::string& message_id = "",
                      const std::string& error_message = "");
    
    /**
     * @brief Log API request in JSON format
     * @param level Log level
     * @param provider API provider name
     * @param endpoint API endpoint
     * @param method HTTP method
     * @param status_code HTTP status code
     * @param response_time_ms Response time in milliseconds
     * @param error_message Error message (optional)
     */
    void logApiRequest(LogLevel level, const std::string& provider, 
                      const std::string& endpoint, const std::string& method,
                      int status_code, long response_time_ms,
                      const std::string& error_message = "");

public:
    /**
     * @brief Constructor (public for make_unique, but singleton pattern enforced via static methods)
     */
    JsonLogger();
    
    /**
     * @brief Copy constructor (deleted)
     */
    JsonLogger(const JsonLogger&) = delete;
    
    /**
     * @brief Assignment operator (deleted)
     */
    JsonLogger& operator=(const JsonLogger&) = delete;
    
    /**
     * @brief Format message as JSON
     * @param level Log level
     * @param message Message to format
     * @param additional_fields Additional fields
     * @return JSON formatted string
     */
    std::string formatJsonMessage(LogLevel level, const std::string& message,
                                 const std::map<std::string, std::string>& additional_fields = {}) const;
    
    /**
     * @brief Get current timestamp in ISO format
     * @return ISO timestamp string
     */
    std::string getCurrentTimestamp() const;
    
    /**
     * @brief Get log level string
     * @param level Log level
     * @return Log level string
     */
    std::string getLogLevelString(LogLevel level) const;
    
    /**
     * @brief Get current thread ID
     * @return Thread ID string
     */
    std::string getCurrentThreadId() const;
    
    /**
     * @brief Escape JSON string
     * @param str String to escape
     * @return Escaped string
     */
    std::string escapeJsonString(const std::string& str) const;
    
    /**
     * @brief Write JSON message to output
     * @param json_message JSON message to write
     */
    void writeJsonMessage(const std::string& json_message);
    
    /**
     * @brief Parse fields string into vector
     * @param fields Comma-separated fields string
     * @return Vector of field names
     */
    std::vector<std::string> parseFields(const std::string& fields) const;

private:
    static std::unique_ptr<JsonLogger> instance_;
    static std::mutex instance_mutex_;
    
    JsonLogConfig config_;
    std::mutex log_mutex_;
    std::vector<std::string> enabled_fields_;
};

/**
 * @brief JSON logging macros for convenience
 */
#define LOG_JSON_DEBUG(logger, message, ...) logger.logJson(LogLevel::DEBUG, message, __VA_ARGS__)
#define LOG_JSON_INFO(logger, message, ...) logger.logJson(LogLevel::INFO, message, __VA_ARGS__)
#define LOG_JSON_WARNING(logger, message, ...) logger.logJson(LogLevel::WARNING, message, __VA_ARGS__)
#define LOG_JSON_ERROR(logger, message, ...) logger.logJson(LogLevel::ERROR, message, __VA_ARGS__)
#define LOG_JSON_CRITICAL(logger, message, ...) logger.logJson(LogLevel::CRITICAL, message, __VA_ARGS__)

} // namespace ssmtp_mailer
