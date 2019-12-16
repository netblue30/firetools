# Firetools

Firetools is the graphical user interface of Firejail security sandbox. It provides a sandbox launcher
integrated with the system tray, sandbox editing, management and statistics. The application is built
using Qt5 library.

Home page: https://firejailtools.wordpress.com

Download: http://sourceforge.net/projects/firejail/files/firetools/

Travis-CI status: https://travis-ci.org/netblue30/firetools

## Firejail GUI Demo

<table><tr>

<td>
<a href="http://www.youtube.com/watch?feature=player_embedded&v=7RMz7tePA98
" target="_blank"><img src="http://img.youtube.com/vi/7RMz7tePA98/0.jpg"
alt="Firejail Intro video" width="240" height="180" border="10" /><br/>Firejail Intro</a>
</td>

<td>
<a href="http://www.youtube.com/watch?feature=player_embedded&v=J1ZsXrpAgBU
" target="_blank"><img src="http://img.youtube.com/vi/J1ZsXrpAgBU/0.jpg"
alt="Firejail Intro video" width="240" height="180" border="10" /><br/>Firejail Demo</a>
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
$ git clone  https://github.com/netblue30/firetools
$ cd firetools

(Debian/Ubuntu)
$ ./configure

(CentOS 7)
./configure --with-qmake=/usr/lib64/qt5/bin/qmake

$ make
$ sudo make install-strip
`````

