#!/bin/sh -e

#  FLAC - Free Lossless Audio Codec
#  Copyright (C) 2004-2009  Josh Coalson
#  Copyright (C) 2011-2025  Xiph.Org Foundation
#
#  This file is part the FLAC project.  FLAC is comprised of several
#  components distributed under different licenses.  The codec libraries
#  are distributed under Xiph.Org's BSD-like license (see the file
#  COPYING.Xiph in this distribution).  All other programs, libraries, and
#  plugins are distributed under the GPL (see COPYING.GPL).  The documentation
#  is distributed under the Gnu FDL (see COPYING.FDL).  Each file in the
#  FLAC distribution contains at the top the terms under which it may be
#  distributed.
#
#  Since this particular file is relevant to all components of FLAC,
#  it may be distributed under the Xiph.Org license, which is the least
#  restrictive of those mentioned above.  See the file COPYING.Xiph in this
#  distribution.

. ./common.sh

PATH=../src/flac:$PATH
PATH=../src/metaflac:$PATH
PATH=../src/test_seeking:$PATH
PATH=../src/test_streams:$PATH
PATH=../objs/$BUILD/bin:$PATH

if [ -z "$FLAC__TEST_LEVEL" ] ; then
	FLAC__TEST_LEVEL=1
fi

flac${EXE} --help 1>/dev/null 2>/dev/null || die "ERROR can't find flac executable"
metaflac${EXE} --help 1>/dev/null 2>/dev/null || die "ERROR can't find metaflac executable"

run_flac ()
{
	if [ "$FLAC__TEST_WITH_VALGRIND" = yes ] ; then
		echo "valgrind --leak-check=yes --show-reachable=yes --num-callers=50 flac $*" >>test_seeking.valgrind.log
		valgrind --leak-check=yes --show-reachable=yes --num-callers=50 --log-fd=4 flac${EXE} --no-error-on-compression-fail $* 4>>test_seeking.valgrind.log
	else
		flac${EXE} --no-error-on-compression-fail $*
	fi
}

run_metaflac ()
{
	if [ "$FLAC__TEST_WITH_VALGRIND" = yes ] ; then
		echo "valgrind --leak-check=yes --show-reachable=yes --num-callers=50 metaflac $*" >>test_seeking.valgrind.log
		valgrind --leak-check=yes --show-reachable=yes --num-callers=50 --log-fd=4 metaflac${EXE} $* 4>>test_seeking.valgrind.log
	else
		metaflac${EXE} $*
	fi
}

run_test_seeking ()
{
	if [ "$FLAC__TEST_WITH_VALGRIND" = yes ] ; then
		echo "valgrind --leak-check=yes --show-reachable=yes --num-callers=50 test_seeking $*" >>test_seeking.valgrind.log
		valgrind --leak-check=yes --show-reachable=yes --num-callers=50 --log-fd=4 test_seeking $* 4>>test_seeking.valgrind.log
	else
		test_seeking${EXE} $*
	fi
}

echo $ECHO_N "Checking for --ogg support in flac ... " $ECHO_C
if flac${EXE} --ogg --no-error-on-compression-fail --silent --force-raw-format --endian=little --sign=signed --channels=1 --bps=8 --sample-rate=44100 -c $0 1>/dev/null 2>&1 ; then
	has_ogg=yes;
else
	has_ogg=no;
fi
echo ${has_ogg}

echo "Generating streams..."
if [ ! -f noise.raw ] ; then
	test_streams || die "ERROR during test_streams"
fi

echo "generating FLAC files for seeking:"
run_flac --verify --force --silent --force-raw-format --endian=big --sign=signed --sample-rate=44100 --bps=8 --channels=1 --blocksize=576 -S- --output-name=tiny.flac noise8m32.raw || die "ERROR generating FLAC file"
run_flac --verify --force --silent --force-raw-format --endian=big --sign=signed --sample-rate=44100 --bps=16 --channels=2 --blocksize=576 -S- --output-name=small.flac noise.raw || die "ERROR generating FLAC file"
run_flac --verify --force --silent --force-raw-format --endian=big --sign=signed --sample-rate=44100 --bps=8 --channels=1 --blocksize=576 -S10x --output-name=tiny-s.flac noise8m32.raw || die "ERROR generating FLAC file"
run_flac --verify --force --silent --force-raw-format --endian=big --sign=signed --sample-rate=44100 --bps=16 --channels=2 --blocksize=576 -S10x --output-name=small-s.flac noise.raw || die "ERROR generating FLAC file"

