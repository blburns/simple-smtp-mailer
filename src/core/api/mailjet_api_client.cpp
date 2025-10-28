#include "simple-smtp-mailer/api_client.hpp"
#include "simple-smtp-mailer/http_client.hpp"
#include "simple-smtp-mailer/mailer.hpp"
#include <sstream>
#include <json/json.h>
#include <iostream>

namespace ssmtp_mailer {

MailjetAPIClient::MailjetAPIClient(const APIClientConfig& config) 
    : config_(config) {
    // Set default Mailjet configuration if not provided
    if (config_.request.base_url.empty()) {
        config_.request.base_url = "https://api.mailjet.com";
    }
    if (config_.request.endpoint.empty()) {
        config_.request.endpoint = "/v3.1/send";
    }
}

APIResponse MailjetAPIClient::sendEmail(const Email& email) {
    APIResponse response;
    
    if (!isValid()) {
        response.error_message = "Invalid Mailjet API client configuration";
        return response;
    }
    
    try {
        // Build request body
        std::string requestBody = buildRequestBody(email);
        std::map<std::string, std::string> headers = buildHeaders();
        
        // Make HTTP request to Mailjet API
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
                if (root.isMember("Messages") && root["Messages"].isArray() && !root["Messages"].empty()) {
                    auto firstMessage = root["Messages"][0];
                    if (firstMessage.isMember("To") && firstMessage["To"].isArray() && !firstMessage["To"].empty()) {
                        auto firstTo = firstMessage["To"][0];
                        if (firstTo.isMember("MessageID")) {
                            response.message_id = firstTo["MessageID"].asString();
                        }
                    }
                }
            }
        } else {
            response.success = false;
            parseErrorResponse(httpResponse, response);
        }
        
    } catch (const std::exception& e) {
        response.success = false;
        response.error_message = "Exception in Mailjet API client: " + std::string(e.what());
    }
    
    return response;
}

std::vector<APIResponse> MailjetAPIClient::sendBatch(const std::vector<Email>& emails) {
    std::vector<APIResponse> responses;
    responses.reserve(emails.size());
    
    // Mailjet supports batch sending through their API
    for (const auto& email : emails) {
        responses.push_back(sendEmail(email));
    }
    
    return responses;
}

bool MailjetAPIClient::testConnection() {
    APIResponse response;
    try {
        auto httpClient = HTTPClientFactory::createClient();
        httpClient->setTimeout(config_.request.timeout_seconds);
        httpClient->setSSLVerification(config_.request.verify_ssl);
        
        HTTPRequest request;
        request.method = HTTPMethod::GET;
        request.url = config_.request.base_url + "/v3/REST/user";
        request.headers = buildHeaders();
        
        HTTPResponse httpResponse = httpClient->sendRequest(request);
        
        return (httpResponse.status_code >= 200 && httpResponse.status_code < 300);
    } catch (...) {
        return false;
    }
}

std::string MailjetAPIClient::getProviderName() const {
    return "Mailjet";
}

bool MailjetAPIClient::isValid() const {
    return !config_.auth.api_key.empty() && !config_.auth.api_secret.empty();
}

std::string MailjetAPIClient::buildRequestBody(const Email& email) {
    Json::Value root;
    Json::Value messages(Json::arrayValue);
    Json::Value message;
    
    // From
    if (!config_.sender_email.empty()) {
        message["From"]["Email"] = config_.sender_email;
    } else {
        message["From"]["Email"] = email.from;
    }
    if (!config_.sender_name.empty()) {
        message["From"]["Name"] = config_.sender_name;
    }
    
    // To
    Json::Value toArray(Json::arrayValue);
    for (const auto& to : email.to) {
        Json::Value toObj;
        toObj["Email"] = to;
        toArray.append(toObj);
    }
    message["To"] = toArray;
    
    // CC
    if (!email.cc.empty()) {
        Json::Value ccArray(Json::arrayValue);
        for (const auto& cc : email.cc) {
            Json::Value ccObj;
            ccObj["Email"] = cc;
            ccArray.append(ccObj);
        }
        message["Cc"] = ccArray;
    }
    
    // BCC
    if (!email.bcc.empty()) {
        Json::Value bccArray(Json::arrayValue);
        for (const auto& bcc : email.bcc) {
            Json::Value bccObj;
            bccObj["Email"] = bcc;
            bccArray.append(bccObj);
        }
        message["Bcc"] = bccArray;
    }
    
    // Subject and body
    message["Subject"] = email.subject;
    if (!email.html_body.empty()) {
        message["HTMLPart"] = email.html_body;
    }
    message["TextPart"] = email.body;
    
    // Attachments
    if (!email.attachments.empty()) {
        Json::Value attachmentsArray(Json::arrayValue);
        for (const auto& attachment : email.attachments) {
            Json::Value att;
            att["Filename"] = attachment;
            // att["ContentType"] = mime_type;
            // att["Base64Content"] = base64_encoded_content;
            attachmentsArray.append(att);
        }
        message["Attachments"] = attachmentsArray;
    }
    
    messages.append(message);
    root["Messages"] = messages;
    
    // Tracking
    if (config_.enable_tracking) {
        root["TrackOpens"] = "enabled";
        root["TrackClicks"] = "enabled";
    }
    
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    return Json::writeString(builder, root);
}

std::map<std::string, std::string> MailjetAPIClient::buildHeaders() {
    std::map<std::string, std::string> headers;
    
    // Mailjet requires Basic Authentication
    headers["Accept"] = "application/json";
    headers["Content-Type"] = "application/json";
    
    // Basic Authentication using API Key and Secret
    if (!config_.auth.api_key.empty() && !config_.auth.api_secret.empty()) {
        std::string credentials = config_.auth.api_key + ":" + config_.auth.api_secret;
        // In a real implementation, this would be base64 encoded
        headers["Authorization"] = "Basic " + credentials;  // Note: should be base64 encoded
    }
    
    return headers;
}

void MailjetAPIClient::parseErrorResponse(const HTTPResponse& httpResponse, APIResponse& apiResponse) {
    apiResponse.success = false;
    
    try {
        Json::Value root;
        Json::Reader reader;
        if (reader.parse(httpResponse.body, root)) {
            if (root.isMember("ErrorInfo")) {
                apiResponse.error_message = root["ErrorInfo"].asString();
            } else if (root.isMember("ErrorMessage")) {
                apiResponse.error_message = root["ErrorMessage"].asString();
            } else if (root.isMember("Messages") && root["Messages"].isArray()) {
                // First message may contain error info
                if (!root["Messages"].empty() && root["Messages"][0].isMember("Errors")) {
                    auto errors = root["Messages"][0]["Errors"];
                    if (errors.isArray() && !errors.empty()) {
                        apiResponse.error_message = errors[0]["ErrorMessage"].asString();
                    }
                } else {
                    apiResponse.error_message = "HTTP " + std::to_string(httpResponse.status_code);
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

