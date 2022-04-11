## File Open API

The `pfd::save_file` class handles file saving dialogs. It can be provided a title, a starting
directory and/or pre-selected file, an optional filter for recognised file types, and an optional
flag to allow multiple selection:

```cpp
pfd::save_file::save_file(std::string const &title,
                          std::string const &initial_path,
                          std::vector<std::string> filters = { "All Files", "*" },
                          pfd::opt option = pfd::opt::none);
```

The `option` parameter can be `pfd::opt::force_overwrite` to disable a potential warning when
saving to an existing file.

The selected file is queried using `pfd::save_file::result()`. If the user canceled the
operation, the returned file name is empty:

```cpp
std::string pfd::save_file::result();
```

It is possible to ask the file save dialog whether the user took action using the
`pfd::message::ready()` method, with an optional `timeout` argument. If the user did not validate
the dialog within `timeout` milliseconds, the function will return `false`:

```cpp
bool pfd::save_file::ready(int timeout = pfd::default_wait_timeout);
```

## Example 1: simple file selection

Using `pfd::save_file::result()` will wait for user action before returning. This operation will
block and return the user choice:

```cpp
auto destination = pfd::save_file("Select a file").result();
if (!destination.empty())
    std::cout << "User selected file " << destination << "\n";
```

## Example 2: filters

The filter list enumerates filter names and corresponded space-separated wildcard lists. It
defaults to `{ "All Files", "*" }`, but here is how to use other options:

```cpp
auto destination = pfd::save_file("Select a file", ".",
                                  { "Image Files", "*.png *.jpg *.jpeg *.bmp",
                                    "Audio Files", "*.wav *.mp3",
                                    "All Files", "*" },
                                  pfd::opt::force_overwrite).result();
// Do something with destination
std::cout << "Selected file: " << destination << "\n";
```

## Example 3: asynchronous file save

Using `pfd::save_file::ready()` allows the application to perform other tasks while waiting for
user input:

```cpp
// File save dialog
auto dialog = pfd::save_file("Select file to save");

// Do something while waiting for user input
while (!dialog.ready(1000))
    std::cout << "Waited 1 second for user input...\n";

// Act depending on the user choice
std::cout << "User selected file: " << dialog.result() << "\n";
```
