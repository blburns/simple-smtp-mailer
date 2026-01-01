# Ansible Remote Build Guide

This guide explains how to use Ansible to build simple-smtp-mailer on remote virtual machines, including VMware Fusion VMs.

## Overview

The Ansible automation allows you to:
- Build simple-smtp-mailer on multiple remote VMs simultaneously
- Support different Linux distributions (Ubuntu/Debian and CentOS/RHEL/Fedora)
- Automate dependency installation and build processes
- Run builds on different branches and configurations
- Create packages automatically

## Prerequisites

### On Your Local Machine (Mac)

1. **Install Ansible:**
   ```bash
   pip3 install ansible
   ```

2. **Verify installation:**
   ```bash
   ansible --version
   ```

### On Your VMs

1. **Linux Distribution:**
   - Ubuntu 20.04+ or Debian 11+ (for DEB packages)
   - CentOS 8+, RHEL 8+, or Fedora (for RPM packages)

2. **SSH Access:**
   - SSH key authentication configured
   - User with sudo privileges (or passwordless sudo recommended)
   - VMs accessible from your Mac

3. **Network:**
   - Internet access for cloning repository and installing packages

## Quick Start

### 1. Configure Inventory

Edit `automation/ansible/inventory-vmware.ini` with your VM details:

```ini
[build-vms]
# Ubuntu/Debian build VM
BUILD_DEB ansible_host=172.16.4.101 ansible_user=build ansible_ssh_private_key_file=~/.ssh/id_rsa

# CentOS/RHEL build VM
BUILD_RPM ansible_host=172.16.4.102 ansible_user=build ansible_ssh_private_key_file=~/.ssh/id_rsa

[build-vms:vars]
ansible_python_interpreter=/usr/bin/python3
ansible_ssh_common_args='-o StrictHostKeyChecking=no'
ansible_become=yes
ansible_become_method=sudo
```

**Important:** Replace IP addresses, usernames, and SSH key paths with your actual values.

### 2. Configure Sudo (Recommended)

For passwordless sudo, on each VM run:

```bash
sudo visudo
```

Add this line:
```
build ALL=(ALL) NOPASSWD: ALL
```

Replace `build` with your actual username.

### 3. Test Connectivity

```bash
cd automation/ansible
ansible build-vms -i inventory-vmware.ini -m ping
```

You should see successful pings from both VMs.

### 4. Run Your First Build

```bash
./scripts/remote-build.sh
```

This will build on all configured VMs.

## Usage

### Basic Commands

**Build on all VMs:**
```bash
cd automation/ansible
./scripts/remote-build.sh
```

**Build on specific VM:**
```bash
./scripts/remote-build.sh -l BUILD_DEB
```

**Build specific branch:**
```bash
./scripts/remote-build.sh -b develop
```

**Debug build:**
```bash
./scripts/remote-build.sh -t Debug
```

**Clean build:**
```bash
./scripts/remote-build.sh -c
```

**Build with packages:**
```bash
./scripts/remote-build.sh --packages
```

**If sudo requires password:**
```bash
./scripts/remote-build.sh --ask-become-pass
```

### Direct Ansible Usage

You can also run the playbook directly:

```bash
cd automation/ansible

# Build on all VMs
ansible-playbook -i inventory-vmware.ini playbook-build.yml

# Build on specific VM
ansible-playbook -i inventory-vmware.ini playbook-build.yml --limit BUILD_DEB

# Custom variables
ansible-playbook -i inventory-vmware.ini playbook-build.yml \
  -e git_branch=develop \
  -e build_type=Debug \
  -e clean_build=true \
  -e run_tests=false
```

## Configuration Options

### Inventory Variables

In `inventory-vmware.ini`, you can configure:

- `ansible_host`: VM IP address
- `ansible_user`: SSH username
- `ansible_ssh_private_key_file`: Path to SSH private key
- `ansible_become`: Enable sudo (yes/no)
- `ansible_become_method`: Method (sudo/su)
- `ansible_become_pass`: Sudo password (not recommended, use --ask-become-pass instead)

### Playbook Variables

The playbook accepts these variables:

- `git_branch`: Git branch to build (default: `main`)
- `build_type`: Build type - `Release` or `Debug` (default: `Release`)
- `clean_build`: Clean build directory first (default: `false`)
- `run_tests`: Run tests after build (default: `true`)
- `create_packages`: Create packages after build (default: `false`)

## Build Process

The Ansible playbook performs these steps:

1. **Gather OS Information** - Detects Linux distribution
2. **Update Package Cache** - Updates apt/yum repositories
3. **Install Dependencies** - Installs build tools and libraries:
   - Build tools (gcc, cmake, make)
   - Libraries (OpenSSL, jsoncpp, curl, yaml-cpp)
   - Testing frameworks (googletest)
   - Python 3 and requests library
