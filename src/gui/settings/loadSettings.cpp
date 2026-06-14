/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
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

// loadSettings.cpp: the code which loads and stores settings from/to the config file.

#include "../gui.h"
#include "../guiConst.h"

// yes, this array must live here.
const char* audioBackends[]={
  "JACK",
  "SDL",
  "PortAudio",
  // pipe (invalid choice in GUI)
  "Uhh, can you explain to me what exactly you were trying to do?",
  "ASIO"
};

// default setting values

#if defined(_WIN32) || defined(__APPLE__) || defined(IS_MOBILE)
#define POWER_SAVE_DEFAULT 1
#else
// currently off on Linux/other due to Mesa catch-up behavior.
#define POWER_SAVE_DEFAULT 0
#endif

#ifdef HAVE_FREETYPE
#define FONT_BACKEND_DEFAULT 1
#else
#define FONT_BACKEND_DEFAULT 0
#endif

#if defined(__HAIKU__) || defined(IS_MOBILE) || (defined(_WIN32) && !defined(_WIN64))
// NFD doesn't support Haiku
// NFD doesn't support Windows XP either
#define SYS_FILE_DIALOG_DEFAULT 0
#else
#define SYS_FILE_DIALOG_DEFAULT 1
#endif


#define clampSetting(x,minV,maxV) \
  if (x<minV) { \
    x=minV; \
  } \
  if (x>maxV) { \
    x=maxV; \
  }

/**************************************
 *                                    *
 *           CONFIG READING           *
 *                                    *
 **************************************/

// this function consists of two sections.
// the first reads config values from a config object.
// the next one clamps these values.

