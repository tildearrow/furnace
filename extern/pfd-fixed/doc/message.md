## Message Box API

Displaying a message box is done using the `pfd::message` class. It can be provided a title, a
message text, a `choice` representing which buttons need to be rendered, and an `icon` for the
message:

```cpp
pfd::message::message(std::string const &title,
                      std::string const &text,
                      pfd::choice choice = pfd::choice::ok_cancel,
                      pfd::icon icon = pfd::icon::info);

enum class pfd::choice { ok, ok_cancel, yes_no, yes_no_cancel };

enum class pfd::icon { info, warning, error, question };
```

The pressed button is queried using `pfd::message::result()`. If the dialog box is closed by any
other means, the `pfd::button::cancel` is assumed:

```cpp
pfd::button pfd::message::result();

enum class pfd::button { ok, cancel, yes, no };
```

It is possible to ask the dialog box whether the user took action using the `pfd::message::ready()`
method, with an optional `timeout` argument. If the user did not press a button within `timeout`
milliseconds, the function will return `false`:

```cpp
bool pfd::message::ready(int timeout = pfd::default_wait_timeout);
```

## Example 1: simple notification

The `pfd::message` destructor waits for user action, so this operation will block until the user
closes the message box:

```cpp
pfd::message("Problem", "An error occurred while doing things",
             pfd::choice::ok, pfd::icon::error);
```

## Example 2: retrieving the pressed button

Using `pfd::message::result()` will also wait for user action before returning. This operation will block and return the user choice:

```cpp
// Ask for user opinion
auto button = pfd::message("Action requested", "Do you want to proceed with things?",
                           pfd::choice::yes_no, pfd::icon::question).result();
// Do something with button…
```

## Example 3: asynchronous message box

Using `pfd::message::ready()` allows the application to perform other tasks while waiting for
user input:

```cpp
// Message box with nice message
auto box = pfd::message("Unsaved Files", "Do you want to save the current "
                        "document before closing the application?",
                        pfd::choice::yes_no_cancel,
                        pfd::icon::warning);

// Do something while waiting for user input
while (!box.ready(1000))
    std::cout << "Waited 1 second for user input...\n";

// Act depending on the selected button
switch (box.result())
{   
    case pfd::button::yes:    std::cout << "User agreed.\n"; break;
    case pfd::button::no:     std::cout << "User disagreed.\n"; break;
    case pfd::button::cancel: std::cout << "User freaked out.\n"; break;
}   
```

## Screenshots

#### Windows 10

![warning-win32](https://user-images.githubusercontent.com/245089/47136607-76919a00-d2b4-11e8-8f42-e2d62c4f9570.png)

#### Mac OS X

![warning-osx-dark](https://user-images.githubusercontent.com/245089/56053001-22dba700-5d53-11e9-8233-ca7a2c58188d.png) ![warning-osx-light](https://user-images.githubusercontent.com/245089/56053055-49014700-5d53-11e9-8306-e9a03a25e044.png)

#### Linux (GNOME desktop)

![warning-gnome](https://user-images.githubusercontent.com/245089/47140824-8662ab80-d2bf-11e8-9c87-2742dd5b58af.png)

#### Linux (KDE desktop)

![warning-kde](https://user-images.githubusercontent.com/245089/47149255-4dcccd00-d2d3-11e8-84c9-f85612784680.png)
