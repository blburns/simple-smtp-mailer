#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <chrono>
#include <csignal>
#include "simple-smtp-mailer/mailer.hpp"
#include "simple-smtp-mailer/unified_mailer.hpp"
#include "simple-smtp-mailer/cli_manager.hpp"
#include "simple-smtp-mailer/daemon.hpp"
#include "simple-smtp-mailer/queue_types.hpp"
#include "core/queue/email_queue.hpp"
#include "core/logging/logger.hpp"

void printUsage() {
    std::cout << "\nUsage: simple-smtp-mailer [OPTIONS] [COMMAND] [ARGS...]" << std::endl;
    std::cout << "\nOptions:" << std::endl;
    std::cout << "  --help, -h           Show this help message" << std::endl;
    std::cout << "  --version, -v        Show version information" << std::endl;
    std::cout << "  --config, -c FILE    Use specified configuration file" << std::endl;
    std::cout << "  --verbose, -V        Enable verbose logging" << std::endl;
    std::cout << "  --daemon             Run in daemon mode (background)" << std::endl;
    std::cout << "  --pid-file FILE      Specify PID file location" << std::endl;
    std::cout << "  --stop               Stop running daemon" << std::endl;
    std::cout << "  --reload             Reload daemon configuration" << std::endl;
    std::cout << "  --status             Check daemon status" << std::endl;
    
    std::cout << "\nCommands:" << std::endl;
    std::cout << "  send                 Send an email" << std::endl;
    std::cout << "  send-api             Send an email via API" << std::endl;
    std::cout << "  test                 Test SMTP connection" << std::endl;
    std::cout << "  test-api             Test API connection" << std::endl;
    std::cout << "  config               Show configuration status" << std::endl;
    std::cout << "  queue                Manage email queue" << std::endl;
    std::cout << "  api                  Manage API configurations" << std::endl;
    std::cout << "  cli                  Configuration management CLI" << std::endl;
    
    std::cout << "\nCLI Subcommands:" << std::endl;
    std::cout << "  Configuration Management:" << std::endl;
    std::cout << "    cli config domain add <domain> --smtp-server <server> --port <port> [options]" << std::endl;
    std::cout << "    cli config domain list" << std::endl;
    std::cout << "    cli config domain show <domain>" << std::endl;
    std::cout << "    cli config user add <email> --domain <domain> [options]" << std::endl;
    std::cout << "    cli config user list [--domain <domain>]" << std::endl;
    std::cout << "    cli config global show" << std::endl;
    
    std::cout << "\n  Authentication Management:" << std::endl;
    std::cout << "    cli auth oauth2 setup --provider <google|microsoft> --domain <domain>" << std::endl;
    std::cout << "    cli auth oauth2 test --domain <domain>" << std::endl;
    std::cout << "    cli auth service-account add --domain <domain> --file <json-file>" << std::endl;
    std::cout << "    cli auth service-account list" << std::endl;
    std::cout << "    cli auth service-account test --domain <domain>" << std::endl;
    
    std::cout << "\n  Template Management:" << std::endl;
    std::cout << "    cli template create <name> --subject <subject> --body <body> [--html <html>]" << std::endl;
    std::cout << "    cli template list" << std::endl;
    std::cout << "    cli template show <name>" << std::endl;
    std::cout << "    cli template test <name> --to <email> [--from <email>]" << std::endl;
    std::cout << "    cli template address create <pattern> --domain <domain> --types <types>" << std::endl;
    std::cout << "    cli template address list" << std::endl;
    
    std::cout << "\n  Validation & Testing:" << std::endl;
    std::cout << "    cli validate config [--fix] [--verbose]" << std::endl;
    std::cout << "    cli test connections [--smtp-only] [--api-only] [--domain <domain>]" << std::endl;
    std::cout << "    cli config backup [--file <backup-file>]" << std::endl;
    std::cout << "    cli config restore --file <backup-file>" << std::endl;
    
    std::cout << "\n  Interactive Setup:" << std::endl;
    std::cout << "    cli setup wizard" << std::endl;
    std::cout << "    cli setup domain <domain>" << std::endl;
    
    std::cout << "\n  API Provider Management:" << std::endl;
    std::cout << "    cli api provider add <provider> --api-key <key> --sender <email>" << std::endl;
    std::cout << "    cli api provider list" << std::endl;
    
    std::cout << "\nQueue Subcommands:" << std::endl;
    std::cout << "  start                Start the email processing queue" << std::endl;
    std::cout << "  stop                 Stop the email processing queue" << std::endl;
    std::cout << "  status               Show queue status" << std::endl;
    std::cout << "  add                  Add email to queue" << std::endl;
    std::cout << "  list                 List pending emails" << std::endl;
    std::cout << "  failed               List failed emails" << std::endl;
    
    std::cout << "\nExamples:" << std::endl;
    std::cout << "  # Basic email sending:" << std::endl;
    std::cout << "  simple-smtp-mailer send --from user@example.com --to recipient@domain.com --subject 'Test' --body 'Hello'" << std::endl;
    std::cout << "  simple-smtp-mailer send-api --provider sendgrid --from user@example.com --to recipient@domain.com --subject 'Test' --body 'Hello'" << std::endl;
    
    std::cout << "\n  # Queue management:" << std::endl;
    std::cout << "  simple-smtp-mailer queue add --from user@example.com --to recipient@domain.com --subject 'Queued' --body 'Hello'" << std::endl;
    std::cout << "  simple-smtp-mailer queue start" << std::endl;
    std::cout << "  simple-smtp-mailer queue status" << std::endl;
    
    std::cout << "\n  # Testing connections:" << std::endl;
    std::cout << "  simple-smtp-mailer test" << std::endl;
    std::cout << "  simple-smtp-mailer test-api --provider sendgrid" << std::endl;
    
    std::cout << "\n  # CLI configuration management:" << std::endl;
    std::cout << "  simple-smtp-mailer cli setup wizard" << std::endl;
    std::cout << "  simple-smtp-mailer cli config domain add example.com --smtp-server smtp.gmail.com --port 587" << std::endl;
    std::cout << "  simple-smtp-mailer cli config user add user@example.com --domain example.com" << std::endl;
    std::cout << "  simple-smtp-mailer cli auth oauth2 setup --provider google --domain example.com" << std::endl;
    std::cout << "  simple-smtp-mailer cli template create welcome --subject 'Welcome!' --body 'Welcome to our service!'" << std::endl;
    std::cout << "  simple-smtp-mailer cli validate config --verbose" << std::endl;
    
    std::cout << "\n  # Using custom config:" << std::endl;
    std::cout << "  simple-smtp-mailer --config /path/to/config.conf send --from user@example.com --to recipient@domain.com --subject 'Test' --body 'Hello'" << std::endl;
}

