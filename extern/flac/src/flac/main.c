/* flac - Command-line FLAC encoder/decoder
 * Copyright (C) 2000-2009  Josh Coalson
 * Copyright (C) 2011-2025  Xiph.Org Foundation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#ifdef _WIN32
#include <windows.h>
#endif
#endif

#if !defined _MSC_VER && !defined __MINGW32__
/* unlink is in stdio.h in VC++ */
#include <unistd.h> /* for unlink() */
#endif
#include "FLAC/all.h"
#include "share/alloc.h"
#include "share/grabbag.h"
#include "share/compat.h"
#include "share/endswap.h"
#include "share/safe_str.h"
#include "analyze.h"
#include "decode.h"
#include "encode.h"
#include "local_string_utils.h" /* for flac__strlcat() and flac__strlcpy() */
#include "utils.h"
#include "vorbiscomment.h"

#if 0
/*[JEC] was:#if HAVE_GETOPT_LONG*/
/*[JEC] see flac/include/share/getopt.h as to why the change */
#  include <getopt.h>
#else
#  include "share/getopt.h"
#endif

static int do_it(void);

static FLAC__bool init_options(void);
static int parse_options(int argc, char *argv[]);
static int parse_option(int short_option, const char *long_option, const char *option_argument);
static void free_options(void);
static void add_compression_setting_bool(compression_setting_type_t type, FLAC__bool value);
static void add_compression_setting_string(compression_setting_type_t type, const char *value);
static void add_compression_setting_uint32_t(compression_setting_type_t type, uint32_t value);

static int usage_error(const char *message, ...);
static void short_usage(void);
static void show_version(void);
static void show_help(void);
static void format_mistake(const char *infilename, FileFormat wrong, FileFormat right);

static int encode_file(const char *infilename, FLAC__bool is_first_file, FLAC__bool is_last_file);
static int decode_file(const char *infilename);

static const char *get_encoded_outfilename(const char *infilename);
static const char *get_decoded_outfilename(const char *infilename, const FileFormat format);
static const char *get_outfilename(const char *infilename, const char *suffix);

static void die(const char *message);
static int conditional_fclose(FILE *f);
static char *local_strdup(const char *source);

/*
 * share__getopt format struct; note that for long options with no
 * short option equivalent we just set the 'val' field to 0.
 */
static struct share__option long_options_[] = {
	/*
	 * general options
	 */
	{ "help"                             , share__no_argument, 0, 'h' },
	{ "version"                          , share__no_argument, 0, 'v' },
	{ "decode"                           , share__no_argument, 0, 'd' },
	{ "analyze"                          , share__no_argument, 0, 'a' },
	{ "test"                             , share__no_argument, 0, 't' },
	{ "stdout"                           , share__no_argument, 0, 'c' },
	{ "silent"                           , share__no_argument, 0, 's' },
	{ "totally-silent"                   , share__no_argument, 0, 0 },
	{ "warnings-as-errors"               , share__no_argument, 0, 'w' },
	{ "force"                            , share__no_argument, 0, 'f' },
	{ "delete-input-file"                , share__no_argument, 0, 0 },
	{ "preserve-modtime"                 , share__no_argument, 0, 0 },
	{ "keep-foreign-metadata"            , share__no_argument, 0, 0 },
	{ "keep-foreign-metadata-if-present" , share__no_argument, 0, 0 },
	{ "output-prefix"                    , share__required_argument, 0, 0 },
	{ "output-name"                      , share__required_argument, 0, 'o' },
	{ "skip"                             , share__required_argument, 0, 0 },
	{ "until"                            , share__required_argument, 0, 0 },
	{ "channel-map"                      , share__required_argument, 0, 0 }, /* undocumented */

	/*
	 * decoding options
	 */
	{ "decode-through-errors", share__no_argument, 0, 'F' },
	{ "cue"                  , share__required_argument, 0, 0 },
	{ "apply-replaygain-which-is-not-lossless", share__optional_argument, 0, 0 }, /* undocumented */

	/*
	 * encoding options
	 */
	{ "cuesheet"                  , share__required_argument, 0, 0 },
	{ "no-cued-seekpoints"        , share__no_argument, 0, 0 },
	{ "picture"                   , share__required_argument, 0, 0 },
	{ "tag"                       , share__required_argument, 0, 'T' },
	{ "tag-from-file"             , share__required_argument, 0, 0 },
	{ "compression-level-0"       , share__no_argument, 0, '0' },
	{ "compression-level-1"       , share__no_argument, 0, '1' },
	{ "compression-level-2"       , share__no_argument, 0, '2' },
	{ "compression-level-3"       , share__no_argument, 0, '3' },
	{ "compression-level-4"       , share__no_argument, 0, '4' },
	{ "compression-level-5"       , share__no_argument, 0, '5' },
	{ "compression-level-6"       , share__no_argument, 0, '6' },
	{ "compression-level-7"       , share__no_argument, 0, '7' },
	{ "compression-level-8"       , share__no_argument, 0, '8' },
	{ "compression-level-9"       , share__no_argument, 0, '9' },
	{ "best"                      , share__no_argument, 0, '8' },
	{ "fast"                      , share__no_argument, 0, '0' },
	{ "verify"                    , share__no_argument, 0, 'V' },
	{ "force-raw-format"          , share__no_argument, 0, 0 },
	{ "force-aiff-format"         , share__no_argument, 0, 0 },
	{ "force-rf64-format"         , share__no_argument, 0, 0 },
	{ "force-wave64-format"       , share__no_argument, 0, 0 },
	{ "force-legacy-wave-format"  , share__no_argument, 0, 0 },
	{ "force-extensible-wave-format",share__no_argument,0, 0 },
	{ "force-aiff-c-none-format"  , share__no_argument, 0, 0 },
	{ "force-aiff-c-sowt-format"  , share__no_argument, 0, 0 },
	{ "lax"                       , share__no_argument, 0, 0 },
	{ "replay-gain"               , share__no_argument, 0, 0 },
	{ "ignore-chunk-sizes"        , share__no_argument, 0, 0 },
	{ "seekpoint"                 , share__required_argument, 0, 'S' },
	{ "padding"                   , share__required_argument, 0, 'P' },
#if FLAC__HAS_OGG
	{ "ogg"                       , share__no_argument, 0, 0 },
	{ "decode-chained-stream"     , share__no_argument, 0, 0 },
	{ "serial-number"             , share__required_argument, 0, 0 },
#endif
	{ "blocksize"                 , share__required_argument, 0, 'b' },
	{ "exhaustive-model-search"   , share__no_argument, 0, 'e' },
	{ "max-lpc-order"             , share__required_argument, 0, 'l' },
	{ "apodization"               , share__required_argument, 0, 'A' },
	{ "mid-side"                  , share__no_argument, 0, 'm' },
	{ "adaptive-mid-side"         , share__no_argument, 0, 'M' },
	{ "qlp-coeff-precision-search", share__no_argument, 0, 'p' },
	{ "qlp-coeff-precision"       , share__required_argument, 0, 'q' },
	{ "rice-partition-order"      , share__required_argument, 0, 'r' },
	{ "threads"                   , share__required_argument, 0, 'j' },
	{ "endian"                    , share__required_argument, 0, 0 },
	{ "channels"                  , share__required_argument, 0, 0 },
	{ "bps"                       , share__required_argument, 0, 0 },
	{ "sample-rate"               , share__required_argument, 0, 0 },
	{ "sign"                      , share__required_argument, 0, 0 },
	{ "input-size"                , share__required_argument, 0, 0 },
	{ "error-on-compression-fail" , share__no_argument, 0, 0 },
	{ "limit-min-bitrate"         , share__no_argument, 0, 0 },

	/*
	 * analysis options
	 */
	{ "residual-gnuplot", share__no_argument, 0, 0 },
	{ "residual-text", share__no_argument, 0, 0 },

	/*
	 * negatives
	 */
	{ "no-preserve-modtime"       , share__no_argument, 0, 0 },
	{ "no-decode-through-errors"  , share__no_argument, 0, 0 },
	{ "no-silent"                 , share__no_argument, 0, 0 },
	{ "no-force"                  , share__no_argument, 0, 0 },
	{ "no-seektable"              , share__no_argument, 0, 0 },
	{ "no-delete-input-file"      , share__no_argument, 0, 0 },
	{ "no-keep-foreign-metadata"  , share__no_argument, 0, 0 },
	{ "no-replay-gain"            , share__no_argument, 0, 0 },
	{ "no-ignore-chunk-sizes"     , share__no_argument, 0, 0 },
	{ "no-utf8-convert"           , share__no_argument, 0, 0 },
	{ "no-lax"                    , share__no_argument, 0, 0 },
#if FLAC__HAS_OGG
	{ "no-ogg"                    , share__no_argument, 0, 0 },
#endif
	{ "no-exhaustive-model-search", share__no_argument, 0, 0 },
	{ "no-mid-side"               , share__no_argument, 0, 0 },
	{ "no-adaptive-mid-side"      , share__no_argument, 0, 0 },
	{ "no-qlp-coeff-prec-search"  , share__no_argument, 0, 0 },
	{ "no-padding"                , share__no_argument, 0, 0 },
	{ "no-verify"                 , share__no_argument, 0, 0 },
	{ "no-warnings-as-errors"     , share__no_argument, 0, 0 },
	{ "no-residual-gnuplot"       , share__no_argument, 0, 0 },
	{ "no-residual-text"          , share__no_argument, 0, 0 },
	{ "no-error-on-compression-fail", share__no_argument, 0, 0 },
	/*
	 * undocumented debugging options for the test suite
	 */
	{ "disable-constant-subframes", share__no_argument, 0, 0 },
	{ "disable-fixed-subframes"   , share__no_argument, 0, 0 },
	{ "disable-verbatim-subframes", share__no_argument, 0, 0 },
	{ "no-md5-sum"                , share__no_argument, 0, 0 },

	{0, 0, 0, 0}
};


/*
 * global to hold command-line option values
 */

static struct {
	FLAC__bool show_help;
	FLAC__bool show_version;
	FLAC__bool mode_decode;
	FLAC__bool verify;
	FLAC__bool treat_warnings_as_errors;
	FLAC__bool force_file_overwrite;
	FLAC__bool continue_through_decode_errors;
	replaygain_synthesis_spec_t replaygain_synthesis_spec;
	FLAC__bool lax;
	FLAC__bool test_only;
	FLAC__bool analyze;
	FLAC__bool use_ogg;
	FLAC__bool has_serial_number; /* true iff --serial-number was used */
	FLAC__bool decode_chained_stream;
	long serial_number; /* this is the Ogg serial number and is unused for native FLAC */
	FLAC__bool force_to_stdout;
	FLAC__bool force_raw_format;
	FLAC__bool force_aiff_format;
	FLAC__bool force_rf64_format;
	FLAC__bool force_wave64_format;
	FLAC__bool force_legacy_wave_format;
	FLAC__bool force_extensible_wave_format;
	FLAC__bool force_aiff_c_none_format;
	FLAC__bool force_aiff_c_sowt_format;
	FLAC__bool delete_input;
	FLAC__bool preserve_modtime;
	FLAC__bool keep_foreign_metadata;
	FLAC__bool keep_foreign_metadata_if_present;
	FLAC__bool replay_gain;
	FLAC__bool ignore_chunk_sizes;
	FLAC__bool utf8_convert; /* true by default, to convert tag strings from locale to utf-8, false if --no-utf8-convert used */
	const char *cmdline_forced_outfilename;
	const char *output_prefix;
	analysis_options aopts;
	int padding; /* -1 => no -P options were given, 0 => -P- was given, else -P value */
	size_t num_compression_settings;
	compression_setting_t compression_settings[64]; /* bad MAGIC NUMBER but buffer overflow is checked */
	uint32_t threads;
	const char *skip_specification;
	const char *until_specification;
	const char *cue_specification;
	int format_is_big_endian;
	int format_is_unsigned_samples;
	int format_channels;
	int format_bps;
	int format_sample_rate;
	FLAC__off_t format_input_size;
	char requested_seek_points[5000]; /* bad MAGIC NUMBER but buffer overflow is checked */
	int num_requested_seek_points; /* -1 => no -S options were given, 0 => -S- was given */
	const char *cuesheet_filename;
	FLAC__bool cued_seekpoints;
	FLAC__bool channel_map_none; /* --channel-map=none specified, eventually will expand to take actual channel map */
	FLAC__bool error_on_compression_fail;
	FLAC__bool limit_min_bitrate;

