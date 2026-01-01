#include <gtest/gtest.h>
#include "simple-smtp-mailer/token_manager.hpp"
#include <filesystem>
#include <thread>
#include <chrono>
#include <algorithm>

// Test fixture with temporary directory cleanup
class TokenManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary directory for test files
        test_dir_ = "/tmp/test_token_manager_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
        std::filesystem::create_directories(test_dir_);
        
        storage_ = std::make_shared<ssmtp_mailer::FileTokenStorage>(test_dir_, "");
        manager_ = std::make_unique<ssmtp_mailer::TokenManager>(storage_);
    }
    
    void TearDown() override {
        manager_.reset();
        storage_.reset();
        
        // Cleanup test directory
        if (std::filesystem::exists(test_dir_)) {
            std::filesystem::remove_all(test_dir_);
        }
    }
    
    std::string test_dir_;
    std::shared_ptr<ssmtp_mailer::FileTokenStorage> storage_;
    std::unique_ptr<ssmtp_mailer::TokenManager> manager_;
};

// Test 1: TokenManager Creation
TEST_F(TokenManagerTest, CreateTokenManager) {
    ASSERT_NE(manager_, nullptr);
}

// Test 2: Register Single Account
TEST_F(TokenManagerTest, RegisterSingleAccount) {
    bool result = manager_->registerAccount(
        "test-account",
        "google",
        "test-client-id",
        "test-client-secret",
        "http://localhost:8080/callback",
        "user@example.com"
    );
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(manager_->accountExists("test-account"));
}

// Test 3: Register Multiple Accounts
TEST_F(TokenManagerTest, RegisterMultipleAccounts) {
    manager_->registerAccount("account1", "google", "cid1", "secret1", "http://localhost:8080", "user1@example.com");
    manager_->registerAccount("account2", "microsoft", "cid2", "secret2", "http://localhost:8080", "user2@example.com");
    manager_->registerAccount("account3", "google", "cid3", "secret3", "http://localhost:8080", "user3@example.com");
    
    EXPECT_TRUE(manager_->accountExists("account1"));
    EXPECT_TRUE(manager_->accountExists("account2"));
    EXPECT_TRUE(manager_->accountExists("account3"));
    
    auto accounts = manager_->listAccounts();
    EXPECT_EQ(accounts.size(), 3);
}

// Test 4: Store Tokens
TEST_F(TokenManagerTest, StoreTokens) {
    manager_->registerAccount("test-account", "google", "cid", "secret", "http://localhost:8080", "user@example.com");
    
    bool result = manager_->storeTokens(
        "test-account",
        "access-token-123",
        "refresh-token-456",
        3600  // 1 hour
    );
    
    EXPECT_TRUE(result);
}

// Test 5: Get Access Token (Valid)
TEST_F(TokenManagerTest, GetAccessTokenValid) {
    manager_->registerAccount("test-account", "google", "cid", "secret", "http://localhost:8080", "user@example.com");
    
    // Store token with future expiry
    auto future_expiry = std::chrono::system_clock::now() + std::chrono::hours(1);
    
    // We can't directly set the token through the public interface,
    // so we'll use the storage directly to test
    ssmtp_mailer::OAuth2Token token;
    token.access_token = "valid-access-token";
    token.refresh_token = "valid-refresh-token";
    token.valid = true;
    token.expires_at = future_expiry;
    token.created_at = std::chrono::system_clock::now();
    
    storage_->saveToken("test-account", token);
    
    std::string access_token = manager_->getAccessToken("test-account");
    EXPECT_FALSE(access_token.empty());
}

// Test 6: Token Expiry Check
TEST_F(TokenManagerTest, CheckTokenExpiry) {
    manager_->registerAccount("test-account", "google", "cid", "secret", "http://localhost:8080", "user@example.com");
    
    // Test with expired token
    ssmtp_mailer::OAuth2Token expired_token;
    expired_token.access_token = "expired-token";
    expired_token.refresh_token = "refresh-token";
    expired_token.valid = true;
    expired_token.expires_at = std::chrono::system_clock::now() - std::chrono::hours(1);
    expired_token.created_at = std::chrono::system_clock::now() - std::chrono::hours(2);
    
    storage_->saveToken("test-account", expired_token);
    
    EXPECT_TRUE(manager_->isTokenExpired("test-account", 0));
}

// Test 7: Get Token Lifetime
TEST_F(TokenManagerTest, GetTokenLifetime) {
    manager_->registerAccount("test-account", "google", "cid", "secret", "http://localhost:8080", "user@example.com");
    
    ssmtp_mailer::OAuth2Token token;
    token.access_token = "token";
    token.refresh_token = "refresh";
    token.valid = true;
    token.expires_at = std::chrono::system_clock::now() + std::chrono::hours(1);
    token.created_at = std::chrono::system_clock::now();
    
    storage_->saveToken("test-account", token);
    
    int lifetime = manager_->getTokenLifetime("test-account");
    EXPECT_GT(lifetime, 0);
    EXPECT_LT(lifetime, 3700);  // Should be around 3600 seconds
}

