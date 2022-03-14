//
//  Portable File Dialogs
//
//  Copyright © 2018—2020 Sam Hocevar <sam@hocevar.net>
//
//  This program is free software. It comes without any warranty, to
//  the extent permitted by applicable law. You can redistribute it
//  and/or modify it under the terms of the Do What the Fuck You Want
//  to Public License, Version 2, as published by the WTFPL Task Force.
//  See http://www.wtfpl.net/ for more details.
//

#include "portable-file-dialogs.h"

#include <iostream>

#if _WIN32
#define DEFAULT_PATH "C:\\"
#else
#define DEFAULT_PATH "/tmp"
#endif

int main()
{
    // Check that a backend is available
    if (!pfd::settings::available())
    {
        std::cout << "Portable File Dialogs are not available on this platform.\n";
        return 1;
    }

    // Set verbosity to true
    pfd::settings::verbose(true);

    // Notification
    pfd::notify("Important Notification",
                "This is ' a message, pay \" attention \\ to it!",
                pfd::icon::info);

    // Message box with nice message
    auto m = pfd::message("Personal Message",
                          "You are an amazing person, don’t let anyone make you think otherwise.",
                          pfd::choice::yes_no_cancel,
                          pfd::icon::warning);

    // Optional: do something while waiting for user action
    for (int i = 0; i < 10 && !m.ready(1000); ++i)
        std::cout << "Waited 1 second for user input...\n";

    // Do something according to the selected button
    switch (m.result())
    {
        case pfd::button::yes: std::cout << "User agreed.\n"; break;
        case pfd::button::no: std::cout << "User disagreed.\n"; break;
        case pfd::button::cancel: std::cout << "User freaked out.\n"; break;
        default: break; // Should not happen
    }

    // Directory selection
    auto dir = pfd::select_folder("Select any directory", DEFAULT_PATH).result();
    std::cout << "Selected dir: " << dir << "\n";

    // File open
    auto f = pfd::open_file("Choose files to read", DEFAULT_PATH,
                            { "Text Files (.txt .text)", "*.txt *.text",
                              "All Files", "*" },
                            pfd::opt::multiselect);
    std::cout << "Selected files:";
    for (auto const &name : f.result())
        std::cout << " " + name;
    std::cout << "\n";
}

// Unused function that just tests the whole API
void api()
{
    // pfd::settings
    pfd::settings::verbose(true);
    pfd::settings::rescan();

    // pfd::notify
    pfd::notify("", "");
    pfd::notify("", "", pfd::icon::info);
    pfd::notify("", "", pfd::icon::warning);
    pfd::notify("", "", pfd::icon::error);
    pfd::notify("", "", pfd::icon::question);

    pfd::notify a("", "");
    (void)a.ready();
    (void)a.ready(42);

    // pfd::message
    pfd::message("", "");
    pfd::message("", "", pfd::choice::ok);
    pfd::message("", "", pfd::choice::ok_cancel);
    pfd::message("", "", pfd::choice::yes_no);
    pfd::message("", "", pfd::choice::yes_no_cancel);
    pfd::message("", "", pfd::choice::retry_cancel);
    pfd::message("", "", pfd::choice::abort_retry_ignore);
    pfd::message("", "", pfd::choice::ok, pfd::icon::info);
    pfd::message("", "", pfd::choice::ok, pfd::icon::warning);
    pfd::message("", "", pfd::choice::ok, pfd::icon::error);
    pfd::message("", "", pfd::choice::ok, pfd::icon::question);

    pfd::message b("", "");
    (void)b.ready();
    (void)b.ready(42);
    (void)b.result();
}

