#include "simple-smtp-mailer/cli_commands.hpp"
#include "simple-smtp-mailer/config_utils.hpp"
#include "../config/config_manager.hpp"
#include "../logging/logger.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>
#include <ctime>
#include <iomanip>
#include <cstdlib>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

namespace ssmtp_mailer {

// Forward declarations for helper functions
namespace {
    void validateGlobalConfig(std::vector<std::string>& errors, 
                                   std::vector<std::string>& warnings, 
                                   bool verbose);
    void validateDomainConfigs(std::vector<std::string>& errors, 
                                    std::vector<std::string>& warnings, 
                                    bool verbose);
    void validateDomainConfig(const std::string& config_file, 
                                   const std::string& domain,
                                   std::vector<std::string>& errors, 
                                   std::vector<std::string>& warnings, 
                                   bool verbose);
    void validateUserConfigs(std::vector<std::string>& errors, 
                                  std::vector<std::string>& warnings, 
                                  bool verbose);
    void validateUserConfig(const std::string& config_file, 
                                 const std::string& user,
                                 std::vector<std::string>& errors, 
                                 std::vector<std::string>& warnings, 
                                 bool verbose);
    void validateOAuth2Configs(std::vector<std::string>& errors, 
                                    std::vector<std::string>& warnings, 
                                    bool verbose);
    void validateOAuth2Config(const std::string& config_file, 
                                   const std::string& domain,
                                   std::vector<std::string>& errors, 
                                   std::vector<std::string>& warnings, 
                                   bool verbose);
    void validateServiceAccountConfigs(std::vector<std::string>& errors, 
                                            std::vector<std::string>& warnings, 
                                            bool verbose);
    void validateServiceAccountConfig(const std::string& config_file, 
                                           const std::string& domain,
                                           std::vector<std::string>& errors, 
                                           std::vector<std::string>& warnings, 
                                           bool verbose);
    void validateTemplates(std::vector<std::string>& errors, 
                                std::vector<std::string>& warnings, 
                                bool verbose);
    void validateTemplate(const std::string& config_file, 
                               const std::string& template_name,
                               std::vector<std::string>& errors, 
                               std::vector<std::string>& warnings, 
                               bool verbose);
    void testSMTPConnections(const std::string& domain_filter);
    void testAPIConnections(const std::string& domain_filter);
}

// Validation command implementations

CLIResult ValidationCommands::validateConfig(const std::vector<std::string>& args) {
        bool fix = false;
        bool verbose = false;
        
        // Parse arguments
        for (const auto& arg : args) {
            if (arg == "--fix") {
                fix = true;
            } else if (arg == "--verbose" || arg == "-v") {
                verbose = true;
            }
        }
        
        std::vector<std::string> errors;
        std::vector<std::string> warnings;
        
        try {
            std::cout << "Configuration Validation Results\n";
            std::cout << "===============================\n\n";
            
            // Validate global configuration
            if (verbose) {
                std::cout << "Validating global configuration...\n";
            }
            validateGlobalConfig(errors, warnings, verbose);
            
            // Validate domain configurations
            if (verbose) {
                std::cout << "Validating domain configurations...\n";
            }
            validateDomainConfigs(errors, warnings, verbose);
            
            // Validate user configurations
            if (verbose) {
                std::cout << "Validating user configurations...\n";
            }
            validateUserConfigs(errors, warnings, verbose);
            
            // Validate OAuth2 configurations
            if (verbose) {
                std::cout << "Validating OAuth2 configurations...\n";
            }
            validateOAuth2Configs(errors, warnings, verbose);
            
            // Validate service account configurations
            if (verbose) {
                std::cout << "Validating service account configurations...\n";
            }
            validateServiceAccountConfigs(errors, warnings, verbose);
            
            // Validate templates
            if (verbose) {
                std::cout << "Validating templates...\n";
            }
            validateTemplates(errors, warnings, verbose);
            
            if (verbose) {
                std::cout << "\n";
            }
            
            if (warnings.empty() && errors.empty()) {
                std::cout << "✓ All configurations are valid!\n";
                return CLIResult::success_result();
            }
            
            if (!warnings.empty()) {
                std::cout << "Warnings (" << warnings.size() << "):\n";
                for (const auto& warning : warnings) {
                    std::cout << "  ⚠ " << warning << "\n";
                }
                std::cout << "\n";
            }
            
            if (!errors.empty()) {
                std::cout << "Errors (" << errors.size() << "):\n";
                for (const auto& error : errors) {
                    std::cout << "  ✗ " << error << "\n";
                }
                std::cout << "\n";
            }
            
            if (fix && !errors.empty()) {
                std::cout << "Auto-fix is not yet implemented. Please fix errors manually.\n";
            }
            
            return errors.empty() ? CLIResult::success_result() : CLIResult::error_result("Configuration validation failed");
            
        } catch (const std::exception& e) {
            return CLIResult::error_result("Validation failed: " + std::string(e.what()));
        }
    }
    
CLIResult ValidationCommands::testConnections(const std::vector<std::string>& args) {
        bool test_smtp = true;
        bool test_api = true;
        std::string domain_filter;
        
        // Parse arguments
        for (size_t i = 0; i < args.size(); ++i) {
            if (args[i] == "--smtp-only") {
                test_api = false;
            } else if (args[i] == "--api-only") {
                test_smtp = false;
            } else if (args[i] == "--domain" && i + 1 < args.size()) {
                domain_filter = args[++i];
            }
        }
        
        std::cout << "Connection Testing\n";
        std::cout << "==================\n\n";
        
        try {
            if (test_smtp) {
                testSMTPConnections(domain_filter);
            }
            
            if (test_api) {
                testAPIConnections(domain_filter);
            }
            
            return CLIResult::success_result();
            
        } catch (const std::exception& e) {
            return CLIResult::error_result("Connection testing failed: " + std::string(e.what()));
        }
    }
    
CLIResult ValidationCommands::backupConfig(const std::vector<std::string>& args) {
        std::string backup_file;
        
        // Parse arguments
        for (size_t i = 0; i < args.size(); ++i) {
            if (args[i] == "--file" && i + 1 < args.size()) {
                backup_file = args[++i];
            }
        }
        
        if (backup_file.empty()) {
            // Generate default backup filename
            auto now = std::time(nullptr);
            auto tm = *std::localtime(&now);
            std::ostringstream oss;
            oss << "simple-smtp-mailer-backup-" << std::put_time(&tm, "%Y%m%d-%H%M%S") << ".tar.gz";
            backup_file = oss.str();
        }
        
        try {
            std::string config_dir = ConfigUtils::getConfigDirectory();
            
            if (!std::filesystem::exists(config_dir)) {
                return CLIResult::error_result("Configuration directory not found: " + config_dir);
            }
            
            std::string backup_command = "tar -czf " + backup_file + " -C " + config_dir + " .";
            
            int result = std::system(backup_command.c_str());
            if (result != 0) {
                return CLIResult::error_result("Failed to create backup: " + backup_file);
            }
            
            std::cout << "Configuration backup created: " << backup_file << "\n";
            return CLIResult::success_result();
            
        } catch (const std::exception& e) {
            return CLIResult::error_result("Backup failed: " + std::string(e.what()));
        }
    }
    
CLIResult ValidationCommands::restoreConfig(const std::vector<std::string>& args) {
        if (args.empty()) {
            return CLIResult::error_result("Usage: config restore --file <backup-file>");
        }
        
        std::string backup_file;
        
        // Parse arguments
        for (size_t i = 0; i < args.size(); ++i) {
            if (args[i] == "--file" && i + 1 < args.size()) {
                backup_file = args[++i];
            }
        }
        
        if (backup_file.empty()) {
            return CLIResult::error_result("--file is required");
        }
        
        if (!std::filesystem::exists(backup_file)) {
            return CLIResult::error_result("Backup file not found: " + backup_file);
        }
        
        try {
            std::string config_dir = ConfigUtils::getConfigDirectory();
            
            // Ensure config directory exists
            if (!ConfigUtils::ensureConfigDirectory(config_dir)) {
                return CLIResult::error_result("Failed to create configuration directory: " + config_dir);
            }
            
            // Create backup of current config before restore
            auto now = std::time(nullptr);
            auto tm = *std::localtime(&now);
            std::ostringstream oss;
            oss << "simple-smtp-mailer-pre-restore-" << std::put_time(&tm, "%Y%m%d-%H%M%S") << ".tar.gz";
            std::string pre_restore_backup = oss.str();
            
            std::string backup_current = "tar -czf " + pre_restore_backup + " -C " + config_dir + " .";
            int result = std::system(backup_current.c_str());
            if (result != 0) {
                return CLIResult::error_result("Failed to create pre-restore backup");
            }
            
            // Restore from backup
            std::string restore_command = "tar -xzf " + backup_file + " -C " + config_dir;
            result = std::system(restore_command.c_str());
            if (result != 0) {
                return CLIResult::error_result("Failed to restore from backup: " + backup_file);
            }
            
            std::cout << "Configuration restored from: " << backup_file << "\n";
            std::cout << "Previous configuration backed up to: " << pre_restore_backup << "\n";
            return CLIResult::success_result();
            
        } catch (const std::exception& e) {
            return CLIResult::error_result("Restore failed: " + std::string(e.what()));
        }
    }

namespace {
    void validateGlobalConfig(std::vector<std::string>& errors, 
                                   std::vector<std::string>& warnings, 
                                   bool verbose) {
        std::string config_dir = ConfigUtils::getConfigDirectory();
        std::string config_file = config_dir + "/simple-smtp-mailer.conf";
        
        if (!std::filesystem::exists(config_file)) {
            warnings.push_back("Global configuration file not found: " + config_file + " (using defaults)");
            return;
        }
        
        std::ifstream file(config_file);
        if (!file.is_open()) {
            errors.push_back("Cannot read global configuration file: " + config_file);
            return;
        }
        
        // Basic validation - check for required sections
        std::string line;
        bool has_global_section = false;
        while (std::getline(file, line)) {
            if (line.find("[global]") != std::string::npos) {
                has_global_section = true;
                break;
            }
        }
        
        if (!has_global_section) {
            warnings.push_back("Global configuration missing [global] section");
        }
    }
    