void printVersion() {
    std::cout << "simple-smtp-mailer v0.2.0" << std::endl;
    std::cout << "Simple SMTP Mailer for Linux and macOS" << std::endl;
}

bool parseSendCommand(const std::vector<std::string>& args, std::string& from, std::string& to, 
                     std::string& subject, std::string& body, std::string& html_body) {
    from.clear();
    to.clear();
    subject.clear();
    body.clear();
    html_body.clear();
    
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--from" && i + 1 < args.size()) {
            from = args[++i];
        } else if (args[i] == "--to" && i + 1 < args.size()) {
            to = args[++i];
        } else if (args[i] == "--subject" && i + 1 < args.size()) {
            subject = args[++i];
        } else if (args[i] == "--body" && i + 1 < args.size()) {
            body = args[++i];
        } else if (args[i] == "--html" && i + 1 < args.size()) {
            html_body = args[++i];
        }
    }
    
    return !from.empty() && !to.empty() && !subject.empty() && !body.empty();
}

bool parseSendAPICommand(const std::vector<std::string>& args, std::string& provider, std::string& from, 
                        std::string& to, std::string& subject, std::string& body, std::string& html_body) {
    provider.clear();
    from.clear();
    to.clear();
    subject.clear();
    body.clear();
    html_body.clear();
    
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--provider" && i + 1 < args.size()) {
            provider = args[++i];
        } else if (args[i] == "--from" && i + 1 < args.size()) {
            from = args[++i];
        } else if (args[i] == "--to" && i + 1 < args.size()) {
            to = args[++i];
        } else if (args[i] == "--subject" && i + 1 < args.size()) {
            subject = args[++i];
        } else if (args[i] == "--body" && i + 1 < args.size()) {
            body = args[++i];
        } else if (args[i] == "--html" && i + 1 < args.size()) {
            html_body = args[++i];
        }
    }
    
    return !provider.empty() && !from.empty() && !to.empty() && !subject.empty() && !body.empty();
}

