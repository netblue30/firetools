AC_PREREQ([2.68])
AC_INIT(firetools, 0.9.39, netblue30@yahoo.com, , http://firejail.sourceforge.net)
AC_CONFIG_SRCDIR([src/firetools/main.cpp])
#AC_CONFIG_HEADERS([config.h])

m4_include([pkg.m4])

AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_RANLIB

# checking pthread library
AC_CHECK_LIB([pthread], [main], [], AC_MSG_ERROR([*** POSIX thread support not installed - please install first ***]))
AC_CHECK_HEADER(pthread.h,,AC_MSG_ERROR([*** POSIX thread support not installed - please install first ***]))

# testing pkg-config
PKGCONFIG=`which pkg-config`
if test -z "$PKGCONFIG"
then
	AC_MSG_ERROR([pkg-config is missing, please install it (https://www.freedesktop.org/wiki/Software/pkg-config/).])
fi
PKG_CHECK_MODULES(QT5, [Qt5Widgets], [], AC_MSG_ERROR([*** Qt5 not installed]))

# testing qmake
QMAKE=`which qmake`
if test -z "$QMAKE"
then
	# fix for Fedora
	if test -f /usr/lib64/qt4/bin/qmake; then
		QMAKE=/usr/lib64/qt4/bin/qmake
	fi
	
	if test -z "$QMAKE"
	then
		AC_MSG_ERROR([qmake is missing, please install Qt4 or Qt5 development packages.])
	fi
fi
echo "Found $QMAKE"
$QMAKE -v

cd src/firetools && $QMAKE && cd ../..
cd src/firemgr && $QMAKE && cd ../..

# build sanitizer.sh file
echo "#!/bin/bash" >sanitizer.sh
echo "export CFLAGS=-fsanitize=address" >> sanitizer.sh
echo "export CXXFLAGS=-fsanitize=address" >> sanitizer.sh
echo "export LDFLAGS=-lasan" >> sanitizer.sh
echo "cd src/firetools && $QMAKE && cd ../.." >> sanitizer.sh
chmod +x sanitizer.sh

# testing firejail
FIREJAIL=`which firejail`
if test -z "$FIREJAIL"
then
	AC_MSG_WARN([Firejail is required for running firetools, please install it (http://firejail.sf.net/).])
else
	echo "Found $FIREJAIL"
fi

# testing xterm
XTERM=`which xterm`
if test -z "$XTERM"
then
	AC_MSG_WARN([xterm is required for running firetools, please install it.])
else
	echo "Found $XTERM"
fi

cp confdefs.h firetools_config.h
AC_OUTPUT(Makefile)
