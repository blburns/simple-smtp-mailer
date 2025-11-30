#!/bin/bash
# Pre-uninstallation script for simple-smtp-mailer RPM

set -e

PROJECT_NAME="simple-smtp-mailer"

# Stop service before removal
if [ "$1" -eq 0 ]; then
    systemctl stop "$simple-smtp-mailer" 2>/dev/null || true
    systemctl disable "$simple-smtp-mailer" 2>/dev/null || true
fi

exit 0

