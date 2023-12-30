# Things we need help with

- Editing the release build scripts (`scripts/release-*.sh`) so they can be launched in MinGW (or similar; must be runnable in MSYS2 shell on Windows and Linux, at least) environment and not obscure Arch-specific one they work only in right now
  - Bonus points if they automatically install all the required tools (e.g. use Flipper Zero's `fbt` toolchain as a reference)
  - Even more bonus points for the script that automatically builds all the releases for all operating systems
  - **EVEN MORE** bonus points if you add Android builds there
- Adding proper `clang-format`: the settings file so contributors can run it locally, and add check to build workflow so monolithic code formatting style is enforced in Furnace code (does not affect submodules)
- Refactoring the code, more specifically, making it better structured and separated. E.g. separating `drawInsEdit()` contents into separate instrument-specific functions, the same goes with file import routines. It is very tedious job but it makes future developers' lifes easier (especially for the ones that add new chips).
