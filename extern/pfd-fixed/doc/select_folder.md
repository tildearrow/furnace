## Folder Selection API

The `pfd::select_folder` class handles folder opening dialogs. It can be provided a title, and an
optional starting directory:

```cpp
pfd::select_folder::select_folder(std::string const &title,
                                  std::string const &default_path = "",
                                  pfd::opt option = pfd::opt::none);
```

The `option` parameter can be `pfd::opt::force_path` to force the operating system to use the
provided path. Some systems default to the most recently used path, if applicable.

The selected folder is queried using `pfd::select_folder::result()`. If the user canceled the
operation, the returned string is empty:

```cpp
std::string pfd::select_folder::result();
```

It is possible to ask the folder selection dialog whether the user took action using the
`pfd::message::ready()` method, with an optional `timeout` argument. If the user did not validate
the dialog within `timeout` milliseconds, the function will return `false`:

```cpp
bool pfd::select_folder::ready(int timeout = pfd::default_wait_timeout);
```

## Example 1: simple folder selection

Using `pfd::select_folder::result()` will wait for user action before returning. This operation
will block and return the user choice:

```cpp
auto selection = pfd::select_folder("Select a folder").result();
if (!selection.empty())
    std::cout << "User selected folder " << selection << "\n";
```

## Example 2: asynchronous folder open

Using `pfd::select_folder::ready()` allows the application to perform other tasks while waiting for user input:

```cpp
// Folder selection dialog
auto dialog = pfd::select_folder("Select folder to open");

// Do something while waiting for user input
while (!dialog.ready(1000))
    std::cout << "Waited 1 second for user input...\n";

// Act depending on the user choice
std::cout << "Selected folder: " << dialog.result() << "\n";
```
