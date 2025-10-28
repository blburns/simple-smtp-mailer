#pragma once

#include <string>
#include <memory>

namespace ssmtp_mailer {

/**
 * @brief Daemon utilities for background process management
 */
class Daemon {
public:
    /**
     * @brief Daemonize the current process (Unix/Linux/macOS)
     * @param pid_file Path to PID file for tracking daemon process
     * @param log_file Path to log file for daemon output
     * @return true if daemonization successful, false otherwise
     */
    static bool daemonize(const std::string& pid_file = "", const std::string& log_file = "");
    
    /**
     * @brief Check if daemon is already running by checking PID file
     * @param pid_file Path to PID file
     * @return true if daemon is running, false otherwise
     */
    static bool isRunning(const std::string& pid_file);
    
    /**
     * @brief Get the PID of the running daemon
     * @param pid_file Path to PID file
     * @return Process ID if running, -1 otherwise
     */
    static int getPid(const std::string& pid_file);
    
    /**
     * @brief Stop the daemon process
     * @param pid_file Path to PID file
     * @return true if stopped successfully, false otherwise
     */
    static bool stop(const std::string& pid_file);
    
    /**
     * @brief Reload daemon configuration (send SIGHUP)
     * @param pid_file Path to PID file
     * @return true if reload signal sent successfully, false otherwise
     */
    static bool reload(const std::string& pid_file);
    
    /**
     * @brief Write PID to file
     * @param pid_file Path to PID file
     * @param pid Process ID to write
     * @return true if successful, false otherwise
     */
    static bool writePid(const std::string& pid_file, int pid);
    
    /**
     * @brief Read PID from file
     * @param pid_file Path to PID file
     * @return Process ID, -1 if error
     */
    static int readPid(const std::string& pid_file);
    
    /**
     * @brief Get default PID file path
     * @return PID file path
     */
    static std::string getDefaultPidFile();
    
    /**
     * @brief Get default log file path
     * @return Log file path
     */
    static std::string getDefaultLogFile();
    
private:
    static void setupSignalHandlers();
    static bool createDirectories(const std::string& pid_file, const std::string& log_file);
};

} // namespace ssmtp_mailer