	uint32_t num_files;
	char **filenames;

	FLAC__StreamMetadata *vorbis_comment;
	FLAC__StreamMetadata *pictures[64];
	uint32_t num_pictures;

	struct {
		FLAC__bool disable_constant_subframes;
		FLAC__bool disable_fixed_subframes;
		FLAC__bool disable_verbatim_subframes;
		FLAC__bool do_md5;
	} debug;
} option_values;


/*
 * miscellaneous globals
 */


#ifndef FUZZ_TOOL_FLAC
int main(int argc, char *argv[])
#else
static int main_to_fuzz(int argc, char *argv[])
#endif
{
	int retval = 0;

#ifdef __EMX__
	_response(&argc, &argv);
	_wildcard(&argc, &argv);
#endif
#ifdef _WIN32
	if (get_utf8_argv(&argc, &argv) != 0) {
		fprintf(stderr, "ERROR: failed to convert command line parameters to UTF-8\n");
		return 1;
	}
	SetConsoleOutputCP(CP_UTF8);
	_setmode(fileno(stderr),_O_U8TEXT);
#endif

#ifdef HAVE_SYS_TIME_H
	{
		struct timeval tv;

		if (gettimeofday(&tv, 0) < 0) {
			/* fall back when gettimeofday fails */
			srand(((uint32_t)time(0) << 8) + (uint32_t)clock());
		}
		else {
			srand(((uint32_t)(tv.tv_sec) << 20) + (uint32_t)tv.tv_usec);
		}
	}
#else
#ifdef _WIN32
	/* For Windows, use time and GetTickCount */
	srand(((uint32_t)time(0) << 8) + ((uint32_t)GetTickCount() & 0xff));
#else
	/* time(0) does not have sufficient resolution when flac is invoked more than
	 * once in quick succession (for example in the test suite). As far as I know,
	 * clock() is the only sub-second portable alternative, but measures
	 * execution time, which is often quite similar between runs. From limited
	 * testing, it seems the value varies by about 100, so that would make
	 * collision 100 times less likely than without. Therefore, use
	 * both together to generate a random number seed.
	 */
	srand(((uint32_t)time(0) << 8) + (uint32_t)clock());
#endif
#endif

#ifdef _WIN32
	{
		const char *var;
		var = getenv("LC_ALL");
		if (!var)
			var = getenv("LC_NUMERIC");
		if (!var)
			var = getenv("LANG");
		if (!var || strcmp(var, "C") != 0)
			setlocale(LC_ALL, "");
	}
#else
	setlocale(LC_ALL, "");
#endif
	if(!init_options()) {
		flac__utils_printf(stderr, 1, "ERROR: allocating memory\n");
		retval = 1;
	}
	else {
		if((retval = parse_options(argc, argv)) == 0)
			retval = do_it();
	}

	free_options();

	return retval;
}

int do_it(void)
{
	int retval = 0;

	if(option_values.show_version) {
		show_version();
		return 0;
	}
	else if(option_values.show_help) {
		show_help();
		return 0;
	}
	else {
		if(option_values.num_files == 0) {
			if(flac__utils_verbosity_ >= 1)
				short_usage();
			return 0;
		}

		/*
		 * tweak options; validate the values
		 */
		if(!option_values.mode_decode) {
			if(0 != option_values.cue_specification)
				return usage_error("ERROR: --cue must be used together with -d\n");
			if(0 != option_values.decode_chained_stream)
				return usage_error("ERROR: --decode-chained-streams must be used together with -d, -t or -a\n");
		}
		else {
			if(option_values.test_only) {
				if(0 != option_values.skip_specification)
					return usage_error("ERROR: --skip is not allowed in test mode\n");
				if(0 != option_values.until_specification)
					return usage_error("ERROR: --until is not allowed in test mode\n");
				if(0 != option_values.cue_specification)
					return usage_error("ERROR: --cue is not allowed in test mode\n");
				if(0 != option_values.analyze)
					return usage_error("ERROR: analysis mode (-a/--analyze) and test mode (-t/--test) cannot be used together\n");
			}
		}

		if(0 != option_values.cue_specification && (0 != option_values.skip_specification || 0 != option_values.until_specification))
			return usage_error("ERROR: --cue may not be combined with --skip or --until\n");

		if(option_values.format_channels >= 0) {
			if(option_values.format_channels == 0 || (uint32_t)option_values.format_channels > FLAC__MAX_CHANNELS)
				return usage_error("ERROR: invalid number of channels '%u', must be > 0 and <= %u\n", option_values.format_channels, FLAC__MAX_CHANNELS);
		}
		if(option_values.format_bps >= 0) {
			if(option_values.format_bps != 8 && option_values.format_bps != 16 && option_values.format_bps != 24 && option_values.format_bps != 32)
				return usage_error("ERROR: invalid bits per sample '%u' (must be 8/16/24/32)\n", option_values.format_bps);
		}
		if(option_values.format_sample_rate >= 0) {
			if(!FLAC__format_sample_rate_is_valid(option_values.format_sample_rate))
				return usage_error("ERROR: invalid sample rate '%u', must be > 0 and <= %u\n", option_values.format_sample_rate, FLAC__MAX_SAMPLE_RATE);
		}
		if((option_values.force_raw_format?1:0) +
		   (option_values.force_aiff_format?1:0) +
		   (option_values.force_rf64_format?1:0) +
		   (option_values.force_wave64_format?1:0) +
		   (option_values.force_legacy_wave_format?1:0) +
		   (option_values.force_extensible_wave_format?1:0) +
		   (option_values.force_aiff_c_none_format?1:0) +
		   (option_values.force_aiff_c_sowt_format?1:0)
		    > 1)
			return usage_error("ERROR: only one of force format options allowed\n");
		if(option_values.mode_decode) {
			if(!option_values.force_raw_format) {
				if(option_values.format_is_big_endian >= 0)
					return usage_error("ERROR: --endian only allowed with --force-raw-format\n");
				if(option_values.format_is_unsigned_samples >= 0)
					return usage_error("ERROR: --sign only allowed with --force-raw-format\n");
			}
			if(option_values.format_channels >= 0)
				return usage_error("ERROR: --channels not allowed with --decode\n");
			if(option_values.format_bps >= 0)
				return usage_error("ERROR: --bps not allowed with --decode\n");
			if(option_values.format_sample_rate >= 0)
				return usage_error("ERROR: --sample-rate not allowed with --decode\n");
		}

		if(option_values.ignore_chunk_sizes) {
			if(option_values.mode_decode)
				return usage_error("ERROR: --ignore-chunk-sizes only allowed for encoding\n");
			if(0 != option_values.until_specification)
				return usage_error("ERROR: --ignore-chunk-sizes not allowed with --until\n");
			if(0 != option_values.cue_specification)
				return usage_error("ERROR: --ignore-chunk-sizes not allowed with --cue\n");
			if(0 != option_values.cuesheet_filename)
				return usage_error("ERROR: --ignore-chunk-sizes not allowed with --cuesheet\n");
		}
		if(option_values.replay_gain) {
			if(option_values.force_to_stdout)
				return usage_error("ERROR: --replay-gain not allowed with -c/--stdout\n");
			if(option_values.mode_decode)
				return usage_error("ERROR: --replay-gain only allowed for encoding\n");
			if(option_values.format_channels > 2)
				return usage_error("ERROR: --replay-gain can only be done with mono/stereo input\n");
			if(option_values.format_sample_rate >= 0 && !grabbag__replaygain_is_valid_sample_frequency(option_values.format_sample_rate))
				return usage_error("ERROR: invalid sample rate used with --replay-gain\n");
			if(
				(option_values.padding >= 0 && option_values.padding < (int)GRABBAG__REPLAYGAIN_MAX_TAG_SPACE_REQUIRED) ||
				(option_values.padding < 0 && FLAC_ENCODE__DEFAULT_PADDING < (int)GRABBAG__REPLAYGAIN_MAX_TAG_SPACE_REQUIRED)
			) {
				flac__utils_printf(stderr, 1, "NOTE: --replay-gain may leave a small PADDING block even with --no-padding\n");
			}
		}
		if(option_values.num_files > 1 && option_values.cmdline_forced_outfilename) {
			return usage_error("ERROR: -o/--output-name cannot be used with multiple files\n");
		}
		if(option_values.cmdline_forced_outfilename && option_values.output_prefix) {
			return usage_error("ERROR: --output-prefix conflicts with -o/--output-name\n");
		}
		if(!option_values.mode_decode && 0 != option_values.cuesheet_filename && option_values.num_files > 1) {
			return usage_error("ERROR: --cuesheet cannot be used when encoding multiple files\n");
		}
		if(option_values.keep_foreign_metadata || option_values.keep_foreign_metadata_if_present) {
			/* we're not going to try and support the re-creation of broken WAVE files */
			if(option_values.ignore_chunk_sizes)
				return usage_error("ERROR: using --keep-foreign-metadata cannot be used with --ignore-chunk-sizes\n");
			if(option_values.test_only)
				return usage_error("ERROR: --keep-foreign-metadata is not allowed in test mode\n");
			if(option_values.analyze)
				return usage_error("ERROR: --keep-foreign-metadata is not allowed in analyis mode\n");
			flac__utils_printf(stderr, 2, "NOTE: --keep-foreign-metadata is a new feature; make sure to test the output file before deleting the original.\n");
		}
		if(0 != option_values.decode_chained_stream) {
			if(0 != option_values.skip_specification)
				return usage_error("ERROR: --skip is not supported when decoding chained streams\n");
			if(0 != option_values.until_specification)
				return usage_error("ERROR: --until is not supported when decoding chained streams\n");
			if(0 != option_values.cue_specification)
				return usage_error("ERROR: --cue is not supported when decoding chained streams\n");
			if(option_values.continue_through_decode_errors)
				return usage_error("ERROR: decoding through errors is not supported when decoding chained streams\n");

		}

	}

	flac__utils_printf(stderr, 2, "\n");
	flac__utils_printf(stderr, 2, "flac %s\n", FLAC__VERSION_STRING);
	flac__utils_printf(stderr, 2, "Copyright (C) 2000-2009  Josh Coalson, 2011-2025  Xiph.Org Foundation\n");
	flac__utils_printf(stderr, 2, "flac comes with ABSOLUTELY NO WARRANTY.  This is free software, and you are\n");
	flac__utils_printf(stderr, 2, "welcome to redistribute it under certain conditions.  Type `flac' for details.\n\n");

	if(option_values.mode_decode) {
		FLAC__bool first = true;

		if(option_values.num_files == 0) {
			retval = decode_file("-");
		}
		else {
			uint32_t i;
			if(option_values.num_files > 1)
				option_values.cmdline_forced_outfilename = 0;
			for(i = 0, retval = 0; i < option_values.num_files; i++) {
				if(0 == strcmp(option_values.filenames[i], "-") && !first)
					continue;
				retval |= decode_file(option_values.filenames[i]);
				first = false;
			}
		}
	}
	else { /* encode */
		FLAC__bool first = true;

		if(option_values.ignore_chunk_sizes)
			flac__utils_printf(stderr, 1, "INFO: Make sure you know what you're doing when using --ignore-chunk-sizes.\n      Improper use can cause flac to encode non-audio data as audio.\n");

		if(option_values.num_files == 0) {
			retval = encode_file("-", first, true);
		}
		else {
			uint32_t i;
			if(option_values.num_files > 1)
				option_values.cmdline_forced_outfilename = 0;
			for(i = 0, retval = 0; i < option_values.num_files; i++) {
				if(0 == strcmp(option_values.filenames[i], "-") && !first)
					continue;
				if(encode_file(option_values.filenames[i], first, i == (option_values.num_files-1)))
					retval = 1;
				else
					first = false;
			}
			if(option_values.replay_gain && retval == 0) {
				float album_gain, album_peak;
				grabbag__replaygain_get_album(&album_gain, &album_peak);
				for(i = 0; i < option_values.num_files; i++) {
					const char *error, *outfilename = get_encoded_outfilename(option_values.filenames[i]);
					if(0 == outfilename) {
						flac__utils_printf(stderr, 1, "ERROR: filename too long: %s", option_values.filenames[i]);
						return 1;
					}
					if(0 != (error = grabbag__replaygain_store_to_file_album(outfilename, album_gain, album_peak, option_values.preserve_modtime))) {
						flac__utils_printf(stderr, 1, "%s: ERROR writing ReplayGain album tags (%s)\n", outfilename, error);
						retval = 1;
					}
				}
			}
		}
	}

	return retval;
}

