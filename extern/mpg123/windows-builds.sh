#!/bin/sh

# A dirty script to create some windows binaries (shared, static, debug, ...) using the MSYS environment.

# give build type as command line argument
# x86, x86_64, x86-cross, x86_64-cross
build_type=$1
if test -z "$build_type"; then
  echo "Please specify a build type as argument, one of:"
  echo "x86, x86_64, x86-cross, x86_64-cross"
  echo "Optionally set a number of parallel make processes as second argument."
  echo "A third argument might override the list of output modules to build."
  exit 1
fi

build_procs=$2
# Could use parallel make with number of cores in system,
# but it turns out we burn most time in configure with
# slow forks anyway.
#if test -z "$build_procs"; then
#  build_procs=$(nproc 2>/dev/null)
#fi

# -D__MINGW_USE_VC2005_COMPAT=1 use 64bit time internally for 32bit, so XP and earlier don't get into
# missing _time32 errors

modules=${3:-win32_wasapi,win32}

echo "build type: $build_type"
case $build_type in
  x86)
    decoder=x86
    strip=strip
    hostopt="CPPFLAGS=-D__MINGW_USE_VC2005_COMPAT=1"
  ;;
  x86_64)
    decoder=x86-64
    strip=strip
    hostopt=
  ;;
  x86-cross)
    decoder=x86
    strip=i686-w64-mingw32-strip
    hostopt="--host=i686-w64-mingw32 --build=`./build/config.guess` CPPFLAGS=-D__MINGW_USE_VC2005_COMPAT=1"
  ;;
  x86_64-cross)
    decoder=x86-64
    strip=x86_64-w64-mingw32-strip
    hostopt="--host=x86_64-w64-mingw32 --build=`./build/config.guess`"
  ;;
  *)
    echo "Unknown build type!"
    exit 1
  ;;
esac

temp="$PWD/tmp"
final="$PWD/releases"
txt="README COPYING NEWS"
# let's try with modules
opts="--with-audio=$modules LDFLAGS=-static-libgcc"

# Get the version for the build from version.h.
major=$(grep '#define MPG123_MAJOR' src/version.h | cut -f 3 -d ' ')
minor=$(grep '#define MPG123_MINOR' src/version.h | cut -f 3 -d ' ')
patch=$(grep '#define MPG123_PATCH' src/version.h | cut -f 3 -d ' ')
suffix=$(grep '#define MPG123_SUFFIX' src/version.h | cut -f 2 -d '"')
version="$major.$minor.$patch$suffix"

echo "Building binaries for version $version"

prepare_dir()
{
	test -e "$final" || mkdir "$final"
}

prepare_unix2dos()
{
	echo "preparing unix2dos tool"
	# I'll include documentation in DOS-style, with the help of this little unix2dos variant.
	test -x "unix2dos" || cat <<'EOT' > unix2dos.c && gcc -O -o unix2dos unix2dos.c
#include <unistd.h>
#include <stdio.h>
int main()
{
	char buf[1000];
	ssize_t got;
	while((got=read(0, buf, 1000))>0)
	{
		ssize_t end=0;
		ssize_t pos=0;
		for(end=0;end<got;++end)
		{
			if(buf[end] == '\n')
			{
				write(1, buf+pos, end-pos);
				write(1, "\r\n", 2);
				pos=end+1;
			}
		}
		write(1, buf+pos, end-pos);
	}
}
EOT
}

mpg123_build()
{
	cpu=$1
	stat=$2
	debug=$3
	myopts=$opts
	if test "$stat" = "y"; then
		echo "static build (stat=$stat)" &&
		name=mpg123-$version-static-$cpu
		myopts="$myopts --disable-shared"
	else
		echo "dynamic build (stat=$stat)" &&
		name=mpg123-$version-$cpu
	fi &&
	if test "$debug" = "y"; then
		echo "Debugging build!"
		name=$name-debug
		myopts="$myopts --enable-debug"
	fi &&
	tmp=$temp-$name &&
	echo "REMOVING $tmp!" &&
	sleep 5 &&
	if test -e Makefile; then make clean; fi &&
	rm -rvf $tmp &&
	# We do not like dependency to libgcc_s_dw2-1.dll.
	# Libout123 somehow pulls in that one. Not sure if avoiding
	# it is an option.
	./configure $hostopt \
	  --prefix=$tmp $myopts --with-cpu=$cpu &&
	make -j${build_procs:-1} && make install &&
	rm -rf "$final/$name" &&
	mkdir  "$final/$name" &&
	cp -v "$tmp/bin/"*.exe "$final/$name" &&
	if test "$debug" = y; then
		echo "Not stripping the debug build..."
	else
		$strip --strip-unneeded "$final/$name/"*.exe
	fi &&
	if test "$stat" = "y"; then
		echo "No DLL there..."
	else
		cp -v "$tmp"/bin/lib*123*.dll "$tmp"/include/*123*.h "$final/$name" &&
		for f in src/lib*123/.libs/lib*123*.dll.def
		do
			cp -v "$f" "$final/$name/$(basename ${f%.dll.def}.def)"
		done &&
		if test "$debug" = y; then
			echo "Not stripping the debug build..."
		else
			$strip --strip-unneeded "$final/$name/"*.dll || exit 1
		fi &&
		for i in $tmp/lib/mpg123/*.dll
		do
			if test -e "$i"; then
				plugdir="$final/$name/plugins"
				mkdir -p "$plugdir" &&
				cp -v "$i" "$plugdir"
			fi
		done
	fi &&
	for i in $txt
	do
		echo "text file $i -> $final/$name/$i.txt"
		./unix2dos < "$i" > "$final/$name/$i.txt"
	done &&
	cp doc/windows-notes.html "$final/$name/"
}

prepare_dir &&
prepare_unix2dos &&
mpg123_build $decoder y n &&
mpg123_build $decoder n n &&
mpg123_build $decoder n y &&
echo "Hurray!" || echo "Bleh..."
