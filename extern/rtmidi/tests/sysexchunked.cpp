//*****************************************//
//  sysexchunked.cpp
//  by Eric Bateman, 2026.
//
//  Sends a SysEx of any size in pieces and checks it arrives whole (#214).
//  --resync checks that a SysEx whose end was lost doesn't corrupt the next one.
//  --interrupt checks that a note sent after a SysEx whose end was lost still arrives.
//  Exits 0 when the message comes back identical.
//
//  Measured limits on one sendMessage(), which sending in pieces avoids:
//    ALSA seq   5572 bytes between two applications (the receiver's kernel pool): a larger
//               SysEx is lost at the receiver, whose error callback reports an input overrun,
//               and so can a message sent right behind one close to that size. The sender
//               gets an error only from 16356 bytes (alsa-lib's output buffer).
//               To a hardware port, 4096 bytes (snd-seq-midi's buffer), cut off with no
//               error past that; pieces must also be paced to the device. The 512 B / 2 ms
//               default here suits apps and USB; DIN (3125 B/s) needs about 512 B / 170 ms.
//    JACK       16379 bytes (RtMidi's output ringbuffer), refused with a warning past that. JACK's
//               own port buffer limits what goes out in one period: 32720 bytes on JACK2 at
//               these settings; on JACK1 it depends on the period size and can be smaller
//    CoreMIDI   about 117 MB on macOS, 19 MB on iOS, dropped silently past that;
//               in pieces, about 650 to 760 pieces per message
//    WinMM      none found (256 MB), but sendMessage() blocks until the data is sent
//  Measured with alsa-lib 1.2.14, JACK 1.9.21, macOS 26.4.1, iPadOS 26.7, Windows 11 25H2.
//
//  Receiving needs ignoreTypes( false, ... ).  The SysEx must arrive in one callback,
//  with the default input buffers; WinMM used to deliver one piece per buffer (#370).
//  On iOS, virtual ports need UIBackgroundModes "audio" in the app's Info.plist.
//*****************************************//

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include "RtMidi.h"

#if defined(_WIN32)
  #define NOMINMAX  // windows.h's min() macro breaks std::min
  #include <windows.h>
  #define SLEEP( milliseconds ) Sleep( (DWORD) milliseconds )
#else // Unix variants
  #include <unistd.h>
  #define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )
#endif

// Any size works: only the first piece carries F0 and only the last F7.
static const size_t kSysExSpan = 512;

void sendLargeSysEx( RtMidiOut &midiout, const std::vector<unsigned char> &message )
{
  for ( size_t offset = 0; offset < message.size(); offset += kSysExSpan ) {
    size_t count = std::min( kSysExSpan, message.size() - offset );
    midiout.sendMessage( &message[offset], count );
    SLEEP( 2 );  // JACK takes about 32 kB per period, so spread the pieces out
  }
}

void usage( void )
{
  std::cout << "\nusage: sysexchunked N [out in]\n";
  std::cout << "       sysexchunked -l\n";
  std::cout << "       sysexchunked --listen [seconds [in]]\n";
  std::cout << "       sysexchunked --resync [out in]\n";
  std::cout << "       sysexchunked --interrupt [out in]\n\n";
  std::cout << "    N         length of the SysEx message to send.  Try a size a\n";
  std::cout << "              single sendMessage() cannot carry, such as 66312,\n";
  std::cout << "              the size of a real firmware dump.\n";
  std::cout << "    --listen  receive only, and report what arrives.  Useful for\n";
  std::cout << "              checking what a device actually sends, and whether\n";
  std::cout << "              a large message survives the transport.\n";
  std::cout << "    --resync  send the start of a SysEx with no F7, then a whole one,\n";
  std::cout << "              and check that only the whole one arrives.\n";
  std::cout << "    --interrupt  send the start of a SysEx with no F7, then a note on,\n";
  std::cout << "              and check that the note arrives.\n";
  std::cout << "    out in    port numbers, on APIs without virtual ports (Windows, Android).\n";
  std::cout << "              On ALSA and JACK the test connects to itself.\n";
  std::cout << "              The default is port 0, which is rarely a loopback.\n";
  std::cout << "    -l        list port numbers; they can change when devices are replugged.\n\n";
  exit( 0 );
}

