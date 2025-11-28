/* metaflac - Command-line FLAC metadata editor
 * Copyright (C) 2001-2009  Josh Coalson
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

#include "utils.h"
#include "usage.h"
#include "FLAC/format.h"
#include <stdarg.h>
#include <stdio.h>
#include "share/compat.h"

static void usage_header(FILE *out)
{
	flac_fprintf(out, "==============================================================================\n");
	flac_fprintf(out, "metaflac - Command-line FLAC metadata editor version %s\n", FLAC__VERSION_STRING);
	flac_fprintf(out, "Copyright (C) 2001-2009  Josh Coalson\n");
	flac_fprintf(out, "Copyright (C) 2011-2025  Xiph.Org Foundation\n");
	flac_fprintf(out, "\n");
	flac_fprintf(out, "This program is free software; you can redistribute it and/or\n");
	flac_fprintf(out, "modify it under the terms of the GNU General Public License\n");
	flac_fprintf(out, "as published by the Free Software Foundation; either version 2\n");
	flac_fprintf(out, "of the License, or (at your option) any later version.\n");
	flac_fprintf(out, "\n");
	flac_fprintf(out, "This program is distributed in the hope that it will be useful,\n");
	flac_fprintf(out, "but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
	flac_fprintf(out, "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
	flac_fprintf(out, "GNU General Public License for more details.\n");
	flac_fprintf(out, "\n");
	flac_fprintf(out, "You should have received a copy of the GNU General Public License along\n");
	flac_fprintf(out, "with this program; if not, write to the Free Software Foundation, Inc.,\n");
	flac_fprintf(out, "51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.\n");
	flac_fprintf(out, "==============================================================================\n");
}

static void usage_summary(FILE *out)
{
	flac_fprintf(out, "Usage:\n");
	flac_fprintf(out, "  metaflac [options] [operations] FLACfile [FLACfile ...]\n");
	flac_fprintf(out, "\n");
	flac_fprintf(out, "Use metaflac to list, add, remove, or edit metadata in one or more FLAC files.\n");
	flac_fprintf(out, "You may perform one major operation, or many shorthand operations at a time.\n");
	flac_fprintf(out, "\n");
	flac_fprintf(out, "Options:\n");
	flac_fprintf(out, "-o, --output-name=FILENAME   Write changes to a new file, instead of doing all\n");
	flac_fprintf(out, "                             operations on the input files\n");
	flac_fprintf(out, "--preserve-modtime    Preserve the original modification time in spite of edits\n");
	flac_fprintf(out, "                      This option does nothing when combined with --output-name\n");
	flac_fprintf(out, "--with-filename       Prefix each output line with the FLAC file name\n");
	flac_fprintf(out, "                      (the default if more than one FLAC file is specified).\n");
	flac_fprintf(out, "                      This option has no effect for options exporting to a\n");
	flac_fprintf(out, "                      file, like --export-tags-to.\n");
	flac_fprintf(out, "--no-filename         Do not prefix each output line with the FLAC file name\n");
	flac_fprintf(out, "                      (the default if only one FLAC file is specified)\n");
	flac_fprintf(out, "--no-utf8-convert     Do not convert tags from UTF-8 to local charset,\n");
	flac_fprintf(out, "                      or vice versa.  This is useful for scripts, and setting\n");
	flac_fprintf(out, "                      tags in situations where the locale is wrong.\n");
	flac_fprintf(out, "--dont-use-padding    By default metaflac tries to use padding where possible\n");
	flac_fprintf(out, "                      to avoid rewriting the entire file if the metadata size\n");
	flac_fprintf(out, "                      changes.  Use this option to tell metaflac to not take\n");
	flac_fprintf(out, "                      advantage of padding this way.\n");
}

int short_usage(const char *message, ...)
{
	va_list args;

	if(message) {
		va_start(args, message);

		(void) flac_vfprintf(stderr, message, args);

		va_end(args);

	}
	usage_header(stderr);
	flac_fprintf(stderr, "\n");
	flac_fprintf(stderr, "This is the short help; for full help use 'metaflac --help'\n");
	flac_fprintf(stderr, "\n");
	usage_summary(stderr);

	return message? 1 : 0;
}

int long_usage(const char *message, ...)
{
	FILE *out = (message? stderr : stdout);
	va_list args;

	if(message) {
		va_start(args, message);

		(void) flac_vfprintf(stderr, message, args);

		va_end(args);

	}
	usage_header(out);
	flac_fprintf(out, "\n");
	usage_summary(out);
	flac_fprintf(out, "\n");
	flac_fprintf(out, "Shorthand operations:\n");
	flac_fprintf(out, "--show-md5sum         Show the MD5 signature from the STREAMINFO block.\n");
	flac_fprintf(out, "--show-min-blocksize  Show the minimum block size from the STREAMINFO block.\n");
	flac_fprintf(out, "--show-max-blocksize  Show the maximum block size from the STREAMINFO block.\n");
	flac_fprintf(out, "--show-min-framesize  Show the minimum frame size from the STREAMINFO block.\n");
	flac_fprintf(out, "--show-max-framesize  Show the maximum frame size from the STREAMINFO block.\n");
	flac_fprintf(out, "--show-sample-rate    Show the sample rate from the STREAMINFO block.\n");
	flac_fprintf(out, "--show-channels       Show the number of channels from the STREAMINFO block.\n");
	flac_fprintf(out, "--show-bps            Show the # of bits per sample from the STREAMINFO block.\n");
	flac_fprintf(out, "--show-total-samples  Show the total # of samples from the STREAMINFO block.\n");
	flac_fprintf(out, "\n");
	flac_fprintf(out, "--show-vendor-tag     Show the vendor string from the VORBIS_COMMENT block.\n");
	flac_fprintf(out, "--show-tag=NAME       Show all tags where the field name matches 'NAME'.\n");
	flac_fprintf(out, "--show-all-tags       Show all tags. This is an alias for --export-tags-to=-.\n");
	flac_fprintf(out, "--remove-tag=NAME     Remove all tags whose field name is 'NAME'.\n");
	flac_fprintf(out, "--remove-first-tag=NAME  Remove first tag whose field name is 'NAME'.\n");
	flac_fprintf(out, "--remove-all-tags     Remove all tags, leaving only the vendor string.\n");
	flac_fprintf(out, "--remove-all-tags-except=NAME1[=NAME2[=...]] Remove all tags, except the vendor\n");
	flac_fprintf(out, "                      string and the tag names specified. Tag names must be\n");
	flac_fprintf(out, "                      separated by an = character.\n");
	flac_fprintf(out, "--set-tag=FIELD       Add a tag.  The FIELD must comply with the Vorbis comment\n");
	flac_fprintf(out, "                      spec, of the form \"NAME=VALUE\".  If there is currently\n");
	flac_fprintf(out, "                      no tag block, one will be created.\n");
	flac_fprintf(out, "--set-tag-from-file=FIELD   Like --set-tag, except the VALUE is a filename\n");
	flac_fprintf(out, "                      whose contents will be read verbatim to set the tag value.\n");
	flac_fprintf(out, "                      Unless --no-utf8-convert is specified, the contents will\n");
	flac_fprintf(out, "                      be converted to UTF-8 from the local charset.  This can\n");
	flac_fprintf(out, "                      be used to store a cuesheet in a tag (e.g.\n");
	flac_fprintf(out, "                      --set-tag-from-file=\"CUESHEET=image.cue\").  Do not try\n");
	flac_fprintf(out, "                      to store binary data in tag fields!  Use APPLICATION\n");
	flac_fprintf(out, "                      blocks for that.\n");
	flac_fprintf(out, "--import-tags-from=FILE Import tags from a file.  Use '-' for stdin.  Each line\n");
	flac_fprintf(out, "                      should be of the form NAME=VALUE.  Multi-line comments\n");
	flac_fprintf(out, "                      are currently not supported.  Specify --remove-all-tags\n");
	flac_fprintf(out, "                      and/or --no-utf8-convert before --import-tags-from if\n");
	flac_fprintf(out, "                      necessary.  If FILE is '-' (stdin), only one FLAC file\n");
	flac_fprintf(out, "                      may be specified.\n");
	flac_fprintf(out, "--export-tags-to=FILE Export tags to a file.  Use '-' for stdout.  Each line\n");
	flac_fprintf(out, "                      will be of the form NAME=VALUE.  Specify\n");
	flac_fprintf(out, "                      --no-utf8-convert if necessary.\n");
	flac_fprintf(out, "--import-cuesheet-from=FILE  Import a cuesheet from a file.  Use '-' for stdin.\n");
	flac_fprintf(out, "                      Only one FLAC file may be specified.  A seekpoint will be\n");
	flac_fprintf(out, "                      added for each index point in the cuesheet to the\n");
	flac_fprintf(out, "                      SEEKTABLE unless --no-cued-seekpoints is specified.\n");
	flac_fprintf(out, "--export-cuesheet-to=FILE  Export CUESHEET block to a cuesheet file, suitable\n");
	flac_fprintf(out, "                      for use by CD authoring software.  Use '-' for stdout.\n");
	flac_fprintf(out, "                      Only one FLAC file may be specified on the command line.\n");
	flac_fprintf(out, "--import-picture-from=FILENAME|SPECIFICATION  Import a picture and store it in a\n");
	flac_fprintf(out, "                      PICTURE block.  Either a filename for the picture file or\n");
	flac_fprintf(out, "                      a more complete specification form can be used.  The\n");
	flac_fprintf(out, "                      SPECIFICATION is a string whose parts are separated by |\n");
	flac_fprintf(out, "                      characters.  Some parts may be left empty to invoke\n");
	flac_fprintf(out, "                      default values.  FILENAME is just shorthand for\n");
	flac_fprintf(out, "                      \"||||FILENAME\".  The format of SPECIFICATION is:\n");
	flac_fprintf(out, "         [TYPE]|[MIME-TYPE]|[DESCRIPTION]|[WIDTHxHEIGHTxDEPTH[/COLORS]]|FILE\n");
	flac_fprintf(out, "           TYPE is optional; it is a number from one of:\n");
	flac_fprintf(out, "              0: Other\n");
	flac_fprintf(out, "              1: 32x32 pixels 'file icon' (PNG only)\n");
	flac_fprintf(out, "              2: Other file icon\n");
	flac_fprintf(out, "              3: Cover (front)\n");
	flac_fprintf(out, "              4: Cover (back)\n");
	flac_fprintf(out, "              5: Leaflet page\n");
	flac_fprintf(out, "              6: Media (e.g. label side of CD)\n");
	flac_fprintf(out, "              7: Lead artist/lead performer/soloist\n");
	flac_fprintf(out, "              8: Artist/performer\n");
	flac_fprintf(out, "              9: Conductor\n");
	flac_fprintf(out, "             10: Band/Orchestra\n");
	flac_fprintf(out, "             11: Composer\n");
	flac_fprintf(out, "             12: Lyricist/text writer\n");
	flac_fprintf(out, "             13: Recording Location\n");
	flac_fprintf(out, "             14: During recording\n");
	flac_fprintf(out, "             15: During performance\n");
	flac_fprintf(out, "             16: Movie/video screen capture\n");
	flac_fprintf(out, "             17: A bright coloured fish\n");
	flac_fprintf(out, "             18: Illustration\n");
	flac_fprintf(out, "             19: Band/artist logotype\n");
	flac_fprintf(out, "             20: Publisher/Studio logotype\n");
	flac_fprintf(out, "             The default is 3 (front cover).  There may only be one picture each\n");
	flac_fprintf(out, "             of type 1 and 2 in a file.\n");
	flac_fprintf(out, "           MIME-TYPE is optional; if left blank, it will be detected from the\n");
	flac_fprintf(out, "             file.  For best compatibility with players, use pictures with MIME\n");
	flac_fprintf(out, "             type image/jpeg or image/png.  The MIME type can also be --> to\n");
	flac_fprintf(out, "             mean that FILE is actually a URL to an image, though this use is\n");
	flac_fprintf(out, "             discouraged.\n");
	flac_fprintf(out, "           DESCRIPTION is optional; the default is an empty string\n");
	flac_fprintf(out, "           The next part specifies the resolution and color information.  If\n");
	flac_fprintf(out, "             the MIME-TYPE is image/jpeg, image/png, or image/gif, you can\n");
	flac_fprintf(out, "             usually leave this empty and they can be detected from the file.\n");
	flac_fprintf(out, "             Otherwise, you must specify the width in pixels, height in pixels,\n");
	flac_fprintf(out, "             and color depth in bits-per-pixel.  If the image has indexed colors\n");
	flac_fprintf(out, "             you should also specify the number of colors used.\n");
	flac_fprintf(out, "           FILE is the path to the picture file to be imported, or the URL if\n");
	flac_fprintf(out, "             MIME type is -->\n");
	flac_fprintf(out, "--export-picture-to=FILE  Export PICTURE block to a file.  Use '-' for stdout.\n");
	flac_fprintf(out, "                      Only one FLAC file may be specified.  The first PICTURE\n");
	flac_fprintf(out, "                      block will be exported unless --export-picture-to is\n");
	flac_fprintf(out, "                      preceded by a --block-number=# option to specify the exact\n");
	flac_fprintf(out, "                      metadata block to extract.  Note that the block number is\n");
	flac_fprintf(out, "                      the one shown by --list.\n");
	flac_fprintf(out, "--add-replay-gain     Calculates the title and album gains/peaks of the given\n");
	flac_fprintf(out, "                      FLAC files as if all the files were part of one album,\n");
	flac_fprintf(out, "                      then stores them in the VORBIS_COMMENT block.  The tags\n");
	flac_fprintf(out, "                      are the same as those used by vorbisgain.  Existing\n");
	flac_fprintf(out, "                      ReplayGain tags will be replaced.  If only one FLAC file\n");
	flac_fprintf(out, "                      is given, the album and title gains will be the same.\n");
	flac_fprintf(out, "                      Since this operation requires two passes, it is always\n");
	flac_fprintf(out, "                      executed last, after all other operations have been\n");
	flac_fprintf(out, "                      completed and written to disk.  All FLAC files specified\n");
	flac_fprintf(out, "                      must have the same resolution, sample rate, and number\n");
	flac_fprintf(out, "                      of channels.  Only mono and stereo files are allowed,\n");
	flac_fprintf(out, "                      and the sample rate must be 8, 11.025, 12, 16, 18.9,\n");
	flac_fprintf(out, "                      22.05, 24, 28, 32, 36, 37.8, 44.1, 48, 56, 64, 72, 75.6,\n");
	flac_fprintf(out, "                      88.2, 96, 112, 128, 144, 151.2, 176.4, 192, 224, 256,\n");
	flac_fprintf(out, "                      288, 302.4, 352.8, 384, 448, 512, 576, or 604.8 kHz.\n");
	flac_fprintf(out, "--scan-replay-gain    Like --add-replay-gain, but only analyzes the files\n");
	flac_fprintf(out, "                      rather than writing them to tags.\n");
	flac_fprintf(out, "--remove-replay-gain  Removes the ReplayGain tags.\n");
	flac_fprintf(out, "--add-seekpoint={#|X|#x|#s}  Add seek points to a SEEKTABLE block\n");
	flac_fprintf(out, "       #  : a specific sample number for a seek point\n");
	flac_fprintf(out, "       X  : a placeholder point (always goes at the end of the SEEKTABLE)\n");
	flac_fprintf(out, "       #x : # evenly spaced seekpoints, the first being at sample 0\n");
	flac_fprintf(out, "       #s : a seekpoint every # seconds; # does not have to be a whole number\n");
	flac_fprintf(out, "                      If no SEEKTABLE block exists, one will be created.  If\n");
	flac_fprintf(out, "                      one already exists, points will be added to the existing\n");
	flac_fprintf(out, "                      table, and any duplicates will be turned into placeholder\n");
	flac_fprintf(out, "                      points.  You may use many --add-seekpoint options; the\n");
	flac_fprintf(out, "                      resulting SEEKTABLE will be the unique-ified union of\n");
	flac_fprintf(out, "                      all such values.  Example: --add-seekpoint=100x\n");
	flac_fprintf(out, "                      --add-seekpoint=3.5s will add 100 evenly spaced\n");
	flac_fprintf(out, "                      seekpoints and a seekpoint every 3.5 seconds.\n");
	flac_fprintf(out, "--add-padding=length  Add a padding block of the given length (in bytes).\n");
	flac_fprintf(out, "                      The overall length of the new block will be 4 + length;\n");
	flac_fprintf(out, "                      the extra 4 bytes is for the metadata block header.\n");
	flac_fprintf(out, "\n");
	flac_fprintf(out, "Major operations:\n");
	flac_fprintf(out, "--version\n");
	flac_fprintf(out, "    Show the metaflac version number.\n");
	flac_fprintf(out, "--list\n");
	flac_fprintf(out, "    List the contents of one or more metadata blocks to stdout.  By default,\n");
	flac_fprintf(out, "    all metadata blocks are listed in text format.  Use the following options\n");
	flac_fprintf(out, "    to change this behavior:\n");
	flac_fprintf(out, "\n");
	flac_fprintf(out, "    --block-number=#[,#[...]]\n");
	flac_fprintf(out, "    An optional comma-separated list of block numbers to display.  The first\n");
	flac_fprintf(out, "    block, the STREAMINFO block, is block 0.\n");
	flac_fprintf(out, "\n");
	flac_fprintf(out, "    --block-type=type[,type[...]]\n");
	flac_fprintf(out, "    --except-block-type=type[,type[...]]\n");
	flac_fprintf(out, "    An optional comma-separated list of block types to be included or ignored\n");
	flac_fprintf(out, "    with this option.  Use only one of --block-type or --except-block-type.\n");
	flac_fprintf(out, "    The valid block types are: STREAMINFO, PADDING, APPLICATION, SEEKTABLE,\n");
	flac_fprintf(out, "    VORBIS_COMMENT.  You may narrow down the types of APPLICATION blocks\n");
	flac_fprintf(out, "    displayed as follows:\n");
	flac_fprintf(out, "        APPLICATION:abcd        The APPLICATION block(s) whose textual repre-\n");
	flac_fprintf(out, "                                sentation of the 4-byte ID is \"abcd\"\n");
	flac_fprintf(out, "        APPLICATION:0xXXXXXXXX  The APPLICATION block(s) whose hexadecimal big-\n");
	flac_fprintf(out, "                                endian representation of the 4-byte ID is\n");
	flac_fprintf(out, "                                \"0xXXXXXXXX\".  For the example \"abcd\" above the\n");
	flac_fprintf(out, "                                hexadecimal equivalalent is 0x61626364\n");
	flac_fprintf(out, "\n");
	flac_fprintf(out, "    NOTE: if both --block-number and --[except-]block-type are specified,\n");
	flac_fprintf(out, "          the result is the logical AND of both arguments.\n");
	flac_fprintf(out, "\n");
	flac_fprintf(out, "    --data-format=binary|binary-headerless|text\n");
	flac_fprintf(out, "    By default a human-readable text representation of the data is displayed.\n");
	flac_fprintf(out, "    You may specify --data-format=binary to dump the raw binary form of each\n");
	flac_fprintf(out, "    metadata block. Specify --data-format=binary-headerless to omit output of\n");
	flac_fprintf(out, "    metadata block headers, including the id of APPLICATION metadata blocks.\n");
	flac_fprintf(out, "    The output can be read in using a subsequent call to\n");
	flac_fprintf(out, "    \"metaflac --append\"\n");
	flac_fprintf(out, "\n");
	flac_fprintf(out, "    --application-data-format=hexdump|text\n");
	flac_fprintf(out, "    If the application block you are displaying contains binary data but your\n");
	flac_fprintf(out, "    --data-format=text, you can display a hex dump of the application data\n");
	flac_fprintf(out, "    contents instead using --application-data-format=hexdump\n");
	flac_fprintf(out, "\n");
	flac_fprintf(out, "--append\n");
	flac_fprintf(out, "    Insert a metadata block from a file. This must be a binary block as\n");
	flac_fprintf(out, "    exported with --list --data-format=binary. The insertion point is\n");
	flac_fprintf(out, "    defined with --block-number=#.  The new block will be added after the\n");
	flac_fprintf(out, "    given block number.  This prevents the illegal insertion of a block\n");
	flac_fprintf(out, "    before the first STREAMINFO block.  You may not --append another\n");
	flac_fprintf(out, "    STREAMINFO block. It is possible to copy a metadata block from one\n");
	flac_fprintf(out, "    file to another with this option. For example use\n");
	flac_fprintf(out, "    metaflac --list --data-format=binary --block-number=6 file.flac > block\n");
	flac_fprintf(out, "    to export the block, and then import it with\n");
	flac_fprintf(out, "    metaflac --append anotherfile.flac < block\n");
	flac_fprintf(out, "    Insert a metadata block from a file.  The input file must be in the same\n");
	flac_fprintf(out, "    format as generated with --list.\n");
	flac_fprintf(out, "\n");
	flac_fprintf(out, "    --block-number=#\n");
	flac_fprintf(out, "    Specify the insertion point (defaults to last block).  The new block will\n");
	flac_fprintf(out, "    be added after the given block number.  This prevents the illegal insertion\n");
	flac_fprintf(out, "    of a block before the first STREAMINFO block.  You may not --append another\n");
	flac_fprintf(out, "    STREAMINFO block.\n");
	flac_fprintf(out, "\n");
#if 0
	/*@@@ not implemented yet */
	flac_fprintf(out, "    --from-file=filename\n");
	flac_fprintf(out, "    Mandatory 'option' to specify the input file containing the block contents.\n");
	flac_fprintf(out, "\n");
	flac_fprintf(out, "    --data-format=binary|text\n");
	flac_fprintf(out, "    By default the block contents are assumed to be in binary format.  You can\n");
	flac_fprintf(out, "    override this by specifying --data-format=text\n");
	flac_fprintf(out, "\n");
