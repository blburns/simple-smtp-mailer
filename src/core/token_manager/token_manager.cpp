#include "simple-smtp-mailer/token_manager.hpp"
#include <curl/curl.h>
#include <json/json.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace ssmtp_mailer {

// CURL write callback
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Token Storage Implementation

FileTokenStorage::FileTokenStorage(const std::string& storage_path, const std::string& encryption_key)
    : storage_path_(storage_path), encryption_key_(encryption_key) {
    std::filesystem::create_directories(storage_path_);
}

bool FileTokenStorage::saveToken(const std::string& account_id, const OAuth2Token& token) {
    std::lock_guard<std::mutex> lock(storage_mutex_);
    
    try {
        Json::Value json;
        json["access_token"] = token.access_token;
        json["refresh_token"] = token.refresh_token;
        json["scope"] = token.scope;
        json["token_type"] = token.token_type;
        
        // Serialize timestamps
        auto time_to_string = [](const std::chrono::system_clock::time_point& tp) {
            return std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
                tp.time_since_epoch()).count());
        };
        
        json["expires_at"] = time_to_string(token.expires_at);
        json["created_at"] = time_to_string(token.created_at);
        json["valid"] = token.valid;
        
        std::string file_path = getFilePath(account_id);
        std::ofstream file(file_path);
        if (!file.is_open()) {
            return false;
        }
        
        Json::StreamWriterBuilder builder;
        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
        writer->write(json, &file);
        
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool FileTokenStorage::loadToken(const std::string& account_id, OAuth2Token& token) {
    std::lock_guard<std::mutex> lock(storage_mutex_);
    
    try {
        std::string file_path = getFilePath(account_id);
        if (!std::filesystem::exists(file_path)) {
            return false;
        }
        
        std::ifstream file(file_path);
        if (!file.is_open()) {
            return false;
        }
        
        Json::Value json;
        Json::Reader reader;
        if (!reader.parse(file, json)) {
            return false;
        }
        
        token.access_token = json.get("access_token", "").asString();
        token.refresh_token = json.get("refresh_token", "").asString();
        token.scope = json.get("scope", "").asString();
        token.token_type = json.get("token_type", "").asString();
        token.valid = json.get("valid", false).asBool();
        
        // Deserialize timestamps
        auto string_to_time = [](const std::string& s) {
            time_t seconds = std::stoll(s);
            return std::chrono::system_clock::from_time_t(seconds);
        };
        
        token.expires_at = string_to_time(json.get("expires_at", "0").asString());
        token.created_at = string_to_time(json.get("created_at", "0").asString());
        
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool FileTokenStorage::deleteToken(const std::string& account_id) {
    std::lock_guard<std::mutex> lock(storage_mutex_);
    
    std::string file_path = getFilePath(account_id);
    if (std::filesystem::exists(file_path)) {
        return std::filesystem::remove(file_path);
    }
    return true;
}

std::vector<std::string> FileTokenStorage::listAccounts() {
    std::lock_guard<std::mutex> lock(storage_mutex_);
    
    std::vector<std::string> accounts;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(storage_path_)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                accounts.push_back(entry.path().stem().string());
            }
        }
    } catch (const std::exception&) {
        // Ignore errors
    }
    
    return accounts;
}

std::string FileTokenStorage::getFilePath(const std::string& account_id) {
    return storage_path_ + "/" + account_id + ".json";
}

std::string FileTokenStorage::encryptToken(const std::string& plaintext) {
    // TODO: Implement actual encryption
    return plaintext;
}

std::string FileTokenStorage::decryptToken(const std::string& ciphertext) {
    // TODO: Implement actual decryption
    return ciphertext;
}

// Token Manager Implementation

TokenManager::TokenManager(std::shared_ptr<TokenStorage> storage)
    : storage_(std::move(storage))
    , auto_refresh_enabled_(true)
    , refresh_buffer_minutes_(5) {
}

TokenManager::~TokenManager() = default;

bool TokenManager::registerAccount(const std::string& account_id,
                                   const std::string& provider,
                                   const std::string& client_id,
                                   const std::string& client_secret,
                                   const std::string& redirect_uri,
                                   const std::string& user_email) {
    std::lock_guard<std::mutex> lock(accounts_mutex_);
    
    TokenAccount account;
    account.account_id = account_id;
    account.provider = provider;
    account.client_id = client_id;
    account.client_secret = client_secret;
    account.redirect_uri = redirect_uri;
    account.user_email = user_email;
    
    // Load existing tokens if available
    OAuth2Token token;
    if (storage_->loadToken(account_id, token)) {
        account.token = token;
    }
    
    accounts_[account_id] = account;
    return true;
}

std::string TokenManager::getAccessToken(const std::string& account_id) {
    std::lock_guard<std::mutex> lock(accounts_mutex_);
    
    auto it = accounts_.find(account_id);
    if (it == accounts_.end()) {
        return "";
    }
    
    TokenAccount& account = it->second;
    
    // Check if token needs refresh
    if (auto_refresh_enabled_ && isTokenExpired(account_id, refresh_buffer_minutes_)) {
        refreshToken(account_id);
    }
    
    if (account.token.valid && !isTokenExpired(account_id, 0)) {
        return account.token.access_token;
    }
    
    return "";
}

