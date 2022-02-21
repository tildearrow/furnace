//*****************************************//
//  midiclock.cpp
//
//  Simple program to test MIDI clock sync.  Run midiclock_in in one
//  console and midiclock_out in the other, make sure to choose
//  options that connect the clocks between programs on your platform.
//
//  (C)2016 Refer to README.md in this archive for copyright.
//
//*****************************************//

#include <iostream>
#include <cstdlib>
#include "RtMidi.h"

// Platform-dependent sleep routines.
#if defined(WIN32)
  #include <windows.h>
  #define SLEEP( milliseconds ) Sleep( (DWORD) milliseconds ) 
#else // Unix variants
  #include <unistd.h>
  #define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )
#endif

// These functions should be embedded in a try/catch block in case of
// an exception.  It offers the user a choice of MIDI ports to open.
// It returns false if there are no ports available.
bool chooseInputPort( RtMidiIn *rtmidi );
bool chooseOutputPort( RtMidiOut *rtmidi );

void mycallback( double deltatime, std::vector< unsigned char > *message, void *user )
{
  unsigned int *clock_count = reinterpret_cast<unsigned int*>(user);

  // Ignore longer messages
  if (message->size() != 1)
    return;

  unsigned int msg = message->at(0);
  if (msg == 0xFA)
    std::cout << "START received" << std::endl;
  if (msg == 0xFB)
    std::cout << "CONTINUE received" << std::endl;
  if (msg == 0xFC)
    std::cout << "STOP received" << std::endl;
  if (msg == 0xF8) {
    if (++*clock_count == 24) {
      double bpm = 60.0 / 24.0 / deltatime;
      std::cout << "One beat, estimated BPM = " << bpm <<std::endl;
      *clock_count = 0;
    }
  }
  else
    *clock_count = 0;
}

int clock_in()
{
  RtMidiIn *midiin = 0;
  unsigned int clock_count = 0;

  try {

    // RtMidiIn constructor
    midiin = new RtMidiIn();

    // Call function to select port.
    if ( chooseInputPort( midiin ) == false ) goto cleanup;

    // Set our callback function.  This should be done immediately after
    // opening the port to avoid having incoming messages written to the
    // queue instead of sent to the callback function.
    midiin->setCallback( &mycallback, &clock_count );

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

int clock_out()
{
  RtMidiOut *midiout = 0;
  std::vector<unsigned char> message;
  int sleep_ms = 0, k = 0, j = 0;

  // RtMidiOut constructor
  try {
    midiout = new RtMidiOut();
  }
  catch ( RtMidiError &error ) {
    error.printMessage();
    exit( EXIT_FAILURE );
  }

  // Call function to select port.
  try {
    if ( chooseOutputPort( midiout ) == false ) goto cleanup;
  }
  catch ( RtMidiError &error ) {
    error.printMessage();
    goto cleanup;
  }

  // Period in ms = 100 BPM
  // 100*24 ticks / 1 minute, so (60*1000) / (100*24) = 25 ms / tick
  sleep_ms = 25;
  std::cout << "Generating clock at "
            << (60.0 / 24.0 / sleep_ms * 1000.0)
            << " BPM." << std::endl;

  // Send out a series of MIDI clock messages.
  // MIDI start
  message.clear();
  message.push_back( 0xFA );
  midiout->sendMessage( &message );
  std::cout << "MIDI start" << std::endl;

  for (j=0; j < 8; j++)
  {
    if (j > 0)
    {
      // MIDI continue
      message.clear();
      message.push_back( 0xFB );
      midiout->sendMessage( &message );
      std::cout << "MIDI continue" << std::endl;
    }

    for (k=0; k < 96; k++) {
      // MIDI clock
      message.clear();
      message.push_back( 0xF8 );
      midiout->sendMessage( &message );
      if (k % 24 == 0)
        std::cout << "MIDI clock (one beat)" << std::endl;
      SLEEP( sleep_ms );
    }

    // MIDI stop
    message.clear();
    message.push_back( 0xFC );
    midiout->sendMessage( &message );
    std::cout << "MIDI stop" << std::endl;
    SLEEP( 500 );
  }

  // MIDI stop
  message.clear();
  message.push_back( 0xFC );
  midiout->sendMessage( &message );
  std::cout << "MIDI stop" << std::endl;

  SLEEP( 500 );

  std::cout << "Done!" << std::endl;

  // Clean up
 cleanup:
  delete midiout;

  return 0;
}

int main( int, const char *argv[] )
{
  std::string prog(argv[0]);
  if (prog.find("midiclock_in") != prog.npos) {
    clock_in();
  }
  else if (prog.find("midiclock_out") != prog.npos) {
    clock_out();
  }
  else {
    std::cout << "Don't know what to do as " << prog << std::endl;
  }
  return 0;
}

template<typename RT>
bool choosePort( RT *rtmidi, const char *dir )
{
  std::string portName;
  unsigned int i = 0, nPorts = rtmidi->getPortCount();
  if ( nPorts == 0 ) {
    std::cout << "No " << dir << " ports available!" << std::endl;
    return false;
  }

  if ( nPorts == 1 ) {
    std::cout << "\nOpening " << rtmidi->getPortName() << std::endl;
  }
  else {
    for ( i=0; i<nPorts; i++ ) {
      portName = rtmidi->getPortName(i);
      std::cout << "  " << dir << " port #" << i << ": " << portName << '\n';
    }

    do {
      std::cout << "\nChoose a port number: ";
      std::cin >> i;
    } while ( i >= nPorts );
  }

  std::cout << "\n";
  rtmidi->openPort( i );

  return true;
}

bool chooseInputPort( RtMidiIn *rtmidi )
{
  return choosePort<RtMidiIn>( rtmidi, "input" );
}

bool chooseOutputPort( RtMidiOut *rtmidi )
{
  return choosePort<RtMidiOut>( rtmidi, "output" );
}
