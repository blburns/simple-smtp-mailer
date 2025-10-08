#include "json_logger.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <thread>
#include <algorithm>
#include <cctype>

namespace ssmtp_mailer {

std::unique_ptr<JsonLogger> JsonLogger::instance_ = nullptr;
std::mutex JsonLogger::instance_mutex_;

void JsonLogger::initialize(const JsonLogConfig& config) {
    std::lock_guard<std::mutex> lock(instance_mutex_);
    if (!instance_) {
        instance_ = std::make_unique<JsonLogger>();
    }
    instance_->config_ = config;
    instance_->enabled_fields_ = instance_->parseFields(config.fields);
}

JsonLogger& JsonLogger::getInstance() {
    std::lock_guard<std::mutex> lock(instance_mutex_);
    if (!instance_) {
        instance_ = std::make_unique<JsonLogger>();
    }
    return *instance_;
}

JsonLogger::JsonLogger() {
    config_.enabled = false;
    config_.fields = "timestamp,level,message,thread";
    config_.pretty_print = false;
    config_.include_stack_trace = false;
    config_.timestamp_format = "%Y-%m-%dT%H:%M:%S.%fZ";
    enabled_fields_ = parseFields(config_.fields);
}

void JsonLogger::enableJsonLogging(bool enable) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    config_.enabled = enable;
}

void JsonLogger::setJsonFields(const std::string& fields) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    config_.fields = fields;
    enabled_fields_ = parseFields(fields);
}

void JsonLogger::addCustomField(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    config_.custom_fields[key] = value;
}

void JsonLogger::removeCustomField(const std::string& key) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    config_.custom_fields.erase(key);
}

void JsonLogger::setTimestampFormat(const std::string& format) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    config_.timestamp_format = format;
}

void JsonLogger::enablePrettyPrint(bool enable) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    config_.pretty_print = enable;
}

void JsonLogger::enableStackTrace(bool enable) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    config_.include_stack_trace = enable;
}

void JsonLogger::logJson(LogLevel level, const std::string& message,
                        const std::map<std::string, std::string>& additional_fields) {
    if (!config_.enabled) {
        return;
    }

    std::string json_message = formatJsonMessage(level, message, additional_fields);
    writeJsonMessage(json_message);
}

void JsonLogger::logStructured(LogLevel level, const std::map<std::string, std::string>& data) {
    if (!config_.enabled) {
        return;
    }

    std::ostringstream json;
    json << "{";

    bool first = true;

    // Add timestamp if enabled
    if (std::find(enabled_fields_.begin(), enabled_fields_.end(), "timestamp") != enabled_fields_.end()) {
        if (!first) json << ",";
        json << "\"timestamp\":\"" << getCurrentTimestamp() << "\"";
        first = false;
    }

    // Add log level if enabled
    if (std::find(enabled_fields_.begin(), enabled_fields_.end(), "level") != enabled_fields_.end()) {
        if (!first) json << ",";
        json << "\"level\":\"" << getLogLevelString(level) << "\"";
        first = false;
    }

    // Add thread ID if enabled
    if (std::find(enabled_fields_.begin(), enabled_fields_.end(), "thread") != enabled_fields_.end()) {
        if (!first) json << ",";
        json << "\"thread\":\"" << getCurrentThreadId() << "\"";
        first = false;
    }

    // Add structured data
    for (const auto& pair : data) {
        if (!first) json << ",";
        json << "\"" << escapeJsonString(pair.first) << "\":\"" << escapeJsonString(pair.second) << "\"";
        first = false;
    }

    // Add custom fields
    for (const auto& pair : config_.custom_fields) {
        if (!first) json << ",";
        json << "\"" << escapeJsonString(pair.first) << "\":\"" << escapeJsonString(pair.second) << "\"";
        first = false;
    }

    json << "}";

    writeJsonMessage(json.str());
}

