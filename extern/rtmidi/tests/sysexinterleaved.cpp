//*****************************************//
//  sysexinterleaved.cpp
//  by Eric Bateman, 2026.
//
//  Checks how SysEx is framed when other messages arrive in the middle of it,
//  sending and receiving: real-time bytes inside a SysEx (#247), a SysEx that
//  loses its F7 (#407), and a SysEx abandoned for another message.
//
//  It needs MIDI going round a loop. Where the API has virtual ports (ALSA,
//  JACK, CoreMIDI) it makes its own. Elsewhere set RTMIDI_TEST_OUT and
//  RTMIDI_TEST_IN to parts of the names of a connected output and input.
//  RTMIDI_TEST_API picks the API by name (e.g. "winmm", "winmidi2").
//  Exits 0 if every case passes, 1 if one fails, 77 if there is no loop.
//*****************************************//

#if defined(_MSC_VER)
  #define _CRT_SECURE_NO_WARNINGS  // getenv(); the VS projects build with /sdl, which makes C4996 an error
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>
#include <vector>
#include "RtMidi.h"

#if defined(_WIN32)
  #include <windows.h>
  #define SLEEP_MS( ms ) Sleep( (DWORD) ( ms ) )
#else
  #include <unistd.h>
  #define SLEEP_MS( ms ) usleep( (unsigned long) ( ms ) * 1000 )
#endif

typedef std::vector<unsigned char> Bytes;

struct Case {
  const char *name;
  bool ignoreTiming;                 // ignoreTypes() for timing and sensing
  std::vector<Bytes> send;           // one sendMessage() call each
  std::vector<Bytes> expect;         // the messages the input should deliver
  int reopenAfter;                   // close and reopen the output after this send (-1: never)
};

static std::mutex lock;
static std::vector<Bytes> received;

static void onMessage( double, std::vector<unsigned char> *message, void * )
{
  std::lock_guard<std::mutex> g( lock );
  received.push_back( *message );
}

static void onError( RtMidiError::Type, const std::string &text, void * )
{
  printf( "    (%s)\n", text.c_str() );
}

static std::string hex( const std::vector<Bytes> &list )
{
  std::string s;
  char b[4];
  for ( const Bytes &m : list ) {
    s += s.empty() ? "[" : " [";
    for ( size_t i = 0; i < m.size(); i++ ) { snprintf( b, sizeof b, i ? " %02X" : "%02X", m[i] ); s += b; }
    s += "]";
  }
  return s.empty() ? "nothing" : s;
}

static bool openContaining( RtMidi &midi, const char *part, const std::string &as )
{
  for ( unsigned int i = 0; i < midi.getPortCount(); i++ )
    if ( midi.getPortName( i ).find( part ) != std::string::npos ) { midi.openPort( i, as ); return true; }
  return false;
}

// Runs one case on fresh ports.  Returns 1 if it passed, 0 if it failed, -1 if there is no loop.
static int run( RtMidi::Api api, const Case &c, int index )
{
  RtMidiIn in( api, "sysexinterleaved" );
  RtMidiOut out( api, "sysexinterleaved" );
  in.setErrorCallback( &onError );
  out.setErrorCallback( &onError );
  in.ignoreTypes( false, c.ignoreTiming, c.ignoreTiming );
  in.setCallback( &onMessage );

  const char *outName = getenv( "RTMIDI_TEST_OUT" ), *inName = getenv( "RTMIDI_TEST_IN" );
  std::string port = "sysexinterleaved in " + std::to_string( index );
  if ( outName && inName ) {
    if ( !openContaining( in, inName, "sysexinterleaved in" ) ) return -1;
    port = outName;
  }
  else {
    try { in.openVirtualPort( port ); } catch ( RtMidiError & ) { return -1; }
  }
  if ( !openContaining( out, port.c_str(), "sysexinterleaved out" ) ) return -1;
  SLEEP_MS( 300 );
  {
    std::lock_guard<std::mutex> g( lock );
    received.clear();
  }

  for ( size_t i = 0; i < c.send.size(); i++ ) {
    out.sendMessage( &c.send[i] );
    SLEEP_MS( 20 );
    if ( (int) i == c.reopenAfter ) {
      out.closePort();
      if ( !openContaining( out, port.c_str(), "sysexinterleaved out" ) ) return -1;
      SLEEP_MS( 300 );
    }
  }
  for ( int t = 0; t < 100; t++ ) {        // up to 1 s for everything, then 200 ms for strays
    { std::lock_guard<std::mutex> g( lock ); if ( received.size() >= c.expect.size() ) break; }
    SLEEP_MS( 10 );
  }
  SLEEP_MS( 200 );
  in.cancelCallback();

  std::lock_guard<std::mutex> g( lock );
  bool ok = ( received == c.expect );
  // With the October 2026 Windows 11 update (build 26200.9711) the Windows MIDI
  // service, which carries WinMM and UWP output, closes a cut-off SysEx with F7
  // itself when the next status byte arrives, ahead of the expected messages.
  bool closedByWindows = false;
  RtMidi::Api outApi = out.getCurrentApi();
  if ( !ok && ( outApi == RtMidi::WINDOWS_MM || outApi == RtMidi::WINDOWS_UWP ) &&
       !c.send.empty() && c.send[0].size() > 1 && c.send[0][0] == 0xF0 && c.send[0].back() != 0xF7 ) {
    std::vector<Bytes> alt( 1, c.send[0] );
    alt[0].push_back( 0xF7 );
    alt.insert( alt.end(), c.expect.begin(), c.expect.end() );
    ok = closedByWindows = ( received == alt );
  }
  printf( "%s %s%s\n", ok ? "ok:  " : "FAIL:", c.name, closedByWindows ? " (Windows closed the cut-off SysEx itself)" : "" );
  if ( !ok )
    printf( "      expected %s\n      received %s\n", hex( c.expect ).c_str(), hex( received ).c_str() );
  return ok ? 1 : 0;
}

