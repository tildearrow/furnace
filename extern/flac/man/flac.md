% flac(1) Version 1.5.0 | Free Lossless Audio Codec conversion tool

# NAME

flac - Free Lossless Audio Codec

# SYNOPSIS

**flac** \[ *OPTIONS* \] \[ *infile.wav* \| *infile.rf64* \|
*infile.aiff* \| *infile.raw* \| *infile.flac* \| *infile.oga* \|
*infile.ogg* \| **-** *...* \]

**flac** \[ **-d** \| **\--decode** \| **-t** \| **\--test** \| **-a** \|
**\--analyze** \] \[ *OPTIONS* \] \[ *infile.flac* \| *infile.oga* \|
*infile.ogg* \| **-** *...* \]

# DESCRIPTION

**flac** is a command-line tool for encoding, decoding, testing and
analyzing FLAC streams.

# GENERAL USAGE

**flac** supports as input RIFF WAVE, Wave64, RF64, AIFF, FLAC or Ogg
FLAC format, or raw interleaved samples. The decoder currently can output
to RIFF WAVE, Wave64, RF64, or AIFF format, or raw interleaved samples.
flac only supports linear PCM samples (in other words, no A-LAW, uLAW,
etc.), and the input must be between 4 and 32 bits per sample.

flac assumes that files ending in ".wav" or that have the RIFF WAVE
header present are WAVE files, files ending in ".w64" or have the Wave64
header present are Wave64 files, files ending in ".rf64" or have the
RF64 header present are RF64 files, files ending in ".aif" or ".aiff" or
have the AIFF header present are AIFF files, files ending in ".flac"
or have the FLAC header present are FLAC files and files ending in ".oga"
or ".ogg" or have the Ogg FLAC header present are Ogg FLAC files.

Other than this, flac makes no assumptions about file extensions, though
the convention is that FLAC files have the extension ".flac"
(or ".fla" on ancient "8.3" file systems like FAT-16).

Before going into the full command-line description, a few other things
help to sort it out:

1.	flac encodes by default, so you must use -d to decode
2.	Encoding options -0 .. -8 (or \--fast and \--best) that control the
	compression level actually are just synonyms for different groups of
	specific encoding options (described later).  
3.	The order in which options are specified is generally not important 
	except when they contradict each other, then the latter takes 
	precedence except that compression presets are overridden by any
	option given before or after. For example, -0M, -M0, -M2 and -2M are 
	all the same as -1, and -l 12 -6 the same as -7.
4.	flac behaves similarly to gzip in the way it handles input and output 
	files

Skip to the EXAMPLES section below for examples of some typical tasks.

flac will be invoked one of four ways, depending on whether you are
encoding, decoding, testing, or analyzing. Encoding is the default
invocation, but can be switch to decoding with **-d**, analysis with
**-a** or testing with **-t**. Depending on which way is chosen,
encoding, decoding, analysis or testing options can be used, see section
OPTIONS for details. General options can be used for all.

If only one inputfile is specified, it may be "-" for stdin. When stdin
is used as input, flac will write to stdout. Otherwise flac will perform
the desired operation on each input file to similarly named output files
(meaning for encoding, the extension will be replaced with ".flac", or
appended with ".flac" if the input file has no extension, and for
decoding, the extension will be ".wav" for WAVE output and ".raw" for raw
output). The original file is not deleted unless \--delete-input-file is
specified.

If you are encoding/decoding from stdin to a file, you should use the -o
option like so:

    flac [options] -o outputfile
    flac -d [options] -o outputfile

which are better than:

    flac [options] > outputfile
    flac -d [options] > outputfile

since the former allows flac to seek backwards to write the STREAMINFO or
RIFF WAVE header contents when necessary.

Also, you can force output data to go to stdout using -c.

To encode or decode files that start with a dash, use \-- to signal the
end of options, to keep the filenames themselves from being treated as
options:

    flac -V -- -01-filename.wav

The encoding options affect the compression ratio and encoding speed. The
format options are used to tell flac the arrangement of samples if the
input file (or output file when decoding) is a raw file. If it is a RIFF
WAVE, Wave64, RF64, or AIFF file the format options are not needed since
they are read from the file's header.

In test mode, flac acts just like in decode mode, except no output file
is written. Both decode and test modes detect errors in the stream, but
they also detect when the MD5 signature of the decoded audio does not
match the stored MD5 signature, even when the bitstream is valid.

flac can also re-encode FLAC files. In other words, you can specify a
FLAC or Ogg FLAC file as an input to the encoder and it will decoder it
and re-encode it according to the options you specify. It will also
preserve all the metadata unless you override it with other options (e.g.
specifying new tags, seekpoints, cuesheet, padding, etc.).

