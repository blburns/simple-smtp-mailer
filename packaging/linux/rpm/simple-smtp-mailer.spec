Name:           simple-smtp-mailer
Version:        0.2.0
Release:        1%{?dist}
Summary:        High-performance SMTP mailer with queue management and OAuth2 support
License:        Apache-2.0
URL:            https://github.com/blburns/simple-smtp-mailer
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake >= 3.16
BuildRequires:  gcc-c++
BuildRequires:  openssl-devel
BuildRequires:  jsoncpp-devel
BuildRequires:  libcurl-devel
BuildRequires:  yaml-cpp-devel
BuildRequires:  python3
BuildRequires:  python3-pip
Requires:       openssl-libs >= 3.0
Requires:       jsoncpp
Requires:       libcurl
Requires:       python3-requests

%description
simple-smtp-mailer is a modern, high-performance SMTP mailer with advanced features
including queue management, OAuth2 authentication, and multi-provider API support.

Features:
* High-performance email delivery
* Queue management for reliable delivery
* OAuth2 authentication support
* Multi-provider API support (Gmail, SendGrid, Mailgun, etc.)
* Systemd integration
* JSON logging
* Webhook support for email events

%prep
%setup -q

%build
mkdir -p build
cd build
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DENABLE_TESTS=OFF \
    -DENABLE_PACKAGING=OFF \
    -DENABLE_SSL=ON \
    -DENABLE_JSON=ON \
    -DENABLE_CURL=ON
make %{?_smp_mflags}

%install
mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_sysconfdir}/%{name}
mkdir -p %{buildroot}%{_localstatedir}/log/%{name}
mkdir -p %{buildroot}%{_localstatedir}/lib/%{name}
mkdir -p %{buildroot}%{_unitdir}
mkdir -p %{buildroot}%{_datadir}/%{name}/oauth2-helper
mkdir -p %{buildroot}%{_datadir}/doc/%{name}

# Install binary
install -m 755 build/%{name} %{buildroot}%{_bindir}/%{name}

# Install configuration files
install -m 644 config/simple-smtp-mailer.conf.example %{buildroot}%{_sysconfdir}/%{name}/simple-smtp-mailer.conf.example
if [ -d config/conf.d ]; then
    mkdir -p %{buildroot}%{_sysconfdir}/%{name}/conf.d
    install -m 644 config/conf.d/*.example %{buildroot}%{_sysconfdir}/%{name}/conf.d/ 2>/dev/null || true
fi

# Install systemd service file
install -m 644 deployment/systemd/%{name}.service %{buildroot}%{_unitdir}/%{name}.service

# Install OAuth2 helper tools
if [ -d tools/oauth2-helper ]; then
    cp -r tools/oauth2-helper/* %{buildroot}%{_datadir}/%{name}/oauth2-helper/ 2>/dev/null || true
fi

# Install documentation
install -m 644 LICENSE %{buildroot}%{_datadir}/doc/%{name}/LICENSE
install -m 644 README.md %{buildroot}%{_datadir}/doc/%{name}/README.md

%pre
# Create service user if it doesn't exist
if [ "$1" -eq 1 ]; then
    if ! id -u simple-smtp-mailer &>/dev/null; then
        useradd -r -s /sbin/nologin -d /var/lib/%{name} -c "%{name} service user" simple-smtp-mailer 2>/dev/null || true
    fi
fi

%post
systemctl daemon-reload || true
# Don't auto-start on install, let user configure first
# systemctl enable %{name} || true
# systemctl start %{name} || true

%preun
if [ "$1" -eq 0 ]; then
    systemctl stop %{name} 2>/dev/null || true
    systemctl disable %{name} 2>/dev/null || true
fi

%postun
systemctl daemon-reload || true

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%config(noreplace) %{_sysconfdir}/%{name}/simple-smtp-mailer.conf.example
%{_sysconfdir}/%{name}/conf.d/*.example
%{_unitdir}/%{name}.service
%{_datadir}/%{name}/oauth2-helper/*
%doc %{_datadir}/doc/%{name}/LICENSE
%doc %{_datadir}/doc/%{name}/README.md
%license %{_datadir}/doc/%{name}/LICENSE

%changelog
* Wed Jan 01 2025 blburns <support@example.com> - 0.2.0-1
- Initial package release
- High-performance SMTP mailer with OAuth2 support
- Queue management and multi-provider API support

