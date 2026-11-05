# Building and Using RtMidi on Windows

## Requirements

Before building, ensure the following are installed via the **Visual Studio Installer**:

| Component | Notes |
|---|---|
| **Visual Studio 2019 or newer** | Visual Studio 2019 or newer will work |
| **Desktop development with C++** | Core workload required for the build |
| **Windows SDK 22000 or newer** | Required; select version 10.0.22000.0 or later in the Visual Studio Installer |

---


## Building RtMidi

The Visual Studio solution is located at:

```
tests/RtMidi.sln
```

1. Open `tests/RtMidi.sln` in Visual Studio.
2. Select the desired configuration and platform from the toolbar:
   - **Release | x64** (recommended for production use)
   - **Debug | x64** (recommended during development)
3. Build the solution (**Build → Build Solution** or <kbd>Ctrl+Shift+B</kbd>).

After a successful build, the compiled static library will be at:

| Configuration | Output path |
|---|---|
| Release \| x64 | `tests/x64/Release/rtmidilib.lib` |
| Debug \| x64 | `tests/x64/Debug/rtmidilib.lib` |

The RtMidi library build is now complete.

---

## Using RtMidi in Your Own Project

Building RtMidi and linking it into your own project are two separate steps. Once you have the library built, you need two files from this repository to use RtMidi in a personal C++ project:

| File | Purpose |
|---|---|
| `RtMidi.h` | Header — add its containing directory as an include path |
| `tests/x64/Release/rtmidilib.lib` or `tests/x64/Debug/rtmidilib.lib` | Static library matching your chosen configuration — link it into your project |

### Visual Studio Project Configuration

Open your project's **Property Pages** (**Project → Properties**) and set the following:

| Property Page | Setting | Value |
|---|---|---|
| **C/C++ → General** | Additional Include Directories | Path to the directory containing `RtMidi.h` (the repository root) |
| **Linker → General** | Additional Library Directories | `tests/x64/$(Configuration)` |
| **Linker → Input** | Additional Dependencies | `rtmidilib.lib; winmm.lib; windowsapp.lib` |

> [!IMPORTANT]
> - `$(Configuration)` is a Visual Studio macro that resolves automatically to `Debug` or `Release` to match the active build configuration.
> - Your consuming application must target **x64** to match the library architecture.
> - The application and the library must use the **same configuration**. Linking a Debug application against the Release `rtmidilib.lib` (or vice versa) will cause an **LNK2038** runtime-library mismatch error.

---

### Microsoft GS Wavetable Synth

