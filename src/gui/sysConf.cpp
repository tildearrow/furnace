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

#include "gui.h"

void FurnaceGUI::drawSysConf(int i) {
  unsigned int flags=e->song.systemFlags[i];
  bool restart=settings.restartOnFlagChange;
  bool sysPal=flags&1;
  switch (e->song.system[i]) {
    case DIV_SYSTEM_YM2612:
    case DIV_SYSTEM_YM2612_EXT: {
      if (ImGui::RadioButton("NTSC (7.67MHz)",(flags&3)==0)) {
        e->setSysFlags(i,(flags&0x80000000)|0,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("PAL (7.61MHz)",(flags&3)==1)) {
        e->setSysFlags(i,(flags&0x80000000)|1,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("FM Towns (8MHz)",(flags&3)==2)) {
        e->setSysFlags(i,(flags&0x80000000)|2,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("AtGames Genesis (6.13MHz)",(flags&3)==3)) {
        e->setSysFlags(i,(flags&0x80000000)|3,restart);
        updateWindowTitle();
      }
      bool ladder=flags&0x80000000;
      if (ImGui::Checkbox("Enable DAC distortion",&ladder)) {
        e->setSysFlags(i,(flags&(~0x80000000))|(ladder?0x80000000:0),restart);
        updateWindowTitle();
      }
      break;
    }
    case DIV_SYSTEM_SMS: {
      ImGui::Text("Clock rate:");
      if (ImGui::RadioButton("NTSC (3.58MHz)",(flags&3)==0)) {
        e->setSysFlags(i,(flags&(~3))|0,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("PAL (3.55MHz)",(flags&3)==1)) {
        e->setSysFlags(i,(flags&(~3))|1,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("BBC Micro (4MHz)",(flags&3)==2)) {
        e->setSysFlags(i,(flags&(~3))|2,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("Half NTSC (1.79MHz)",(flags&3)==3)) {
        e->setSysFlags(i,(flags&(~3))|3,restart);
        updateWindowTitle();
      }
      ImGui::Text("Chip type:");
      if (ImGui::RadioButton("Sega VDP/Master System",((flags>>2)&3)==0)) {
        e->setSysFlags(i,(flags&(~12))|0,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("TI SN76489",((flags>>2)&3)==1)) {
        e->setSysFlags(i,(flags&(~12))|4,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("TI SN76489 with Atari-like short noise",((flags>>2)&3)==2)) {
        e->setSysFlags(i,(flags&(~12))|8,restart);
        updateWindowTitle();
      }
      /*if (ImGui::RadioButton("Game Gear",(flags>>2)==3)) {
        e->setSysFlags(i,(flags&3)|12);
      }*/

      bool noPhaseReset=flags&16;
      if (ImGui::Checkbox("Disable noise period change phase reset",&noPhaseReset)) {
        e->setSysFlags(i,(flags&(~16))|(noPhaseReset<<4),restart);
        updateWindowTitle();
      }
      break;
    }
    case DIV_SYSTEM_OPLL:
    case DIV_SYSTEM_OPLL_DRUMS:
    case DIV_SYSTEM_VRC7: {
      ImGui::Text("Clock rate:");
      if (ImGui::RadioButton("NTSC (3.58MHz)",(flags&15)==0)) {
        e->setSysFlags(i,(flags&(~15))|0,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("PAL (3.55MHz)",(flags&15)==1)) {
        e->setSysFlags(i,(flags&(~15))|1,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("BBC Micro (4MHz)",(flags&15)==2)) {
        e->setSysFlags(i,(flags&(~15))|2,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("Half NTSC (1.79MHz)",(flags&15)==3)) {
        e->setSysFlags(i,(flags&(~15))|3,restart);
        updateWindowTitle();
      }
      if (e->song.system[i]!=DIV_SYSTEM_VRC7) {
        ImGui::Text("Patch set:");
        if (ImGui::RadioButton("Yamaha YM2413",((flags>>4)&15)==0)) {
          e->setSysFlags(i,(flags&(~0xf0))|0,restart);
          updateWindowTitle();
        }
        if (ImGui::RadioButton("Yamaha YMF281",((flags>>4)&15)==1)) {
          e->setSysFlags(i,(flags&(~0xf0))|0x10,restart);
          updateWindowTitle();
        }
        if (ImGui::RadioButton("Yamaha YM2423",((flags>>4)&15)==2)) {
          e->setSysFlags(i,(flags&(~0xf0))|0x20,restart);
          updateWindowTitle();
        }
        if (ImGui::RadioButton("Konami VRC7",((flags>>4)&15)==3)) {
          e->setSysFlags(i,(flags&(~0xf0))|0x30,restart);
          updateWindowTitle();
        }
      }
      break;
    }
    case DIV_SYSTEM_YM2151:
      if (ImGui::RadioButton("NTSC/X16 (3.58MHz)",flags==0)) {
        e->setSysFlags(i,0,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("PAL (3.55MHz)",flags==1)) {
        e->setSysFlags(i,1,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("X1/X68000 (4MHz)",flags==2)) {
        e->setSysFlags(i,2,restart);
        updateWindowTitle();
      }
      break;
    case DIV_SYSTEM_NES:
    case DIV_SYSTEM_VRC6:
    case DIV_SYSTEM_FDS:
    case DIV_SYSTEM_MMC5:
      if (ImGui::RadioButton("NTSC (1.79MHz)",flags==0)) {
        e->setSysFlags(i,0,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("PAL (1.67MHz)",flags==1)) {
        e->setSysFlags(i,1,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("Dendy (1.77MHz)",flags==2)) {
        e->setSysFlags(i,2,restart);
        updateWindowTitle();
      }
      break;
    case DIV_SYSTEM_C64_8580:
    case DIV_SYSTEM_C64_6581:
      if (ImGui::RadioButton("NTSC (1.02MHz)",flags==0)) {
        e->setSysFlags(i,0,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("PAL (0.99MHz)",flags==1)) {
        e->setSysFlags(i,1,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("SSI 2001 (0.89MHz)",flags==2)) {
        e->setSysFlags(i,2,restart);
        updateWindowTitle();
      }
      break;
    case DIV_SYSTEM_AY8910:
    case DIV_SYSTEM_AY8930: {
      ImGui::Text("Clock rate:");
      if (ImGui::RadioButton("1.79MHz (ZX Spectrum NTSC/MSX)",(flags&15)==0)) {
        e->setSysFlags(i,(flags&(~15))|0,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("1.77MHz (ZX Spectrum)",(flags&15)==1)) {
        e->setSysFlags(i,(flags&(~15))|1,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("1.75MHz (ZX Spectrum)",(flags&15)==2)) {
        e->setSysFlags(i,(flags&(~15))|2,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("2MHz (Atari ST/Sharp X1)",(flags&15)==3)) {
        e->setSysFlags(i,(flags&(~15))|3,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("1.5MHz (Vectrex)",(flags&15)==4)) {
        e->setSysFlags(i,(flags&(~15))|4,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("1MHz (Amstrad CPC)",(flags&15)==5)) {
        e->setSysFlags(i,(flags&(~15))|5,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("0.89MHz (Sunsoft 5B)",(flags&15)==6)) {
        e->setSysFlags(i,(flags&(~15))|6,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("1.67MHz (?)",(flags&15)==7)) {
        e->setSysFlags(i,(flags&(~15))|7,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("0.83MHz (Sunsoft 5B on PAL)",(flags&15)==8)) {
        e->setSysFlags(i,(flags&(~15))|8,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("1.10MHz (Gamate/VIC-20 PAL)",(flags&15)==9)) {
        e->setSysFlags(i,(flags&(~15))|9,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("2^21Hz (Game Boy)",(flags&15)==10)) {
        e->setSysFlags(i,(flags&(~15))|10,restart);
        updateWindowTitle();
      }
      if (e->song.system[i]==DIV_SYSTEM_AY8910) {
        ImGui::Text("Chip type:");
        if (ImGui::RadioButton("AY-3-8910",(flags&0x30)==0)) {
          e->setSysFlags(i,(flags&(~0x30))|0,restart);
          updateWindowTitle();
        }
        if (ImGui::RadioButton("YM2149(F)",(flags&0x30)==16)) {
          e->setSysFlags(i,(flags&(~0x30))|16,restart);
          updateWindowTitle();
        }
        if (ImGui::RadioButton("Sunsoft 5B",(flags&0x30)==32)) {
          e->setSysFlags(i,(flags&(~0x30))|32,restart);
          updateWindowTitle();
        }
        if (ImGui::RadioButton("AY-3-8914",(flags&0x30)==48)) {
          e->setSysFlags(i,(flags&(~0x30))|48,restart);
          updateWindowTitle();
        }
      }
      bool stereo=flags&0x40;
      ImGui::BeginDisabled((flags&0x30)==32);
      if (ImGui::Checkbox("Stereo##_AY_STEREO",&stereo)) {
        e->setSysFlags(i,(flags&(~0x40))|(stereo?0x40:0),restart);
        updateWindowTitle();
      }
      ImGui::EndDisabled();
      break;
    }
    case DIV_SYSTEM_SAA1099:
      if (ImGui::RadioButton("SAM Coupé (8MHz)",flags==0)) {
        e->setSysFlags(i,0,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("NTSC (7.15MHz)",flags==1)) {
        e->setSysFlags(i,1,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("PAL (7.09MHz)",flags==2)) {
        e->setSysFlags(i,2,restart);
        updateWindowTitle();
      }
      break;
    case DIV_SYSTEM_AMIGA: {
      ImGui::Text("Stereo separation:");
      int stereoSep=(flags>>8)&127;
      if (CWSliderInt("##StereoSep",&stereoSep,0,127)) {
        if (stereoSep<0) stereoSep=0;
        if (stereoSep>127) stereoSep=127;
        e->setSysFlags(i,(flags&(~0x7f00))|((stereoSep&127)<<8),restart);
        updateWindowTitle();
      } rightClickable
      if (ImGui::RadioButton("Amiga 500 (OCS)",(flags&2)==0)) {
        e->setSysFlags(i,flags&(~2),restart);
      }
      if (ImGui::RadioButton("Amiga 1200 (AGA)",(flags&2)==2)) {
        e->setSysFlags(i,(flags&(~2))|2,restart);
      }
      sysPal=flags&1;
      if (ImGui::Checkbox("PAL",&sysPal)) {
        e->setSysFlags(i,(flags&(~1))|sysPal,restart);
        updateWindowTitle();
      }
      bool bypassLimits=flags&4;
      if (ImGui::Checkbox("Bypass frequency limits",&bypassLimits)) {
        e->setSysFlags(i,(flags&(~4))|(bypassLimits<<2),restart);
        updateWindowTitle();
      }
      break;
    }
    case DIV_SYSTEM_PCSPKR: {
      ImGui::Text("Speaker type:");
      if (ImGui::RadioButton("Unfiltered",(flags&3)==0)) {
        e->setSysFlags(i,(flags&(~3))|0,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("Cone",(flags&3)==1)) {
        e->setSysFlags(i,(flags&(~3))|1,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("Piezo",(flags&3)==2)) {
        e->setSysFlags(i,(flags&(~3))|2,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("Use system beeper (Linux only!)",(flags&3)==3)) {
        e->setSysFlags(i,(flags&(~3))|3,restart);
        updateWindowTitle();
      }
      break;
    }
    case DIV_SYSTEM_QSOUND: {
      ImGui::Text("Echo delay:");
      int echoBufSize=2725 - (flags & 4095);
      if (CWSliderInt("##EchoBufSize",&echoBufSize,0,2725)) {
        if (echoBufSize<0) echoBufSize=0;
        if (echoBufSize>2725) echoBufSize=2725;
        e->setSysFlags(i,(flags & ~4095) | ((2725 - echoBufSize) & 4095),restart);
        updateWindowTitle();
      } rightClickable
      ImGui::Text("Echo feedback:");
      int echoFeedback=(flags>>12)&255;
      if (CWSliderInt("##EchoFeedback",&echoFeedback,0,255)) {
        if (echoFeedback<0) echoFeedback=0;
        if (echoFeedback>255) echoFeedback=255;
        e->setSysFlags(i,(flags & ~(255 << 12)) | ((echoFeedback & 255) << 12),restart);
        updateWindowTitle();
      } rightClickable
      break;
    }
    case DIV_SYSTEM_X1_010: {
      ImGui::Text("Clock rate:");
      if (ImGui::RadioButton("16MHz (Seta 1)",(flags&15)==0)) {
        e->setSysFlags(i,(flags&(~15))|0,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("16.67MHz (Seta 2)",(flags&15)==1)) {
        e->setSysFlags(i,(flags&(~15))|1,restart);
        updateWindowTitle();
      }
      bool x1_010Stereo=flags&16;
      if (ImGui::Checkbox("Stereo",&x1_010Stereo)) {
        e->setSysFlags(i,(flags&(~16))|(x1_010Stereo<<4),restart);
        updateWindowTitle();
      }
      break;
    }
    case DIV_SYSTEM_N163: {
      ImGui::Text("Clock rate:");
      if (ImGui::RadioButton("NTSC (1.79MHz)",(flags&15)==0)) {
        e->setSysFlags(i,(flags&(~15))|0,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("PAL (1.67MHz)",(flags&15)==1)) {
        e->setSysFlags(i,(flags&(~15))|1,restart);
        updateWindowTitle();
      }
      if (ImGui::RadioButton("Dendy (1.77MHz)",(flags&15)==2)) {
        e->setSysFlags(i,(flags&(~15))|2,restart);
        updateWindowTitle();
      }
      ImGui::Text("Initial channel limit:");
      int initialChannelLimit=((flags>>4)&7)+1;
      if (CWSliderInt("##InitialChannelLimit",&initialChannelLimit,1,8)) {
        if (initialChannelLimit<1) initialChannelLimit=1;
        if (initialChannelLimit>8) initialChannelLimit=8;
        e->setSysFlags(i,(flags & ~(7 << 4)) | (((initialChannelLimit-1) & 7) << 4),restart);
        updateWindowTitle();
      } rightClickable
      break;
    }
    case DIV_SYSTEM_GB:
    case DIV_SYSTEM_SWAN:
    case DIV_SYSTEM_VERA:
    case DIV_SYSTEM_BUBSYS_WSG:
    case DIV_SYSTEM_YM2610:
    case DIV_SYSTEM_YM2610_EXT:
    case DIV_SYSTEM_YM2610_FULL:
    case DIV_SYSTEM_YM2610_FULL_EXT:
    case DIV_SYSTEM_YM2610B:
    case DIV_SYSTEM_YM2610B_EXT:
    case DIV_SYSTEM_YMU759:
    case DIV_SYSTEM_PET:
      ImGui::Text("nothing to configure");
      break;
    default:
      if (ImGui::Checkbox("PAL",&sysPal)) {
        e->setSysFlags(i,sysPal,restart);
        updateWindowTitle();
      }
      break;
  }
}