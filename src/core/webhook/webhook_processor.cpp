#include "simple-smtp-mailer/webhook_handler.hpp"
#include "simple-smtp-mailer/http_client.hpp"
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace ssmtp_mailer {

WebhookEventProcessor::WebhookEventProcessor(const WebhookConfig& config)
    : config_(config) {
    // Initialize statistics
    statistics_["total_events"] = 0;
    statistics_["delivered"] = 0;
    statistics_["bounced"] = 0;
    statistics_["opened"] = 0;
    statistics_["clicked"] = 0;
    statistics_["failed"] = 0;
}

void WebhookEventProcessor::registerCallback(WebhookEventType event_type, WebhookEventCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_[event_type].push_back(callback);
}

bool WebhookEventProcessor::processWebhook(const std::string& payload,
                                           const std::map<std::string, std::string>& headers,
                                           const std::string& provider) {
    try {
        // Verify signature if configured
        if (!config_.secret.empty()) {
            auto it = headers.find("X-Webhook-Signature");
            if (it != headers.end()) {
                // Get handler for the provider
                auto handler = WebhookHandlerFactory::createHandler(provider);
                if (!handler->verifySignature(payload, it->second, config_.secret)) {
                    std::cerr << "Webhook signature verification failed for provider: " << provider << std::endl;
                    statistics_["failed"]++;
                    return false;
                }
            }
        }

        // Get handler for the provider
        auto handler = WebhookHandlerFactory::createHandler(provider);
        
        // Process payload
        auto events = handler->processPayload(payload, headers, provider);
        
        // Update statistics
        for (const auto& event : events) {
            updateStatistics(event);
            
            // Trigger callbacks
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = callbacks_.find(event.type);
            if (it != callbacks_.end()) {
                for (const auto& callback : it->second) {
                    callback(event);
                }
            }
        }
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error processing webhook: " << e.what() << std::endl;
        statistics_["failed"]++;
        return false;
    }
}

std::map<std::string, int> WebhookEventProcessor::getStatistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return statistics_;
}

void WebhookEventProcessor::updateStatistics(const WebhookEvent& event) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    statistics_["total_events"]++;
    
    switch (event.type) {
        case WebhookEventType::DELIVERED:
            statistics_["delivered"]++;
            break;
        case WebhookEventType::BOUNCED:
            statistics_["bounced"]++;
            break;
        case WebhookEventType::OPENED:
            statistics_["opened"]++;
            break;
        case WebhookEventType::CLICKED:
            statistics_["clicked"]++;
            break;
        default:
            break;
    }
}

} // namespace ssmtp_mailer

