# ALSA automatic reconnection

The ALSA backend restores an explicitly opened USB MIDI connection after device
removal. Both input and output use one shared announcement client and monitoring
thread per loaded RtMidi library. Normal MIDI delivery does not acquire the
monitor's registry lock or wait for its polling interval.

`supportsAutoReconnect()` reports backend capability, not whether a particular
port has a usable identity. Recovery defaults on. `setAutoReconnect(false)`
disables reconnection without breaking an existing subscription; enabling it
again also checks for a device lost while disabled. `closePort()` cancels recovery.
As before, serialize open, close, and destruction on each instance. The control
API can wait for an identity scan; it is not a realtime callback API.

## Identity and limitations

- Requires Linux ALSA headers/library 1.1.1 or newer and readable USB sysfs data.
- Matches vendor/product, serial number, and port number/name. Without a serial,
  the USB location is used instead: another identical unit in that socket cannot
  be distinguished. Ambiguous identities are not automatically reconnected.
- A different ALSA client address is supported. Enumeration is still a snapshot;
  applications explicitly reopening a port must enumerate again.
- Software/virtual and unidentified ports retain their previous behavior.
- `isPortOpen()` describes the application's open intent, including absence.
  Lost notes and controller messages are not replayed; applications may need
  their usual panic/reset controls after unplugging while notes are held.
- Ordinary patchbay unsubscribes alone do not trigger reconnection. After lost
  announcements or monitor restart, reconciliation restores open intent; it
  cannot distinguish a missed unplug from a manual unsubscribe.
- Background ALSA failures are reported to `std::cerr` and retried. Persistent
  pipe/thread startup failures use the caller's normal RtMidi error handler;
  another enable/open operation can retry starting the worker.
- Full ALSA sequencer teardown, fork-without-exec after MIDI initialization, and
  application callbacks that destroy their own RtMidi instance are outside this
  change's guarantees.

## Regression test

ALSA CMake and Autotools builds compile and run `alsahotplug` automatically.
Cross-compilation builds the test but does not run it. A native run needs access
to `/dev/snd/seq`; no physical MIDI hardware or root access is required once that
device is available. Exit 77 explicitly reports unavailable ALSA or unsupported
ALSA versions, rather than treating the integration cases as passed. The dummy
backend's capability and enable/disable behavior are checked before that skip.

The test creates only its own virtual ports and temporary identity files. Linker
wrappers supply USB identities and inject failures; they are linked into the
test executable, never the library. Concurrent runs take an advisory lock on the
sequencer device to stabilize client-number reuse. Runs are bounded to 30 seconds.

Coverage includes input/output delivery, changed/reused addresses, wrong and
ambiguous identities, runtime disable/enable, close/destruction, concurrent port
lifecycle operations, one shared worker, announcement overflow, repeated monitor
and subscription failures, and startup error callbacks that close the port.

Run `ctest --test-dir build -R rtmidi_alsahotplug --output-on-failure` for CMake,
or `make check TESTS=alsahotplug` in the Autotools build's `tests` directory.
The source header also gives a direct compiler command suitable for adding
`-fsanitize=address,undefined` or `-fsanitize=thread` during review.

Before a release, also unplug/replug real USB hardware while an input callback
and output sender are active. Exercise multiple ports, disabled recovery, and
closing while absent. Synthetic identity tests do not replace that hardware test.