flac has been tuned so that the default settings yield a good speed vs.
compression tradeoff for many kinds of input. However, if you are looking
to maximize the compression rate or speed, or want to use the full power
of FLAC's metadata system, see the page titled 'About the FLAC Format' on
the FLAC website.

# EXAMPLES

Some typical encoding and decoding tasks using flac:

## Encoding examples

`flac abc.wav`
:	Encode abc.wav to abc.flac using the default compression setting. abc.wav is not deleted.

`flac --delete-input-file abc.wav`
:	Like above, except abc.wav is deleted if there were no errors.

`flac --delete-input-file -w abc.wav`
:	Like above, except abc.wav is deleted if there were no errors and no warnings.

`flac --best abc.wav` or `flac -8 abc.wav`
:	Encode abc.wav to abc.flac using the highest compression preset. 

`flac --verify abc.wav` or `flac -V abc.wav`
:	Encode abc.wav to abc.flac and internally decode abc.flac to make sure it matches abc.wav.

`flac -o my.flac abc.wav`
:	Encode abc.wav to my.flac.

`flac abc.aiff foo.rf64 bar.w64`
:	Encode abc.aiff to abc.flac, foo.rf64 to foo.flac and bar.w64 to bar.flac

`flac *.wav *.aif?`
:	Wildcards are supported. This command will encode all .wav files and all 
	.aif/.aiff/.aifc files (as well as other supported files ending in 
 	.aif+one character) in the current directory.

`flac abc.flac --force` or `flac abc.flac -f`
:	Recompresses, keeping metadata like tags. The syntax is a little 
	tricky: this is an *encoding* command (which is the default: you need 
	to specify -d for decoded output), and will thus want to output the 
	file abc.flac - which already exists. flac will require the \--force 
	or shortform -f option to overwrite an existing file. Recompression 
	will first write a temporary file, which afterwards replaces the old 
	abc.flac (provided flac has write access to that file).
	The above example uses default settings. More often, recompression is
	combined with a different - usually higher - compression option.
 	Note: If the FLAC file does not end with .flac - say, it is abc.fla
	- the -f is not needed: A new abc.flac will be created and the old 
	kept, just like for an uncompressed input file.

`flac --tag-from-file="ALBUM=albumtitle.txt" -T "ARTIST=Queen" *.wav`
:	Encode every .wav file in the directory and add some tags. Every 
	file will get the same set of tags.
	Warning: Will wipe all existing tags, when the input file is (Ogg) 
	FLAC - not just those tags listed in the option. Use the metaflac 
	utility to tag FLAC files.

`flac --keep-foreign-metadata-if-present abc.wav`
:	FLAC files can store non-audio chunks of input WAVE/AIFF/RF64/W64
	files. The related option \--keep-foreign-metadata works the same
	way, but will instead exit with an error if the input has no such 
	non-audio chunks.
	The encoder only stores the chunks as they are, it cannot import 
	the content into its own tags (vorbis comments). To transfer such
	tags from a source file, use tagging software which supports them.

`flac -Vj2 -m3fo Track07.flac  -- -7.wav`
:	flac employs the commonplace convention that options in a short 
	version - invoked with single dash - can be shortened together until 
	one that takes an argument. Here -j and -o do, and after the "2" a 
	whitespace is needed to start new options with single/double dash. 
	The -m option does not, and the following "3" is the -3 compression
	setting. The options could equally well have been written out as 
	-V -j 2 -m -3 -f -o Track04.flac , or as -fo Track04.flac -3mVj2. 
	flac also employs the convention that `-- ` (with whitespace!) 
	signifies end of options, treating everything to follow as filename.
	That is needed when an input filenames could otherwise be read as an
	option, and "-7" is one such.
	In total, this line takes the input file -7.wav as input; -o will 
	give output filename as Track07.flac, and the -f will overwrite if 
	the file Track04.flac is already present. The encoder will select 
	encoding preset -3 modified with the -m switch, and use two CPU 
	threads. Afterwards, the -V will make it decode the flac file and 
	compare the audio to the input, to ensure they are indeed equal. 


## Decoding examples

`flac --decode abc.flac` or `flac -d abc.flac`
:	Decode abc.flac to abc.wav. abc.flac is not deleted. If abc.wav is
	already present, the process will exit with an error instead of 
	overwriting; use --force / -f to force overwrite.
	NOTE: A mere flac abc.flac *without --decode or its shortform -d*, 
	would mean to re-encode abc.flac to abc.flac (see above), and that 
	command would err out because abc.flac already exists.