FLAC__bool init_options(void)
{
	option_values.show_help = false;
	option_values.show_version = false;
	option_values.mode_decode = false;
	option_values.verify = false;
	option_values.treat_warnings_as_errors = false;
	option_values.force_file_overwrite = false;
	option_values.continue_through_decode_errors = false;
	option_values.replaygain_synthesis_spec.apply = false;
	option_values.replaygain_synthesis_spec.use_album_gain = true;
	option_values.replaygain_synthesis_spec.limiter = RGSS_LIMIT__HARD;
	option_values.replaygain_synthesis_spec.noise_shaping = NOISE_SHAPING_LOW;
	option_values.replaygain_synthesis_spec.preamp = 0.0;
	option_values.lax = false;
	option_values.test_only = false;
	option_values.analyze = false;
	option_values.use_ogg = false;
	option_values.decode_chained_stream = false;
	option_values.has_serial_number = false;
	option_values.serial_number = 0;
	option_values.force_to_stdout = false;
	option_values.force_raw_format = false;
	option_values.force_aiff_format = false;
	option_values.force_rf64_format = false;
	option_values.force_wave64_format = false;
	option_values.force_legacy_wave_format = false;
	option_values.force_extensible_wave_format = false;
	option_values.force_aiff_c_none_format = false;
	option_values.force_aiff_c_sowt_format = false;
	option_values.delete_input = false;
	option_values.preserve_modtime = true;
	option_values.keep_foreign_metadata = false;
	option_values.keep_foreign_metadata_if_present = false;
	option_values.replay_gain = false;
	option_values.ignore_chunk_sizes = false;
	option_values.utf8_convert = true;
	option_values.cmdline_forced_outfilename = 0;
	option_values.output_prefix = 0;
	option_values.aopts.do_residual_text = false;
	option_values.aopts.do_residual_gnuplot = false;
	option_values.padding = -1;
	option_values.threads = 1;
	option_values.num_compression_settings = 1;
	option_values.compression_settings[0].type = CST_COMPRESSION_LEVEL;
	option_values.compression_settings[0].value.t_unsigned = 5;
	option_values.skip_specification = 0;
	option_values.until_specification = 0;
	option_values.cue_specification = 0;
	option_values.format_is_big_endian = -1;
	option_values.format_is_unsigned_samples = -1;
	option_values.format_channels = -1;
	option_values.format_bps = -1;
	option_values.format_sample_rate = -1;
	option_values.format_input_size = (FLAC__off_t)(-1);
	option_values.requested_seek_points[0] = '\0';
	option_values.num_requested_seek_points = -1;
	option_values.cuesheet_filename = 0;
	option_values.cued_seekpoints = true;
	option_values.channel_map_none = false;
	option_values.error_on_compression_fail = false;
	option_values.limit_min_bitrate = false;

	option_values.num_files = 0;
	option_values.filenames = 0;
	option_values.num_pictures = 0;

	option_values.debug.disable_constant_subframes = false;
	option_values.debug.disable_fixed_subframes = false;
	option_values.debug.disable_verbatim_subframes = false;
	option_values.debug.do_md5 = true;

	if(0 == (option_values.vorbis_comment = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT)))
		return false;

	return true;
}

int parse_options(int argc, char *argv[])
{
	int short_option;
	int option_index = 1;
	FLAC__bool had_error = false;
	const char *short_opts = "0123456789aA:b:cdefFhj:l:mMo:pP:q:r:sS:tT:vVw";

	while ((short_option = share__getopt_long(argc, argv, short_opts, long_options_, &option_index)) != -1) {
		switch (short_option) {
			case 0: /* long option with no equivalent short option */
				had_error |= (parse_option(short_option, long_options_[option_index].name, share__optarg) != 0);
				break;
			case '?':
			case ':':
				had_error = true;
				break;
			default: /* short option */
				had_error |= (parse_option(short_option, 0, share__optarg) != 0);
				break;
		}
	}

	if(had_error) {
		return 1;
	}

	FLAC__ASSERT(share__optind <= argc);

	option_values.num_files = argc - share__optind;

	if(option_values.num_files > 0) {
		uint32_t i = 0;
		if(0 == (option_values.filenames = malloc(sizeof(char*) * option_values.num_files)))
			die("out of memory allocating space for file names list");
		while(share__optind < argc)
			option_values.filenames[i++] = local_strdup(argv[share__optind++]);
	}

	return 0;
}