void runDaemonMode(const std::string& config_file, const std::string& pid_file, bool verbose) {
    ssmtp_mailer::Logger& logger = ssmtp_mailer::Logger::getInstance();
    
    logger.info("Starting daemon mode");
    
    // Daemonize the process
    std::string pid_file_path = pid_file.empty() ? ssmtp_mailer::Daemon::getDefaultPidFile() : pid_file;
    std::string log_file = ssmtp_mailer::Daemon::getDefaultLogFile();
    
    if (!ssmtp_mailer::Daemon::daemonize(pid_file_path, log_file)) {
        logger.error("Failed to daemonize process");
        exit(1);
    }
    
    logger.info("Daemon started successfully");
    
    // Initialize mailer
    ssmtp_mailer::Mailer mailer(config_file);
    
    if (!mailer.isConfigured()) {
        logger.error("Mailer not properly configured: " + mailer.getLastError());
        exit(1);
    }
    
    logger.info("Daemon mailer initialized successfully");
    
    // Initialize email queue
    ssmtp_mailer::EmailQueue queue;
    
    // Set up send callback
    queue.setSendCallback([&mailer](const ssmtp_mailer::Email* email) -> ssmtp_mailer::SMTPResult {
        // Send the email using the mailer
        std::string to_combined;
        if (!email->to.empty()) {
            to_combined = email->to[0];
        }
        return mailer.send(email->from, to_combined, email->subject, email->body);
    });
    
    // Start the queue processing
    queue.start();
    logger.info("Email queue started");
    
    // Main daemon loop - process queue continuously
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
        
        // Log queue statistics periodically
        if (queue.isRunning()) {
            logger.info("Queue status - Size: " + std::to_string(queue.size()) + 
                       ", Processed: " + std::to_string(queue.getTotalProcessed()) +
                       ", Failed: " + std::to_string(queue.getTotalFailed()));
        }
    }
    
    // Clean up
    queue.stop();
    logger.info("Daemon stopped");
}

