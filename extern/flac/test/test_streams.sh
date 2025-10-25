#!/bin/sh -e

#  FLAC - Free Lossless Audio Codec
#  Copyright (C) 2001-2009  Josh Coalson
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
PATH=../src/test_streams:$PATH
PATH=../objs/$BUILD/bin:$PATH

if [ -z "$FLAC__TEST_LEVEL" ] ; then
	FLAC__TEST_LEVEL=1
fi

flac${EXE} --help 1>/dev/null 2>/dev/null || die "ERROR can't find flac executable"

run_flac ()
{
	if [ "$FLAC__TEST_WITH_VALGRIND" = yes ] ; then
		echo "valgrind --leak-check=yes --show-reachable=yes --num-callers=50 flac $*" >>test_streams.valgrind.log
		valgrind --leak-check=yes --show-reachable=yes --num-callers=50 --log-fd=4 flac --no-error-on-compression-fail $* 4>>test_streams.valgrind.log
	else
		flac${EXE} --no-error-on-compression-fail $*
	fi
}

echo "Generating streams..."
if [ ! -f wacky1.wav ] ; then
	test_streams || die "ERROR: missing files"
fi

#
# single-file test routines
#

test_file ()
{
	name=$1
	channels=$2
	bps=$3
	encode_options="$4"

	echo $ECHO_N "$name (--channels=$channels --bps=$bps $encode_options): encode..." $ECHO_C
	cmd="run_flac --verify --silent --force --force-raw-format --endian=little --sign=signed --sample-rate=44100 --bps=$bps --channels=$channels $encode_options --no-padding -j2 $name.raw"
	echo "### ENCODE $name #######################################################" >> ./streams.log
	echo "###    cmd=$cmd" >> ./streams.log
	$cmd 2>>./streams.log || die "ERROR during encode of $name"

	echo $ECHO_N "decode..." $ECHO_C
	cmd="run_flac --silent --force --endian=little --sign=signed --decode --force-raw-format --output-name=$name.cmp $name.flac"
	echo "### DECODE $name #######################################################" >> ./streams.log
	echo "###    cmd=$cmd" >> ./streams.log
	$cmd 2>>./streams.log || die "ERROR during decode of $name"

	ls -1l $name.raw >> ./streams.log
	ls -1l $name.flac >> ./streams.log
	ls -1l $name.cmp >> ./streams.log

	echo $ECHO_N "compare..." $ECHO_C
	cmp $name.raw $name.cmp || die "ERROR during compare of $name"

	echo OK
}

test_file_piped ()
{
	name=$1
	channels=$2
	bps=$3
	encode_options="$4"

	if [ "$(env | grep -ic '^comspec=')" != 0 ] ; then
		is_win=yes
	else
		is_win=no
	fi

	echo $ECHO_N "$name: encode via pipes..." $ECHO_C
	if [ $is_win = yes ] ; then
		cmd="run_flac --verify --silent --force --force-raw-format --endian=little --sign=signed --sample-rate=44100 --bps=$bps --channels=$channels $encode_options --no-padding --stdout $name.raw"
		echo "### ENCODE $name #######################################################" >> ./streams.log
		echo "###    cmd=$cmd" >> ./streams.log
		$cmd 1>$name.flac 2>>./streams.log || die "ERROR during encode of $name"
	else
		cmd="run_flac --verify --silent --force --force-raw-format --endian=little --sign=signed --sample-rate=44100 --bps=$bps --channels=$channels $encode_options --no-padding --stdout -"
		echo "### ENCODE $name #######################################################" >> ./streams.log
		echo "###    cmd=$cmd" >> ./streams.log
		$cmd < $name.raw 1>$name.flac 2>>./streams.log || die "ERROR during encode of $name"
	fi
	echo $ECHO_N "decode via pipes..." $ECHO_C
	if [ $is_win = yes ] ; then
		cmd="run_flac --silent --force --endian=little --sign=signed --decode --force-raw-format --stdout $name.flac"
		echo "### DECODE $name #######################################################" >> ./streams.log
		echo "###    cmd=$cmd" >> ./streams.log
		$cmd 1>$name.cmp 2>>./streams.log || die "ERROR during decode of $name"
	else
		cmd="run_flac --silent --force --endian=little --sign=signed --decode --force-raw-format --stdout -"
		echo "### DECODE $name #######################################################" >> ./streams.log
		echo "###    cmd=$cmd" >> ./streams.log
		$cmd < $name.flac 1>$name.cmp 2>>./streams.log || die "ERROR during decode of $name"
	fi
	ls -1l $name.raw >> ./streams.log
	ls -1l $name.flac >> ./streams.log
	ls -1l $name.cmp >> ./streams.log

	echo $ECHO_N "compare..." $ECHO_C
	cmp $name.raw $name.cmp || die "ERROR during compare of $name"

	echo OK
}

