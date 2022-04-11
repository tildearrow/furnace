## File Open API

The `pfd::open_file` class handles file opening dialogs. It can be provided a title, a starting
directory and/or pre-selected file, an optional filter for recognised file types, and an optional
flag to allow multiple selection:

```cpp
pfd::open_file::open_file(std::string const &title,
                          std::string const &initial_path,
                          std::vector<std::string> filters = { "All Files", "*" },
                          pfd::opt option = pfd::opt::none);
```

The `option` parameter can be `pfd::opt::multiselect` to allow selecting multiple files.

The selected files are queried using `pfd::open_file::result()`. If the user canceled the
operation, the returned list is empty:

```cpp
std::vector<std::string> pfd::open_file::result();
```

It is possible to ask the file open dialog whether the user took action using the
`pfd::message::ready()` method, with an optional `timeout` argument. If the user did not validate
the dialog within `timeout` milliseconds, the function will return `false`:

```cpp
bool pfd::open_file::ready(int timeout = pfd::default_wait_timeout);
```

## Example 1: simple file selection

Using `pfd::open_file::result()` will wait for user action before returning. This operation will
block and return the user choice:

```cpp
auto selection = pfd::open_file("Select a file").result();
if (!selection.empty())
    std::cout << "User selected file " << selection[0] << "\n";
```

## Example 2: filters

The filter list enumerates filter names and corresponded space-separated wildcard lists. It
defaults to `{ "All Files", "*" }`, but here is how to use other options:

```cpp
auto selection = pfd::open_file("Select a file", ".",
                                { "Image Files", "*.png *.jpg *.jpeg *.bmp",
                                  "Audio Files", "*.wav *.mp3",
                                  "All Files", "*" },
                                pfd::opt::multiselect).result();
// Do something with selection
for (auto const &filename : dialog.result())
    std::cout << "Selected file: " << filename << "\n";
```

## Example 3: asynchronous file open

Using `pfd::open_file::ready()` allows the application to perform other tasks while waiting for
user input:

```cpp
// File open dialog
auto dialog = pfd::open_file("Select file to open");

// Do something while waiting for user input
while (!dialog.ready(1000))
    std::cout << "Waited 1 second for user input...\n";

// Act depending on the user choice
std::cout << "Number of selected files: " << dialog.result().size() << "\n";
```

## Screenshots

Windows 10:
![open-win32](https://user-images.githubusercontent.com/245089/47155865-0f8cd900-d2e6-11e8-8041-1e20b6f77dee.png)

Mac OS X (dark theme):
![image](https://user-images.githubusercontent.com/245089/56053378-39363280-5d54-11e9-9583-9f1c978fa0db.png)

Mac OS X (light theme):
![image](https://user-images.githubusercontent.com/245089/56053413-4fdc8980-5d54-11e9-85e3-e9e5d0e10772.png)

Linux (GNOME desktop):
![open-gnome](https://user-images.githubusercontent.com/245089/47155867-0f8cd900-d2e6-11e8-93af-275636491ec4.png)

Linux (KDE desktop):
![open-kde](https://user-images.githubusercontent.com/245089/47155866-0f8cd900-d2e6-11e8-8006-f14b948afc55.png)