int main(int argc, char* argv[]) {
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) {
        args.push_back(argv[i]);
    }
    
    // Parse global options
    std::string config_file;
    std::string pid_file;
    bool verbose = false;
    bool daemon_mode = false;
    bool stop_daemon = false;
    bool reload_daemon = false;
    bool status_daemon = false;
    
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--help" || args[i] == "-h") {
            printUsage();
            return 0;
        } else if (args[i] == "--version" || args[i] == "-v") {
            printVersion();
            return 0;
        } else if (args[i] == "--config" || args[i] == "-c") {
            if (i + 1 < args.size()) {
                config_file = args[++i];
            } else {
                std::cerr << "Error: --config requires a file path" << std::endl;
                return 1;
            }
        } else if (args[i] == "--verbose" || args[i] == "-V") {
            verbose = true;
        } else if (args[i] == "--daemon") {
            daemon_mode = true;
        } else if (args[i] == "--pid-file") {
            if (i + 1 < args.size()) {
                pid_file = args[++i];
            } else {
                std::cerr << "Error: --pid-file requires a file path" << std::endl;
                return 1;
            }
        } else if (args[i] == "--stop") {
            stop_daemon = true;
        } else if (args[i] == "--reload") {
            reload_daemon = true;
        } else if (args[i] == "--status") {
            status_daemon = true;
        }
    }
    
    // Handle daemon control commands
    if (stop_daemon) {
        std::string pid_file_path = pid_file.empty() ? ssmtp_mailer::Daemon::getDefaultPidFile() : pid_file;
        if (ssmtp_mailer::Daemon::stop(pid_file_path)) {
            std::cout << "Daemon stopped successfully" << std::endl;
            return 0;
        } else {
            std::cerr << "Failed to stop daemon" << std::endl;
            return 1;
        }
    }
    
    if (reload_daemon) {
        std::string pid_file_path = pid_file.empty() ? ssmtp_mailer::Daemon::getDefaultPidFile() : pid_file;
        if (ssmtp_mailer::Daemon::reload(pid_file_path)) {
            std::cout << "Daemon reload signal sent successfully" << std::endl;
            return 0;
        } else {
            std::cerr << "Failed to reload daemon" << std::endl;
            return 1;
        }
    }
    
    if (status_daemon) {
        std::string pid_file_path = pid_file.empty() ? ssmtp_mailer::Daemon::getDefaultPidFile() : pid_file;
        if (ssmtp_mailer::Daemon::isRunning(pid_file_path)) {
            int pid = ssmtp_mailer::Daemon::getPid(pid_file_path);
            std::cout << "Daemon is running (PID: " << pid << ")" << std::endl;
            return 0;
        } else {
            std::cout << "Daemon is not running" << std::endl;
            return 1;
        }
    }
    
    // Initialize logging
    try {
        if (verbose) {
            ssmtp_mailer::Logger::initialize("", ssmtp_mailer::LogLevel::DEBUG);
        } else {
            ssmtp_mailer::Logger::initialize("", ssmtp_mailer::LogLevel::INFO);
        }
    } catch (const std::exception& e) {
        std::cerr << "Warning: Failed to initialize logger: " << e.what() << std::endl;
    }
    
    ssmtp_mailer::Logger& logger = ssmtp_mailer::Logger::getInstance();
    logger.info("simple-smtp-mailer v0.2.0 starting up");
    
    // If daemon mode is requested, run as daemon
    if (daemon_mode) {
        runDaemonMode(config_file, pid_file, verbose);
        return 0;
    }
    
    std::string command = args[0];
    
    try {
        // Initialize mailer
        ssmtp_mailer::Mailer mailer(config_file);
        
        if (!mailer.isConfigured()) {
            std::cerr << "Error: Mailer not properly configured: " << mailer.getLastError() << std::endl;
            logger.error("Mailer configuration failed: " + mailer.getLastError());
            return 1;
        }
        
        logger.info("Mailer initialized successfully");
        
        if (command == "send") {
            // Parse send command arguments
            std::vector<std::string> send_args(args.begin() + 1, args.end());
            std::string from, to, subject, body, html_body;
            
            if (!parseSendCommand(send_args, from, to, subject, body, html_body)) {
                std::cerr << "Error: Invalid send command arguments" << std::endl;
                std::cerr << "Usage: send --from EMAIL --to EMAIL --subject SUBJECT --body BODY [--html HTML_BODY]" << std::endl;
                return 1;
            }
            
            logger.info("Sending email from " + from + " to " + to);
            
            ssmtp_mailer::SMTPResult result;
            if (!html_body.empty()) {
                result = mailer.sendHtml(from, to, subject, body, html_body);
            } else {
                result = mailer.send(from, to, subject, body);
            }
            
            if (result.success) {
                std::cout << "Email sent successfully!" << std::endl;
                std::cout << "Message ID: " << result.message_id << std::endl;
                logger.info("Email sent successfully with message ID: " + result.message_id);
                return 0;
            } else {
                std::cerr << "Failed to send email: " << result.error_message << std::endl;
                logger.error("Email sending failed: " + result.error_message);
                return 1;
            }
            
        } else if (command == "test") {
            logger.info("Testing SMTP connection");
            std::cout << "Testing SMTP connection..." << std::endl;
            
            if (mailer.testConnection()) {
                std::cout << "Connection test successful!" << std::endl;
                logger.info("SMTP connection test successful");
                return 0;
            } else {
                std::cerr << "Connection test failed: " << mailer.getLastError() << std::endl;
                logger.error("SMTP connection test failed: " + mailer.getLastError());
                return 1;
            }
            
        } else if (command == "config") {
            logger.info("Showing configuration status");
            std::cout << "Configuration Status:" << std::endl;
            std::cout << "  Mailer configured: " << (mailer.isConfigured() ? "Yes" : "No") << std::endl;
            
            if (!mailer.isConfigured()) {
                std::cout << "  Error: " << mailer.getLastError() << std::endl;
            } else {
                std::cout << "  Status: Ready" << std::endl;
            }
            
            return 0;
            
        } else if (command == "send-api") {
            // Parse send-api command arguments
            std::vector<std::string> send_args(args.begin() + 1, args.end());
            std::string provider, from, to, subject, body, html_body;
            
            if (!parseSendAPICommand(send_args, provider, from, to, subject, body, html_body)) {
                std::cerr << "Error: Invalid send-api command arguments" << std::endl;
                std::cout << "Usage: send-api --provider PROVIDER --from EMAIL --to EMAIL --subject SUBJECT --body BODY [--html HTML_BODY]" << std::endl;
                std::cout << "Supported providers: sendgrid, mailgun, ses" << std::endl;
                return 1;
            }
            
            logger.info("Sending email via API from " + from + " to " + to + " using " + provider);
            
            // Validate provider
            if (provider != "sendgrid" && provider != "mailgun" && provider != "ses") {
                std::cerr << "Error: Unsupported provider '" << provider << "'" << std::endl;
                std::cout << "Supported providers: sendgrid, mailgun, ses" << std::endl;
                return 1;
            }
            
            std::cout << "API-based email sending configured for provider: " << provider << std::endl;
            std::cout << "From: " << from << std::endl;
            std::cout << "To: " << to << std::endl;
            std::cout << "Subject: " << subject << std::endl;
            std::cout << "Body: " << body << std::endl;
            if (!html_body.empty()) {
                std::cout << "HTML Body: " << html_body << std::endl;
            }
            
            std::cout << "\nNote: To actually send emails, configure the API credentials in api-config.conf" << std::endl;
            std::cout << "and use the unified mailer programmatically or implement the full integration." << std::endl;
            
            return 0;
            
        } else if (command == "test-api") {
            logger.info("Testing API connection");
            
            if (args.size() < 2) {
                std::cerr << "Error: test-api requires --provider argument" << std::endl;
                std::cerr << "Usage: test-api --provider PROVIDER" << std::endl;
                return 1;
            }
            
            std::string provider;
            for (size_t i = 1; i < args.size(); ++i) {
                if (args[i] == "--provider" && i + 1 < args.size()) {
                    provider = args[++i];
                    break;
                }
            }
            
            if (provider.empty()) {
                std::cerr << "Error: No provider specified" << std::endl;
                return 1;
            }
            
            // Validate provider
            if (provider != "sendgrid" && provider != "mailgun" && provider != "ses") {
                std::cerr << "Error: Unsupported provider '" << provider << "'" << std::endl;
                std::cout << "Supported providers: sendgrid, mailgun, ses" << std::endl;
                return 1;
            }
            
            std::cout << "Testing API connection for provider: " << provider << std::endl;
            std::cout << "API testing configured for provider: " << provider << std::endl;
            std::cout << "\nNote: To actually test connections, configure the API credentials in api-config.conf" << std::endl;
            std::cout << "and use the unified mailer programmatically or implement the full integration." << std::endl;
            
            return 0;
            
        } else if (command == "queue") {
            logger.info("Queue management command");
            
            if (args.size() < 2) {
                std::cerr << "Error: Queue command requires subcommand" << std::endl;
                std::cerr << "Usage: queue [start|stop|status|add|list|failed]" << std::endl;
                return 1;
            }
            
            std::string subcommand = args[1];
            
            if (subcommand == "start") {
                mailer.startQueue();
                std::cout << "Email queue started" << std::endl;
                logger.info("Email queue started");
                return 0;
                
            } else if (subcommand == "stop") {
                mailer.stopQueue();
                std::cout << "Email queue stopped" << std::endl;
                logger.info("Email queue stopped");
                return 0;
                
            } else if (subcommand == "status") {
                std::cout << "Queue Status:" << std::endl;
                std::cout << "  Running: " << (mailer.isQueueRunning() ? "Yes" : "No") << std::endl;
                std::cout << "  Size: " << mailer.getQueueSize() << std::endl;
                return 0;
                
            } else if (subcommand == "add") {
                if (args.size() < 6) {
                    std::cerr << "Error: Queue add requires --from, --to, --subject, --body" << std::endl;
                    std::cerr << "Usage: queue add --from EMAIL --to EMAIL --subject SUBJECT --body BODY" << std::endl;
                    return 1;
                }
                
                std::vector<std::string> queue_args(args.begin() + 2, args.end());
                std::string from, to, subject, body, html_body;
                
                if (!parseSendCommand(queue_args, from, to, subject, body, html_body)) {
                    std::cerr << "Error: Invalid queue add arguments" << std::endl;
                    return 1;
                }
                
                ssmtp_mailer::Email email(from, to, subject, body);
                mailer.enqueue(email);
                std::cout << "Email added to queue" << std::endl;
                logger.info("Email queued from " + from + " to " + to);
                return 0;
                
            } else if (subcommand == "list") {
                auto pending = mailer.getPendingEmails();
                std::cout << "Pending emails: " << pending.size() << std::endl;
                for (const auto& queued : pending) {
                    std::string recipient = queued.to_addresses.empty() ? "none" : queued.to_addresses[0];
                    std::cout << "  - " << queued.from_address << " -> " << recipient 
                              << " (Priority: " << static_cast<int>(queued.priority) << ")" << std::endl;
                }
                return 0;
                
            } else if (subcommand == "failed") {
                auto failed = mailer.getFailedEmails();
                std::cout << "Failed emails: " << failed.size() << std::endl;
                for (const auto& queued : failed) {
                    std::string recipient = queued.to_addresses.empty() ? "none" : queued.to_addresses[0];
                    std::cout << "  - " << queued.from_address << " -> " << recipient 
                              << " (Error: " << queued.error_message << ")" << std::endl;
                }
                return 0;
                
            } else {
                std::cerr << "Error: Unknown queue subcommand: " << subcommand << std::endl;
                std::cerr << "Usage: queue [start|stop|status|add|list|failed]" << std::endl;
                return 1;
            }
            
        } else if (command == "cli") {
            // Initialize CLI manager
            ssmtp_mailer::CLIManager cli_manager;
            if (!cli_manager.initialize()) {
                std::cerr << "Error: Failed to initialize CLI manager" << std::endl;
                return 1;
            }
            
            // Parse CLI command
            if (args.size() < 2) {
                cli_manager.printHelp();
                return 0;
            }
            
            std::string cli_command = args[1];
            std::vector<std::string> cli_args(args.begin() + 2, args.end());
            
            // Execute CLI command
            auto result = cli_manager.executeCommand(cli_command, cli_args);
            
            if (!result.success) {
                std::cerr << "Error: " << result.message << std::endl;
                return result.exit_code;
            }
            
            if (!result.message.empty()) {
                std::cout << result.message << std::endl;
            }
            
            return 0;
            
        } else {
            std::cerr << "Error: Unknown command: " << command << std::endl;
            printUsage();
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        logger.error("Application error: " + std::string(e.what()));
        return 1;
    }
    
    return 0;
}
