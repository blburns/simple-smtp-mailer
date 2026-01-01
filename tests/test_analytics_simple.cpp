#include <gtest/gtest.h>
#include "simple-smtp-mailer/analytics_simple.hpp"
#include <thread>
#include <chrono>
#include <algorithm>

class SimpleAnalyticsTest : public ::testing::Test {
protected:
    void SetUp() override {
        analytics_ = std::make_unique<ssmtp_mailer::SimpleAnalyticsManager>();
    }
    
    void TearDown() override {
        analytics_.reset();
    }
    
    std::unique_ptr<ssmtp_mailer::SimpleAnalyticsManager> analytics_;
};

// Test 1: Create Analytics Manager
TEST_F(SimpleAnalyticsTest, CreateAnalyticsManager) {
    ASSERT_NE(analytics_, nullptr);
}

// Test 2: Record Email Sent
TEST_F(SimpleAnalyticsTest, RecordEmailSent) {
    analytics_->recordEmailSent("sendgrid");
    analytics_->recordEmailSent("mailgun");
    analytics_->recordEmailSent("sendgrid");
    
    auto metrics = analytics_->getOverallMetrics();
    EXPECT_EQ(metrics.total_sent, 3);
}

// Test 3: Record Email Delivered
TEST_F(SimpleAnalyticsTest, RecordEmailDelivered) {
    analytics_->recordEmailSent("sendgrid");
    analytics_->recordEmailSent("sendgrid");
    analytics_->recordEmailDelivered("sendgrid");
    
    auto metrics = analytics_->getOverallMetrics();
    EXPECT_EQ(metrics.total_sent, 2);
    EXPECT_EQ(metrics.total_delivered, 1);
}

// Test 4: Record Email Failed
TEST_F(SimpleAnalyticsTest, RecordEmailFailed) {
    analytics_->recordEmailSent("mailgun");
    analytics_->recordEmailFailed("mailgun", "Connection timeout");
    
    auto metrics = analytics_->getOverallMetrics();
    EXPECT_EQ(metrics.total_sent, 1);
    EXPECT_EQ(metrics.total_failed, 1);
}

// Test 5: Record Email Bounced
TEST_F(SimpleAnalyticsTest, RecordEmailBounced) {
    analytics_->recordEmailSent("ses");
    analytics_->recordEmailBounced("ses");
    
    auto metrics = analytics_->getOverallMetrics();
    EXPECT_EQ(metrics.total_sent, 1);
    EXPECT_EQ(metrics.total_bounced, 1);
}

// Test 6: Calculate Delivery Rate
TEST_F(SimpleAnalyticsTest, CalculateDeliveryRate) {
    analytics_->recordEmailSent("sendgrid");
    analytics_->recordEmailSent("sendgrid");
    analytics_->recordEmailDelivered("sendgrid");
    analytics_->recordEmailDelivered("sendgrid");
    
    double rate = analytics_->getDeliveryRate();
    EXPECT_NEAR(rate, 100.0, 0.1);
}

// Test 7: Calculate Error Rate
TEST_F(SimpleAnalyticsTest, CalculateErrorRate) {
    for (int i = 0; i < 10; ++i) {
        analytics_->recordEmailSent("mailgun");
    }
    
    for (int i = 0; i < 2; ++i) {
        analytics_->recordEmailFailed("mailgun", "Error");
    }
    
    double rate = analytics_->getErrorRate();
    EXPECT_NEAR(rate, 20.0, 0.1);
}

// Test 8: Calculate Bounce Rate
TEST_F(SimpleAnalyticsTest, CalculateBounceRate) {
    for (int i = 0; i < 5; ++i) {
        analytics_->recordEmailSent("ses");
    }
    
    for (int i = 0; i < 1; ++i) {
        analytics_->recordEmailBounced("ses");
    }
    
    double rate = analytics_->getBounceRate();
    EXPECT_NEAR(rate, 20.0, 0.1);
}

// Test 9: Get Provider Metrics
TEST_F(SimpleAnalyticsTest, GetProviderMetrics) {
    analytics_->recordEmailSent("sendgrid");
    analytics_->recordEmailSent("mailgun");
    analytics_->recordEmailDelivered("sendgrid");
    
    auto providers = analytics_->getProviderMetrics();
    EXPECT_EQ(providers.size(), 2);
    
    // Find sendgrid
    auto sendgrid_it = std::find_if(providers.begin(), providers.end(),
                                   [](const ssmtp_mailer::ProviderMetrics& m) {
                                       return m.provider_name == "sendgrid";
                                   });
    ASSERT_NE(sendgrid_it, providers.end());
    EXPECT_EQ(sendgrid_it->total_sent, 1);
    EXPECT_EQ(sendgrid_it->total_delivered, 1);
}

// Test 10: Record API Call with Latency
TEST_F(SimpleAnalyticsTest, RecordAPICallWithLatency) {
    analytics_->recordAPICall("sendgrid", 200, 150);
    analytics_->recordAPICall("sendgrid", 200, 200);
    analytics_->recordAPICall("sendgrid", 200, 100);
    
    auto metrics = analytics_->getProviderMetrics("sendgrid");
    EXPECT_NEAR(metrics.avg_response_time_ms, 150.0, 10.0);
}

// Test 11: JSON Report Generation
TEST_F(SimpleAnalyticsTest, GenerateJSONReport) {
    analytics_->recordEmailSent("sendgrid");
    analytics_->recordEmailDelivered("sendgrid");
    
    std::string json = analytics_->toJSON();
    ASSERT_FALSE(json.empty());
    EXPECT_NE(json.find("analytics"), std::string::npos);
    EXPECT_NE(json.find("total_sent"), std::string::npos);
    EXPECT_NE(json.find("sendgrid"), std::string::npos);
}