    void validateDomainConfigs(std::vector<std::string>& errors, 
                                    std::vector<std::string>& warnings, 
                                    bool verbose) {
        std::string domains_dir = ConfigUtils::getDomainsDirectory();
        
        if (!std::filesystem::exists(domains_dir)) {
            if (verbose) {
                warnings.push_back("Domains directory not found: " + domains_dir);
            }
            return;
        }
        
        int domain_count = 0;
        for (const auto& entry : std::filesystem::directory_iterator(domains_dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".conf") {
                std::string domain = entry.path().stem().string();
                validateDomainConfig(entry.path().string(), domain, errors, warnings, verbose);
                domain_count++;
            }
        }
        
        if (domain_count == 0 && verbose) {
            warnings.push_back("No domain configurations found");
        }
    }
    
    void validateDomainConfig(const std::string& config_file, 
                                   const std::string& domain,
                                   std::vector<std::string>& errors, 
                                   std::vector<std::string>& warnings, 
                                   bool verbose) {
        std::ifstream file(config_file);
        if (!file.is_open()) {
            errors.push_back("Cannot read domain configuration: " + domain);
            return;
        }
        
        std::string line;
        bool has_smtp_server = false;
        bool has_smtp_port = false;
        bool has_auth_method = false;
        
        while (std::getline(file, line)) {
            if (line.find("smtp_server = ") == 0) {
                has_smtp_server = true;
            } else if (line.find("smtp_port = ") == 0) {
                has_smtp_port = true;
            } else if (line.find("auth_method = ") == 0) {
                has_auth_method = true;
            }
        }
        
        if (!has_smtp_server) {
            errors.push_back("Domain " + domain + ": Missing smtp_server");
        }
        if (!has_smtp_port) {
            errors.push_back("Domain " + domain + ": Missing smtp_port");
        }
        if (!has_auth_method) {
            warnings.push_back("Domain " + domain + ": Missing auth_method (defaulting to LOGIN)");
        }
    }
    
