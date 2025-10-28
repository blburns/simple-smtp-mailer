#include "simple-smtp-mailer/analytics_simple.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace ssmtp_mailer {

SimpleAnalyticsManager::SimpleAnalyticsManager()
    : total_emails_sent_(0)
    , total_emails_delivered_(0)
    , total_emails_failed_(0)
    , total_emails_bounced_(0) {
}

void SimpleAnalyticsManager::recordEmailSent(const std::string& provider, const std::string& message_id) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    total_emails_sent_++;
    
    // Update provider metrics
    if (provider_metrics_.find(provider) == provider_metrics_.end()) {
        provider_metrics_[provider] = ProviderMetrics();
        provider_metrics_[provider].provider_name = provider;
    }
    
    provider_metrics_[provider].total_sent++;
    
    // Calculate rates
    if (provider_metrics_[provider].total_sent > 0) {
        provider_metrics_[provider].delivery_rate = 
            (static_cast<double>(provider_metrics_[provider].total_delivered) / 
             provider_metrics_[provider].total_sent) * 100.0;
        provider_metrics_[provider].error_rate = 
            (static_cast<double>(provider_metrics_[provider].total_failed) / 
             provider_metrics_[provider].total_sent) * 100.0;
    }
}

void SimpleAnalyticsManager::recordEmailDelivered(const std::string& provider, const std::string& message_id) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    total_emails_delivered_++;
    
    if (provider_metrics_.find(provider) != provider_metrics_.end()) {
        provider_metrics_[provider].total_delivered++;
        
        if (provider_metrics_[provider].total_sent > 0) {
            provider_metrics_[provider].delivery_rate = 
                (static_cast<double>(provider_metrics_[provider].total_delivered) / 
                 provider_metrics_[provider].total_sent) * 100.0;
        }
    }
}

void SimpleAnalyticsManager::recordEmailFailed(const std::string& provider, const std::string& reason) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    total_emails_failed_++;
    
    if (provider_metrics_.find(provider) != provider_metrics_.end()) {
        provider_metrics_[provider].total_failed++;
        
        if (provider_metrics_[provider].total_sent > 0) {
            provider_metrics_[provider].error_rate = 
                (static_cast<double>(provider_metrics_[provider].total_failed) / 
                 provider_metrics_[provider].total_sent) * 100.0;
        }
    }
}

void SimpleAnalyticsManager::recordEmailBounced(const std::string& provider, const std::string& message_id) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    total_emails_bounced_++;
    
    if (provider_metrics_.find(provider) != provider_metrics_.end()) {
        provider_metrics_[provider].total_bounced++;
    }
}

void SimpleAnalyticsManager::recordAPICall(const std::string& provider, int response_code, int latency_ms) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    // Track latency
    provider_latencies_[provider].push_back(latency_ms);
    
    // Keep only last 100 latencies per provider to limit memory
    if (provider_latencies_[provider].size() > 100) {
        provider_latencies_[provider].erase(provider_latencies_[provider].begin());
    }
    
    // Calculate average latency
    if (provider_metrics_.find(provider) != provider_metrics_.end()) {
        const auto& latencies = provider_latencies_[provider];
        if (!latencies.empty()) {
            int sum = 0;
            for (int latency : latencies) {
                sum += latency;
            }
            provider_metrics_[provider].avg_response_time_ms = 
                static_cast<double>(sum) / latencies.size();
        }
    }
}

EmailMetrics SimpleAnalyticsManager::getOverallMetrics() const {
    EmailMetrics metrics;
    
    metrics.total_sent = total_emails_sent_.load();
    metrics.total_delivered = total_emails_delivered_.load();
    metrics.total_failed = total_emails_failed_.load();
    metrics.total_bounced = total_emails_bounced_.load();
    
    if (metrics.total_sent > 0) {
        metrics.delivery_rate = (static_cast<double>(metrics.total_delivered) / metrics.total_sent) * 100.0;
        metrics.error_rate = (static_cast<double>(metrics.total_failed) / metrics.total_sent) * 100.0;
        metrics.bounce_rate = (static_cast<double>(metrics.total_bounced) / metrics.total_sent) * 100.0;
    }
    
    return metrics;
}

std::vector<ProviderMetrics> SimpleAnalyticsManager::getProviderMetrics() const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    std::vector<ProviderMetrics> result;
    for (const auto& pair : provider_metrics_) {
        result.push_back(pair.second);
    }
    
    // Sort by total sent (descending)
    std::sort(result.begin(), result.end(), 
              [](const ProviderMetrics& a, const ProviderMetrics& b) {
                  return a.total_sent > b.total_sent;
              });
    
    return result;
}

