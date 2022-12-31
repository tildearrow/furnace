/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2022 tildearrow and contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#define _USE_MATH_DEFINES
#include "gui.h"
#include <math.h>

const char* aboutLine[]={
  "tildearrow",
  "is proud to present",
  "",
  ("Furnace " DIV_VERSION),
  "",
  "the biggest multi-system chiptune tracker!",
  "featuring DefleMask song compatibility.",
  "",
  "> CREDITS <",
  "",
  "-- program --",
  "tildearrow",
  "akumanatt",
  "cam900",
  "djtuBIG-MaliceX",
  "laoo",
  "OPNA2608",
  "superctr",
  "System64",
  "",
  "-- graphics/UI design --",
  "tildearrow",
  "BlastBrothers",
  "Mahbod Karamoozian",
  "nicco1690",
  "Raijin",
  "",
  "-- documentation --",
  "tildearrow",
  "freq-mod",
  "nicco1690",
  "DeMOSic",
  "cam900",
  "",
  "-- demo songs --",
  "0x5066",
  "Abstract 64",
  "ActualNK358",
  "akumanatt",
  "AmigaX",
  "AURORA*FIELDS",
  "BlueElectric05",
  "breakthetargets",
  "brickblock369",
  "Burnt Fishy",
  "CaptainMalware",
  "DeMOSic",
  "DevEd",
  "Dippy",
  "dumbut",
  "FΛDE",
  "Forte",
  "Fragmare",
  "freq-mod",
  "iyatemu",
  "JayBOB18",
  "Jimmy-DS",
  "Kagamiin~",
  "kleeder",
  "jaezu",
  "Laggy",
  "LovelyA72",
  "LunaMoth",
  "Lunathir",
  "LVintageNerd",
  "Mahbod Karamoozian",
  "Martin Demsky",
  "Miker",
  "nicco1690",
  "<nk>",
  "potatoTeto",
  "psxdominator",
  "Raijin",
  "SnugglyBun",
  "SuperJet Spade",
  "SwapXFO",
  "TakuikaNinja",
  "The Blender Fiddler",
  "TheDuccinator",
  "theloredev",
  "TheRealHedgehogSonic",
  "tildearrow",
  "Uhrwerk Klockwerx",
  "Ultraprogramer",
  "UserSniper",
  "Weeppiko",
  "ZoomTen (Zumi)",
  "",
  "-- additional feedback/fixes --",
  "fd",
  "GENATARi",
  "host12prog",
  "Lumigado",
  "Lunathir",
  "plane",
  "TheEssem",
  "",
  "powered by:",
  "Dear ImGui by Omar Cornut",
  "SDL2 by Sam Lantinga",
  "zlib by Jean-loup Gailly",
  "and Mark Adler",
  "libsndfile by Erik de Castro Lopo",
  "Portable File Dialogs by Sam Hocevar",
  "Native File Dialog by Frogtoss Games",
  "RtMidi by Gary P. Scavone",
  "FFTW by Matteo Frigo and Steven G. Johnson",
  "backward-cpp by Google",
  "adpcm by superctr",
  "Nuked-OPL3/OPLL/OPM/OPN2/PSG by Nuke.YKT",
  "ymfm by Aaron Giles",
  "MAME SN76496 by Nicola Salmoria",
  "MAME AY-3-8910 by Couriersud",
  "with AY8930 fixes by Eulous, cam900 and Grauw",
  "MAME SAA1099 by Juergen Buchmueller and Manuel Abadia",
  "MAME Namco WSG by Nicola Salmoria and Aaron Giles",
  "MAME RF5C68 core by Olivier Galibert and Aaron Giles",
  "MAME MSM5232 core by Jarek Burczynski and Hiromitsu Shioya",
  "MAME MSM6258 core by Barry Rodewald",
  "MAME YMZ280B core by Aaron Giles",
  "MAME GA20 core by Acho A. Tang and R. Belmont",
  "SAASound by Dave Hooper and Simon Owen",
  "SameBoy by Lior Halphon",
  "Mednafen PCE, WonderSwan, T6W28 and Virtual Boy audio cores",
  "SNES DSP core by Blargg",
  "puNES (NES, MMC5 and FDS) by FHorse",
  "NSFPlay (NES and FDS) by Brad Smith and Brezza",
  "reSID by Dag Lem",
  "reSIDfp by Dag Lem, Antti Lankila",
  "and Leandro Nini",
  "Stella by Stella Team",
  "QSound emulator by superctr and Valley Bell",
  "VICE VIC-20 sound core by Rami Rasanen and viznut",
  "VERA sound core by Frank van den Hoef",
  "mzpokeysnd POKEY emulator by Michael Borisov",
  "ASAP POKEY emulator by Piotr Fusik",
  "ported by laoo to C++",
  "K005289 emulator by cam900",
  "Namco 163 emulator by cam900",
  "Seta X1-010 emulator by cam900",
  "Konami VRC6 emulator by cam900",
  "Konami SCC emulator by cam900",
  "MSM6295 emulator by cam900",
  "",
  "greetings to:",
  "NEOART Costa Rica",
  "all members of Deflers of Noice!",
  "",
  "copyright © 2021-2022 tildearrow",
  "(and contributors).",
  "licensed under GPLv2+! see",
  "LICENSE for more information.",
  "",
  "help Furnace grow:",
  "https://github.com/tildearrow/furnace",
  "",
  "contact tildearrow at:",
  "https://tildearrow.org/?p=contact",
  "",
  "disclaimer:",
  "despite the fact this program works",
  "with the .dmf file format, it is NOT",
  "affiliated with Delek or DefleMask in",
  "any way, nor it is a replacement for",
  "the original program.",
  "",
  "it also comes with ABSOLUTELY NO WARRANTY.",
  "",
  "thanks to all contributors/bug reporters!"
};

