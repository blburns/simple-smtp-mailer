#include "simple-smtp-mailer/api_client.hpp"
#include "simple-smtp-mailer/http_client.hpp"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <iomanip>

namespace ssmtp_mailer {

AmazonSESAPIClient::AmazonSESAPIClient(const APIClientConfig& config) : config_(config) {
    // Set default Amazon SES configuration if not provided
    if (config_.request.base_url.empty()) {
        // Extract region from config or use default
        std::string region = getRegionFromConfig();
        if (region.empty()) {
            region = "us-east-1"; // Default region
        }
        config_.request.base_url = "https://email." + region + ".amazonaws.com";
    }
    if (config_.request.endpoint.empty()) {
        config_.request.endpoint = "/v2/email";
    }
}

APIResponse AmazonSESAPIClient::sendEmail(const Email& email) {
    APIResponse response;

    // Validate configuration
    if (!isValid()) {
        response.error_message = "Amazon SES client not properly configured";
        response.success = false;
        response.http_code = 400;
        return response;
    }

    // Validate email
    if (email.from.empty() || email.to.empty()) {
        response.error_message = "Email must have at least one recipient and a sender";
        response.success = false;
        response.http_code = 400;
        return response;
    }

    // Create HTTP client
    auto http_client = HTTPClientFactory::createClient();

    // Build request
    HTTPRequest http_request;
    http_request.method = HTTPMethod::POST;
    http_request.url = config_.request.base_url + config_.request.endpoint;
    http_request.body = buildRequestBody(email);
    http_request.headers = buildHeaders();
    http_request.timeout_seconds = config_.request.timeout_seconds;
    http_request.verify_ssl = config_.request.verify_ssl;

    // Send request
    HTTPResponse http_response = http_client->sendRequest(http_request);

    // Process response
    response.http_code = http_response.status_code;
    response.success = http_response.success;
    response.raw_response = http_response.body;

    if (response.success) {
        // Extract message ID from response
        response.message_id = extractMessageId(http_response.body);

        // If no message ID in body, try headers
        if (response.message_id.empty()) {
            auto it = http_response.headers.find("X-Amzn-RequestId");
            if (it != http_response.headers.end()) {
                response.message_id = it->second;
            }
        }

        // Log success
        std::cout << "✓ Amazon SES email sent successfully. Message ID: " << response.message_id << std::endl;
    } else {
        // Enhanced error handling for Amazon SES-specific errors
        response.error_message = parseAmazonSESError(http_response);
        if (response.error_message.empty()) {
            response.error_message = http_response.error_message;
        }
        if (response.error_message.empty() && !http_response.body.empty()) {
            response.error_message = http_response.body;
        }

        // Log error
        std::cerr << "✗ Amazon SES email failed: " << response.error_message << " (HTTP " << response.http_code << ")" << std::endl;
    }

    return response;
}

std::vector<APIResponse> AmazonSESAPIClient::sendBatch(const std::vector<Email>& emails) {
    std::vector<APIResponse> responses;
    responses.reserve(emails.size());

    // Amazon SES supports batch sending through their API
    // For now, we'll send them individually, but this could be optimized
    for (const auto& email : emails) {
        responses.push_back(sendEmail(email));
    }

    return responses;
}

bool AmazonSESAPIClient::testConnection() {
    // Test connection by making a simple API call to get sending statistics
    auto http_client = HTTPClientFactory::createClient();

    HTTPRequest http_request;
    http_request.method = HTTPMethod::GET;
    http_request.url = config_.request.base_url + "/v2/account/sending";
    http_request.headers = buildHeaders();
    http_request.timeout_seconds = config_.request.timeout_seconds;
    http_request.verify_ssl = config_.request.verify_ssl;

    HTTPResponse http_response = http_client->sendRequest(http_request);

    // Check if the credentials are valid
    bool is_valid = http_response.status_code >= 200 && http_response.status_code < 300;

    if (is_valid) {
        std::cout << "✓ Amazon SES connection test successful" << std::endl;
    } else {
        std::cerr << "✗ Amazon SES connection test failed: " << http_response.body << " (HTTP " << http_response.status_code << ")" << std::endl;
    }

    return is_valid;
}

bool AmazonSESAPIClient::isValid() const {
    return !config_.auth.api_key.empty() &&
           !config_.auth.api_secret.empty() &&
           !config_.request.base_url.empty() &&
           !config_.sender_email.empty();
}

std::string AmazonSESAPIClient::buildRequestBody(const Email& email) {
    // Build Amazon SES v2 API request body (JSON format)
    std::ostringstream body;

    body << "{";

    // From email address
    body << "\"FromEmailAddress\":\"" << email.from << "\"";

    // Destination
    body << ",\"Destination\":{";
    body << "\"ToAddresses\":[";
    for (size_t i = 0; i < email.to.size(); ++i) {
        if (i > 0) body << ",";
        body << "\"" << email.to[i] << "\"";
    }
    body << "]";

    // CC addresses
    if (!email.cc.empty()) {
        body << ",\"CcAddresses\":[";
        for (size_t i = 0; i < email.cc.size(); ++i) {
            if (i > 0) body << ",";
            body << "\"" << email.cc[i] << "\"";
        }
        body << "]";
    }

    // BCC addresses
    if (!email.bcc.empty()) {
        body << ",\"BccAddresses\":[";
        for (size_t i = 0; i < email.bcc.size(); ++i) {
            if (i > 0) body << ",";
            body << "\"" << email.bcc[i] << "\"";
        }
        body << "]";
    }

    body << "}";

    // Content
    body << ",\"Content\":{";
    body << "\"Simple\":{";

    // Subject
    body << "\"Subject\":{\"Data\":\"" << escapeJson(email.subject) << "\"}";

    // Body
    body << ",\"Body\":{";

    // Text body
    if (!email.body.empty()) {
        body << "\"Text\":{\"Data\":\"" << escapeJson(email.body) << "\"}";
    }

    // HTML body
    if (!email.html_body.empty()) {
        if (!email.body.empty()) body << ",";
        body << "\"Html\":{\"Data\":\"" << escapeJson(email.html_body) << "\"}";
    }

    body << "}"; // End Body
    body << "}"; // End Simple
    body << "}"; // End Content

    // Configuration set (if specified)
    std::string config_set = getConfigurationSetFromConfig();
    if (!config_set.empty()) {
        body << ",\"ConfigurationSetName\":\"" << config_set << "\"";
    }

    // Tags for analytics
    body << ",\"EmailTags\":[";
    body << "{\"Name\":\"Source\",\"Value\":\"ssmtp-mailer\"}";
    body << ",{\"Name\":\"Environment\",\"Value\":\"production\"}";
    body << "]";

    body << "}";

    return body.str();
}

std::map<std::string, std::string> AmazonSESAPIClient::buildHeaders() {
    std::map<std::string, std::string> headers;

    // Amazon SES uses AWS Signature Version 4 for authentication
    // For now, we'll use a simplified approach with API key/secret
    // In production, you'd want to implement proper AWS SigV4 signing

    // Content type
    headers["Content-Type"] = "application/json";

    // User agent
    headers["User-Agent"] = "ssmtp-mailer/0.2.0";

    // AWS-specific headers
    std::string region = getRegionFromConfig();
    if (!region.empty()) {
        headers["X-Amz-Region"] = region;
    }

    // For now, we'll use a simplified authentication approach
    // In production, implement proper AWS SigV4 signing
    headers["X-Amz-Access-Key"] = config_.auth.api_key;
    headers["X-Amz-Secret-Key"] = config_.auth.api_secret;

    // Add custom headers from config
    for (const auto& header : config_.request.headers) {
        headers[header.first] = header.second;
    }

    return headers;
}

std::string AmazonSESAPIClient::getRegionFromConfig() const {
    // Try to get region from custom headers first
    auto it = config_.request.custom_headers.find("region");
    if (it != config_.request.custom_headers.end()) {
        return it->second;
    }

    // Try to extract region from base URL
    std::string base_url = config_.request.base_url;
    size_t region_start = base_url.find("email.");
    if (region_start != std::string::npos) {
        region_start += 6; // length of "email."
        size_t region_end = base_url.find(".amazonaws.com", region_start);
        if (region_end != std::string::npos) {
            return base_url.substr(region_start, region_end - region_start);
        }
    }

    return "";
}

std::string AmazonSESAPIClient::getConfigurationSetFromConfig() const {
    // Try to get configuration set from custom headers
    auto it = config_.request.custom_headers.find("ses_configuration_set");
    if (it != config_.request.custom_headers.end()) {
        return it->second;
    }

    return "";
}

std::string AmazonSESAPIClient::extractMessageId(const std::string& response_body) {
    // Amazon SES v2 response format: {"MessageId":"abc123-def456-ghi789"}

    // Look for "MessageId" field in the response
    size_t id_pos = response_body.find("\"MessageId\":\"");
    if (id_pos != std::string::npos) {
        id_pos += 14; // length of "\"MessageId\":\""
        size_t end_pos = response_body.find("\"", id_pos);
        if (end_pos != std::string::npos) {
            return response_body.substr(id_pos, end_pos - id_pos);
        }
    }

    return "";
}

std::string AmazonSESAPIClient::escapeJson(const std::string& str) {
    std::ostringstream escaped;

    for (char c : str) {
        switch (c) {
            case '"':  escaped << "\\\""; break;
            case '\\': escaped << "\\\\"; break;
            case '\b': escaped << "\\b";  break;
            case '\f': escaped << "\\f";  break;
            case '\n': escaped << "\\n";  break;
            case '\r': escaped << "\\r";  break;
            case '\t': escaped << "\\t";  break;
            default:
                if (c < 32 || c > 126) {
                    // Escape non-printable characters
                    escaped << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)c;
                } else {
                    escaped << c;
                }
                break;
        }
    }