ProviderMetrics SimpleAnalyticsManager::getProviderMetrics(const std::string& provider) const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    auto it = provider_metrics_.find(provider);
    if (it != provider_metrics_.end()) {
        return it->second;
    }
    
    return ProviderMetrics();
}

double SimpleAnalyticsManager::getDeliveryRate() const {
    size_t sent = total_emails_sent_.load();
    if (sent == 0) return 0.0;
    
    size_t delivered = total_emails_delivered_.load();
    return (static_cast<double>(delivered) / sent) * 100.0;
}

double SimpleAnalyticsManager::getErrorRate() const {
    size_t sent = total_emails_sent_.load();
    if (sent == 0) return 0.0;
    
    size_t failed = total_emails_failed_.load();
    return (static_cast<double>(failed) / sent) * 100.0;
}

double SimpleAnalyticsManager::getBounceRate() const {
    size_t sent = total_emails_sent_.load();
    if (sent == 0) return 0.0;
    
    size_t bounced = total_emails_bounced_.load();
    return (static_cast<double>(bounced) / sent) * 100.0;
}

void SimpleAnalyticsManager::reset() {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    total_emails_sent_ = 0;
    total_emails_delivered_ = 0;
    total_emails_failed_ = 0;
    total_emails_bounced_ = 0;
    
    provider_metrics_.clear();
    provider_latencies_.clear();
}

std::string SimpleAnalyticsManager::toJSON() const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    std::stringstream ss;
    ss << "{\n";
    ss << "  \"analytics\": {\n";
    ss << "    \"overall\": {\n";
    ss << "      \"total_sent\": " << total_emails_sent_.load() << ",\n";
    ss << "      \"total_delivered\": " << total_emails_delivered_.load() << ",\n";
    ss << "      \"total_failed\": " << total_emails_failed_.load() << ",\n";
    ss << "      \"total_bounced\": " << total_emails_bounced_.load() << ",\n";
    ss << "      \"delivery_rate\": " << std::fixed << std::setprecision(2) << getDeliveryRate() << ",\n";
    ss << "      \"error_rate\": " << std::setprecision(2) << getErrorRate() << ",\n";
    ss << "      \"bounce_rate\": " << std::setprecision(2) << getBounceRate() << "\n";
    ss << "    },\n";
    ss << "    \"providers\": [\n";
    
    auto providers = getProviderMetrics();
    for (size_t i = 0; i < providers.size(); ++i) {
        const auto& p = providers[i];
        ss << "      {\n";
        ss << "        \"name\": \"" << p.provider_name << "\",\n";
        ss << "        \"total_sent\": " << p.total_sent << ",\n";
        ss << "        \"total_delivered\": " << p.total_delivered << ",\n";
        ss << "        \"total_failed\": " << p.total_failed << ",\n";
        ss << "        \"delivery_rate\": " << std::setprecision(2) << p.delivery_rate << ",\n";
        ss << "        \"error_rate\": " << std::setprecision(2) << p.error_rate << ",\n";
        ss << "        \"avg_latency_ms\": " << std::setprecision(2) << p.avg_response_time_ms << "\n";
        ss << "      }";
        if (i < providers.size() - 1) {
            ss << ",";
        }
        ss << "\n";
    }
    
    ss << "    ]\n";
    ss << "  }\n";
    ss << "}";
    
    return ss.str();
}

std::string SimpleAnalyticsManager::toCSV() const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    std::stringstream ss;
    
    // Header
    ss << "Metric,Value\n";
    ss << "Total Emails Sent," << total_emails_sent_.load() << "\n";
    ss << "Total Emails Delivered," << total_emails_delivered_.load() << "\n";
    ss << "Total Emails Failed," << total_emails_failed_.load() << "\n";
    ss << "Total Emails Bounced," << total_emails_bounced_.load() << "\n";
    ss << "Delivery Rate," << std::fixed << std::setprecision(2) << getDeliveryRate() << "%\n";
    ss << "Error Rate," << std::setprecision(2) << getErrorRate() << "%\n";
    ss << "Bounce Rate," << std::setprecision(2) << getBounceRate() << "%\n\n";
    
    // Provider metrics
    ss << "Provider,Total Sent,Total Delivered,Total Failed,Delivery Rate,Error Rate,Avg Latency\n";
    auto providers = getProviderMetrics();
    for (const auto& p : providers) {
        ss << p.provider_name << "," 
           << p.total_sent << ","
           << p.total_delivered << ","
           << p.total_failed << ","
           << std::setprecision(2) << p.delivery_rate << ","
           << p.error_rate << ","
           << p.avg_response_time_ms << "\n";
    }
    
    return ss.str();
}

} // namespace ssmtp_mailer