void FurnaceGUI::readConfig(DivConfig& conf, FurnaceGUISettingGroups groups) {
  /// READING
  if (groups&GUI_SETTINGS_GENERAL) {
    settings.renderDriver=conf.getString("renderDriver","");
    settings.noDMFCompat=conf.getBool("noDMFCompat",0);

    settings.dpiScale=conf.getFloat("dpiScale",0.0f);

    settings.initialSysName=conf.getString("initialSysName","");

    // initial system
    String initialSys2=conf.getString("initialSys2","");
    bool oldVol=conf.getInt("configVersion",DIV_ENGINE_VERSION)<135;
    if (initialSys2.empty()) {
      initialSys2=e->decodeSysDesc(conf.getString("initialSys",""));
      oldVol=false;
    }
    settings.initialSys.clear();
    settings.initialSys.loadFromBase64(initialSys2.c_str());
    if (settings.initialSys.getInt("id0",0)==0) {
      settings.initialSys.clear();
      settings.initialSys.set("id0",e->systemToFileFur(DIV_SYSTEM_YM2612));
      settings.initialSys.set("vol0",1.0f);
      settings.initialSys.set("pan0",0.0f);
      settings.initialSys.set("fr0",0.0f);
      settings.initialSys.set("flags0","");
      settings.initialSys.set("id1",e->systemToFileFur(DIV_SYSTEM_SMS));
      settings.initialSys.set("vol1",0.5f);
      settings.initialSys.set("pan1",0);
      settings.initialSys.set("fr1",0);
      settings.initialSys.set("flags1","");
    } else {
      if (oldVol) {
        for (int i=0; settings.initialSys.getInt(fmt::sprintf("id%d",i),0); i++) {
          float newVol=settings.initialSys.getInt(fmt::sprintf("vol%d",i),64);
          float newPan=settings.initialSys.getInt(fmt::sprintf("pan%d",i),0);
          newVol/=64.0f;
          newPan/=127.0f;
          settings.initialSys.set(fmt::sprintf("vol%d",i),newVol);
          settings.initialSys.set(fmt::sprintf("pan%d",i),newPan);
        }
        conf.set("initialSys2",settings.initialSys.toBase64());
        conf.set("configVersion",DIV_ENGINE_VERSION);
      }
    }

    settings.noThreadedInput=conf.getBool("noThreadedInput",0);
    settings.powerSave=conf.getBool("powerSave",POWER_SAVE_DEFAULT);
    settings.eventDelay=conf.getBool("eventDelay",0);

    settings.renderBackend=conf.getString("renderBackend",GUI_BACKEND_DEFAULT_NAME);
    settings.renderClearPos=conf.getBool("renderClearPos",0);

    settings.glRedSize=conf.getInt("glRedSize",8);
    settings.glGreenSize=conf.getInt("glGreenSize",8);
    settings.glBlueSize=conf.getInt("glBlueSize",8);
    settings.glAlphaSize=conf.getInt("glAlphaSize",0);
    settings.glDepthSize=conf.getInt("glDepthSize",24);
    settings.glSetBS=conf.getBool("glSetBS",0);
    settings.glStencilSize=conf.getInt("glStencilSize",0);
    settings.glBufferSize=conf.getInt("glBufferSize",32);
    settings.glDoubleBuffer=conf.getBool("glDoubleBuffer",1);

    settings.vsync=conf.getBool("vsync",1);
    settings.frameRateLimit=conf.getInt("frameRateLimit",100);
    settings.displayRenderTime=conf.getBool("displayRenderTime",0);

    settings.chanOscThreads=conf.getInt("chanOscThreads",0);
    settings.renderPoolThreads=conf.getInt("renderPoolThreads",0);
    settings.shaderOsc=conf.getBool("shaderOsc",0);
    settings.writeInsNames=conf.getBool("writeInsNames",0);
    settings.readInsNames=conf.getBool("readInsNames",1);
    settings.defaultAuthorName=conf.getString("defaultAuthorName","");

    settings.hiddenSystems=conf.getBool("hiddenSystems",0);
    settings.mswEnabled=conf.getBool("mswEnabled",0);
    settings.allowEditDocking=conf.getBool("allowEditDocking",1);
#ifndef FLATPAK_WORKAROUNDS
    settings.sysFileDialog=conf.getBool("sysFileDialog",SYS_FILE_DIALOG_DEFAULT);
#endif
    settings.displayAllInsTypes=conf.getBool("displayAllInsTypes",0);

    settings.blankIns=conf.getBool("blankIns",0);
    settings.warnNotePassthrough=conf.getBool("warnNotePassthrough",0);

    settings.saveWindowPos=conf.getBool("saveWindowPos",1);

    settings.saveUnusedPatterns=conf.getBool("saveUnusedPatterns",0);
    settings.maxRecentFile=conf.getInt("maxRecentFile",10);

    settings.doubleClickTime=conf.getFloat("doubleClickTime",0.3f);
    settings.disableFadeIn=conf.getBool("disableFadeIn",0);
    settings.alwaysPlayIntro=conf.getInt("alwaysPlayIntro",0);
    settings.noMaximizeWorkaround=conf.getBool("noMaximizeWorkaround",0);

    settings.compress=conf.getBool("compress",1);
    settings.newSongBehavior=conf.getInt("newSongBehavior",0);
    settings.playOnLoad=conf.getInt("playOnLoad",0);
    settings.centerPopup=conf.getBool("centerPopup",1);

    settings.vibrationStrength=conf.getFloat("vibrationStrength",0.5f);
    settings.vibrationLength=conf.getInt("vibrationLength",20);

    settings.s3mOPL3=conf.getBool("s3mOPL3",1);
    settings.sampleImportInstDetune=conf.getBool("sampleImportInstDetune",0);

    settings.backupEnable=conf.getBool("backupEnable",1);
    settings.backupInterval=conf.getInt("backupInterval",30);
    settings.backupMaxCopies=conf.getInt("backupMaxCopies",5);

    settings.autoFillSave=conf.getBool("autoFillSave",0);

    settings.locale=conf.getString("locale","");

    settings.backgroundPlay=conf.getBool("backgroundPlay",0);
  }

  if (groups&GUI_SETTINGS_AUDIO) {
    settings.audioEngine=(conf.getString("audioEngine","SDL")=="SDL")?1:0;
    if (conf.getString("audioEngine","SDL")=="JACK") {
      settings.audioEngine=DIV_AUDIO_JACK;
    } else if (conf.getString("audioEngine","SDL")=="PortAudio") {
      settings.audioEngine=DIV_AUDIO_PORTAUDIO;
    } else if (conf.getString("audioEngine","SDL")=="ASIO") {
      settings.audioEngine=DIV_AUDIO_ASIO;
    } else {
      settings.audioEngine=DIV_AUDIO_SDL;
    }
    settings.audioDevice=conf.getString("audioDevice","");
    settings.sdlAudioDriver=conf.getString("sdlAudioDriver","");
    settings.audioQuality=conf.getInt("audioQuality",0);
    settings.audioHiPass=conf.getBool("audioHiPass",1);
    settings.audioBufSize=conf.getInt("audioBufSize",1024);
    settings.audioRate=conf.getInt("audioRate",44100);
    settings.audioChans=conf.getInt("audioChans",2);

    settings.lowLatency=conf.getBool("lowLatency",0);

    settings.metroVol=conf.getInt("metroVol",100);
    settings.sampleVol=conf.getInt("sampleVol",50);

    settings.wasapiEx=conf.getBool("wasapiEx",0);

    settings.clampSamples=conf.getBool("clampSamples",0);
    settings.forceMono=conf.getBool("forceMono",0);
  }

  if (groups&GUI_SETTINGS_MIDI) {
    settings.midiInDevice=conf.getString("midiInDevice","");
    settings.midiOutDevice=conf.getString("midiOutDevice","");
    settings.midiOutClock=conf.getBool("midiOutClock",0);
    settings.midiOutTime=conf.getBool("midiOutTime",0);
    settings.midiOutProgramChange=conf.getBool("midiOutProgramChange",0);
    settings.midiOutMode=conf.getInt("midiOutMode",1);
    settings.midiOutTimeRate=conf.getInt("midiOutTimeRate",0);
  }

  if (groups&GUI_SETTINGS_KEYBOARD) {
    // keybinds
    for (int i=0; i<GUI_ACTION_MAX; i++) {
      if (guiActions[i].isNotABind()) continue;

      // use { -1 } as a fallback to let us know there was an issue
      actionKeys[i]=conf.getIntList(String("keybind_GUI_ACTION_")+String(guiActions[i].name),{-1});
      if (actionKeys[i].size()==1 && actionKeys[i][0]==-1) {
        actionKeys[i]=guiActions[i].defaultBind;
      } else {
        for (size_t j=0; j<actionKeys[i].size(); j++) {
          if (actionKeys[i][j]==-1) {
            actionKeys[i].erase(actionKeys[i].begin()+j);
          }
        }
      }
    }

    decodeKeyMap(noteKeys,conf.getString("noteKeys",DEFAULT_NOTE_KEYS));
    decompileNoteKeys();
  }

  if (groups&GUI_SETTINGS_BEHAVIOR) {
    settings.soloAction=conf.getInt("soloAction",0);
    settings.ctrlWheelModifier=conf.getInt("ctrlWheelModifier",0);
    settings.pullDeleteBehavior=conf.getBool("pullDeleteBehavior",1);
    settings.wrapHorizontal=conf.getInt("wrapHorizontal",0);
    settings.wrapVertical=conf.getInt("wrapVertical",0);

    settings.stepOnDelete=conf.getBool("stepOnDelete",0);
    settings.scrollStep=conf.getInt("scrollStep",0);
    settings.avoidRaisingPattern=conf.getBool("avoidRaisingPattern",0);
    settings.insFocusesPattern=conf.getBool("insFocusesPattern",1);
    settings.stepOnInsert=conf.getBool("stepOnInsert",0);
    settings.effectCursorDir=conf.getInt("effectCursorDir",1);
    settings.cursorPastePos=conf.getBool("cursorPastePos",1);

    settings.effectDeletionAltersValue=conf.getBool("effectDeletionAltersValue",1);

    settings.pushNibble=conf.getBool("pushNibble",0);
    settings.scrollChangesOrder=conf.getInt("scrollChangesOrder",0);
    settings.cursorMoveNoScroll=conf.getBool("cursorMoveNoScroll",0);

    settings.notePreviewBehavior=conf.getInt("notePreviewBehavior",1);

    settings.absorbInsInput=conf.getBool("absorbInsInput",0);

    settings.moveWindowTitle=conf.getBool("moveWindowTitle",1);

    settings.doubleClickColumn=conf.getBool("doubleClickColumn",1);
    settings.dragMovesSelection=conf.getInt("dragMovesSelection",2);
    settings.draggableDataView=conf.getBool("draggableDataView",1);

    settings.cursorFollowsOrder=conf.getBool("cursorFollowsOrder",1);

    settings.insertBehavior=conf.getBool("insertBehavior",1);
    settings.pullDeleteRow=conf.getBool("pullDeleteRow",1);
    settings.cursorFollowsWheel=conf.getInt("cursorFollowsWheel",0);
    settings.cursorWheelStep=conf.getInt("cursorWheelStep",0);
    settings.removeInsOff=conf.getBool("removeInsOff",0);
    settings.removeVolOff=conf.getBool("removeVolOff",0);
    settings.insTypeMenu=conf.getBool("insTypeMenu",1);

    settings.selectAssetOnLoad=conf.getBool("selectAssetOnLoad",1);

    settings.inputRepeat=conf.getBool("inputRepeat",1);
  }

  if (groups&GUI_SETTINGS_FONT) {
    settings.mainFontSize=conf.getInt("mainFontSize",GUI_FONT_SIZE_DEFAULT);
    settings.headFontSize=conf.getInt("headFontSize",27);
    settings.headFontSize2=conf.getInt("headFontSize2",20);
    settings.headFontSize3=conf.getInt("headFontSize3",16);
    settings.headFontSize4=conf.getInt("headFontSize4",13);
    settings.patFontSize=conf.getInt("patFontSize",GUI_FONT_SIZE_DEFAULT);
    settings.iconSize=conf.getInt("iconSize",GUI_ICON_SIZE_DEFAULT);

    settings.mainFont=conf.getInt("mainFont",GUI_MAIN_FONT_DEFAULT);
    settings.headFont=conf.getInt("headFont",0);
    settings.patFont=conf.getInt("patFont",GUI_PAT_FONT_DEFAULT);
    settings.mainFontPath=conf.getString("mainFontPath","");
    settings.headFontPath=conf.getString("headFontPath","");
    settings.patFontPath=conf.getString("patFontPath","");

    settings.loadFallback=conf.getBool("loadFallback",1);
    settings.loadFallbackPat=conf.getBool("loadFallbackPat",1);

    settings.fontBackend=conf.getInt("fontBackend",FONT_BACKEND_DEFAULT);
    settings.fontHinting=conf.getInt("fontHinting",GUI_FONT_HINTING_DEFAULT);
    settings.fontBitmap=conf.getBool("fontBitmap",0);
    settings.fontAutoHint=conf.getInt("fontAutoHint",1);
    settings.fontAntiAlias=conf.getBool("fontAntiAlias",GUI_FONT_ANTIALIAS_DEFAULT);
    settings.fontOversample=conf.getInt("fontOversample",GUI_OVERSAMPLE_DEFAULT);
  }

  if (groups&GUI_SETTINGS_APPEARANCE) {
    settings.oscRoundedCorners=conf.getBool("oscRoundedCorners",GUI_DECORATIONS_DEFAULT);
    settings.oscTakesEntireWindow=conf.getBool("oscTakesEntireWindow",0);
    settings.oscBorder=conf.getBool("oscBorder",1);
    settings.oscEscapesBoundary=conf.getBool("oscEscapesBoundary",0);
    settings.oscMono=conf.getBool("oscMono",1);
    settings.oscAntiAlias=conf.getBool("oscAntiAlias",1);
    settings.oscLineSize=conf.getFloat("oscLineSize",1.0f);

    settings.songNotesWrap=conf.getBool("songNotesWrap",0);

    settings.rackShowLEDs=conf.getBool("rackShowLEDs",1);

    settings.mixerStyle=conf.getInt("mixerStyle",1);
    settings.mixerLayout=conf.getInt("mixerLayout",0);

    settings.channelColors=conf.getInt("channelColors",1);
    settings.channelTextColors=conf.getInt("channelTextColors",0);
    settings.channelStyle=conf.getInt("channelStyle",1);
    settings.channelVolStyle=conf.getInt("channelVolStyle",0);
    settings.channelFeedbackStyle=conf.getInt("channelFeedbackStyle",1);
    settings.channelFeedbackGamma=conf.getFloat("channelFeedbackGamma",1.0f);
    settings.channelFont=conf.getInt("channelFont",1);
    settings.channelTextCenter=conf.getBool("channelTextCenter",1);

    settings.roundedWindows=conf.getBool("roundedWindows",GUI_DECORATIONS_DEFAULT);
    settings.roundedButtons=conf.getBool("roundedButtons",GUI_DECORATIONS_DEFAULT);
    settings.roundedMenus=conf.getBool("roundedMenus",0);
    settings.roundedTabs=conf.getBool("roundedTabs",GUI_DECORATIONS_DEFAULT);
    settings.roundedScrollbars=conf.getBool("roundedScrollbars",GUI_DECORATIONS_DEFAULT);

    settings.separateFMColors=conf.getBool("separateFMColors",0);
    settings.insEditColorize=conf.getBool("insEditColorize",0);

    settings.overflowHighlight=conf.getBool("overflowHighlight",0);
    settings.flatNotes=conf.getBool("flatNotes",0);
    settings.germanNotation=conf.getBool("germanNotation",0);

    settings.frameBorders=conf.getBool("frameBorders",0);

    settings.noteOffLabel=conf.getString("noteOffLabel","OFF");
    settings.noteRelLabel=conf.getString("noteRelLabel","===");
    settings.macroRelLabel=conf.getString("macroRelLabel","REL");
    settings.emptyLabel=conf.getString("emptyLabel","...");
    settings.emptyLabel2=conf.getString("emptyLabel2","..");

    settings.noteCellSpacing=conf.getInt("noteCellSpacing",0);
    settings.insCellSpacing=conf.getInt("insCellSpacing",0);
    settings.volCellSpacing=conf.getInt("volCellSpacing",0);
    settings.effectCellSpacing=conf.getInt("effectCellSpacing",0);
    settings.effectValCellSpacing=conf.getInt("effectValCellSpacing",0);

    settings.patRowsBase=conf.getInt("patRowsBase",0);
    settings.orderRowsBase=conf.getInt("orderRowsBase",1);
    settings.fmNames=conf.getInt("fmNames",0);
    settings.statusDisplay=conf.getInt("statusDisplay",0);
    settings.playbackTime=conf.getBool("playbackTime",1);
    settings.viewPrevPattern=conf.getBool("viewPrevPattern",1);
    settings.susPosition=conf.getInt("susPosition",0);

    settings.titleBarInfo=conf.getInt("titleBarInfo",1);
    settings.titleBarSys=conf.getBool("titleBarSys",1);

    settings.oplStandardWaveNames=conf.getBool("oplStandardWaveNames",0);

    settings.horizontalDataView=conf.getBool("horizontalDataView",0);
    settings.noMultiSystem=conf.getBool("noMultiSystem",0);
    settings.oldMacroVSlider=conf.getBool("oldMacroVSlider",0);
    settings.unsignedDetune=conf.getBool("unsignedDetune",0);
    settings.centerPattern=conf.getBool("centerPattern",0);
    settings.ordersCursor=conf.getBool("ordersCursor",1);
    settings.oneDigitEffects=conf.getBool("oneDigitEffects",0);
    settings.orderButtonPos=conf.getInt("orderButtonPos",2);
    settings.memUsageUnit=conf.getInt("memUsageUnit",1);
    settings.capitalMenuBar=conf.getBool("capitalMenuBar",0);
    settings.insIconsStyle=conf.getInt("insIconsStyle",1);
    settings.sysSeparators=conf.getBool("sysSeparators",1);

    settings.autoMacroStepSize=conf.getInt("autoMacroStepSize",0);
  }

  if (groups&GUI_SETTINGS_LAYOUTS) {
    settings.fmLayout=conf.getInt("fmLayout",4);
    settings.exportOptionsLayout=conf.getInt("exportOptionsLayout",1);
    settings.unifiedDataView=conf.getBool("unifiedDataView",0);
    settings.macroLayout=conf.getInt("macroLayout",0);
    settings.controlLayout=conf.getInt("controlLayout",3);
    settings.classicChipOptions=conf.getBool("classicChipOptions",0);
  }

  if (groups&GUI_SETTINGS_COLOR) {
    settings.guiColorsBase=conf.getInt("guiColorsBase",0);
    settings.guiColorsShading=conf.getInt("guiColorsShading",0);
    settings.basicColors=conf.getBool("basicColors",1);

    // colors
    for (int i=0; i<GUI_COLOR_MAX; i++) {
      uiColors[i]=ImGui::ColorConvertU32ToFloat4(conf.getInt(guiColors[i].name,guiColors[i].defaultColor));
    }
  }

  if (groups&GUI_SETTINGS_EMULATION) {
    settings.arcadeCore=conf.getInt("arcadeCore",0);
    settings.ym2612Core=conf.getInt("ym2612Core",0);
    settings.snCore=conf.getInt("snCore",0);
    settings.nesCore=conf.getInt("nesCore",0);
    settings.fdsCore=conf.getInt("fdsCore",0);
    settings.c64Core=conf.getInt("c64Core",0);
    settings.pokeyCore=conf.getInt("pokeyCore",1);
    settings.opn1Core=conf.getInt("opn1Core",1);
    settings.opnaCore=conf.getInt("opnaCore",1);
    settings.opnbCore=conf.getInt("opnbCore",1);
    settings.opl2Core=conf.getInt("opl2Core",0);
    settings.opl3Core=conf.getInt("opl3Core",0);
    settings.opl4Core=conf.getInt("opl4Core",0);
    settings.esfmCore=conf.getInt("esfmCore",0);
    settings.opllCore=conf.getInt("opllCore",0);
    settings.ayCore=conf.getInt("ayCore",0);
    settings.swanCore=conf.getInt("swanCore",0);

    settings.dsidQuality=conf.getInt("dsidQuality",3);
    settings.gbQuality=conf.getInt("gbQuality",3);
    settings.pnQuality=conf.getInt("pnQuality",3);
    settings.saaQuality=conf.getInt("saaQuality",3);

    settings.arcadeCoreRender=conf.getInt("arcadeCoreRender",1);
    settings.ym2612CoreRender=conf.getInt("ym2612CoreRender",0);
    settings.snCoreRender=conf.getInt("snCoreRender",0);
    settings.nesCoreRender=conf.getInt("nesCoreRender",0);
    settings.fdsCoreRender=conf.getInt("fdsCoreRender",1);
    settings.c64CoreRender=conf.getInt("c64CoreRender",1);
    settings.pokeyCoreRender=conf.getInt("pokeyCoreRender",1);
    settings.opn1CoreRender=conf.getInt("opn1CoreRender",1);
    settings.opnaCoreRender=conf.getInt("opnaCoreRender",1);
    settings.opnbCoreRender=conf.getInt("opnbCoreRender",1);
    settings.opl2CoreRender=conf.getInt("opl2CoreRender",0);
    settings.opl3CoreRender=conf.getInt("opl3CoreRender",0);
    settings.opl4CoreRender=conf.getInt("opl4CoreRender",0);
    settings.esfmCoreRender=conf.getInt("esfmCoreRender",0);
    settings.opllCoreRender=conf.getInt("opllCoreRender",0);
    settings.ayCoreRender=conf.getInt("ayCoreRender",0);
    settings.swanCoreRender=conf.getInt("swanCoreRender",0);

    settings.dsidQualityRender=conf.getInt("dsidQualityRender",3);
    settings.gbQualityRender=conf.getInt("gbQualityRender",3);
    settings.pnQualityRender=conf.getInt("pnQualityRender",3);
    settings.saaQualityRender=conf.getInt("saaQualityRender",3);

    settings.pcSpeakerOutMethod=conf.getInt("pcSpeakerOutMethod",0);

    settings.yrw801Path=conf.getString("yrw801Path","");
    settings.tg100Path=conf.getString("tg100Path","");
    settings.mu5Path=conf.getString("mu5Path","");
  }

  /// CLAMPING
  clampSetting(settings.mainFontSize,2,96);
  clampSetting(settings.headFontSize,2,96);
  clampSetting(settings.headFontSize2,2,96);
  clampSetting(settings.headFontSize3,2,96);
  clampSetting(settings.headFontSize4,2,96);
  clampSetting(settings.patFontSize,2,96);
  clampSetting(settings.iconSize,2,48);
  clampSetting(settings.audioEngine,0,4);
  clampSetting(settings.audioQuality,0,1);
  clampSetting(settings.audioBufSize,32,4096);
  clampSetting(settings.audioRate,8000,384000);
  clampSetting(settings.audioChans,1,16);
  clampSetting(settings.arcadeCore,0,1);
  clampSetting(settings.ym2612Core,0,2);
  clampSetting(settings.snCore,0,1);
  clampSetting(settings.nesCore,0,1);
  clampSetting(settings.fdsCore,0,1);
  clampSetting(settings.c64Core,0,2);
  clampSetting(settings.pokeyCore,0,1);
  clampSetting(settings.opn1Core,0,2);
  clampSetting(settings.opnaCore,0,2);
  clampSetting(settings.opnbCore,0,2);
  clampSetting(settings.opl2Core,0,2);
  clampSetting(settings.opl3Core,0,2);
  clampSetting(settings.opl4Core,0,1);
  clampSetting(settings.esfmCore,0,1);
  clampSetting(settings.opllCore,0,1);
  clampSetting(settings.ayCore,0,1);
  clampSetting(settings.swanCore,0,1);
  clampSetting(settings.dsidQuality,0,5);
  clampSetting(settings.gbQuality,0,5);
  clampSetting(settings.pnQuality,0,5);
  clampSetting(settings.saaQuality,0,5);
  clampSetting(settings.arcadeCoreRender,0,1);
  clampSetting(settings.ym2612CoreRender,0,2);
  clampSetting(settings.snCoreRender,0,1);
  clampSetting(settings.nesCoreRender,0,1);
  clampSetting(settings.fdsCoreRender,0,1);
  clampSetting(settings.c64CoreRender,0,2);
  clampSetting(settings.pokeyCoreRender,0,1);
  clampSetting(settings.opn1CoreRender,0,2);
  clampSetting(settings.opnaCoreRender,0,2);
  clampSetting(settings.opnbCoreRender,0,2);
  clampSetting(settings.opl2CoreRender,0,2);
  clampSetting(settings.opl3CoreRender,0,2);
  clampSetting(settings.opl4CoreRender,0,1);
  clampSetting(settings.esfmCoreRender,0,1);
  clampSetting(settings.opllCoreRender,0,1);
  clampSetting(settings.ayCoreRender,0,1);
  clampSetting(settings.swanCoreRender,0,1);
  clampSetting(settings.dsidQualityRender,0,5);
  clampSetting(settings.gbQualityRender,0,5);
  clampSetting(settings.pnQualityRender,0,5);
  clampSetting(settings.saaQualityRender,0,5);
  clampSetting(settings.pcSpeakerOutMethod,0,4);
  clampSetting(settings.mainFont,0,6);
  clampSetting(settings.patFont,0,6);
  clampSetting(settings.patRowsBase,0,1);
  clampSetting(settings.orderRowsBase,0,1);
  clampSetting(settings.soloAction,0,2);
  clampSetting(settings.ctrlWheelModifier,0,3);
  clampSetting(settings.wrapHorizontal,0,2);
  clampSetting(settings.wrapVertical,0,3);
  clampSetting(settings.fmNames,0,2);
  clampSetting(settings.scrollStep,0,1);
  clampSetting(settings.controlLayout,0,3);
  clampSetting(settings.statusDisplay,0,3);
  clampSetting(settings.dpiScale,0.0f,4.0f);
  clampSetting(settings.guiColorsBase,0,1);
  clampSetting(settings.guiColorsShading,0,100);
  clampSetting(settings.fmLayout,0,7);
  clampSetting(settings.susPosition,0,3);
  clampSetting(settings.effectCursorDir,0,2);
  clampSetting(settings.titleBarInfo,0,3);
  clampSetting(settings.metroVol,0,200);
  clampSetting(settings.sampleVol,0,100);
  clampSetting(settings.scrollChangesOrder,0,2);
  clampSetting(settings.notePreviewBehavior,0,3);
  clampSetting(settings.noteCellSpacing,0,32);
  clampSetting(settings.insCellSpacing,0,32);
  clampSetting(settings.volCellSpacing,0,32);
  clampSetting(settings.effectCellSpacing,0,32);
  clampSetting(settings.effectValCellSpacing,0,32);
  clampSetting(settings.dragMovesSelection,0,5);
  clampSetting(settings.channelColors,0,2);
  clampSetting(settings.channelTextColors,0,2);
  clampSetting(settings.channelStyle,0,5);
  clampSetting(settings.channelVolStyle,0,4);
  clampSetting(settings.channelFeedbackStyle,0,4);
  clampSetting(settings.channelFeedbackGamma,0.0f,2.0f);
  clampSetting(settings.channelFont,0,1);
  clampSetting(settings.maxRecentFile,0,30);
  clampSetting(settings.midiOutMode,0,2);
  clampSetting(settings.midiOutTimeRate,0,4);
  clampSetting(settings.macroLayout,0,4);
  clampSetting(settings.doubleClickTime,0.02,1.0);
  clampSetting(settings.alwaysPlayIntro,0,3);
  clampSetting(settings.orderButtonPos,0,2);
  clampSetting(settings.newSongBehavior,0,1);
  clampSetting(settings.memUsageUnit,0,1);
  clampSetting(settings.cursorFollowsWheel,0,2);
  clampSetting(settings.playOnLoad,0,2);
  clampSetting(settings.insIconsStyle,0,2);
  clampSetting(settings.exportOptionsLayout,0,2);
  clampSetting(settings.chanOscThreads,0,256);
  clampSetting(settings.renderPoolThreads,0,DIV_MAX_CHIPS);
  clampSetting(settings.fontBackend,0,1);
  clampSetting(settings.fontHinting,0,3);
  clampSetting(settings.fontAutoHint,0,2);
  clampSetting(settings.fontOversample,1,3);
  clampSetting(settings.oscLineSize,0.25f,16.0f);
  clampSetting(settings.mixerStyle,0,2);
  clampSetting(settings.mixerLayout,0,1);
  clampSetting(settings.cursorWheelStep,0,2);
  clampSetting(settings.frameRateLimit,0,1000);
  clampSetting(settings.vibrationStrength,0.0f,1.0f);
  clampSetting(settings.vibrationLength,10,500);
  clampSetting(settings.glRedSize,0,32);
  clampSetting(settings.glGreenSize,0,32);
  clampSetting(settings.glBlueSize,0,32);
  clampSetting(settings.glAlphaSize,0,32);
  clampSetting(settings.glDepthSize,0,128);
  clampSetting(settings.glStencilSize,0,32);
  clampSetting(settings.glBufferSize,0,128);
  clampSetting(settings.backupInterval,10,86400);
  clampSetting(settings.backupMaxCopies,1,100);
  clampSetting(settings.autoMacroStepSize,0,2);
}

