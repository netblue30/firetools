# Firetools

Firetools is the graphical user interface of Firejail security sandbox. It provides a sandbox launcher
integrated with the system tray, sandbox editing, management and statistics. The application is built
using Qt5 library.

Home page: https://firejailtools.wordpress.com

Download: http://sourceforge.net/projects/firejail/files/firetools/

Travis-CI status: https://travis-ci.org/netblue30/firetools


<table><tr>

<td>
<a href="https://odysee.com/@netblue30:9/firetools:6" target="_blank">
<img src="https://thumbs.odycdn.com/f696288045a51bc504e00c35b0a3b206.png"
alt="Firetools Demo" width="240" height="142" border="10" /><br/>Firetools Demo</a>
</td>

<td>
<a href="https://odysee.com/@netblue30:9/intro:ceb" target="_blank">
<img src="https://thumbs.odycdn.com/a909846bdce7992f1aacceb0dcc8898b.png"
alt="Firejail Introduction" width="240" height="142" border="10" /><br/>Firejail Introduction</a>
</td>
</tr></table>

## Setting up a compilation environment:
`````
(Debian/Ubuntu)
$ sudo apt-get install build-essential qt5-default qt5-qmake qtbase5-dev-tools libqt5svg5 git

(CentOS 7)
$ sudo yum install gcc-c++ qt5-qtbase-devel qt5-qtsvg.x86_64 git
`````

## Compile & Install

`````
$ git clone https://github.com/netblue30/firetools
$ cd firetools

(Debian/Ubuntu)
$ ./configure

(CentOS 7)
./configure --with-qmake=/usr/lib64/qt5/bin/qmake

$ make
$ sudo make install-strip
`````

## Usage:
`````
FIRETOOLS(1)                                firetools man page                               FIRETOOLS(1)

NAME
       Firetools - Graphical tools collection for Firejail security sandbox

SYNOPSIS
       firetools [OPTIONS]

DESCRIPTION
       Firetools is a GUI application for Firejail.  It offers a system tray launcher for sandboxed apps,
       sandbox editing, management, and statistics.  The software package also includes a sandbox config‐
       uration wizard, firejail-ui.

       The list of applications recognized automatically by Firetools is stored in /usr/lib/firetools/ap‐
       plist.  To add more applications to the list drop a similar file in your home directory in ~/.con‐
       fig/firetools/uiapps.

OPTIONS
       --autostart
              Configure firetools to run automatically in system tray when X11 session is started.

       --debug
              Print debug messages.

       -?, --help
              Print options end exit.

       --version
              Print software version and exit.

CONFIGURATION
       /usr/lib/firetools/uiapps  file  contains  the default list of applications recognized by default.
       The user can add more applications by creating a simillar file  in  ~/.config/firetools/uiapps  in
       user home directory. Each line describes an application as follows:

              executable; description; icon; (optional) firejail command

       Some examples:

              inkscape;Inkscape SVG Editor;inkscape
              calibre;eBook Reader;/usr/share/calibre/images/lt.png
              mpv;MPV;mpv;firejail mpv --player-operation-mode=pseudo-gui

ABOUT FIREJAIL
       Firejail  is  a SUID sandbox program that reduces the risk of security breaches by restricting the
       running environment of untrusted applications using Linux namespaces, seccomp-bpf and Linux  capa‐
       bilities.  It allows a process and all its descendants to have their own private view of the glob‐
       ally shared kernel resources, such as the network stack, process table, mount table.  Firejail can
       work in a SELinux or AppArmor environment, and it is integrated with Linux Control Groups.


LICENSE
       This program is free software; you can redistribute it and/or modify it under the terms of the GNU
       General Public License as published by the Free Software Foundation; either version 2 of  the  Li‐
       cense, or (at your option) any later version.

       Homepage: http://firejail.wordpress.com

SEE ALSO
       firejail(1), firejail-ui(1),
`````
