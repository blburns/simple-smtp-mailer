#include "simple-smtp-mailer/api_client.hpp"
#include "simple-smtp-mailer/http_client.hpp"
#include "simple-smtp-mailer/mailer.hpp"
#include <sstream>
#include <json/json.h>
#include <iostream>

namespace ssmtp_mailer {

SparkPostAPIClient::SparkPostAPIClient(const APIClientConfig& config) 
    : config_(config) {
    // Set default SparkPost configuration if not provided
    if (config_.request.base_url.empty()) {
        config_.request.base_url = "https://api.sparkpost.com";
    }
    if (config_.request.endpoint.empty()) {
        config_.request.endpoint = "/api/v1/transmissions";
    }
}

APIResponse SparkPostAPIClient::sendEmail(const Email& email) {
    APIResponse response;
    
    if (!isValid()) {
        response.error_message = "Invalid SparkPost API client configuration";
        return response;
    }
    
    try {
        // Build request body
        std::string requestBody = buildRequestBody(email);
        std::map<std::string, std::string> headers = buildHeaders();
        
        // Make HTTP request to SparkPost API
        auto httpClient = HTTPClientFactory::createClient();
        httpClient->setTimeout(config_.request.timeout_seconds);
        httpClient->setSSLVerification(config_.request.verify_ssl);
        
        HTTPRequest request;
        request.method = HTTPMethod::POST;
        request.url = config_.request.base_url + config_.request.endpoint;
        request.headers = headers;
        request.body = requestBody;
        
        HTTPResponse httpResponse = httpClient->sendRequest(request);
        
        response.http_code = httpResponse.status_code;
        response.raw_response = httpResponse.body;
        
        if (httpResponse.status_code >= 200 && httpResponse.status_code < 300) {
            response.success = true;
            
            // Parse response to extract message ID
            Json::Value root;
            Json::Reader reader;
            if (reader.parse(httpResponse.body, root)) {
                if (root.isMember("results") && root["results"].isMember("id")) {
                    response.message_id = root["results"]["id"].asString();
                } else if (root.isMember("id")) {
                    response.message_id = root["id"].asString();
                }
            }
        } else {
            response.success = false;
            parseErrorResponse(httpResponse, response);
        }
        
    } catch (const std::exception& e) {
        response.success = false;
        response.error_message = "Exception in SparkPost API client: " + std::string(e.what());
    }
    
    return response;
}

std::vector<APIResponse> SparkPostAPIClient::sendBatch(const std::vector<Email>& emails) {
    std::vector<APIResponse> responses;
    responses.reserve(emails.size());
    
    // SparkPost supports batch sending through their API
    for (const auto& email : emails) {
        responses.push_back(sendEmail(email));
    }
    
    return responses;
}

bool SparkPostAPIClient::testConnection() {
    APIResponse response;
    try {
        auto httpClient = HTTPClientFactory::createClient();
        httpClient->setTimeout(config_.request.timeout_seconds);
        httpClient->setSSLVerification(config_.request.verify_ssl);
        
        HTTPRequest request;
        request.method = HTTPMethod::GET;
        request.url = config_.request.base_url + "/api/v1/subaccounts";
        request.headers = buildHeaders();
        
        HTTPResponse httpResponse = httpClient->sendRequest(request);
        
        return (httpResponse.status_code >= 200 && httpResponse.status_code < 300);
    } catch (...) {
        return false;
    }
}

bool SparkPostAPIClient::isValid() const {
    return !config_.auth.api_key.empty();
}

std::string SparkPostAPIClient::buildRequestBody(const Email& email) {
    Json::Value root;
    Json::Value content;
    Json::Value recipients(Json::arrayValue);
    Json::Value recipient;
    
    // Build recipients array
    recipient["address"]["email"] = email.to[0];  // SparkPost uses array format
    recipients.append(recipient);
    root["recipients"] = recipients;
    
    // Build content
    if (!email.html_body.empty()) {
        content["html"] = email.html_body;
    }
    content["text"] = email.body;
    
    // From
    if (!config_.sender_email.empty()) {
        content["from"]["email"] = config_.sender_email;
    } else {
        content["from"]["email"] = email.from;
    }
    if (!config_.sender_name.empty()) {
        content["from"]["name"] = config_.sender_name;
    }
    
    // Subject
    content["subject"] = email.subject;
    
    root["content"] = content;
    
    // Options
    if (config_.enable_tracking) {
        root["options"]["open_tracking"] = true;
        root["options"]["click_tracking"] = true;
    }
    
    // Campaign
    if (!config_.request.custom_headers.empty()) {
        auto it = config_.request.custom_headers.find("campaign");
        if (it != config_.request.custom_headers.end()) {
            root["campaign_id"] = it->second;
        }
    }
    
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    return Json::writeString(builder, root);
}

std::map<std::string, std::string> SparkPostAPIClient::buildHeaders() {
    std::map<std::string, std::string> headers;
    
    // SparkPost requires authorization header
    headers["Accept"] = "application/json";
    headers["Content-Type"] = "application/json";
    
    // API Key authentication
    if (!config_.auth.api_key.empty()) {
        headers["Authorization"] = config_.auth.api_key;
    }
    
    return headers;
}

void SparkPostAPIClient::parseErrorResponse(const HTTPResponse& httpResponse, APIResponse& apiResponse) {
    apiResponse.success = false;
    
    try {
        Json::Value root;
        Json::Reader reader;
        if (reader.parse(httpResponse.body, root)) {
            if (root.isMember("errors") && root["errors"].isArray() && !root["errors"].empty()) {
                auto firstError = root["errors"][0];
                if (firstError.isMember("message")) {
                    apiResponse.error_message = firstError["message"].asString();
                } else if (firstError.isMember("description")) {
                    apiResponse.error_message = firstError["description"].asString();
                } else {
                    apiResponse.error_message = httpResponse.body;
                }
            } else {
                apiResponse.error_message = "HTTP " + std::to_string(httpResponse.status_code) + ": " + httpResponse.body;
            }
        } else {
            apiResponse.error_message = "HTTP " + std::to_string(httpResponse.status_code) + ": " + httpResponse.body;
        }
    } catch (...) {
        apiResponse.error_message = "HTTP " + std::to_string(httpResponse.status_code) + ": " + httpResponse.body;
    }
}

} // namespace ssmtp_mailer

