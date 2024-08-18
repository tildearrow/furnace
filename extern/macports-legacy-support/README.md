# MODIFIED

this is a modified version of macports-legacy-support which removes APSL-licensed files.

# MacPorts Support for Legacy OSX Versions

Installs wrapper headers and library functions that add common
functions missing in various older OSX releases to bring them
approximately up to current expected standards.

Three different libraries are provided

 - libMacportsLegacySupport.a      - A static library with the missing functions for the given OS.
 - libMacportsLegacySupport.dylib  - A dynamic library with the missing functions for the given OS.
 - libMacportsLegacySystem.B.dylib - Similar to libMacportsLegacySupport.dylib but in addition re-exports the symbols from libSystem.B.dylib.

To use this library within [MacPorts](https://github.com/macports)
add the `legacysupport` PortGroup to the Portfile. This will add the
required include paths and libraries to allow the library to do it's
magic with most build systems.

Wrapped headers and replaced functions are:

<table>
  <tr>
    <th>Header File</th>
    <th>Feature</th>
    <th>Max Version Needing Feature</th>
  </tr>
  <tr>
    <td><code>assert.h</code></td>
    <td>Adds C11 <code>static_assert</code> definition</td>
    <td>OSX10.10</td>
  </tr>
  <tr>
    <td><code>cmath</code></td>
    <td>Adds the same functions as those provided by the herein <code>math.h</code>,
        in namespace <code>std::</code>.</td>
    <td>see <code>math.h</code></td>
  </tr>
  <tr>
    <td><code>copyfile.h</code></td>
    <td>Wraps <code>copyfile_state_get</code> to support <code>COPYFILE_STATE_COPIED</code></td>
    <td>OSX10.5</td>
  </tr>
  <tr>
    <td><code>dirent.h</code></td>
    <td>Adds <code>fdopendir</code> function.
    <td>OSX10.9</td>
  </tr>
  <tr>
    <td><code>os/lock.h</code></td>
    <td>Adds <code>os_unfair_lock_lock</code>, <code>os_unfair_lock_trylock</code>, and <code>os_unfair_lock_unlock</code> functions</td>
    <td>OSX10.11</td>
  </tr>
  <tr>
    <td><code>math.h</code></td>
    <td>Adds declaration of various <code>long long</code> methods (OSX10.6) and <code>__sincos</code> (macOS10.8)</td>
    <td>OSX10.6(8), GCC 8</td>
  </tr>
  <tr>
    <td><code>netdb.h</code></td>
    <td>Adds declaration of <code>AI_NUMERICSERV</code></td>
    <td>OSX10.5</td>
  </tr>
  <tr>
    <td rowspan="2"><code>pthread.h</code></td>
    <td>Adds <code>PTHREAD_RWLOCK_INITIALIZER</code></td>
    <td>OSX10.4</td>
  </tr>
  <tr>
    <td>Adds <code>pthread_setname_np</code> function</td>
    <td>OSX10.5</td>
  </tr>
  <tr>
    <td><code>stdio.h</code></td>
    <td>Adds <code>dprintf</code>, <code>getline</code>, <code>getdelim</code>,
        <code>open_memstream</code>, and <code>fmemopen</code> functions</td>
    <td>OSX10.6, OSX10.12 (open_memstream)</td>
  </tr>
  <tr>
    <td rowspan="2"><code>stdlib.h</code></td>
    <td>Adds <code>posix_memalign</code> functional replacement, and wraps <code>realpath</code>
        to accept a <code>NULL</code> buffer argument</td>
    <td>OSX10.5</td>
  </tr>
  <tr>
    <td>Adds <code>arc4random_uniform</code> and <code>arc4random_buf</code> functions</td>
    <td>OSX10.6</td>
  </tr>
  <tr>
    <td><code>string.h</code></td>
    <td>Adds <code>stpncpy</code>, <code>strnlen</code>, <code>strndup</code> and <code>memmem</code> functions</td>
    <td>OSX10.6</td>
  </tr>
  <tr>
    <td><code>strings.h</code></td>
    <td>Adds <code>fls,flsl,ffsl</code>(OSX10.4) and <code>flsll,ffsll</code>(macOS10.8) functions</td>
    <td>OSX10.4(8)</td>
  </tr>
  <tr>
    <td><code>time.h</code></td>
    <td>Adds functions <code>clock_gettime</code>(macOS10.11) and <code>timespec_get</code>(macOS10.14). Defines <code>TIME_UTC</code> (macOS10.14). Declares <code>asctime_r</code>, <code>ctime_r</code>, <code>gmtime_r</code>, and <code>localtime_r</code> functions that are otherwise hidden in the presence of <code>_ANSI_SOURCE</code>, <code>_POSIX_C_SOURCE</code>, or <code>_XOPEN_SOURCE</code> (OSX10.4)</td>
    <td>OSX10.4(11,14)</td>
  </tr>
  <tr>
    <td><code>wchar.h</code></td>
    <td>Adds <code>wcsdup</code>, <code>wcsnlen</code>, <code>wcpcpy</code>,
        <code>wcpncpy</code>, <code>wcscasecmp</code>, and <code>wcsncasecmp</code>
        functions</td>
    <td>OSX10.6</td>
  </tr>
  <tr>
    <td><code>mach/machine.h</code></td>
    <td>Adds missing machine definitions</td>
    <td>OSX10.13</td>
  </tr>
  <tr>
    <td><code>net/if.h</code></td>
    <td>Adds include <code>sys/socket.h</code>, expected on current macOS systems</td>
    <td>OSX10.8</td>
  </tr>
  <tr>
    <td><code>xlocale/_wchar.h</code></td>
    <td>Adds <code>wcscasecmp_l</code>, <code>wcsncasecmp_l</code> functions</td>
    <td>OSX10.6</td>
  </tr>
  <tr>
    <td><code>sys/aio.h</code></td>
    <td>Adjusts includes and defines to match SDK 10.5+</td>
    <td>OSX10.4</td>
  </tr>
  <tr>
    <td rowspan="2"><code>sys/fcntl.h</code></td>
    <td>Adds missing <code>O_CLOEXEC</code>, <code>AT_FDCWD</code>, <code>AT_EACCESS</code>,
        <code>AT_SYMLINK_NOFOLLOW</code>, <code>AT_SYMLINK_FOLLOW</code>, and
        <code>AT_REMOVEDIR</code> definitions</td>
    <td>as required (?)</td>
  </tr>
  <tr>
    <td>Adds <code>openat</code> function</td>
    <td>OSX10.9</td>
  </tr>
  <tr>
    <td><code>sys/fsgetpath.h</code></td>
    <td>Adds missing <code>utimensat</code>, <code>fsgetpath</code> and <code>setattrlistat</code> functions</td>
    <td>OSX10.12</td>
  </tr>
  <tr>
    <td><code>sys/mman.h</code></td>
    <td>Adds missing <code>MAP_ANONYMOUS</code> definition</td>
    <td>OSX10.10</td>
  </tr>
  <tr>
    <td><code>sys/stdio.h</code></td>
    <td>Adds <code>renameat</code> function</td>
    <td>OSX10.9</td>
  </tr>
  <tr>
    <td rowspan="2"><code>sys/stat.h</code></td>
    <td>Adds <code>fchmodat</code>, <code>fstatat</code>, <code>fstatat64</code> (if required, and on 10.5+),
        and <code>mkdirat</code> functions</td>
    <td>OSX10.9</td>
  </tr>
  <tr>
    <td>Adds <code>lchmod</code> function</td>
    <td>OSX10.4</td>
  </tr>
  <tr>
    <td><code>sys/random.h</code></td>
    <td>Adds <code>getentropy</code> function</td>
    <td>OSX10.11</td>
  </tr>
  <tr>
    <td><code>sys/socket.h</code></td>
    <td>Corrects <code>CMSG_DATA</code> definition</td>
    <td>OSX10.5</td>
  </tr>
  <tr>
    <td><code>sys/time.h</code></td>
    <td>Adds <code>lutimes</code> function</td>
    <td>OSX10.4</td>
  </tr>
  <tr>
    <td><code>sys/types.h</code></td>
    <td>Adds definitions for <code>u_char</code>, <code>u_short</code>, <code>u_int</code>, <code>u_long</code>, <code>ushort</code>, and <code>uint</code> types that can be exposed via <code>_DARWIN_C_SOURCE</code></td>
    <td>OSX10.4</td>
  </tr>
  <tr>
    <td rowspan="3"><code>sys/unistd.h</code></td>
    <td>Adds <code>getattrlistat</code>, <code>readlinkat</code>, <code>faccessat</code>,
        <code>fchownat</code>, <code>linkat</code>, <code>symlinkat</code>,
        and <code>unlinkat</code> functions</td>
    <td>OSX10.9</td>
  </tr>
  <tr>
    <td>Wraps <code>sysconf</code> to support <code>_SC_NPROCESSORS_CONF</code> and
        <code>_SC_NPROCESSORS_ONLN</code></td>
    <td>OSX10.4</td>
  </tr>
  <tr>
    <td>Wraps <code>sysconf</code> to support <code>_SC_PHYS_PAGES</code></td>
    <td>OSX10.10</td>
  </tr>
  <tr>
    <td><code>uuid/uuid.h</code></td>
    <td>Adds typedef of <code>uuid_string_t</code></td>
    <td>OSX10.5</td>
  </tr>
  <tr>
    <td><code>CoreFoundation/CoreFoundation.h</code></td>
    <td>Adds <code>CFPropertyListCreateWithStream</code> function</td>
    <td>OSX10.5</td>
  </tr>
  <tr>
    <td><code>OpenGL/gliDispatch.h</code></td>
    <td>Wraps <code>gliDispatch.h</code> to prevent including
        <code>glext.h</code> and thereby match behaviour of newer systems.</td>
    <td>OSX10.6</td>
  </tr>
  <tr>
    <td><code>TargetConditionals.h</code></td>
    <td>Adds definitions for <code>TARGET_CPU_ARM</code>, <code>TARGET_CPU_ARM64</code>,
        <code>TARGET_OS_SIMULATOR</code>, <code>TARGET_OS_IOS</code>, <code>TARGET_OS_TV</code>,
        <code>TARGET_OS_WATCH</code> and <code>TARGET_OS_OSX</code> if needed.</td>
    <td>OSX10.10</td>
  </tr>
  <tr>
    <td><code>-</code></td>
    <td>Adds <code>__bzero</code> library symbol</td>
    <td>OSX10.5</td>
  </tr>
  <tr>
    <td><code>-</code></td>
    <td>Adds <code>_dirfd</code> library symbol</td>
    <td>OSX10.7</td>
  </tr>
  <tr>
    <td><code>-</code></td>
    <td>Adds <code>_fstatat$INODE64</code> library symbol</td>
    <td>OSX10.9</td>
  </tr>
</table>

For information on building this library outside MacPorts, see BUILDING.txt.