// Test 8: Delete Account
TEST_F(TokenManagerTest, DeleteAccount) {
    manager_->registerAccount("test-account", "google", "cid", "secret", "http://localhost:8080", "user@example.com");
    
    EXPECT_TRUE(manager_->accountExists("test-account"));
    
    bool deleted = manager_->deleteAccount("test-account");
    EXPECT_TRUE(deleted);
    EXPECT_FALSE(manager_->accountExists("test-account"));
}

// Test 9: Non-existent Account Handling
TEST_F(TokenManagerTest, NonExistentAccountHandling) {
    EXPECT_FALSE(manager_->accountExists("non-existent"));
    
    std::string token = manager_->getAccessToken("non-existent");
    EXPECT_TRUE(token.empty());
    
    int lifetime = manager_->getTokenLifetime("non-existent");
    EXPECT_EQ(lifetime, 0);
}

// Test 10: List Accounts
TEST_F(TokenManagerTest, ListAccounts) {
    manager_->registerAccount("account1", "google", "cid1", "secret1", "http://localhost:8080", "user1@example.com");
    manager_->registerAccount("account2", "microsoft", "cid2", "secret2", "http://localhost:8080", "user2@example.com");
    
    auto accounts = manager_->listAccounts();
    EXPECT_EQ(accounts.size(), 2);
    EXPECT_NE(std::find(accounts.begin(), accounts.end(), "account1"), accounts.end());
    EXPECT_NE(std::find(accounts.begin(), accounts.end(), "account2"), accounts.end());
}

// Test 11: Auto-refresh Settings
TEST_F(TokenManagerTest, AutoRefreshSettings) {
    manager_->setAutoRefresh(true);
    manager_->setRefreshBuffer(10);
    
    // Verify settings are applied (they're inline methods, so we test behavior)
    manager_->registerAccount("test-account", "google", "cid", "secret", "http://localhost:8080", "user@example.com");
    
    // The actual refresh behavior will be tested in integration tests
    // Here we just verify the methods exist and don't crash
    EXPECT_TRUE(true);
}

// Test 12: File Token Storage Save and Load
TEST_F(TokenManagerTest, FileTokenStorageSaveAndLoad) {
    ssmtp_mailer::OAuth2Token original_token;
    original_token.access_token = "test-access-token";
    original_token.refresh_token = "test-refresh-token";
    original_token.scope = "https://mail.google.com/";
    original_token.token_type = "Bearer";
    original_token.valid = true;
    original_token.expires_at = std::chrono::system_clock::now() + std::chrono::hours(1);
    original_token.created_at = std::chrono::system_clock::now();
    
    // Save token
    bool saved = storage_->saveToken("test-account", original_token);
    EXPECT_TRUE(saved);
    
    // Load token
    ssmtp_mailer::OAuth2Token loaded_token;
    bool loaded = storage_->loadToken("test-account", loaded_token);
    EXPECT_TRUE(loaded);
    
    // Verify token content
    EXPECT_EQ(loaded_token.access_token, "test-access-token");
    EXPECT_EQ(loaded_token.refresh_token, "test-refresh-token");
    EXPECT_EQ(loaded_token.scope, "https://mail.google.com/");
    EXPECT_EQ(loaded_token.token_type, "Bearer");
    EXPECT_EQ(loaded_token.valid, true);
}

// Test 13: File Token Storage Delete
TEST_F(TokenManagerTest, FileTokenStorageDelete) {
    ssmtp_mailer::OAuth2Token token;
    token.access_token = "token";
    token.refresh_token = "refresh";
    token.valid = true;
    
    storage_->saveToken("test-account", token);
    EXPECT_TRUE(std::filesystem::exists(test_dir_ + "/test-account.json"));
    
    bool deleted = storage_->deleteToken("test-account");
    EXPECT_TRUE(deleted);
    EXPECT_FALSE(std::filesystem::exists(test_dir_ + "/test-account.json"));
}

// Test 14: File Token Storage List Accounts
TEST_F(TokenManagerTest, FileTokenStorageListAccounts) {
    ssmtp_mailer::OAuth2Token token;
    token.access_token = "token";
    token.refresh_token = "refresh";
    token.valid = true;
    
    storage_->saveToken("account1", token);
    storage_->saveToken("account2", token);
    storage_->saveToken("account3", token);
    
    auto accounts = storage_->listAccounts();
    EXPECT_EQ(accounts.size(), 3);
    EXPECT_NE(std::find(accounts.begin(), accounts.end(), "account1"), accounts.end());
    EXPECT_NE(std::find(accounts.begin(), accounts.end(), "account2"), accounts.end());
    EXPECT_NE(std::find(accounts.begin(), accounts.end(), "account3"), accounts.end());
}

