# simple-smtp-mailer CLI Documentation

The simple-smtp-mailer CLI provides comprehensive configuration management for the simple-smtp-mailer application. This document describes all available commands and their usage.

## Overview

The CLI is accessed through the main `simple-smtp-mailer` command with the `cli` subcommand:

```bash
simple-smtp-mailer cli <command> [options]
```

## Quick Start

### Interactive Setup Wizard

For first-time setup, use the interactive wizard:

```bash
simple-smtp-mailer cli setup-wizard
```

This will guide you through:
1. Global configuration
2. Domain setup
3. User configuration
4. Authentication setup
5. API provider configuration
6. Validation and testing

### Basic Domain Setup

```bash
# Add a domain with Gmail SMTP
simple-smtp-mailer cli config-domain-add example.com \
  --smtp-server smtp.gmail.com \
  --port 587 \
  --auth-method LOGIN \
  --username mailer@example.com \
  --password your_app_password

# List all domains
simple-smtp-mailer cli config-domain-list

# Show domain details
simple-smtp-mailer cli config-domain-show example.com
```

### User Management

```bash
# Add a user
simple-smtp-mailer cli config-user-add user@example.com \
  --domain example.com \
  --can-send-from true \
  --can-send-to true

# List users
simple-smtp-mailer cli config-user-list

# List users for specific domain
simple-smtp-mailer cli config-user-list --domain example.com
```

## Command Reference

### Configuration Commands

#### Domain Management

**Add Domain**
```bash
simple-smtp-mailer cli config-domain-add <domain> [options]
```

Options:
- `--smtp-server <server>` - SMTP server hostname (required)
- `--port <port>` - SMTP port (default: 587)
- `--auth-method <method>` - Authentication method (LOGIN, OAUTH2, SERVICE_ACCOUNT)
- `--username <username>` - Username for authentication
- `--password <password>` - Password for authentication
- `--use-ssl` - Use SSL encryption
- `--use-starttls` - Use STARTTLS encryption (default)

**List Domains**
```bash
simple-smtp-mailer cli config-domain-list
```

**Show Domain**
```bash
simple-smtp-mailer cli config-domain-show <domain>
```

**Remove Domain**
```bash
simple-smtp-mailer cli config-domain-remove <domain>
```

**Enable Domain**
```bash
simple-smtp-mailer cli config-domain-enable <domain>
```

**Disable Domain**
```bash
simple-smtp-mailer cli config-domain-disable <domain>
```

#### User Management

**Add User**
```bash
simple-smtp-mailer cli config-user-add <email> [options]
```

Options:
- `--domain <domain>` - Domain for this user (required)
- `--can-send-from <true|false>` - Allow sending from this address
- `--can-send-to <true|false>` - Allow sending to recipients
- `--template-address` - This is a template address (e.g., contact-{type}@domain)
- `--allowed-recipients <list>` - Comma-separated list of allowed recipients
- `--allowed-domains <list>` - Comma-separated list of allowed domains

**List Users**
```bash
simple-smtp-mailer cli config-user-list [--domain <domain>]
```

**Show User**
```bash
simple-smtp-mailer cli config-user-show <email>
```

**Remove User**
```bash
simple-smtp-mailer cli config-user-remove <email>
```

**Enable User**
```bash
simple-smtp-mailer cli config-user-enable <email>
```

**Disable User**
```bash
simple-smtp-mailer cli config-user-disable <email>
```

#### Global Configuration

**Show Global Config**
```bash
simple-smtp-mailer cli config-global-show
```

### Authentication Commands

#### OAuth2 Setup

**Setup OAuth2**
```bash
simple-smtp-mailer cli auth-oauth2-setup --provider <google|microsoft> --domain <domain>
```

**Test OAuth2**
```bash
simple-smtp-mailer cli auth-oauth2-test --domain <domain>
```

#### Service Account Management

**Add Service Account**
```bash
simple-smtp-mailer cli auth-service-account-add --domain <domain> --file <json-file>
```

**List Service Accounts**
```bash
simple-smtp-mailer cli auth-service-account-list
```

**Test Service Account**
```bash
simple-smtp-mailer cli auth-service-account-test --domain <domain>
```

### Template Commands

#### Email Templates

**Create Template**
```bash
simple-smtp-mailer cli template-create <name> --subject <subject> --body <body> [--html <html>]
```

**List Templates**
```bash
simple-smtp-mailer cli template-list
```

**Show Template**
```bash
simple-smtp-mailer cli template-show <name>
```

**Remove Template**
```bash
simple-smtp-mailer cli template-remove <name>
```

**Test Template**
```bash
simple-smtp-mailer cli template-test <name> --to <email> [--from <email>]
```

#### Address Templates

**Create Address Template**
```bash
simple-smtp-mailer cli template-address-create <pattern> --domain <domain> --types <types>
```