bool TokenManager::refreshToken(const std::string& account_id) {
    std::lock_guard<std::mutex> lock(accounts_mutex_);
    
    auto it = accounts_.find(account_id);
    if (it == accounts_.end()) {
        return false;
    }
    
    TokenAccount& account = it->second;
    std::string new_access_token;
    
    if (account.provider == "google") {
        new_access_token = refreshGoogleToken(account);
    } else if (account.provider == "microsoft") {
        new_access_token = refreshMicrosoftToken(account);
    } else {
        return false;
    }
    
    if (!new_access_token.empty()) {
        account.token.access_token = new_access_token;
        account.token.created_at = std::chrono::system_clock::now();
        account.token.expires_at = account.token.created_at + std::chrono::hours(1);
        account.token.valid = true;
        
        storage_->saveToken(account_id, account.token);
        return true;
    }
    
    return false;
}

bool TokenManager::isTokenExpired(const std::string& account_id, int buffer_minutes) const {
    auto it = accounts_.find(account_id);
    if (it == accounts_.end()) {
        return true;
    }
    
    const OAuth2Token& token = it->second.token;
    auto now = std::chrono::system_clock::now();
    auto expires = token.expires_at - std::chrono::minutes(buffer_minutes);
    
    return now >= expires;
}

bool TokenManager::storeTokens(const std::string& account_id,
                                const std::string& access_token,
                                const std::string& refresh_token,
                                int expires_in) {
    std::lock_guard<std::mutex> lock(accounts_mutex_);
    
    auto it = accounts_.find(account_id);
    if (it == accounts_.end()) {
        return false;
    }
    
    TokenAccount& account = it->second;
    
    account.token.access_token = access_token;
    account.token.refresh_token = refresh_token;
    account.token.created_at = std::chrono::system_clock::now();
    account.token.expires_at = account.token.created_at + std::chrono::seconds(expires_in);
    account.token.valid = true;
    
    return storage_->saveToken(account_id, account.token);
}

TokenAccount* TokenManager::getAccount(const std::string& account_id) {
    std::lock_guard<std::mutex> lock(accounts_mutex_);
    auto it = accounts_.find(account_id);
    return (it != accounts_.end()) ? &it->second : nullptr;
}

std::vector<std::string> TokenManager::listAccounts() const {
    std::lock_guard<std::mutex> lock(accounts_mutex_);
    
    std::vector<std::string> result;
    for (const auto& pair : accounts_) {
        result.push_back(pair.first);
    }
    
    return result;
}

bool TokenManager::deleteAccount(const std::string& account_id) {
    std::lock_guard<std::mutex> lock(accounts_mutex_);
    
    bool deleted = accounts_.erase(account_id) > 0;
    if (deleted) {
        storage_->deleteToken(account_id);
    }
    
    return deleted;
}

void TokenManager::setAutoRefresh(bool enabled) {
    auto_refresh_enabled_ = enabled;
}

void TokenManager::setRefreshBuffer(int minutes) {
    refresh_buffer_minutes_ = minutes;
}

int TokenManager::getTokenLifetime(const std::string& account_id) const {
    std::lock_guard<std::mutex> lock(accounts_mutex_);
    
    auto it = accounts_.find(account_id);
    if (it == accounts_.end()) {
        return 0;
    }
    
    const OAuth2Token& token = it->second.token;
    auto now = std::chrono::system_clock::now();
    auto duration = token.expires_at - now;
    
    return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
}

bool TokenManager::accountExists(const std::string& account_id) const {
    std::lock_guard<std::mutex> lock(accounts_mutex_);
    return accounts_.find(account_id) != accounts_.end();
}

std::string TokenManager::refreshGoogleToken(const TokenAccount& account) {
    std::map<std::string, std::string> params;
    params["grant_type"] = "refresh_token";
    params["client_id"] = account.client_id;
    params["client_secret"] = account.client_secret;
    params["refresh_token"] = account.token.refresh_token;
    
    return performTokenRequest("https://oauth2.googleapis.com/token", params);
}

std::string TokenManager::refreshMicrosoftToken(const TokenAccount& account) {
    std::map<std::string, std::string> params;
    params["grant_type"] = "refresh_token";
    params["client_id"] = account.client_id;
    params["client_secret"] = account.client_secret;
    params["refresh_token"] = account.token.refresh_token;
    
    return performTokenRequest("https://login.microsoftonline.com/common/oauth2/v2.0/token", params);
}

std::string TokenManager::performTokenRequest(const std::string& url,
                                               const std::map<std::string, std::string>& params) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return "";
    }
    
    // Build form data
    std::string post_data;
    for (const auto& pair : params) {
        if (!post_data.empty()) {
            post_data += "&";
        }
        
        char* escaped_key = curl_easy_escape(curl, pair.first.c_str(), pair.first.length());
        char* escaped_value = curl_easy_escape(curl, pair.second.c_str(), pair.second.length());
        
        post_data += std::string(escaped_key) + "=" + std::string(escaped_value);
        
        curl_free(escaped_key);
        curl_free(escaped_value);
    }
    
    std::string response;
    long http_code = 0;
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    
    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK || http_code != 200) {
        return "";
    }
    
    // Parse JSON response
    Json::Value json;
    Json::Reader reader;
    if (!reader.parse(response, json)) {
        return "";
    }
    
    if (json.isMember("error")) {
        return "";
    }
    
    return json.get("access_token", "").asString();
}

} // namespace ssmtp_mailer