void JsonLogger::logEmailEvent(LogLevel level, const std::string& from,
                              const std::vector<std::string>& to, const std::string& subject,
                              const std::string& status, const std::string& message_id,
                              const std::string& error_message) {
    if (!config_.enabled) {
        return;
    }

    std::map<std::string, std::string> data;
    data["event_type"] = "email_send";
    data["from"] = from;
    data["to"] = "";
    for (size_t i = 0; i < to.size(); ++i) {
        if (i > 0) data["to"] += ",";
        data["to"] += to[i];
    }
    data["subject"] = subject;
    data["status"] = status;
    if (!message_id.empty()) {
        data["message_id"] = message_id;
    }
    if (!error_message.empty()) {
        data["error"] = error_message;
    }

    logStructured(level, data);
}

void JsonLogger::logApiRequest(LogLevel level, const std::string& provider,
                              const std::string& endpoint, const std::string& method,
                              int status_code, long response_time_ms,
                              const std::string& error_message) {
    if (!config_.enabled) {
        return;
    }

    std::map<std::string, std::string> data;
    data["event_type"] = "api_request";
    data["provider"] = provider;
    data["endpoint"] = endpoint;
    data["method"] = method;
    data["status_code"] = std::to_string(status_code);
    data["response_time_ms"] = std::to_string(response_time_ms);
    if (!error_message.empty()) {
        data["error"] = error_message;
    }

    logStructured(level, data);
}

std::string JsonLogger::formatJsonMessage(LogLevel level, const std::string& message,
                                         const std::map<std::string, std::string>& additional_fields) const {
    std::ostringstream json;
    json << "{";

    bool first = true;

    // Add enabled fields
    for (const auto& field : enabled_fields_) {
        if (!first) json << ",";

        if (field == "timestamp") {
            json << "\"timestamp\":\"" << getCurrentTimestamp() << "\"";
        } else if (field == "level") {
            json << "\"level\":\"" << getLogLevelString(level) << "\"";
        } else if (field == "message") {
            json << "\"message\":\"" << escapeJsonString(message) << "\"";
        } else if (field == "thread") {
            json << "\"thread\":\"" << getCurrentThreadId() << "\"";
        } else if (additional_fields.find(field) != additional_fields.end()) {
            json << "\"" << escapeJsonString(field) << "\":\"" << escapeJsonString(additional_fields.at(field)) << "\"";
        }

        first = false;
    }

    // Add additional fields not in enabled_fields_
    for (const auto& pair : additional_fields) {
        if (std::find(enabled_fields_.begin(), enabled_fields_.end(), pair.first) == enabled_fields_.end()) {
            if (!first) json << ",";
            json << "\"" << escapeJsonString(pair.first) << "\":\"" << escapeJsonString(pair.second) << "\"";
            first = false;
        }
    }

    // Add custom fields
    for (const auto& pair : config_.custom_fields) {
        if (!first) json << ",";
        json << "\"" << escapeJsonString(pair.first) << "\":\"" << escapeJsonString(pair.second) << "\"";
        first = false;
    }

    json << "}";
    return json.str();
}

std::string JsonLogger::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(3) << ms.count() << "Z";

    return oss.str();
}

std::string JsonLogger::getLogLevelString(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

std::string JsonLogger::getCurrentThreadId() const {
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    return oss.str();
}

std::string JsonLogger::escapeJsonString(const std::string& str) const {
    std::ostringstream escaped;
    for (char c : str) {
        switch (c) {
            case '"':  escaped << "\\\""; break;
            case '\\': escaped << "\\\\"; break;
            case '\b': escaped << "\\b"; break;
            case '\f': escaped << "\\f"; break;
            case '\n': escaped << "\\n"; break;
            case '\r': escaped << "\\r"; break;
            case '\t': escaped << "\\t"; break;
            default:
                if (c < 0x20) {
                    escaped << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
                } else {
                    escaped << c;
                }
                break;
        }
    }
    return escaped.str();
}

void JsonLogger::writeJsonMessage(const std::string& json_message) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    std::cout << json_message << std::endl;
}

std::vector<std::string> JsonLogger::parseFields(const std::string& fields) const {
    std::vector<std::string> result;
    std::stringstream ss(fields);
    std::string field;

    while (std::getline(ss, field, ',')) {
        // Trim whitespace
        field.erase(0, field.find_first_not_of(" \t"));
        field.erase(field.find_last_not_of(" \t") + 1);

        if (!field.empty()) {
            result.push_back(field);
        }
    }

    return result;
}

} // namespace ssmtp_mailer