Example:
```bash
simple-smtp-mailer cli template-address-create contact-{type}@example.com \
  --domain example.com \
  --types "legal,privacy,general,support,sales"
```

**List Address Templates**
```bash
simple-smtp-mailer cli template-address-list
```

**Remove Address Template**
```bash
simple-smtp-mailer cli template-address-remove <pattern>
```

### API Provider Management Commands

#### API Provider Configuration

**Add API Provider**
```bash
simple-smtp-mailer cli api-provider-add <provider> --api-key <key> --sender <email> [options]
```

Options:
- `--api-key <key>` - API key for authentication (required)
- `--api-secret <secret>` - API secret (for providers that require it)
- `--sender <email>` - Verified sender email address (required)
- `--sender-name <name>` - Sender display name
- `--base-url <url>` - Base URL for API calls
- `--endpoint <endpoint>` - API endpoint path
- `--region <region>` - Region (for AWS SES, Mailgun)
- `--domain <domain>` - Domain (for Mailgun)
- `--timeout <seconds>` - Request timeout (default: 30)
- `--no-verify-ssl` - Disable SSL certificate verification
- `--enable-tracking` - Enable email tracking

Supported providers: `sendgrid`, `mailgun`, `amazon-ses`, `ses`, `postmark`, `sparkpost`, `mailjet`

Example:
```bash
simple-smtp-mailer cli api-provider-add sendgrid \
  --api-key SG.your_api_key_here \
  --sender sender@yourdomain.com \
  --sender-name "Your Company" \
  --enable-tracking
```

**List API Providers**
```bash
simple-smtp-mailer cli api-provider-list
```

**Show API Provider**
```bash
simple-smtp-mailer cli api-provider-show <provider>
```

**Remove API Provider**
```bash
simple-smtp-mailer cli api-provider-remove <provider>
```

**Test API Provider**
```bash
simple-smtp-mailer cli api-provider-test <provider>
```

### Validation Commands

#### Configuration Validation

**Validate Configuration**
```bash
simple-smtp-mailer cli validate-config [--fix] [--verbose]
```

**Test Connections**
```bash
simple-smtp-mailer cli test-connections [--smtp-only] [--api-only] [--domain <domain>]
```

#### Backup and Restore

**Backup Configuration**
```bash
simple-smtp-mailer cli config-backup [--file <backup-file>]
```

**Restore Configuration**
```bash
simple-smtp-mailer cli config-restore --file <backup-file>
```

### Setup Commands

#### Interactive Setup

**Run Setup Wizard**
```bash
simple-smtp-mailer cli setup-wizard
```

**Setup Domain Interactively**
```bash
simple-smtp-mailer cli setup-domain <domain>
```

## Configuration File Structure

The CLI creates and manages configuration files in the following structure:

```
/etc/simple-smtp-mailer/
├── simple-smtp-mailer.conf          # Global configuration
├── domains/                   # Domain configurations
│   ├── example.com.conf
│   └── example.com.service-account.conf
├── users/                     # User configurations
│   ├── user@example.com.conf
│   └── admin@example.com.conf
├── templates/                 # Email templates
│   ├── welcome.conf
│   └── notification.conf
├── address-templates/         # Address templates
│   └── contact-{type}@domain.com.conf
├── oauth2/                    # OAuth2 configurations
│   └── example.com.conf
├── service-accounts/          # Service account files
│   └── example.com.json
├── api-config.conf            # API provider configurations
└── mappings/                  # Address mappings
    └── relay-mappings.conf
```

## Examples

### Complete Setup Example

```bash
# 1. Run setup wizard
simple-smtp-mailer cli setup-wizard

# 2. Add a domain
simple-smtp-mailer cli config-domain-add example.com \
  --smtp-server smtp.gmail.com \
  --port 587 \
  --auth-method LOGIN \
  --username mailer@example.com \
  --password your_app_password

# 3. Add users
simple-smtp-mailer cli config-user-add mailer@example.com \
  --domain example.com

simple-smtp-mailer cli config-user-add admin@example.com \
  --domain example.com \
  --allowed-recipients "support@example.com,customers@*"

# 4. Create email templates
simple-smtp-mailer cli template-create welcome \
  --subject "Welcome to {{service_name}}!" \
  --body "Hello {{user_name}}, welcome to our service!" \
  --html "<h1>Welcome!</h1><p>Hello {{user_name}}, welcome to our service!</p>"

# 5. Create address templates
simple-smtp-mailer cli template-address-create contact-{type}@example.com \
  --domain example.com \
  --types "legal,privacy,general,support,sales"

# 6. Validate configuration
simple-smtp-mailer cli validate-config --verbose

# 7. Add API provider
simple-smtp-mailer cli api-provider-add sendgrid \
  --api-key your_sendgrid_api_key \
  --sender sender@example.com

# 8. Validate configuration
simple-smtp-mailer cli validate-config --verbose

# 9. Test connections
simple-smtp-mailer cli test-connections
```

