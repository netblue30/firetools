#!/bin/bash
VER="0.9.40.1"

cd ~
rm -fr rpmbuild

mkdir -p ~/rpmbuild/{RPMS,SRPMS,BUILD,SOURCES,SPECS,tmp}
cat <<EOF >~/.rpmmacros
%_topdir   %(echo $HOME)/rpmbuild
%_tmppath  %{_topdir}/tmp
EOF

cd ~/rpmbuild

mkdir -p firetools-$VER/usr/bin
install -m 755 /usr/bin/firetools firetools-$VER/usr/bin/.
install -m 755 /usr/bin/firemgr firetools-$VER/usr/bin/.

mkdir -p firetools-$VER/usr/share/man/man1
install -m 644 /usr/share/man/man1/firetools.1.gz firetools-$VER/usr/share/man/man1/.
install -m 644 /usr/share/man/man1/firemgr.1.gz firetools-$VER/usr/share/man/man1/.

mkdir -p firetools-$VER/usr/share/doc/firetools
install -m 644 /usr/share/doc/firetools/COPYING firetools-$VER/usr/share/doc/firetools/.
install -m 644 /usr/share/doc/firetools/README firetools-$VER/usr/share/doc/firetools/.
install -m 644 /usr/share/doc/firetools/RELNOTES firetools-$VER/usr/share/doc/firetools/.

mkdir -p firetools-$VER/usr/share/applications/
install -m 644 /usr/share/applications/firetools.desktop firetools-$VER/usr/share/applications/.
mkdir -p  firetools-$VER/usr/share/pixmaps
install -m 644 /usr/share/pixmaps/firetools.png firetools-$VER/usr/share/pixmaps/.

tar -czvf firetools-$VER.tar.gz firetools-$VER
cp firetools-$VER.tar.gz SOURCES/.

cat <<EOF > SPECS/firetools.spec
%define        __spec_install_post %{nil}
%define          debug_package %{nil}
%define        __os_install_post %{_dbpath}/brp-compress

Summary: Firejail user interface
Name: firetools
Version: $VER
Release: 1
License: GPL+
Group: Development/Tools
SOURCE0 : %{name}-%{version}.tar.gz
URL: http://firejail.sourceforege.net

BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

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

%install
rm -rf %{buildroot}
mkdir -p  %{buildroot}

cp -a * %{buildroot}


%clean
rm -rf %{buildroot}


%files
%defattr(-,root,root,-)
%{_bindir}/*
%{_docdir}/*
%{_mandir}/*
/usr/share/applications/firetools.desktop
/usr/share/pixmaps/firetools.png
 

%changelog
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

EOF


rpmbuild -ba SPECS/firetools.spec
rpm -qpl RPMS/x86_64/firetools-$VER-1.x86_64.rpm
cd ..
rm -f firetools-$VER-1.x86_64.rpm
cp rpmbuild/RPMS/x86_64/firetools-$VER-1.x86_64.rpm .

