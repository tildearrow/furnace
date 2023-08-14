# IMPORTANT NOTICE

1. this section is exclusively for ISSUES related to Furnace (bugs, major annoyances and others).
2. this section is NOT for Suggestions, Questions, Showcase or any other Discussions that do not meet the criteria and definition of an ISSUE.
  - see the Discussions section if you wish to submit these.
3. check whether your issue has been reported already.
  - go to the Issues section, and use the search bar that appears on top of the Issues list.
4. include the following information:
  - version of Furnace (help > about)
  - operating system (and version)
  - whether you have downloaded Furnace, or built it from source.
5. provide these details if you believe the issue is operating system and/or computer-specific:
  - CPU model
    - Windows: go to Control Panel > System.
    - macOS: go to the Apple menu and select About This Mac...
    - Linux: use `lscpu` or `cat /proc/cpuinfo`.
  - graphics card (and driver version)
    - Windows: open `dxdiag` and observe the Render tab.
    - macOS: go to the Apple menu and select About This Mac...
      - this information is not always shown.
      - this information is not necessary if you use an Apple silicon Mac.
    - Linux: use `glxinfo | grep OpenGL`.
6. if your issue is an abnormal program termination (a "Crash"), you must provide additional details:
  - the furnace_crash.txt file that is created by Furnace after a Crash. this file is located in the following paths:
    - Windows: `C:\Users\<username>\furnace_crash.txt`
    - Linux/other: `/tmp/furnace_crash.txt`
    - on macOS this file is not generated. you may retrieve information about the Crash by clicking on "Report..." or "Show Details" in the "quit unexpectedly" dialog that appears following the Crash.
      - make sure to remove any personal information for privacy reasons.
      - be sure to select "Don't Send" afterwards.
  - the furnace.log file located in:
    - Windows: `C:\Users\<username>\AppData\Roaming\furnace\furnace.log`
    - macOS: `~/Library/Application Support/furnace/furnace.log`
    - Linux: `~/.config/furnace/furnace.log`
    - make sure to remove any personal information for privacy reasons.

BY SUBMITTING A TICKET, YOU HEREBY AGREE TO COMPLY WITH THESE TERMS.
FAILURE TO DO SO MAY RESULT IN YOUR TICKET BEING DECLARED VOID.

***END OF NOTICE*** --- REMOVE THIS NOTICE AFTER READING!
