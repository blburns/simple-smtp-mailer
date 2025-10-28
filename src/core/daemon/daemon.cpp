#include "simple-smtp-mailer/daemon.hpp"
#include "simple-smtp-mailer/platform.hpp"
#include "core/logging/logger.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <csignal>
#include <cstring>

#ifdef _WIN32
    #include <windows.h>
    #include <process.h>
    #include <io.h>
    #include <direct.h>
    #include <shlobj.h>
    #define getpid _getpid
#else
    #include <unistd.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <fcntl.h>
    #include <dirent.h>
#endif

namespace ssmtp_mailer {

// Global flag for signal handling
static volatile bool g_running = true;
static volatile bool g_reload = false;

void Daemon::setupSignalHandlers() {
    #ifndef _WIN32
    // SIGTERM - graceful shutdown
    signal(SIGTERM, [](int sig) {
        g_running = false;
    });
    
    // SIGHUP - reload configuration
    signal(SIGHUP, [](int sig) {
        g_reload = true;
    });
    
    // SIGINT - immediate shutdown
    signal(SIGINT, [](int sig) {
        g_running = false;
    });
    
    // Ignore SIGPIPE
    signal(SIGPIPE, SIG_IGN);
    #endif
}

bool Daemon::createDirectories(const std::string& pid_file, const std::string& log_file) {
    // Extract directory paths
    std::string pid_dir = pid_file.substr(0, pid_file.find_last_of("/\\"));
    std::string log_dir = log_file.substr(0, log_file.find_last_of("/\\"));
    
    if (!pid_dir.empty() && !Platform::fileExists(pid_dir)) {
        if (!Platform::createDirectory(pid_dir)) {
            std::cerr << "Failed to create PID directory: " << pid_dir << std::endl;
            return false;
        }
    }
    
    if (!log_dir.empty() && !Platform::fileExists(log_dir)) {
        if (!Platform::createDirectory(log_dir)) {
            std::cerr << "Failed to create log directory: " << log_dir << std::endl;
            return false;
        }
    }
    
    return true;
}

bool Daemon::daemonize(const std::string& pid_file, const std::string& log_file) {
    #ifdef _WIN32
    // Windows doesn't support traditional daemonization
    // Instead, we just redirect output to log file
    if (!log_file.empty()) {
        freopen(log_file.c_str(), "a", stdout);
        freopen(log_file.c_str(), "a", stderr);
    }
    return true;
    #else
    // Unix/Linux/macOS daemonization
    Logger& logger = Logger::getInstance();
    
    // Check if already running
    if (!pid_file.empty() && isRunning(pid_file)) {
        logger.error("Daemon is already running (PID: " + std::to_string(readPid(pid_file)) + ")");
        return false;
    }
    
    // Create directories if needed
    if (!createDirectories(pid_file, log_file)) {
        logger.error("Failed to create daemon directories");
        return false;
    }
    
    // Fork the process
    pid_t pid = fork();
    
    if (pid < 0) {
        logger.error("Failed to fork daemon process");
        return false;
    }
    
    if (pid > 0) {
        // Parent process - exit
        std::cout << "Daemon started with PID: " << pid << std::endl;
        exit(0);
    }
    
    // Child process continues below
    
    // Create a new session
    if (setsid() < 0) {
        logger.error("Failed to create new session");
        return false;
    }
    
    // Second fork to avoid being a session leader
    pid = fork();
    
    if (pid < 0) {
        logger.error("Failed to fork daemon process (second fork)");
        return false;
    }
    
    if (pid > 0) {
        // Parent process - exit
        exit(0);
    }
    
    // Set signal handlers
    setupSignalHandlers();
    
    // Change working directory
    chdir("/");
    
    // Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    // Redirect outputs to log file or /dev/null
    if (!log_file.empty()) {
        int fd = open(log_file.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0644);
        if (fd >= 0) {
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
        }
    } else {
        int fd = open("/dev/null", O_RDWR);
        if (fd >= 0) {
            dup2(fd, STDIN_FILENO);
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
        }
    }
    
    // Write PID file
    if (!pid_file.empty()) {
        writePid(pid_file, getpid());
    }
    
    logger.info("Daemon started successfully with PID: " + std::to_string(getpid()));
    
    return true;
    #endif
}

bool Daemon::isRunning(const std::string& pid_file) {
    if (pid_file.empty()) {
        return false;
    }
    
    int pid = readPid(pid_file);
    if (pid < 0) {
        return false;
    }
    
    #ifdef _WIN32
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (hProcess) {
        CloseHandle(hProcess);
        return true;
    }
    return false;
    #else
    // Check if process with this PID exists
    return (kill(pid, 0) == 0);
    #endif
}

int Daemon::getPid(const std::string& pid_file) {
    return readPid(pid_file);
}

bool Daemon::stop(const std::string& pid_file) {
    if (pid_file.empty()) {
        return false;
    }
    
    int pid = readPid(pid_file);
    if (pid < 0) {
        return false;
    }
    
    Logger& logger = Logger::getInstance();
    logger.info("Stopping daemon with PID: " + std::to_string(pid));
    
    #ifdef _WIN32
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess) {
        TerminateProcess(hProcess, 0);
        CloseHandle(hProcess);
        
        // Remove PID file
        remove(pid_file.c_str());
        return true;
    }
    return false;
    #else
    // Send SIGTERM to the process
    if (kill(pid, SIGTERM) == 0) {
        // Wait a bit for graceful shutdown
        sleep(2);
        
        // If still running, send SIGKILL
        if (isRunning(pid_file)) {
            kill(pid, SIGKILL);
            sleep(1);
        }
        
        // Remove PID file
        remove(pid_file.c_str());
        return true;
    }
    return false;
    #endif
}

