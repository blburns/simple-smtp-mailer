#include "simple-smtp-mailer/cli_commands.hpp"
#include "simple-smtp-mailer/config_utils.hpp"
#include "../logging/logger.hpp"
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <cctype>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

namespace ssmtp_mailer {

// API command implementations

CLIResult APICommands::addProvider(const std::vector<std::string>& args) {
    if (args.empty()) {
        return CLIResult::error_result("Usage: api provider add <provider> --api-key <key> --sender <email> [options]");
    }
    
    std::string provider_name = args[0];
    std::string api_key;
    std::string api_secret;
    std::string sender_email;
    std::string sender_name;
    std::string base_url;
    std::string endpoint;
    std::string region;
    std::string domain;
    int timeout = 30;
    bool verify_ssl = true;
    bool enable_tracking = false;
    
    // Parse arguments
    for (size_t i = 1; i < args.size(); ++i) {
        if (args[i] == "--api-key" && i + 1 < args.size()) {
            api_key = args[++i];
        } else if (args[i] == "--api-secret" && i + 1 < args.size()) {
            api_secret = args[++i];
        } else if (args[i] == "--sender" && i + 1 < args.size()) {
            sender_email = args[++i];
        } else if (args[i] == "--sender-name" && i + 1 < args.size()) {
            sender_name = args[++i];
        } else if (args[i] == "--base-url" && i + 1 < args.size()) {
            base_url = args[++i];
        } else if (args[i] == "--endpoint" && i + 1 < args.size()) {
            endpoint = args[++i];
        } else if (args[i] == "--region" && i + 1 < args.size()) {
            region = args[++i];
        } else if (args[i] == "--domain" && i + 1 < args.size()) {
            domain = args[++i];
        } else if (args[i] == "--timeout" && i + 1 < args.size()) {
            timeout = std::stoi(args[++i]);
        } else if (args[i] == "--no-verify-ssl") {
            verify_ssl = false;
        } else if (args[i] == "--enable-tracking") {
            enable_tracking = true;
        }
    }
    
    if (api_key.empty()) {
        return CLIResult::error_result("--api-key is required");
    }
    
    if (sender_email.empty()) {
        return CLIResult::error_result("--sender is required");
    }
    
    // Map provider name to provider type
    std::string provider_type = provider_name;
    std::transform(provider_type.begin(), provider_type.end(), provider_type.begin(), ::toupper);
    
    // Set default base URLs and endpoints for known providers
    if (base_url.empty()) {
        if (provider_type == "SENDGRID") {
            base_url = "https://api.sendgrid.com";
            endpoint = "/v3/mail/send";
        } else if (provider_type == "MAILGUN") {
            base_url = "https://api.mailgun.net/v3";
            if (domain.empty()) {
                return CLIResult::error_result("--domain is required for Mailgun");
            }
            endpoint = "/" + domain + "/messages";
        } else if (provider_type == "AMAZON_SES" || provider_type == "AMAZON-SES" || provider_type == "SES") {
            base_url = "https://email.us-east-1.amazonaws.com";
            endpoint = "/v2/email";
            if (region.empty()) {
                region = "us-east-1";
            }
        } else if (provider_type == "POSTMARK") {
            base_url = "https://api.postmarkapp.com";
            endpoint = "/email";
        } else if (provider_type == "SPARKPOST") {
            base_url = "https://api.sparkpost.com/api/v1";
            endpoint = "/transmissions";
        } else if (provider_type == "MAILJET") {
            base_url = "https://api.mailjet.com/v3.1";
            endpoint = "/send";
        }
    }
    
    // Get API config file path
    std::string config_dir = ConfigUtils::getConfigDirectory();
    std::string api_config_file = config_dir + "/api-config.conf";
    
    try {
        // Ensure directory exists
        if (!ConfigUtils::ensureConfigDirectory(config_dir)) {
            return CLIResult::error_result("Failed to create configuration directory: " + config_dir);
        }
        
        // Read existing config if it exists
        std::map<std::string, std::string> existing_config;
        std::vector<std::string> config_lines;
        
        if (std::filesystem::exists(api_config_file)) {
            std::ifstream in_file(api_config_file);
            std::string line;
            while (std::getline(in_file, line)) {
                config_lines.push_back(line);
            }
            in_file.close();
        }
        
        // Check if provider already exists
        bool provider_exists = false;
        size_t section_start = 0;
        size_t section_end = 0;
        
        for (size_t i = 0; i < config_lines.size(); ++i) {
            if (config_lines[i].find("[api:" + provider_name + "]") != std::string::npos) {
                provider_exists = true;
                section_start = i;
                // Find end of section
                for (size_t j = i + 1; j < config_lines.size(); ++j) {
                    if (config_lines[j].find("[") == 0 && config_lines[j].find("]") != std::string::npos) {
                        section_end = j;
                        break;
                    }
                    section_end = j + 1;
                }
                break;
            }
        }
        
        // Create configuration content
        std::ostringstream config_content;
        config_content << "[api:" << provider_name << "]\n";
        config_content << "# " << provider_name << " API configuration\n";
        config_content << "# Generated by simple-smtp-mailer CLI\n\n";
        config_content << "enabled = true\n";
        config_content << "provider = " << provider_type << "\n";
        config_content << "api_key = " << api_key << "\n";
        if (!api_secret.empty()) {
            config_content << "api_secret = " << api_secret << "\n";
        }
        config_content << "sender_email = " << sender_email << "\n";
        if (!sender_name.empty()) {
            config_content << "sender_name = " << sender_name << "\n";
        }
        if (!base_url.empty()) {
            config_content << "base_url = " << base_url << "\n";
        }
        if (!endpoint.empty()) {
            config_content << "endpoint = " << endpoint << "\n";
        }
        if (!region.empty()) {
            config_content << "region = " << region << "\n";
        }
        if (!domain.empty()) {
            config_content << "domain = " << domain << "\n";
        }
        config_content << "timeout_seconds = " << timeout << "\n";
        config_content << "verify_ssl = " << (verify_ssl ? "true" : "false") << "\n";
        config_content << "enable_tracking = " << (enable_tracking ? "true" : "false") << "\n";
        config_content << "\n";
        
        // Write configuration file
        std::ofstream file(api_config_file, std::ios::app);
        if (!file.is_open()) {
            return CLIResult::error_result("Failed to open API configuration file: " + api_config_file);
        }
        
        if (provider_exists) {
            // Replace existing section
            std::vector<std::string> new_lines;
            new_lines.insert(new_lines.end(), config_lines.begin(), config_lines.begin() + section_start);
            new_lines.push_back(config_content.str());
            new_lines.insert(new_lines.end(), config_lines.begin() + section_end, config_lines.end());
            
            file.close();
            file.open(api_config_file, std::ios::out | std::ios::trunc);
            for (const auto& line : new_lines) {
                file << line;
            }
        } else {
            file << config_content.str();
        }
        
        file.close();
        
        return CLIResult::success_result("API provider '" + provider_name + "' configuration added successfully at " + api_config_file);
        
    } catch (const std::exception& e) {
        return CLIResult::error_result("Failed to add API provider: " + std::string(e.what()));
    }
}

CLIResult APICommands::listProviders(const std::vector<std::string>& args) {
    std::string config_dir = ConfigUtils::getConfigDirectory();
    std::string api_config_file = config_dir + "/api-config.conf";
    
    try {
        if (!std::filesystem::exists(api_config_file)) {
            std::cout << "No API providers configured.\n";
            return CLIResult::success_result();
        }
        
        std::ifstream file(api_config_file);
        if (!file.is_open()) {
            return CLIResult::error_result("Failed to read API configuration file");
        }
        
        std::cout << "Configured API Providers:\n";
        std::cout << "========================\n\n";
        
        std::string line;
        std::string current_provider;
        bool in_section = false;
        
        while (std::getline(file, line)) {
            if (line.find("[api:") == 0) {
                if (in_section && !current_provider.empty()) {
                    std::cout << "\n";
                }
                size_t start = line.find(":") + 1;
                size_t end = line.find("]");
                if (end != std::string::npos) {
                    current_provider = line.substr(start, end - start);
                    std::cout << "Provider: " << current_provider << "\n";
                    in_section = true;
                }
            } else if (in_section && line.find("enabled = ") != std::string::npos) {
                std::string enabled = line.substr(line.find("=") + 1);
                enabled.erase(0, enabled.find_first_not_of(" \t"));
                enabled.erase(enabled.find_last_not_of(" \t") + 1);
                std::cout << "  Enabled: " << enabled << "\n";
            } else if (in_section && line.find("provider = ") != std::string::npos) {
                std::string provider_type = line.substr(line.find("=") + 1);
                provider_type.erase(0, provider_type.find_first_not_of(" \t"));
                provider_type.erase(provider_type.find_last_not_of(" \t") + 1);
                std::cout << "  Type: " << provider_type << "\n";
            } else if (in_section && line.find("sender_email = ") != std::string::npos) {
                std::string sender = line.substr(line.find("=") + 1);
                sender.erase(0, sender.find_first_not_of(" \t"));
                sender.erase(sender.find_last_not_of(" \t") + 1);
                std::cout << "  Sender: " << sender << "\n";
            } else if (in_section && (line.empty() || line[0] == '[')) {
                if (line[0] == '[') {
                    in_section = false;
                    std::cout << "\n";
                }
            }
        }
        
        if (in_section && !current_provider.empty()) {
            std::cout << "\n";
        }
        
        return CLIResult::success_result();
        
    } catch (const std::exception& e) {
        return CLIResult::error_result("Failed to list API providers: " + std::string(e.what()));
    }
}

CLIResult APICommands::showProvider(const std::vector<std::string>& args) {
    if (args.empty()) {
        return CLIResult::error_result("Usage: api provider show <provider>");
    }
    
    std::string provider_name = args[0];
    std::string config_dir = ConfigUtils::getConfigDirectory();
    std::string api_config_file = config_dir + "/api-config.conf";
    
    try {
        if (!std::filesystem::exists(api_config_file)) {
            return CLIResult::error_result("API configuration file not found");
        }
        
        std::ifstream file(api_config_file);
        if (!file.is_open()) {
            return CLIResult::error_result("Failed to read API configuration file");
        }
        
        std::string line;
        bool in_section = false;
        bool found = false;
        
        std::cout << "API Provider Configuration: " << provider_name << "\n";
        std::cout << "==========================================\n";
        
        while (std::getline(file, line)) {
            if (line.find("[api:" + provider_name + "]") != std::string::npos) {
                in_section = true;
                found = true;
                continue;
            }
            
            if (in_section) {
                if (line.empty() || line[0] == '#') {
                    if (line[0] == '[') {
                        break;
                    }
                    continue;
                }
                
                if (line[0] == '[') {
                    break;
                }
                
                size_t pos = line.find('=');
                if (pos != std::string::npos) {
                    std::string key = line.substr(0, pos);
                    std::string value = line.substr(pos + 1);
                    
                    // Trim whitespace
                    key.erase(0, key.find_first_not_of(" \t"));
                    key.erase(key.find_last_not_of(" \t") + 1);
                    value.erase(0, value.find_first_not_of(" \t"));
                    value.erase(value.find_last_not_of(" \t") + 1);
                    
                    // Hide sensitive information
                    if (key == "api_key" || key == "api_secret" || key == "oauth2_token" || key == "bearer_token") {
                        if (!value.empty()) {
                            value = "***hidden***";
                        }
                    }
                    
                    std::cout << "  " << key << ": " << value << "\n";
                }
            }
        }
        
        if (!found) {
            return CLIResult::error_result("API provider not found: " + provider_name);
        }
        
        return CLIResult::success_result();
        
    } catch (const std::exception& e) {
        return CLIResult::error_result("Failed to show API provider: " + std::string(e.what()));
    }
}

CLIResult APICommands::removeProvider(const std::vector<std::string>& args) {
    if (args.empty()) {
        return CLIResult::error_result("Usage: api provider remove <provider>");
    }
    
    std::string provider_name = args[0];
    std::string config_dir = ConfigUtils::getConfigDirectory();
    std::string api_config_file = config_dir + "/api-config.conf";
    
    try {
        if (!std::filesystem::exists(api_config_file)) {
            return CLIResult::error_result("API configuration file not found");
        }
        
        std::ifstream in_file(api_config_file);
        std::vector<std::string> lines;
        std::string line;
        
        while (std::getline(in_file, line)) {
            lines.push_back(line);
        }
        in_file.close();
        
        // Find and remove provider section
        bool found = false;
        size_t section_start = 0;
        size_t section_end = 0;
        
        for (size_t i = 0; i < lines.size(); ++i) {
            if (lines[i].find("[api:" + provider_name + "]") != std::string::npos) {
                found = true;
                section_start = i;
                // Find end of section
                for (size_t j = i + 1; j < lines.size(); ++j) {
                    if (lines[j].empty() && j + 1 < lines.size() && lines[j + 1].find("[") == 0) {
                        section_end = j + 1;
                        break;
                    }
                    if (lines[j].find("[") == 0 && lines[j].find("]") != std::string::npos) {
                        section_end = j;
                        break;
                    }
                    section_end = j + 1;
                }
                break;
            }
        }
        
        if (!found) {
            return CLIResult::error_result("API provider not found: " + provider_name);
        }
        
        // Remove section
        std::vector<std::string> new_lines;
        new_lines.insert(new_lines.end(), lines.begin(), lines.begin() + section_start);
        new_lines.insert(new_lines.end(), lines.begin() + section_end, lines.end());
        
        // Write back to file
        std::ofstream out_file(api_config_file);
        for (const auto& l : new_lines) {
            out_file << l << "\n";
        }
        out_file.close();
        
        return CLIResult::success_result("API provider '" + provider_name + "' removed successfully");
        
    } catch (const std::exception& e) {
        return CLIResult::error_result("Failed to remove API provider: " + std::string(e.what()));
    }
}

CLIResult APICommands::testProvider(const std::vector<std::string>& args) {
    if (args.empty()) {
        return CLIResult::error_result("Usage: api provider test <provider>");
    }
    
    std::string provider_name = args[0];
    
    std::cout << "Testing API provider: " << provider_name << "\n";
    std::cout << "====================\n\n";
    std::cout << "Note: Actual API connection testing requires network access and valid credentials.\n";
    std::cout << "This is a placeholder for future implementation.\n";
    
    return CLIResult::success_result();
}

} // namespace ssmtp_mailer

#pragma GCC diagnostic pop

