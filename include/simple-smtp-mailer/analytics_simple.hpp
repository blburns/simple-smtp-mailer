#pragma once

#include <string>
#include <map>
#include <vector>
#include <chrono>
#include <mutex>
#include <atomic>

namespace ssmtp_mailer {

/**
 * @brief Simple email metrics (avoiding atomic copy issues)
 */
struct EmailMetrics {
    size_t total_sent;
    size_t total_delivered;
    size_t total_bounced;
    size_t total_failed;
    double delivery_rate;
    double error_rate;
    double bounce_rate;
    
    EmailMetrics() 
        : total_sent(0), total_delivered(0), total_bounced(0), 
          total_failed(0), delivery_rate(0.0), error_rate(0.0), bounce_rate(0.0) {}
};

/**
 * @brief Provider-specific metrics
 */
struct ProviderMetrics {
    std::string provider_name;
    size_t total_sent;
    size_t total_delivered;
    size_t total_failed;
    size_t total_bounced;
    double delivery_rate;
    double error_rate;
    double avg_response_time_ms;
    
    ProviderMetrics() 
        : total_sent(0), total_delivered(0), total_failed(0), total_bounced(0),
          delivery_rate(0.0), error_rate(0.0), avg_response_time_ms(0.0) {}
};

/**
 * @brief Simple Analytics Manager
 * Tracks email metrics without complex atomic structures
 */
class SimpleAnalyticsManager {
private:
    mutable std::mutex metrics_mutex_;
    
    // Overall metrics (using atomic for thread safety)
    std::atomic<size_t> total_emails_sent_;
    std::atomic<size_t> total_emails_delivered_;
    std::atomic<size_t> total_emails_failed_;
    std::atomic<size_t> total_emails_bounced_;
    
    // Provider-specific metrics
    std::map<std::string, ProviderMetrics> provider_metrics_;
    
    // Timing data
    std::map<std::string, std::vector<int>> provider_latencies_; // Millisecond latencies
    
public:
    SimpleAnalyticsManager();
    ~SimpleAnalyticsManager() = default;
    
    /**
     * @brief Record email sent
     */
    void recordEmailSent(const std::string& provider, const std::string& message_id = "");
    
    /**
     * @brief Record email delivered
     */
    void recordEmailDelivered(const std::string& provider, const std::string& message_id = "");
    
    /**
     * @brief Record email failed
     */
    void recordEmailFailed(const std::string& provider, const std::string& reason = "");
    
    /**
     * @brief Record email bounced
     */
    void recordEmailBounced(const std::string& provider, const std::string& message_id = "");
    
    /**
     * @brief Record API call with latency
     */
    void recordAPICall(const std::string& provider, int response_code, int latency_ms);
    
    /**
     * @brief Get overall email metrics
     */
    EmailMetrics getOverallMetrics() const;
    
    /**
     * @brief Get provider metrics
     */
    std::vector<ProviderMetrics> getProviderMetrics() const;
    
    /**
     * @brief Get metrics for specific provider
     */
    ProviderMetrics getProviderMetrics(const std::string& provider) const;
    
    /**
     * @brief Get delivery rate percentage
     */
    double getDeliveryRate() const;
    
    /**
     * @brief Get error rate percentage
     */
    double getErrorRate() const;
    
    /**
     * @brief Get bounce rate percentage
     */
    double getBounceRate() const;
    
    /**
     * @brief Reset all metrics
     */
    void reset();
    
    /**
     * @brief Get metrics as JSON string
     */
    std::string toJSON() const;
    
    /**
     * @brief Get metrics as CSV string
     */
    std::string toCSV() const;
};

} // namespace ssmtp_mailer

