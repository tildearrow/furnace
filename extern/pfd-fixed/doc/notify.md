## Notification API

Displaying a desktop notification is done using the `pfd::notify` class. It can be provided a
title, a message text, and an `icon` for the notification style:

```cpp
pfd::notify::notify(std::string const &title,
                    std::string const &text,
                    pfd::icon icon = pfd::icon::info);

enum class pfd::icon { info, warning, error };
```

## Example

Displaying a notification is straightforward. Emoji are supported:

```cpp
pfd::notify("System event", "Something might be on fire 🔥",
            pfd::icon::warning);
```

The `pfd::notify` object needs not be kept around, letting the object clean up itself is enough.

## Screenshots

Windows 10:
![notify-win32](https://user-images.githubusercontent.com/245089/47142453-2ff76c00-d2c3-11e8-871a-1a110ac91eb2.png)

Mac OS X (dark theme):
![image](https://user-images.githubusercontent.com/245089/56053188-bc0abd80-5d53-11e9-8298-68aa96315c6c.png)

Mac OS X (light theme):
![image](https://user-images.githubusercontent.com/245089/56053137-92ea2d00-5d53-11e9-8cf2-049486c45713.png)

Linux (GNOME desktop):
![notify-gnome](https://user-images.githubusercontent.com/245089/47142455-30900280-d2c3-11e8-8b76-ea16c7e502d4.png)

Linux (KDE desktop):
![notify-kde](https://user-images.githubusercontent.com/245089/47149206-27a72d00-d2d3-11e8-8f1b-96e462f08c2b.png)
