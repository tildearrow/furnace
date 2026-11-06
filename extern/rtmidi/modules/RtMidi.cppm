module;

#include "RtMidi.h"

export module rt.midi;

export namespace rt::midi {
    using rt::midi::RtMidiError;
    using rt::midi::RtMidiErrorCallback;
    using rt::midi::RtMidi;
    using rt::midi::RtMidiIn;
    using rt::midi::RtMidiOut;
    using rt::midi::MidiApi;
    using rt::midi::MidiInApi;
    using rt::midi::MidiOutApi;
}

#ifndef RTMIDI_USE_NAMESPACE
using namespace rt::midi;
#endif
