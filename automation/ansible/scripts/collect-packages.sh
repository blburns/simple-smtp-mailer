#!/bin/bash
# Collect packages from remote VMs to local dist directory
# This script fetches all built packages (DEB, RPM, TGZ, ZIP) from the Ansible VMs
# and organizes them in a local dist directory for GitHub release

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ANSIBLE_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
PROJECT_ROOT="$(cd "$ANSIBLE_DIR/../.." && pwd)"
DIST_DIR="$PROJECT_ROOT/dist"
INVENTORY_FILE="$ANSIBLE_DIR/inventory-vmware.ini"
PROJECT_NAME="simple-smtp-mailer"
REMOTE_PROJECT_DIR="/opt/simple-smtp-mailer"
REMOTE_DIST_DIR="$REMOTE_PROJECT_DIR/dist"
REMOTE_BUILD_DIR="$REMOTE_PROJECT_DIR/build"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Print functions
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check prerequisites
check_prerequisites() {
    print_info "Checking prerequisites..."
    
    if ! command -v ansible &> /dev/null; then
        print_error "ansible is not installed. Install with: pip3 install ansible"
        exit 1
    fi
    
    if [ ! -f "$INVENTORY_FILE" ]; then
        print_error "Inventory file not found: $INVENTORY_FILE"
        exit 1
    fi
    
    print_success "Prerequisites check passed"
}

# Create local dist directory structure
create_dist_structure() {
    print_info "Creating local dist directory structure..."
    
    mkdir -p "$DIST_DIR"/{linux,source}
    mkdir -p "$DIST_DIR/linux"/{deb,rpm,archive}
    
    print_success "Dist directory structure created: $DIST_DIR"
}

# Fetch packages from remote VMs
fetch_packages() {
    print_info "Fetching packages from remote VMs..."
    
    # Create a temporary playbook for fetching packages
    TEMP_PLAYBOOK=$(mktemp)
    cat > "$TEMP_PLAYBOOK" << 'PLAYBOOK_EOF'
---
- name: Collect packages from remote VMs
  hosts: build-vms
  gather_facts: no
  vars:
    project_dir: "/opt/simple-smtp-mailer"
    remote_dist_dir: "/opt/simple-smtp-mailer/dist"
    remote_build_dir: "/opt/simple-smtp-mailer/build"
    local_dist_dir: "{{ playbook_dir }}/../../dist"
    
  tasks:
    - name: Check if dist directory exists on remote
      stat:
        path: "{{ remote_dist_dir }}"
      register: dist_dir_stat
      
    - name: Check if build directory exists on remote
      stat:
        path: "{{ remote_build_dir }}"
      register: build_dir_stat
      
    - name: List packages in dist directory
      find:
        paths: "{{ remote_dist_dir }}"
        patterns:
          - "*.deb"
          - "*.rpm"
          - "*.tar.gz"
          - "*.zip"
        excludes: "*.tar.Z"
      register: dist_packages
      when: dist_dir_stat.stat.exists
      
    - name: List packages in build directory (fallback)
      find:
        paths: "{{ remote_build_dir }}"
        patterns:
          - "*.deb"
          - "*.rpm"
          - "*.tar.gz"
          - "*.zip"
        excludes: "*.tar.Z"
      register: build_packages
      when: build_dir_stat.stat.exists
      
    - name: Fetch DEB packages from dist
      fetch:
        src: "{{ item.path }}"
        dest: "{{ local_dist_dir }}/linux/deb/{{ inventory_hostname }}-{{ item.path | basename }}"
        flat: yes
      loop: "{{ dist_packages.files | default([]) }}"
      when: 
        - dist_packages.files is defined
        - item.path | regex_search('\.deb$')
        
    - name: Fetch RPM packages from dist
      fetch:
        src: "{{ item.path }}"
        dest: "{{ local_dist_dir }}/linux/rpm/{{ inventory_hostname }}-{{ item.path | basename }}"
        flat: yes
      loop: "{{ dist_packages.files | default([]) }}"
      when: 
        - dist_packages.files is defined
        - item.path | regex_search('\.rpm$')
        
    - name: Fetch archive packages (TGZ, ZIP) from dist
      fetch:
        src: "{{ item.path }}"
        dest: "{{ local_dist_dir }}/linux/archive/{{ inventory_hostname }}-{{ item.path | basename }}"
        flat: yes
      loop: "{{ dist_packages.files | default([]) }}"
      when: 
        - dist_packages.files is defined
        - item.path | regex_search('\.(tar\.gz|zip)$')
        
    - name: Fetch DEB packages from build (fallback)
      fetch:
        src: "{{ item.path }}"
        dest: "{{ local_dist_dir }}/linux/deb/{{ inventory_hostname }}-{{ item.path | basename }}"
        flat: yes
      loop: "{{ build_packages.files | default([]) }}"
      when: 
        - build_packages.files is defined
        - item.path | regex_search('\.deb$')
        - dist_packages.files | default([]) | length == 0
        
    - name: Fetch RPM packages from build (fallback)
      fetch:
        src: "{{ item.path }}"
        dest: "{{ local_dist_dir }}/linux/rpm/{{ inventory_hostname }}-{{ item.path | basename }}"
        flat: yes
      loop: "{{ build_packages.files | default([]) }}"
      when: 
        - build_packages.files is defined
        - item.path | regex_search('\.rpm$')
        - dist_packages.files | default([]) | length == 0
        
    - name: Fetch archive packages from build (fallback)
      fetch:
        src: "{{ item.path }}"
        dest: "{{ local_dist_dir }}/linux/archive/{{ inventory_hostname }}-{{ item.path | basename }}"
        flat: yes
      loop: "{{ build_packages.files | default([]) }}"
      when: 
        - build_packages.files is defined
        - item.path | regex_search('\.(tar\.gz|zip)$')
        - dist_packages.files | default([]) | length == 0
PLAYBOOK_EOF

    # Run the playbook
    if ansible-playbook -i "$INVENTORY_FILE" "$TEMP_PLAYBOOK"; then
        print_success "Packages fetched successfully"
    else
        print_error "Failed to fetch packages"
        rm -f "$TEMP_PLAYBOOK"
        exit 1
    fi
    
    # Clean up
    rm -f "$TEMP_PLAYBOOK"
}