int parse_option(int short_option, const char *long_option, const char *option_argument)
{
	const char *violation;

	if(short_option == 0) {
		FLAC__ASSERT(0 != long_option);
		if(0 == strcmp(long_option, "totally-silent")) {
			flac__utils_verbosity_ = 0;
		}
		else if(0 == strcmp(long_option, "delete-input-file")) {
			option_values.delete_input = true;
		}
		else if(0 == strcmp(long_option, "preserve-modtime")) {
			option_values.preserve_modtime = true;
		}
		else if(0 == strcmp(long_option, "keep-foreign-metadata")) {
			option_values.keep_foreign_metadata = true;
		}
		else if(0 == strcmp(long_option, "keep-foreign-metadata-if-present")) {
			option_values.keep_foreign_metadata_if_present = true;
		}
		else if(0 == strcmp(long_option, "output-prefix")) {
			FLAC__ASSERT(0 != option_argument);
			option_values.output_prefix = option_argument;
		}
		else if(0 == strcmp(long_option, "skip")) {
			FLAC__ASSERT(0 != option_argument);
			option_values.skip_specification = option_argument;
		}
		else if(0 == strcmp(long_option, "until")) {
			FLAC__ASSERT(0 != option_argument);
			option_values.until_specification = option_argument;
		}
		else if(0 == strcmp(long_option, "input-size")) {
			FLAC__ASSERT(0 != option_argument);
			{
				char *end;
				FLAC__int64 ix;
				ix = strtoll(option_argument, &end, 10);
				if(0 == strlen(option_argument) || *end)
					return usage_error("ERROR: --%s must be a number\n", long_option);
				option_values.format_input_size = (FLAC__off_t)ix;
				if(option_values.format_input_size != ix) /* check if FLAC__off_t is smaller than long long */
					return usage_error("ERROR: --%s too large; this build of flac does not support filesizes over 2GB\n", long_option);
				if(option_values.format_input_size <= 0)
					return usage_error("ERROR: --%s must be > 0\n", long_option);
			}
		}
		else if(0 == strcmp(long_option, "cue")) {
			FLAC__ASSERT(0 != option_argument);
			option_values.cue_specification = option_argument;
		}
		else if(0 == strcmp(long_option, "apply-replaygain-which-is-not-lossless")) {
			option_values.replaygain_synthesis_spec.apply = true;
			if (0 != option_argument) {
				char *p;
				option_values.replaygain_synthesis_spec.limiter = RGSS_LIMIT__NONE;
				option_values.replaygain_synthesis_spec.noise_shaping = NOISE_SHAPING_NONE;
				option_values.replaygain_synthesis_spec.preamp = strtod(option_argument, &p);
				for ( ; *p; p++) {
					if (*p == 'a')
						option_values.replaygain_synthesis_spec.use_album_gain = true;
					else if (*p == 't')
						option_values.replaygain_synthesis_spec.use_album_gain = false;
					else if (*p == 'l')
						option_values.replaygain_synthesis_spec.limiter = RGSS_LIMIT__PEAK;
					else if (*p == 'L')
						option_values.replaygain_synthesis_spec.limiter = RGSS_LIMIT__HARD;
					else if (*p == 'n' && p[1] >= '0' && p[1] <= '3') {
						option_values.replaygain_synthesis_spec.noise_shaping = p[1] - '0';
						p++;
					}
					else
						return usage_error("ERROR: bad specification string \"%s\" for --%s\n", option_argument, long_option);
				}
			}
		}
		else if(0 == strcmp(long_option, "channel-map")) {
			if (0 == option_argument || strcmp(option_argument, "none"))
				return usage_error("ERROR: only --channel-map=none currently supported\n");
			option_values.channel_map_none = true;
		}
		else if(0 == strcmp(long_option, "cuesheet")) {
			FLAC__ASSERT(0 != option_argument);
			option_values.cuesheet_filename = option_argument;
		}
		else if(0 == strcmp(long_option, "picture")) {
			const uint32_t max_pictures = sizeof(option_values.pictures)/sizeof(option_values.pictures[0]);
			FLAC__ASSERT(0 != option_argument);
			if(option_values.num_pictures >= max_pictures)
				return usage_error("ERROR: too many --picture arguments, only %u allowed\n", max_pictures);
			if(0 == (option_values.pictures[option_values.num_pictures] = grabbag__picture_parse_specification(option_argument, &violation)))
				return usage_error("ERROR: (--picture) %s\n", violation);
			option_values.num_pictures++;
		}
		else if(0 == strcmp(long_option, "tag-from-file")) {
			FLAC__ASSERT(0 != option_argument);
			if(!flac__vorbiscomment_add(option_values.vorbis_comment, option_argument, /*value_from_file=*/true, /*raw=*/!option_values.utf8_convert, &violation))
				return usage_error("ERROR: (--tag-from-file) %s\n", violation);
		}
		else if(0 == strcmp(long_option, "no-cued-seekpoints")) {
			option_values.cued_seekpoints = false;
		}
		else if(0 == strcmp(long_option, "force-raw-format")) {
			option_values.force_raw_format = true;
		}
		else if(0 == strcmp(long_option, "force-aiff-format")) {
			option_values.force_aiff_format = true;
		}
		else if(0 == strcmp(long_option, "force-rf64-format")) {
			option_values.force_rf64_format = true;
		}
		else if(0 == strcmp(long_option, "force-wave64-format")) {
			option_values.force_wave64_format = true;
		}
		else if(0 == strcmp(long_option, "force-legacy-wave-format")) {
			option_values.force_legacy_wave_format = true;
		}
		else if(0 == strcmp(long_option, "force-extensible-wave-format")) {
			option_values.force_extensible_wave_format = true;
		}
		else if(0 == strcmp(long_option, "force-aiff-c-none-format")) {
			option_values.force_aiff_c_none_format = true;
		}
		else if(0 == strcmp(long_option, "force-aiff-c-sowt-format")) {
			option_values.force_aiff_c_sowt_format = true;
		}
		else if(0 == strcmp(long_option, "lax")) {
			option_values.lax = true;
		}
		else if(0 == strcmp(long_option, "replay-gain")) {
			option_values.replay_gain = true;
		}
		else if(0 == strcmp(long_option, "ignore-chunk-sizes")) {
			option_values.ignore_chunk_sizes = true;
		}
#if FLAC__HAS_OGG
		else if(0 == strcmp(long_option, "ogg")) {
			option_values.use_ogg = true;
		}
		else if (0 == strcmp(long_option, "decode-chained-stream")) {
			option_values.decode_chained_stream = true;
		}
		else if(0 == strcmp(long_option, "serial-number")) {
			option_values.has_serial_number = true;
			option_values.serial_number = atol(option_argument);
		}
#endif
		else if(0 == strcmp(long_option, "endian")) {
			FLAC__ASSERT(0 != option_argument);
			if(0 == strncmp(option_argument, "big", strlen(option_argument)))
				option_values.format_is_big_endian = true;
			else if(0 == strncmp(option_argument, "little", strlen(option_argument)))
				option_values.format_is_big_endian = false;
			else
				return usage_error("ERROR: argument to --endian must be \"big\" or \"little\"\n");
		}
		else if(0 == strcmp(long_option, "channels")) {
			FLAC__ASSERT(0 != option_argument);
			option_values.format_channels = atoi(option_argument);
		}
		else if(0 == strcmp(long_option, "bps")) {
			FLAC__ASSERT(0 != option_argument);
			option_values.format_bps = atoi(option_argument);
		}
		else if(0 == strcmp(long_option, "sample-rate")) {
			FLAC__ASSERT(0 != option_argument);
			option_values.format_sample_rate = atoi(option_argument);
		}
		else if(0 == strcmp(long_option, "sign")) {
			FLAC__ASSERT(0 != option_argument);
			if(0 == strncmp(option_argument, "signed", strlen(option_argument)))
				option_values.format_is_unsigned_samples = false;
			else if(0 == strncmp(option_argument, "unsigned", strlen(option_argument)))
				option_values.format_is_unsigned_samples = true;
			else
				return usage_error("ERROR: argument to --sign must be \"signed\" or \"unsigned\"\n");
		}
		else if(0 == strcmp(long_option, "residual-gnuplot")) {
			option_values.aopts.do_residual_gnuplot = true;
		}
		else if(0 == strcmp(long_option, "residual-text")) {
			option_values.aopts.do_residual_text = true;
		}
		else if(0 == strcmp(long_option, "limit-min-bitrate")) {
			option_values.limit_min_bitrate = true;
		}
		/*
		 * negatives
		 */
		else if(0 == strcmp(long_option, "no-preserve-modtime")) {
			option_values.preserve_modtime = false;
		}
		else if(0 == strcmp(long_option, "no-decode-through-errors")) {
			option_values.continue_through_decode_errors = false;
		}
		else if(0 == strcmp(long_option, "no-silent")) {
#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
			flac__utils_verbosity_ = 2;
#endif
		}
		else if(0 == strcmp(long_option, "no-force")) {
			option_values.force_file_overwrite = false;
		}
		else if(0 == strcmp(long_option, "no-seektable")) {
			option_values.num_requested_seek_points = 0;
			option_values.requested_seek_points[0] = '\0';
		}
		else if(0 == strcmp(long_option, "no-delete-input-file")) {
			option_values.delete_input = false;
		}
		else if(0 == strcmp(long_option, "no-keep-foreign-metadata")) {
			option_values.keep_foreign_metadata = false;
			option_values.keep_foreign_metadata_if_present = false;
		}
		else if(0 == strcmp(long_option, "no-replay-gain")) {
			option_values.replay_gain = false;
		}
		else if(0 == strcmp(long_option, "no-ignore-chunk-sizes")) {
			option_values.ignore_chunk_sizes = false;
		}
		else if(0 == strcmp(long_option, "no-utf8-convert")) {
			option_values.utf8_convert = false;
		}
		else if(0 == strcmp(long_option, "no-lax")) {
			option_values.lax = false;
		}
#if FLAC__HAS_OGG
		else if(0 == strcmp(long_option, "no-ogg")) {
			option_values.use_ogg = false;
		}
#endif
		else if(0 == strcmp(long_option, "no-exhaustive-model-search")) {
			add_compression_setting_bool(CST_DO_EXHAUSTIVE_MODEL_SEARCH, false);
		}
		else if(0 == strcmp(long_option, "no-mid-side")) {
			add_compression_setting_bool(CST_DO_MID_SIDE, false);
			add_compression_setting_bool(CST_LOOSE_MID_SIDE, false);
		}
		else if(0 == strcmp(long_option, "no-adaptive-mid-side")) {
			add_compression_setting_bool(CST_DO_MID_SIDE, false);
			add_compression_setting_bool(CST_LOOSE_MID_SIDE, false);
		}
		else if(0 == strcmp(long_option, "no-qlp-coeff-prec-search")) {
			add_compression_setting_bool(CST_DO_QLP_COEFF_PREC_SEARCH, false);
		}
		else if(0 == strcmp(long_option, "no-padding")) {
			option_values.padding = 0;
		}
		else if(0 == strcmp(long_option, "no-verify")) {
			option_values.verify = false;
		}
		else if(0 == strcmp(long_option, "no-warnings-as-errors")) {
			option_values.treat_warnings_as_errors = false;
		}
		else if(0 == strcmp(long_option, "no-residual-gnuplot")) {
			option_values.aopts.do_residual_gnuplot = false;
		}
		else if(0 == strcmp(long_option, "no-residual-text")) {
			option_values.aopts.do_residual_text = false;
		}
		else if(0 == strcmp(long_option, "disable-constant-subframes")) {
			option_values.debug.disable_constant_subframes = true;
		}
		else if(0 == strcmp(long_option, "disable-fixed-subframes")) {
			option_values.debug.disable_fixed_subframes = true;
		}
		else if(0 == strcmp(long_option, "disable-verbatim-subframes")) {
			option_values.debug.disable_verbatim_subframes = true;
		}
		else if(0 == strcmp(long_option, "no-md5-sum")) {
			option_values.debug.do_md5 = false;
		}
		else if(0 == strcmp(long_option, "no-error-on-compression-fail")) {
			option_values.error_on_compression_fail = false;
		}
		else if(0 == strcmp(long_option, "error-on-compression-fail")) {
			option_values.error_on_compression_fail = true;
		}
	}
	else {
		switch(short_option) {
			case 'h':
				option_values.show_help = true;
				break;
			case 'v':
				option_values.show_version = true;
				break;
			case 'd':
				option_values.mode_decode = true;
				break;
			case 'a':
				option_values.mode_decode = true;
				option_values.analyze = true;
				break;
			case 't':
				option_values.mode_decode = true;
				option_values.test_only = true;
				break;
			case 'c':
				option_values.force_to_stdout = true;
				break;
			case 's':
#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
				flac__utils_verbosity_ = 1;
#endif
				break;
			case 'f':
				option_values.force_file_overwrite = true;
				break;
			case 'o':
				FLAC__ASSERT(0 != option_argument);
				option_values.cmdline_forced_outfilename = option_argument;
				break;
			case 'F':
				option_values.continue_through_decode_errors = true;
				break;
			case 'T':
				FLAC__ASSERT(0 != option_argument);
				if(!flac__vorbiscomment_add(option_values.vorbis_comment, option_argument, /*value_from_file=*/false, /*raw=*/!option_values.utf8_convert, &violation))
					return usage_error("ERROR: (-T/--tag) %s\n", violation);
				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
				add_compression_setting_uint32_t(CST_COMPRESSION_LEVEL, short_option-'0');
				break;
			case '9':
				return usage_error("ERROR: compression level '9' is reserved\n");
			case 'V':
				option_values.verify = true;
				break;
			case 'w':
				option_values.treat_warnings_as_errors = true;
				break;
			case 'S':
				FLAC__ASSERT(0 != option_argument);
				if(0 == strcmp(option_argument, "-")) {
					option_values.num_requested_seek_points = 0;
					option_values.requested_seek_points[0] = '\0';
				}
				else {
					if(option_values.num_requested_seek_points < 0)
						option_values.num_requested_seek_points = 0;
					option_values.num_requested_seek_points++;
					if(strlen(option_values.requested_seek_points)+strlen(option_argument)+2 >= sizeof(option_values.requested_seek_points)) {
						return usage_error("ERROR: too many seekpoints requested\n");
					}
					else {
						size_t len = strlen(option_values.requested_seek_points);
						flac_snprintf(option_values.requested_seek_points+len, sizeof(option_values.requested_seek_points) - len, "%s;", option_argument);
					}
				}
				break;
			case 'P':
				FLAC__ASSERT(0 != option_argument);
				option_values.padding = atoi(option_argument);
				if(option_values.padding < 0)
					return usage_error("ERROR: argument to -%c must be >= 0; for no padding use -%c-\n", short_option, short_option);
				break;
			case 'b':
				{
					uint32_t i ;
					FLAC__ASSERT(0 != option_argument);
					i = atoi(option_argument);
					if((i < (int)FLAC__MIN_BLOCK_SIZE || i > (int)FLAC__MAX_BLOCK_SIZE))
						return usage_error("ERROR: invalid blocksize (-%c) '%d', must be >= %u and <= %u\n", short_option, i, FLAC__MIN_BLOCK_SIZE, FLAC__MAX_BLOCK_SIZE);
					add_compression_setting_uint32_t(CST_BLOCKSIZE, (uint32_t)i);
				}
				break;
			case 'e':
				add_compression_setting_bool(CST_DO_EXHAUSTIVE_MODEL_SEARCH, true);
				break;
			case 'E':
				add_compression_setting_bool(CST_DO_ESCAPE_CODING, true);
				break;
			case 'l':
				{
					uint32_t i ;
					FLAC__ASSERT(0 != option_argument);
					i = atoi(option_argument);
					if(i > FLAC__MAX_LPC_ORDER)
						return usage_error("ERROR: invalid LPC order (-%c) '%d', must be >= %u and <= %u\n", short_option, i, 0, FLAC__MAX_LPC_ORDER);
					add_compression_setting_uint32_t(CST_MAX_LPC_ORDER, i);
				}
				break;
			case 'A':
				FLAC__ASSERT(0 != option_argument);
				add_compression_setting_string(CST_APODIZATION, option_argument);
				break;
			case 'm':
				add_compression_setting_bool(CST_DO_MID_SIDE, true);
				add_compression_setting_bool(CST_LOOSE_MID_SIDE, false);
				break;
			case 'M':
				add_compression_setting_bool(CST_DO_MID_SIDE, true);
				add_compression_setting_bool(CST_LOOSE_MID_SIDE, true);
				break;
			case 'p':
				add_compression_setting_bool(CST_DO_QLP_COEFF_PREC_SEARCH, true);
				break;
			case 'q':
				{
					uint32_t i ;
					FLAC__ASSERT(0 != option_argument);
					i = atoi(option_argument);
					if((i > 0 && (i < FLAC__MIN_QLP_COEFF_PRECISION || i > FLAC__MAX_QLP_COEFF_PRECISION)))
						return usage_error("ERROR: invalid value '%d' for qlp coeff precision (-%c), must be 0 or between %u and %u, inclusive\n", i, short_option, FLAC__MIN_QLP_COEFF_PRECISION, FLAC__MAX_QLP_COEFF_PRECISION);
					add_compression_setting_uint32_t(CST_QLP_COEFF_PRECISION, i);
				}
				break;
			case 'r':
				{
					uint32_t i;
					char * p;
					FLAC__ASSERT(0 != option_argument);
					p = strchr(option_argument, ',');
					if(0 == p) {
						add_compression_setting_uint32_t(CST_MIN_RESIDUAL_PARTITION_ORDER, 0);
						i = atoi(option_argument);
						if(i > FLAC__MAX_RICE_PARTITION_ORDER)
							return usage_error("ERROR: invalid value '%d' for residual partition order (-%c), must be between 0 and %u, inclusive\n", i, short_option, FLAC__MAX_RICE_PARTITION_ORDER);
						add_compression_setting_uint32_t(CST_MAX_RESIDUAL_PARTITION_ORDER, i);
					}
					else {
						i = atoi(option_argument);
						if(i > FLAC__MAX_RICE_PARTITION_ORDER)
							return usage_error("ERROR: invalid value '%d' for min residual partition order (-%c), must be between 0 and %u, inclusive\n", i, short_option, FLAC__MAX_RICE_PARTITION_ORDER);
						add_compression_setting_uint32_t(CST_MIN_RESIDUAL_PARTITION_ORDER, i);
						i = atoi(++p);
						if(i > FLAC__MAX_RICE_PARTITION_ORDER)
							return usage_error("ERROR: invalid value '%d' for max residual partition order (-%c), must be between 0 and %u, inclusive\n", i, short_option, FLAC__MAX_RICE_PARTITION_ORDER);
						add_compression_setting_uint32_t(CST_MAX_RESIDUAL_PARTITION_ORDER, i);
					}
				}
				break;
			case 'R':
				{
					uint32_t i;
					i = atoi(option_argument);
					add_compression_setting_uint32_t(CST_RICE_PARAMETER_SEARCH_DIST, i);
				}
				break;
			case 'j':
				{
					option_values.threads = atoi(option_argument);
				}
				break;
			default:
				FLAC__ASSERT(0);
		}
	}

	return 0;
}