`flac -d --force-aiff-format abc.flac` or `flac -d -o abc.aiff abc.flac`
:	Two different ways of decoding abc.flac to abc.aiff (AIFF format).
	abc.flac is not deleted. -d -o could be shortened to -do.
 	The decoder can force other output formats, or different versions 
	of the WAVE/AIFF formats, see the options below.

`flac -d --keep-foreign-metadata-if-present abc.flac`
:	If the FLAC file has non-audio chunks stored from the original
	input file, this option will restore both audio and non-audio. 
	The chunks will reveal the original file type, and the decoder 
	will select output format and output file extension accordingly 
	- note that this is not compatible with forcing a particular 
	output format except if it coincides with the original, as the
	decoder cannot transcode non-audio between formats.
	If there are no such chunks stored, it will decode to abc.wav.
	The related option \--keep-foreign-metadata will instead exit 
	with an error if no such non-audio chunks are found.

`flac -d -F abc.flac`
:	Decode abc.flac to abc.wav and don't abort if errors are found.
	This is potentially useful for recovering as much as possible from 
	a corrupted file.
	Note: Be careful about trying to "repair" files this way. Often it
	will only conceal an error, and not play any subjectively "better"
	than the corrupted file. It is a good idea to at least keep it,
	and possibly try several decoders, including the one that generated 
	the file, and hear if one has less detrimental audible errors than 
	another. Make sure output volume is limited, as corrupted audio can
	generate loud noises.


# OPTIONS

A summary of options is included below. Several of the options can be 
negated, see the **Negative options** section below. 


## GENERAL OPTIONS

**-v**, **\--version**
:	Show the flac version number, and quit.

**-h**, **\--help**
:	Show basic usage and a list of all options, and quit.

**-d**, **\--decode**
:	Decode (the default behavior is to encode)

**-t**, **\--test**
:	Test a flac encoded file. This works the same as -d except no
	decoded file is written, and with some additional checks like parsing
	of all metadata blocks.

**-a**, **\--analyze**
:	Analyze a FLAC encoded file. This works the same as -d except the 
	output is an analysis file, not a decoded file.

**-c**, **\--stdout**
:	Write output to stdout

**-f**, **\--force**
:	Force overwriting of output files. By default, flac warns that the
	output file already exists and continues to the next file.

**\--delete-input-file**
:	Automatically delete the input file after a successful encode or
	decode. If there was an error (including a verify error) the input
	file is left intact.

**-o** *FILENAME*, **\--output-name**=*FILENAME*
:	Force the output file name (usually flac just changes the extension).
	May only be used when encoding a single file. May not be used in
	conjunction with \--output-prefix.

