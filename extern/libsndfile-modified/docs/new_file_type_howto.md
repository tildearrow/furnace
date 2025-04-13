---
layout: page
---

# How to add new file format

   Original : Wed May 23 19:05:07 EST 2001
   Update 1 : Fri Jul 11 22:12:38 EST 2003

This document will attempt to explain as fully as possible how to add code to
libsndfile to allow the reading and writing of new file types. By new file
type I particularly mean a new header type rather than a new encoding method
for an existing file type.

This HOWTO will take the form of a step by step guide. It will assume that you
have all required tools including :

- gcc
- make (should really be the GNU version)
- autoconf
- automake
- libtool

These should all be available on the GNU ftp site: <ftp://ftp.gnu.org/pub/gnu/>.

To help make these steps clearer let's suppose we are adding support for the
Whacky file format whose files contain 'W','A','C' and 'K' as the first four
bytes of the file format. Lets also assume that Whacky files contain PCM encoded
data.

## Step 1

Create a new .c file in the src/ directory of the libsndfile source tree. The
file name should be reasonable descriptive so that is is obvious that files of
the new type are handled by this file. In this particular case the file might
be named 'whacky.c'.

## Step 2

Add your new source code file to the build process.

Edit the file src/Makefile.am and add the name of your file handler to the
FILESPECIFIC list of handlers. This list looks something like this:  

    FILESPECIFIC = aiff.c au.c au_g72x.c nist.c paf.c raw.c samplitude.c \
                    svx.c wav.c wav_float.c wav_gsm610.c wav_ima_adpcm.c \
                    wav_ms_adpcm.c

Then, run the script named 'reconf' in the libsndfile top level directory,
which will run autoconf and other associated tools. Finally run "./configure"
in the top level directory. You may want to use the "--disable-gcc-opt" option
to disable gcc optimisations and make debugging with gdb/ddd easier.

## Step 3

Add a unique identifier for the new file type.

Edit src/sndfile.h and find the enum containing the SF_FORMAT_XXX identifiers.
Since you will be adding a major file type you should add your identifier to the
top part of the list where the values are above 0x10000 in value. The easiest
way to do this is to find the largest value in the list, add 0x10000 to it and
make that your new identifier value. The identifier should be something like
SF_FORMAT_WACK.

## Step 4

Add code to the file type recogniser function.

Edit src/sndfile.c and find the function guess_file_type (). This function
reads the first 3 ints of the file and from that makes a guess at the file
type. In our case we would add:

    if (buffer [0] == MAKE_MARKER ('W','A','C','K'))
        return SF_FORMAT_WACK ;

The use of the MAKE_MARKER macro should be pretty obvious and it is defined at
the top of file should you need to have a look at it.

## Step 5

Add a call to your open function from psf_open_file ().

Edit src/sndfile.c and find the switch statement in psf_open_file (). It starts
like this:

    switch (filetype)
    {   case SF_FORMAT_WAV :
                error = wav_open (psf) ;
                break ;

        case SF_FORMAT_AIFF :
                error = aiff_open (psf) ;
                break ;

Towards the bottom of this switch statement your should add one for the new file
type. Something like:

        case    SF_FORMAT_WACK :
                sf_errno = whacky_open (psf) ;
                break ;

## Step 6

Add prototypes for new open read and open write functions.

Edit src/common.h, go to the bottom of the file and add something like

    int     whacky_open   (SF_PRIVATE *psf) ;

## Step 7

Implement your open read function. The best way to do this is by coding
something much like one of the other file formats. The file src/au.c might be a
good place to start.

In src/whacky.c you should now implement the function whacky_open() which
was prototyped in src/common.h. This function should return 0 on success and
a non-zero number on error.

Error values are defined in src/common.h in a enum which starts at SFE_NO_ERROR.
When adding a new error value, you also need to add an error string to the
SndfileErrors array in src/sndfile.c.

To parse the header of your new file type you should avoid using standard read/
write/seek functions (and the fread/fwrite/fseek etc) and instead use
psf_binheader_readf () which is implemented and documented in src/common.h.

During the parsing process, you should also print logging information to
libsndfile's internal log buffer using the psf_log_printf() function.

At the end of the open read process, you should have set a number of fields in
the SF_PRIVATE structure pointed to by psf.

**THIS FILE IS INCOMPLETE**
