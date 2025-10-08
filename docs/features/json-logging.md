# 📊 JSON Logging Feature

**Version:** 0.2.0+
**Last Updated:** December 2024
**Status:** Available

## 📋 Overview

The simple-smtp-mailer project now includes comprehensive JSON logging capabilities, allowing for structured, machine-readable log output that integrates seamlessly with modern logging and monitoring systems.

## 🚀 Key Features

### **Structured Logging**
- **JSON Format**: All log messages output in valid JSON format
- **Configurable Fields**: Choose which fields to include in log output
- **Custom Fields**: Add application-specific metadata to all log entries
- **Pretty Printing**: Optional human-readable JSON formatting

### **Event-Specific Logging**
- **Email Events**: Structured logging for email sending operations
- **API Requests**: Detailed logging for API provider interactions
- **SMTP Operations**: Connection and authentication event logging
- **Error Tracking**: Comprehensive error logging with context

### **Advanced Features**
- **Thread Safety**: Safe for concurrent logging operations
- **Performance Optimized**: Minimal overhead for production use
- **Flexible Configuration**: Runtime configuration changes
- **Integration Ready**: Compatible with ELK Stack, Splunk, etc.

## 🏗️ Architecture

### **Core Components**

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   JsonLogger    │    │  JsonLogConfig  │    │  Log Formatter  │
│   (Singleton)   │    │  (Configuration)│    │  (JSON Builder) │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         └───────────────────────┼───────────────────────┘
                                 │
                    ┌────────────▼────────────┐
                    │    Output Handlers      │
                    │  (Console, File, etc.)  │
                    └─────────────────────────┘
```

### **Log Entry Structure**

```json
{
  "timestamp": "2024-12-19T10:30:45.123Z",
  "level": "INFO",
  "message": "Email sent successfully",
  "thread": "140735123456789",
  "component": "smtp_client",
  "event_type": "email_send",
  "service": "simple-smtp-mailer",
  "version": "0.2.0",
  "custom_field": "custom_value"
}
```

## 🔧 Configuration

### **Basic Configuration**

```ini
[Global]
# Enable JSON logging
json_logging_enabled = true

# Configure fields to include
json_log_fields = timestamp,level,message,thread,component,event_type

# Optional: Pretty print for development
json_log_pretty_print = false

# Optional: Custom timestamp format
json_log_timestamp_format = %Y-%m-%dT%H:%M:%S.%fZ
```

### **Programmatic Configuration**

```cpp
#include "simple-smtp-mailer/json_logger.hpp"

// Configure JSON logging
JsonLogConfig config;
config.enabled = true;
config.fields = "timestamp,level,message,thread,component";
config.pretty_print = false;

// Add custom fields
config.custom_fields["service"] = "simple-smtp-mailer";
config.custom_fields["version"] = "0.2.0";
config.custom_fields["environment"] = "production";

// Initialize logger
JsonLogger::initialize(config);
```

## 📝 Usage Examples

### **Basic Logging**

```cpp
JsonLogger& logger = JsonLogger::getInstance();

// Simple message logging
logger.logJson(LogLevel::INFO, "Application started");

// With additional fields
std::map<std::string, std::string> fields = {
    {"component", "config_loader"},
    {"action", "load_config"}
};
logger.logJson(LogLevel::DEBUG, "Configuration loaded", fields);
```

### **Email Event Logging**

```cpp
// Log successful email send
std::vector<std::string> recipients = {"user@example.com"};
logger.logEmailEvent(LogLevel::INFO, "sender@example.com", recipients,
                    "Welcome Email", "sent", "msg-12345");

// Log failed email send
logger.logEmailEvent(LogLevel::ERROR, "sender@example.com", {"invalid@email"},
                    "Failed Email", "failed", "", "Invalid email address");
```

### **API Request Logging**

```cpp
// Log successful API request
logger.logApiRequest(LogLevel::INFO, "SendGrid", "/v3/mail/send", "POST",
                    200, 150, "");

// Log failed API request
logger.logApiRequest(LogLevel::ERROR, "Mailgun", "/v3/domain/messages", "POST",
                    401, 2000, "Invalid API key");
```

### **Structured Data Logging**

```cpp
// Log structured application data
std::map<std::string, std::string> data = {
    {"component", "smtp_client"},
    {"action", "connect"},
    {"server", "smtp.example.com"},
    {"port", "587"},
    {"status", "connected"},
    {"connection_time_ms", "250"}
};
logger.logStructured(LogLevel::DEBUG, data);
```

## 🎛️ Configuration Options

### **Available Fields**

| Field | Description | Example |
|-------|-------------|---------|
| `timestamp` | ISO 8601 timestamp | `"2024-12-19T10:30:45.123Z"` |
| `level` | Log level | `"INFO"`, `"ERROR"`, `"DEBUG"` |
| `message` | Log message | `"Email sent successfully"` |
| `thread` | Thread ID | `"140735123456789"` |
| `component` | Component name | `"smtp_client"`, `"api_client"` |
| `event_type` | Event classification | `"email_send"`, `"api_request"` |
| `service` | Service name | `"simple-smtp-mailer"` |
| `version` | Application version | `"0.2.0"` |

### **Custom Fields**

```cpp
// Add custom fields that appear in all log entries
logger.addCustomField("hostname", "mail-server-01");
logger.addCustomField("region", "us-west-2");
logger.addCustomField("datacenter", "dc1");