tiny_samples="$(metaflac${EXE} --show-total-samples tiny.flac)"
small_samples="$(metaflac${EXE} --show-total-samples small.flac)"

tiny_seek_count=100
if [ "$FLAC__TEST_LEVEL" -gt 1 ] ; then
	small_seek_count=10000
else
	small_seek_count=100
fi

for suffix in '' '-s' ; do
	echo "testing tiny$suffix.flac:"
	if run_test_seeking tiny$suffix.flac $tiny_seek_count $tiny_samples noise8m32.raw ; then : ; else
		die "ERROR: during test_seeking"
	fi

	echo "testing small$suffix.flac:"
	if run_test_seeking small$suffix.flac $small_seek_count $small_samples noise.raw ; then : ; else
		die "ERROR: during test_seeking"
	fi

	echo "removing sample count from tiny$suffix.flac and small$suffix.flac:"
	if run_metaflac --no-filename --set-total-samples=0 tiny$suffix.flac small$suffix.flac ; then : ; else
		die "ERROR: during metaflac"
	fi

	echo "testing tiny$suffix.flac with total_samples=0:"
	if run_test_seeking tiny$suffix.flac $tiny_seek_count $tiny_samples noise8m32.raw ; then : ; else
		die "ERROR: during test_seeking"
	fi

	echo "testing small$suffix.flac with total_samples=0:"
	if run_test_seeking small$suffix.flac $small_seek_count $small_samples noise.raw ; then : ; else
		die "ERROR: during test_seeking"
	fi
done

if [ $has_ogg = "yes" ] ; then

	echo "generating Ogg FLAC files for seeking:"
	run_flac --verify --force --silent --force-raw-format --endian=big --sign=signed --sample-rate=44100 --bps=8 --channels=1 --blocksize=576 --output-name=tiny.oga --ogg noise8m32.raw || die "ERROR generating Ogg FLAC file"
	run_flac --verify --force --silent --force-raw-format --endian=big --sign=signed --sample-rate=44100 --bps=16 --channels=2 --blocksize=576 --output-name=small.oga --ogg noise.raw || die "ERROR generating Ogg FLAC file"
	# seek tables are not used in Ogg FLAC

	echo "testing tiny.oga:"
	if run_test_seeking tiny.oga $tiny_seek_count $tiny_samples noise8m32.raw ; then : ; else
		die "ERROR: during test_seeking"
	fi

	echo "testing small.oga:"
	if run_test_seeking small.oga $small_seek_count $small_samples noise.raw ; then : ; else
		die "ERROR: during test_seeking"
	fi

	echo "generating chained Ogg FLAC files for seeking:"
	# need to generate a second set with a different serial number
	tail -c 750000 noise.raw > noise-secondhalf.raw
	run_flac --verify --force --silent --force-raw-format --endian=big --sign=signed --sample-rate=44100 --bps=16 --channels=2 --blocksize=576 --output-name=small2.oga --ogg noise-secondhalf.raw || die "ERROR generating Ogg FLAC file"

	cat small.oga small2.oga > chained.oga
	cat noise.raw noise-secondhalf.raw > chained.raw

	echo "testing chained.oga:"
	chained_samples=$(($small_samples+187500))
	if run_test_seeking chained.oga $small_seek_count $chained_samples chained.raw ; then : ; else
		die "ERROR: during test_seeking"
	fi

	if command -v oggz > /dev/null ; then
		if command -v oggenc > /dev/null ; then
			oggenc -Q --skeleton -o small-vorbis.oga small.flac
			oggenc -Q --skeleton -o small2-vorbis.oga small.flac
			oggz merge -o small-merged.oga small-vorbis.oga small.oga
			oggz merge -o small2-merged.oga small2-vorbis.oga small2.oga
			cat small2-merged.oga small-merged.oga > chained-merged.oga
			cat noise-secondhalf.raw noise.raw > chained.raw

			echo "testing chained-merged.oga:"
			echo run_test_seeking chained-merged.oga $small_seek_count $chained_samples chained.raw
			if run_test_seeking chained-merged.oga $small_seek_count $chained_samples chained.raw ; then : ; else
				die "ERROR: during test_seeking"
			fi

		fi
	fi

fi

rm -f tiny.flac tiny.oga small.flac small.oga tiny-s.flac small-s.flac
