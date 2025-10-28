# Daemon Mode

The simple-smtp-mailer supports running as a background daemon process, providing continuous email queue processing and monitoring capabilities.

## Overview

Daemon mode allows the application to run in the background, processing emails from the queue continuously. This is ideal for production environments where you need:

- Continuous email processing
- Background queue management
- Long-running email delivery services
- Integration with system services (systemd, launchd)

## Starting the Daemon

### Basic Usage

```bash
simple-smtp-mailer --daemon
```

### With Custom Configuration

```bash
simple-smtp-mailer --config /path/to/config.conf --daemon
```

### With Custom PID File

```bash
simple-smtp-mailer --daemon --pid-file /var/run/my-mailer.pid
```

## Managing the Daemon

### Check Daemon Status

```bash
simple-smtp-mailer --status
```

Output:
```
Daemon is running (PID: 12345)
```

If not running:
```
Daemon is not running
```

### Stop the Daemon

```bash
simple-smtp-mailer --stop
```

Or:
```bash
simple-smtp-mailer --stop --pid-file /path/to/custom.pid
```

### Reload Configuration

Send a reload signal to re-read configuration files:

```bash
simple-smtp-mailer --reload
```

This sends a SIGHUP signal to the daemon, triggering a configuration reload while the daemon continues running.

## How It Works

When started in daemon mode:

1. The process forks and detaches from the terminal
2. A new session is created (becomes a daemon process)
3. Standard input/output/error are redirected
4. A PID file is created for process management
5. The email queue processing begins
6. The daemon continuously processes emails from the queue

## Queue Processing

The daemon processes emails using the email queue system:

- **Automatic Processing**: Emails are processed automatically from the queue
- **Retry Logic**: Failed emails are retried with exponential backoff
- **Priority Handling**: High-priority emails are processed first
- **Statistics**: Queue statistics are logged periodically

## Logging

### Log File Location

By default, daemon logs are written to:

- **Linux**: `/var/log/simple-smtp-mailer/simple-smtp-mailer.log`
- **macOS**: `/var/log/simple-smtp-mailer/simple-smtp-mailer.log`
- **Windows**: `%APPDATA%\simple-smtp-mailer\logs\simple-smtp-mailer.log`

### Custom Log File

You can specify a custom log file location:

```bash
simple-smtp-mailer --daemon --pid-file /var/run/mailer.pid
```

The log file path is configurable through the default log file methods.

## System Service Integration

### systemd (Linux)

Create a systemd service file at `/etc/systemd/system/simple-smtp-mailer.service`:

```ini
[Unit]
Description=High-performance SMTP mailer with queue management and OAuth2 support
After=network.target
Wants=network.target

[Service]
Type=simple
User=simple-smtp-mailer
Group=simple-smtp-mailer
ExecStart=/usr/local/bin/simple-smtp-mailer --daemon
ExecReload=/bin/kill -HUP $MAINPID
Restart=always
RestartSec=5
StandardOutput=journal
StandardError=journal
SyslogIdentifier=simple-smtp-mailer

[Install]
WantedBy=multi-user.target
```

Start and enable the service:

```bash
sudo systemctl daemon-reload
sudo systemctl start simple-smtp-mailer
sudo systemctl enable simple-smtp-mailer
```

### launchd (macOS)

Create a plist file at `/Library/LaunchDaemons/com.simple-smtp-mailer.simple-smtp-mailer.plist`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>Label</key>
    <string>com.simple-smtp-mailer.simple-smtp-mailer</string>

    <key>ProgramArguments</key>
    <array>
        <string>/usr/local/bin/simple-smtp-mailer</string>
        <string>--daemon</string>
    </array>

    <key>RunAtLoad</key>
    <true/>

    <key>KeepAlive</key>
    <true/>

    <key>StandardOutPath</key>
    <string>/var/log/simple-smtp-mailer/simple-smtp-mailer.log</string>

    <key>StandardErrorPath</key>
    <string>/var/log/simple-smtp-mailer/simple-smtp-mailer.error.log</string>