**\--output-prefix**=*STRING*
:	Prefix each output file name with the given string. This can be
	useful for encoding or decoding files to a different directory. Make
	sure if your string is a path name that it ends with a trailing \`/'
	(slash).

**\--preserve-modtime**
:	(Enabled by default.) Output files have their timestamps/permissions 
	set to match those of their inputs. Use \--no-preserve-modtime to make
	output files have the current time and default permissions.

**\--keep-foreign-metadata**
:	If encoding, save WAVE, RF64, or AIFF non-audio chunks in FLAC
	metadata. If decoding, restore any saved non-audio chunks from FLAC
	metadata when writing the decoded file. Foreign metadata cannot be
	transcoded, e.g. WAVE chunks saved in a FLAC file cannot be restored
	when decoding to AIFF. Input and output must be regular files (not
	stdin or stdout). With this option, FLAC will pick the right output
	format on decoding. It will exit with error if no such chunks are found.

**\--keep-foreign-metadata-if-present**
:	Like \--keep-foreign-metadata, but without throwing an error if
	foreign metadata cannot be found or restored. Instead, prints a 
	warning.

**\--skip**={\#\|*MM:SS*}
:	Skip the first number of samples of the input. To skip over a given
	initial time, specify instead minutes and seconds: there must then 
	be at least one digit on each side of the colon sign. Fractions of a 
	second can be specified, with locale-dependent decimal point, e.g.
	\--skip=123:9,867 if your decimal point is a comma. 
	A \--skip option is applied to each input file if more are given. 
	This option cannot be used with -t. When used with -a, the analysis
	file will enumerate frames from starting point.
	
**\--until**={\#\|\[+\|\]*MM:SS*}
:	Stop at the given sample number (which is not included). A negative
	number is taken relative to the end of the audio, a \`+' (plus) 
	sign means that the \--until point is taken relative to the \--skip 
	point. For other considerations, see \--skip. 

**\--no-utf8-convert**
:	Do not convert tags from local charset to UTF-8. This is useful for
	scripts, and setting tags in situations where the locale is wrong.
	This option must appear before any tag options!

**-s**, **\--silent**
:	Silent mode (do not write runtime encode/decode statistics to stderr)

**\--totally-silent**
: Do not print anything of any kind, including warnings or errors. The
	exit code will be the only way to determine successful completion.

**-w**, **\--warnings-as-errors**
:	Treat all warnings as errors (which cause flac to terminate with a
	non-zero exit code).


## DECODING OPTIONS

**-F**, **\--decode-through-errors**
:	By default flac stops decoding with an error message and removes the
	partially decoded file if it encounters a bitstream error. With -F,
	errors are still printed but flac will continue decoding to
	completion. Note that errors may cause the decoded audio to be
	missing some samples or have silent sections.

**\--cue**=\[\#.#\]\[-\[\#.#\]\]
:	Set the beginning and ending cuepoints to decode. Decimal points are
	locale-dependent (dot or comma). The optional first \#.# is the track
	and index point at which decoding will start; the default is the
	beginning of the stream. The optional second \#.# is the track and
	index point at which decoding will end; the default is the end of
	the stream. If the cuepoint does not exist, the closest one before
	it (for the start point) or after it (for the end point) will be
	used. If those don't exist , the start of the stream (for the start
	point) or end of the stream (for the end point) will be used. The
	cuepoints are merely translated into sample numbers then used as
	\--skip and \--until. A CD track can always be cued by, for example,
	\--cue=9.1-10.1 for track 9, even if the CD has no 10th track.

**--decode-chained-stream**
: Decode all links in a chained Ogg stream, not just the first one.

**\--apply-replaygain-which-is-not-lossless**\[=*SPECIFICATION*\]
:	Applies ReplayGain values while decoding. **WARNING: THIS IS NOT
	LOSSLESS. DECODED AUDIO WILL NOT BE IDENTICAL TO THE ORIGINAL WITH
	THIS OPTION.** This option is useful for example in transcoding
	media servers, where the client does not support ReplayGain. For
	details on the use of this option, see the section **ReplayGain
	application specification**.


## ENCODING OPTIONS

Encoding will default to -5, -A "tukey(5e-1)" and one CPU thread.

**-V**, **\--verify**
:	Verify a correct encoding by decoding the output in parallel and
	comparing to the original.

**-0**, **\--compression-level-0**, **\--fast**
:	Fastest compression preset. Currently synonymous with `-l 0 -b 1152 -r 3 --no-mid-side`

**-1**, **\--compression-level-1**
:	Currently synonymous with `-l 0 -b 1152 -M -r 3`

**-2**, **\--compression-level-2**
:	Currently synonymous with `-l 0 -b 1152 -m -r 3`

**-3**, **\--compression-level-3**
:	Currently synonymous with `-l 6 -b 4096 -r 4 --no-mid-side`

**-4**, **\--compression-level-4**
:	Currently synonymous with `-l 8 -b 4096 -M -r 4`

**-5**, **\--compression-level-5**
:	Currently synonymous with `-l 8 -b 4096 -m -r 5`

**-6**, **\--compression-level-6**
:	Currently synonymous with `-l 8 -b 4096 -m -r 6 -A "subdivide_tukey(2)"`

**-7**, **\--compression-level-7**
:	Currently synonymous with `-l 12 -b 4096 -m -r 6 -A "subdivide_tukey(2)"`

**-8**, **\--compression-level-8**, **\--best**
:	Currently synonymous with `-l 12 -b 4096 -m -r 6 -A "subdivide_tukey(3)"`

**-l** \#, **\--max-lpc-order**=\#
:	Specifies the maximum LPC order. This number must be \<= 32. 
	For subset streams, it must be \<=12 if the sample rate is \<=48kHz. 
	If 0, the encoder will not attempt generic linear prediction, and 
	only choose among a set of fixed (hard-coded) predictors. Restricting 
	to fixed predictors only is faster, but compresses weaker - typically 
	five percentage points / ten percent larger files.

**-b** \#, **\--blocksize**=\#
:	Specify the blocksize in samples. The current default is 1152 for 
	-l 0, else 4096. Blocksize must be between 16 and 65535 (inclusive). 
 	For subset streams it must be \<= 4608 if the samplerate is \<= 48kHz,
	for subset streams with higher samplerates it must be \<= 16384.

**-m**, **\--mid-side**
:	Try mid-side coding for each frame (stereo only, otherwise ignored).

**-M**, **\--adaptive-mid-side**
:	Adaptive mid-side coding for all frames (stereo only, otherwise ignored).

**-r** \[\#,\]\#, **\--rice-partition-order**=\[\#,\]\#
:	Set the \[min,\]max residual partition order (0..15). For subset 
	streams, max must be \<=8. min defaults to 0. Default is -r 5.
	Actual partitioning will be restricted by block size and prediction 
	order, and the encoder will silently reduce too high values. 

**-A** *FUNCTION(S)*, **\--apodization**=*FUNCTION(S)*
:	Window audio data with given apodization function. More can be given, 
	comma-separated. See section **Apodization functions** for details.

**-e**, **\--exhaustive-model-search**
:	Do exhaustive model search (expensive!).

**-q** \#, **\--qlp-coeff-precision**=\#
:	Precision of the quantized linear-predictor coefficients. This number 
	must be in between 5 and 16, or 0 (the default) to let encoder decide. 
 	Does nothing if using -l 0. 

**-p**, **\--qlp-coeff-precision-search**
:	Do exhaustive search of LP coefficient quantization (expensive!).
	Overrides -q; does nothing if using -l 0.

**\--lax**
:	Allow encoder to generate non-Subset files. The resulting FLAC file
	may not be streamable or might have trouble being played in all
	players (especially hardware devices), so you should only use this
	option in combination with custom encoding options meant for
	archival.

**\--limit-min-bitrate**
:	Limit minimum bitrate by not allowing frames consisting of only 
	constant subframes. This ensures a bitrate of at least 1 bit/sample, 
	for example 48kbit/s for 48kHz input. This is mainly useful for 
	internet streaming.

**-j** \#, **\--threads**=\#
:	Try to set a maximum number of threads to use for encoding. If
	multithreading was not enabled on compilation or when setting a
	number of threads that is too high, this fails with a warning. The
	value of 0 means a default set by the encoder; currently that is 1 
 	thread (i.e. no multithreading), but that could change in the 
	future. Currently, up to 128 threads are supported. Using a value 
 	higher than the number of available CPU threads harms performance.

**\--ignore-chunk-sizes**
:	When encoding to flac, ignore the file size headers in WAV and AIFF
	files to attempt to work around problems with over-sized or malformed
	files. WAV and AIFF files both specifies length of audio data with
 	an unsigned 32-bit number, limiting audio to just over 4 gigabytes. 
	Files larger than this are malformed, but should be read correctly 
 	using this option. Beware however, it could misinterpret any data 
	following the audio chunk, as audio.

**\--replay-gain**
:	Calculate ReplayGain values and store them as FLAC tags, similar to
	vorbisgain. Title gains/peaks will be computed for each input file,
	and an album gain/peak will be computed for all files. All input
	files must have the same resolution, sample rate, and number of
	channels. Only mono and stereo files are allowed, and the sample
	rate must be 8, 11.025, 12, 16, 18.9, 22.05, 24, 28, 32, 36, 37.8,
	44.1, 48, 56, 64, 72, 75.6, 88.2, 96, 112, 128, 144, 151.2, 176.4,
	192, 224, 256, 288, 302.4, 352.8, 384, 448, 512, 576, or 604.8 kHz.
	Also note that this option may leave a few extra bytes in a PADDING
	block as the exact size of the tags is not known until all files
	are processed. Note that this option cannot be used when encoding
	to standard output (stdout).

**\--cuesheet**=*FILENAME*
:	Import the given cuesheet file and store it in a CUESHEET metadata
	block. This option may only be used when encoding a single file. A
	seekpoint will be added for each index point in the cuesheet to the
	SEEKTABLE unless \--no-cued-seekpoints is specified.

**\--picture**={*FILENAME\|SPECIFICATION*}
:	Import a picture and store it in a PICTURE metadata block. More than
	one \--picture option can be specified. Either a filename for the
	picture file or a more complete specification form can be used. The
	*SPECIFICATION* is a string whose parts are separated by \| (pipe)
	characters. Some parts may be left empty to invoke default values.
	Specifying only *FILENAME* is just shorthand for "\|\|\|\|FILENAME". 
	See the section **Picture specification** for *SPECIFICATION* format.

**-S** {\#\|X\|\#x\|\#s}, **\--seekpoint**={\#\|X\|\#x\|\#s}
:	Specifies point(s) to include in SEEKTABLE, to override the encoder's
	default choice of one per ten seconds ('-s 10s'). Using \#, a seek point 
	at that sample number is added. Using X, a placeholder point is added 
	at the end of a the table. Using \#x, \# evenly spaced seek points will
	be added, the first being at sample 0. Using \#s, a seekpoint will be
	added every \# seconds, where decimal points are locale-dependent, e.g. 
	'-s 9.5s' or '-s 9,5s'. 
	Several -S options may be given; the resulting SEEKTABLE will contain 
	all seekpoints specified (duplicates removed).
	Note: '-S \#x' and '-S \#s' will not work if the encoder cannot 
	determine the input size before starting. Note: if you use '-S \#' with 
	\# being \>= the number of samples in the input, there will be either no 
	seek point entered (if the input size is determinable before encoding 
	starts) or a placeholder point (if input size is not determinable).
	Use \--no-seektable for no SEEKTABLE. 

**-P** \#, **\--padding**=\#
:	(Default: 8192 bytes, although 65536 for input above 20 minutes.) 
	Tell the encoder to write a PADDING metadata block of the given
	length (in bytes) after the STREAMINFO block. This is useful for 
 	later tagging, where one can write over the PADDING block instead 
	of having to rewrite the entire file. Note that a block header 
	of 4 bytes will come on top of the length specified.

**-T** "*FIELD=VALUE*"**, \--tag**="*FIELD=VALUE*"
:	Add a FLAC tag. The comment must adhere to the Vorbis comment spec;
	i.e. the FIELD must contain only legal characters, terminated by an
	'equals' sign. Make sure to quote the content if necessary. This
	option may appear more than once to add several Vorbis comments. 
 	NOTE: all tags will be added to all encoded files.

**\--tag-from-file**="*FIELD=FILENAME*"
:	Like \--tag, except FILENAME is a file whose contents will be read
	verbatim to set the tag value. The contents will be converted to
	UTF-8 from the local charset. This can be used to store a cuesheet
	in a tag (e.g. \--tag-from-file="CUESHEET=image.cue"). Do not try to
	store binary data in tag fields! Use APPLICATION blocks for that.


## FORMAT OPTIONS

Encoding defaults to FLAC and not OGG. Decoding defaults to WAVE (more
specifically WAVE\_FORMAT\_PCM for mono/stereo with 8/16 bits, and to 
WAVE\_FORMAT\_EXTENSIBLE otherwise), except: will be overridden by chunks 
found by \--keep-foreign-metadata-if-present or \--keep-foreign-metadata 

**\--ogg**
:	When encoding, generate Ogg FLAC output instead of native FLAC. Ogg
	FLAC streams are FLAC streams wrapped in an Ogg transport layer. The
	resulting file should have an '.oga' extension and will still be
	decodable by flac. When decoding, force the input to be treated as
	Ogg FLAC. This is useful when piping input from stdin or when the
	filename does not end in '.oga' or '.ogg'.

**\--serial-number**=\#
:	When used with \--ogg, specifies the serial number to use for the
	first Ogg FLAC stream, which is then incremented for each additional
	stream. When encoding and no serial number is given, flac uses a
	random number for the first stream, then increments it for each
	additional stream. When decoding and no number is given, flac uses
	the serial number of the first page.

**\--force-aiff-format**  
**\--force-rf64-format**  
**\--force-wave64-format**
:	For decoding: Override default output format and force output to 
	AIFF/RF64/WAVE64, respectively.
	This option is not needed if the output filename (as set by -o) 
	ends with *.aif* or *.aiff*, *.rf64* and *.w64* respectively. 
 	The encoder auto-detects format and ignores this option. 

**\--force-legacy-wave-format**  
**\--force-extensible-wave-format**
:	Instruct the decoder to output a WAVE file with WAVE\_FORMAT\_PCM and
	WAVE\_FORMAT\_EXTENSIBLE respectively, overriding default choice.

**\--force-aiff-c-none-format**  
**\--force-aiff-c-sowt-format**
:	Instruct the decoder to output an AIFF-C file with format NONE and
	sowt respectively.

**\--force-raw-format**
:	Force input (when encoding) or output (when decoding) to be treated
	as raw samples (even if filename suggests otherwise).

### raw format options

When encoding from or decoding to raw PCM, format must be specified.

**\--sign**={signed\|unsigned}
:	Specify the sign of samples.

**\--endian**={big\|little}
:	Specify the byte order for samples

**\--channels**=\#
:	(Input only) specify number of channels. The channels must be 
	interleaved, and in the order of the FLAC format (see the format
	specification); the encoder (/decoder) cannot re-order channels.

**\--bps**=\#
:	(Input only) specify bits per sample (per channel: 16 for CDDA.)

**\--sample-rate**=\#
:	(Input only) specify sample rate (in Hz. Only integers supported.)

**\--input-size**=\#
:	(Input only) specify the size of the raw input in bytes. This option
	is only compulsory when encoding from stdin and using options that need
 	to know the input size beforehand (like, \--skip, \--until, \--cuesheet )
	The encoder will truncate at the specified size if the input stream is
 	bigger. If the input stream is smaller, it will complain about an 
	unexpected end-of-file. 

## ANALYSIS OPTIONS

**\--residual-text**
:	Includes the residual signal in the analysis file. This will make the
	file very big, much larger than even the decoded file.

**\--residual-gnuplot**
:	Generates a gnuplot file for every subframe; each file will contain
	the residual distribution of the subframe. This will create a lot of
	files. gnuplot must be installed separately. 

## NEGATIVE OPTIONS

The following will negate an option previously given:

**\--no-adaptive-mid-side**  
**\--no-cued-seekpoints**  
**\--no-decode-through-errors**  
**\--no-delete-input-file**  
**\--no-preserve-modtime**  
**\--no-keep-foreign-metadata**  
**\--no-exhaustive-model-search**  
**\--no-force**  
**\--no-lax**  
**\--no-mid-side**  
**\--no-ogg**  
**\--no-padding**  
**\--no-qlp-coeff-prec-search**  
**\--no-replay-gain**  
**\--no-residual-gnuplot**  
**\--no-residual-text**  
**\--no-seektable**  
**\--no-silent**  
**\--no-verify**  
**\--no-warnings-as-errors**

## ReplayGain application specification
The option \--apply-replaygain-which-is-not-lossless\[=\<specification\>\]
applies ReplayGain values while decoding. **WARNING: THIS IS NOT
LOSSLESS. DECODED AUDIO WILL NOT BE IDENTICAL TO THE ORIGINAL WITH THIS
OPTION.** This option is useful for example in transcoding media servers,
where the client does not support ReplayGain.
	
The \<specification\> is a shorthand notation for describing how to	apply
ReplayGain. All elements are optional - defaulting to 0aLn1 - but order 
is important.  The format is: 
	
\[\<preamp\>\]\[a\|t\]\[l\|L\]\[n{0\|1\|2\|3}\]

In which the following parameters are used:

-	**preamp**: A floating point number in dB. This is added to the 
	existing gain value.

-	**a\|t**: Specify 'a' to use the album gain, or 't' to use the track
	gain. If tags for the preferred kind (album/track) do not exist but
	tags for the other (track/album) do, those will be used instead.  

-	**l\|L**: Specify 'l' to peak-limit the output, so that the 
	ReplayGain peak value is full-scale. Specify 'L' to use a 6dB hard
	limiter that kicks in when the signal approaches full-scale.

-	**n{0\|1\|2\|3}**: Specify the amount of noise shaping. ReplayGain
	synthesis happens in floating point; the result is dithered before
	converting back to integer. This quantization adds noise. Noise
	shaping tries to move the noise where you won't hear it as much.
	0 means no noise shaping, 1 means 'low', 2 means 'medium', 3 means
	'high'.

For example, the default of 0aLn1 means 0dB preamp, use album gain, 6dB
hard limit, low noise shaping. \--apply-replaygain-which-is-not-lossless=3
means 3dB preamp, use album gain, no limiting, no noise shaping.

flac uses the ReplayGain tags for the calculation. If a stream does
not have the required tags or they can't be parsed, decoding will
continue with a warning, and no ReplayGain is applied to that stream.

## Picture specification
This described the specification used for the **\--picture** option.
\[*TYPE*\]\|\[*MIME-TYPE*\]\|\[*DESCRIPTION*\]\|\[*WIDTHxHEIGHTxDEPTH*\[/*COLORS*\]\]\|*FILE*

*TYPE* is optional; it is a number from one of:

0. Other
1. 32x32 pixels 'file icon' (PNG only)
2. Other file icon
3. Cover (front)
4. Cover (back)
5. Leaflet page
6. Media (e.g. label side of CD)
7. Lead artist/lead performer/soloist
8. Artist/performer
9. Conductor
10. Band/Orchestra
11. Composer
12. Lyricist/text writer
13. Recording Location
14. During recording
15. During performance
16. Movie/video screen capture
17. A bright coloured fish
18. Illustration
19. Band/artist logotype
20. Publisher/Studio logotype

The default is 3 (front cover). There may only be one picture each of
type 1 and 2 in a file.

*MIME-TYPE* is optional; if left blank, it will be detected from the file.
For best compatibility with players, use pictures with MIME type
image/jpeg or image/png. The MIME type can also be \--\> to mean that
FILE is actually a URL to an image, though this use is discouraged.

*DESCRIPTION* is optional; the default is an empty string.

The next part specifies the resolution and color information. If the
*MIME-TYPE* is image/jpeg, image/png, or image/gif, you can usually leave
this empty and they can be detected from the file. Otherwise, you must
specify the width in pixels, height in pixels, and color depth in
bits-per-pixel. If the image has indexed colors you should also specify
the number of colors used. When manually specified, it is not checked
against the file for accuracy.

*FILE* is the path to the picture file to be imported, or the URL if MIME
type is \--\>

**Specification examples:** 
"\|image/jpeg\|\|\|../cover.jpg" will embed the 
JPEG file at ../cover.jpg, defaulting to type 3 (front cover) and an 
empty description. The resolution and color info will be retrieved 
from the file itself. 
"4\|\--\>\|CD\|320x300x24/173\|http://blah.blah/backcover.tiff" will
embed the given URL, with type 4 (back cover), description "CD", and a
manually specified resolution of 320x300, 24 bits-per-pixel, and 173
colors. The file at the URL will not be fetched; the URL itself is
stored in the PICTURE metadata block.

## Apodization functions
To improve LPC analysis, the audio data is windowed. An **-A** option 
applies the specified apodization function(s) instead of the default 
(which is "tukey(5e-1)", though different for presets -6 to -8.)
Specifying one more function effectively means, for each subframe, to 
try another weighting of the data and see if it happens to result in a 
smaller encoded subframe. Specifying several functions is time-expensive, 
at typically diminishing compression gains. 

The subdivide_tukey(*N*) functions (see below) used in presets -6 to -8 
were developed to recycle calculations for speed, compared to using a 
number of independent functions. Even then, a high number like *N*\>4 
or 5, will often become less efficient than other options considered 
expensive, like the slower -p, though results vary with signal.

Up to 32 functions can be given as comma-separated list and/or individual 
**-A** options. Any mis-specified function is silently ignored. Quoting 
a function which takes options (and has parentheses) may be necessary, 
depending on shell. Currently the following functions are implemented: 
bartlett, bartlett_hann, blackman, blackman_harris_4term_92db, connes, 
flattop, gauss(*STDDEV*), hamming, hann, kaiser_bessel, nuttall, 
rectangle, triangle, tukey(*P*), partial_tukey(*N*\[/*OV*\[/*P*\]\]), 
punchout_tukey(*N*\[/*OV*\[/*P*\]\]), subdivide_tukey(*N*\[/*P*\]), welch.

For parameters *P*, *STDDEV* and *OV*, scientific notation is supported, e.g. 
tukey(5e-1). Otherwise, the decimal point must agree with the locale, 
e.g. tukey(0.5) or tukey(0,5) depending on your system.

- For gauss(*STDDEV*), *STDDEV* is the standard deviation (0\<*STDDEV*\<=5e-1).

- For tukey(*P*), *P* (between 0 and 1) specifies the fraction of the window 
that is cosine-tapered; *P*=0 corresponds to "rectangle" and *P*=1 to "hann". 

- partial_tukey(*N*) and punchout_tukey(*N*) are largely obsoleted by the 
more time-effective subdivide_tukey(*N*), see next item. They generate *N* 
functions each spanning a part of each block. Optional arguments are an 
overlap *OV* (\<1, may be negative), for example partial_tukey(2/2e-1); 
and then a taper parameter *P*, for example partial_tukey(2/2e-1/5e-1).

- subdivide_tukey(*N*) is a more efficient reimplementation of partial_tukey 
and punchout_tukey taken together, combining the windows they would 
generate up to the specified *N*. Specifying subdivide_tukey(3) entails a 
tukey, a partial_tukey(2), a partial_tukey(3) and a punchout_tukey(3); 
specifying subdivide_tukey(5) will on top of that add a partial_tukey(4), 
a punchout_tukey(4), a partial_tukey(5) and a punchout_tukey(5) - but all 
with tapering chosen to facilitate the re-use of computation. Thus the *P* 
parameter (defaulting to 5e-1) is applied for the smallest used window:
For example, subdivide_tukey(2/5e-1) results in the same taper as that of
tukey(25e-2) and subdivide_tukey(5) in the same taper as of tukey(1e-1). 

# SEE ALSO

**metaflac(1)**

# AUTHOR

This manual page was initially written by Matt Zimmerman
\<mdz@debian.org\> for the Debian GNU/Linux system (but may be used by
others). It has been kept up-to-date by the Xiph.org Foundation.
