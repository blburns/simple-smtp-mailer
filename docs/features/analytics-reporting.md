# Analytics & Reporting

## Overview

The simple-smtp-mailer includes a comprehensive analytics and reporting system that tracks email delivery metrics, performance data, and provider comparisons. This system provides real-time insights into your email infrastructure.

## 🔍 Features

### Email Metrics Tracking

- **Total Emails Sent**: Track total email volume
- **Total Emails Delivered**: Successful delivery count
- **Total Emails Failed**: Failed delivery count
- **Total Emails Bounced**: Bounced email count

### Rate Calculations

- **Delivery Rate**: Percentage of successfully delivered emails
- **Error Rate**: Percentage of failed emails
- **Bounce Rate**: Percentage of bounced emails

### Provider Performance

- **Per-Provider Metrics**: Individual statistics for each email provider
- **Average Response Time**: Latency tracking with rolling averages
- **Performance Comparison**: Side-by-side provider analysis

### Report Formats

- **JSON Reports**: Structured JSON output for integration
- **CSV Reports**: Comma-separated values for spreadsheet analysis

## 📊 Usage

### Basic Usage

```cpp
#include "simple-smtp-mailer/analytics_simple.hpp"

// Create analytics manager
ssmtp_mailer::SimpleAnalyticsManager analytics;

// Record email sent
analytics.recordEmailSent("sendgrid", "msg-123");

// Record email delivered
analytics.recordEmailDelivered("sendgrid", "msg-123");

// Record email failed
analytics.recordEmailFailed("mailgun", "Connection timeout");

// Record email bounced
analytics.recordEmailBounced("ses", "msg-456");
```

### Getting Metrics

```cpp
// Get overall metrics
auto metrics = analytics.getOverallMetrics();
std::cout << "Total Sent: " << metrics.total_sent << std::endl;
std::cout << "Delivery Rate: " << metrics.delivery_rate << "%" << std::endl;

// Get provider-specific metrics
auto providers = analytics.getProviderMetrics();
for (const auto& provider : providers) {
    std::cout << provider.provider_name 
              << ": " << provider.delivery_rate << "%" << std::endl;
}

// Get specific provider metrics
auto sendgrid_metrics = analytics.getProviderMetrics("sendgrid");
std::cout << "SendGrid: " << sendgrid_metrics.total_sent << " sent" << std::endl;
```

### Recording API Calls with Latency

```cpp
// Track API call performance
analytics.recordAPICall("sendgrid", 200, 150);  // 200 status, 150ms latency
analytics.recordAPICall("mailgun", 200, 200);  // 200 status, 200ms latency

// Get average latency for provider
auto metrics = analytics.getProviderMetrics("sendgrid");
std::cout << "Avg Latency: " << metrics.avg_response_time_ms << "ms" << std::endl;
```

### Generating Reports

```cpp
// Generate JSON report
std::string json_report = analytics.toJSON();
std::cout << json_report << std::endl;

// Generate CSV report
std::string csv_report = analytics.toCSV();
std::cout << csv_report << std::endl;
```

### Resetting Metrics

```cpp
// Reset all metrics
analytics.reset();
```

## 📈 JSON Report Format

```json
{
  "analytics": {
    "overall": {
      "total_sent": 1000,
      "total_delivered": 950,
      "total_failed": 30,
      "total_bounced": 20,
      "delivery_rate": 95.00,
      "error_rate": 3.00,
      "bounce_rate": 2.00
    },
    "providers": [
      {
        "name": "sendgrid",
        "total_sent": 600,
        "total_delivered": 570,
        "total_failed": 20,
        "delivery_rate": 95.00,
        "error_rate": 3.33,
        "avg_latency_ms": 145.50
      },
      {
        "name": "mailgun",
        "total_sent": 400,
        "total_delivered": 380,
        "total_failed": 10,
        "delivery_rate": 95.00,
        "error_rate": 2.50,
        "avg_latency_ms": 178.25
      }
    ]
  }
}
```

## 📊 CSV Report Format

```csv
Metric,Value
Total Emails Sent,1000
Total Emails Delivered,950
Total Emails Failed,30
Total Emails Bounced,20
Delivery Rate,95.00%
Error Rate,3.00%
Bounce Rate,2.00%

Provider,Total Sent,Total Delivered,Total Failed,Delivery Rate,Error Rate,Avg Latency
sendgrid,600,570,20,95.00,3.33,145.50
mailgun,400,380,10,95.00,2.50,178.25
```

## 🔧 Integration Example

### With Email Sending

