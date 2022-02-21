
#include <stdio.h>
#include "rtmidi_c.h"

/* Test that the C API for RtMidi is working. */

struct RtMidiWrapper *midiin;
struct RtMidiWrapper *midiout;

int main() {
    if ((midiin = rtmidi_in_create_default())) {
        unsigned int ports = rtmidi_get_port_count(midiin);
        printf("MIDI input ports found: %u\n", ports);
        rtmidi_close_port(midiin);
        rtmidi_in_free(midiin);
    }

    if ((midiout = rtmidi_out_create_default())) {
        unsigned int ports = rtmidi_get_port_count(midiout);
        printf("MIDI output ports found: %u\n", ports);
        rtmidi_close_port(midiout);
        rtmidi_out_free(midiout);
    }

    return 0;
}
