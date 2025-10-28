#pragma once

#include <string>
#include <map>
#include <memory>
#include <chrono>
#include <mutex>

namespace ssmtp_mailer {

/**
 * @brief OAuth2 Token Information
 */
struct OAuth2Token {
    std::string access_token;
    std::string refresh_token;
    std::string scope;
    std::string token_type;
    
    std::chrono::system_clock::time_point expires_at;
    std::chrono::system_clock::time_point created_at;
    
    bool valid; // Whether the token is currently valid
    
    OAuth2Token() : valid(false) {}
};

/**
 * @brief Token Account Information
 */
struct TokenAccount {
    std::string account_id;
    std::string provider;    // google, microsoft, etc.
    std::string client_id;
    std::string client_secret;
    std::string redirect_uri;
    std::string user_email;
    
    OAuth2Token token;
    
    TokenAccount() = default;
};

/**
 * @brief Token Storage Interface
 * Abstract interface for storing OAuth2 tokens
 */
class TokenStorage {
public:
    virtual ~TokenStorage() = default;
    
    /**
     * @brief Save token for an account
     * @param account_id Account identifier
     * @param token Token to save
     * @return true if saved successfully
     */
    virtual bool saveToken(const std::string& account_id, const OAuth2Token& token) = 0;
    
    /**
     * @brief Load token for an account
     * @param account_id Account identifier
     * @param token Token to load into
     * @return true if loaded successfully
     */
    virtual bool loadToken(const std::string& account_id, OAuth2Token& token) = 0;
    
    /**
     * @brief Delete token for an account
     * @param account_id Account identifier
     * @return true if deleted successfully
     */
    virtual bool deleteToken(const std::string& account_id) = 0;
    
    /**
     * @brief List all account IDs with stored tokens
     * @return Vector of account IDs
     */
    virtual std::vector<std::string> listAccounts() = 0;
};

/**
 * @brief File-based Token Storage
 * Stores tokens in encrypted JSON files
 */
class FileTokenStorage : public TokenStorage {
private:
    std::string storage_path_;
    std::string encryption_key_;
    std::mutex storage_mutex_;
    
public:
    FileTokenStorage(const std::string& storage_path, const std::string& encryption_key = "");
    
    bool saveToken(const std::string& account_id, const OAuth2Token& token) override;
    bool loadToken(const std::string& account_id, OAuth2Token& token) override;
    bool deleteToken(const std::string& account_id) override;
    std::vector<std::string> listAccounts() override;
    
private:
    std::string getFilePath(const std::string& account_id);
    std::string encryptToken(const std::string& plaintext);
    std::string decryptToken(const std::string& ciphertext);
};

/**
 * @brief Enhanced OAuth2 Token Manager
 * Handles token management, refresh, and storage
 */
class TokenManager {
private:
    std::shared_ptr<TokenStorage> storage_;
    std::map<std::string, TokenAccount> accounts_;
    mutable std::mutex accounts_mutex_;
    
    // Automatic token refresh settings
    bool auto_refresh_enabled_;
    int refresh_buffer_minutes_; // Refresh tokens before they expire
    
public:
    /**
     * @brief Constructor
     * @param storage Token storage implementation
     */
    explicit TokenManager(std::shared_ptr<TokenStorage> storage);
    
    /**
     * @brief Destructor
     */
    ~TokenManager();
    
    /**
     * @brief Register a new OAuth2 account
     * @param account_id Unique account identifier
     * @param provider OAuth2 provider (google, microsoft, etc.)
     * @param client_id OAuth2 client ID
     * @param client_secret OAuth2 client secret
     * @param redirect_uri OAuth2 redirect URI
     * @param user_email User email address
     * @return true if registered successfully
     */
    bool registerAccount(const std::string& account_id,
                        const std::string& provider,
                        const std::string& client_id,
                        const std::string& client_secret,
                        const std::string& redirect_uri,
                        const std::string& user_email);
    
    /**
     * @brief Get valid access token for an account (auto-refresh if needed)
     * @param account_id Account identifier
     * @return Access token or empty string if unavailable
     */
    std::string getAccessToken(const std::string& account_id);
    
    /**
     * @brief Manually refresh access token
     * @param account_id Account identifier
     * @return true if refreshed successfully
     */
    bool refreshToken(const std::string& account_id);
    
    /**
     * @brief Check if token is expired or about to expire
     * @param account_id Account identifier
     * @param buffer_minutes Buffer time in minutes
     * @return true if token needs refresh
     */
    bool isTokenExpired(const std::string& account_id, int buffer_minutes = 5) const;
    
    /**
     * @brief Store tokens after OAuth2 callback
     * @param account_id Account identifier
     * @param access_token Access token from provider
     * @param refresh_token Refresh token from provider
     * @param expires_in Seconds until token expires
     * @return true if stored successfully
     */
    bool storeTokens(const std::string& account_id,
                    const std::string& access_token,
                    const std::string& refresh_token,
                    int expires_in);
    
    /**
     * @brief Get account information
     * @param account_id Account identifier
     * @return Account information or nullptr
     */
    TokenAccount* getAccount(const std::string& account_id);
    
    /**
     * @brief List all registered accounts
     * @return Vector of account IDs
     */
    std::vector<std::string> listAccounts() const;
    
    /**
     * @brief Delete an account and its tokens
     * @param account_id Account identifier
     * @return true if deleted successfully
     */
    bool deleteAccount(const std::string& account_id);
    
    /**
     * @brief Enable/disable automatic token refresh
     * @param enabled Whether to enable auto-refresh
     */
    void setAutoRefresh(bool enabled) { auto_refresh_enabled_ = enabled; }
    
    /**
     * @brief Set token refresh buffer time
     * @param minutes Minutes before expiry to refresh
     */
    void setRefreshBuffer(int minutes) { refresh_buffer_minutes_ = minutes; }
    
    /**
     * @brief Get remaining token lifetime
     * @param account_id Account identifier
     * @return Seconds until token expires
     */
    int getTokenLifetime(const std::string& account_id) const;
    
    /**
     * @brief Check if an account exists
     * @param account_id Account identifier
     * @return true if account exists
     */
    bool accountExists(const std::string& account_id) const;

private:
    // Token refresh implementation
    std::string refreshGoogleToken(const TokenAccount& account);
    std::string refreshMicrosoftToken(const TokenAccount& account);
    
    // HTTP utilities for token refresh
    std::string performTokenRequest(const std::string& url, 
                                    const std::map<std::string, std::string>& params);
};

} // namespace ssmtp_mailer

