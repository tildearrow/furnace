## Portable File Dialogs documentation

The library can be used either as a [header-only library](https://en.wikipedia.org/wiki/Header-only),
or as a [single file library](https://github.com/nothings/single_file_libs).

### Use as header-only library

Just include the main header file wherever needed:

```cpp
#include "portable-file-dialogs.h"

/* ... */

    pfd::message::message("Hello", "This is a test");

/* ... */
```

### Use as a single-file library

Defining the `PFD_SKIP_IMPLEMENTATION` macro before including `portable-file-dialogs.h` will
skip all the implementation code and reduce compilation times. You still need to include the
header without the macro at least once, typically in a `pfd-impl.cpp` file.

```cpp
// In pfd-impl.cpp
#include "portable-file-dialogs.h"
```

```cpp
// In all other files
#define PFD_SKIP_IMPLEMENTATION 1
#include "portable-file-dialogs.h"
```

### General concepts

Dialogs inherit from `pfd::dialog` and are created by calling their class constructor. Their
destructor will block until the window is closed by user interaction. So for instance this
will block until the end of the line:

```cpp
pfd::message::message("Hi", "there");
```

Whereas this will only block until the end of the scope, allowing the program to perform
additional operations while the dialog is open:

```cpp
{
    auto m = pfd::message::message("Hi", "there");

    // ... perform asynchronous operations here
}
```

It is possible to call `bool pfd::dialog::ready(timeout)` on the dialog in order to query its
status and perform asynchronous operations as long as the user has not interacted:

```cpp
{
    auto m = pfd::message::message("Hi", "there");

    while (!m.ready())
    {
        // ... perform asynchronous operations here
    }
}
```

If necessary, a dialog can be forcibly closed using `bool pfd::dialog::kill()`. Note that this
may be confusing to the user and should only be used in very specific situations. It is also not
possible to close a Windows message box that provides no _Cancel_ button.

```cpp
{
    auto m = pfd::message::message("Hi", "there");

    while (!m.ready())
    {
        // ... perform asynchronous operations here

        if (too_much_time_has_passed())
            m.kill();
    }
}
```

Finally, the user response can be retrieved using `pfd::dialog::result()`. The return value of
this function depends on which dialog is being used. See their respective documentation for more
information:

  * [`pfd::message`](message.md) (message box)
  * [`pfd::notify`](notify.md) (notification)
  * [`pfd::open_file`](open_file.md) (file open)
  * [`pfd::save_file`](save_file.md) (file save)
  * [`pfd::select_folder`](select_folder.md) (folder selection)

### Settings

The library can be queried and configured through the `pfd::settings` class.

```cpp
bool pfd::settings::available();
void pfd::settings::verbose(bool value);
void pfd::settings::rescan();
```

The return value of `pfd::settings::available()` indicates whether a suitable dialog backend (such
as Zenity or KDialog on Linux) has been found. If not, the library will not work and all dialog
invocations will be no-ops. The program will not crash but you should account for this situation
and add a fallback mechanism or exit gracefully.

Calling `pfd::settings::rescan()` will force a rescan of available backends. This may change the
result of `pfd::settings::available()` if a backend was installed on the system in the meantime.
This is probably only useful for debugging purposes.

Calling `pfd::settings::verbose(true)` may help debug the library. It will output debug information
to `std::cout` about some operations being performed.
