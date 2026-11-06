# Example iOS app

A minimal SwiftUI app demonstrating RtMidi on iOS. Lists MIDI ports, sends
Note On/Off, SysEx, and an Identity Request, and shows received MIDI in a
simple monitor with basic Identity Reply decoding.

At launch it also opens two virtual ports, "RtMidi In" and "RtMidi Out",
and passes everything that arrives on "RtMidi In" straight out "RtMidi Out".
Other apps see them like any other port, so the example works as a MIDI thru
for testing. Its own monitor and buttons only use them if you pick them in
the dropdowns.

Creating a virtual port needs the `audio` background mode. Without it,
`openVirtualPort()` fails with CoreMIDI's `kMIDINotPermitted` (-10844), so
the app's `Info.plist` sets `UIBackgroundModes` to `audio` (an
`INFOPLIST_KEY_` build setting can't, since it only generates strings).

To build RtMidi for iOS in your own CMake project, set the system name and
architecture; CMake then picks the iPhoneOS SDK:

    cmake -B build -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_ARCHITECTURES=arm64

`RtMidiExample/RtMidi.cpp` and `RtMidi.h` are symlinks to the files at the
root of the repository.

Generate the Xcode project with [XcodeGen](https://github.com/yonaskolb/XcodeGen):

```sh
xcodegen generate
open RtMidiExample.xcodeproj
```