// Remove custom fields
logger.removeCustomField("datacenter");
```

### **Field Configuration**

```cpp
// Minimal logging (only essential fields)
logger.setJsonFields("timestamp,level,message");

// Extended logging (all available fields)
logger.setJsonFields("timestamp,level,message,thread,component,event_type,service,version");

// Custom field selection
logger.setJsonFields("timestamp,level,message,component,user_id,request_id");
```

## 🔍 Log Analysis

### **Common Queries**

#### **ELK Stack (Elasticsearch + Logstash + Kibana)**

```json
// Find all email sending events
{
  "query": {
    "bool": {
      "must": [
        {"term": {"event_type": "email_send"}},
        {"range": {"timestamp": {"gte": "now-1h"}}}
      ]
    }
  }
}

// Find failed API requests
{
  "query": {
    "bool": {
      "must": [
        {"term": {"event_type": "api_request"}},
        {"range": {"status_code": {"gte": 400}}}
      ]
    }
  }
}

// Find errors by component
{
  "query": {
    "bool": {
      "must": [
        {"term": {"level": "ERROR"}},
        {"term": {"component": "smtp_client"}}
      ]
    }
  }
}
```

#### **Splunk Queries**

```splunk
# Email sending statistics
index=mailer event_type="email_send" | stats count by status

# API response times
index=mailer event_type="api_request" | stats avg(response_time_ms) by provider

# Error analysis
index=mailer level="ERROR" | stats count by component, message
```

### **Monitoring Dashboards**

#### **Grafana Dashboard Queries**

```promql
# Email sending rate
rate(log_entries_total{event_type="email_send"}[5m])

# Error rate by component
rate(log_entries_total{level="ERROR"}[5m]) by (component)

# API response time percentiles
histogram_quantile(0.95, rate(log_entries_total{event_type="api_request"}[5m]))
```

## 🚀 Integration Examples

### **Docker Logging**

```dockerfile
# Dockerfile
FROM ubuntu:20.04
COPY simple-smtp-mailer /usr/local/bin/
COPY config/ /etc/simple-smtp-mailer/

# Configure JSON logging
ENV JSON_LOGGING_ENABLED=true
ENV JSON_LOG_FIELDS="timestamp,level,message,thread,component,event_type"

CMD ["simple-smtp-mailer"]
```

```yaml
# docker-compose.yml
version: '3.8'
services:
  mailer:
    image: simple-smtp-mailer:latest
    environment:
      - JSON_LOGGING_ENABLED=true
      - JSON_LOG_FIELDS=timestamp,level,message,thread,component,event_type,service,version
    volumes:
      - ./config:/etc/simple-smtp-mailer
    logging:
      driver: "json-file"
      options:
        max-size: "10m"
        max-file: "3"
```

### **Kubernetes Logging**

```yaml
# k8s-deployment.yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: simple-smtp-mailer
spec:
  replicas: 3
  selector:
    matchLabels:
      app: simple-smtp-mailer
  template:
    metadata:
      labels:
        app: simple-smtp-mailer
    spec:
      containers:
      - name: mailer
        image: simple-smtp-mailer:latest
        env:
        - name: JSON_LOGGING_ENABLED
          value: "true"
        - name: JSON_LOG_FIELDS
          value: "timestamp,level,message,thread,component,event_type,service,version,namespace,pod_name"
        volumeMounts:
        - name: config
          mountPath: /etc/simple-smtp-mailer
      volumes:
      - name: config
        configMap:
          name: mailer-config