### OAuth2 Setup Example

```bash
# 1. Setup OAuth2 for Google
simple-smtp-mailer cli auth-oauth2-setup \
  --provider google \
  --domain example.com

# 2. Follow the instructions to complete OAuth2 flow
# 3. Test the configuration
simple-smtp-mailer cli auth-oauth2-test --domain example.com
```

### Service Account Setup Example

```bash
# 1. Add service account
simple-smtp-mailer cli auth-service-account-add \
  --domain example.com \
  --file /path/to/service-account.json

# 2. Test service account
simple-smtp-mailer cli auth-service-account-test --domain example.com
```

### API Provider Setup Example

```bash
# 1. Add SendGrid provider
simple-smtp-mailer cli api-provider-add sendgrid \
  --api-key SG.your_api_key_here \
  --sender sender@yourdomain.com \
  --sender-name "Your Company" \
  --enable-tracking

# 2. List all API providers
simple-smtp-mailer cli api-provider-list

# 3. Show provider details
simple-smtp-mailer cli api-provider-show sendgrid

# 4. Test provider connection
simple-smtp-mailer cli api-provider-test sendgrid

# 5. Remove provider (if needed)
simple-smtp-mailer cli api-provider-remove sendgrid
```

### Domain and User Management Example

```bash
# Enable/disable domains
simple-smtp-mailer cli config-domain-enable example.com
simple-smtp-mailer cli config-domain-disable example.com

# Remove domain
simple-smtp-mailer cli config-domain-remove example.com

# Enable/disable users
simple-smtp-mailer cli config-user-enable user@example.com
simple-smtp-mailer cli config-user-disable user@example.com

# Show user details
simple-smtp-mailer cli config-user-show user@example.com

# Remove user
simple-smtp-mailer cli config-user-remove user@example.com
```

## Troubleshooting

### Common Issues

1. **Permission Denied**: Ensure you have write permissions to `/etc/simple-smtp-mailer/`
2. **Configuration Not Found**: Run `simple-smtp-mailer cli setup-wizard` first
3. **OAuth2 Setup Failed**: Check that client ID and secret are correct
4. **Service Account Invalid**: Verify the JSON file is valid and has required fields

### Debug Mode

Use the `--verbose` flag for detailed output:

```bash
simple-smtp-mailer cli validate-config --verbose
```

### Configuration Validation

Always validate your configuration after making changes:

```bash
simple-smtp-mailer cli validate-config
```

### Testing Connections

Test all connections to ensure everything is working:

```bash
simple-smtp-mailer cli test-connections
```

## Advanced Usage

### Bulk Operations

The CLI supports bulk operations through configuration files. You can create configuration files manually and import them, or export existing configurations for backup.

### Custom Templates

Templates support variable substitution using `{{variable_name}}` syntax. Common variables include:
- `{{user_name}}` - User's name
- `{{service_name}}` - Service name
- `{{domain}}` - Domain name
- `{{date}}` - Current date
- `{{time}}` - Current time

### Address Patterns

Address templates support pattern matching with `{type}` placeholders:
- `contact-{type}@domain.com` matches `contact-legal@domain.com`, `contact-support@domain.com`, etc.
- `{department}@company.com` matches `sales@company.com`, `support@company.com`, etc.

## Daemon Mode

simple-smtp-mailer can run as a background daemon for continuous email processing:

```bash
# Start daemon
simple-smtp-mailer --daemon

# Check daemon status
simple-smtp-mailer --status

# Stop daemon
simple-smtp-mailer --stop

# Reload configuration
simple-smtp-mailer --reload
```

For complete daemon documentation, see [Daemon Mode Guide](../features/daemon-mode.md).

## Integration

The CLI integrates seamlessly with the main simple-smtp-mailer application. Once configured through the CLI, you can use the regular simple-smtp-mailer commands:

```bash
# Send email using configured settings
simple-smtp-mailer send \
  --from user@example.com \
  --to recipient@domain.com \
  --subject "Hello" \
  --body "Hello World!"

# Send using API
simple-smtp-mailer send-api \
  --provider sendgrid \
  --from user@example.com \
  --to recipient@domain.com \
  --subject "Hello" \
  --body "Hello World!"
```

## Security Considerations

1. **File Permissions**: Ensure configuration files have appropriate permissions (600 for sensitive files)
2. **Password Storage**: Passwords are stored in plain text in configuration files
3. **OAuth2 Tokens**: OAuth2 tokens are stored in configuration files and should be protected
4. **Service Account Files**: Keep service account JSON files secure and limit access

## Support

For additional help:
- Run `simple-smtp-mailer cli --help` for general help
- Run `simple-smtp-mailer cli <command> --help` for command-specific help
- Check the main documentation in `/usr/local/share/simple-smtp-mailer/docs/`