void free_options(void)
{
	uint32_t i;
	if(0 != option_values.filenames) {
		for(i = 0; i < option_values.num_files; i++) {
			if(0 != option_values.filenames[i])
				free(option_values.filenames[i]);
		}
		free(option_values.filenames);
	}
	if(0 != option_values.vorbis_comment)
		FLAC__metadata_object_delete(option_values.vorbis_comment);
	for(i = 0; i < option_values.num_pictures; i++)
		FLAC__metadata_object_delete(option_values.pictures[i]);
}

void add_compression_setting_bool(compression_setting_type_t type, FLAC__bool value)
{
	if(option_values.num_compression_settings >= sizeof(option_values.compression_settings)/sizeof(option_values.compression_settings[0]))
#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
		die("too many compression settings");
#else
		return;
#endif
	option_values.compression_settings[option_values.num_compression_settings].type = type;
	option_values.compression_settings[option_values.num_compression_settings].value.t_bool = value;
	option_values.num_compression_settings++;
}

void add_compression_setting_string(compression_setting_type_t type, const char *value)
{
	if(option_values.num_compression_settings >= sizeof(option_values.compression_settings)/sizeof(option_values.compression_settings[0]))
#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
		die("too many compression settings");
#else
		return;
#endif
	option_values.compression_settings[option_values.num_compression_settings].type = type;
	option_values.compression_settings[option_values.num_compression_settings].value.t_string = value;
	option_values.num_compression_settings++;
}

void add_compression_setting_uint32_t(compression_setting_type_t type, uint32_t value)
{
	if(option_values.num_compression_settings >= sizeof(option_values.compression_settings)/sizeof(option_values.compression_settings[0]))
#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
		die("too many compression settings");
#else
		return;
#endif
	if(type == CST_COMPRESSION_LEVEL) {
		/* Compression level always goes first */
		option_values.compression_settings[0].type = type;
		option_values.compression_settings[0].value.t_unsigned = value;
	}
	else {
		option_values.compression_settings[option_values.num_compression_settings].type = type;
		option_values.compression_settings[option_values.num_compression_settings].value.t_unsigned = value;
		option_values.num_compression_settings++;
	}
}

int usage_error(const char *message, ...)
{
	if(flac__utils_verbosity_ >= 1) {
		va_list args;

		FLAC__ASSERT(0 != message);

		va_start(args, message);

#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
		(void) flac_vfprintf(stderr, message, args);
#endif

		va_end(args);

		flac_printf("Type \"flac\" for a usage summary or \"flac --help\" for all options\n");
	}

	return 1;
}

void show_version(void)
{
	printf("flac %s\n", FLAC__VERSION_STRING);
}

