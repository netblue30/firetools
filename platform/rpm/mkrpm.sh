#!/bin/bash

###
###  mkrpm.sh
###  NAME and VER are read from configure.ac.
###  The VER git tag is exported into a tarball and built into an RPM package.
###

set -e # stop upon non-zero return
#set -x # print everything this script does

# Sanity check: necessary tools
for CMD in git sed rpmbuild; do
  if ! which $CMD > /dev/null 2>&1; then
    echo "ERROR: Command not found: $CMD" && exit 255
  fi
done
# Sanity check: base source directory is expected
for DIRNAME in $(dirname $0)/../rpm $(dirname $0)/../../platform; do
  if [ ! -d $DIRNAME ]; then
    echo "ERROR: Unexpected directory, aborting."
    exit 255
  fi
done
PATH_TO_BASESRC=$(readlink -f $(dirname $0)/../../)
NAME=$(grep AC_INIT $PATH_TO_BASESRC/configure.ac | sed -r 's/^AC_INIT\(([a-z]+),.*/\1/')
 VER=$(grep AC_INIT $PATH_TO_BASESRC/configure.ac | sed -r 's/^AC_INIT\([a-z]+, ?([\.0-9]+),.*/\1/')

# export tarball archive from git tag
cd $PATH_TO_BASESRC
mkdir -p build
git -c tar.tar.xz.command="xz -c9" archive --prefix=$NAME-$VER/ -o build/$NAME-$VER.tar.xz $VER
PATH_TO_TARBALL=$(readlink -f build/$NAME-$VER.tar.xz)
PATH_TO_SPEC=$(readlink -f platform/rpm/$NAME.spec)
cd -

# fresh temporary rpmbuild _topdir for each build
mkdir -p $HOME/tmprpmbuild
export RPMTMPDIR=$(mktemp -d -p $HOME/tmprpmbuild)
mkdir -p $RPMTMPDIR/{RPMS,SRPMS,BUILD,SOURCES,SPECS}

# stage rpmbuild inputs
cp $PATH_TO_TARBALL                                 $RPMTMPDIR/SOURCES/
cp $PATH_TO_SPEC                                    $RPMTMPDIR/SPECS/
sed -i "s/Version: FIRETOOLSVERSION/Version: $VER/" $RPMTMPDIR/SPECS/$NAME.spec

# build
rpmbuild --define='_topdir %{getenv:RPMTMPDIR}' -ba $RPMTMPDIR/SPECS/$NAME.spec

# copy rpmbuild outputs to build/ directory
rm -rf $PATH_TO_BASESRC/build/*.rpm
cp $(find $RPMTMPDIR -name '*.rpm') $PATH_TO_BASESRC/build/

# success
echo
echo "    BUILD COMPLETE"
echo
find $PATH_TO_BASESRC/build/ -name '*.rpm'