/**************************************
 *                                    *
 *           CONFIG WRITING           *
 *                                    *
 **************************************/

// this is simpler. it just writes values to a config.

void FurnaceGUI::writeConfig(DivConfig& conf, FurnaceGUISettingGroups groups) {
  // general
  if (groups&GUI_SETTINGS_GENERAL) {
    conf.set("renderDriver",settings.renderDriver);
    conf.set("noDMFCompat",settings.noDMFCompat);

    conf.set("dpiScale",settings.dpiScale);

    conf.set("initialSys2",settings.initialSys.toBase64());
    conf.set("initialSysName",settings.initialSysName);

    conf.set("noThreadedInput",settings.noThreadedInput);
    conf.set("powerSave",settings.powerSave);
    conf.set("eventDelay",settings.eventDelay);

    conf.set("renderBackend",settings.renderBackend);
    conf.set("renderClearPos",settings.renderClearPos);

    conf.set("glRedSize",settings.glRedSize);
    conf.set("glGreenSize",settings.glGreenSize);
    conf.set("glBlueSize",settings.glBlueSize);
    conf.set("glAlphaSize",settings.glAlphaSize);
    conf.set("glDepthSize",settings.glDepthSize);
    conf.set("glSetBS",settings.glSetBS);
    conf.set("glStencilSize",settings.glStencilSize);
    conf.set("glBufferSize",settings.glBufferSize);
    conf.set("glDoubleBuffer",settings.glDoubleBuffer);

    conf.set("vsync",settings.vsync);
    conf.set("frameRateLimit",settings.frameRateLimit);
    conf.set("displayRenderTime",settings.displayRenderTime);

    conf.set("chanOscThreads",settings.chanOscThreads);
    conf.set("renderPoolThreads",settings.renderPoolThreads);
    conf.set("shaderOsc",settings.shaderOsc);
    conf.set("writeInsNames",settings.writeInsNames);
    conf.set("readInsNames",settings.readInsNames);
    conf.set("defaultAuthorName",settings.defaultAuthorName);

    conf.set("hiddenSystems",settings.hiddenSystems);
    conf.set("mswEnabled",settings.mswEnabled);
    conf.set("allowEditDocking",settings.allowEditDocking);
#ifndef FLATPAK_WORKAROUNDS
    conf.set("sysFileDialog",settings.sysFileDialog);
#endif
    conf.set("displayAllInsTypes",settings.displayAllInsTypes);

    conf.set("blankIns",settings.blankIns);
    conf.set("warnNotePassthrough",settings.warnNotePassthrough);

    conf.set("saveWindowPos",settings.saveWindowPos);

    conf.set("saveUnusedPatterns",settings.saveUnusedPatterns);
    conf.set("maxRecentFile",settings.maxRecentFile);

    conf.set("doubleClickTime",settings.doubleClickTime);
    conf.set("disableFadeIn",settings.disableFadeIn);
    conf.set("alwaysPlayIntro",settings.alwaysPlayIntro);
    conf.set("noMaximizeWorkaround",settings.noMaximizeWorkaround);

    conf.set("compress",settings.compress);
    conf.set("newSongBehavior",settings.newSongBehavior);
    conf.set("playOnLoad",settings.playOnLoad);
    conf.set("centerPopup",settings.centerPopup);

    conf.set("vibrationStrength",settings.vibrationStrength);
    conf.set("vibrationLength",settings.vibrationLength);

    conf.set("s3mOPL3",settings.s3mOPL3);
    conf.set("sampleImportInstDetune",settings.sampleImportInstDetune);

    conf.set("backupEnable",settings.backupEnable);
    conf.set("backupInterval",settings.backupInterval);
    conf.set("backupMaxCopies",settings.backupMaxCopies);

    conf.set("autoFillSave",settings.autoFillSave);

    conf.set("locale",settings.locale);

    conf.set("backgroundPlay",settings.backgroundPlay);
  }

  // audio
  if (groups&GUI_SETTINGS_AUDIO) {
    conf.set("audioEngine",String(audioBackends[settings.audioEngine]));
    conf.set("audioDevice",settings.audioDevice);
    conf.set("sdlAudioDriver",settings.sdlAudioDriver);
    conf.set("audioQuality",settings.audioQuality);
    conf.set("audioHiPass",settings.audioHiPass);
    conf.set("audioBufSize",settings.audioBufSize);
    conf.set("audioRate",settings.audioRate);
    conf.set("audioChans",settings.audioChans);

    conf.set("lowLatency",settings.lowLatency);

    conf.set("metroVol",settings.metroVol);
    conf.set("sampleVol",settings.sampleVol);

    conf.set("wasapiEx",settings.wasapiEx);

    conf.set("clampSamples",settings.clampSamples);
    conf.set("forceMono",settings.forceMono);
  }

  // MIDI
  if (groups&GUI_SETTINGS_MIDI) {
    conf.set("midiInDevice",settings.midiInDevice);
    conf.set("midiOutDevice",settings.midiOutDevice);
    conf.set("midiOutClock",settings.midiOutClock);
    conf.set("midiOutTime",settings.midiOutTime);
    conf.set("midiOutProgramChange",settings.midiOutProgramChange);
    conf.set("midiOutMode",settings.midiOutMode);
    conf.set("midiOutTimeRate",settings.midiOutTimeRate);
  }

  // keyboard
  if (groups&GUI_SETTINGS_KEYBOARD) {
    // keybinds
    for (int i=0; i<GUI_ACTION_MAX; i++) {
      if (guiActions[i].isNotABind()) continue;
      conf.set(String("keybind_GUI_ACTION_")+String(guiActions[i].name),actionKeys[i]);
    }

    compileNoteKeys();
    conf.set("noteKeys",encodeKeyMap(noteKeys));
  }

  // behavior
  if (groups&GUI_SETTINGS_BEHAVIOR) {
    conf.set("soloAction",settings.soloAction);
    conf.set("ctrlWheelModifier",settings.ctrlWheelModifier);
    conf.set("pullDeleteBehavior",settings.pullDeleteBehavior);
    conf.set("wrapHorizontal",settings.wrapHorizontal);
    conf.set("wrapVertical",settings.wrapVertical);

    conf.set("stepOnDelete",settings.stepOnDelete);
    conf.set("scrollStep",settings.scrollStep);
    conf.set("avoidRaisingPattern",settings.avoidRaisingPattern);
    conf.set("insFocusesPattern",settings.insFocusesPattern);
    conf.set("stepOnInsert",settings.stepOnInsert);
    conf.set("effectCursorDir",settings.effectCursorDir);
    conf.set("cursorPastePos",settings.cursorPastePos);

    conf.set("effectDeletionAltersValue",settings.effectDeletionAltersValue);

    conf.set("pushNibble",settings.pushNibble);
    conf.set("scrollChangesOrder",settings.scrollChangesOrder);
    conf.set("cursorMoveNoScroll",settings.cursorMoveNoScroll);

    conf.set("notePreviewBehavior",settings.notePreviewBehavior);

    conf.set("absorbInsInput",settings.absorbInsInput);

    conf.set("moveWindowTitle",settings.moveWindowTitle);

    conf.set("doubleClickColumn",settings.doubleClickColumn);
    conf.set("dragMovesSelection",settings.dragMovesSelection);
    conf.set("draggableDataView",settings.draggableDataView);

    conf.set("cursorFollowsOrder",settings.cursorFollowsOrder);

    conf.set("insertBehavior",settings.insertBehavior);
    conf.set("pullDeleteRow",settings.pullDeleteRow);
    conf.set("cursorFollowsWheel",settings.cursorFollowsWheel);
    conf.set("cursorWheelStep",settings.cursorWheelStep);
    conf.set("removeInsOff",settings.removeInsOff);
    conf.set("removeVolOff",settings.removeVolOff);
    conf.set("insTypeMenu",settings.insTypeMenu);

    conf.set("selectAssetOnLoad",settings.selectAssetOnLoad);

    conf.set("inputRepeat",settings.inputRepeat);
  }

  // font
  if (groups&GUI_SETTINGS_FONT) {
    conf.set("mainFontSize",settings.mainFontSize);
    conf.set("headFontSize",settings.headFontSize);
    conf.set("headFontSize2",settings.headFontSize2);
    conf.set("headFontSize3",settings.headFontSize3);
    conf.set("headFontSize4",settings.headFontSize4);
    conf.set("patFontSize",settings.patFontSize);
    conf.set("iconSize",settings.iconSize);

    conf.set("mainFont",settings.mainFont);
    conf.set("headFont",settings.headFont);
    conf.set("patFont",settings.patFont);
    conf.set("mainFontPath",settings.mainFontPath);
    conf.set("headFontPath",settings.headFontPath);
    conf.set("patFontPath",settings.patFontPath);

    conf.set("loadFallback",settings.loadFallback);
    conf.set("loadFallbackPat",settings.loadFallbackPat);

    conf.set("fontBackend",settings.fontBackend);
    conf.set("fontHinting",settings.fontHinting);
    conf.set("fontBitmap",settings.fontBitmap);
    conf.set("fontAutoHint",settings.fontAutoHint);
    conf.set("fontAntiAlias",settings.fontAntiAlias);
    conf.set("fontOversample",settings.fontOversample);
  }

  // appearance
  if (groups&GUI_SETTINGS_APPEARANCE) {
    conf.set("oscRoundedCorners",settings.oscRoundedCorners);
    conf.set("oscTakesEntireWindow",settings.oscTakesEntireWindow);
    conf.set("oscBorder",settings.oscBorder);
    conf.set("oscEscapesBoundary",settings.oscEscapesBoundary);
    conf.set("oscMono",settings.oscMono);
    conf.set("oscAntiAlias",settings.oscAntiAlias);
    conf.set("oscLineSize",settings.oscLineSize);

    conf.set("songNotesWrap",settings.songNotesWrap);

    conf.set("rackShowLEDs",settings.rackShowLEDs);

    conf.set("mixerStyle",settings.mixerStyle);
    conf.set("mixerLayout",settings.mixerLayout);

    conf.set("channelColors",settings.channelColors);
    conf.set("channelTextColors",settings.channelTextColors);
    conf.set("channelStyle",settings.channelStyle);
    conf.set("channelVolStyle",settings.channelVolStyle);
    conf.set("channelFeedbackStyle",settings.channelFeedbackStyle);
    conf.set("channelFeedbackGamma",settings.channelFeedbackGamma);
    conf.set("channelFont",settings.channelFont);
    conf.set("channelTextCenter",settings.channelTextCenter);

    conf.set("roundedWindows",settings.roundedWindows);
    conf.set("roundedButtons",settings.roundedButtons);
    conf.set("roundedMenus",settings.roundedMenus);
    conf.set("roundedTabs",settings.roundedTabs);
    conf.set("roundedScrollbars",settings.roundedScrollbars);

    conf.set("separateFMColors",settings.separateFMColors);
    conf.set("insEditColorize",settings.insEditColorize);

    conf.set("overflowHighlight",settings.overflowHighlight);
    conf.set("flatNotes",settings.flatNotes);
    conf.set("germanNotation",settings.germanNotation);

    conf.set("frameBorders",settings.frameBorders);

    conf.set("noteOffLabel",settings.noteOffLabel);
    conf.set("noteRelLabel",settings.noteRelLabel);
    conf.set("macroRelLabel",settings.macroRelLabel);
    conf.set("emptyLabel",settings.emptyLabel);
    conf.set("emptyLabel2",settings.emptyLabel2);

    conf.set("noteCellSpacing",settings.noteCellSpacing);
    conf.set("insCellSpacing",settings.insCellSpacing);
    conf.set("volCellSpacing",settings.volCellSpacing);
    conf.set("effectCellSpacing",settings.effectCellSpacing);
    conf.set("effectValCellSpacing",settings.effectValCellSpacing);

    conf.set("patRowsBase",settings.patRowsBase);
    conf.set("orderRowsBase",settings.orderRowsBase);
    conf.set("fmNames",settings.fmNames);
    conf.set("statusDisplay",settings.statusDisplay);
    conf.set("playbackTime",settings.playbackTime);
    conf.set("viewPrevPattern",settings.viewPrevPattern);
    conf.set("susPosition",settings.susPosition);

    conf.set("titleBarInfo",settings.titleBarInfo);
    conf.set("titleBarSys",settings.titleBarSys);

    conf.set("oplStandardWaveNames",settings.oplStandardWaveNames);

    conf.set("horizontalDataView",settings.horizontalDataView);
    conf.set("noMultiSystem",settings.noMultiSystem);
    conf.set("oldMacroVSlider",settings.oldMacroVSlider);
    conf.set("unsignedDetune",settings.unsignedDetune);
    conf.set("centerPattern",settings.centerPattern);
    conf.set("ordersCursor",settings.ordersCursor);
    conf.set("oneDigitEffects",settings.oneDigitEffects);
    conf.set("orderButtonPos",settings.orderButtonPos);
    conf.set("memUsageUnit",settings.memUsageUnit);
    conf.set("capitalMenuBar",settings.capitalMenuBar);
    conf.set("insIconsStyle",settings.insIconsStyle);
    conf.set("sysSeparators",settings.sysSeparators);
    conf.set("autoMacroStepSize",settings.autoMacroStepSize);
  }

  // layout
  if (groups&GUI_SETTINGS_LAYOUTS) {
    conf.set("fmLayout",settings.fmLayout);
    conf.set("exportOptionsLayout",settings.exportOptionsLayout);
    conf.set("unifiedDataView",settings.unifiedDataView);
    conf.set("macroLayout",settings.macroLayout);
    conf.set("controlLayout",settings.controlLayout);
    conf.set("classicChipOptions",settings.classicChipOptions);
  }

  // color
  if (groups&GUI_SETTINGS_COLOR) {
    conf.set("guiColorsBase",settings.guiColorsBase);
    conf.set("guiColorsShading",settings.guiColorsShading);
    conf.set("basicColors",settings.basicColors);

    // colors
    for (int i=0; i<GUI_COLOR_MAX; i++) {
      conf.set(guiColors[i].name,(int)ImGui::ColorConvertFloat4ToU32(uiColors[i]));
    }
  }

  // emulation
  if (groups&GUI_SETTINGS_EMULATION) {
    conf.set("arcadeCore",settings.arcadeCore);
    conf.set("ym2612Core",settings.ym2612Core);
    conf.set("snCore",settings.snCore);
    conf.set("nesCore",settings.nesCore);
    conf.set("fdsCore",settings.fdsCore);
    conf.set("c64Core",settings.c64Core);
    conf.set("pokeyCore",settings.pokeyCore);
    conf.set("opn1Core",settings.opn1Core);
    conf.set("opnaCore",settings.opnaCore);
    conf.set("opnbCore",settings.opnbCore);
    conf.set("opl2Core",settings.opl2Core);
    conf.set("opl3Core",settings.opl3Core);
    conf.set("opl4Core",settings.opl4Core);
    conf.set("esfmCore",settings.esfmCore);
    conf.set("opllCore",settings.opllCore);
    conf.set("ayCore",settings.ayCore);
    conf.set("swanCore",settings.swanCore);

    conf.set("dsidQuality",settings.dsidQuality);
    conf.set("gbQuality",settings.gbQuality);
    conf.set("pnQuality",settings.pnQuality);
    conf.set("saaQuality",settings.saaQuality);

    conf.set("arcadeCoreRender",settings.arcadeCoreRender);
    conf.set("ym2612CoreRender",settings.ym2612CoreRender);
    conf.set("snCoreRender",settings.snCoreRender);
    conf.set("nesCoreRender",settings.nesCoreRender);
    conf.set("fdsCoreRender",settings.fdsCoreRender);
    conf.set("c64CoreRender",settings.c64CoreRender);
    conf.set("pokeyCoreRender",settings.pokeyCoreRender);
    conf.set("opn1CoreRender",settings.opn1CoreRender);
    conf.set("opnaCoreRender",settings.opnaCoreRender);
    conf.set("opnbCoreRender",settings.opnbCoreRender);
    conf.set("opl2CoreRender",settings.opl2CoreRender);
    conf.set("opl3CoreRender",settings.opl3CoreRender);
    conf.set("opl4CoreRender",settings.opl4CoreRender);
    conf.set("esfmCoreRender",settings.esfmCoreRender);
    conf.set("opllCoreRender",settings.opllCoreRender);
    conf.set("ayCoreRender",settings.ayCoreRender);
    conf.set("swanCoreRender",settings.swanCoreRender);

    conf.set("dsidQualityRender",settings.dsidQualityRender);
    conf.set("gbQualityRender",settings.gbQualityRender);
    conf.set("pnQualityRender",settings.pnQualityRender);
    conf.set("saaQualityRender",settings.saaQualityRender);

    conf.set("pcSpeakerOutMethod",settings.pcSpeakerOutMethod);

    conf.set("yrw801Path",settings.yrw801Path);
    conf.set("tg100Path",settings.tg100Path);
    conf.set("mu5Path",settings.mu5Path);
  }
}