    void validateUserConfigs(std::vector<std::string>& errors, 
                                  std::vector<std::string>& warnings, 
                                  bool verbose) {
        std::string users_dir = ConfigUtils::getUsersDirectory();
        
        if (!std::filesystem::exists(users_dir)) {
            if (verbose) {
                warnings.push_back("Users directory not found: " + users_dir);
            }
            return;
        }
        
        int user_count = 0;
        for (const auto& entry : std::filesystem::directory_iterator(users_dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".conf") {
                std::string user = entry.path().stem().string();
                validateUserConfig(entry.path().string(), user, errors, warnings, verbose);
                user_count++;
            }
        }
        
        if (user_count == 0 && verbose) {
            warnings.push_back("No user configurations found");
        }
    }
    
    void validateUserConfig(const std::string& config_file, 
                                 const std::string& user,
                                 std::vector<std::string>& errors, 
                                 std::vector<std::string>& warnings, 
                                 bool verbose) {
        std::ifstream file(config_file);
        if (!file.is_open()) {
            errors.push_back("Cannot read user configuration: " + user);
            return;
        }
        
        std::string line;
        bool has_domain = false;
        
        while (std::getline(file, line)) {
            if (line.find("domain = ") == 0) {
                has_domain = true;
                break;
            }
        }
        
        if (!has_domain) {
            errors.push_back("User " + user + ": Missing domain");
        }
    }
    
