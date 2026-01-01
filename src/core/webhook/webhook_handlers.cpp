#include "simple-smtp-mailer/webhook_handler.hpp"
#include <json/json.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <iomanip>

namespace ssmtp_mailer {

// SendGrid Webhook Handler
std::vector<WebhookEvent> SendGridWebhookHandler::processPayload(
    const std::string& payload,
    const std::map<std::string, std::string>& headers,
    const std::string& provider) {
    
    std::vector<WebhookEvent> events;
    
    try {
        Json::Value root;
        Json::Reader reader;
        
        if (reader.parse(payload, root) && root.isArray()) {
            for (const auto& item : root) {
                WebhookEvent event = parseSendGridEvent(item.asString());
                event.provider = "SendGrid";
                events.push_back(event);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing SendGrid webhook: " << e.what() << std::endl;
    }
    
    return events;
}

bool SendGridWebhookHandler::verifySignature(const std::string& payload,
                                           const std::string& signature,
                                           const std::string& secret) {
    // SendGrid uses HMAC-SHA256 for signature verification
    unsigned char* digest = HMAC(EVP_sha256(), secret.c_str(), secret.length(),
                                 (unsigned char*)payload.c_str(), payload.length(), nullptr, nullptr);
    
    std::stringstream ss;
    for (int i = 0; i < 32; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    }
    
    return ss.str() == signature;
}

std::vector<std::string> SendGridWebhookHandler::getSupportedProviders() const {
    return {"SendGrid"};
}

bool SendGridWebhookHandler::isProviderSupported(const std::string& provider) const {
    return provider == "SendGrid" || provider == "sendgrid";
}

WebhookEvent SendGridWebhookHandler::parseSendGridEvent(const std::string& event_data) {
    WebhookEvent event;
    
    try {
        Json::Value root;
        Json::Reader reader;
        
        if (reader.parse(event_data, root)) {
            // Map SendGrid event type
            if (root.isMember("event")) {
                event.type = mapSendGridEventType(root["event"].asString());
            }
            
            if (root.isMember("sg_message_id")) {
                event.message_id = root["sg_message_id"].asString();
            }
            
            if (root.isMember("email")) {
                event.recipient = root["email"].asString();
            }
            
            if (root.isMember("timestamp")) {
                event.timestamp = root["timestamp"].asString();
            }
            
            if (root.isMember("reason")) {
                event.reason = root["reason"].asString();
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing SendGrid event: " << e.what() << std::endl;
    }
    
    return event;
}

WebhookEventType SendGridWebhookHandler::mapSendGridEventType(const std::string& event_type) {
    if (event_type == "delivered") return WebhookEventType::DELIVERED;
    if (event_type == "bounce") return WebhookEventType::BOUNCED;
    if (event_type == "dropped") return WebhookEventType::DROPPED;
    if (event_type == "open") return WebhookEventType::OPENED;
    if (event_type == "click") return WebhookEventType::CLICKED;
    if (event_type == "unsubscribe") return WebhookEventType::UNSUBSCRIBED;
    if (event_type == "spamreport") return WebhookEventType::SPAM_REPORT;
    if (event_type == "deferred") return WebhookEventType::DEFERRED;
    if (event_type == "processed") return WebhookEventType::PROCESSED;
    return WebhookEventType::UNKNOWN;
}

// Mailgun Webhook Handler
std::vector<WebhookEvent> MailgunWebhookHandler::processPayload(
    const std::string& payload,
    const std::map<std::string, std::string>& headers,
    const std::string& provider) {
    
    std::vector<WebhookEvent> events;
    
    try {
        // Mailgun typically sends form-encoded data
        Json::Value root;
        Json::Reader reader;
        
        if (reader.parse(payload, root)) {
            WebhookEvent event = parseMailgunEvent(std::map<std::string, std::string>());
            event.provider = "Mailgun";
            
            // Extract fields from JSON
            if (root.isMember("signature")) {
                event.message_id = root["signature"]["token"].asString();
            }
            
            if (root.isMember("event-data")) {
                if (root["event-data"].isMember("event")) {
                    event.type = mapMailgunEventType(root["event-data"]["event"].asString());
                }
                if (root["event-data"].isMember("recipient")) {
                    event.recipient = root["event-data"]["recipient"].asString();
                }
                if (root["event-data"].isMember("timestamp")) {
                    event.timestamp = root["event-data"]["timestamp"].asString();
                }
            }
            
            events.push_back(event);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing Mailgun webhook: " << e.what() << std::endl;
    }
    
    return events;
}

bool MailgunWebhookHandler::verifySignature(const std::string& payload,
                                           const std::string& signature,
                                           const std::string& secret) {
    // Similar HMAC verification
    return true; // Simplified for now
}

std::vector<std::string> MailgunWebhookHandler::getSupportedProviders() const {
    return {"Mailgun"};
}

bool MailgunWebhookHandler::isProviderSupported(const std::string& provider) const {
    return provider == "Mailgun" || provider == "mailgun";
}

WebhookEvent MailgunWebhookHandler::parseMailgunEvent(const std::map<std::string, std::string>& form_data) {
    WebhookEvent event;
    // Parse form data - implementation would extract fields
    return event;
}

WebhookEventType MailgunWebhookHandler::mapMailgunEventType(const std::string& event_type) {
    if (event_type == "delivered") return WebhookEventType::DELIVERED;
    if (event_type == "bounced" || event_type == "failed") return WebhookEventType::BOUNCED;
    if (event_type == "dropped") return WebhookEventType::DROPPED;
    if (event_type == "opened") return WebhookEventType::OPENED;
    if (event_type == "clicked") return WebhookEventType::CLICKED;
    if (event_type == "unsubscribed") return WebhookEventType::UNSUBSCRIBED;
    return WebhookEventType::UNKNOWN;
}

// Amazon SES Webhook Handler
std::vector<WebhookEvent> AmazonSESWebhookHandler::processPayload(
    const std::string& payload,
    const std::map<std::string, std::string>& headers,
    const std::string& provider) {
    
    std::vector<WebhookEvent> events;
    
    try {
        Json::Value root;
        Json::Reader reader;
        
        if (reader.parse(payload, root)) {
            WebhookEvent event = parseSESEvent(payload);
            event.provider = "Amazon SES";
            events.push_back(event);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing Amazon SES webhook: " << e.what() << std::endl;
    }
    
    return events;
}

bool AmazonSESWebhookHandler::verifySignature(const std::string& payload,
                                            const std::string& signature,
                                            const std::string& secret) {
    return true; // Simplified
}

std::vector<std::string> AmazonSESWebhookHandler::getSupportedProviders() const {
    return {"Amazon SES", "AmazonSES", "SES"};
}

bool AmazonSESWebhookHandler::isProviderSupported(const std::string& provider) const {
    return provider == "Amazon SES" || provider == "AmazonSES" || provider == "SES" || 
           provider == "amazon-ses" || provider == "ses";
}

WebhookEvent AmazonSESWebhookHandler::parseSESEvent(const std::string& event_data) {
    WebhookEvent event;
    
    try {
        Json::Value root;
        Json::Reader reader;
        
        if (reader.parse(event_data, root)) {
            if (root.isMember("Type")) {
                event.type = mapSESEventType(root["Type"].asString());
            }
            
            if (root.isMember("mail")) {
                if (root["mail"].isMember("messageId")) {
                    event.message_id = root["mail"]["messageId"].asString();
                }
                if (root["mail"].isMember("destination")) {
                    if (!root["mail"]["destination"].empty()) {
                        event.recipient = root["mail"]["destination"][0].asString();
                    }
                }
                if (root["mail"].isMember("source")) {
                    event.sender = root["mail"]["source"].asString();
                }
            }
            
            if (root.isMember("bounce")) {
                if (root["bounce"].isMember("bouncedRecipients")) {
                    if (!root["bounce"]["bouncedRecipients"].empty()) {
                        event.reason = root["bounce"]["bouncedRecipients"][0]["diagnosticCode"].asString();
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing Amazon SES event: " << e.what() << std::endl;
    }
    
    return event;
}

WebhookEventType AmazonSESWebhookHandler::mapSESEventType(const std::string& event_type) {
    if (event_type == "Delivery") return WebhookEventType::DELIVERED;
    if (event_type == "Bounce") return WebhookEventType::BOUNCED;
    if (event_type == "Complaint") return WebhookEventType::SPAM_REPORT;
    return WebhookEventType::UNKNOWN;
}

// Webhook Handler Factory
std::shared_ptr<WebhookHandler> WebhookHandlerFactory::createHandler(const std::string& provider) {
    std::string lower_provider = provider;
    std::transform(lower_provider.begin(), lower_provider.end(), lower_provider.begin(), ::tolower);
    
    if (lower_provider == "sendgrid") {
        return std::make_shared<SendGridWebhookHandler>();
    } else if (lower_provider == "mailgun") {
        return std::make_shared<MailgunWebhookHandler>();
    } else if (lower_provider == "amazon ses" || lower_provider == "ses" || lower_provider == "amazon-ses") {
        return std::make_shared<AmazonSESWebhookHandler>();
    } else {
        throw std::runtime_error("Unsupported webhook provider: " + provider);
    }
}

std::vector<std::string> WebhookHandlerFactory::getSupportedProviders() {
    return {"SendGrid", "Mailgun", "Amazon SES"};
}

bool WebhookHandlerFactory::isProviderSupported(const std::string& provider) {
    std::string lower_provider = provider;
    std::transform(lower_provider.begin(), lower_provider.end(), lower_provider.begin(), ::tolower);
    
    return lower_provider == "sendgrid" || lower_provider == "mailgun" || 
           lower_provider == "amazon ses" || lower_provider == "ses" || lower_provider == "amazon-ses";
}

} // namespace ssmtp_mailer