4. **Clone Repository** - Clones or updates the git repository
5. **Configure CMake** - Runs CMake configuration
6. **Build Project** - Compiles the project
7. **Run Tests** - Executes test suite (optional)
8. **Create Packages** - Generates DEB/RPM packages (optional)

## Build Artifacts

After a successful build, artifacts are located at:

```
/opt/simple-smtp-mailer/build/simple-smtp-mailer
```

On each VM.

## Troubleshooting

### SSH Connection Issues

**Problem:** Cannot connect to VMs

**Solutions:**
1. Verify VM IP addresses: `ping <vm_ip>`
2. Test SSH manually: `ssh -i ~/.ssh/id_rsa user@vm_ip`
3. Check SSH key permissions: `chmod 600 ~/.ssh/id_rsa`
4. Ensure VMs are running
5. Check firewall settings

### Permission Errors

**Problem:** Permission denied errors

**Solutions:**
1. Configure passwordless sudo (recommended):
   ```bash
   sudo visudo
   # Add: build ALL=(ALL) NOPASSWD: ALL
   ```

2. Or use `--ask-become-pass`:
   ```bash
   ./scripts/remote-build.sh --ask-become-pass
   ```

### Package Installation Failures

**Problem:** Cannot install dependencies

**Solutions:**
1. Update package cache manually:
   ```bash
   ansible build-vms -i inventory-vmware.ini -m apt -a "update_cache=yes" --become
   ```

2. Check internet connectivity on VMs

3. Verify repository access

### Build Failures

**Problem:** Build fails with compilation errors

**Solutions:**
1. Check build logs in Ansible output
2. SSH into VM and build manually:
   ```bash
   ssh build@vm_ip
   cd /opt/simple-smtp-mailer/build
   cmake ..
   make
   ```
3. Verify all dependencies are installed
4. Check disk space: `df -h`
5. Ensure you're building from a clean state: `./scripts/remote-build.sh -c`

### Python Package Issues

**Problem:** Python pip installation fails (externally-managed-environment)

**Solutions:**
- The playbook automatically handles this by:
  - Using `apt install python3-requests` on Ubuntu/Debian
  - Using `--break-system-packages` flag on CentOS/RHEL
  - Falling back to user installation if needed

## Advanced Usage

### Custom Build Directory

Edit `playbook-build.yml` and change:

```yaml
project_dir: /opt/simple-smtp-mailer
```

### Custom Repository

Use a different repository:

```bash
ansible-playbook -i inventory-vmware.ini playbook-build.yml \
  -e git_repo_url=https://github.com/yourusername/yourrepo.git
```

### Parallel Builds

Ansible runs builds in parallel by default. Control parallelism:

```bash
ansible-playbook -i inventory-vmware.ini playbook-build.yml -f 10
```

### Fetch Build Artifacts

To copy build artifacts to your local machine, edit `playbook-build.yml` and set:

```yaml
- name: Copy build artifacts to local machine (optional)
  fetch:
    src: "{{ project_dir }}/build/simple-smtp-mailer"
    dest: "{{ playbook_dir }}/../dist/{{ inventory_hostname }}-simple-smtp-mailer"
    flat: yes
  when: true  # Change from false to true
```

## File Structure

```
automation/ansible/
├── inventory-vmware.ini      # VM inventory configuration
├── playbook-build.yml        # Main build playbook
├── scripts/
│   └── remote-build.sh       # Convenience script
└── README-REMOTE-BUILD.md   # Quick reference (in automation directory)
```

## Best Practices

1. **Use Passwordless Sudo** - Configure NOPASSWD sudo for seamless automation
2. **Keep VMs Updated** - Regularly update package caches and system packages
3. **Use Specific Branches** - Build from specific branches, not always `main`
4. **Clean Builds** - Use `-c` flag for clean builds when switching branches
5. **Test Before Packaging** - Run tests before creating packages
6. **Version Control** - Keep inventory files in version control (but exclude passwords)

## Integration with CI/CD

The Ansible playbook can be integrated into CI/CD pipelines:

```yaml
# Example GitHub Actions
- name: Build on remote VMs
  run: |
    cd automation/ansible
    ./scripts/remote-build.sh -b ${{ github.ref_name }} --packages
```

## Support

For issues or questions:
- Check the main project README
- Review Ansible playbook output for detailed error messages
- SSH into VMs to debug issues directly
- Check the troubleshooting section above

## Related Documentation

- [Linux Build Guide](./linux-build.md) - Manual Linux build instructions
- [Build Scripts](./build-scripts.md) - Local build script documentation
- [Installation Guide](../installation/linux.md) - Linux installation instructions

