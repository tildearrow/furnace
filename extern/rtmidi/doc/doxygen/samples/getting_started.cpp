#include "RtMidi.h"

int main() {
  try {
    RtMidiIn midiin;
  } catch (RtMidiError &error) {
    // Handle the exception here
    error.printMessage();
  }
  return 0;
}