test_corrupted_file ()
{
	name=$1
	channels=$2
	bps=$3
	encode_options="$4"

	echo $ECHO_N "$name (--channels=$channels --bps=$bps $encode_options): encode..." $ECHO_C
	cmd="run_flac --verify --silent --no-padding --force --force-raw-format --endian=little --sign=signed --sample-rate=44100 --bps=$bps --channels=$channels $encode_options --no-padding $name.raw"
	echo "### ENCODE $name #######################################################" >> ./streams.log
	echo "###    cmd=$cmd" >> ./streams.log
	$cmd 2>>./streams.log || die "ERROR during encode of $name"

	filesize=$(wc -c < $name.flac)
	bs=$((filesize/13))

	# Overwrite with 'garbagegarbagegarbage....'
	yes garbage 2>/dev/null | dd of=$name.flac conv=notrunc bs=$bs seek=1 count=2 2>> ./streams.log
	# Overwrite with 0x00
	dd if=/dev/zero of=$name.flac conv=notrunc bs=$bs seek=4 count=2 2>> ./streams.log
	# Overwrite with 0xFF
	tr '\0' '\377' < /dev/zero | dd of=$name.flac conv=notrunc bs=$bs seek=7 count=2 2>> ./streams.log
	# Remove section
	cp $name.flac $name.tmp.flac
	dd if=$name.tmp.flac of=$name.flac bs=$bs skip=12 seek=10 2>> ./streams.log

	echo $ECHO_N "decode..." $ECHO_C
	cmd="run_flac --silent --decode-through-errors --force --endian=little --sign=signed --decode --force-raw-format --output-name=$name.cmp $name.flac"
	echo "### DECODE $name.corrupt #######################################################" >> ./streams.log
	echo "###    cmd=$cmd" >> ./streams.log
	$cmd 2>>./streams.log || die "ERROR during decode of $name"

	ls -1l $name.raw >> ./streams.log
	ls -1l $name.flac >> ./streams.log
	ls -1l $name.cmp >> ./streams.log

	echo $ECHO_N "compare..." $ECHO_C
	if [ "$(wc -c < $name.raw)" -ne "$(wc -c < $name.cmp)" ]; then
		die "ERROR, length of decoded file not equal to length of original"
	fi

	echo OK
}

if [ "$FLAC__TEST_LEVEL" -gt 1 ] ; then
	max_lpc_order=32
else
	max_lpc_order=16
fi

echo "Testing noise through pipes..."
test_file_piped noise 1 8 "-0"

echo "Testing small files..."
test_file test01 1 16 "-0 -l $max_lpc_order --lax -m -e -p"
test_file test02 2 16 "-0 -l $max_lpc_order --lax -m -e -p"
test_file test03 1 16 "-0 -l $max_lpc_order --lax -m -e -p"
test_file test04 2 16 "-0 -l $max_lpc_order --lax -m -e -p"

for bps in 8 16 24 32 ; do
	echo "Testing $bps-bit full-scale deflection streams..."
	for b in 01 02 03 04 05 06 07 ; do
		test_file fsd$bps-$b 1 $bps "-0 -l $max_lpc_order --lax -m -e -p"
	done