static void usage_header(void)
{
	printf("===============================================================================\n");
	printf("flac - Command-line FLAC encoder/decoder version %s\n", FLAC__VERSION_STRING);
	printf("Copyright (C) 2000-2009  Josh Coalson\n");
	printf("Copyright (C) 2011-2025  Xiph.Org Foundation\n");
	printf("\n");
	printf("This program is free software; you can redistribute it and/or\n");
	printf("modify it under the terms of the GNU General Public License\n");
	printf("as published by the Free Software Foundation; either version 2\n");
	printf("of the License, or (at your option) any later version.\n");
	printf("\n");
	printf("This program is distributed in the hope that it will be useful,\n");
	printf("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
	printf("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
	printf("GNU General Public License for more details.\n");
	printf("\n");
	printf("You should have received a copy of the GNU General Public License along\n");
	printf("with this program; if not, write to the Free Software Foundation, Inc.,\n");
	printf("51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.\n");
	printf("===============================================================================\n");
}

static void usage_summary(void)
{
	printf("Usage:\n");
	printf("\n");
	printf(" Encoding: flac [<general/encoding/format options>] [INPUTFILE [...]]\n");
	printf(" Decoding: flac -d [<general/decoding/format options>] [FLACFILE [...]]\n");
	printf("  Testing: flac -t [<general options>] [FLACFILE [...]]\n");
	printf("Analyzing: flac -a [<general/analysis options>] [FLACFILE [...]]\n");
	printf("\n");
}

void short_usage(void)
{
	usage_header();
	printf("\n");
	printf("This is the short help; for all options use 'flac --help'; for more explanation\n");
	printf("and examples please consult the manual. This manual is often distributed\n");
	printf("alongside the program as a man page or an HTML file. It can also be found\n");
	printf("online at https://xiph.org/flac/documentation_tools_flac.html\n");
	printf("\n");
	printf("To encode:\n");
	printf("  flac [-#] [INPUTFILE [...]]\n");
	printf("\n");
	printf("  -# is -0 (fastest compression) to -8 (highest compression); -5 is the default\n");
	printf("\n");
	printf("To decode:\n");
	printf("  flac -d [INPUTFILE [...]]\n");
	printf("\n");
	printf("To test:\n");
	printf("  flac -t [INPUTFILE [...]]\n");
}

void show_help(void)
{
	usage_header();
	usage_summary();
	printf("\n");
	printf("This help text summarizes all available options, for more explanation and\n");
	printf("examples please consult the manual. This manual is often distributed\n");
	printf("alongside the program as a man page or an HTML file. It can also be found\n");
	printf("online at https://xiph.org/flac/documentation_tools_flac.html\n");
	printf("\n");
	printf("General options:\n");
	printf("  -v, --version                Show the flac version number\n");
	printf("  -h, --help                   Show this screen\n");
	printf("  -d, --decode                 Decode (the default behavior is to encode)\n");
	printf("  -t, --test                   As -d except no decoded file is written,\n");
	printf("                               and with some additional checks.\n");
	printf("  -a, --analyze                As -d except an analysis file is written\n");
	printf("  -c, --stdout                 Write output to stdout\n");
	printf("  -f, --force                  Force overwriting of output files\n");
	printf("      --delete-input-file      Deletes after a successful encode/decode\n");
	printf("  -o, --output-name=FILENAME   Force the output file name\n");
	printf("      --output-prefix=STRING   Prepend STRING to output names\n");
	printf("      --preserve-modtime       (default) Output files keep timestamp of input\n");
	printf("      --keep-foreign-metadata  Save/restore WAVE or AIFF non-audio chunks\n");
	printf("      --keep-foreign-metadata-if-present     Save/restore WAVE or AIFF non-audio\n");
	printf("                           but not return an error when no such chunks are found\n");
	printf("      --skip={#|mm:ss.ss}      Skip the given initial samples for each input\n");
	printf("      --until={#|[+|-]mm:ss.ss}     Stop at the given sample for each input file\n");
	printf("      --no-utf8-convert        Do not convert tags from local charset to UTF-8\n");
	printf("  -s, --silent                 Do not write runtime encode/decode statistics\n");
	printf("      --totally-silent         Do not print anything, including errors\n");
	printf("  -w, --warnings-as-errors     Treat all warnings as errors\n");
	printf("\n");
	printf("Decoding options:\n");
	printf("  -F, --decode-through-errors  Continue decoding through stream errors\n");
	printf("      --cue=[#.#][-[#.#]]      Set the beginning and ending cuepoints to decode\n");
#if FLAC__HAS_OGG
	printf("      --decode-chained-stream  Decode all links in a chained Ogg stream, not\n");
	printf("                               just the first one\n");

#endif
	printf("\n");
	printf("Encoding options, defaulting to -5, -A \"tukey(5e-1)\" and one CPU thread:\n");
	printf("  -V, --verify                       Verify a correct encoding\n");
	printf("  -0, --compression-level-0, --fast  Synonymous with -l 0 -b 1152 -r 3\n");
	printf("  -1, --compression-level-1          Synonymous with -l 0 -b 1152 -M -r 3\n");
	printf("  -2, --compression-level-2          Synonymous with -l 0 -b 1152 -m -r 3\n");
	printf("  -3, --compression-level-3          Synonymous with -l 6 -b 4096 -r 4\n");
	printf("  -4, --compression-level-4          Synonymous with -l 8 -b 4096 -M -r 4\n");
	printf("  -5, --compression-level-5          Synonymous with -l 8 -b 4096 -m -r 5\n");
	printf("  -6, --compression-level-6          Synonymous with -l 8 -b 4096 -m -r 6\n");
	printf("                                         -A \"subdivide_tukey(2)\"\n");
	printf("  -7, --compression-level-7          Synonymous with -l 12 -b 4096 -m -r 6\n");
	printf("                                         -A \"subdivide_tukey(2)\"\n");
	printf("  -8, --compression-level-8, --best  Synonymous with -l 12 -b 4096 -m -r 6\n");
	printf("                                         -A \"subdivide_tukey(3)\"\n");
	printf("  -l, --max-lpc-order=#              Max LPC order; 0 => only fixed predictors\n");
	printf("  -b, --blocksize=#                  Specify blocksize in samples\n");
	printf("  -m, --mid-side                     Try mid-side coding for each frame\n");
	printf("  -M, --adaptive-mid-side            Adaptive choice of mid-side coding\n");
	printf("  -r, --rice-partition-order=[#,]#   Set [min,]max residual partition order\n");
	printf("  -A, --apodization=\"function\"       Window audio data with given function(s)\n");
	printf("  -e, --exhaustive-model-search      Do exhaustive model search (expensive!)\n");
	printf("  -q, --qlp-coeff-precision=#        Specify quantization precision in bits\n");
	printf("                                     (default: let encoder decide)\n");
	printf("  -p, --qlp-coeff-precision-search   Exhaustively search LP coeff quantization\n");
	printf("      --lax                          Allow encoder to generate non-Subset files\n");
	printf("      --limit-min-bitrate            Limit minimum bitrate (for streaming)\n");
	printf("  -j, --threads=#                    Set number of encoding threads\n");
	printf("      --ignore-chunk-sizes           Ignore data chunk sizes in WAVE/AIFF files\n");
	printf("      --replay-gain                  Calculate ReplayGain & store in FLAC tags\n");
	printf("      --cuesheet=FILENAME            Import cuesheet & store in CUESHEET block\n");
	printf("      --picture=SPECIFICATION        Import picture & store in PICTURE block\n");
	printf("  -T, --tag=FIELD=VALUE              Add a FLAC tag; may appear multiple times\n");
	printf("      --tag-from-file=FIELD=FILENAME     Like --tag but gets value from file\n");
	printf("  -S, --seekpoint={#|X|#x|#s}        Add seek point(s)\n");
	printf("  -P, --padding=#                    Write a PADDING block of length # bytes\n");
	printf("\n");
	printf("Format options (encoding defaults to FLAC not OGG; decoding defaults to WAVE, \n");
	printf("             chunks found by --keep-foreign-metadata-if-present will override):\n");
#if FLAC__HAS_OGG
	printf("      --ogg                          Use Ogg transport layer, output .oga\n");
	printf("      --serial-number                Ogg serial number to assign (encoding)\n");
	printf("                                     or to select for decoding\n");
#endif
	printf("      --force-aiff-format            Decode to AIFF format\n");
	printf("      --force-rf64-format            Decode to RF64 format\n");
	printf("      --force-wave64-format          Decode to Wave64 format\n");
	printf("      --force-legacy-wave-format     Decode to legacy wave format\n");
	printf("      --force-extensible-wave-format Decode to extensible wave format\n");
	printf("      --force-aiff-c-none-format     Decode to AIFF-C NONE format\n");
	printf("      --force-aiff-c-sowt-format     Decode to AIFF-C sowt format\n");
	printf("      --force-raw-format             Treat input or output as raw samples\n");
	printf("raw format options:\n");
	printf("      --sign={signed|unsigned}       Sign of samples (input/output) \n");
	printf("      --endian={big|little}          Byte order for samples (input/output)\n");
	printf("      --channels=#                   Number of channels in raw input\n");
	printf("      --bps=#                        Number of bits per sample in raw input\n");
	printf("      --sample-rate=#                Sample rate in Hz in raw input\n");
	printf("      --input-size=#                 Size in bytes of raw input from stdin\n");
	printf("\n");
	printf("Analysis options:\n");
	printf("      --residual-text          Include residual signal in text output\n");
	printf("      --residual-gnuplot       Generate gnuplot files of residual distribution\n");
	printf("\n");
	printf("Negative options (rightmost applied takes precedence):\n");
	printf("      --no-adaptive-mid-side\n");
	printf("      --no-cued-seekpoints\n");
	printf("      --no-decode-through-errors\n");
	printf("      --no-delete-input-file\n");
	printf("      --no-error-on-compression-fail\n");
	printf("      --no-force\n");
	printf("      --no-preserve-modtime\n");
	printf("      --no-keep-foreign-metadata\n");
	printf("      --no-exhaustive-model-search\n");
	printf("      --no-ignore-chunk-sizes\n");
	printf("      --no-lax\n");
	printf("      --no-mid-side\n");
#if FLAC__HAS_OGG
	printf("      --no-ogg\n");
#endif
	printf("      --no-padding\n");
	printf("      --no-qlp-coeff-prec-search\n");
	printf("      --no-replay-gain\n");
	printf("      --no-residual-gnuplot\n");
	printf("      --no-residual-text\n");
	printf("      --no-seektable\n");
	printf("      --no-silent\n");
	printf("      --no-verify\n");
	printf("      --no-warnings-as-errors\n");
}

void format_mistake(const char *infilename, FileFormat wrong, FileFormat right)
{
	/* WATCHOUT: indexed by FileFormat */
	flac__utils_printf(stderr, 1, "WARNING: %s is not a%s file; treating as a%s file\n", infilename, FileFormatString[wrong], FileFormatString[right]);
}

int encode_file(const char *infilename, FLAC__bool is_first_file, FLAC__bool is_last_file)
{
	FILE *encode_infile;
	FLAC__byte lookahead[12];
	uint32_t lookahead_length = 0, master_chunk_size = 0;
	FileFormat input_format = FORMAT_RAW;
	int retval;
	FLAC__off_t infilesize;
	encode_options_t encode_options;
	const char *outfilename = get_encoded_outfilename(infilename); /* the final name of the encoded file */
	/* internal_outfilename is the file we will actually write to; it will be a temporary name if infilename==outfilename */
	char *internal_outfilename = 0; /* NULL implies 'use outfilename' */
	size_t infilename_length;

	if(0 == outfilename) {
		flac__utils_printf(stderr, 1, "ERROR: filename too long: %s", infilename);
		return 1;
	}

	if(0 == strcmp(infilename, "-")) {
		infilesize = (FLAC__off_t)(-1);
		encode_infile = grabbag__file_get_binary_stdin();
	}
	else
	{
		infilesize = grabbag__file_get_filesize(infilename);
		if(0 == (encode_infile = flac_fopen(infilename, "rb"))) {
			flac__utils_printf(stderr, 1, "ERROR: can't open input file %s: %s\n", infilename, strerror(errno));
			return 1;
		}
	}

	if(!option_values.force_raw_format) {
		/* first set format based on name */
		infilename_length = strlen(infilename);
		if(infilename_length >= 4 && 0 == FLAC__STRCASECMP(infilename+(infilename_length-4), ".wav"))
			input_format = FORMAT_WAVE;
		else if(infilename_length >= 5 && 0 == FLAC__STRCASECMP(infilename+(infilename_length-5), ".rf64"))
			input_format = FORMAT_RF64;
		else if(infilename_length >= 4 && 0 == FLAC__STRCASECMP(infilename+(infilename_length-4), ".w64"))
			input_format = FORMAT_WAVE64;
		else if(infilename_length >= 4 && 0 == FLAC__STRCASECMP(infilename+(infilename_length-4), ".aif"))
			input_format = FORMAT_AIFF;
		else if(infilename_length >= 5 && 0 == FLAC__STRCASECMP(infilename+(infilename_length-5), ".aiff"))
			input_format = FORMAT_AIFF;
		else if(infilename_length >= 5 && 0 == FLAC__STRCASECMP(infilename+(infilename_length-5), ".flac"))
			input_format = FORMAT_FLAC;
		else if(infilename_length >= 4 && 0 == FLAC__STRCASECMP(infilename+(infilename_length-4), ".oga"))
			input_format = FORMAT_OGGFLAC;
		else if(infilename_length >= 4 && 0 == FLAC__STRCASECMP(infilename+(infilename_length-4), ".ogg"))
			input_format = FORMAT_OGGFLAC;

		/* attempt to guess the file type based on the first 12 bytes */
		if((lookahead_length = fread(lookahead, 1, 12, encode_infile)) < 12) {
			/* all supported non-raw formats have at least 12 bytes of header to read */
			if(input_format != FORMAT_RAW) {
				format_mistake(infilename, input_format, FORMAT_RAW);
				if(option_values.treat_warnings_as_errors) {
					conditional_fclose(encode_infile);
					return 1;
				}
			}
			/* force to raw */
			input_format = FORMAT_RAW;
		}
		else {
			if(!memcmp(lookahead, "ID3", 3)) {
				flac__utils_printf(stderr, 1, "ERROR: input file %s has an ID3v2 tag\n", infilename);
				conditional_fclose(encode_infile);
				return 1;
			}
			else if(!memcmp(lookahead, "RIFF", 4) && !memcmp(lookahead+8, "WAVE", 4))
				input_format = FORMAT_WAVE;
			else if(!memcmp(lookahead, "RF64", 4) && !memcmp(lookahead+8, "WAVE", 4))
				input_format = FORMAT_RF64;
			else if(!memcmp(lookahead, "riff\x2E\x91\xCF\x11\xA5\xD6\x28\xDB", 12)) /* just check 1st 12 bytes of GUID */
				input_format = FORMAT_WAVE64;
			else if(!memcmp(lookahead, "FORM", 4) && !memcmp(lookahead+8, "AIFF", 4))
				input_format = FORMAT_AIFF;
			else if(!memcmp(lookahead, "FORM", 4) && !memcmp(lookahead+8, "AIFC", 4))
				input_format = FORMAT_AIFF_C;
			else if(!memcmp(lookahead, FLAC__STREAM_SYNC_STRING, sizeof(FLAC__STREAM_SYNC_STRING)))
				input_format = FORMAT_FLAC;
			/*@@@ this could be made more accurate by looking at the first packet to make sure it's Ogg FLAC and not, say, Ogg Vorbis.  we do catch such problems later though. */
			else if(!memcmp(lookahead, "OggS", 4))
				input_format = FORMAT_OGGFLAC;
			else {
				/* didn't find header of any supported format */
				if(input_format != FORMAT_RAW) {
					format_mistake(infilename, input_format, FORMAT_RAW);
					if(option_values.treat_warnings_as_errors) {
						conditional_fclose(encode_infile);
						return 1;
					}
				}
				/* force to raw */
				input_format = FORMAT_RAW;
			}
		}
	}

	if(!option_values.ignore_chunk_sizes
	   && (input_format == FORMAT_WAVE || input_format == FORMAT_AIFF || input_format == FORMAT_AIFF_C)
	   && infilesize >= UINT32_MAX) {
		conditional_fclose(encode_infile);
		return usage_error("ERROR: file %s is too large to be valid.\n"
		                   "Please consult the manual on the --ignore-chunk-sizes option\n\n", infilename);
	}

	if(input_format == FORMAT_WAVE || input_format == FORMAT_AIFF || input_format == FORMAT_AIFF_C) {
		memcpy(&master_chunk_size,lookahead+4,sizeof(master_chunk_size));
		if((input_format != FORMAT_WAVE) != CPU_IS_BIG_ENDIAN /* logical xor */)
			/* true for WAVE on big endian CPUs or AIFF/AIFF-C on little endian CPUs */
			master_chunk_size = ENDSWAP_32(master_chunk_size);

		if(infilesize != (FLAC__off_t)(-1) && infilesize > 8 && (infilesize - 8) != master_chunk_size) {
			flac__utils_printf(stderr, 1, "WARNING: %s chunk size of file %s does not agree with filesize\n", (input_format == FORMAT_WAVE)?"RIFF":"FORM", infilename);
			if(option_values.treat_warnings_as_errors)
				return 1;
		}
	}

	if(option_values.keep_foreign_metadata || option_values.keep_foreign_metadata_if_present) {
		if(encode_infile == stdin || option_values.force_to_stdout) {
			conditional_fclose(encode_infile);
			return usage_error("ERROR: --keep-foreign-metadata cannot be used when encoding from stdin or to stdout\n");
		}
		if(input_format != FORMAT_WAVE && input_format != FORMAT_WAVE64 && input_format != FORMAT_RF64 && input_format != FORMAT_AIFF && input_format != FORMAT_AIFF_C) {
			conditional_fclose(encode_infile);
			return usage_error("ERROR: --keep-foreign-metadata can only be used with WAVE, Wave64, RF64, or AIFF input\n");
		}
	}

	/*
	 * Error if output file already exists (and -f not used).
	 * Use grabbag__file_get_filesize() as a cheap way to check.
	 */
	if(!option_values.test_only && !option_values.force_file_overwrite && strcmp(outfilename, "-") && grabbag__file_get_filesize(outfilename) != (FLAC__off_t)(-1)) {
		if(input_format == FORMAT_FLAC) {
			/* need more detailed error message when re-flac'ing to avoid confusing the user */
			flac__utils_printf(stderr, 1,
				"ERROR: output file %s already exists.\n\n"
				"By default flac encodes files to FLAC format; if you meant to decode this file\n"
				"from FLAC to something else, use -d.  If you meant to re-encode this file from\n"
				"FLAC to FLAC again, use -f to force writing to the same file, or -o to specify\n"
				"a different output filename.\n",
				outfilename
			);
		}
		else if(input_format == FORMAT_OGGFLAC) {
			/* need more detailed error message when re-flac'ing to avoid confusing the user */
			flac__utils_printf(stderr, 1,
				"ERROR: output file %s already exists.\n\n"
				"By default 'flac -ogg' encodes files to Ogg FLAC format; if you meant to decode\n"
				"this file from Ogg FLAC to something else, use -d.  If you meant to re-encode\n"
				"this file from Ogg FLAC to Ogg FLAC again, use -f to force writing to the same\n"
				"file, or -o to specify a different output filename.\n",
				outfilename
			);
		}
		else
			flac__utils_printf(stderr, 1, "ERROR: output file %s already exists, use -f to override\n", outfilename);
		conditional_fclose(encode_infile);
		return 1;
	}

	if(option_values.format_input_size >= 0) {
	   	if (input_format != FORMAT_RAW || infilesize >= 0) {
			flac__utils_printf(stderr, 1, "ERROR: can only use --input-size when encoding raw samples from stdin\n");
			conditional_fclose(encode_infile);
			return 1;
		}
		else {
			infilesize = option_values.format_input_size;
		}
	}

	if(input_format == FORMAT_RAW) {
		if(option_values.format_is_big_endian < 0 || option_values.format_is_unsigned_samples < 0 || option_values.format_channels < 0 || option_values.format_bps < 0 || option_values.format_sample_rate < 0) {
			conditional_fclose(encode_infile);
			return usage_error("ERROR: for encoding a raw file you must specify a value for --endian, --sign, --channels, --bps, and --sample-rate\n");
		}
	}
	else {
		if(option_values.format_is_big_endian >= 0 || option_values.format_is_unsigned_samples >= 0 || option_values.format_channels >= 0 || option_values.format_bps >= 0 || option_values.format_sample_rate >= 0) {
			conditional_fclose(encode_infile);
			return usage_error("ERROR: raw format options (--endian, --sign, --channels, --bps, and --sample-rate) are not allowed for non-raw input\n");
		}
	}

	if(option_values.force_to_stdout) {
		if(option_values.replay_gain) {
			conditional_fclose(encode_infile);
			return usage_error("ERROR: --replay-gain cannot be used when encoding to stdout\n");
		}
	}
	if(option_values.replay_gain && option_values.use_ogg) {
		conditional_fclose(encode_infile);
		return usage_error("ERROR: --replay-gain cannot be used when encoding to Ogg FLAC yet\n");
	}

	if(!flac__utils_parse_skip_until_specification(option_values.skip_specification, &encode_options.skip_specification) || encode_options.skip_specification.is_relative) {
		conditional_fclose(encode_infile);
		return usage_error("ERROR: invalid value for --skip\n");
	}

	if(!flac__utils_parse_skip_until_specification(option_values.until_specification, &encode_options.until_specification)) { /*@@@@ more checks: no + without --skip, no - unless known total_samples_to_{en,de}code */
		conditional_fclose(encode_infile);
		return usage_error("ERROR: invalid value for --until\n");
	}
	/* if there is no "--until" we want to default to "--until=-0" */
	if(0 == option_values.until_specification)
		encode_options.until_specification.is_relative = true;

	encode_options.verify = option_values.verify;
	encode_options.treat_warnings_as_errors = option_values.treat_warnings_as_errors;
#if FLAC__HAS_OGG
	encode_options.use_ogg = option_values.use_ogg;
	/* set a random serial number if one has not yet been specified */
	if(!option_values.has_serial_number) {
	        if (RAND_MAX < 0x7fffffff)
			option_values.serial_number = (uint32_t)(rand() & 0x7fff) << 16 | (uint32_t)(rand());
	        else
			option_values.serial_number = rand();
		option_values.has_serial_number = true;
	}
	encode_options.serial_number = option_values.serial_number;
	option_values.serial_number = (uint32_t)option_values.serial_number + 1;
#endif
	encode_options.lax = option_values.lax;
	encode_options.padding = option_values.padding;
	encode_options.num_compression_settings = option_values.num_compression_settings;
	FLAC__ASSERT(sizeof(encode_options.compression_settings) >= sizeof(option_values.compression_settings));
	memcpy(encode_options.compression_settings, option_values.compression_settings, sizeof(option_values.compression_settings));
	encode_options.threads = option_values.threads;
	encode_options.requested_seek_points = option_values.requested_seek_points;
	encode_options.num_requested_seek_points = option_values.num_requested_seek_points;
	encode_options.cuesheet_filename = option_values.cuesheet_filename;
	encode_options.continue_through_decode_errors = option_values.continue_through_decode_errors;
	encode_options.cued_seekpoints = option_values.cued_seekpoints;
	encode_options.channel_map_none = option_values.channel_map_none;
	encode_options.is_first_file = is_first_file;
	encode_options.is_last_file = is_last_file;
	encode_options.replay_gain = option_values.replay_gain;
	encode_options.ignore_chunk_sizes = option_values.ignore_chunk_sizes;
	encode_options.vorbis_comment = option_values.vorbis_comment;
	FLAC__ASSERT(sizeof(encode_options.pictures) >= sizeof(option_values.pictures));
	memcpy(encode_options.pictures, option_values.pictures, sizeof(option_values.pictures));
	encode_options.num_pictures = option_values.num_pictures;
	encode_options.format = input_format;
	encode_options.debug.disable_constant_subframes = option_values.debug.disable_constant_subframes;
	encode_options.debug.disable_fixed_subframes = option_values.debug.disable_fixed_subframes;
	encode_options.debug.disable_verbatim_subframes = option_values.debug.disable_verbatim_subframes;
	encode_options.debug.do_md5 = option_values.debug.do_md5;
	encode_options.error_on_compression_fail = option_values.error_on_compression_fail;
	encode_options.limit_min_bitrate = option_values.limit_min_bitrate;
	encode_options.relaxed_foreign_metadata_handling = option_values.keep_foreign_metadata_if_present;

	/* if infilename and outfilename point to the same file, we need to write to a temporary file */
	if(encode_infile != stdin && grabbag__file_are_same(infilename, outfilename)) {
		static const char *tmp_suffix = ".tmp,fl-ac+en'c";
		size_t dest_len = strlen(outfilename) + strlen(tmp_suffix) + 1;
		/*@@@@ still a remote possibility that a file with this filename exists */
		if((internal_outfilename = safe_malloc_(dest_len)) == NULL) {
			flac__utils_printf(stderr, 1, "ERROR allocating memory for tempfile name\n");
			conditional_fclose(encode_infile);
			return 1;
		}
		flac_snprintf(internal_outfilename, dest_len, "%s%s", outfilename, tmp_suffix);
	}

	if(input_format == FORMAT_RAW) {
		encode_options.format_options.raw.is_big_endian = option_values.format_is_big_endian;
		encode_options.format_options.raw.is_unsigned_samples = option_values.format_is_unsigned_samples;
		encode_options.format_options.raw.channels = option_values.format_channels;
		encode_options.format_options.raw.bps = option_values.format_bps;
		encode_options.format_options.raw.sample_rate = option_values.format_sample_rate;

		retval = flac__encode_file(encode_infile, infilesize, infilename, internal_outfilename? internal_outfilename : outfilename, lookahead, lookahead_length, encode_options);
	}
	else if(input_format == FORMAT_FLAC || input_format == FORMAT_OGGFLAC) {
		retval = flac__encode_file(encode_infile, infilesize, infilename, internal_outfilename? internal_outfilename : outfilename, lookahead, lookahead_length, encode_options);
	}
	else if(input_format == FORMAT_WAVE || input_format == FORMAT_WAVE64 || input_format == FORMAT_RF64 || input_format == FORMAT_AIFF || input_format == FORMAT_AIFF_C) {
		encode_options.format_options.iff.foreign_metadata = 0;

		/* initialize foreign metadata if requested */
		if(option_values.keep_foreign_metadata || option_values.keep_foreign_metadata_if_present) {
			encode_options.format_options.iff.foreign_metadata =
				flac__foreign_metadata_new(
					input_format==FORMAT_WAVE || input_format==FORMAT_RF64?
						FOREIGN_BLOCK_TYPE__RIFF :
					input_format==FORMAT_WAVE64?
						FOREIGN_BLOCK_TYPE__WAVE64 :
						FOREIGN_BLOCK_TYPE__AIFF
				);
			if(0 == encode_options.format_options.iff.foreign_metadata) {
				flac__utils_printf(stderr, 1, "ERROR: creating foreign metadata object\n");
				conditional_fclose(encode_infile);
				if(internal_outfilename != 0)
					free(internal_outfilename);
				return 1;
			}
		}

		retval = flac__encode_file(encode_infile, infilesize, infilename, internal_outfilename? internal_outfilename : outfilename, lookahead, lookahead_length, encode_options);

		if(encode_options.format_options.iff.foreign_metadata)
			flac__foreign_metadata_delete(encode_options.format_options.iff.foreign_metadata);
	}
	else {
		FLAC__ASSERT(0);
		retval = 1; /* double protection */
	}

	if(retval == 0) {
		if(strcmp(outfilename, "-")) {
			if(option_values.replay_gain) {
				float title_gain, title_peak;
				const char *error;
				grabbag__replaygain_get_title(&title_gain, &title_peak);
				if(
					0 != (error = grabbag__replaygain_store_to_file_reference(internal_outfilename? internal_outfilename : outfilename, option_values.preserve_modtime)) ||
					0 != (error = grabbag__replaygain_store_to_file_title(internal_outfilename? internal_outfilename : outfilename, title_gain, title_peak, option_values.preserve_modtime))
				) {
					flac__utils_printf(stderr, 1, "%s: ERROR writing ReplayGain reference/title tags (%s)\n", outfilename, error);
					retval = 1;
				}
			}
			if(option_values.preserve_modtime && strcmp(infilename, "-"))
				grabbag__file_copy_metadata(infilename, internal_outfilename? internal_outfilename : outfilename);
		}
	}

	/* rename temporary file if necessary */
	if(retval == 0 && internal_outfilename != 0) {
		if(flac_rename(internal_outfilename, outfilename) < 0) {
#if defined _MSC_VER || defined __MINGW32__ || defined __EMX__
			/* on some flavors of windows, flac_rename() will fail if the destination already exists, so we unlink and try again */
			if(flac_unlink(outfilename) < 0) {
				flac__utils_printf(stderr, 1, "ERROR: moving new FLAC file %s back on top of original FLAC file %s, keeping both\n", internal_outfilename, outfilename);
				retval = 1;
			}
			else if(flac_rename(internal_outfilename, outfilename) < 0) {
				flac__utils_printf(stderr, 1, "ERROR: moving new FLAC file %s back on top of original FLAC file %s, you must do it\n", internal_outfilename, outfilename);
				retval = 1;
			}
#else
			flac__utils_printf(stderr, 1, "ERROR: moving new FLAC file %s back on top of original FLAC file %s, keeping both\n", internal_outfilename, outfilename);
			retval = 1;
#endif
		}
	}

	/* handle --delete-input-file, but don't want to delete if piping from stdin, or if input filename and output filename are the same */
	if(retval == 0 && option_values.delete_input && strcmp(infilename, "-") && internal_outfilename == 0)
		flac_unlink(infilename);

	if(internal_outfilename != 0)
		free(internal_outfilename);

	return retval;
}

int decode_file(const char *infilename)
{
	int retval;
	FLAC__bool treat_as_ogg = false;
	FileFormat output_format = FORMAT_WAVE;
	FileSubFormat output_subformat = SUBFORMAT_UNSPECIFIED;
	decode_options_t decode_options;
	foreign_metadata_t *foreign_metadata = 0;
	const char *outfilename = get_outfilename(infilename, ".    "); /* Placeholder until we know what the actual suffix is */
	size_t infilename_length;

	if(0 == outfilename) {
		flac__utils_printf(stderr, 1, "ERROR: filename too long: %s", infilename);
		return 1;
	}

	if(!option_values.analyze && !option_values.test_only &&(option_values.keep_foreign_metadata || option_values.keep_foreign_metadata_if_present)) {
		const char *error;
		if(0 == strcmp(infilename, "-") || 0 == strcmp(outfilename, "-"))
			return usage_error("ERROR: --keep-foreign-metadata cannot be used when decoding from stdin or to stdout\n");
		if(output_format == FORMAT_RAW)
			return usage_error("ERROR: --keep-foreign-metadata cannot be used with raw output\n");
		decode_options.format_options.iff.foreign_metadata = 0;
		/* initialize foreign metadata structure */
		foreign_metadata = flac__foreign_metadata_new(FOREIGN_BLOCK_TYPE__RIFF); /* RIFF is just a placeholder */
		if(0 == foreign_metadata) {
			flac__utils_printf(stderr, 1, "ERROR: creating foreign metadata object\n");
			return 1;
		}
		if(!flac__foreign_metadata_read_from_flac(foreign_metadata, infilename, &error)) {
			if(option_values.keep_foreign_metadata_if_present) {
				flac__utils_printf(stderr, 1, "%s: WARNING reading foreign metadata: %s\n", infilename, error);
				if(option_values.treat_warnings_as_errors) {
					flac__foreign_metadata_delete(foreign_metadata);
					return 1;
				}
				else {
					/* Couldn't find foreign metadata, stop processing */
					flac__foreign_metadata_delete(foreign_metadata);
					foreign_metadata = 0;
				}
			}
			else {
				flac__utils_printf(stderr, 1, "%s: ERROR reading foreign metadata: %s\n", infilename, error);
				flac__foreign_metadata_delete(foreign_metadata);
				return 1;
			}
		}
	}

	if(option_values.force_raw_format)
		output_format = FORMAT_RAW;
	else if(
		option_values.force_aiff_format ||
		(strlen(outfilename) >= 4 && 0 == FLAC__STRCASECMP(outfilename+(strlen(outfilename)-4), ".aif")) ||
		(strlen(outfilename) >= 5 && 0 == FLAC__STRCASECMP(outfilename+(strlen(outfilename)-5), ".aiff"))
	)
		output_format = FORMAT_AIFF;
	else if(
		option_values.force_rf64_format ||
		(strlen(outfilename) >= 5 && 0 == FLAC__STRCASECMP(outfilename+(strlen(outfilename)-5), ".rf64"))
	)
		output_format = FORMAT_RF64;
	else if(
		option_values.force_wave64_format ||
		(strlen(outfilename) >= 4 && 0 == FLAC__STRCASECMP(outfilename+(strlen(outfilename)-4), ".w64"))
	)
		output_format = FORMAT_WAVE64;
	else if(foreign_metadata != NULL) {
		/* Pick a format based on what the foreign metadata contains */
		if(foreign_metadata->type == FOREIGN_BLOCK_TYPE__WAVE64)
			output_format = FORMAT_WAVE64;
		else if(foreign_metadata->is_rf64)
			output_format = FORMAT_RF64;
		else if(foreign_metadata->type == FOREIGN_BLOCK_TYPE__AIFF) {
			output_format = FORMAT_AIFF;
			if(foreign_metadata->is_aifc) {
				output_format = FORMAT_AIFF_C;
			}
		}
		else
			output_format = FORMAT_WAVE;
	}
	else
		output_format = FORMAT_WAVE;

	/* Now do subformats */
	if(option_values.force_legacy_wave_format) {
		output_format = FORMAT_WAVE;
		output_subformat = SUBFORMAT_WAVE_PCM;
	}
	else if(option_values.force_extensible_wave_format) {
		output_format = FORMAT_WAVE;
		output_subformat = SUBFORMAT_WAVE_EXTENSIBLE;
	}
	else if(option_values.force_aiff_c_none_format) {
		output_format = FORMAT_AIFF_C;
		output_subformat = SUBFORMAT_AIFF_C_NONE;
	}
	else if(option_values.force_aiff_c_sowt_format) {
		output_format = FORMAT_AIFF_C;
		output_subformat = SUBFORMAT_AIFF_C_SOWT;
	}
	else if(foreign_metadata != NULL) {
		if(foreign_metadata->is_wavefmtex)
			output_subformat = SUBFORMAT_WAVE_EXTENSIBLE;
		else if(output_format == FORMAT_WAVE)
			output_subformat = SUBFORMAT_WAVE_PCM;
		else if(foreign_metadata->is_aifc) {
			if(foreign_metadata->is_sowt)
				output_subformat = SUBFORMAT_AIFF_C_SOWT;
			else
				output_subformat = SUBFORMAT_AIFF_C_NONE;
		}
	}


	/* Check whether output format agrees with foreign metadata */
	if(foreign_metadata != NULL) {
		if((output_format != FORMAT_WAVE && output_format != FORMAT_RF64) && foreign_metadata->type == FOREIGN_BLOCK_TYPE__RIFF) {
			flac__foreign_metadata_delete(foreign_metadata);
			return usage_error("ERROR: foreign metadata type RIFF cannot be restored to a%s file, only to WAVE and RF64\n",FileFormatString[output_format]);
		}
		if((output_format != FORMAT_AIFF && output_format != FORMAT_AIFF_C) && foreign_metadata->type == FOREIGN_BLOCK_TYPE__AIFF) {
			flac__foreign_metadata_delete(foreign_metadata);
			return usage_error("ERROR: foreign metadata type AIFF cannot be restored to a%s file, only to AIFF and AIFF-C\n",FileFormatString[output_format]);
		}
		if(output_format != FORMAT_WAVE64 && foreign_metadata->type == FOREIGN_BLOCK_TYPE__WAVE64) {
			flac__foreign_metadata_delete(foreign_metadata);
			return usage_error("ERROR: foreign metadata type Wave64 cannot be restored to a%s file, only to Wave64\n",FileFormatString[output_format]);
		}
	}

	/* Now reassemble outfilename */
	get_decoded_outfilename(infilename, output_format);

	/*
	 * Error if output file already exists (and -f not used).
	 * Use grabbag__file_get_filesize() as a cheap way to check.
	 */
	if(!option_values.test_only && !option_values.force_file_overwrite && strcmp(outfilename, "-") && grabbag__file_get_filesize(outfilename) != (FLAC__off_t)(-1)) {
		flac__utils_printf(stderr, 1, "ERROR: output file %s already exists, use -f to override\n", outfilename);
		flac__foreign_metadata_delete(foreign_metadata);
		return 1;
	}

	if(!option_values.test_only && !option_values.analyze) {
		if(output_format == FORMAT_RAW && (option_values.format_is_big_endian < 0 || option_values.format_is_unsigned_samples < 0)) {
			flac__foreign_metadata_delete(foreign_metadata);
			return usage_error("ERROR: for decoding to a raw file you must specify a value for --endian and --sign\n");
		}
	}

	infilename_length = strlen(infilename);
	if(option_values.use_ogg)
		treat_as_ogg = true;
	else if(infilename_length >= 4 && 0 == FLAC__STRCASECMP(infilename+(infilename_length-4), ".oga"))
		treat_as_ogg = true;
	else if(infilename_length >= 4 && 0 == FLAC__STRCASECMP(infilename+(infilename_length-4), ".ogg"))
		treat_as_ogg = true;
	else
		treat_as_ogg = false;

#if !FLAC__HAS_OGG
	if(treat_as_ogg) {
		flac__utils_printf(stderr, 1, "%s: Ogg support has not been built into this copy of flac\n", infilename);
		flac__foreign_metadata_delete(foreign_metadata);
		return 1;
	}
#endif

	if(!flac__utils_parse_skip_until_specification(option_values.skip_specification, &decode_options.skip_specification) || decode_options.skip_specification.is_relative) {
		flac__foreign_metadata_delete(foreign_metadata);
		return usage_error("ERROR: invalid value for --skip\n");
	}

	if(!flac__utils_parse_skip_until_specification(option_values.until_specification, &decode_options.until_specification)) { /*@@@ more checks: no + without --skip, no - unless known total_samples_to_{en,de}code */
		flac__foreign_metadata_delete(foreign_metadata);
		return usage_error("ERROR: invalid value for --until\n");
	}
	/* if there is no "--until" we want to default to "--until=-0" */
	if(0 == option_values.until_specification)
		decode_options.until_specification.is_relative = true;

	if(option_values.cue_specification) {
		if(!flac__utils_parse_cue_specification(option_values.cue_specification, &decode_options.cue_specification)) {
			flac__foreign_metadata_delete(foreign_metadata);
			return usage_error("ERROR: invalid value for --cue\n");
		}
		decode_options.has_cue_specification = true;
	}
	else
		decode_options.has_cue_specification = false;

	decode_options.treat_warnings_as_errors = option_values.treat_warnings_as_errors;
	decode_options.continue_through_decode_errors = option_values.continue_through_decode_errors;
	decode_options.relaxed_foreign_metadata_handling = option_values.keep_foreign_metadata_if_present;
	decode_options.replaygain_synthesis_spec = option_values.replaygain_synthesis_spec;
	decode_options.force_subformat = output_subformat;
#if FLAC__HAS_OGG
	decode_options.is_ogg = treat_as_ogg;
	decode_options.decode_chained_stream = option_values.decode_chained_stream;
	decode_options.use_first_serial_number = !option_values.has_serial_number;
	decode_options.serial_number = option_values.serial_number;
#endif
	decode_options.channel_map_none = option_values.channel_map_none;
	decode_options.format = output_format;

	if(output_format == FORMAT_RAW) {
		decode_options.format_options.raw.is_big_endian = option_values.format_is_big_endian;
		decode_options.format_options.raw.is_unsigned_samples = option_values.format_is_unsigned_samples;

		retval = flac__decode_file(infilename, option_values.test_only? 0 : outfilename, option_values.analyze, option_values.aopts, decode_options);
	}
	else {
		decode_options.format_options.iff.foreign_metadata = foreign_metadata;

		retval = flac__decode_file(infilename, option_values.test_only? 0 : outfilename, option_values.analyze, option_values.aopts, decode_options);

	}

	if(foreign_metadata)
		flac__foreign_metadata_delete(foreign_metadata);

	if(retval == 0 && strcmp(infilename, "-")) {
		if(option_values.preserve_modtime && strcmp(outfilename, "-"))
			grabbag__file_copy_metadata(infilename, outfilename);
		if(option_values.delete_input && !option_values.test_only && !option_values.analyze)
			flac_unlink(infilename);
	}

	return retval;
}

const char *get_encoded_outfilename(const char *infilename)
{
	const char *suffix = (option_values.use_ogg? ".oga" : ".flac");
	const char *p;

	if(option_values.output_prefix) {
		p = grabbag__file_get_basename(infilename);
	}
	else {
		p = infilename;
	}

	return get_outfilename(p, suffix);
}

const char *get_decoded_outfilename(const char *infilename, const FileFormat format)
{
	const char *suffix;
	const char *p;

	if(option_values.output_prefix) {
		p = grabbag__file_get_basename(infilename);
	}
	else {
		p = infilename;
	}

	if(option_values.analyze) {
		suffix = ".ana";
	}
	else if(format == FORMAT_RAW) {
		suffix = ".raw";
	}
	else if(format == FORMAT_AIFF) {
		suffix = ".aiff";
	}
	else if(format == FORMAT_AIFF_C) {
		suffix = ".aifc";
	}
	else if(format == FORMAT_RF64) {
		suffix = ".rf64";
	}
	else if(format == FORMAT_WAVE64) {
		suffix = ".w64";
	}
	else {
		suffix = ".wav";
	}
	return get_outfilename(p, suffix);
}

const char *get_outfilename(const char *infilename, const char *suffix)
{
	if(0 == option_values.cmdline_forced_outfilename) {
		static char buffer[4096];

		if(0 == strcmp(infilename, "-") || option_values.force_to_stdout) {
			buffer [0] = '-';
			buffer [1] = 0;
		}
		else {
			char *p;
			if (flac__strlcpy(buffer, option_values.output_prefix? option_values.output_prefix : "", sizeof buffer) >= sizeof buffer)
				return 0;
			if (flac__strlcat(buffer, infilename, sizeof buffer) >= sizeof buffer)
				return 0;
			/* the . must come after any / to avoid problems with, e.g. "some.directory/extensionless-filename" */
			if(0 == (p = strrchr(buffer, '.')) || strchr(p, '/')) {
				if (flac__strlcat(buffer, suffix, sizeof buffer) >= sizeof buffer)
					return 0;
			}
			else {
				*p = '\0';
				if (flac__strlcat(buffer, suffix, sizeof buffer) >= sizeof buffer)
					return 0;
			}
		}
		return buffer;
	}
	else
		return option_values.cmdline_forced_outfilename;
}

void die(const char *message)
{
	FLAC__ASSERT(0 != message);
	flac__utils_printf(stderr, 1, "ERROR: %s\n", message);
	exit(1);
}

int conditional_fclose(FILE *f)
{
	if(f == 0 || f == stdin || f == stdout)
		return 0;
	else
		return fclose(f);
}

char *local_strdup(const char *source)
{
	char *ret;
	FLAC__ASSERT(0 != source);
	if(0 == (ret = strdup(source)))
		die("out of memory during strdup()");
	return ret;
}
