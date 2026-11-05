# Contributing to RtMidi

RtMidi is a small library with a long life: it has provided one MIDI API across
many operating systems since 2003, and code written against it fifteen years ago
still compiles. This stability over time is valuable and must be preserved as
much as possible.

Linux, MacOS, and now Windows have all updated their backends to support MIDI 2.0.
While each of these backends still implements MIDI 1.0 messaging, and indeed the
MIDI 2.0 protocol provides a specific UMP transport for MIDI 1.0 messages, we
are already seeing changes in how these subsystems and their MIDI 1.0 translation
layers behave. RtMidi must adapt to these changes to continue support for MIDI 1.0,
add the new MIDI 2.0 UMP message format, and use this opportunity to fix longstanding
issues that are worth addressing.

---

## Guiding Principles

### Keep it supportable

RtMidi is one header and one source file, deliberately, so it can be dropped
into a project without a build system. Every backend lives behind `#if defined()`
guards in the same translation unit. The maintainers cannot test every backend,
and often cannot test *yours*.

So:

- Prefer the simple mechanism. A clever one costs someone else a weekend in 2031.
- Prefer code a reviewer can verify by reading. If correctness depends on
  hardware nobody else has, explain in the PR how you verified it and what you
  could not.
- Comment the *why*, not the *what*, wherever behaviour is surprising. Several
  places in RtMidi look wrong until you know which driver bug they work around.
- Do not add a dependency. Not a library, not a package manager, not a header
  outside the platform SDK.

### Keep the API consistent across subsystems

This one is difficult because the majority of contributors do not have the ability
to test on all backends, but it is also the core mission of RtMidi - a consistent
library across all platforms. An application that works on Linux and silently 
misbehaves on Windows is worse than one that fails everywhere.

If we change a public behaviour, it must change in **every** backend — ALSA,
CoreMIDI, WinMM, Windows MIDI Services, UWP, JACK, Web MIDI, Android AMidi and
Dummy — in the same commit. Not "the ones I can test." A backend you cannot
build still needs its implementation, even if that implementation is a
documented no-op.

Concretely, when adding or changing a method, ask:

- What does it do on a backend where the underlying concept does not exist?
  A no-op is often correct — but say so in the header comment.
- Does the return value or error behaviour mean the same thing everywhere?
  A function that returns a byte count on one backend and a failure code on
  another has two contracts, not one.
- Does it need a compile-time or runtime capability query so callers can ask
  rather than guess?

A platform-specific feature with no path to the other backends is not
automatically rejected — but say so honestly in the PR, and expect the reviewer
to ask what the other subsystems would do.

---

## Technical constraints

These are hard requirements. A patch that breaks one cannot be merged as-is.

**C++11.** The floor, for embedded and long-lived toolchains. No
`std::string_view`, no structured bindings, no `if constexpr`. 

**No variable-length arrays.** VLAs are a compiler extension, not standard C++,
and they break MSVC and warn under Clang's `-Wvla-cxx-extension`. Use
`std::vector`.

**One header, one source file.** `RtMidi.h` and `RtMidi.cpp`. New backends go
inside the existing guards; they do not get their own files.

**Both build systems.** CMake and Autotools must both work. If you touch the
build, touch both. Autotools on Windows means MinGW, which constrains what a
Windows backend can require.

**The C API tracks the C++ API.** `rtmidi_c.h` / `rtmidi_c.cpp` and the
`apinames` test must stay in step, including enum tables. Some C++ functions
have no C equivalent yet (`checkApiAvailability()`, `setDefaultErrorCallback()`,
the auto-reconnect functions and `setBufferSize()`); a new one should get one.

**Exceptions and callbacks.** Destructors and callback paths must not throw:
`closePort()` runs from destructors, and an escaping `RtMidiError` terminates the
process. Report those as warnings instead.

### ABI and versioning

RtMidi ships a versioned shared library. The SO version lives in `configure.ac`
as `lt_current:lt_revision:lt_age`, and `RtMidi.h` carries
`RTMIDI_VERSION_MAJOR/MINOR/PATCH`. The build checks that the two agree and will
fail if they do not — bump both or neither.

