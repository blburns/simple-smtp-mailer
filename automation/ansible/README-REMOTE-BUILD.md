# Remote Build Setup for VMware Fusion VMs

This guide explains how to set up and use Ansible to build simple-smtp-mailer on remote VMware Fusion virtual machines.

## Prerequisites

1. **Ansible installed on your Mac:**
   ```bash
   pip3 install ansible
   ```

2. **SSH access to your VMs:**
   - SSH key authentication set up
   - VMs accessible from your Mac
   - VMs have internet access (for cloning repo and installing packages)

3. **VM Requirements:**
   - Linux distribution (Ubuntu/Debian or CentOS/RHEL/Fedora)
   - User with sudo privileges
   - Python 3 installed

## Setup Instructions

### 1. Configure Inventory File

Edit `inventory-vmware.ini` and update with your VM details:

```ini
[build-vms]
vm1 ansible_host=192.168.1.100 ansible_user=your_username ansible_ssh_private_key_file=~/.ssh/id_rsa
vm2 ansible_host=192.168.1.101 ansible_user=your_username ansible_ssh_private_key_file=~/.ssh/id_rsa
```

**Important:** Replace:
- `192.168.1.100` and `192.168.1.101` with your actual VM IP addresses
- `your_username` with the username on your VMs
- `~/.ssh/id_rsa` with the path to your SSH private key (if different)

### 2. Test Connectivity

Test that you can connect to your VMs:

```bash
cd automation/ansible
ansible build-vms -i inventory-vmware.ini -m ping
```

You should see successful pings from both VMs.

### 3. Find Your VM IP Addresses

If you don't know your VM IP addresses:

**From your Mac:**
```bash
# Check VMware Fusion network settings
# Or use arp to find VMs on your network
arp -a | grep -i vmware
```

**From inside the VM:**
```bash
# Ubuntu/Debian
ip addr show
# or
hostname -I

# CentOS/RHEL
ip addr
```

## Usage

### Basic Build

Build on all VMs:

```bash
cd automation/ansible
./scripts/remote-build.sh
```

### Build on Specific VM

Build only on vm1:

```bash
./scripts/remote-build.sh -l vm1
```

### Build Specific Branch

Build the `develop` branch:

```bash
./scripts/remote-build.sh -b develop
```

### Debug Build

Build in Debug mode:

```bash
./scripts/remote-build.sh -t Debug
```

### Clean Build

Clean build directory before building:

```bash
./scripts/remote-build.sh -c
```

### Build with Packages

Build and create packages:

```bash
./scripts/remote-build.sh --packages
```

### List Available Hosts

```bash
./scripts/remote-build.sh --list-hosts
```

### Combined Options

```bash
# Clean build of develop branch in Debug mode on vm1 only
./scripts/remote-build.sh -c -b develop -t Debug -l vm1
```

## Direct Ansible Usage

You can also run the playbook directly:

```bash
cd automation/ansible

# Build on all VMs
ansible-playbook -i inventory-vmware.ini playbook-build.yml

# Build on specific VM
ansible-playbook -i inventory-vmware.ini playbook-build.yml --limit vm1

# Build with custom variables
ansible-playbook -i inventory-vmware.ini playbook-build.yml \
  -e git_branch=develop \
  -e build_type=Debug \
  -e clean_build=true
```

## Build Artifacts

After a successful build, the binary will be located at:

```
/opt/simple-smtp-mailer/build/simple-smtp-mailer
```

On each VM.

## Troubleshooting

### SSH Connection Issues

**Problem:** Cannot connect to VMs

**Solutions:**
1. Verify VM IP addresses are correct
2. Check SSH key authentication:
   ```bash
   ssh -i ~/.ssh/id_rsa your_username@192.168.1.100
   ```
3. Ensure VMs are running
4. Check firewall settings on VMs

### Permission Issues

**Problem:** Permission denied errors

**Solutions:**
1. Ensure the user has sudo privileges:
   ```bash
   ansible build-vms -i inventory-vmware.ini -m command -a "sudo -v" --become
   ```
2. Check sudoers configuration on VMs

### Package Installation Issues

**Problem:** Cannot install dependencies

**Solutions:**
1. Update package cache manually:
   ```bash
   ansible build-vms -i inventory-vmware.ini -m apt -a "update_cache=yes" --become
   ```
2. Check internet connectivity on VMs
3. Verify repository access

### Build Failures

**Problem:** Build fails with errors

**Solutions:**
1. Check build logs in Ansible output
2. SSH into VM and check:
   ```bash
   ssh your_username@vm_ip
   cd /opt/simple-smtp-mailer/build
   cmake ..
   make
   ```
3. Verify all dependencies are installed
4. Check disk space: `df -h`

## Advanced Configuration

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

Build on multiple VMs in parallel (Ansible does this by default):

```bash
ansible-playbook -i inventory-vmware.ini playbook-build.yml -f 10
```

## File Structure

```
automation/ansible/
├── inventory-vmware.ini      # VM inventory configuration
├── playbook-build.yml        # Build playbook
├── scripts/
│   └── remote-build.sh      # Convenience script
└── README-REMOTE-BUILD.md   # This file
```

## Next Steps

1. Update `inventory-vmware.ini` with your VM details
2. Test connectivity: `ansible build-vms -i inventory-vmware.ini -m ping`
3. Run your first build: `./scripts/remote-build.sh`
4. Check build artifacts on VMs

## Support

For issues or questions:
- Check the main project README
- Review Ansible playbook output for errors
- SSH into VMs to debug issues directly