bool Daemon::reload(const std::string& pid_file) {
    if (pid_file.empty()) {
        return false;
    }
    
    int pid = readPid(pid_file);
    if (pid < 0) {
        return false;
    }
    
    Logger& logger = Logger::getInstance();
    logger.info("Reloading daemon configuration (PID: " + std::to_string(pid) + ")");
    
    #ifdef _WIN32
    // Windows doesn't have SIGHUP, so we can't reload
    logger.warning("Reload not supported on Windows");
    return false;
    #else
    return (kill(pid, SIGHUP) == 0);
    #endif
}

bool Daemon::writePid(const std::string& pid_file, int pid) {
    if (pid_file.empty()) {
        return false;
    }
    
    std::ofstream file(pid_file);
    if (!file.is_open()) {
        return false;
    }
    
    file << pid << std::endl;
    file.close();
    
    return true;
}

int Daemon::readPid(const std::string& pid_file) {
    if (pid_file.empty() || !Platform::fileExists(pid_file)) {
        return -1;
    }
    
    std::ifstream file(pid_file);
    if (!file.is_open()) {
        return -1;
    }
    
    int pid;
    file >> pid;
    file.close();
    
    return pid;
}

std::string Daemon::getDefaultPidFile() {
    #ifdef _WIN32
    return Platform::getEnvironmentVariable("APPDATA") + "/simple-smtp-mailer/simple-smtp-mailer.pid";
    #else
    #ifdef __APPLE__
    return "/var/run/simple-smtp-mailer.pid";
    #else
    return "/var/run/simple-smtp-mailer/simple-smtp-mailer.pid";
    #endif
    #endif
}

std::string Daemon::getDefaultLogFile() {
    #ifdef _WIN32
    return Platform::getEnvironmentVariable("APPDATA") + "/simple-smtp-mailer/logs/simple-smtp-mailer.log";
    #else
    return "/var/log/simple-smtp-mailer/simple-smtp-mailer.log";
    #endif
}

} // namespace ssmtp_mailer