// Test 12: CSV Report Generation
TEST_F(SimpleAnalyticsTest, GenerateCSVReport) {
    analytics_->recordEmailSent("mailgun");
    analytics_->recordEmailDelivered("mailgun");
    analytics_->recordEmailFailed("sendgrid", "Error");
    
    std::string csv = analytics_->toCSV();
    ASSERT_FALSE(csv.empty());
    EXPECT_NE(csv.find("Provider"), std::string::npos);
    EXPECT_NE(csv.find("mailgun"), std::string::npos);
    EXPECT_NE(csv.find("sendgrid"), std::string::npos);
}

// Test 13: Reset Metrics
TEST_F(SimpleAnalyticsTest, ResetMetrics) {
    analytics_->recordEmailSent("sendgrid");
    analytics_->recordEmailDelivered("sendgrid");
    
    auto metrics_before = analytics_->getOverallMetrics();
    EXPECT_GT(metrics_before.total_sent, 0);
    
    analytics_->reset();
    
    auto metrics_after = analytics_->getOverallMetrics();
    EXPECT_EQ(metrics_after.total_sent, 0);
    EXPECT_EQ(metrics_after.total_delivered, 0);
}

// Test 14: Thread Safety
TEST_F(SimpleAnalyticsTest, ThreadSafety) {
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([this, i]() {
            analytics_->recordEmailSent("provider-" + std::to_string(i % 3));
            analytics_->recordEmailDelivered("provider-" + std::to_string(i % 3));
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto metrics = analytics_->getOverallMetrics();
    EXPECT_EQ(metrics.total_sent, 10);
}

// Test 15: Multi-Provider Comparison
TEST_F(SimpleAnalyticsTest, MultiProviderComparison) {
    // SendGrid: 10 sent, 9 delivered
    for (int i = 0; i < 10; ++i) {
        analytics_->recordEmailSent("sendgrid");
    }
    for (int i = 0; i < 9; ++i) {
        analytics_->recordEmailDelivered("sendgrid");
    }
    analytics_->recordEmailFailed("sendgrid", "Failed");
    
    // Mailgun: 10 sent, 8 delivered
    for (int i = 0; i < 10; ++i) {
        analytics_->recordEmailSent("mailgun");
    }
    for (int i = 0; i < 8; ++i) {
        analytics_->recordEmailDelivered("mailgun");
    }
    analytics_->recordEmailFailed("mailgun", "Failed");
    analytics_->recordEmailFailed("mailgun", "Failed");
    
    auto providers = analytics_->getProviderMetrics();
    EXPECT_EQ(providers.size(), 2);
    
    // Check sorted order (by total sent)
    EXPECT_GE(providers[0].total_sent, providers[1].total_sent);
}

// Test 16: Provider Not Found
TEST_F(SimpleAnalyticsTest, ProviderNotFound) {
    auto metrics = analytics_->getProviderMetrics("nonexistent");
    EXPECT_EQ(metrics.provider_name, "");
    EXPECT_EQ(metrics.total_sent, 0);
}

// Test 17: Zero Division Handling
TEST_F(SimpleAnalyticsTest, ZeroDivisionHandling) {
    double delivery_rate = analytics_->getDeliveryRate();
    double error_rate = analytics_->getErrorRate();
    double bounce_rate = analytics_->getBounceRate();
    
    EXPECT_DOUBLE_EQ(delivery_rate, 0.0);
    EXPECT_DOUBLE_EQ(error_rate, 0.0);
    EXPECT_DOUBLE_EQ(bounce_rate, 0.0);
}

// Test 18: Record Multiple API Calls
TEST_F(SimpleAnalyticsTest, RecordMultipleAPICalls) {
    for (int i = 0; i < 50; ++i) {
        analytics_->recordAPICall("sendgrid", 200, 100 + (i % 50));
    }
    
    auto metrics = analytics_->getProviderMetrics("sendgrid");
    EXPECT_GT(metrics.avg_response_time_ms, 0.0);
    
    // Should average to around 124 (100 + average of 0-49)
    EXPECT_NEAR(metrics.avg_response_time_ms, 124.0, 10.0);
}

// Test 19: Complex Metrics Scenario
TEST_F(SimpleAnalyticsTest, ComplexMetricsScenario) {
    // Simulate a realistic scenario
    std::vector<std::string> providers = {"sendgrid", "mailgun", "ses"};
    
    for (int i = 0; i < 100; ++i) {
        std::string provider = providers[i % 3];
        analytics_->recordEmailSent(provider);
        
        // 90% delivery rate
        if (i % 10 != 0) {
            analytics_->recordEmailDelivered(provider);
        }
        
        // 5% bounce rate
        if (i % 20 == 0) {
            analytics_->recordEmailBounced(provider);
        }
        
        // Simulate API calls
        analytics_->recordAPICall(provider, 200, 100 + (i % 100));
    }
    
    auto overall = analytics_->getOverallMetrics();
    EXPECT_EQ(overall.total_sent, 100);
    EXPECT_NEAR(overall.delivery_rate, 90.0, 1.0);
    EXPECT_NEAR(overall.bounce_rate, 5.0, 1.0);
}

// Test 20: Latency Rolling Average
TEST_F(SimpleAnalyticsTest, LatencyRollingAverage) {
    // Record more than 100 calls to test rolling average
    for (int i = 0; i < 150; ++i) {
        analytics_->recordAPICall("test", 200, i);
    }
    
    auto metrics = analytics_->getProviderMetrics("test");
    // Should only include last 100 calls, averaging around 50 (0-99 average)
    EXPECT_NEAR(metrics.avg_response_time_ms, 49.5, 5.0);
}