```cpp
#include "simple-smtp-mailer/analytics_simple.hpp"
#include "simple-smtp-mailer/mailer.hpp"

class EmailService {
private:
    ssmtp_mailer::Mailer mailer_;
    ssmtp_mailer::SimpleAnalyticsManager analytics_;
    
public:
    void sendEmail(const Email& email) {
        auto start = std::chrono::steady_clock::now();
        
        try {
            // Send email
            std::string message_id = mailer_.send(email);
            
            auto end = std::chrono::steady_clock::now();
            auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            
            // Record success
            analytics_.recordEmailSent("sendgrid", message_id);
            analytics_.recordEmailDelivered("sendgrid", message_id);
            analytics_.recordAPICall("sendgrid", 200, latency.count());
            
        } catch (const std::exception& e) {
            // Record failure
            analytics_.recordEmailFailed("sendgrid", e.what());
        }
    }
};
```

### Daily Report Generation

```cpp
void generateDailyReport() {
    ssmtp_mailer::SimpleAnalyticsManager analytics;
    
    // Load metrics from storage (implement your own storage)
    // ... load metrics ...
    
    // Generate JSON report
    std::string json_report = analytics.toJSON();
    
    // Save to file
    std::ofstream file("daily_report.json");
    file << json_report;
    file.close();
    
    // Or send to monitoring system
    sendToMonitoringSystem(json_report);
}
```

## 🎯 Monitoring Provider Performance

### Compare Providers

```cpp
void compareProviderPerformance() {
    auto providers = analytics.getProviderMetrics();
    
    std::cout << "Provider Performance Comparison:\n";
    std::cout << "===============================\n";
    
    for (const auto& provider : providers) {
        std::cout << provider.provider_name << ":\n";
        std::cout << "  Total Sent: " << provider.total_sent << "\n";
        std::cout << "  Delivery Rate: " << provider.delivery_rate << "%\n";
        std::cout << "  Error Rate: " << provider.error_rate << "%\n";
        std::cout << "  Avg Latency: " << provider.avg_response_time_ms << "ms\n";
        std::cout << "\n";
    }
}
```

## 📊 Thread Safety

The `SimpleAnalyticsManager` is thread-safe and can be used concurrently:

```cpp
std::vector<std::thread> workers;

for (int i = 0; i < 10; ++i) {
    workers.emplace_back([&analytics, i]() {
        analytics.recordEmailSent("provider-" + std::to_string(i % 3));
        analytics.recordEmailDelivered("provider-" + std::to_string(i % 3));
    });
}

// Wait for all threads
for (auto& worker : workers) {
    worker.join();
}
```

## 🔍 Best Practices

### 1. Regular Report Generation

```cpp
// Generate reports periodically
void scheduledReportGeneration() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::hours(24));
        
        std::string report = analytics.toJSON();
        saveReport(report);
        
        // Reset or archive old metrics
        // analytics.reset();
    }
}
```

### 2. Track Detailed Metadata

```cpp
// Track additional context
void sendWithMetadata(const Email& email, const std::string& campaign_id) {
    auto start = std::chrono::steady_clock::now();
    
    try {
        std::string msg_id = mailer_.send(email);
        auto latency = calculateLatency(start);
        
        analytics.recordEmailSent("sendgrid", msg_id);
        analytics.recordEmailDelivered("sendgrid", msg_id);
        analytics.recordAPICall("sendgrid", 200, latency.count());
        
        // Store additional metadata elsewhere
        storeMetadata(msg_id, campaign_id);
        
    } catch (const std::exception& e) {
        analytics.recordEmailFailed("sendgrid", e.what());
    }
}
```

### 3. Error Analysis

```cpp
void analyzeErrors() {
    auto overall = analytics.getOverallMetrics();
    
    if (overall.error_rate > 5.0) {
        std::cerr << "WARNING: High error rate: " 
                  << overall.error_rate << "%" << std::endl;
        
        // Send alert
        sendAlert("High error rate detected");
    }
}
```

## 🚀 Advanced Usage

### Custom Metrics Implementation

```cpp
class AdvancedAnalytics : public ssmtp_mailer::SimpleAnalyticsManager {
public:
    void trackCampaignPerformance(const std::string& campaign_id) {
        // Custom tracking logic
    }
    
    void generateCustomReport() {
        // Custom report generation
    }
};
```

## 📚 Related Documentation

- [OAuth2 Token Management](../oauth2/token-management.md)
- [Webhook Processing](./webhooks.md)
- [API Client Usage](./api-clients.md)

