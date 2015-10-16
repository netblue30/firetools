AC_PREREQ([2.68])
AC_INIT(firetools, 0.9.30, netblue30@yahoo.com, , http://firejail.sourceforge.net)
AC_CONFIG_SRCDIR([src/firetools/main.cpp])
#AC_CONFIG_HEADERS([config.h])


AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_RANLIB

# checking pthread library
AC_CHECK_LIB([pthread], [main], [], AC_MSG_ERROR([*** POSIX thread support not installed - please install first ***]))
AC_CHECK_HEADER(pthread.h,,AC_MSG_ERROR([*** POSIX thread support not installed - please install first ***]))

# checking qt4/qt5 library
AC_ARG_WITH([qt4],
    AS_HELP_STRING([--with-qt4], [Enable qt4 support]),
    if test "x$with_qt4" == xno; then
        with_qt5=yes
    fi
    )
AC_ARG_WITH([qt5],
    AS_HELP_STRING([--with-qt5], [Enable qt5 support [default]]),
    if test "x$with_qt5" == xno; then
        with_qt4=yes
    fi
    )

AS_IF([test "x$with_qt4" = "x$with_qt5" ],
    AC_MSG_ERROR([*** Choose one of QT version ***])
    )

if test "x$with_qt4" = xyes; then
    eval $(qtchooser --qt=4 --print-env)
    export QT_SELECT
    LDFLAGS="${LDFLAGS} -L${QTLIBDIR} "
    AC_CHECK_LIB([QtCore], [main], [], AC_MSG_ERROR([*** QtCore library not installed - please install Qt4 or Qt5 first ***]))
    AC_CHECK_LIB([QtGui], [main], [], AC_MSG_ERROR([*** QtCore library not installed - please install Qt4 or Qt5 first ***]))
fi

if test "x$with_qt5" = xyes; then
    eval $(qtchooser --qt=5 --print-env)
    export QT_SELECT
    LDFLAGS="${LDFLAGS} -L${QTLIBDIR} "
    AC_CHECK_LIB([Qt5Core], [main], [], AC_MSG_ERROR([*** QtCore library not installed - please install Qt4 or Qt5 first ***]))
    AC_CHECK_LIB([Qt5Gui], [main], [], AC_MSG_ERROR([*** QtCore library not installed - please install Qt4 or Qt5 first ***]))
    AC_CHECK_LIB([Qt5Svg], [main], [], AC_MSG_ERROR([*** Qt5Svg library not installed ***]))
fi

# testing qmake
QMAKE=`which qmake`
echo "Found $QMAKE"
$QMAKE -v

cd src/firetools && $QMAKE && cd ../..

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

