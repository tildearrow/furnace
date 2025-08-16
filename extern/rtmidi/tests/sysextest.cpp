//*****************************************//
//  sysextest.cpp
//  by Gary Scavone, 2003-2005.
//
//  Simple program to test MIDI sysex sending and receiving.
//
//*****************************************//

#include <iostream>
#include <cstdlib>
#include <typeinfo>
#include "RtMidi.h"

void usage( void ) {
  std::cout << "\nuseage: sysextest N\n";
  std::cout << "    where N = length of sysex data to send / receive.\n\n";
  exit( 0 );
}

// Platform-dependent sleep routines.
#if defined(_WIN32)
  #include <windows.h>
  #define SLEEP( milliseconds ) Sleep( (DWORD) milliseconds ) 
#else // Unix variants
  #include <unistd.h>
  #define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )
#endif

// This function should be embedded in a try/catch block in case of
// an exception.  It offers the user a choice of MIDI ports to open.
// It returns false if there are no ports available.
bool chooseMidiPort( RtMidi *rtmidi );

RtMidi::Api chooseMidiApi();

void mycallback( double deltatime, std::vector< unsigned char > *message, void * /*userData*/ )
{
  unsigned int nBytes = message->size();
  for ( unsigned int i=0; i<nBytes; i++ )
    std::cout << "Byte " << i << " = " << (int)message->at(i) << ", ";
  if ( nBytes > 0 )
    std::cout << "# of bytes = " << nBytes << ", stamp = " << deltatime << std::endl;
}

int main( int argc, char *argv[] )
{
  RtMidiOut *midiout = 0;
  RtMidiIn *midiin = 0;
  std::vector<unsigned char> message;
  unsigned int i, nBytes;

  // Minimal command-line check.
  if ( argc != 2 ) usage();
  nBytes = (unsigned int) atoi( argv[1] );

  // RtMidiOut and RtMidiIn constructors
  try {
    RtMidi::Api api = chooseMidiApi();
    midiout = new RtMidiOut( api );
    midiin = new RtMidiIn( api );
  }
  catch ( RtMidiError &error ) {
    error.printMessage();
    goto cleanup;
  }

  // Don't ignore sysex, timing, or active sensing messages.
  midiin->ignoreTypes( false, true, true );

  try {
    if ( chooseMidiPort( midiin ) == false ) goto cleanup;
    if ( chooseMidiPort( midiout ) == false ) goto cleanup;

    midiin->setCallback( &mycallback );

    message.push_back( 0xF6 );
    midiout->sendMessage( &message );
    SLEEP( 500 ); // pause a little

    // Create a long sysex message of numbered bytes and send it out ... twice.
    for ( int n=0; n<2; n++ ) {
      message.clear();
      message.push_back( 240 );
      for ( i=0; i<nBytes; i++ )
        message.push_back( i % 128 );
      message.push_back( 247 );
      midiout->sendMessage( &message );

      SLEEP( 500 ); // pause a little
    }
  }
  catch ( RtMidiError &error ) {
    error.printMessage();
    goto cleanup;
  }

  // Clean up
 cleanup:
  delete midiout;
  delete midiin;

  return 0;
}

bool chooseMidiPort( RtMidi *rtmidi )
{
  bool isInput = false;
  if ( typeid( *rtmidi ) == typeid( RtMidiIn ) )
    isInput = true;

  if ( isInput )
    std::cout << "\nWould you like to open a virtual input port? [y/N] ";
  else
    std::cout << "\nWould you like to open a virtual output port? [y/N] ";

  std::string keyHit;
  std::getline( std::cin, keyHit );
  if ( keyHit == "y" ) {
    rtmidi->openVirtualPort();
    return true;
  }

  std::string portName;
  unsigned int i = 0, nPorts = rtmidi->getPortCount();
  if ( nPorts == 0 ) {
    if ( isInput )
      std::cout << "No input ports available!" << std::endl;
    else
      std::cout << "No output ports available!" << std::endl;
    return false;
  }

  if ( nPorts == 1 ) {
    std::cout << "\nOpening " << rtmidi->getPortName() << std::endl;
  }
  else {
    for ( i=0; i<nPorts; i++ ) {
      portName = rtmidi->getPortName(i);
      if ( isInput )
        std::cout << "  Input port #" << i << ": " << portName << '\n';
      else
        std::cout << "  Output port #" << i << ": " << portName << '\n';
    }

    do {
      std::cout << "\nChoose a port number: ";
      std::cin >> i;
    } while ( i >= nPorts );
    std::getline(std::cin, keyHit);  // used to clear out stdin
  }

  std::cout << std::endl;
  rtmidi->openPort( i );

  return true;
}

RtMidi::Api chooseMidiApi()
{
  std::vector< RtMidi::Api > apis;
  RtMidi::getCompiledApi(apis);

  if (apis.size() <= 1)
    return RtMidi::Api::UNSPECIFIED;

  std::cout << "\nAPIs\n  API #0: unspecified / default\n";
  for (size_t n = 0; n < apis.size(); n++)
    std::cout << "  API #" << apis[n] << ": " << RtMidi::getApiDisplayName(apis[n]) << "\n";

  std::cout << "\nChoose an API number: ";
  unsigned int i;
  std::cin >> i;

  std::string dummy;
  std::getline(std::cin, dummy);  // used to clear out stdin

  return static_cast<RtMidi::Api>(i);
}