    void validateOAuth2Configs(std::vector<std::string>& errors, 
                                    std::vector<std::string>& warnings, 
                                    bool verbose) {
        std::string config_dir = ConfigUtils::getConfigDirectory();
        std::string oauth2_dir = config_dir + "/oauth2";
        
        if (!std::filesystem::exists(oauth2_dir)) {
            if (verbose) {
                warnings.push_back("No OAuth2 configurations found");
            }
            return;
        }
        
        for (const auto& entry : std::filesystem::directory_iterator(oauth2_dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".conf") {
                std::string domain = entry.path().stem().string();
                validateOAuth2Config(entry.path().string(), domain, errors, warnings, verbose);
            }
        }
    }
    
    void validateOAuth2Config(const std::string& config_file, 
                                   const std::string& domain,
                                   std::vector<std::string>& errors, 
                                   std::vector<std::string>& warnings, 
                                   bool verbose) {
        std::ifstream file(config_file);
        if (!file.is_open()) {
            errors.push_back("Cannot read OAuth2 configuration: " + domain);
            return;
        }
        
        std::string line;
        bool has_client_id = false;
        bool has_client_secret = false;
        
        while (std::getline(file, line)) {
            if (line.find("client_id = ") == 0) {
                has_client_id = true;
            } else if (line.find("client_secret = ") == 0) {
                has_client_secret = true;
            }
        }
        
        if (!has_client_id) {
            errors.push_back("OAuth2 " + domain + ": Missing client_id");
        }
        if (!has_client_secret) {
            errors.push_back("OAuth2 " + domain + ": Missing client_secret");
        }
    }
    