done

echo "Testing 16-bit wasted-bits-per-sample streams..."
for b in 01 ; do
	test_file wbps16-$b 1 16 "-0 -l $max_lpc_order --lax -m -e -p"
done

for bps in 8 16 24 32; do
	echo "Testing $bps-bit sine wave streams..."
	for b in 00 ; do
		test_file sine${bps}-$b 1 $bps "-0 -l $max_lpc_order --lax -m -e --sample-rate=48000"
	done
	for b in 01 ; do
		test_file sine${bps}-$b 1 $bps "-0 -l $max_lpc_order --lax -m -e --sample-rate=96000"
	done
	for b in 02 03 04 ; do
		test_file sine${bps}-$b 1 $bps "-0 -l $max_lpc_order --lax -m -e"
	done
	for b in 10 11 ; do
		test_file sine${bps}-$b 2 $bps "-0 -l $max_lpc_order --lax -m -e --sample-rate=48000"
	done
	for b in 12 ; do
		test_file sine${bps}-$b 2 $bps "-0 -l $max_lpc_order --lax -m -e --sample-rate=96000"
	done
	for b in 13 14 15 16 17 18 19 ; do
		test_file sine${bps}-$b 2 $bps "-0 -l $max_lpc_order --lax -m -e"
	done
done

echo "Testing blocksize variations..."
for disable in '' '--disable-verbatim-subframes --disable-constant-subframes' '--disable-verbatim-subframes --disable-constant-subframes --disable-fixed-subframes' ; do
	for blocksize in 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 ; do
		for lpc_order in 0 1 2 3 4 5 7 8 9 15 16 17 31 32 ; do
			if [ $lpc_order = 0 ] || [ $lpc_order -le $blocksize ] ; then
				test_file noise8m32 1 8 "-8 -p -e -l $lpc_order --lax --blocksize=$blocksize $disable"
			fi
		done
	done
done

echo "Testing blocksize variations with subdivide apodization..."
for blocksize in 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 ; do
	for lpc_order in 0 1 2 3 4 5 7 8 9 15 16 17 31 32 ; do
		if [ $lpc_order = 0 ] || [ $lpc_order -le $blocksize ] ; then
			test_file noise8m32 1 8 "-8 -p -e -A \"subdivide_tukey(32)\" -l $lpc_order --lax --blocksize=$blocksize"
		fi
	done
done

echo "Testing some frame header variations..."
test_file sine16-01 1 16 "-0 -l $max_lpc_order -m -e -p --lax -b $max_lpc_order"
test_file sine16-01 1 16 "-0 -l $max_lpc_order -m -e -p --lax -b 65535"
test_file sine16-01 1 16 "-0 -l $max_lpc_order -m -e -p --lax --sample-rate=9"
test_file sine16-01 1 16 "-0 -l $max_lpc_order -m -e -p --lax --sample-rate=90"
test_file sine16-01 1 16 "-0 -l $max_lpc_order -m -e -p --lax --sample-rate=90000"
test_file sine16-01 1 16 "-0 -l $max_lpc_order -m -e -p --lax --sample-rate=9"
test_file sine16-01 1 16 "-0 -l $max_lpc_order -m -e -p --lax --sample-rate=90"
test_file sine16-01 1 16 "-0 -l $max_lpc_order -m -e -p --lax --sample-rate=90000"

echo "Testing option variations..."
for f in 00 01 02 03 04 ; do
	for disable in '' '--disable-verbatim-subframes --disable-constant-subframes' '--disable-verbatim-subframes --disable-constant-subframes --disable-fixed-subframes' ; do
		if [ -z "$disable" ] || [ "$FLAC__TEST_LEVEL" -gt 0 ] ; then
			for opt in 0 1 2 4 5 6 8 ; do
				for extras in '' '-p' '-e' ; do
					if [ -z "$extras" ] || [ "$FLAC__TEST_LEVEL" -gt 0 ] ; then
						test_file sine16-$f 1 16 "-$opt $extras $disable"
					fi
				done
			done
			if [ "$FLAC__TEST_LEVEL" -gt 1 ] ; then
				test_file sine16-$f 1 16 "-b 16384 -m -r 8 -l $max_lpc_order --lax -e -p $disable"
			fi
		fi
	done