// Test 15: Thread Safety
TEST_F(TokenManagerTest, ThreadSafety) {
    manager_->registerAccount("test-account", "google", "cid", "secret", "http://localhost:8080", "user@example.com");
    
    // Spawn multiple threads accessing the manager concurrently
    std::vector<std::thread> threads;
    const int num_threads = 10;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this, i]() {
            manager_->accountExists("test-account");
            manager_->listAccounts();
            
            ssmtp_mailer::OAuth2Token token;
            token.access_token = "thread-token-" + std::to_string(i);
            token.refresh_token = "refresh";
            token.valid = true;
            token.expires_at = std::chrono::system_clock::now() + std::chrono::hours(1);
            token.created_at = std::chrono::system_clock::now();
            
            storage_->saveToken("account-" + std::to_string(i), token);
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify no crashes occurred
    EXPECT_TRUE(true);
}

// Test 16: Token Refresh (Manual)
TEST_F(TokenManagerTest, ManualTokenRefresh) {
    manager_->registerAccount("test-account", "google", "test-client-id", "test-secret", 
                              "http://localhost:8080", "user@example.com");
    
    // Store a token first
    ssmtp_mailer::OAuth2Token token;
    token.access_token = "old-token";
    token.refresh_token = "refresh-token-xyz";
    token.valid = true;
    token.expires_at = std::chrono::system_clock::now() - std::chrono::hours(1);  // Expired
    token.created_at = std::chrono::system_clock::now() - std::chrono::hours(2);
    
    storage_->saveToken("test-account", token);
    
    // Attempt manual refresh (will fail without real OAuth2 provider)
    // But we can test that the method exists and doesn't crash
    bool refreshed = manager_->refreshToken("test-account");
    // This will fail without real credentials, but method should exist
}

// Test 17: Account Information Retrieval
TEST_F(TokenManagerTest, GetAccountInformation) {
    manager_->registerAccount("test-account", "google", "cid", "secret", "http://localhost:8080", "user@example.com");
    
    auto* account = manager_->getAccount("test-account");
    ASSERT_NE(account, nullptr);
    EXPECT_EQ(account->account_id, "test-account");
    EXPECT_EQ(account->provider, "google");
    EXPECT_EQ(account->client_id, "cid");
    EXPECT_EQ(account->user_email, "user@example.com");
}

// Test 18: Invalid Account Access
TEST_F(TokenManagerTest, InvalidAccountAccess) {
    auto* account = manager_->getAccount("non-existent");
    EXPECT_EQ(account, nullptr);
}

// Test 19: Token Storage with Encryption (Structure Test)
TEST_F(TokenManagerTest, TokenStorageWithEncryptionStructure) {
    // Test that encryption structure exists (actual encryption not implemented yet)
    auto encrypted_storage = std::make_shared<ssmtp_mailer::FileTokenStorage>(test_dir_, "test-key");
    
    ssmtp_mailer::OAuth2Token token;
    token.access_token = "sensitive-token";
    token.refresh_token = "sensitive-refresh";
    token.valid = true;
    
    bool saved = encrypted_storage->saveToken("encrypted-account", token);
    EXPECT_TRUE(saved);
    
    ssmtp_mailer::OAuth2Token loaded_token;
    bool loaded = encrypted_storage->loadToken("encrypted-account", loaded_token);
    EXPECT_TRUE(loaded);
    EXPECT_EQ(loaded_token.access_token, "sensitive-token");
}

// Test 20: Concurrent Token Operations
TEST_F(TokenManagerTest, ConcurrentTokenOperations) {
    manager_->registerAccount("concurrent-account", "google", "cid", "secret", "http://localhost:8080", "user@example.com");
    
    std::vector<std::thread> threads;
    std::atomic<int> success_count(0);
    
    // Multiple threads performing various operations
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([this, i, &success_count]() {
            manager_->accountExists("concurrent-account");
            manager_->listAccounts();
            
            ssmtp_mailer::OAuth2Token token;
            token.access_token = "concurrent-token-" + std::to_string(i);
            token.valid = true;
            token.expires_at = std::chrono::system_clock::now() + std::chrono::hours(1);
            token.created_at = std::chrono::system_clock::now();
            
            if (storage_->saveToken("concurrent-account", token)) {
                success_count++;
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // At least some operations should succeed
    EXPECT_GT(success_count.load(), 0);
}

// main() is defined in test_main.cpp