#endif
	flac_fprintf(out, "--remove\n");
	flac_fprintf(out, "    Remove one or more metadata blocks from the metadata.  Unless\n");
	flac_fprintf(out, "    --dont-use-padding is specified, the blocks will be replaced with padding.\n");
	flac_fprintf(out, "    You may not remove the STREAMINFO block.\n");
	flac_fprintf(out, "\n");
	flac_fprintf(out, "    --block-number=#[,#[...]]\n");
	flac_fprintf(out, "    --block-type=type[,type[...]]\n");
	flac_fprintf(out, "    --except-block-type=type[,type[...]]\n");
	flac_fprintf(out, "    See --list above for usage.\n");
	flac_fprintf(out, "\n");
	flac_fprintf(out, "    NOTE: if both --block-number and --[except-]block-type are specified,\n");
	flac_fprintf(out, "          the result is the logical AND of both arguments.\n");
	flac_fprintf(out, "\n");
	flac_fprintf(out, "--remove-all\n");
	flac_fprintf(out, "    Remove all metadata blocks (except the STREAMINFO block) from the\n");
	flac_fprintf(out, "    metadata.  Unless --dont-use-padding is specified, the blocks will be\n");
	flac_fprintf(out, "    replaced with padding.\n");
	flac_fprintf(out, "\n");
	flac_fprintf(out, "--merge-padding\n");
	flac_fprintf(out, "    Merge adjacent PADDING blocks into single blocks.\n");
	flac_fprintf(out, "\n");
	flac_fprintf(out, "--sort-padding\n");
	flac_fprintf(out, "    Move all PADDING blocks to the end of the metadata and merge them into a\n");
	flac_fprintf(out, "    single block.\n");

	return message? 1 : 0;
}
