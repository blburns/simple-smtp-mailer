#!/bin/bash
# Post-installation script for simple-smtp-mailer RPM

set -e

PROJECT_NAME="simple-smtp-mailer"
SERVICE_USER="smtpmailerdev"

# Create service user if it doesn't exist
if ! id "$SERVICE_USER" &>/dev/null; then
    useradd -r -s /sbin/nologin -d /var/lib/$simple-smtp-mailer -c "$simple-smtp-mailer service user" "$SERVICE_USER"
fi

# Set ownership
chown -R "$SERVICE_USER:$SERVICE_USER" /etc/$simple-smtp-mailer 2>/dev/null || true
chown -R "$SERVICE_USER:$SERVICE_USER" /var/log/$simple-smtp-mailer 2>/dev/null || true
chown -R "$SERVICE_USER:$SERVICE_USER" /var/lib/$simple-smtp-mailer 2>/dev/null || true

# Enable and start service
systemctl daemon-reload
systemctl enable "$simple-smtp-mailer" 2>/dev/null || true
systemctl start "$simple-smtp-mailer" 2>/dev/null || true

exit 0

