AC_PREREQ([2.68])
AC_INIT(firetools, 0.9.58, netblue30@yahoo.com, , http://firejail.wordpress.com)
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


HAVE_FATAL_WARNINGS=""
AC_ARG_ENABLE([fatal_warnings],
    AS_HELP_STRING([--enable-fatal-warnings], [-W -Wall -Werror]))
AS_IF([test "x$enable_fatal_warnings" = "xyes"], [
	HAVE_FATAL_WARNINGS="-W -Wall -Werror"
	AC_SUBST(HAVE_FATAL_WARNINGS)
])



QMAKE=""
AC_ARG_WITH(
    qmake,
    AS_HELP_STRING(
	[--with-qmake=PATH],
	[Path to qmake executable]
    ),
    [
	qmake="$withval"
	QMAKE="$qmake"
    ],
    [
	AC_PATH_TOOL([QMAKE],[qmake])
    ]
)
echo "trying $QMAKE"


#if test -f /usr/lib64/qt4/bin/qmake; then
#	QMAKE=/usr/lib64/qt4/bin/qmake
#fi

if test -z "$QMAKE"
then
	AC_MSG_ERROR([qmake and/or Qt are missing, please install them.])
fi
if test -f "$QMAKE"
then
	echo "found $QMAKE"
else
	AC_MSG_ERROR([qmake and/or Qt are missing, please install them.])
fi

$QMAKE -v

cd src/firetools && CXXFLAGS="$CXXFLAGS $HAVE_FATAL_WARNINGS" $QMAKE && cd ../..
cd src/firejail-ui && CXXFLAGS="$CXXFLAGS $HAVE_FATAL_WARNINGS" $QMAKE && cd ../..
cd src/fstats && CXXFLAGS="$CXXFLAGS $HAVE_FATAL_WARNINGS" $QMAKE && cd ../..
cd src/fmgr && CXXFLAGS="$CXXFLAGS $HAVE_FATAL_WARNINGS" $QMAKE && cd ../..

cp confdefs.h firetools_config.h
AC_OUTPUT(Makefile)

echo
echo "Required run time packages:"
echo "      firejail"
echo "      libqt5svg5 (Qt5 SVG icon library) if the program was compiled for Qt5."
echo
