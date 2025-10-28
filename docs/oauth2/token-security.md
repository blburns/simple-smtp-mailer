# OAuth2 Token Security Guide

## Overview

This document outlines the security features and best practices for OAuth2 token management in simple-smtp-mailer.

## 🔒 Security Features

### 1. Encrypted Token Storage

- **File-based storage**: Tokens are stored in encrypted JSON files
- **Automatic encryption**: Access and refresh tokens are encrypted at rest
- **Secure key management**: Encryption keys are managed securely

### 2. Automatic Token Refresh

- **Proactive refresh**: Tokens are refreshed before expiry
- **Configurable buffer**: Default 5-minute buffer before token expiration
- **Background refresh**: Automatic refresh during token access
- **Failure handling**: Graceful handling of refresh failures

### 3. Thread-Safe Operations

- **Mutex protection**: All token operations are thread-safe
- **Atomic updates**: Token updates are atomic and consistent
- **Concurrent access**: Multiple threads can safely access tokens

### 4. Token Lifecycle Management

- **Expiry tracking**: Precise tracking of token expiration times
- **Automatic renewal**: Seamless token renewal without user intervention
- **Cleanup**: Automatic removal of expired tokens

## 🛡️ Security Best Practices

### Token Storage

1. **Use encrypted storage**: Always enable encryption for token storage
2. **Secure file permissions**: Set appropriate file permissions (600) for token files
3. **Separate storage**: Store tokens in a dedicated directory

### Token Handling

1. **Secure transmission**: Always use HTTPS for token exchange
2. **Token validation**: Validate tokens before use
3. **Token rotation**: Regularly rotate refresh tokens
4. **Audit logging**: Log all token operations for security auditing

### Access Control

1. **Minimum permissions**: Request only necessary scopes
2. **Regular review**: Periodically review granted permissions
3. **Revocation**: Immediately revoke tokens if compromised

## 🔐 Implementation Details

### Token Manager

The `TokenManager` class provides:
- Centralized token management
- Multi-account support
- Automatic token refresh
- Secure token storage

### Token Storage

The `FileTokenStorage` class implements:
- Encrypted token files
- JSON-based storage format
- Thread-safe operations
- Automatic cleanup

### Usage Example

```cpp
// Create token storage
auto storage = std::make_shared<FileTokenStorage>("/etc/smtp-mailer/tokens");

// Create token manager
TokenManager manager(storage);

// Register account
manager.registerAccount(
    "account-id",
    "google",
    client_id,
    client_secret,
    redirect_uri,
    "user@example.com"
);

// Get access token (auto-refreshes if needed)
std::string token = manager.getAccessToken("account-id");

// Manual refresh
manager.refreshToken("account-id");
```

## 📋 Security Considerations

### Token Storage Security

- **Encryption**: All tokens stored with encryption
- **File permissions**: Token files have restricted permissions
- **Backup**: Secure backup procedures for token storage

### Network Security

- **HTTPS only**: All OAuth2 flows use HTTPS
- **Certificate validation**: Proper certificate validation
- **Timeouts**: Network timeouts prevent hanging connections

### Error Handling

- **Graceful degradation**: Failures don't expose sensitive data
- **Error logging**: Security-relevant errors are logged
- **Failure recovery**: Automatic retry for transient failures

## 🚨 Security Incident Response

### If Tokens Are Compromised

1. **Immediately revoke**: Revoke tokens via provider console
2. **Regenerate**: Generate new OAuth2 credentials
3. **Remove storage**: Delete compromised token files
4. **Review logs**: Audit logs for unauthorized access
5. **Notify users**: Inform affected users if applicable

### Token Expiry

- **Access tokens**: Expire after 1 hour
- **Refresh tokens**: Long-lived but revocable
- **Automatic refresh**: Proactive refresh before expiry

## 📊 Token Monitoring

### Metrics Tracked

- Token refresh success rate
- Token expiry timing
- Failed authentication attempts
- Token storage operations

### Logging

All token operations are logged:
- Token acquisition
- Token refresh
- Token expiry
- Security errors

## 🔄 Migration and Updates

### Updating Token Storage

When updating the token storage format:
1. **Backup**: Always backup existing tokens
2. **Migration**: Provide migration path for existing tokens
3. **Rollback**: Maintain ability to rollback
4. **Testing**: Thoroughly test migration process

## ✅ Compliance

### Data Protection

- **GDPR compliance**: Token storage compliant with GDPR
- **Data minimization**: Store only necessary token information
- **Right to deletion**: Support token deletion on request

### Access Controls

- **Role-based access**: Implement appropriate access controls
- **Authentication**: Require authentication for token access
- **Authorization**: Enforce authorization checks

## 🔗 Related Documentation

- [OAuth2 Overview](oauth2-overview.md)
- [Token Management](token-management.md)
- [OAuth2 Security](oauth2-security.md)

