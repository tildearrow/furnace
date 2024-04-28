# modified version

this is a modified version of SAASound for Furnace.

it fixes some warnings and doesn't use its CMakeLists because I need to static-link it and disable config file support.
it also replaces long with int, to fix a discrepancy between MSVC and GCC/Clang.