    return escaped.str();
}

std::string AmazonSESAPIClient::parseAmazonSESError(const HTTPResponse& response) {
    // Parse Amazon SES-specific error messages
    std::string error_message;

    // Check for common HTTP status codes
    switch (response.status_code) {
        case 400:
            error_message = "Bad Request - Invalid email format or missing required fields";
            break;
        case 401:
            error_message = "Unauthorized - Invalid AWS credentials or authentication failed";
            break;
        case 403:
            error_message = "Forbidden - AWS credentials don't have permission for SES";
            break;
        case 404:
            error_message = "Not Found - SES endpoint not found or region not supported";
            break;
        case 413:
            error_message = "Request Entity Too Large - Email size exceeds SES limits";
            break;
        case 429:
            error_message = "Too Many Requests - SES sending quota exceeded";
            break;
        case 500:
            error_message = "Internal Server Error - Amazon SES service error";
            break;
        case 502:
            error_message = "Bad Gateway - Amazon SES service temporarily unavailable";
            break;
        case 503:
            error_message = "Service Unavailable - Amazon SES service maintenance";
            break;
        default:
            if (response.status_code >= 400) {
                error_message = "HTTP " + std::to_string(response.status_code) + " error";
            }
            break;
    }

    // Try to parse JSON error response from Amazon SES
    if (!response.body.empty() && response.body.find("{") != std::string::npos) {
        // Look for error message in JSON response
        size_t error_pos = response.body.find("\"message\":\"");
        if (error_pos != std::string::npos) {
            error_pos += 11; // length of "\"message\":\""
            size_t end_pos = response.body.find("\"", error_pos);
            if (end_pos != std::string::npos) {
                std::string json_error = response.body.substr(error_pos, end_pos - error_pos);
                if (!json_error.empty()) {
                    error_message = json_error;
                }
            }
        }

        // Look for AWS error type
        size_t type_pos = response.body.find("\"__type\":\"");
        if (type_pos != std::string::npos) {
            type_pos += 11; // length of "\"__type\":\""
            size_t end_pos = response.body.find("\"", type_pos);
            if (end_pos != std::string::npos) {
                std::string error_type = response.body.substr(type_pos, end_pos - type_pos);
                if (!error_type.empty()) {
                    error_message = error_type + ": " + error_message;
                }
            }
        }
    }

    return error_message;
}

} // namespace ssmtp_mailer
