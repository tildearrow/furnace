Support development of IconFontCppHeaders through [GitHub Sponsors](https://github.com/sponsors/dougbinks) or [Patreon](https://www.patreon.com/enkisoftware)

[<img src="https://img.shields.io/static/v1?logo=github&label=Github&message=Sponsor&color=#ea4aaa" width="200"/>](https://github.com/sponsors/dougbinks)    [<img src="https://c5.patreon.com/external/logo/become_a_patron_button@2x.png" alt="Become a Patron" width="150"/>](https://www.patreon.com/enkisoftware)


# IconFontCppHeaders

[https://github.com/juliettef/IconFontCppHeaders](https://github.com/juliettef/IconFontCppHeaders)

C++11, C89 headers and C# classes for icon fonts Font Awesome, Fork Awesome, Google Material Design icons, Kenney game icons and Fontaudio.

A set of header files and classes for using icon fonts in C, C++ and C#, along with the python generator used to create the files.

Each header contains defines for one font, with each icon code point defined as ICON_*, along with the min and max code points for font loading purposes.

## Icon Fonts

### [Font Awesome](https://fontawesome.com)

* [https://github.com/FortAwesome/Font-Awesome](https://github.com/FortAwesome/Font-Awesome)

#### Font Awesome 4 
* [icons.yml](https://github.com/FortAwesome/Font-Awesome/blob/fa-4/src/icons.yml)
* [fontawesome-webfont.ttf](https://github.com/FortAwesome/Font-Awesome/blob/fa-4/fonts/fontawesome-webfont.ttf)

#### Font Awesome 5 - see [notes below](#notes-about-font-awesome-5)
* [icons.yml](https://github.com/FortAwesome/Font-Awesome/blob/master/metadata/icons.yml)
* [fa-brands-400.ttf](https://github.com/FortAwesome/Font-Awesome/blob/master/webfonts/fa-brands-400.ttf)
* [fa-regular-400.ttf](https://github.com/FortAwesome/Font-Awesome/blob/master/webfonts/fa-regular-400.ttf)
* [fa-solid-900.ttf](https://github.com/FortAwesome/Font-Awesome/blob/master/webfonts/fa-solid-900.ttf)

#### Font Awesome 5 Pro - this is a paid product, see [notes below](#notes-about-font-awesome-5)
Files downloaded from [fontawesome.com](https://fontawesome.com)  

* ..\fontawesome-pro-n.n.n-web\metadata\icons.yml  
* ..\fontawesome-pro-n.n.n-web\webfonts\fa-brands-400.ttf  
* ..\fontawesome-pro-n.n.n-web\webfonts\fa-light-300.ttf  
* ..\fontawesome-pro-n.n.n-web\webfonts\fa-regular-400.ttf  
* ..\fontawesome-pro-n.n.n-web\webfonts\fa-solid-900.ttf  

### [Fork Awesome](https://forkawesome.github.io/Fork-Awesome)
* [https://github.com/ForkAwesome/Fork-Awesome](https://github.com/ForkAwesome/Fork-Awesome)
* [icons.yml](https://github.com/ForkAwesome/Fork-Awesome/blob/master/src/icons/icons.yml)
* [forkawesome-webfont.ttf](https://github.com/ForkAwesome/Fork-Awesome/blob/master/fonts/forkawesome-webfont.ttf)

### [Google Material Design icons](https://design.google.com/icons)
* [https://github.com/google/material-design-icons](https://github.com/google/material-design-icons)
* [codepoints](https://github.com/google/material-design-icons/blob/master/iconfont/codepoints)
* [MaterialIcons-Regular.ttf](https://github.com/google/material-design-icons/blob/master/iconfont/MaterialIcons-Regular.ttf)

### [Kenney Game icons](http://kenney.nl/assets/game-icons) and [Game icons expansion](http://kenney.nl/assets/game-icons-expansion) 
* [https://github.com/nicodinh/kenney-icon-font](https://github.com/nicodinh/kenney-icon-font)
* [kenney-icons.css](https://github.com/nicodinh/kenney-icon-font/blob/master/css/kenney-icons.css)
* [kenney-icon-font.ttf](https://github.com/nicodinh/kenney-icon-font/blob/master/fonts/kenney-icon-font.ttf)

### [Fontaudio](https://github.com/fefanto/fontaudio)
* [https://github.com/fefanto/fontaudio](https://github.com/fefanto/fontaudio)
* [fontaudio.css](https://github.com/fefanto/fontaudio/blob/master/font/fontaudio.css)
* [fontaudio.ttf](https://github.com/fefanto/fontaudio/blob/master/font/fontaudio.ttf)

## Notes about Font Awesome 5
### Codepoints grouping
Font Awesome 5 splits the different styles of icons into different font files with identical codepoints for *light*, *regular* and *solid* styles, and a different set of codepoints for *brands*. We have put the brands into a separate header file.

### Generating Pro header files
Download the Font Awesome Pro Web package. To generate the headers, drop *icons.yml* in the same directory as *GenerateIconFontCppHeaders.py* before running the script. The file *icons.yml* is under *..\fontawesome-pro-n.n.n-web\metadata\icons.yml* where *n.n.n* is the version number.

## Ionicons and webfont Material Design Icons
Unsupported as of 29 Apr 2020. See [Issue #16](https://github.com/juliettef/IconFontCppHeaders/issues/16).

## Example Code

Using [Dear ImGui](https://github.com/ocornut/imgui) as an example UI library:

```Cpp

#include "IconsFontAwesome5.h"

ImGuiIO& io = ImGui::GetIO();
io.Fonts->AddFontDefault();
 
// merge in icons from Font Awesome
static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;
io.Fonts->AddFontFromFileTTF( FONT_ICON_FILE_NAME_FAS, 16.0f, &icons_config, icons_ranges );
// use FONT_ICON_FILE_NAME_FAR if you want regular instead of solid

// in an imgui window somewhere...
ImGui::Text( ICON_FA_PAINT_BRUSH "  Paint" );    // use string literal concatenation
// outputs a paint brush icon and 'Paint' as a string.
```

## Projects using the font icon header files

### [Avoyd](https://www.enkisoftware.com/avoyd)
Avoyd is a 6 degrees of freedom voxel game that includes a voxel editor tool.   
[www.avoyd.com](https://www.avoyd.com)

The voxel editor's UI uses Dear ImGui with Font Awesome icon fonts.  

![Screenshot of the the game Avoyd's Voxel Editor UI using an IconFontCppHeaders header file for Font Awesome with Dear ImGui](https://github.com/juliettef/Media/blob/master/IconFontCppHeaders_Avoyd_voxel_editor.png?raw=true)

### [bgfx](https://github.com/bkaradzic/bgfx)
Cross-platform rendering library.  
[bkaradzic.github.io/bgfx/overview](https://bkaradzic.github.io/bgfx/overview.html)

### [glChAoS.P](https://github.com/BrutPitt/glChAoS.P)
Real time 3D strange attractors scout.  
[www.michelemorrone.eu/glchaosp](https://www.michelemorrone.eu/glchaosp)

![Screenshot of glChAoS.P UI using IconFontCppHeaders header file for Font Awesome with Dear ImGui](https://raw.githubusercontent.com/BrutPitt/glChAoS.P/master/imgsCapture/ssWGL_half.jpg)

### [iPlug2](https://github.com/iplug2/iplug2)
Cross platform C++ audio plug-in framework  
[iplug2.github.io](https://iplug2.github.io)

### [Tracy Profiler](https://bitbucket.org/wolfpld/tracy/src/master/)
Real time, nanosecond resolution, remote telemetry frame profiler for games and other applications.  

[![New features in Tracy Profiler v0.6](https://img.youtube.com/vi/uJkrFgriuOo/0.jpg)](https://www.youtube.com/watch?v=uJkrFgriuOo)

### [Visual 6502 Remix](https://github.com/floooh/v6502r)
Transistor level 6502 Hardware Simulation  
[floooh.github.io/visual6502remix](https://floooh.github.io/visual6502remix)

## Credits

Development - [Juliette Foucaut](http://www.enkisoftware.com/about.html#juliette) - [@juliettef](https://github.com/juliettef)  
Requirements - [Doug Binks](http://www.enkisoftware.com/about.html#doug) - [@dougbinks](https://github.com/dougbinks)  
[None](https://bitbucket.org/duangle/nonelang/src) language implementation and [refactoring](https://gist.github.com/paniq/4a734e9d8e86a2373b5bc4ca719855ec) - [Leonard Ritter](http://www.leonard-ritter.com/) - [@paniq](https://github.com/paniq)  
Suggestion to add a define for the ttf file name - [Sean Barrett](https://nothings.org/) - [@nothings](https://github.com/nothings)  
Initial Font Awesome 5 implementation - [Codecat](https://codecat.nl/) - [@codecat](https://github.com/codecat)  
Suggestion to add Fork Awesome - [Julien Deswaef](http://xuv.be/) - [@xuv](https://github.com/xuv)  
Suggestion to add Ionicons - [Omar Cornut](http://www.miracleworld.net/) - [@ocornut](https://github.com/ocornut)  
C# language implementation - Rokas Kupstys - [@rokups](https://github.com/rokups)  
Suggestion to add Material Design Icons - Gustav Madeso - [@madeso](https://github.com/madeso)  
Fontaudio implementation - [Oli Larkin](https://www.olilarkin.co.uk/) - [@olilarkin](https://github.com/olilarkin)
