//*****************************************//
//  cmidiin.cpp
//  by Gary Scavone, 2003-2004.
//
//  Simple program to test MIDI input and
//  use of a user callback function.
//
//*****************************************//

#include <iostream>
#include <cstdlib>
#include "RtMidi.h"

void usage( void ) {
  // Error function in case of incorrect command-line
  // argument specifications.
  std::cout << "\nuseage: cmidiin <port>\n";
  std::cout << "    where port = the device to use (first / default = 0).\n\n";
  exit( 0 );
}

void mycallback( double deltatime, std::vector< unsigned char > *message, void */*userData*/ )
{
  unsigned int nBytes = message->size();
  for ( unsigned int i=0; i<nBytes; i++ )
    std::cout << "Byte " << i << " = " << (int)message->at(i) << ", ";
  if ( nBytes > 0 )
    std::cout << "stamp = " << deltatime << std::endl;
}

// This function should be embedded in a try/catch block in case of
// an exception.  It offers the user a choice of MIDI ports to open.
// It returns false if there are no ports available.
bool chooseMidiPort( RtMidiIn *rtmidi );

RtMidi::Api chooseMidiApi();

int main( int argc, char ** /*argv[]*/ )
{
  RtMidiIn *midiin = 0;

  // Minimal command-line check.
  if ( argc > 2 ) usage();

  try {

    // RtMidiIn constructor
    midiin = new RtMidiIn(chooseMidiApi());

    // Call function to select port.
    if ( chooseMidiPort( midiin ) == false ) goto cleanup;

    // Set our callback function.  This should be done immediately after
    // opening the port to avoid having incoming messages written to the
    // queue instead of sent to the callback function.
    midiin->setCallback( &mycallback );

    // Don't ignore sysex, timing, or active sensing messages.
    midiin->ignoreTypes( false, false, false );

    std::cout << "\nReading MIDI input ... press <enter> to quit.\n";
    char input;
    std::cin.get(input);

  } catch ( RtMidiError &error ) {
    error.printMessage();
  }

 cleanup:

  delete midiin;

  return 0;
}

bool chooseMidiPort( RtMidiIn *rtmidi )
{
  std::cout << "\nWould you like to open a virtual input port? [y/N] ";

  std::string keyHit;
  std::getline( std::cin, keyHit );
  if ( keyHit == "y" ) {
    rtmidi->openVirtualPort();
    return true;
  }

  std::string portName;
  unsigned int i = 0, nPorts = rtmidi->getPortCount();
  if ( nPorts == 0 ) {
    std::cout << "No input ports available!" << std::endl;
    return false;
  }

  if ( nPorts == 1 ) {
    std::cout << "\nOpening " << rtmidi->getPortName() << std::endl;
  }
  else {
    for ( i=0; i<nPorts; i++ ) {
      portName = rtmidi->getPortName(i);
      std::cout << "  Input port #" << i << ": " << portName << '\n';
    }

    do {
      std::cout << "\nChoose a port number: ";
      std::cin >> i;
    } while ( i >= nPorts );
    std::getline( std::cin, keyHit );  // used to clear out stdin
  }

  rtmidi->openPort( i );

  return true;
}

RtMidi::Api chooseMidiApi()
{
  std::vector< RtMidi::Api > apis;
  RtMidi::getCompiledApi( apis );

  if (apis.size() <= 1)
      return RtMidi::Api::UNSPECIFIED;

  std::cout << "\nAPIs\n  API #0: unspecified / default\n";
  for (size_t n = 0; n < apis.size(); n++)
      std::cout << "  API #" << apis[n] << ": " << RtMidi::getApiDisplayName(apis[n]) << "\n";

  std::cout << "\nChoose an API number: ";
  unsigned int i;
  std::cin >> i;

  std::string dummy;
  std::getline( std::cin, dummy );  // used to clear out stdin

  return static_cast< RtMidi::Api >( i );
}