With the Multimedia Library (WinMM) backend, opening the Microsoft GS
Wavetable Synth can fail unless the thread that uses RtMidi initializes COM
first: call `CoInitializeEx` on that thread before using RtMidi, and
`CoUninitialize` when it is done ([#77](https://github.com/thestk/rtmidi/issues/77)).

---

## Example Usage

The following minimal program verifies that your project can include and link against RtMidi successfully:

```cpp
#include <iostream>
#include "RtMidi.h"

using namespace std;

int main()
{
    RtMidiOut midiout;
    cout << "MIDI output ports: " << midiout.getPortCount() << endl;
    return 0;
}
```

Build this in your project. If it compiles and links without errors, RtMidi is correctly set up.

---

## Windows MIDI Services

RtMidi 6.1.0 adds a backend for Windows MIDI Services, Microsoft's new MIDI
stack, through its `Windows.Devices.Midi2` API. It gives your app MIDI 1.0
devices plus everything the new service adds, such as loopback endpoints, and
it can be built alongside the older Multimedia Library (WinMM) backend, so your
app uses Windows MIDI Services where it works and WinMM everywhere else.

### Where Windows MIDI Services stands

- The MIDI service is part of Windows 11 25H2 and later. The
  `Windows.Devices.Midi2` API that apps use is not part of Windows yet:
  Microsoft has said it will be added to Windows 11 25H2 and later in a Windows
  update in November 2026.
- Until then, the API comes from Microsoft's preview releases on GitHub. The
  current one is **In-box Preview 11** (`0.99.92-preview.11`, October 2026),
  which locks the API: Microsoft no longer allows breaking changes to it.
- Windows 11 24H2 and earlier will not get the API. On those systems RtMidi
  uses WinMM.

Microsoft's resources:

- [microsoft/MIDI on GitHub](https://github.com/microsoft/MIDI): source, issues
  and [releases](https://github.com/microsoft/MIDI/releases), where each preview
  is published with release notes and downloads
- [Windows MIDI Services documentation](https://microsoft.github.io/MIDI/),
  including [porting a MIDI library](https://microsoft.github.io/MIDI/kb/porting-midi-libraries/),
  [moving from WinMM](https://microsoft.github.io/MIDI/kb/moving-from-winmm-to-wms/),
  [switching between the new and legacy MIDI stacks](https://microsoft.github.io/MIDI/kb/how-to-change-api-mode/)
  and the [tools](https://microsoft.github.io/MIDI/tools/)
- [The Windows MIDI Services Discord](https://aka.ms/mididiscord), where the
  Microsoft developers answer questions
- Code samples in the repository's [samples](https://github.com/microsoft/MIDI/tree/main/samples) folder

### Building RtMidi with Windows MIDI Services

The backend is built with CMake only; the Visual Studio solution above builds
WinMM and UWP. You need:

- Visual Studio 2022 or later with **Desktop development with C++** (tested with
  Visual Studio 2026). The backend uses C++/WinRT and compiles as C++20.
- A Windows SDK that includes C++/WinRT, 10.0.22000 or later (tested with
  10.0.26100).
- CMake 3.24 or later. Check with `cmake --version`: an older CMake earlier on
  your PATH is used even in a Visual Studio command prompt.
- The `Windows.Devices.Midi2` NuGet package from Microsoft's release page,
  until Windows and its SDK include the API.
- A 64-bit app (x64 or ARM64). Microsoft's package has no 32-bit (x86) build,
  so a 32-bit app can only use WinMM.

1. From the [current preview release](https://github.com/microsoft/MIDI/releases),
   download `Windows.Devices.Midi2.<version>.nupkg` (for Preview 11,
   `Windows.Devices.Midi2.0.99.92-preview.11.nupkg`).
2. Unpack it. A `.nupkg` is a zip file; in cmd or PowerShell, Windows' own
   `tar` reads it (the `tar` in Git Bash can't):

   ```
   mkdir wms
   tar -xf Windows.Devices.Midi2.0.99.92-preview.11.nupkg -C wms
   ```

3. Configure and build, pointing CMake at the package's `ref/native` folder.
   WinMM is on by default on Windows, so this builds both backends:

   ```
   cmake -B build -A x64 -DRTMIDI_API_WMS=ON -DWindowsMidiServices_ROOT=C:/path/to/wms/ref/native
   cmake --build build --config Release
   ```

   `WindowsMidiServices_ROOT` can also be set as an environment variable.
   CMake generates the C++/WinRT headers for the API from the package's
   `Windows.Devices.Midi2.winmd`. This builds RtMidi as a DLL,
   `build\Release\rtmidi.dll`, which your executable needs next to it; add
   `-DBUILD_SHARED_LIBS=OFF` to link RtMidi statically instead.

4. Until Windows includes the API, your app has to carry its own copy: copy
   `Windows.Devices.Midi2.dll` and `Windows.Devices.Midi2.pri` from the
   package's `runtimes/win-x64/native` folder (or `win-arm64`) next to your
   executable. Without them, RtMidi reports Windows MIDI Services as not
   available and falls back to WinMM. On a PC whose Windows has the API,
   Windows uses its own copy instead of yours.

### Falling back to WinMM in your app

With both backends built in, an `RtMidiIn` or `RtMidiOut` created with no API
argument tries Windows MIDI Services first, and uses WinMM when Windows MIDI
Services can't run (no API on this Windows or in your app, the MIDI service not
running, or Windows set to its legacy MIDI stack), or when it has no ports and
WinMM does. You don't need any code for that.

To choose explicitly and tell the user why, ask first:

```cpp
RtMidi::Api api = RtMidi::WINDOWS_MM;
RtMidi::RtMidiApiAvailability wms = RtMidi::checkApiAvailability( RtMidi::WINDOWS_MIDI_SERVICES );
if ( wms.available )
  api = RtMidi::WINDOWS_MIDI_SERVICES;
else
  std::cout << "Using the Multimedia Library: " << wms.message << std::endl;
RtMidiIn midiin( api );
```

The `midiprobe` test program prints each compiled API's availability and the
API RtMidi picks with no argument, which is a quick way to check a PC.

One change to expect from Windows itself: with the October 2026 update to
Windows 11 25H2 (build 26200.9711), when an app abandons a SysEx (sends F0 and
data but no F7) and then sends another message through WinMM or the older
Windows.Devices.Midi (UWP) API, the Windows MIDI service closes the SysEx with
an F7 and delivers it as if it were complete. On build 26200.9457 it dropped
the messages that followed instead. Sending through Windows MIDI Services
doesn't do this. Receivers can't tell a cut-off SysEx from a whole one, so
check SysEx contents (a length or checksum) where it matters.

### Shipping an app before the November update

Shipping the preview's `Windows.Devices.Midi2.dll` with your app is allowed
under Microsoft's "go live" terms, which are in each preview's
[release notes](https://github.com/microsoft/MIDI/releases/tag/inbox-preview-11).
In short, you agree to:

- update your app to newer previews, and remove the DLL once Windows includes
  the API
- tell your users that your app uses a preview Windows MIDI Services API
- not ship the API to Windows older than Windows 11 25H2
- report problems on Microsoft's GitHub

If those terms don't suit you, ship with WinMM only (leave out
`-DRTMIDI_API_WMS=ON`) until the update, then add Windows MIDI Services.

### What to tell your users

Now, while the API is in preview:

- Windows MIDI Services needs Windows 11 25H2 or later, kept up to date. On
  older Windows the app uses the classic Windows MIDI support instead.
- If the app says Windows is set to its legacy MIDI stack, Microsoft explains
  how to switch in
  [How to change API mode](https://microsoft.github.io/MIDI/kb/how-to-change-api-mode/).
- Optionally, Microsoft's preview **Tools** package adds MIDI Settings, a
  monitor and troubleshooting tools, and the **Transports** package adds
  loopback, network and Bluetooth MIDI. Both are on the
  [release page](https://github.com/microsoft/MIDI/releases); Microsoft asks
  users to uninstall older previews from Settings > Apps first.

After the November 2026 Windows update:

- Users on Windows 11 25H2 or later get the API through Windows Update; there
  is nothing extra to install. Your app then uses Windows' copy of the API
  automatically, and your next release should drop the bundled DLL.

For help with Windows MIDI Services itself, point users to the
[Discord](https://aka.ms/mididiscord) or
[Microsoft's issue tracker](https://github.com/microsoft/MIDI/issues); for
RtMidi, to [RtMidi's issues](https://github.com/thestk/rtmidi/issues).