# Organize and rename packages
organize_packages() {
    print_info "Organizing packages..."
    
    # Remove hostname prefix from package names
    for dir in "$DIST_DIR/linux"/{deb,rpm,archive}; do
        if [ -d "$dir" ]; then
            cd "$dir"
            for file in *; do
                if [ -f "$file" ] && [[ "$file" =~ ^(BUILD_DEB|BUILD_RPM)- ]]; then
                    new_name="${file#BUILD_DEB-}"
                    new_name="${new_name#BUILD_RPM-}"
                    if [ "$file" != "$new_name" ]; then
                        mv "$file" "$new_name"
                        print_info "Renamed: $file -> $new_name"
                    fi
                fi
            done
        fi
    done
    
    # Create symlinks or copies in main linux directory
    cd "$DIST_DIR/linux"
    for file in deb/*.deb rpm/*.rpm archive/*.{tar.gz,zip} 2>/dev/null; do
        if [ -f "$file" ]; then
            filename=$(basename "$file")
            if [ ! -f "$filename" ]; then
                ln -s "${file#linux/}" "$filename" 2>/dev/null || cp "$file" "$filename"
            fi
        fi
    done
    
    print_success "Packages organized"
}

# Display summary
display_summary() {
    print_info "Package collection summary:"
    echo ""
    
    local total=0
    local count=0
    
    echo "DEB packages:"
    if ls "$DIST_DIR/linux/deb"/*.deb 1> /dev/null 2>&1; then
        for file in "$DIST_DIR/linux/deb"/*.deb; do
            if [ -f "$file" ]; then
                size=$(stat -f%z "$file" 2>/dev/null || stat -c%s "$file" 2>/dev/null || echo "0")
                echo "  $(basename "$file") ($(numfmt --to=iec-i --suffix=B "$size" 2>/dev/null || echo "${size}B"))"
                total=$((total + size))
                count=$((count + 1))
            fi
        done
    else
        echo "  (none)"
    fi
    
    echo ""
    echo "RPM packages:"
    if ls "$DIST_DIR/linux/rpm"/*.rpm 1> /dev/null 2>&1; then
        for file in "$DIST_DIR/linux/rpm"/*.rpm; do
            if [ -f "$file" ]; then
                size=$(stat -f%z "$file" 2>/dev/null || stat -c%s "$file" 2>/dev/null || echo "0")
                echo "  $(basename "$file") ($(numfmt --to=iec-i --suffix=B "$size" 2>/dev/null || echo "${size}B"))"
                total=$((total + size))
                count=$((count + 1))
            fi
        done
    else
        echo "  (none)"
    fi
    
    echo ""
    echo "Archive packages (TGZ, ZIP):"
    if ls "$DIST_DIR/linux/archive"/*.{tar.gz,zip} 1> /dev/null 2>&1; then
        for file in "$DIST_DIR/linux/archive"/*.{tar.gz,zip}; do
            if [ -f "$file" ]; then
                size=$(stat -f%z "$file" 2>/dev/null || stat -c%s "$file" 2>/dev/null || echo "0")
                echo "  $(basename "$file") ($(numfmt --to=iec-i --suffix=B "$size" 2>/dev/null || echo "${size}B"))"
                total=$((total + size))
                count=$((count + 1))
            fi
        done
    else
        echo "  (none)"
    fi
    
    echo ""
    if [ $count -gt 0 ]; then
        print_success "Total: $count package(s), $(numfmt --to=iec-i --suffix=B "$total" 2>/dev/null || echo "${total}B")"
        echo ""
        print_info "Packages are ready in: $DIST_DIR"
        echo ""
        print_info "Directory structure:"
        tree -L 3 "$DIST_DIR" 2>/dev/null || find "$DIST_DIR" -type f | head -20
    else
        print_warning "No packages found. Make sure packages were built on the VMs."
    fi
}

# Main execution
main() {
    print_info "=== Package Collection Script ==="
    echo ""
    
    check_prerequisites
    create_dist_structure
    fetch_packages
    organize_packages
    display_summary
    
    print_success "Package collection completed!"
}

# Run main function
main "$@"

