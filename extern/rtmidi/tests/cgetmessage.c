/* cgetmessage.c
 *
 * Checks rtmidi_in_get_message() with a buffer that is big enough and with one
 * that is too small for the message (#272). It needs MIDI going round a loop:
 *
 *   - Where the API has virtual ports (ALSA, JACK, CoreMIDI), it makes its own:
 *     an input virtual port, and an output connected to it.
 *   - Elsewhere, set RTMIDI_TEST_OUT and RTMIDI_TEST_IN to parts of the names of
 *     an output and an input that are connected, e.g. on Windows
 *     RTMIDI_TEST_OUT="Loopback (A)" RTMIDI_TEST_IN="Loopback (B)".
 *
 * Without either it exits with 77, which CTest and Automake count as skipped.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rtmidi_c.h"

#if defined(_WIN32)
  #include <windows.h>
  #define SLEEP_MS( ms ) Sleep( ms )
#else
  #include <unistd.h>
  #define SLEEP_MS( ms ) usleep( (ms) * 1000 )
#endif

#define SKIPPED 77

static int failures = 0;

static void check( int ok, const char *what )
{
  printf( "%s %s\n", ok ? "ok:  " : "FAIL:", what );
  if ( !ok ) failures++;
}

/* Opens the first port whose name contains `part`; returns 0 if there is none. */
static int openPortContaining( RtMidiPtr device, const char *part )
{
  unsigned int n = rtmidi_get_port_count( device );
  for ( unsigned int i = 0; i < n; i++ ) {
    char name[256];
    int len = sizeof name;
    if ( rtmidi_get_port_name( device, i, name, &len ) > 0 && strstr( name, part ) ) {
      rtmidi_open_port( device, i, "cgetmessage" );
      printf( "  opened \"%s\"\n", name );
      return device->ok;
    }
  }
  return 0;
}

/* Waits up to 2 s for the next message, offering a buffer of `capacity` bytes.
 * Returns what rtmidi_in_get_message() returned; *size is as it left it. */
static double receive( RtMidiInPtr in, unsigned char *buf, size_t capacity, size_t *size )
{
  double delta = 0;
  for ( int t = 0; t < 2000; t += 10 ) {
    *size = capacity;
    delta = rtmidi_in_get_message( in, buf, size );
    if ( *size > 0 || !in->ok ) break;
    SLEEP_MS( 10 );
  }
  return delta;
}

int main( void )
{
  RtMidiInPtr in = rtmidi_in_create_default();
  RtMidiOutPtr out = rtmidi_out_create_default();
  if ( !in->ok || !out->ok ) {
    printf( "skipped: cannot create MIDI ports (%s)\n", !in->ok ? in->msg : out->msg );
    return SKIPPED;
  }
  rtmidi_in_ignore_types( in, false, true, true );   /* keep SysEx */

  /* Connect the output to the input. */
  const char *outName = getenv( "RTMIDI_TEST_OUT" ), *inName = getenv( "RTMIDI_TEST_IN" );
  int connected;
  if ( outName && inName ) {
    connected = openPortContaining( in, inName ) && openPortContaining( out, outName );
  } else {
    rtmidi_open_virtual_port( in, "cgetmessage in" );
    connected = in->ok && openPortContaining( out, "cgetmessage in" );
  }
  if ( !connected ) {
    printf( "skipped: no loopback; set RTMIDI_TEST_OUT and RTMIDI_TEST_IN (see cgetmessage.c)\n" );
    rtmidi_in_free( in );
    rtmidi_out_free( out );
    return SKIPPED;
  }
  SLEEP_MS( 200 );

  const unsigned char noteA[] = { 0x90, 0x3C, 0x64 }, noteB[] = { 0x90, 0x3E, 0x64 };
  unsigned char sysex[600];
  sysex[0] = 0xF0;
  sysex[1] = 0x7D;   /* non-commercial manufacturer ID */
  for ( size_t k = 2; k < sizeof sysex - 1; k++ ) sysex[k] = (unsigned char) ( k & 0x7F );
  sysex[sizeof sysex - 1] = 0xF7;

  unsigned char buf[1024];
  size_t size;
  double delta;

  /* A buffer that is big enough. */
  rtmidi_out_send_message( out, noteA, sizeof noteA );
  receive( in, buf, sizeof buf, &size );
  check( in->ok && size == sizeof noteA && memcmp( buf, noteA, size ) == 0, "a 3-byte message into a 1024-byte buffer" );

  /* A buffer that is too small: reported, and the message is kept. */
  rtmidi_out_send_message( out, sysex, sizeof sysex );
  rtmidi_out_send_message( out, noteB, sizeof noteB );
  delta = receive( in, buf, 16, &size );
  check( !in->ok && in->msg && in->msg[0], "a 600-byte SysEx into a 16-byte buffer sets ok to false with a message" );
  printf( "       msg: %s\n", in->msg ? in->msg : "(null)" );
  check( size == sizeof sysex, "  ...and *size gives the length the buffer needs" );
  check( delta == -1, "  ...and it returns -1" );

  /* Calling again with a big enough buffer returns the SysEx, then the note. */
  receive( in, buf, sizeof buf, &size );
  check( in->ok && size == sizeof sysex && memcmp( buf, sysex, size ) == 0, "the next call returns the whole SysEx" );
  receive( in, buf, sizeof buf, &size );
  check( in->ok && size == sizeof noteB && memcmp( buf, noteB, size ) == 0, "and the call after that returns the next message" );

  /* A NULL buffer asks for the length without taking the message. */
  rtmidi_out_send_message( out, noteA, sizeof noteA );
  for ( int t = 0; t < 2000; t += 10 ) {
    size = 0;
    delta = rtmidi_in_get_message( in, NULL, &size );
    if ( size > 0 || !in->ok ) break;
    SLEEP_MS( 10 );
  }
  check( in->ok && size == sizeof noteA && delta == 0, "a NULL buffer reports the next message's length" );
  receive( in, buf, sizeof buf, &size );
  check( in->ok && size == sizeof noteA && memcmp( buf, noteA, size ) == 0, "  ...and leaves the message for the next call" );

  rtmidi_close_port( out );
  rtmidi_close_port( in );
  rtmidi_out_free( out );
  rtmidi_in_free( in );
  printf( "%s\n", failures ? "SOME CHECKS FAILED" : "all checks passed" );
  return failures ? 1 : 0;
}