// On ALSA and JACK an output can open our own virtual input, so the test needs no patching.
static bool openOwnInput( RtMidiOut &midiout )
{
  if ( midiout.getCurrentApi() != RtMidi::LINUX_ALSA && midiout.getCurrentApi() != RtMidi::UNIX_JACK )
    return false;
  for ( unsigned int i = 0; i < midiout.getPortCount(); i++ )
    if ( midiout.getPortName( i ).find( "sysexchunked in" ) != std::string::npos ) {
      midiout.openPort( i );
      return true;
    }
  return false;
}

void listPorts( void )
{
  RtMidiOut midiout;
  RtMidiIn midiin;
  for ( unsigned int i = 0; i < midiout.getPortCount(); i++ )
    std::cout << "out " << i << ": " << midiout.getPortName( i ) << "\n";
  for ( unsigned int i = 0; i < midiin.getPortCount(); i++ )
    std::cout << "in  " << i << ": " << midiin.getPortName( i ) << "\n";
}

static std::vector<unsigned char> received;
static bool complete = false;
static unsigned int callbacks = 0;
static bool gotNote = false;

void mycallback( double /*deltatime*/, std::vector<unsigned char> *message, void * /*userData*/ )
{
  if ( !message->empty() && ( *message )[0] == 0x90 ) { gotNote = true; return; }
  received.insert( received.end(), message->begin(), message->end() );
  callbacks++;
  if ( !received.empty() && received.back() == 0xF7 ) complete = true;
}

