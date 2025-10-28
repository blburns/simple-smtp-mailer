#include "simple-smtp-mailer/api_client.hpp"
#include "simple-smtp-mailer/http_client.hpp"
#include "simple-smtp-mailer/mailer.hpp"
#include <sstream>
#include <json/json.h>
#include <iostream>

namespace ssmtp_mailer {

PostmarkAPIClient::PostmarkAPIClient(const APIClientConfig& config) 
    : config_(config) {
    // Set default Postmark configuration if not provided
    if (config_.request.base_url.empty()) {
        config_.request.base_url = "https://api.postmarkapp.com";
    }
    if (config_.request.endpoint.empty()) {
        config_.request.endpoint = "/email";
    }
}

APIResponse PostmarkAPIClient::sendEmail(const Email& email) {
    APIResponse response;
    
    if (!isValid()) {
        response.error_message = "Invalid Postmark API client configuration";
        return response;
    }
    
    try {
        // Build request body
        std::string requestBody = buildRequestBody(email);
        std::map<std::string, std::string> headers = buildHeaders();
        
        // Make HTTP request to Postmark API
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
                if (root.isMember("MessageID")) {
                    response.message_id = root["MessageID"].asString();
                } else if (root.isMember("MessageId")) {
                    response.message_id = root["MessageId"].asString();
                }
            }
        } else {
            response.success = false;
            parseErrorResponse(httpResponse, response);
        }
        
    } catch (const std::exception& e) {
        response.success = false;
        response.error_message = "Exception in Postmark API client: " + std::string(e.what());
    }
    
    return response;
}

std::vector<APIResponse> PostmarkAPIClient::sendBatch(const std::vector<Email>& emails) {
    std::vector<APIResponse> responses;
    responses.reserve(emails.size());
    
    // Postmark supports batch sending through their API
    for (const auto& email : emails) {
        responses.push_back(sendEmail(email));
    }
    
    return responses;
}

bool PostmarkAPIClient::testConnection() {
    APIResponse response;
    try {
        auto httpClient = HTTPClientFactory::createClient();
        httpClient->setTimeout(config_.request.timeout_seconds);
        httpClient->setSSLVerification(config_.request.verify_ssl);
        
        HTTPRequest request;
        request.method = HTTPMethod::GET;
        request.url = config_.request.base_url + "/server";
        request.headers = buildHeaders();
        
        HTTPResponse httpResponse = httpClient->sendRequest(request);
        
        return (httpResponse.status_code >= 200 && httpResponse.status_code < 300);
    } catch (...) {
        return false;
    }
}

bool PostmarkAPIClient::isValid() const {
    return !config_.auth.api_key.empty();
}

std::string PostmarkAPIClient::buildRequestBody(const Email& email) {
    Json::Value root;
    
    // From
    root["From"] = email.from;
    
    // To
    if (!email.to.empty()) {
        root["To"] = email.to[0];  // Postmark uses single recipient per call
    }
    
    // Subject
    root["Subject"] = email.subject;
    
    // Body
    if (!email.html_body.empty()) {
        root["HtmlBody"] = email.html_body;
    }
    root["TextBody"] = email.body;
    
    // CC
    if (!email.cc.empty()) {
        Json::Value ccArray(Json::arrayValue);
        for (const auto& cc : email.cc) {
            ccArray.append(cc);
        }
        root["Cc"] = ccArray;
    }
    
    // BCC
    if (!email.bcc.empty()) {
        Json::Value bccArray(Json::arrayValue);
        for (const auto& bcc : email.bcc) {
            bccArray.append(bcc);
        }
        root["Bcc"] = bccArray;
    }
    
    // Attachments
    if (!email.attachments.empty()) {
        Json::Value attachmentsArray(Json::arrayValue);
        for (const auto& attachment : email.attachments) {
            // Note: In a real implementation, you'd need to encode file content
            // For now, we'll just add the file path
            Json::Value att;
            att["Name"] = attachment;
            // att["Content"] = base64_encoded_content;
            // att["ContentType"] = mime_type;
            attachmentsArray.append(att);
        }
        root["Attachments"] = attachmentsArray;
    }
    
    // Tracking
    if (config_.enable_tracking) {
        root["TrackOpens"] = true;
        root["TrackLinks"] = true;
    }
    
    // Tag
    if (!config_.request.custom_headers.empty()) {
        auto it = config_.request.custom_headers.find("Tag");
        if (it != config_.request.custom_headers.end()) {
            root["Tag"] = it->second;
        }
    }
    
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    return Json::writeString(builder, root);
}

std::map<std::string, std::string> PostmarkAPIClient::buildHeaders() {
    std::map<std::string, std::string> headers;
    
    // Postmark requires two headers
    headers["Accept"] = "application/json";
    headers["Content-Type"] = "application/json";
    
    // API Key authentication
    if (!config_.auth.api_key.empty()) {
        headers["X-Postmark-Server-Token"] = config_.auth.api_key;
    }
    
    return headers;
}

void PostmarkAPIClient::parseErrorResponse(const HTTPResponse& httpResponse, APIResponse& apiResponse) {
    apiResponse.success = false;
    
    try {
        Json::Value root;
        Json::Reader reader;
        if (reader.parse(httpResponse.body, root)) {
            if (root.isMember("Message")) {
                apiResponse.error_message = root["Message"].asString();
            } else if (root.isMember("ErrorCode")) {
                apiResponse.error_message = "Postmark Error " + std::to_string(root["ErrorCode"].asInt());
            } else {
                apiResponse.error_message = httpResponse.body;
            }
        } else {
            apiResponse.error_message = "HTTP " + std::to_string(httpResponse.status_code) + ": " + httpResponse.body;
        }
    } catch (...) {
        apiResponse.error_message = "HTTP " + std::to_string(httpResponse.status_code) + ": " + httpResponse.body;
    }
}

} // namespace ssmtp_mailer

