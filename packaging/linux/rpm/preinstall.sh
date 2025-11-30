#!/bin/bash
# Pre-installation script for simple-smtp-mailer RPM
# License acceptance check

set -e

PROJECT_NAME="simple-smtp-mailer"

# Display license and require acceptance
echo "=========================================="
echo "simple-smtp-mailer License Agreement"
echo "=========================================="
echo ""
cat /usr/share/doc/$simple-smtp-mailer/LICENSE 2>/dev/null || \
    cat /usr/share/$simple-smtp-mailer/LICENSE.txt 2>/dev/null || \
    echo "Please review the license at: https://github.com/simpledaemons/$simple-smtp-mailer/blob/main/LICENSE"
echo ""
echo "By continuing, you agree to the terms of the license."
echo ""
read -p "Do you accept the license? [y/N]: " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "License not accepted. Installation cancelled."
    exit 1
fi

exit 0