int main()
{
  RtMidi::Api api = RtMidi::UNSPECIFIED;
  if ( const char *name = getenv( "RTMIDI_TEST_API" ) ) api = RtMidi::getCompiledApiByName( name );

  const Bytes note = { 0x90, 0x3C, 0x40 }, empty = { 0xF0, 0xF7 };
  const std::vector<Case> cases = {
    { "SysEx sent in two pieces arrives whole", false,
      { { 0xF0, 0x7D, 0x01 }, { 0x02, 0xF7 } },
      { { 0xF0, 0x7D, 0x01, 0x02, 0xF7 } }, -1 },
    { "clock between two pieces of a SysEx is delivered on its own", false,
      { { 0xF0, 0x7D, 0x01, 0x02 }, { 0xF8 }, { 0x03, 0x04, 0xF7 } },
      { { 0xF8 }, { 0xF0, 0x7D, 0x01, 0x02, 0x03, 0x04, 0xF7 } }, -1 },
    { "clock inside a one-call SysEx is delivered on its own", false,
      { { 0xF0, 0x7D, 0x01, 0xF8, 0x02, 0xF7 } },
      { { 0xF8 }, { 0xF0, 0x7D, 0x01, 0x02, 0xF7 } }, -1 },
    { "ignored active sensing inside a SysEx is dropped", true,
      { { 0xF0, 0x7D, 0x01, 0x02 }, { 0xFE }, { 0x03, 0x04, 0xF7 } },
      { { 0xF0, 0x7D, 0x01, 0x02, 0x03, 0x04, 0xF7 } }, -1 },
    { "a SysEx that loses its F7 is dropped, the next one arrives", false,
      { { 0xF0, 0x7D, 0x7F, 0x7F, 0x7F }, { 0xF0, 0x7D, 0x01, 0xF7 } },
      { { 0xF0, 0x7D, 0x01, 0xF7 } }, -1 },
    { "a SysEx abandoned for a note is closed with F0 F7, then the note", false,
      { { 0xF0, 0x7D, 0x7F, 0x7F, 0x7F }, note },
      { empty, note }, -1 },
    { "a SysEx abandoned for a program change", false,
      { { 0xF0, 0x7D, 0x7F, 0x7F, 0x7F }, { 0xC0, 0x05 }, note },
      { empty, { 0xC0, 0x05 }, note }, -1 },
    { "a SysEx abandoned for an MTC quarter frame", false,
      { { 0xF0, 0x7D, 0x7F, 0x7F, 0x7F }, { 0xF1, 0x20 }, note },
      { empty, { 0xF1, 0x20 }, note }, -1 },
    { "a SysEx left open when the output closes isn't closed on the next open", false,
      { { 0xF0, 0x7D, 0x7F, 0x7F, 0x7F }, note },
      { note }, 0 },
  };

  int failed = 0, index = 0;
  for ( const Case &c : cases ) {
    int r;
    try { r = run( api, c, index++ ); }
    catch ( RtMidiError &e ) { printf( "skipped: %s\n", e.getMessage().c_str() ); return 77; }
    if ( r < 0 ) {
      printf( "skipped: no loopback; set RTMIDI_TEST_OUT and RTMIDI_TEST_IN (see sysexinterleaved.cpp)\n" );
      return 77;
    }
    if ( r == 0 ) failed++;
  }
  printf( "%s\n", failed ? "SOME CASES FAILED" : "all cases passed" );
  return failed ? 1 : 0;
}