int main( int argc, char *argv[] )
{
  if ( argc < 2 ) usage();

  if ( std::string( argv[1] ) == "-l" ) {
    try { listPorts(); }
    catch ( RtMidiError &error ) { error.printMessage(); return 1; }
    return 0;
  }

  bool listen = ( std::string( argv[1] ) == "--listen" );
  bool resync = ( std::string( argv[1] ) == "--resync" );
  bool interrupt = ( std::string( argv[1] ) == "--interrupt" );
  size_t nBytes = 0;
  int seconds = 30;
  int result = 1;
  unsigned int outPort = 0, inPort = 0;
  bool selfConnected = false;

  if ( listen ) {
    if ( argc > 2 ) seconds = atoi( argv[2] );
    if ( argc > 3 ) inPort = (unsigned int) atoi( argv[3] );
  } else {
    if ( argc != 2 && argc != 4 ) usage();
    nBytes = ( resync || interrupt ) ? 6 : (size_t) atoi( argv[1] );
    if ( nBytes < 3 ) usage();
    if ( argc == 4 ) {
      outPort = (unsigned int) atoi( argv[2] );
      inPort = (unsigned int) atoi( argv[3] );
    }
  }

  RtMidiOut *midiout = 0;
  RtMidiIn *midiin = 0;

  try {
    midiout = new RtMidiOut();
    midiin = new RtMidiIn();

    // Windows and Android have no virtual ports, so use real ports there.
    if ( midiin->getCurrentApi() == RtMidi::WINDOWS_MM ||
         midiin->getCurrentApi() == RtMidi::WINDOWS_UWP ||
         midiin->getCurrentApi() == RtMidi::WINDOWS_MIDI_SERVICES ||
         midiin->getCurrentApi() == RtMidi::ANDROID_AMIDI ) {
      if ( inPort >= midiin->getPortCount() || outPort >= midiout->getPortCount() ) {
        std::cout << "No such MIDI port.\n";
        goto cleanup;
      }
      if ( listen )
        std::cout << "Listening on \"" << midiin->getPortName( inPort ) << "\".\n";
      else
        std::cout << "Opening \"" << midiout->getPortName( outPort ) << "\" for output and\n"
                  << "        \"" << midiin->getPortName( inPort ) << "\" for input.\n"
                  << "Connect them externally for the round trip to complete.\n";
      midiout->openPort( outPort );
      midiin->openPort( inPort );
    }
    else {
      midiin->openVirtualPort( "sysexchunked in" );
      if ( listen )
        std::cout << "Opened virtual port \"sysexchunked in\".\n"
                  << "Connect a device or another application to it.\n";
      else if ( openOwnInput( *midiout ) ) {
        std::cout << "Sending to our own virtual port \"sysexchunked in\".\n";
        selfConnected = true;
      }
      else {
        midiout->openVirtualPort( "sysexchunked out" );
        std::cout << "Opened virtual ports \"sysexchunked out\" and\n"
                  << "\"sysexchunked in\". Connect them now";
      }
    }

    midiin->ignoreTypes( false, true, true );   // do not ignore SysEx

    // No setBufferSize(): a SysEx longer than WinMM's 1 kB buffers must still arrive whole.
    midiin->setCallback( &mycallback );

    if ( listen ) {
      std::cout << "Listening for " << seconds << " seconds...\n";
      for ( int i = 0; i < seconds * 10 && !complete; i++ ) SLEEP( 100 );

      if ( received.empty() ) {
        std::cout << "Nothing received.\n";
      } else {
        // More than one callback means the message arrived in pieces.
        std::cout << "Received " << received.size() << " bytes in "
                  << callbacks << " callback(s).\n";
        std::cout << "First bytes:";
        for ( size_t i = 0; i < received.size() && i < 8; i++ )
          std::cout << " " << std::hex << (int) received[i] << std::dec;
        std::cout << ( complete ? "  (ends with F7)\n" : "  (no F7 seen)\n" );
        result = complete ? 0 : 1;
      }
    }
    else {
      // Give the user a moment to patch the two ports together before sending.
      if ( selfConnected ) SLEEP( 300 );
      else {
        for ( int i = 0; i < 10; i++ ) { std::cout << "." << std::flush; SLEEP( 500 ); }
        std::cout << "\n";
      }

      // F0 7D <data...> F7.  0x7D is the non-commercial manufacturer id.
      std::vector<unsigned char> message;
      message.push_back( 0xF0 );
      message.push_back( 0x7D );
      while ( message.size() < nBytes - 1 )
        message.push_back( (unsigned char) ( message.size() & 0x7F ) );
      message.push_back( 0xF7 );

      if ( interrupt ) {
        std::vector<unsigned char> truncated = { 0xF0, 0x7D, 0x7F, 0x7F, 0x7F };
        std::vector<unsigned char> note = { 0x90, 0x3C, 0x40 };
        std::cout << "Sending the start of a SysEx with no F7, then a note on...\n";
        midiout->sendMessage( &truncated );
        SLEEP( 50 );
        midiout->sendMessage( &note );
        for ( int i = 0; i < 400 && !gotNote; i++ ) SLEEP( 5 );
        std::cout << ( gotNote ? "The note arrived.\n" : "The note was lost.\n" );
        result = gotNote ? 0 : 1;
        goto cleanup;
      }

      if ( resync ) {
        std::vector<unsigned char> truncated = { 0xF0, 0x7D, 0x7F, 0x7F, 0x7F };
        std::cout << "Sending the start of a SysEx with no F7, then a whole one...\n";
        midiout->sendMessage( &truncated );
        SLEEP( 50 );
        midiout->sendMessage( &message );
      }
      else {
        std::cout << "Sending " << message.size() << " bytes in "
                  << kSysExSpan << "-byte pieces...\n";
        sendLargeSysEx( *midiout, message );
      }

      for ( int i = 0; i < 2000 && !complete; i++ ) SLEEP( 5 );
      if ( resync ) SLEEP( 200 );   // let a second message arrive, see below

      // With the October 2026 Windows 11 update (build 26200.9711) the Windows MIDI
      // service, which carries WinMM and UWP output, closes the cut-off SysEx with
      // F7 itself and delivers it before the whole one.
      bool closedByWindows = false;
      if ( resync && callbacks == 2 &&
           ( midiout->getCurrentApi() == RtMidi::WINDOWS_MM || midiout->getCurrentApi() == RtMidi::WINDOWS_UWP ) ) {
        std::vector<unsigned char> closed = { 0xF0, 0x7D, 0x7F, 0x7F, 0x7F, 0xF7 };
        closed.insert( closed.end(), message.begin(), message.end() );
        closedByWindows = ( received == closed );
      }

      if ( received.empty() )
        std::cout << "Nothing received. Are the two ports connected?\n";
      else if ( closedByWindows )
        std::cout << "Received the cut-off SysEx, closed by Windows, then the whole one: accepted.\n";
      else
        std::cout << "Received " << received.size() << " bytes in " << callbacks << " callback(s): "
                  << ( received == message ? "identical." : "MISMATCH." ) << "\n";
      result = ( ( received == message && callbacks == 1 ) || closedByWindows ) ? 0 : 1;
    }
  }
  catch ( RtMidiError &error ) {
    error.printMessage();
  }

 cleanup:
  delete midiout;
  delete midiin;
  return result;
}