done

for f in 10 11 12 13 14 15 16 17 18 19 ; do
	for disable in '' '--disable-verbatim-subframes --disable-constant-subframes' '--disable-verbatim-subframes --disable-constant-subframes --disable-fixed-subframes' ; do
		if [ -z "$disable" ] || [ "$FLAC__TEST_LEVEL" -gt 0 ] ; then
			for opt in 0 1 2 4 5 6 8 ; do
				for extras in '' '-p' '-e' ; do
					if [ -z "$extras" ] || [ "$FLAC__TEST_LEVEL" -gt 0 ] ; then
						test_file sine16-$f 2 16 "-$opt $extras $disable"
					fi
				done
			done
			if [ "$FLAC__TEST_LEVEL" -gt 1 ] ; then
				test_file sine16-$f 2 16 "-b 16384 -m -r 8 -l $max_lpc_order --lax -e -p $disable"
			fi
		fi
	done
done

echo "Testing corruption handling..."
for bps in 8 16 24 ; do
	for f in 00 01 02 03 04 10 11 12 13 14 15 16 17 18 19; do
		for disable in '' '--disable-verbatim-subframes --disable-constant-subframes' '--disable-verbatim-subframes --disable-constant-subframes --disable-fixed-subframes' ; do
			if [ -z "$disable" ] || [ "$FLAC__TEST_LEVEL" -gt 0 ] ; then
				for opt in 0 1 2 4 5 6 8 ; do
					for extras in '' '-p' '-e' ; do
						if [ -z "$extras" -o "$FLAC__TEST_LEVEL" -gt 0 ] && { [ "$bps" -eq 16 -a "$f" -lt 15 ] || [ "$FLAC__TEST_LEVEL" -gt 1 ]; } ; then
							if [ "$f" -lt 10 ] ; then
								test_corrupted_file sine$bps-$f 1 $bps "-$opt $extras $disable"
							else
								test_corrupted_file sine$bps-$f 2 $bps "-$opt $extras $disable"
							fi
						fi
					done
				done
			fi
		done
	done
done

echo "Testing noise..."
for disable in '' '--disable-verbatim-subframes --disable-constant-subframes' '--disable-verbatim-subframes --disable-constant-subframes --disable-fixed-subframes' ; do
	if [ -z "$disable" ] || [ "$FLAC__TEST_LEVEL" -gt 0 ] ; then
		for channels in 1 2 4 8 ; do
			if [ $channels -le 2 ] || [ "$FLAC__TEST_LEVEL" -gt 1 ] ; then
				for bps in 8 16 24 32; do
					for opt in 0 1 2 3 4 5 6 7 8 ; do
						for extras in '' '-p' '-e' ; do
                                                        if { [ -z "$extras" ] || [ "$FLAC__TEST_LEVEL" -gt 0 ]; } && { [ "$extras" != '-p' ] || [ "$opt" -gt 2 ]; } ; then
								for blocksize in '' '--lax -b 32' '--lax -b 32768' '--lax -b 65535' ; do
									if [ -z "$blocksize" ] || [ "$FLAC__TEST_LEVEL" -gt 0 ] ; then
										test_file noise $channels $bps "-$opt $extras $blocksize $disable"
									fi
								done
							fi
						done
					done
					if [ "$FLAC__TEST_LEVEL" -gt 1 ] ; then
						test_file noise $channels $bps "-b 16384 -m -r 8 -l $max_lpc_order --lax -e -p $disable"
					fi
				done
			fi
		done
	fi
done
