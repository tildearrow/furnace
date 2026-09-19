# MODIFIED

this is a modified version of https://github.com/lubgr/lua-cmake which lowers the minimum CMake version and fixes some other things.

# Easily embed lua into applications managed with CMake

Due to its simplicity and portability, [Lua](https://www.lua.org) became a popular choice for
extending applications through a scripting language. The interpreter is available on most platforms,
however, depending on globally installed packages is not always reliable (outdated versions, static
vs. dynamic library etc.). As compile times of the interpreter are neglible, embedding it as a **git
submodule** can be preferrable. To simplify the Lua integration into `cmake`-managed projects, this
repository offers two simple facilities.

- The `CMakeLists.txt` contains instructions for building the interpreter as a library. It is
  intended to be used via `cmake`'s `add_subdirectory` command, and sets up the `lua::lib` target
  that can be linked against. Public headers are taken care of and are propagated as a usage
  requirement.
- A git clone of the upstream [sources](https://github.com/lua/lua) is configured as a submodule,
  such that recursive initialization of this repository pulls them in.

The Lua version pulled in is the current v5.4.1. The library is built as C (not C++, which is
possible), with default upstream compiler flags. Whether a shared or static library is built depends
on `cmake`'s
[`BUILD_SHARED_LIBS`](https://cmake.org/cmake/help/latest/variable/BUILD_SHARED_LIBS.html) flag. No
`install` target is configured and no standalone executable is built, given the intended use case.

Here's how to use it:
```bash
cd path/to/your/project

git submodule add https://github.com/lubgr/lua-cmake external/lua

# Necessary to automatically pull the upstream Lua sources:
git submodule update --init --recursive external/lua
```
In the `CMakeLists.txt` of your application, add
```
add_subdirectory(external/lua)

target_link_libraries(yourTarget PRIVATE lua::lib)
```
That's it. Further integration with a library to facilitate the bindings (e.g.
[sol2](https://github.com/ThePhD/sol2)) is straightfoward.
