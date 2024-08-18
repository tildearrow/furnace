# MODIFIED

this is a modified version of `https://github.com/JesusKrists/imgui_software_renderer`, which itself is a modified version of `https://github.com/emilk/imgui_software_renderer`.

# Dear ImGui software renderer
This is a software renderer for [Dear ImGui](https://github.com/ocornut/imgui).
I built it not out of a specific need, but because it was fun.
The goal was to get something accurate and decently fast in not too many lines of code.
It renders a complex GUI in 1-10 milliseconds on a modern laptop.

## What it is:
As the name implies, this is a software renderer for ImGui. It does not handle any windows or input. In the supplied example I use [SDL2](www.libsdl.org) for that.

## How to use it
Just copy `imgui_sw.hpp` and `imgui_sw.cpp`. There are no other dependencies beside Dear ImGui. Requires C++11.

## How to test it
```
git clone https://github.com/emilk/imgui_software_renderer.git
cd imgui_software_renderer
git submodule update --init --recursive
./build_and_run.sh
```

For the example to work you will need to have SDL2 on your system.

## Example:
This renders in 7 ms on my MacBook Pro:

![Software rendered](screenshots/imgui_sw.png)

## Alternatives
There is another software rasterizer for ImGui (which I did not know about when I wrote mine) at https://github.com/sronsse/imgui/tree/sw_rasterizer_example/examples/sdl_sw_example.
I have not compared the two (yet).

## Future work:
* We do not yet support painting with any other texture than the default font texture.
* Optimize rendering of gradient rectangles (common for color pickers)
* Compare my software renderer to [the one by](https://github.com/sronsse/imgui/tree/sw_rasterizer_example/examples/sdl_sw_example) @sronsse

## License:
This software is dual-licensed to the public domain and under the following
license: you are granted a perpetual, irrevocable license to copy, modify,
publish, and distribute this file as you see fit.
