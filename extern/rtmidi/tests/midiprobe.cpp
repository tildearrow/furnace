// midiprobe.cpp
//
// Simple program to check MIDI inputs and outputs.
//
// by Gary Scavone, 2003-2012.

#include <iostream>
#include <cstdlib>
#include "RtMidi.h"

static std::string apiName( RtMidi::Api api )
{
  return RtMidi::getApiDisplayName( api );
}

int main()
{
  std::vector< RtMidi::Api > apis;
  RtMidi :: getCompiledApi( apis );

  // An API can be compiled in but unusable here, e.g. Windows MIDI Services
  // before Windows 11 25H2 or with Windows in legacy MIDI mode.
  std::cout << "\nCompiled APIs:\n";
  for ( unsigned int i=0; i<apis.size(); i++ ) {
    RtMidi::RtMidiApiAvailability availability = RtMidi::checkApiAvailability( apis[i] );
    std::cout << "  " << apiName( apis[i] );
    if ( !availability.available ) std::cout << " (not available: " << availability.message << ")";
    std::cout << std::endl;
  }

  // With no API argument, RtMidi uses the first available compiled API that has a port.
  try {
    RtMidiIn defaultIn;
    RtMidiOut defaultOut;
    std::cout << "\nDefault API (no api argument): input " << apiName( defaultIn.getCurrentApi() )
              << ", output " << apiName( defaultOut.getCurrentApi() ) << std::endl;
  } catch ( RtMidiError &error ) {
    error.printMessage();
  }
  std::cout << std::endl;

  for ( unsigned int i=0; i<apis.size(); i++ ){
    if ( !RtMidi::checkApiAvailability( apis[i] ).available ) continue;
    std::cout << "Probing with API " << apiName( apis[i] ) << std::endl;

    RtMidiIn  *midiin = 0;
    RtMidiOut *midiout = 0;

    try {

      // RtMidiIn constructor ... exception possible
      midiin = new RtMidiIn(apis[i]);

      std::cout << "\nCurrent input API: " << apiName( midiin->getCurrentApi() ) << std::endl;

      // Check inputs.
      unsigned int nPorts = midiin->getPortCount();
      std::cout << "\nThere are " << nPorts << " MIDI input sources available.\n";

      for ( unsigned i=0; i<nPorts; i++ ) {
        std::string portName = midiin->getPortName(i);
        std::cout << "  Input Port #" << i << ": " << portName << '\n';
      }

      // RtMidiOut constructor ... exception possible
      midiout = new RtMidiOut(apis[i]);

      std::cout << "\nCurrent output API: " << apiName( midiout->getCurrentApi() ) << std::endl;

      // Check outputs.
      nPorts = midiout->getPortCount();
      std::cout << "\nThere are " << nPorts << " MIDI output ports available.\n";

      for ( unsigned i=0; i<nPorts; i++ ) {
        std::string portName = midiout->getPortName(i);
        std::cout << "  Output Port #" << i << ": " << portName << std::endl;
      }
      std::cout << std::endl;

    } catch ( RtMidiError &error ) {
      error.printMessage();
    }

    delete midiin;
    delete midiout;
  }

  return 0;
}
