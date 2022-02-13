#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

DIE=0

if test -z "$*"; then
  echo "**Warning**: I am going to run \`configure' with arguments for"
  echo "developer/maintainer mode.  If you wish to pass extra arguments,"
  echo "(such as --prefix), please specify them on the \`$0'"
  echo "command line."
  echo "If you wish to run configure yourself, please specify --no-configure."
  echo
fi

(test -f $srcdir/configure.ac) || {
    echo -n "**Error**: Directory "\`$srcdir\'" does not look like the"
    echo " top-level package directory"
    exit 1
}

# Make some directories required by automake, if they don't exist
if ! [ -d config ]; then mkdir -v config; fi
if ! [ -d m4     ]; then mkdir -v m4;     fi

if ! autoreconf --version </dev/null >/dev/null 2>&1
then

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "**Error**: You must have \`autoconf' installed."
  echo "Download the appropriate package for your distribution,"
  echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
  DIE=1
}

(grep "^LT_INIT" $srcdir/configure.ac >/dev/null) && {
  (libtoolize --version) < /dev/null > /dev/null 2>&1 \
	  && LIBTOOLIZE=libtoolize || {
	(glibtoolize --version) < /dev/null > /dev/null 2>&1 \
		&& LIBTOOLIZE=glibtoolize || {
      echo
      echo "**Error**: You must have \`libtool' installed."
      echo "You can get it from: ftp://ftp.gnu.org/pub/gnu/"
      DIE=1
    }
  }
}

(automake --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "**Error**: You must have \`automake' installed."
  echo "You can get it from: ftp://ftp.gnu.org/pub/gnu/"
  DIE=1
  NO_AUTOMAKE=yes
}


# if no automake, don't bother testing for aclocal
test -n "$NO_AUTOMAKE" || (aclocal --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "**Error**: Missing \`aclocal'.  The version of \`automake'"
  echo "installed doesn't appear recent enough."
  echo "You can get automake from ftp://ftp.gnu.org/pub/gnu/"
  DIE=1
}

if test "$DIE" -eq 1; then
  exit 1
fi

case $CC in
xlc )
  am_opt=--include-deps;;
esac

echo "Running aclocal $aclocalinclude ..."
aclocal $ACLOCAL_FLAGS || exit 1
echo "Running $LIBTOOLIZE ..."
$LIBTOOLIZE || exit 1
echo "Running automake --gnu $am_opt ..."
automake --add-missing --gnu $am_opt || exit 1
echo "Running autoconf ..."
autoconf || exit 1

else # autoreconf instead

    echo "Running autoreconf --verbose --install ..."
    autoreconf --verbose --install || exit 1

fi

if ( echo "$@" | grep -q -e "--no-configure" ); then
  NOCONFIGURE=1
fi

conf_flags="--enable-maintainer-mode --enable-debug --disable-silent-rules"

if test x$NOCONFIGURE = x; then
  echo Running $srcdir/configure $conf_flags "$@" ...
  $srcdir/configure $conf_flags "$@" \
  && echo Now type \`make\' to compile. || exit 1
else
  echo Skipping configure process.
fi
