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

#include <cstdio>

int main()
{
    // Set verbosity to true
    pfd::settings::verbose(true);

    // Message box with nice message
    auto m = pfd::message("Upgrade software?",
                          "Press OK to upgrade this software.\n"
                          "\n"
                          "By default, the software will update itself\n"
                          "automatically in 10 seconds.",
                          pfd::choice::ok_cancel,
                          pfd::icon::warning);

    // Wait for an answer for up to 10 seconds
    for (int i = 0; i < 10 && !m.ready(1000); ++i)
        ;

    // Upgrade software if user clicked OK, or if user didn’t interact
    bool upgrade = m.ready() ? m.result() == pfd::button::ok : m.kill();
    if (upgrade)
        std::cout << "Upgrading software!\n";
    else
        std::cout << "Not upgrading software.\n";
}