    void validateServiceAccountConfigs(std::vector<std::string>& errors, 
                                            std::vector<std::string>& warnings, 
                                            bool verbose) {
        std::string config_dir = ConfigUtils::getConfigDirectory();
        std::string service_account_dir = config_dir + "/service-accounts";
        
        if (!std::filesystem::exists(service_account_dir)) {
            if (verbose) {
                warnings.push_back("No service account configurations found");
            }
            return;
        }
        
        for (const auto& entry : std::filesystem::directory_iterator(service_account_dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                std::string domain = entry.path().stem().string();
                validateServiceAccountConfig(entry.path().string(), domain, errors, warnings, verbose);
            }
        }
    }
    
    void validateServiceAccountConfig(const std::string& config_file, 
                                           const std::string& domain,
                                           std::vector<std::string>& errors, 
                                           std::vector<std::string>& warnings, 
                                           bool verbose) {
        std::ifstream file(config_file);
        if (!file.is_open()) {
            errors.push_back("Cannot read service account file: " + domain);
            return;
        }
        
        // Basic JSON validation - check for required fields
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        
        if (content.find("\"client_email\"") == std::string::npos) {
            errors.push_back("Service account " + domain + ": Missing client_email field");
        }
        if (content.find("\"private_key\"") == std::string::npos) {
            errors.push_back("Service account " + domain + ": Missing private_key field");
        }
    }
    
    void validateTemplates(std::vector<std::string>& errors, 
                                std::vector<std::string>& warnings, 
                                bool verbose) {
        std::string config_dir = ConfigUtils::getConfigDirectory();
        std::string templates_dir = config_dir + "/templates";
        
        if (!std::filesystem::exists(templates_dir)) {
            if (verbose) {
                warnings.push_back("No templates found");
            }
            return;
        }
        
        for (const auto& entry : std::filesystem::directory_iterator(templates_dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".conf") {
                std::string template_name = entry.path().stem().string();
                validateTemplate(entry.path().string(), template_name, errors, warnings, verbose);
            }
        }
    }
    
    void validateTemplate(const std::string& config_file, 
                               const std::string& template_name,
                               std::vector<std::string>& errors, 
                               std::vector<std::string>& warnings, 
                               bool verbose) {
        std::ifstream file(config_file);
        if (!file.is_open()) {
            errors.push_back("Cannot read template: " + template_name);
            return;
        }
        
        std::string line;
        bool has_subject = false;
        bool has_body = false;
        
        while (std::getline(file, line)) {
            if (line.find("subject = ") == 0) {
                has_subject = true;
            } else if (line.find("body = ") == 0) {
                has_body = true;
            }
        }
        
        if (!has_subject) {
            errors.push_back("Template " + template_name + ": Missing subject");
        }
        if (!has_body) {
            errors.push_back("Template " + template_name + ": Missing body");
        }
    }
    
