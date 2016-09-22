%define        __spec_install_post %{nil}
%define          debug_package %{nil}
%define        __os_install_post %{_dbpath}/brp-compress

Summary: Firejail graphical user interface
Name: firetools
Version: FIRETOOLSVERSION
Release: 1%{?dist}
License: GPL+
Group: Development/Tools
SOURCE0 : %{name}-%{version}.tar.xz
URL: http://firejail.sourceforege.net

BuildRequires: qt5-qtbase-devel
Requires:      firejail qt5-qtsvg

%description
Firetools is the graphical user interface of Firejail.
Firejail is a SUID sandbox program that reduces the risk of security breaches
by restricting the running environment of untrusted applications using Linux
namespaces, seccomp-bpf and Linux capabilities. It allows a process and all
its descendants to have their  own  private view of the globally  shared  kernel
resources, such as the network stack, process table, mount table.  Firejail can
work in a SELinux or AppArmor environment, and it is integrated with Linux
Control Groups.

%prep
%setup -q


%build
%configure --with-qmake=/usr/bin/qmake-qt5
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
%make_install
rm -rf $RPM_BUILD_ROOT/%{_docdir}/


%files
%defattr(-,root,root,-)
%doc COPYING README RELNOTES
%{_bindir}/*
%{_mandir}/*
%{_datadir}/applications/firetools.desktop
%{_datadir}/pixmaps/firetools.png
 

%changelog
* Wed Sep 21 2016 Warren Togami <wtogami@gmail.com> 0.9.40.1-1
- clean up rpm spec to roughly Fedora Packaging Guidelines
- easy self-contained build from git repo with ./mkrpm.sh as a non-root user

* Wed Jun 15 2016 netblue30 <netblue30@yahoo.com> 0.9.40.1-1
- bugfixes

* Sun May 29 2016 netblue30 <netblue30@yahoo.com> 0.9.40-1
 - Grsecurity support
 - updated the default application list
 - sandbox file manager (firemgr) application
 - protocols and cpu cores support
 - sandbox name support
 - X11 dispaly support
 - bugfixes

* Sat Oct 3 2015 netblue30 <netblue30@yahoo.com> 0.9.30-1
 - 1h and 12h statistics support
 - user namespaces support
 - QT5 support
 - applist update
 - bugfixes

* Mon Jun 15 2015  netblue30 <netblue30@yahoo.com> 0.9.26.1
 - First rpm package release

