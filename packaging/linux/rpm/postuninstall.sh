#!/bin/bash
# Post-uninstallation script for simple-smtp-mailer RPM

set -e

# Reload systemd
systemctl daemon-reload

exit 0