const size_t aboutCount=sizeof(aboutLine)/sizeof(aboutLine[0]);

void FurnaceGUI::drawAbout() {
  // do stuff
  if (ImGui::Begin("About Furnace",NULL,ImGuiWindowFlags_Modal|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoDocking|ImGuiWindowFlags_NoTitleBar)) {
    ImGui::SetWindowPos(ImVec2(0,0));
    ImGui::SetWindowSize(ImVec2(canvasW,canvasH));
    ImGui::PushFont(bigFont);
    ImDrawList* dl=ImGui::GetWindowDrawList();
    float r=0;
    float g=0;
    float b=0;
    float peakMix=settings.partyTime?((peak[0]+peak[1])*0.5):0.3;
    ImGui::ColorConvertHSVtoRGB(aboutHue,1.0,0.25+MIN(0.75f,peakMix*0.75f),r,g,b);
    dl->AddRectFilled(ImVec2(0,0),ImVec2(canvasW,canvasH),0xff000000);
    bool skip=false;
    bool skip2=false;
    for (int i=(-80-sin(double(aboutSin)*2*M_PI/120.0)*80.0)*2*dpiScale; i<canvasW; i+=160*dpiScale) {
      skip2=!skip2;
      skip=skip2;
      for (int j=(-80-cos(double(aboutSin)*2*M_PI/150.0)*80.0)*2*dpiScale; j<canvasH; j+=160*dpiScale) {
        skip=!skip;
        if (skip) continue;
        dl->AddRectFilled(ImVec2(i,j),ImVec2(i+160*dpiScale,j+160*dpiScale),ImGui::GetColorU32(ImVec4(r*0.25,g*0.25,b*0.25,1.0)));
      }
    }

    skip=false;
    skip2=false;
    for (int i=(-80-cos(double(aboutSin)*2*M_PI/120.0)*80.0)*2*dpiScale; i<canvasW; i+=160*dpiScale) {
      skip2=!skip2;
      skip=skip2;
      for (int j=(-80-sin(double(aboutSin)*2*M_PI/150.0)*80.0)*2*dpiScale; j<canvasH; j+=160*dpiScale) {
        skip=!skip;
        if (skip) continue;
        dl->AddRectFilled(ImVec2(i,j),ImVec2(i+160*dpiScale,j+160*dpiScale),ImGui::GetColorU32(ImVec4(r*0.5,g*0.5,b*0.5,1.0)));
      }
    }

    skip=false;
    skip2=false;
    for (int i=(-160+fmod(aboutSin*2,160))*2*dpiScale; i<canvasW; i+=160*dpiScale) {
      skip2=!skip2;
      skip=skip2;
      for (int j=(-240-cos(double(aboutSin*M_PI/300.0))*240.0)*2*dpiScale; j<canvasH; j+=160*dpiScale) {
        skip=!skip;
        if (skip) continue;
        dl->AddRectFilled(ImVec2(i,j),ImVec2(i+160*dpiScale,j+160*dpiScale),ImGui::GetColorU32(ImVec4(r*0.75,g*0.75,b*0.75,1.0)));
      }
    }

    for (size_t i=0; i<aboutCount; i++) {
      double posX=(canvasW/2.0)+(sin(double(i)*0.5+double(aboutScroll)/(90.0*dpiScale))*120*dpiScale)-(ImGui::CalcTextSize(aboutLine[i]).x*0.5);
      double posY=(canvasH-aboutScroll+42*i*dpiScale);
      if (posY<-80*dpiScale || posY>canvasH) continue;
      dl->AddText(bigFont,bigFont->FontSize,
                  ImVec2(posX+dpiScale,posY+dpiScale),
                  0xff000000,aboutLine[i]);
      dl->AddText(bigFont,bigFont->FontSize,
                  ImVec2(posX+dpiScale,posY-dpiScale),
                  0xff000000,aboutLine[i]);
      dl->AddText(bigFont,bigFont->FontSize,
                  ImVec2(posX-dpiScale,posY+dpiScale),
                  0xff000000,aboutLine[i]);
      dl->AddText(bigFont,bigFont->FontSize,
                  ImVec2(posX-dpiScale,posY-dpiScale),
                  0xff000000,aboutLine[i]);
      dl->AddText(bigFont,bigFont->FontSize,
                  ImVec2(posX,posY),
                  0xffffffff,aboutLine[i]);
    }
    ImGui::PopFont();

    float timeScale=60.0f*ImGui::GetIO().DeltaTime;

    aboutHue+=(0.001+peakMix*0.004)*timeScale;
    aboutScroll+=(2+(peakMix>0.78)*3)*timeScale*dpiScale;
    aboutSin+=(1+(peakMix>0.75)*2)*timeScale;

    while (aboutHue>1) aboutHue--;
    while (aboutSin>=2400) aboutSin-=2400;
    if (aboutScroll>(42*dpiScale*aboutCount+canvasH)) aboutScroll=-20*dpiScale;

    WAKE_UP;
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_ABOUT;
  ImGui::End();
}