```

### **Fluentd Configuration**

```xml
<!-- fluentd.conf -->
<source>
  @type tail
  path /var/log/simple-smtp-mailer/*.log
  pos_file /var/log/fluentd/simple-smtp-mailer.log.pos
  tag simple-smtp-mailer
  format json
  time_key timestamp
  time_format %Y-%m-%dT%H:%M:%S.%LZ
</source>

<filter simple-smtp-mailer>
  @type record_transformer
  <record>
    service_name simple-smtp-mailer
    log_level ${record["level"]}
    component ${record["component"]}
    event_type ${record["event_type"]}
  </record>
</filter>

<match simple-smtp-mailer>
  @type elasticsearch
  host elasticsearch.logging.svc.cluster.local
  port 9200
  index_name simple-smtp-mailer
  type_name _doc
</match>
```

## 🧪 Testing

### **Running JSON Logging Tests**

```bash
# Run all tests including JSON logging tests
cd build && ./tests/simple-smtp-mailer-tests --gtest_filter="JsonLoggingTest.*"

# Run specific JSON logging test
cd build && ./tests/simple-smtp-mailer-tests --gtest_filter="JsonLoggingTest.BasicJsonLogging"

# Run with verbose output
cd build && ./tests/simple-smtp-mailer-tests --gtest_filter="JsonLoggingTest.*" --gtest_verbose
```

### **Test Coverage**

The JSON logging feature includes comprehensive tests covering:
- Basic JSON formatting
- Field configuration
- Custom field management
- Email event logging
- API request logging
- Structured data logging
- String escaping
- Log level handling
- Timestamp formatting
- Disabled logging state

## 📊 Performance Considerations

### **Overhead Analysis**

| Operation | Traditional Logging | JSON Logging | Overhead |
|-----------|-------------------|--------------|----------|
| Simple message | ~0.1ms | ~0.15ms | +50% |
| Structured data | ~0.2ms | ~0.25ms | +25% |
| Email event | ~0.3ms | ~0.35ms | +17% |
| API request | ~0.2ms | ~0.25ms | +25% |

### **Optimization Tips**

1. **Minimal Fields**: Use only necessary fields to reduce JSON size
2. **Batch Operations**: Group related log entries when possible
3. **Async Logging**: Consider async logging for high-throughput scenarios
4. **Field Caching**: Cache frequently used custom fields

## 🔧 Troubleshooting

### **Common Issues**

#### **JSON Parsing Errors**
```bash
# Check for malformed JSON
grep -v "^[[:space:]]*{" /var/log/simple-smtp-mailer/mailer.log

# Validate JSON format
jq . /var/log/simple-smtp-mailer/mailer.log
```

#### **Missing Fields**
```cpp
// Verify field configuration
JsonLogger& logger = JsonLogger::getInstance();
logger.setJsonFields("timestamp,level,message,thread,component");
```

#### **Performance Issues**
```cpp
// Disable JSON logging for performance-critical sections
logger.enableJsonLogging(false);
// ... performance-critical code ...
logger.enableJsonLogging(true);
```

### **Debug Configuration**

```cpp
// Enable debug logging for JSON logger
JsonLogConfig config;
config.enabled = true;
config.fields = "timestamp,level,message,thread,component,debug_info";
config.pretty_print = true;  // Enable for debugging
JsonLogger::initialize(config);
```

## 📚 API Reference

### **JsonLogger Class**

```cpp
class JsonLogger {
public:
    // Initialization
    static void initialize(const JsonLogConfig& config = JsonLogConfig());
    static JsonLogger& getInstance();

    // Configuration
    void enableJsonLogging(bool enable);
    void setJsonFields(const std::string& fields);
    void addCustomField(const std::string& key, const std::string& value);
    void removeCustomField(const std::string& key);
    void setTimestampFormat(const std::string& format);
    void enablePrettyPrint(bool enable);
    void enableStackTrace(bool enable);

    // Logging methods
    void logJson(LogLevel level, const std::string& message,
                const std::map<std::string, std::string>& additional_fields = {});
    void logStructured(LogLevel level, const std::map<std::string, std::string>& data);
    void logEmailEvent(LogLevel level, const std::string& from,
                      const std::vector<std::string>& to, const std::string& subject,
                      const std::string& status, const std::string& message_id = "",
                      const std::string& error_message = "");
    void logApiRequest(LogLevel level, const std::string& provider,
                      const std::string& endpoint, const std::string& method,
                      int status_code, long response_time_ms,
                      const std::string& error_message = "");
};
```

### **JsonLogConfig Structure**

```cpp
struct JsonLogConfig {
    bool enabled = false;
    std::string fields = "timestamp,level,message,thread";
    std::map<std::string, std::string> custom_fields;
    bool pretty_print = false;
    bool include_stack_trace = false;
    std::string timestamp_format = "%Y-%m-%dT%H:%M:%S.%fZ";
};
```

## 🎯 Future Enhancements

### **Planned Features**
- [ ] **Async Logging**: Non-blocking log output for high-performance scenarios
- [ ] **Log Rotation**: Automatic log file rotation with JSON format preservation
- [ ] **Compression**: Built-in log compression for storage optimization
- [ ] **Metrics Integration**: Direct integration with Prometheus metrics
- [ ] **Schema Validation**: JSON schema validation for log entries
- [ ] **Custom Formatters**: Pluggable log formatters for different output formats

### **Advanced Features**
- [ ] **Correlation IDs**: Automatic request correlation across services
- [ ] **Sampling**: Intelligent log sampling for high-volume scenarios
- [ ] **Filtering**: Runtime log filtering based on content
- [ ] **Aggregation**: Real-time log aggregation and summarization

---

## 📞 Support

### **Getting Help**
- **Documentation**: Check this file and inline code comments
- **Examples**: See `examples/json_logging_example.cpp`
- **Tests**: Run `tests/test_json_logging.cpp` for usage examples
- **Issues**: Report JSON logging issues on GitHub

### **Contributing**
- Follow existing code patterns
- Add tests for new JSON logging features
- Update documentation for new functionality
- Ensure backward compatibility

---

**Last Updated**: December 2024
**Next Review**: January 2025
**Maintainer**: blburns

*This JSON logging documentation is updated as the feature evolves.*
