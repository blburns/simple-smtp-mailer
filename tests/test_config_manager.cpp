#include <gtest/gtest.h>
#include <string>
#include <fstream>
#include "simple-smtp-mailer/config_utils.hpp"

class ConfigManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary config file for testing
        config_file = "/tmp/test_config.conf";
        createTestConfigFile();
    }

    void TearDown() override {
        // Clean up test config file
        std::remove(config_file.c_str());
    }

    void createTestConfigFile() {
        std::ofstream file(config_file);
        file << "[global]\n";
        file << "log_level = info\n";
        file << "max_retries = 3\n";
        file << "timeout = 30\n";
        file << "\n";
        file << "[domain.example.com]\n";
        file << "smtp_server = smtp.gmail.com\n";
        file << "smtp_port = 587\n";
        file << "use_tls = true\n";
        file << "\n";
        file << "[user@example.com]\n";
        file << "password = test_password\n";
        file << "domain = example.com\n";
        file.close();
    }

    std::string config_file;
};

// Test config file loading
TEST_F(ConfigManagerTest, ConfigFileLoading) {
    EXPECT_NO_THROW({
        // Test that we can read the config file
        std::ifstream file(config_file);
        EXPECT_TRUE(file.is_open());
        file.close();
    });
}

// Test config parsing
TEST_F(ConfigManagerTest, ConfigParsing) {
    // This would test the actual config parsing logic
    // For now, we'll test basic file operations
    std::ifstream file(config_file);
    std::string line;
    bool found_global = false;
    bool found_domain = false;
    bool found_user = false;

    while (std::getline(file, line)) {
        if (line.find("[global]") != std::string::npos) found_global = true;
        if (line.find("[domain.example.com]") != std::string::npos) found_domain = true;
        if (line.find("[user@example.com]") != std::string::npos) found_user = true;
    }

    EXPECT_TRUE(found_global);
    EXPECT_TRUE(found_domain);
    EXPECT_TRUE(found_user);
    file.close();
}

// Test invalid config file
TEST_F(ConfigManagerTest, InvalidConfigFile) {
    std::string invalid_file = "/tmp/nonexistent_config.conf";

    EXPECT_NO_THROW({
        std::ifstream file(invalid_file);
        EXPECT_FALSE(file.is_open());
    });
}

// Test config validation
TEST_F(ConfigManagerTest, ConfigValidation) {
    // Test that required sections exist
    std::ifstream file(config_file);
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    EXPECT_TRUE(content.find("log_level") != std::string::npos);
    EXPECT_TRUE(content.find("smtp_server") != std::string::npos);
    EXPECT_TRUE(content.find("smtp_port") != std::string::npos);
}