</dict>
</plist>
```

Load the service:

```bash
sudo launchctl load /Library/LaunchDaemons/com.simple-smtp-mailer.simple-smtp-mailer.plist
```

### Windows Service

For Windows, use the provided service file template and configure it through the Windows Service Manager or use the `sc` command:

```cmd
sc create SimpleSMTPMailer binPath= "C:\Program Files\simple-smtp-mailer\bin\simple-smtp-mailer.exe --daemon"
sc start SimpleSMTPMailer
```

## Signal Handling

The daemon handles the following signals:

- **SIGTERM**: Graceful shutdown - processes remaining queue items then exits
- **SIGHUP**: Reload configuration - re-reads configuration files
- **SIGINT**: Immediate shutdown
- **SIGPIPE**: Ignored (prevents crashes from broken pipes)

## Configuration

Configure daemon behavior through the main configuration file:

```ini
[Daemon]
pid_file = /var/run/simple-smtp-mailer.pid
log_file = /var/log/simple-smtp-mailer/daemon.log
autostart = true
queue_batch_size = 10
queue_retry_delay = 60
```

## Troubleshooting

### Daemon Won't Start

1. Check if another instance is already running:
   ```bash
   simple-smtp-mailer --status
   ```

2. Check the log file for errors:
   ```bash
   tail -f /var/log/simple-smtp-mailer/simple-smtp-mailer.log
   ```

3. Verify configuration:
   ```bash
   simple-smtp-mailer config
   ```

### Daemon Crashes

1. Check system resources:
   ```bash
   df -h  # Check disk space
   free -m  # Check memory
   ```

2. Check permissions:
   ```bash
   ls -l /var/log/simple-smtp-mailer/
   ls -l /var/run/simple-smtp-mailer.pid
   ```

3. Review logs for errors:
   ```bash
   journalctl -u simple-smtp-mailer  # systemd
   tail -n 100 /var/log/simple-smtp-mailer/daemon.log
   ```

### Queue Processing Issues

1. Check queue status:
   ```bash
   # Via daemon logs or through monitoring
   ```

2. Verify SMTP/API connectivity:
   ```bash
   simple-smtp-mailer test
   simple-smtp-mailer test-api --provider sendgrid
   ```

3. Check configuration:
   ```bash
   simple-smtp-mailer config
   ```

## Performance Optimization

### Adjust Queue Settings

Configure queue processing in your config file:

```ini
[Queue]
max_queue_size = 10000
batch_size = 50
retry_delay = 300
max_retries = 5
```

### Resource Limits

Set appropriate resource limits:

**systemd**:
```ini
[Service]
LimitNOFILE=65536
LimitNPROC=4096
```

**launchd**:
```xml
<key>SoftResourceLimits</key>
<dict>
    <key>NumberOfFiles</key>
    <integer>65536</integer>
    <key>NumberOfProcesses</key>
    <integer>4096</integer>
</dict>
```

## Security Considerations

1. **User Permissions**: Run the daemon with minimal required permissions
2. **File Permissions**: Restrict access to configuration and PID files
3. **Network Access**: Only allow necessary network access
4. **Logging**: Enable secure logging with appropriate log rotation

Example security settings for systemd:

```ini
[Service]
NoNewPrivileges=true
PrivateTmp=true
ProtectSystem=strict
ProtectHome=true
ReadWritePaths=/var/lib/simple-smtp-mailer /var/log/simple-smtp-mailer
```

## Best Practices

1. **Always use system services** (systemd/launchd) for production deployments
2. **Enable logging** and monitor log files regularly
3. **Set resource limits** appropriate for your workload
4. **Use monitoring** to track queue size and processing rates
5. **Implement log rotation** to manage disk space
6. **Test configuration changes** before applying to production
7. **Monitor queue statistics** for optimal performance

## Example: Production Setup

Complete production setup script:

```bash
#!/bin/bash

# Create user and directories
sudo useradd -r -s /bin/false simple-smtp-mailer
sudo mkdir -p /var/log/simple-smtp-mailer
sudo mkdir -p /etc/simple-smtp-mailer
sudo chown -R simple-smtp-mailer:simple-smtp-mailer /var/log/simple-smtp-mailer
sudo chown -R simple-smtp-mailer:simple-smtp-mailer /etc/simple-smtp-mailer

# Copy configuration
sudo cp config/simple-smtp-mailer.conf /etc/simple-smtp-mailer/

# Copy service file
sudo cp deployment/systemd/simple-smtp-mailer.service /etc/systemd/system/

# Enable and start service
sudo systemctl daemon-reload
sudo systemctl enable simple-smtp-mailer
sudo systemctl start simple-smtp-mailer

# Check status
sudo systemctl status simple-smtp-mailer
```

This provides a complete production-ready daemon setup.