Any change to a public signature, virtual layout, or struct is an ABI break: it
needs `lt_current` incremented and the package major version raised. Adding a
non-virtual method to an existing class is usually safe; adding a virtual one is
not. If you are unsure, say so in the PR rather than guessing.

Releases are tagged on `master`. There are no version branches.

---

## How work is organised

**`master` is the only long-lived branch.** Development is trunk-based: cut a
short-lived branch, open a PR, merge, delete. Longer development efforts (i.e.
MIDI 2.0) can get a scoped/dedicated branch if necessary. It should be labelled
as unstable, pull from master often, and eventually be deprecated when merged 
into master.

**Milestones** group issues and PRs by release — for example a minor release for
backward-compatible work, a major one for an ABI break.

**Tracking issues** are how larger efforts are organised. A big objective —
MIDI 2.0 support, hotplug notification across backends, device metadata — gets
one issue holding the design discussion and a checklist linking the individual
issues and PRs beneath it. If you want to help with something large, find its
tracking issue and comment there first; several of these efforts span backends
and benefit enormously from someone who owns one platform.

**Labels** mark the backend a report concerns, and flag issues suitable for a
first contribution.

---

## Reporting an issue

Include:

- RtMidi version or commit, operating system and version, and the **backend**
  (ALSA, CoreMIDI, WinMM, Windows MIDI Services, UWP, JACK, Web MIDI,
  AMidi).
- How it was built — CMake or Autotools, compiler and version, shared or static.
- The MIDI hardware involved, if any. Device-specific behaviour is common, and
  the exact model matters more than you would expect.
- A minimal program that shows the problem. RtMidi has no shortage of reports
  that turned out to be the surrounding application.

If the problem is timing- or driver-related, say what you observed rather than
what you concluded. "The send returned success and the device received nothing"
is far more useful than "sendMessage is broken."

---

## Submitting a pull request

**One idea per PR.** A bug fix and a refactor in one branch cannot be reviewed,
and cannot be reverted independently when one half turns out to be wrong.

**Say what you verified, and what you did not.** This matters more here than in
most projects, because reviewers frequently cannot reproduce your platform. A PR
that says "tested on macOS 26 with an AKAI MPC2000; the WinMM path is unverified" is
more useful and more likely to be merged than one implying universal coverage.

**Include a control.** If you fixed a bug, show that the failure happens without
your change and stops with it, on the same tree. A test that passes identically
before and after has demonstrated nothing.

**Match the surrounding style.** Two-space indent, `Class :: method` spacing in
definitions, existing brace placement. Do not reformat code you are not changing;
whitespace churn hides the actual diff.

**Reference the issues you close.** `Fixes #123`, and be precise: if you fix part
of a report, say which part and what remains.

For anything large, open a tracking issue or comment on an existing one before
writing the code. It is much cheaper to disagree about design in an issue than in
a 600-line diff.

---

## Testing

The programs in `tests/` are the de facto test suite: `midiprobe` enumerates,
`midiout`/`midiin` and the callback variants exercise the basics, `apinames`
checks the API tables, and `testcapi` covers the C wrapper. `make check` runs
what can run without hardware.

Before submitting, at minimum:

- Build clean with warnings on — `-Wall -Wextra` for GCC/Clang, `/W4` for MSVC.
  Do not introduce new warnings.
- Run `apinames` and `testcapi`.
- Confirm the backends you did not touch still compile. A change inside one
  `#if defined()` block can still break another through a shared helper.

If you have hardware, say which devices and which operations — enumerate, open,
send, receive, SysEx, close, and reopen. Reopen after close is a frequent source
of real bugs and a frequent gap in testing.

---

## Getting help, and helping

RtMidi is maintained by volunteers and currently has no funded support. The most
valuable contribution is often not code: reproducing someone else's bug on a
platform you own, or confirming that a proposed change behaves correctly on your
hardware, unblocks work that would otherwise sit for months.

If you own a platform and are willing to test changes on it, say so on an issue.
That is genuinely scarce and genuinely welcome.