    void testSMTPConnections(const std::string& domain_filter) {
        std::cout << "Testing SMTP Connections:\n";
        std::cout << "-------------------------\n";
        
        std::string domains_dir = ConfigUtils::getDomainsDirectory();
        
        if (!std::filesystem::exists(domains_dir)) {
            std::cout << "  No domains configured\n";
            return;
        }
        
        int tested = 0;
        int passed = 0;
        
        for (const auto& entry : std::filesystem::directory_iterator(domains_dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".conf") {
                std::string domain = entry.path().stem().string();
                
                if (!domain_filter.empty() && domain != domain_filter) {
                    continue;
                }
                
                tested++;
                
                // Read domain config to get SMTP server
                std::ifstream file(entry.path());
                std::string line;
                std::string smtp_server;
                std::string smtp_port;
                bool enabled = true;
                
                while (std::getline(file, line)) {
                    if (line.find("enabled = ") != std::string::npos) {
                        std::string val = line.substr(line.find("=") + 1);
                        val.erase(0, val.find_first_not_of(" \t"));
                        val.erase(val.find_last_not_of(" \t") + 1);
                        enabled = (val == "true");
                    } else if (line.find("smtp_server = ") != std::string::npos) {
                        smtp_server = line.substr(line.find("=") + 1);
                        smtp_server.erase(0, smtp_server.find_first_not_of(" \t"));
                        smtp_server.erase(smtp_server.find_last_not_of(" \t") + 1);
                    } else if (line.find("smtp_port = ") != std::string::npos) {
                        smtp_port = line.substr(line.find("=") + 1);
                        smtp_port.erase(0, smtp_port.find_first_not_of(" \t"));
                        smtp_port.erase(smtp_port.find_last_not_of(" \t") + 1);
                    }
                }
                
                if (!enabled) {
                    std::cout << "  " << domain << " - SKIPPED (disabled)\n";
                    continue;
                }
                
                if (smtp_server.empty()) {
                    std::cout << "  " << domain << " - FAILED (no SMTP server configured)\n";
                    continue;
                }
                
                // Note: Actual connection testing would require SMTP client implementation
                // For now, we just validate configuration
                std::cout << "  " << domain << " - CONFIGURED (" << smtp_server;
                if (!smtp_port.empty()) {
                    std::cout << ":" << smtp_port;
                }
                std::cout << ")\n";
                std::cout << "    Note: Actual connection test requires network access\n";
                passed++;
            }
        }
        
        if (tested == 0) {
            std::cout << "  No domains to test";
            if (!domain_filter.empty()) {
                std::cout << " (filter: " << domain_filter << ")";
            }
            std::cout << "\n";
        } else {
            std::cout << "\n  Tested: " << tested << ", Configured: " << passed << "\n";
        }
    }
    
    void testAPIConnections(const std::string& domain_filter) {
        std::cout << "\nTesting API Connections:\n";
        std::cout << "------------------------\n";
        
        std::string config_dir = ConfigUtils::getConfigDirectory();
        std::string api_config_file = config_dir + "/api-config.conf";
        
        if (!std::filesystem::exists(api_config_file)) {
            std::cout << "  No API providers configured\n";
            return;
        }
        
        std::ifstream file(api_config_file);
        std::string line;
        std::string current_provider;
        bool in_section = false;
        int tested = 0;
        int passed = 0;
        
        while (std::getline(file, line)) {
            if (line.find("[api:") == 0) {
                if (in_section && !current_provider.empty()) {
                    tested++;
                    std::cout << "  " << current_provider << " - CONFIGURED\n";
                    std::cout << "    Note: Actual connection test requires network access and valid credentials\n";
                    passed++;
                }
                
                size_t start = line.find(":") + 1;
                size_t end = line.find("]");
                if (end != std::string::npos) {
                    current_provider = line.substr(start, end - start);
                    in_section = true;
                }
            } else if (in_section && line.find("enabled = ") != std::string::npos) {
                std::string enabled = line.substr(line.find("=") + 1);
                enabled.erase(0, enabled.find_first_not_of(" \t"));
                enabled.erase(enabled.find_last_not_of(" \t") + 1);
                if (enabled != "true") {
                    in_section = false;
                    current_provider.clear();
                }
            } else if (in_section && (line.empty() || (line[0] == '[' && line.find("]") != std::string::npos))) {
                if (line[0] == '[') {
                    if (!current_provider.empty()) {
                        tested++;
                        std::cout << "  " << current_provider << " - CONFIGURED\n";
                        std::cout << "    Note: Actual connection test requires network access and valid credentials\n";
                        passed++;
                    }
                    in_section = false;
                    current_provider.clear();
                }
            }
        }
        
        if (in_section && !current_provider.empty()) {
            tested++;
            std::cout << "  " << current_provider << " - CONFIGURED\n";
            std::cout << "    Note: Actual connection test requires network access and valid credentials\n";
            passed++;
        }
        
        if (tested == 0) {
            std::cout << "  No API providers to test\n";
        } else {
            std::cout << "\n  Tested: " << tested << ", Configured: " << passed << "\n";
        }
    }
}

} // namespace ssmtp_mailer

#pragma GCC diagnostic pop
