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

bool FurnaceGUI::drawSysConf(int chan, DivSystem type, DivConfig& flags, bool modifyOnChange) {
  bool altered=false;
  bool restart=settings.restartOnFlagChange && modifyOnChange;

  switch (type) {
    case DIV_SYSTEM_YM2612:
    case DIV_SYSTEM_YM2612_EXT: 
    case DIV_SYSTEM_YM2612_FRAC:
    case DIV_SYSTEM_YM2612_FRAC_EXT: {
      int clockSel=flags.getInt("clockSel",0);
      bool ladder=flags.getBool("ladderEffect",0);

      if (ImGui::RadioButton("NTSC (7.67MHz)",clockSel==0)) {
        clockSel=0;
        altered=true;
      }
      if (ImGui::RadioButton("PAL (7.61MHz)",clockSel==1)) {
        clockSel=1;
        altered=true;
      }
      if (ImGui::RadioButton("FM Towns (8MHz)",clockSel==2)) {
        clockSel=2;
        altered=true;
      }
      if (ImGui::RadioButton("AtGames Genesis (6.13MHz)",clockSel==3)) {
        clockSel=3;
        altered=true;
      }
      if (ImGui::RadioButton("Sega System 32 (8.05MHz)",clockSel==4)) {
        clockSel=4;
        altered=true;
      }
      if (ImGui::Checkbox("Enable DAC distortion",&ladder)) {
        altered=true;
      }
      
      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",clockSel);
          flags.set("ladderEffect",ladder);
        });
      }
      break;
    }
    case DIV_SYSTEM_SMS: {
      int clockSel=flags.getInt("clockSel",0);
      int chipType=flags.getInt("chipType",0);
      bool noPhaseReset=flags.getBool("noPhaseReset",false);

      ImGui::Text("Clock rate:");
      if (ImGui::RadioButton("3.58MHz (NTSC)",clockSel==0)) {
        clockSel=0;
        altered=true;
      }
      if (ImGui::RadioButton("3.55MHz (PAL)",clockSel==1)) {
        clockSel=1;
        altered=true;
      }
      if (ImGui::RadioButton("4MHz (BBC Micro)",clockSel==2)) {
        clockSel=2;
        altered=true;
      }
      if (ImGui::RadioButton("1.79MHz (Half NTSC)",clockSel==3)) {
        clockSel=3;
        altered=true;
      }
      if (ImGui::RadioButton("3MHz (Exed Exes)",clockSel==4)) {
        clockSel=4;
        altered=true;
      }
      if (ImGui::RadioButton("2MHz (Sega System 1)",clockSel==5)) {
        clockSel=5;
        altered=true;
      }
      if (ImGui::RadioButton("447KHz (TI-99/4A)",clockSel==6)) {
        clockSel=6;
        altered=true;
      }
      ImGui::Text("Chip type:");
      if (ImGui::RadioButton("Sega VDP/Master System",chipType==0)) {
        chipType=0;
        altered=true;
      }
      if (ImGui::RadioButton("TI SN76489",chipType==1)) {
        chipType=1;
        altered=true;
      }
      if (ImGui::RadioButton("TI SN76489 with Atari-like short noise",chipType==2)) {
        chipType=2;
        altered=true;
      }
      if (ImGui::RadioButton("Game Gear",chipType==3)) {
        chipType=3;
        altered=true;
      }
      if (ImGui::RadioButton("TI SN76489A",chipType==4)) {
        chipType=4;
        altered=true;
      }
      if (ImGui::RadioButton("TI SN76496",chipType==5)) {
        chipType=5;
        altered=true;
      }
      if (ImGui::RadioButton("NCR 8496",chipType==6)) {
        chipType=6;
        altered=true;
      }
      if (ImGui::RadioButton("Tandy PSSJ 3-voice sound",chipType==7)) {
        chipType=7;
        altered=true;
      }
      if (ImGui::RadioButton("TI SN94624",chipType==8)) {
        chipType=8;
        altered=true;
      }
      if (ImGui::RadioButton("TI SN76494",chipType==9)) {
        chipType=9;
        altered=true;
      }

      if (ImGui::Checkbox("Disable noise period change phase reset",&noPhaseReset)) {
        altered=true;
      }

      if (altered) {
        e->lockSave([&]() {
          flags.set("clockSel",clockSel);
          flags.set("chipType",chipType);
          flags.set("noPhaseReset",noPhaseReset);
        });
      }
      break;
    }
    case DIV_SYSTEM_PCE: {
      bool clockSel=flags.getInt("clockSel",0);
      int chipType=flags.getInt("chipType",0);
      bool noAntiClick=flags.getBool("noAntiClick",false);

      if (ImGui::Checkbox("Pseudo-PAL",&clockSel)) {
        altered=true;
      }
      if (ImGui::Checkbox("Disable anti-click",&noAntiClick)) {
        altered=true;
      }
      ImGui::Text("Chip revision:");
      if (ImGui::RadioButton("HuC6280 (original)",chipType==0)) {
        chipType=0;
        altered=true;
      }
      if (ImGui::RadioButton("HuC6280A (SuperGrafx)",chipType==1)) {
        chipType=1;
        altered=true;
      }

      if (altered) {
        flags.set("clockSel",(int)clockSel);
        flags.set("chipType",chipType);
        flags.set("noPhaseReset",noAntiClick);
      }
      break;
    }
    case DIV_SYSTEM_SOUND_UNIT: {
      int clockSel=flags.getInt("clockSel",0);
      bool echo=flags.getBool("echo",false);
      bool swapEcho=flags.getBool("swapEcho",false);
      int sampleMemSize=flags.getInt("sampleMemSize",0);
      bool pdm=flags.getBool("pdm",false);
      int echoDelay=flags.getInt("echoDelay",0);
      int echoFeedback=flags.getInt("echoFeedback",0);
      int echoResolution=flags.getInt("echoResolution",0);
      int echoVol=(signed char)flags.getInt("echoVol",0);

      ImGui::Text("CPU rate:");
      if (ImGui::RadioButton("6.18MHz (NTSC)",clockSel==0)) {
        clockSel=0;
        altered=true;
      }
      if (ImGui::RadioButton("5.95MHz (PAL)",clockSel==1)) {
        clockSel=1;
        altered=true;
      }
      ImGui::Text("Sample memory:");
      if (ImGui::RadioButton("8K (rev A/B/E)",sampleMemSize==0)) {
        sampleMemSize=0;
        altered=true;
      }
      if (ImGui::RadioButton("64K (rev D/F)",sampleMemSize==1)) {
        sampleMemSize=1;
        altered=true;
      }
      ImGui::Text("DAC resolution:");
      if (ImGui::RadioButton("16-bit (rev A/B/D/F)",pdm==0)) {
        pdm=false;
        altered=true;
      }
      if (ImGui::RadioButton("1-bit PDM (rev C/E)",pdm==1)) {
        pdm=true;
        altered=true;
      }
      if (ImGui::Checkbox("Enable echo",&echo)) {
        altered=true;
      }
      if (ImGui::Checkbox("Swap echo channels",&swapEcho)) {
        altered=true;
      }
      ImGui::Text("Echo delay:");
      if (CWSliderInt("##EchoBufSize",&echoDelay,0,63)) {
        if (echoDelay<0) echoDelay=0;
        if (echoDelay>63) echoDelay=63;
        altered=true;
      } rightClickable
      ImGui::Text("Echo resolution:");
      if (CWSliderInt("##EchoResolution",&echoResolution,0,15)) {
        if (echoResolution<0) echoResolution=0;
        if (echoResolution>15) echoResolution=15;
        altered=true;
      } rightClickable
      ImGui::Text("Echo feedback:");
      if (CWSliderInt("##EchoFeedback",&echoFeedback,0,15)) {
        if (echoFeedback<0) echoFeedback=0;
        if (echoFeedback>15) echoFeedback=15;
        altered=true;
      } rightClickable
      ImGui::Text("Echo volume:");
      if (CWSliderInt("##EchoVolume",&echoVol,-128,127)) {
        if (echoVol<-128) echoVol=-128;
        if (echoVol>127) echoVol=127;
        altered=true;
      } rightClickable

      if (altered) {
        flags.set("clockSel",clockSel);
        flags.set("echo",echo);
        flags.set("swapEcho",swapEcho);
        flags.set("sampleMemSize",sampleMemSize);
        flags.set("pdm",pdm);
        flags.set("echoDelay",echoDelay);
        flags.set("echoFeedback",echoFeedback);
        flags.set("echoResolution",echoResolution);
        flags.set("echoVol",(unsigned char)echoVol);
      }
      break;
    }
    /*
    case DIV_SYSTEM_GB: {
      bool antiClick=flags&8;
      if (ImGui::Checkbox("Disable anti-click",&antiClick)) {
        copyOfFlags=(flags&(~8))|(antiClick<<3);
      }
      ImGui::Text("Chip revision:");
      if (ImGui::RadioButton("Original (DMG)",(flags&7)==0)) {
        copyOfFlags=(flags&(~7))|0;
      }
      if (ImGui::RadioButton("Game Boy Color (rev C)",(flags&7)==1)) {
        copyOfFlags=(flags&(~7))|1;
      }
      if (ImGui::RadioButton("Game Boy Color (rev E)",(flags&7)==2)) {
        copyOfFlags=(flags&(~7))|2;
      }
      if (ImGui::RadioButton("Game Boy Advance",(flags&7)==3)) {
        copyOfFlags=(flags&(~7))|3;
      }
      break;
    }
    case DIV_SYSTEM_OPLL:
    case DIV_SYSTEM_OPLL_DRUMS:
    case DIV_SYSTEM_VRC7: {
      ImGui::Text("Clock rate:");
      if (ImGui::RadioButton("NTSC (3.58MHz)",(flags&15)==0)) {
        copyOfFlags=(flags&(~15))|0;
      }
      if (ImGui::RadioButton("PAL (3.55MHz)",(flags&15)==1)) {
        copyOfFlags=(flags&(~15))|1;
      }
      if (ImGui::RadioButton("Arcade (4MHz)",(flags&15)==2)) {
        copyOfFlags=(flags&(~15))|2;
      }
      if (ImGui::RadioButton("Half NTSC (1.79MHz)",(flags&15)==3)) {
        copyOfFlags=(flags&(~15))|3;
      }
      if (type!=DIV_SYSTEM_VRC7) {
        ImGui::Text("Patch set:");
        if (ImGui::RadioButton("Yamaha YM2413",((flags>>4)&15)==0)) {
          copyOfFlags=(flags&(~0xf0))|0;
        }
        if (ImGui::RadioButton("Yamaha YMF281",((flags>>4)&15)==1)) {
          copyOfFlags=(flags&(~0xf0))|0x10;
        }
        if (ImGui::RadioButton("Yamaha YM2423",((flags>>4)&15)==2)) {
          copyOfFlags=(flags&(~0xf0))|0x20;
        }
        if (ImGui::RadioButton("Konami VRC7",((flags>>4)&15)==3)) {
          copyOfFlags=(flags&(~0xf0))|0x30;
        }
      }
      break;
    }
    case DIV_SYSTEM_YM2151:
      if (ImGui::RadioButton("NTSC/X16 (3.58MHz)",flags==0)) {
        copyOfFlags=0;
      }
      if (ImGui::RadioButton("PAL (3.55MHz)",flags==1)) {
        copyOfFlags=1;
      }
      if (ImGui::RadioButton("X1/X68000 (4MHz)",flags==2)) {
        copyOfFlags=2;
      }
      break;
    case DIV_SYSTEM_NES:
    case DIV_SYSTEM_VRC6:
    case DIV_SYSTEM_FDS:
    case DIV_SYSTEM_MMC5:
      if (ImGui::RadioButton("NTSC (1.79MHz)",flags==0)) {
        copyOfFlags=0;
      }
      if (ImGui::RadioButton("PAL (1.67MHz)",flags==1)) {
        copyOfFlags=1;
      }
      if (ImGui::RadioButton("Dendy (1.77MHz)",flags==2)) {
        copyOfFlags=2;
      }
      break;
    case DIV_SYSTEM_C64_8580:
    case DIV_SYSTEM_C64_6581:
      if (ImGui::RadioButton("NTSC (1.02MHz)",flags==0)) {
        copyOfFlags=0;
      }
      if (ImGui::RadioButton("PAL (0.99MHz)",flags==1)) {
        copyOfFlags=1;
      }
      if (ImGui::RadioButton("SSI 2001 (0.89MHz)",flags==2)) {
        copyOfFlags=2;
      }
      break;
    case DIV_SYSTEM_YM2610:
    case DIV_SYSTEM_YM2610_EXT:
    case DIV_SYSTEM_YM2610_FULL:
    case DIV_SYSTEM_YM2610_FULL_EXT:
    case DIV_SYSTEM_YM2610B:
    case DIV_SYSTEM_YM2610B_EXT:
      if (ImGui::RadioButton("8MHz (Neo Geo MVS)",(flags&0xff)==0)) {
        copyOfFlags=(flags&(~0xff))|0;
      }
      if (ImGui::RadioButton("8.06MHz (Neo Geo AES)",(flags&0xff)==1)) {
        copyOfFlags=(flags&(~0xff))|1;
      }
      break;
    case DIV_SYSTEM_AY8910:
    case DIV_SYSTEM_AY8930: {
      ImGui::Text("Clock rate:");
      if (ImGui::RadioButton("1.79MHz (ZX Spectrum NTSC/MSX)",(flags&15)==0)) {
        copyOfFlags=(flags&(~15))|0;
      }
      if (ImGui::RadioButton("1.77MHz (ZX Spectrum)",(flags&15)==1)) {
        copyOfFlags=(flags&(~15))|1;
      }
      if (ImGui::RadioButton("1.75MHz (ZX Spectrum)",(flags&15)==2)) {
        copyOfFlags=(flags&(~15))|2;
      }
      if (ImGui::RadioButton("2MHz (Atari ST/Sharp X1)",(flags&15)==3)) {
        copyOfFlags=(flags&(~15))|3;
      }
      if (ImGui::RadioButton("1.5MHz (Vectrex)",(flags&15)==4)) {
        copyOfFlags=(flags&(~15))|4;
      }
      if (ImGui::RadioButton("1MHz (Amstrad CPC)",(flags&15)==5)) {
        copyOfFlags=(flags&(~15))|5;
      }
      if (ImGui::RadioButton("0.89MHz (Pre-divided Sunsoft 5B)",(flags&15)==6)) {
        copyOfFlags=(flags&(~15))|6;
      }
      if (ImGui::RadioButton("1.67MHz (?)",(flags&15)==7)) {
        copyOfFlags=(flags&(~15))|7;
      }
      if (ImGui::RadioButton("0.83MHz (Pre-divided Sunsoft 5B on PAL)",(flags&15)==8)) {
        copyOfFlags=(flags&(~15))|8;
      }
      if (ImGui::RadioButton("1.10MHz (Gamate/VIC-20 PAL)",(flags&15)==9)) {
        copyOfFlags=(flags&(~15))|9;
      }
      if (ImGui::RadioButton("2^21Hz (Game Boy)",(flags&15)==10)) {
        copyOfFlags=(flags&(~15))|10;
      }
      if (ImGui::RadioButton("3.58MHz (Darky)",(flags&15)==11)) {
        copyOfFlags=(flags&(~15))|11;
      }
      if (ImGui::RadioButton("3.6MHz (Darky)",(flags&15)==12)) {
        copyOfFlags=(flags&(~15))|12;
      }
      if (ImGui::RadioButton("1.25MHz (Mag Max)",(flags&15)==13)) {
        copyOfFlags=(flags&(~15))|13;
      }
      if (ImGui::RadioButton("1.536MHz (Kyugo)",(flags&15)==14)) {
        copyOfFlags=(flags&(~15))|14;
      }
      if (type==DIV_SYSTEM_AY8910) {
        ImGui::Text("Chip type:");
        if (ImGui::RadioButton("AY-3-8910",(flags&0x30)==0)) {
          copyOfFlags=(flags&(~0x30))|0;
        }
        if (ImGui::RadioButton("YM2149(F)",(flags&0x30)==16)) {
          copyOfFlags=(flags&(~0x30))|16;
        }
        if (ImGui::RadioButton("Sunsoft 5B",(flags&0x30)==32)) {
          copyOfFlags=(flags&(~0x30))|32;
        }
        if (ImGui::RadioButton("AY-3-8914",(flags&0x30)==48)) {
          copyOfFlags=(flags&(~0x30))|48;
        }
      }
      bool stereo=flags&0x40;
      ImGui::BeginDisabled((type==DIV_SYSTEM_AY8910) && ((flags&0x30)==32));
      if (ImGui::Checkbox("Stereo##_AY_STEREO",&stereo)) {
        copyOfFlags=(flags&(~0x40))|(stereo?0x40:0);
      }
      if (stereo) {
        int sep=256-((flags>>8)&255);
        if (CWSliderInt("Separation",&sep,1,256)) {
          if (sep<1) sep=1;
          if (sep>256) sep=256;
          copyOfFlags=(flags&(~0xff00))|((256-sep)<<8);
        }
      }
      ImGui::EndDisabled();
      bool clockSel=flags&0x80;
      ImGui::BeginDisabled((type==DIV_SYSTEM_AY8910) && ((flags&0x30)!=16));
      if (ImGui::Checkbox("Half Clock divider##_AY_CLKSEL",&clockSel)) {
        copyOfFlags=(flags&(~0x80))|(clockSel?0x80:0);
      }
      ImGui::EndDisabled();
      break;
    }
    case DIV_SYSTEM_SAA1099:
      if (ImGui::RadioButton("SAM Coupé (8MHz)",flags==0)) {
        copyOfFlags=0;
      }
      if (ImGui::RadioButton("NTSC (7.15MHz)",flags==1)) {
        copyOfFlags=1;
      }
      if (ImGui::RadioButton("PAL (7.09MHz)",flags==2)) {
        copyOfFlags=2;
      }
      break;
    case DIV_SYSTEM_AMIGA: {
      ImGui::Text("Stereo separation:");
      int stereoSep=(flags>>8)&127;
      if (CWSliderInt("##StereoSep",&stereoSep,0,127)) {
        if (stereoSep<0) stereoSep=0;
        if (stereoSep>127) stereoSep=127;
        copyOfFlags=(flags&(~0x7f00))|((stereoSep&127)<<8);
      } rightClickable
      if (ImGui::RadioButton("Amiga 500 (OCS)",(flags&2)==0)) {
        copyOfFlags=flags&(~2);
      }
      if (ImGui::RadioButton("Amiga 1200 (AGA)",(flags&2)==2)) {
        copyOfFlags=(flags&(~2))|2;
      }
      sysPal=flags&1;
      if (ImGui::Checkbox("PAL",&sysPal)) {
        copyOfFlags=(flags&(~1))|(unsigned int)sysPal;
      }
      bool bypassLimits=flags&4;
      if (ImGui::Checkbox("Bypass frequency limits",&bypassLimits)) {
        copyOfFlags=(flags&(~4))|(bypassLimits<<2);
      }
      break;
    }
    case DIV_SYSTEM_TIA: {
      ImGui::Text("Mixing mode:");
      if (ImGui::RadioButton("Mono",(flags&6)==0)) {
        copyOfFlags=(flags&(~6));
      }
      if (ImGui::RadioButton("Mono (no distortion)",(flags&6)==2)) {
        copyOfFlags=(flags&(~6))|2;
      }
      if (ImGui::RadioButton("Stereo",(flags&6)==4)) {
        copyOfFlags=(flags&(~6))|4;
      }

      sysPal=flags&1;
      if (ImGui::Checkbox("PAL",&sysPal)) {
        copyOfFlags=(flags&(~1))|(unsigned int)sysPal;
      }
      break;
    }
    case DIV_SYSTEM_PCSPKR: {
      ImGui::Text("Speaker type:");
      if (ImGui::RadioButton("Unfiltered",(flags&3)==0)) {
        copyOfFlags=(flags&(~3))|0;
      }
      if (ImGui::RadioButton("Cone",(flags&3)==1)) {
        copyOfFlags=(flags&(~3))|1;
      }
      if (ImGui::RadioButton("Piezo",(flags&3)==2)) {
        copyOfFlags=(flags&(~3))|2;
      }
      if (ImGui::RadioButton("Use system beeper (Linux only!)",(flags&3)==3)) {
        copyOfFlags=(flags&(~3))|3;
      }
      break;
    }
    case DIV_SYSTEM_QSOUND: {
      ImGui::Text("Echo delay:");
      int echoBufSize=2725 - (flags & 4095);
      if (CWSliderInt("##EchoBufSize",&echoBufSize,0,2725)) {
        if (echoBufSize<0) echoBufSize=0;
        if (echoBufSize>2725) echoBufSize=2725;
        copyOfFlags=(flags & ~4095) | ((2725 - echoBufSize) & 4095);
      } rightClickable
      ImGui::Text("Echo feedback:");
      int echoFeedback=(flags>>12)&255;
      if (CWSliderInt("##EchoFeedback",&echoFeedback,0,255)) {
        if (echoFeedback<0) echoFeedback=0;
        if (echoFeedback>255) echoFeedback=255;
        copyOfFlags=(flags & ~(255 << 12)) | ((echoFeedback & 255) << 12);
      } rightClickable
      break;
    }
    case DIV_SYSTEM_X1_010: {
      ImGui::Text("Clock rate:");
      if (ImGui::RadioButton("16MHz (Seta 1)",(flags&15)==0)) {
        copyOfFlags=(flags&(~15))|0;
      }
      if (ImGui::RadioButton("16.67MHz (Seta 2)",(flags&15)==1)) {
        copyOfFlags=(flags&(~15))|1;
      }
      bool x1_010Stereo=flags&16;
      if (ImGui::Checkbox("Stereo",&x1_010Stereo)) {
        copyOfFlags=(flags&(~16))|(x1_010Stereo<<4);
      }
      break;
    }
    case DIV_SYSTEM_N163: {
      ImGui::Text("Clock rate:");
      if (ImGui::RadioButton("NTSC (1.79MHz)",(flags&15)==0)) {
        copyOfFlags=(flags&(~15))|0;
      }
      if (ImGui::RadioButton("PAL (1.67MHz)",(flags&15)==1)) {
        copyOfFlags=(flags&(~15))|1;
      }
      if (ImGui::RadioButton("Dendy (1.77MHz)",(flags&15)==2)) {
        copyOfFlags=(flags&(~15))|2;
      }
      ImGui::Text("Initial channel limit:");
      int initialChannelLimit=((flags>>4)&7)+1;
      if (CWSliderInt("##InitialChannelLimit",&initialChannelLimit,1,8)) {
        if (initialChannelLimit<1) initialChannelLimit=1;
        if (initialChannelLimit>8) initialChannelLimit=8;
        copyOfFlags=(flags & ~(7 << 4)) | (((initialChannelLimit-1) & 7) << 4);
        
      } rightClickable
      bool n163Multiplex=flags&128;
      if (ImGui::Checkbox("Disable hissing",&n163Multiplex)) {
        copyOfFlags=(flags&(~128))|(n163Multiplex<<7);
      }
      break;
    }
    case DIV_SYSTEM_OPN:
    case DIV_SYSTEM_OPN_EXT: {
      ImGui::Text("Clock rate:");
      if (ImGui::RadioButton("3.58MHz (NTSC)",(flags&31)==0)) {
        copyOfFlags=(flags&(~31))|0;
      }
      if (ImGui::RadioButton("3.54MHz (PAL)",(flags&31)==1)) {
        copyOfFlags=(flags&(~31))|1;
      }
      if (ImGui::RadioButton("4MHz",(flags&31)==2)) {
        copyOfFlags=(flags&(~31))|2;
      }
      if (ImGui::RadioButton("3MHz",(flags&31)==3)) {
        copyOfFlags=(flags&(~31))|3;
      }
      if (ImGui::RadioButton("3.9936MHz (PC-88/PC-98)",(flags&31)==4)) {
        copyOfFlags=(flags&(~31))|4;
      }
      if (ImGui::RadioButton("1.5MHz",(flags&31)==5)) {
        copyOfFlags=(flags&(~31))|5;
      }
      ImGui::Text("Output rate:");
      if (ImGui::RadioButton("FM: clock / 72, SSG: clock / 16",(flags&96)==0)) {
        copyOfFlags=(flags&(~96))|0;
      }
      if (ImGui::RadioButton("FM: clock / 36, SSG: clock / 8",(flags&96)==32)) {
        copyOfFlags=(flags&(~96))|32;
      }
      if (ImGui::RadioButton("FM: clock / 24, SSG: clock / 4",(flags&96)==64)) {
        copyOfFlags=(flags&(~96))|64;
      }
      break;
    }
    case DIV_SYSTEM_PC98:
    case DIV_SYSTEM_PC98_EXT: {
      ImGui::Text("Clock rate:");
      if (ImGui::RadioButton("8MHz (Arcade)",(flags&31)==0)) {
        copyOfFlags=(flags&(~31))|0;
      }
      if (ImGui::RadioButton("7.987MHz (PC-88/PC-98)",(flags&31)==1)) {
        copyOfFlags=(flags&(~31))|1;
      }
      ImGui::Text("Output rate:");
      if (ImGui::RadioButton("FM: clock / 144, SSG: clock / 32",(flags&96)==0)) {
        copyOfFlags=(flags&(~96))|0;
      }
      if (ImGui::RadioButton("FM: clock / 72, SSG: clock / 16",(flags&96)==32)) {
        copyOfFlags=(flags&(~96))|32;
      }
      if (ImGui::RadioButton("FM: clock / 48, SSG: clock / 8",(flags&96)==64)) {
        copyOfFlags=(flags&(~96))|64;
      }
      break;
    }
    case DIV_SYSTEM_RF5C68: {
      ImGui::Text("Clock rate:");
      if (ImGui::RadioButton("8MHz (FM Towns)",(flags&15)==0)) {
        copyOfFlags=(flags&(~15))|0;
      }
      if (ImGui::RadioButton("10MHz (Sega System 18)",(flags&15)==1)) {
        copyOfFlags=(flags&(~15))|1;
      }
      if (ImGui::RadioButton("12.5MHz (Sega CD/System 32)",(flags&15)==2)) {
        copyOfFlags=(flags&(~15))|2;
      }
      ImGui::Text("Chip type:");
      if (ImGui::RadioButton("RF5C68 (10-bit output)",((flags>>4)&15)==0)) {
        copyOfFlags=(flags&(~240))|0;
      }
      if (ImGui::RadioButton("RF5C164 (16-bit output)",((flags>>4)&15)==1)) {
        copyOfFlags=(flags&(~240))|16;
      }
      break;
    }
    case DIV_SYSTEM_MSM6258: {
      ImGui::Text("Clock rate:");
      if (ImGui::RadioButton("4MHz",flags==0)) {
        copyOfFlags=0;
      }
      if (ImGui::RadioButton("4.096MHz",flags==1)) {
        copyOfFlags=1;
      }
      if (ImGui::RadioButton("8MHz (X68000)",flags==2)) {
        copyOfFlags=2;
      }
      if (ImGui::RadioButton("8.192MHz",flags==3)) {
        copyOfFlags=3;
      }
      break;
    }
    case DIV_SYSTEM_MSM6295: {
      ImGui::Text("Clock rate:");
      if (ImGui::RadioButton("1MHz",(flags&127)==0)) {
        copyOfFlags=(flags&(~127))|0;
      }
      if (ImGui::RadioButton("1.056MHz",(flags&127)==1)) {
        copyOfFlags=(flags&(~127))|1;
      }
      if (ImGui::RadioButton("4MHz",(flags&127)==2)) {
        copyOfFlags=(flags&(~127))|2;
      }
      if (ImGui::RadioButton("4.224MHz",(flags&127)==3)) {
        copyOfFlags=(flags&(~127))|3;
      }
      if (ImGui::RadioButton("3.58MHz",(flags&127)==4)) {
        copyOfFlags=(flags&(~127))|4;
      }
      if (ImGui::RadioButton("1.79MHz",(flags&127)==5)) {
        copyOfFlags=(flags&(~127))|5;
      }
      if (ImGui::RadioButton("1.02MHz",(flags&127)==6)) {
        copyOfFlags=(flags&(~127))|6;
      }
      if (ImGui::RadioButton("0.89MHz",(flags&127)==7)) {
        copyOfFlags=(flags&(~127))|7;
      }
      if (ImGui::RadioButton("2MHz",(flags&127)==8)) {
        copyOfFlags=(flags&(~127))|8;
      }
      if (ImGui::RadioButton("2.112MHz",(flags&127)==9)) {
        copyOfFlags=(flags&(~127))|9;
      }
      if (ImGui::RadioButton("0.875MHz",(flags&127)==10)) {
        copyOfFlags=(flags&(~127))|10;
      }
      if (ImGui::RadioButton("0.9375MHz",(flags&127)==11)) {
        copyOfFlags=(flags&(~127))|11;
      }
      if (ImGui::RadioButton("1.5MHz",(flags&127)==12)) {
        copyOfFlags=(flags&(~127))|12;
      }
      if (ImGui::RadioButton("3MHz",(flags&127)==13)) {
        copyOfFlags=(flags&(~127))|13;
      }
      if (ImGui::RadioButton("1.193MHz (Atari)",(flags&127)==14)) {
        copyOfFlags=(flags&(~127))|14;
      }
      ImGui::Text("Output rate:");
      if (ImGui::RadioButton("clock / 132",(flags&128)==0)) {
        copyOfFlags=(flags&(~128))|0;
      }
      if (ImGui::RadioButton("clock / 165",(flags&128)==128)) {
        copyOfFlags=(flags&(~128))|128;
      }
      break;
    }
    case DIV_SYSTEM_SCC:
    case DIV_SYSTEM_SCC_PLUS: {
      ImGui::Text("Clock rate:");
      if (ImGui::RadioButton("1.79MHz (NTSC/MSX)",(flags&127)==0)) {
        copyOfFlags=(flags&(~127))|0;
      }
      if (ImGui::RadioButton("1.77MHz (PAL)",(flags&127)==1)) {
        copyOfFlags=(flags&(~127))|1;
      }
      if (ImGui::RadioButton("1.5MHz (Arcade)",(flags&127)==2)) {
        copyOfFlags=(flags&(~127))|2;
      }
      if (ImGui::RadioButton("2MHz",(flags&127)==3)) {
        copyOfFlags=(flags&(~127))|3;
      }
      break;
    }
    case DIV_SYSTEM_OPL:
    case DIV_SYSTEM_OPL_DRUMS:
    case DIV_SYSTEM_OPL2:
    case DIV_SYSTEM_OPL2_DRUMS:
    case DIV_SYSTEM_Y8950:
    case DIV_SYSTEM_Y8950_DRUMS: {
      ImGui::Text("Clock rate:");
      if (ImGui::RadioButton("3.58MHz (NTSC)",(flags&255)==0)) {
        copyOfFlags=(flags&(~255))|0;
      }
      if (ImGui::RadioButton("3.54MHz (PAL)",(flags&255)==1)) {
        copyOfFlags=(flags&(~255))|1;
      }
      if (ImGui::RadioButton("4MHz",(flags&255)==2)) {
        copyOfFlags=(flags&(~255))|2;
      }
      if (ImGui::RadioButton("3MHz",(flags&255)==3)) {
        copyOfFlags=(flags&(~255))|3;
      }
      if (ImGui::RadioButton("3.9936MHz (PC-88/PC-98)",(flags&255)==4)) {
        copyOfFlags=(flags&(~255))|4;
      }
      if (ImGui::RadioButton("3.5MHz",(flags&255)==5)) {
        copyOfFlags=(flags&(~255))|5;
      }
      break;
    }
    case DIV_SYSTEM_OPL3:
    case DIV_SYSTEM_OPL3_DRUMS: {
      ImGui::Text("Clock rate:");
      if (ImGui::RadioButton("14.32MHz (NTSC)",(flags&255)==0)) {
        copyOfFlags=(flags&(~255))|0;
      }
      if (ImGui::RadioButton("14.19MHz (PAL)",(flags&255)==1)) {
        copyOfFlags=(flags&(~255))|1;
      }
      if (ImGui::RadioButton("14MHz",(flags&255)==2)) {
        copyOfFlags=(flags&(~255))|2;
      }
      if (ImGui::RadioButton("16MHz",(flags&255)==3)) {
        copyOfFlags=(flags&(~255))|3;
      }
      if (ImGui::RadioButton("15MHz",(flags&255)==4)) {
        copyOfFlags=(flags&(~255))|4;
      }
      break;
    }
    case DIV_SYSTEM_YMZ280B: {
      ImGui::Text("Clock rate:");
      if (ImGui::RadioButton("16.9344MHz",(flags&255)==0)) {
        copyOfFlags=(flags&(~255))|0;
      }
      if (ImGui::RadioButton("14.32MHz (NTSC)",(flags&255)==1)) {
        copyOfFlags=(flags&(~255))|1;
      }
      if (ImGui::RadioButton("14.19MHz (PAL)",(flags&255)==2)) {
        copyOfFlags=(flags&(~255))|2;
      }
      if (ImGui::RadioButton("16MHz",(flags&255)==3)) {
        copyOfFlags=(flags&(~255))|3;
      }
      if (ImGui::RadioButton("16.67MHz",(flags&255)==4)) {
        copyOfFlags=(flags&(~255))|4;
      }
      if (ImGui::RadioButton("14MHz",(flags&255)==5)) {
        copyOfFlags=(flags&(~255))|5;
      }
      break;
    }
    case DIV_SYSTEM_PCM_DAC: {
      // default to 44100Hz 16-bit stereo
      if (!flags) copyOfFlags=flags=0x1f0000|44099;
      int sampRate=(flags&65535)+1;
      int bitDepth=((flags>>16)&15)+1;
      bool stereo=(flags>>20)&1;
      ImGui::Text("Output rate:");
      if (CWSliderInt("##SampRate",&sampRate,1000,65536)) {
        if (sampRate<1000) sampRate=1000;
        if (sampRate>65536) sampRate=65536;
        copyOfFlags=(flags&(~65535))|(sampRate-1);
      } rightClickable
      ImGui::Text("Output bit depth:");
      if (CWSliderInt("##BitDepth",&bitDepth,1,16)) {
        if (bitDepth<1) bitDepth=1;
        if (bitDepth>16) bitDepth=16;
        copyOfFlags=(flags&(~(15<<16)))|((bitDepth-1)<<16);
      } rightClickable
      if (ImGui::Checkbox("Stereo",&stereo)) {
        copyOfFlags=(flags&(~(1<<20)))|(stereo<<20);
      }
      break;
    }
    case DIV_SYSTEM_SNES: {
      ImGui::Text("Volume scale:");
      int vsL=127-(flags&127);
      int vsR=127-((flags>>8)&127);
      if (CWSliderInt("Left##VolScaleL",&vsL,0,127)) {
        if (vsL<0) vsL=0;
        if (vsL>127) vsL=127;
        copyOfFlags=(flags&(~0x7f))|(127-vsL);
      } rightClickable
      if (CWSliderInt("Right##VolScaleL",&vsR,0,127)) {
        if (vsR<0) vsR=0;
        if (vsR>127) vsR=127;
        copyOfFlags=(flags&(~0x7f00))|((127-vsR)<<8);
      } rightClickable
      break;
    }
    case DIV_SYSTEM_SWAN:
    case DIV_SYSTEM_VERA:
    case DIV_SYSTEM_BUBSYS_WSG:
    case DIV_SYSTEM_YMU759:
    case DIV_SYSTEM_PET:
    case DIV_SYSTEM_T6W28:
      ImGui::Text("nothing to configure");
      break;
    default:
      if (ImGui::Checkbox("PAL",&sysPal)) {
        copyOfFlags=sysPal;
      }
      break;
    */
    default:
      ImGui::Text("Wait... is that right? No... I don't think so.");

      if (ImGui::Button("But it is right!")) {
        showError("https://github.com/tildearrow/furnace/issues/new");
      }
      break;
  }

  if (altered) {
    if (chan>=0) {
      e->updateSysFlags(chan,restart);
      if (e->song.autoSystem) {
        autoDetectSystem();
      }
      updateWindowTitle();
    }
    MARK_MODIFIED;
  }

  return altered;
}
