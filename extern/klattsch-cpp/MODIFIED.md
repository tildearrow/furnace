# modification disclaimer

this is a trimmed snapshot of klattsch-cpp, vendored for the furnace build.

the upstream commit is recorded in `VERSION`. relative to upstream, the test
suite (`tests/`), fixture-generation tooling (`tools/`), documentation
(`docs/`) and JavaScript packaging manifests (`package.json`,
`package-lock.json`, `library.json`) have been removed, and the `tests`
subdirectory reference in `CMakeLists.txt` dropped, as none are used by the
furnace build.

`ja-hecko-2026`, which is present in the canonical JavaScript package but was
missing from this klattsch-cpp revision, has been added to the compiled phoneme
banks and registry. the text compiler is now optional in CMake so Furnace can
omit code it does not use; standalone builds still include it by default. the
CMake compatibility range has been lowered to 3.5...3.19, with the project
description omitted because that argument requires CMake 3.9. the synthesizer
and text compiler source are otherwise unmodified.
