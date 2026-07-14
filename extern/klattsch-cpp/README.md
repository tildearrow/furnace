# klattsch-cpp

C++ port of [klattsch](https://github.com/tgies/klattsch), a parallel-formant
(Klatt 1980) speech synthesizer.

## API

```cpp
#include <klattsch/klattsch.hpp>

klattsch::Synth synth(48000);
klattsch::ParamUpdate update;
update.set(klattsch::ParamId::F0, 220.0f);
update.set(klattsch::ParamId::Voicing, 1.0f);
synth.setTarget(update, 1440);  // 30 ms @ 48 kHz

float buffer[1024] = {0};
synth.process(buffer, 1024);
```

A C facade is available for FFI consumers:

```c
#include <klattsch/klattsch_c.h>

klattsch_synth* s = klattsch_synth_new(48000, 0);
klattsch_synth_process(s, buffer, 1024);
klattsch_synth_free(s);
```

### Compiling a phoneme string

```cpp
#include <klattsch/compile.hpp>
#include <klattsch/banks.hpp>

klattsch::CompileOptions opts;
opts.baseF0 = 120.0f;

klattsch::CompileResult result =
    klattsch::compileString("HH AH L OW", opts,
                            klattsch::builtInBanks(), 48000);

// result.events is a vector of ScheduleEvent ready for Synth
// result.totalSamples is the total render length
```

### Phoneme banks

```cpp
const klattsch::BankRegistry& banks = klattsch::builtInBanks();

// List registered banks
for (const auto& name : banks.list()) { /* ... */ }

// Look up a specific bank
const klattsch::PhonemeBank* bank = banks.get("ja-mokhtari-2000");
```

### Borrowing a pre-sorted schedule (no allocation)

```cpp
// When events are already sorted and owned by the caller:
synth.setScheduleView(events.data(), events.size());
```

This avoids the copy+sort that `setSchedule()` performs internally.

## Building

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

Requires a C++14 compiler. No external dependencies.

## Vendoring

`klattsch-cpp` is meant to be vendored into consumers as a snapshot.

Snapshot recipe:

```sh
rsync -av --delete \
  --exclude .git --exclude build --exclude node_modules \
  ../klattsch-cpp/ lib/klattsch-cpp/
( cd ../klattsch-cpp && git rev-parse HEAD ) > lib/klattsch-cpp/VERSION
```

From the host project's `CMakeLists.txt`:

```cmake
add_subdirectory(lib/klattsch-cpp)
target_link_libraries(my_target PRIVATE klattsch::klattsch)
```

## Versioning

Independent SemVer track, not mirrored to the JS `klattsch` package. Parity
tested against canonical JS sample fixtures within a fixed per-sample
tolerance.

## License

MIT.
