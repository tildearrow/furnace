/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
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

#ifndef _FUR_GUI_H
#define _FUR_GUI_H

#include "../engine/engine.h"
#include "../engine/workPool.h"
#include "../engine/waveSynth.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include <SDL.h>
#include <fftw3.h>
#include <stdint.h>
#include <initializer_list>
#include <future>
#include <memory>
#include <tuple>
#include "../pch.h"

#include "fileDialog.h"

#define FURNACE_APP_ID "org.tildearrow.furnace"

#define rightClickable if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) ImGui::SetKeyboardFocusHere(-1);
#define ctrlWheeling (isCtrlWheelModifierHeld() && wheelY!=0)

#define handleUnimportant if (settings.insFocusesPattern && patternOpen) {nextWindow=GUI_WINDOW_PATTERN;}
#define unimportant(x) if (x) {handleUnimportant}

#define MARK_MODIFIED modified=true;
#define WAKE_UP drawHalt=5;

#define RESET_WAVE_MACRO_ZOOM \
  for (DivInstrument* _wi: e->song.ins) { \
    _wi->std.waveMacro.vZoom=-1; \
    _wi->std.waveMacro.vScroll=-1; \
  }

#define CHECK_LONG_HOLD (mobileUI && ImGui::GetIO().MouseDown[ImGuiMouseButton_Left] && ImGui::GetIO().MouseDownDuration[ImGuiMouseButton_Left]>=longThreshold && ImGui::GetIO().MouseDownDurationPrev[ImGuiMouseButton_Left]<longThreshold && ImGui::GetIO().MouseDragMaxDistanceSqr[ImGuiMouseButton_Left]<=ImGui::GetIO().ConfigInertialScrollToleranceSqr)
#define CHECK_BUTTON_LONG_HOLD (mobileUI && ImGui::GetIO().MouseDown[ImGuiMouseButton_Left] && ImGui::GetIO().MouseDownDuration[ImGuiMouseButton_Left]>=buttonLongThreshold && ImGui::GetIO().MouseDownDurationPrev[ImGuiMouseButton_Left]<buttonLongThreshold)
// for now
#define NOTIFY_LONG_HOLD \
  if (vibrator && vibratorAvailable) { \
    if (SDL_HapticRumblePlay(vibrator,settings.vibrationStrength,settings.vibrationLength)!=0) { \
      logV("could not vibrate: %s!",SDL_GetError()); \
    } \
  } else { \
    fputc(7,stderr); /* bell */ \
    logI("beep!"); \
  }

#define BIND_FOR(x) getKeyName(actionKeys[x],true).c_str()

#define FM_PREVIEW_SIZE 512

#define CHECK_HIDDEN_SYSTEM(x) \
  (x==DIV_SYSTEM_YMU759 || x==DIV_SYSTEM_UPD1771C || x==DIV_SYSTEM_DUMMY || x==DIV_SYSTEM_SEGAPCM_COMPAT || x==DIV_SYSTEM_PONG)

enum FurnaceGUIRenderBackend {
  GUI_BACKEND_SDL=0,
  GUI_BACKEND_GL3,
  GUI_BACKEND_GL2,
  GUI_BACKEND_GL1,
  GUI_BACKEND_DX11,
  GUI_BACKEND_DX9,
  GUI_BACKEND_METAL,
  GUI_BACKEND_SOFTWARE
};

#ifdef HAVE_RENDER_DX11
  #define GUI_BACKEND_DEFAULT GUI_BACKEND_DX11
  #define GUI_BACKEND_DEFAULT_NAME "DirectX 11"
#else
  #ifdef HAVE_RENDER_GL
    #ifdef SUPPORT_XP
      #define GUI_BACKEND_DEFAULT GUI_BACKEND_GL1
      #define GUI_BACKEND_DEFAULT_NAME "OpenGL 1.1"
    #else
      #ifdef USE_GLES
        #define GUI_BACKEND_DEFAULT GUI_BACKEND_GL3
        #define GUI_BACKEND_DEFAULT_NAME "OpenGL ES 2.0"
      #else
        #define GUI_BACKEND_DEFAULT GUI_BACKEND_GL3
        #define GUI_BACKEND_DEFAULT_NAME "OpenGL 3.0"
      #endif
    #endif
  #else
    #ifdef HAVE_RENDER_SDL
      #define GUI_BACKEND_DEFAULT GUI_BACKEND_SDL
      #define GUI_BACKEND_DEFAULT_NAME "SDL"
    #else
      #define GUI_BACKEND_DEFAULT GUI_BACKEND_SOFTWARE
      #define GUI_BACKEDN_DEFAULT_NAME "Software"
    #endif
  #endif
#endif

#ifdef SUPPORT_XP
#define GUI_WIDTH_DEFAULT 800
#define GUI_HEIGHT_DEFAULT 600
#define GUI_MAIN_FONT_DEFAULT 3
#define GUI_PAT_FONT_DEFAULT 3
#define GUI_FONT_SIZE_DEFAULT 13
#define GUI_ICON_SIZE_DEFAULT 12
#define GUI_OVERSAMPLE_DEFAULT 1
#define GUI_FONT_ANTIALIAS_DEFAULT 0
#define GUI_FONT_HINTING_DEFAULT 1
#define GUI_DECORATIONS_DEFAULT 0
#else
#define GUI_WIDTH_DEFAULT 1280
#define GUI_HEIGHT_DEFAULT 800
#define GUI_MAIN_FONT_DEFAULT 0
#define GUI_PAT_FONT_DEFAULT 0
#define GUI_FONT_SIZE_DEFAULT 18
#define GUI_ICON_SIZE_DEFAULT 16
#define GUI_OVERSAMPLE_DEFAULT 2
#define GUI_FONT_ANTIALIAS_DEFAULT 1
#define GUI_FONT_HINTING_DEFAULT 0
#define GUI_DECORATIONS_DEFAULT 1
#endif

#ifdef HAVE_MOMO
#define ngettext momo_ngettext
#endif

#define GUI_EDIT_OCTAVE_MIN -5
#define GUI_EDIT_OCTAVE_MAX 7

// TODO:
// - add colors for FM envelope and waveform
// - maybe add "alternate" color for FM modulators/carriers (a bit difficult)
enum FurnaceGUIColors {
  GUI_COLOR_BACKGROUND=0,
  GUI_COLOR_FRAME_BACKGROUND,
  GUI_COLOR_FRAME_BACKGROUND_CHILD,
  GUI_COLOR_FRAME_BACKGROUND_POPUP,
  GUI_COLOR_MODAL_BACKDROP,
  GUI_COLOR_HEADER,
  GUI_COLOR_TEXT,
  GUI_COLOR_TEXT_DISABLED,
  GUI_COLOR_ACCENT_PRIMARY,
  GUI_COLOR_ACCENT_SECONDARY,
  GUI_COLOR_TITLE_INACTIVE,
  GUI_COLOR_TITLE_COLLAPSED,
  GUI_COLOR_MENU_BAR,
  GUI_COLOR_BORDER,
  GUI_COLOR_BORDER_SHADOW,
  GUI_COLOR_SCROLL_BACKGROUND,
  GUI_COLOR_SCROLL,
  GUI_COLOR_SCROLL_HOVER,
  GUI_COLOR_SCROLL_ACTIVE,
  GUI_COLOR_SEPARATOR,
  GUI_COLOR_SEPARATOR_HOVER,
  GUI_COLOR_SEPARATOR_ACTIVE,
  GUI_COLOR_DOCKING_PREVIEW,
  GUI_COLOR_DOCKING_EMPTY,
  GUI_COLOR_TABLE_HEADER,
  GUI_COLOR_TABLE_BORDER_HARD,
  GUI_COLOR_TABLE_BORDER_SOFT,
  GUI_COLOR_DRAG_DROP_TARGET,
  GUI_COLOR_NAV_HIGHLIGHT,
  GUI_COLOR_NAV_WIN_HIGHLIGHT,
  GUI_COLOR_NAV_WIN_BACKDROP,
  GUI_COLOR_PLOT_LINES,
  GUI_COLOR_PLOT_LINES_HOVER,
  GUI_COLOR_PLOT_HISTOGRAM,
  GUI_COLOR_PLOT_HISTOGRAM_HOVER,

  GUI_COLOR_BUTTON,
  GUI_COLOR_BUTTON_HOVER,
  GUI_COLOR_BUTTON_ACTIVE,
  GUI_COLOR_TAB,
  GUI_COLOR_TAB_HOVER,
  GUI_COLOR_TAB_ACTIVE,
  GUI_COLOR_TAB_UNFOCUSED,
  GUI_COLOR_TAB_UNFOCUSED_ACTIVE,
  GUI_COLOR_IMGUI_HEADER,
  GUI_COLOR_IMGUI_HEADER_HOVER,
  GUI_COLOR_IMGUI_HEADER_ACTIVE,
  GUI_COLOR_RESIZE_GRIP,
  GUI_COLOR_RESIZE_GRIP_HOVER,
  GUI_COLOR_RESIZE_GRIP_ACTIVE,
  GUI_COLOR_WIDGET_BACKGROUND,
  GUI_COLOR_WIDGET_BACKGROUND_HOVER,
  GUI_COLOR_WIDGET_BACKGROUND_ACTIVE,
  GUI_COLOR_SLIDER_GRAB,
  GUI_COLOR_SLIDER_GRAB_ACTIVE,
  GUI_COLOR_TITLE_BACKGROUND_ACTIVE,
  GUI_COLOR_CHECK_MARK,
  GUI_COLOR_TEXT_SELECTION,
  GUI_COLOR_TABLE_ROW_EVEN,
  GUI_COLOR_TABLE_ROW_ODD,

  GUI_COLOR_TOGGLE_OFF,
  GUI_COLOR_TOGGLE_ON,
  GUI_COLOR_EDITING,
  GUI_COLOR_EDITING_CLONE,
  GUI_COLOR_SONG_LOOP,
  GUI_COLOR_DESTRUCTIVE,
  GUI_COLOR_WARNING,
  GUI_COLOR_ERROR,

  GUI_COLOR_FILE_DIR,
  GUI_COLOR_FILE_SONG_NATIVE,
  GUI_COLOR_FILE_SONG_IMPORT,
  GUI_COLOR_FILE_INSTR,
  GUI_COLOR_FILE_AUDIO,
  GUI_COLOR_FILE_WAVE,
  GUI_COLOR_FILE_VGM,
  GUI_COLOR_FILE_ZSM,
  GUI_COLOR_FILE_FONT,
  GUI_COLOR_FILE_OTHER,

  GUI_COLOR_OSC_BG1,
  GUI_COLOR_OSC_BG2,
  GUI_COLOR_OSC_BG3,
  GUI_COLOR_OSC_BG4,
  GUI_COLOR_OSC_BORDER,
  GUI_COLOR_OSC_WAVE,
  GUI_COLOR_OSC_WAVE_PEAK,
  GUI_COLOR_OSC_WAVE_CH0,
  GUI_COLOR_OSC_WAVE_CH1,
  GUI_COLOR_OSC_WAVE_CH2,
  GUI_COLOR_OSC_WAVE_CH3,
  GUI_COLOR_OSC_WAVE_CH4,
  GUI_COLOR_OSC_WAVE_CH5,
  GUI_COLOR_OSC_WAVE_CH6,
  GUI_COLOR_OSC_WAVE_CH7,
  GUI_COLOR_OSC_WAVE_CH8,
  GUI_COLOR_OSC_WAVE_CH9,
  GUI_COLOR_OSC_WAVE_CH10,
  GUI_COLOR_OSC_WAVE_CH11,
  GUI_COLOR_OSC_WAVE_CH12,
  GUI_COLOR_OSC_WAVE_CH13,
  GUI_COLOR_OSC_WAVE_CH14,
  GUI_COLOR_OSC_WAVE_CH15,
  GUI_COLOR_OSC_REF,
  GUI_COLOR_OSC_GUIDE,

  GUI_COLOR_VOLMETER_LOW,
  GUI_COLOR_VOLMETER_HIGH,
  GUI_COLOR_VOLMETER_PEAK,

  GUI_COLOR_ORDER_ROW_INDEX,
  GUI_COLOR_ORDER_ACTIVE,
  GUI_COLOR_ORDER_SELECTED,
  GUI_COLOR_ORDER_SIMILAR,
  GUI_COLOR_ORDER_INACTIVE,

  GUI_COLOR_FM_ALG_BG,
  GUI_COLOR_FM_ALG_LINE,
  GUI_COLOR_FM_MOD,
  GUI_COLOR_FM_PRIMARY_MOD,
  GUI_COLOR_FM_SECONDARY_MOD,
  GUI_COLOR_FM_BORDER_MOD,
  GUI_COLOR_FM_BORDER_SHADOW_MOD,
  GUI_COLOR_FM_CAR,
  GUI_COLOR_FM_PRIMARY_CAR,
  GUI_COLOR_FM_SECONDARY_CAR,
  GUI_COLOR_FM_BORDER_CAR,
  GUI_COLOR_FM_BORDER_SHADOW_CAR,

  GUI_COLOR_FM_ENVELOPE,
  GUI_COLOR_FM_ENVELOPE_SUS_GUIDE,
  GUI_COLOR_FM_ENVELOPE_RELEASE,
  GUI_COLOR_FM_SSG,
  GUI_COLOR_FM_WAVE,

  GUI_COLOR_MACRO_HIGHLIGHT,
  GUI_COLOR_MACRO_VOLUME,
  GUI_COLOR_MACRO_PITCH,
  GUI_COLOR_MACRO_OTHER,
  GUI_COLOR_MACRO_WAVE,
  GUI_COLOR_MACRO_NOISE,
  GUI_COLOR_MACRO_FILTER,
  GUI_COLOR_MACRO_ENVELOPE,
  GUI_COLOR_MACRO_GLOBAL,

  GUI_COLOR_INSTR_STD,
  GUI_COLOR_INSTR_FM,
  GUI_COLOR_INSTR_GB,
  GUI_COLOR_INSTR_C64,
  GUI_COLOR_INSTR_AMIGA,
  GUI_COLOR_INSTR_PCE,
  GUI_COLOR_INSTR_AY,
  GUI_COLOR_INSTR_AY8930,
  GUI_COLOR_INSTR_TIA,
  GUI_COLOR_INSTR_SAA1099,
  GUI_COLOR_INSTR_VIC,
  GUI_COLOR_INSTR_PET,
  GUI_COLOR_INSTR_VRC6,
  GUI_COLOR_INSTR_OPLL,
  GUI_COLOR_INSTR_OPL,
  GUI_COLOR_INSTR_FDS,
  GUI_COLOR_INSTR_VBOY,
  GUI_COLOR_INSTR_N163,
  GUI_COLOR_INSTR_SCC,
  GUI_COLOR_INSTR_OPZ,
  GUI_COLOR_INSTR_POKEY,
  GUI_COLOR_INSTR_BEEPER,
  GUI_COLOR_INSTR_SWAN,
  GUI_COLOR_INSTR_MIKEY,
  GUI_COLOR_INSTR_VERA,
  GUI_COLOR_INSTR_X1_010,
  GUI_COLOR_INSTR_VRC6_SAW,
  GUI_COLOR_INSTR_ES5506,
  GUI_COLOR_INSTR_MULTIPCM,
  GUI_COLOR_INSTR_SNES,
  GUI_COLOR_INSTR_SU,
  GUI_COLOR_INSTR_NAMCO,
  GUI_COLOR_INSTR_OPL_DRUMS,
  GUI_COLOR_INSTR_OPM,
  GUI_COLOR_INSTR_NES,
  GUI_COLOR_INSTR_MSM6258,
  GUI_COLOR_INSTR_MSM6295,
  GUI_COLOR_INSTR_ADPCMA,
  GUI_COLOR_INSTR_ADPCMB,
  GUI_COLOR_INSTR_SEGAPCM,
  GUI_COLOR_INSTR_QSOUND,
  GUI_COLOR_INSTR_YMZ280B,
  GUI_COLOR_INSTR_RF5C68,
  GUI_COLOR_INSTR_MSM5232,
  GUI_COLOR_INSTR_T6W28,
  GUI_COLOR_INSTR_K007232,
  GUI_COLOR_INSTR_GA20,
  GUI_COLOR_INSTR_POKEMINI,
  GUI_COLOR_INSTR_SM8521,
  GUI_COLOR_INSTR_PV1000,
  GUI_COLOR_INSTR_K053260,
  GUI_COLOR_INSTR_SCSP,
  GUI_COLOR_INSTR_TED,
  GUI_COLOR_INSTR_C140,
  GUI_COLOR_INSTR_C219,
  GUI_COLOR_INSTR_ESFM,
  GUI_COLOR_INSTR_POWERNOISE,
  GUI_COLOR_INSTR_POWERNOISE_SLOPE,
  GUI_COLOR_INSTR_DAVE,
  GUI_COLOR_INSTR_NDS,
  GUI_COLOR_INSTR_GBA_DMA,
  GUI_COLOR_INSTR_GBA_MINMOD,
  GUI_COLOR_INSTR_BIFURCATOR,
  GUI_COLOR_INSTR_SID2,
  GUI_COLOR_INSTR_SUPERVISION,
  GUI_COLOR_INSTR_UPD1771C,
  GUI_COLOR_INSTR_SID3,
  GUI_COLOR_INSTR_UNKNOWN,

  GUI_COLOR_CHANNEL_BG,
  GUI_COLOR_CHANNEL_FG,
  GUI_COLOR_CHANNEL_FM,
  GUI_COLOR_CHANNEL_PULSE,
  GUI_COLOR_CHANNEL_NOISE,
  GUI_COLOR_CHANNEL_WAVE,
  GUI_COLOR_CHANNEL_PCM,
  GUI_COLOR_CHANNEL_OP,
  GUI_COLOR_CHANNEL_MUTED,

  GUI_COLOR_PATTERN_PLAY_HEAD,
  GUI_COLOR_PATTERN_CURSOR,
  GUI_COLOR_PATTERN_CURSOR_HOVER,
  GUI_COLOR_PATTERN_CURSOR_ACTIVE,
  GUI_COLOR_PATTERN_SELECTION,
  GUI_COLOR_PATTERN_SELECTION_HOVER,
  GUI_COLOR_PATTERN_SELECTION_ACTIVE,
  GUI_COLOR_PATTERN_HI_1,
  GUI_COLOR_PATTERN_HI_2,
  GUI_COLOR_PATTERN_ROW_INDEX,
  GUI_COLOR_PATTERN_ROW_INDEX_HI1,
  GUI_COLOR_PATTERN_ROW_INDEX_HI2,
  GUI_COLOR_PATTERN_ACTIVE,
  GUI_COLOR_PATTERN_INACTIVE,
  GUI_COLOR_PATTERN_ACTIVE_HI1,
  GUI_COLOR_PATTERN_INACTIVE_HI1,
  GUI_COLOR_PATTERN_ACTIVE_HI2,
  GUI_COLOR_PATTERN_INACTIVE_HI2,
  GUI_COLOR_PATTERN_INS,
  GUI_COLOR_PATTERN_INS_WARN,
  GUI_COLOR_PATTERN_INS_ERROR,
  GUI_COLOR_PATTERN_VOLUME_MAX,
  GUI_COLOR_PATTERN_VOLUME_HALF,
  GUI_COLOR_PATTERN_VOLUME_MIN,
  GUI_COLOR_PATTERN_EFFECT_INVALID,
  GUI_COLOR_PATTERN_EFFECT_PITCH,
  GUI_COLOR_PATTERN_EFFECT_VOLUME,
  GUI_COLOR_PATTERN_EFFECT_PANNING,
  GUI_COLOR_PATTERN_EFFECT_SONG,
  GUI_COLOR_PATTERN_EFFECT_TIME,
  GUI_COLOR_PATTERN_EFFECT_SPEED,
  GUI_COLOR_PATTERN_EFFECT_SYS_PRIMARY,
  GUI_COLOR_PATTERN_EFFECT_SYS_SECONDARY,
  GUI_COLOR_PATTERN_EFFECT_MISC,

  GUI_COLOR_PATTERN_STATUS_OFF,
  GUI_COLOR_PATTERN_STATUS_REL,
  GUI_COLOR_PATTERN_STATUS_REL_ON,
  GUI_COLOR_PATTERN_STATUS_ON,
  GUI_COLOR_PATTERN_STATUS_VOLUME,
  GUI_COLOR_PATTERN_STATUS_PITCH,
  GUI_COLOR_PATTERN_STATUS_PANNING,
  GUI_COLOR_PATTERN_STATUS_SYS1,
  GUI_COLOR_PATTERN_STATUS_SYS2,
  GUI_COLOR_PATTERN_STATUS_MIXING,
  GUI_COLOR_PATTERN_STATUS_DSP,
  GUI_COLOR_PATTERN_STATUS_NOTE,
  GUI_COLOR_PATTERN_STATUS_MISC1,
  GUI_COLOR_PATTERN_STATUS_MISC2,
  GUI_COLOR_PATTERN_STATUS_MISC3,
  GUI_COLOR_PATTERN_STATUS_ATTACK,
  GUI_COLOR_PATTERN_STATUS_DECAY,
  GUI_COLOR_PATTERN_STATUS_SUSTAIN,
  GUI_COLOR_PATTERN_STATUS_RELEASE,
  GUI_COLOR_PATTERN_STATUS_DEC_LINEAR,
  GUI_COLOR_PATTERN_STATUS_DEC_EXP,
  GUI_COLOR_PATTERN_STATUS_INC,
  GUI_COLOR_PATTERN_STATUS_BENT,
  GUI_COLOR_PATTERN_STATUS_DIRECT,
  GUI_COLOR_PATTERN_PAIR,

  GUI_COLOR_SAMPLE_BG,
  GUI_COLOR_SAMPLE_FG,
  GUI_COLOR_SAMPLE_LOOP,
  GUI_COLOR_SAMPLE_CENTER,
  GUI_COLOR_SAMPLE_GRID,
  GUI_COLOR_SAMPLE_SEL,
  GUI_COLOR_SAMPLE_SEL_POINT,
  GUI_COLOR_SAMPLE_NEEDLE,
  GUI_COLOR_SAMPLE_NEEDLE_PLAYING,
  GUI_COLOR_SAMPLE_LOOP_POINT,
  GUI_COLOR_SAMPLE_TIME_BG,
  GUI_COLOR_SAMPLE_TIME_FG,
  GUI_COLOR_SAMPLE_CHIP_DISABLED,
  GUI_COLOR_SAMPLE_CHIP_ENABLED,
  GUI_COLOR_SAMPLE_CHIP_WARNING,

  GUI_COLOR_PAT_MANAGER_NULL,
  GUI_COLOR_PAT_MANAGER_USED,
  GUI_COLOR_PAT_MANAGER_OVERUSED,
  GUI_COLOR_PAT_MANAGER_EXTREMELY_OVERUSED,
  GUI_COLOR_PAT_MANAGER_COMBO_BREAKER,
  GUI_COLOR_PAT_MANAGER_UNUSED,

  GUI_COLOR_PIANO_BACKGROUND,
  GUI_COLOR_PIANO_KEY_BOTTOM,
  GUI_COLOR_PIANO_KEY_TOP,
  GUI_COLOR_PIANO_KEY_BOTTOM_HIT,
  GUI_COLOR_PIANO_KEY_TOP_HIT,
  GUI_COLOR_PIANO_KEY_BOTTOM_ACTIVE,
  GUI_COLOR_PIANO_KEY_TOP_ACTIVE,

  GUI_COLOR_CLOCK_TEXT,
  GUI_COLOR_CLOCK_BEAT_LOW,
  GUI_COLOR_CLOCK_BEAT_HIGH,

  GUI_COLOR_PATCHBAY_PORTSET,
  GUI_COLOR_PATCHBAY_PORT,
  GUI_COLOR_PATCHBAY_PORT_HIDDEN,
  GUI_COLOR_PATCHBAY_CONNECTION,
  GUI_COLOR_PATCHBAY_CONNECTION_BG,
  GUI_COLOR_PATCHBAY_CONNECTION_HI,

  GUI_COLOR_MEMORY_BG,
  GUI_COLOR_MEMORY_DATA,
  GUI_COLOR_MEMORY_FREE,
  GUI_COLOR_MEMORY_PADDING,
  GUI_COLOR_MEMORY_RESERVED,
  GUI_COLOR_MEMORY_SAMPLE,
  GUI_COLOR_MEMORY_SAMPLE_ALT1,
  GUI_COLOR_MEMORY_SAMPLE_ALT2,
  GUI_COLOR_MEMORY_SAMPLE_ALT3,
  GUI_COLOR_MEMORY_WAVE_RAM,
  GUI_COLOR_MEMORY_WAVE_STATIC,
  GUI_COLOR_MEMORY_ECHO,
  GUI_COLOR_MEMORY_N163_LOAD,
  GUI_COLOR_MEMORY_N163_PLAY,
  GUI_COLOR_MEMORY_BANK0,
  GUI_COLOR_MEMORY_BANK1,
  GUI_COLOR_MEMORY_BANK2,
  GUI_COLOR_MEMORY_BANK3,
  GUI_COLOR_MEMORY_BANK4,
  GUI_COLOR_MEMORY_BANK5,
  GUI_COLOR_MEMORY_BANK6,
  GUI_COLOR_MEMORY_BANK7,

  GUI_COLOR_LOGLEVEL_ERROR,
  GUI_COLOR_LOGLEVEL_WARNING,
  GUI_COLOR_LOGLEVEL_INFO,
  GUI_COLOR_LOGLEVEL_DEBUG,
  GUI_COLOR_LOGLEVEL_TRACE,

  GUI_COLOR_EE_VALUE,
  GUI_COLOR_PLAYBACK_STAT,
  GUI_COLOR_MAX
};

enum FurnaceGUIWindows {
  GUI_WINDOW_NOTHING=0,
  GUI_WINDOW_EDIT_CONTROLS,
  GUI_WINDOW_SONG_INFO,
  GUI_WINDOW_SPEED,
  GUI_WINDOW_ORDERS,
  GUI_WINDOW_INS_LIST,
  GUI_WINDOW_PATTERN,
  GUI_WINDOW_INS_EDIT,
  GUI_WINDOW_WAVE_LIST,
  GUI_WINDOW_WAVE_EDIT,
  GUI_WINDOW_SAMPLE_LIST,
  GUI_WINDOW_SAMPLE_EDIT,
  GUI_WINDOW_MIXER,
  GUI_WINDOW_ABOUT,
  GUI_WINDOW_SETTINGS,
  GUI_WINDOW_DEBUG,
  GUI_WINDOW_OSCILLOSCOPE,
  GUI_WINDOW_VOL_METER,
  GUI_WINDOW_STATS,
  GUI_WINDOW_COMPAT_FLAGS,
  GUI_WINDOW_PIANO,
  GUI_WINDOW_NOTES,
  GUI_WINDOW_CHANNELS,
  GUI_WINDOW_PAT_MANAGER,
  GUI_WINDOW_SYS_MANAGER,
  GUI_WINDOW_REGISTER_VIEW,
  GUI_WINDOW_LOG,
  GUI_WINDOW_EFFECT_LIST,
  GUI_WINDOW_CHAN_OSC,
  GUI_WINDOW_SUBSONGS,
  GUI_WINDOW_FIND,
  GUI_WINDOW_CLOCK,
  GUI_WINDOW_GROOVES,
  GUI_WINDOW_XY_OSC,
  GUI_WINDOW_INTRO_MON,
  GUI_WINDOW_MEMORY,
  GUI_WINDOW_CS_PLAYER,
  GUI_WINDOW_USER_PRESETS,
  GUI_WINDOW_SPOILER
};

enum FurnaceGUIMobileScenes {
  GUI_SCENE_PATTERN,
  GUI_SCENE_ORDERS,
  GUI_SCENE_INSTRUMENT,
  GUI_SCENE_WAVETABLE,
  GUI_SCENE_SAMPLE,
  GUI_SCENE_SONG,
  GUI_SCENE_CHANNELS,
  GUI_SCENE_CHIPS,
  GUI_SCENE_MIXER,
  GUI_SCENE_OTHER,
};

enum FurnaceGUISettingGroups: unsigned int {
  GUI_SETTINGS_GENERAL=1,
  GUI_SETTINGS_AUDIO=2,
  GUI_SETTINGS_MIDI=4,
  GUI_SETTINGS_KEYBOARD=8,
  GUI_SETTINGS_BEHAVIOR=16,
  GUI_SETTINGS_FONT=32,
  GUI_SETTINGS_APPEARANCE=64,
  GUI_SETTINGS_LAYOUTS=128,
  GUI_SETTINGS_COLOR=256,
  GUI_SETTINGS_EMULATION=512,

  GUI_SETTINGS_ALL=0xffffffff
};

enum FurnaceGUIFileDialogs {
  GUI_FILE_OPEN,
  GUI_FILE_OPEN_BACKUP,
  GUI_FILE_SAVE,
  GUI_FILE_SAVE_DMF,
  GUI_FILE_SAVE_DMF_LEGACY,
  GUI_FILE_INS_OPEN,
  GUI_FILE_INS_OPEN_REPLACE,
  GUI_FILE_INS_SAVE,
  GUI_FILE_INS_SAVE_DMP,
  GUI_FILE_INS_SAVE_ALL,
  GUI_FILE_WAVE_OPEN,
  GUI_FILE_WAVE_OPEN_REPLACE,
  GUI_FILE_WAVE_SAVE,
  GUI_FILE_WAVE_SAVE_DMW,
  GUI_FILE_WAVE_SAVE_RAW,
  GUI_FILE_WAVE_SAVE_ALL,
  GUI_FILE_SAMPLE_OPEN,
  GUI_FILE_SAMPLE_OPEN_RAW,
  GUI_FILE_SAMPLE_OPEN_REPLACE,
  GUI_FILE_SAMPLE_OPEN_REPLACE_RAW,
  GUI_FILE_SAMPLE_SAVE,
  GUI_FILE_SAMPLE_SAVE_RAW,
  GUI_FILE_SAMPLE_SAVE_ALL,
  GUI_FILE_EXPORT_AUDIO_ONE,
  GUI_FILE_EXPORT_AUDIO_PER_SYS,
  GUI_FILE_EXPORT_AUDIO_PER_CHANNEL,
  GUI_FILE_EXPORT_VGM,
  GUI_FILE_EXPORT_CMDSTREAM,
  GUI_FILE_EXPORT_TEXT,
  GUI_FILE_EXPORT_ROM,
  GUI_FILE_LOAD_MAIN_FONT,
  GUI_FILE_LOAD_HEAD_FONT,
  GUI_FILE_LOAD_PAT_FONT,
  GUI_FILE_IMPORT_COLORS,
  GUI_FILE_IMPORT_KEYBINDS,
  GUI_FILE_IMPORT_LAYOUT,
  GUI_FILE_IMPORT_USER_PRESETS,
  GUI_FILE_IMPORT_USER_PRESETS_REPLACE,
  GUI_FILE_IMPORT_CONFIG,
  GUI_FILE_EXPORT_COLORS,
  GUI_FILE_EXPORT_KEYBINDS,
  GUI_FILE_EXPORT_LAYOUT,
  GUI_FILE_EXPORT_USER_PRESETS,
  GUI_FILE_EXPORT_CONFIG,
  GUI_FILE_YRW801_ROM_OPEN,
  GUI_FILE_TG100_ROM_OPEN,
  GUI_FILE_MU5_ROM_OPEN,
  GUI_FILE_CMDSTREAM_OPEN,

  GUI_FILE_TEST_OPEN,
  GUI_FILE_TEST_OPEN_MULTI,
  GUI_FILE_TEST_SAVE
};

enum FurnaceGUIWarnings {
  GUI_WARN_QUIT,
  GUI_WARN_NEW,
  GUI_WARN_OPEN,
  GUI_WARN_OPEN_BACKUP,
  GUI_WARN_OPEN_DROP,
  GUI_WARN_RESET_LAYOUT,
  GUI_WARN_RESET_COLORS,
  GUI_WARN_RESET_KEYBINDS,
  GUI_WARN_CLOSE_SETTINGS,
  GUI_WARN_CLEAR,
  GUI_WARN_SUBSONG_DEL,
  GUI_WARN_SYSTEM_DEL,
  GUI_WARN_CLEAR_HISTORY,
  GUI_WARN_CV,
  GUI_WARN_RESET_CONFIG,
  GUI_WARN_GENERIC
};

enum FurnaceGUIExportTypes {
  GUI_EXPORT_NONE=-1,

  GUI_EXPORT_AUDIO=0,
  GUI_EXPORT_VGM,
  GUI_EXPORT_ROM,
  GUI_EXPORT_CMD_STREAM,
  GUI_EXPORT_TEXT,
  GUI_EXPORT_DMF
};

enum FurnaceGUIFMAlgs {
  FM_ALGS_4OP,
  FM_ALGS_2OP_OPL,
  FM_ALGS_4OP_OPL
};

enum FurnaceGUIActions {
  GUI_ACTION_GLOBAL_MIN=0,
  GUI_ACTION_NEW,
  GUI_ACTION_OPEN,
  GUI_ACTION_OPEN_BACKUP,
  GUI_ACTION_SAVE,
  GUI_ACTION_SAVE_AS,
  GUI_ACTION_EXPORT,
  GUI_ACTION_UNDO,
  GUI_ACTION_REDO,
  GUI_ACTION_QUIT,
  GUI_ACTION_PLAY_TOGGLE,
  GUI_ACTION_PLAY,
  GUI_ACTION_STOP,
  GUI_ACTION_PLAY_START,
  GUI_ACTION_PLAY_REPEAT,
  GUI_ACTION_PLAY_CURSOR,
  GUI_ACTION_STEP_ONE,
  GUI_ACTION_OCTAVE_UP,
  GUI_ACTION_OCTAVE_DOWN,
  GUI_ACTION_INS_UP,
  GUI_ACTION_INS_DOWN,
  GUI_ACTION_STEP_UP,
  GUI_ACTION_STEP_DOWN,
  GUI_ACTION_TOGGLE_EDIT,
  GUI_ACTION_METRONOME,
  GUI_ACTION_REPEAT_PATTERN,
  GUI_ACTION_FOLLOW_ORDERS,
  GUI_ACTION_FOLLOW_PATTERN,
  GUI_ACTION_FULLSCREEN,
  GUI_ACTION_TX81Z_REQUEST,
  GUI_ACTION_PANIC,
  GUI_ACTION_CLEAR,

  GUI_ACTION_COMMAND_PALETTE,
  GUI_ACTION_CMDPAL_MIN,
  GUI_ACTION_CMDPAL_RECENT,
  GUI_ACTION_CMDPAL_INSTRUMENTS,
  GUI_ACTION_CMDPAL_SAMPLES,
  GUI_ACTION_CMDPAL_INSTRUMENT_CHANGE,
  GUI_ACTION_CMDPAL_ADD_CHIP,
  GUI_ACTION_CMDPAL_MAX,

  GUI_ACTION_WINDOW_EDIT_CONTROLS,
  GUI_ACTION_WINDOW_ORDERS,
  GUI_ACTION_WINDOW_INS_LIST,
  GUI_ACTION_WINDOW_INS_EDIT,
  GUI_ACTION_WINDOW_SONG_INFO,
  GUI_ACTION_WINDOW_SPEED,
  GUI_ACTION_WINDOW_PATTERN,
  GUI_ACTION_WINDOW_WAVE_LIST,
  GUI_ACTION_WINDOW_WAVE_EDIT,
  GUI_ACTION_WINDOW_SAMPLE_LIST,
  GUI_ACTION_WINDOW_SAMPLE_EDIT,
  GUI_ACTION_WINDOW_ABOUT,
  GUI_ACTION_WINDOW_SETTINGS,
  GUI_ACTION_WINDOW_MIXER,
  GUI_ACTION_WINDOW_DEBUG,
  GUI_ACTION_WINDOW_OSCILLOSCOPE,
  GUI_ACTION_WINDOW_VOL_METER,
  GUI_ACTION_WINDOW_STATS,
  GUI_ACTION_WINDOW_COMPAT_FLAGS,
  GUI_ACTION_WINDOW_PIANO,
  GUI_ACTION_WINDOW_NOTES,
  GUI_ACTION_WINDOW_CHANNELS,
  GUI_ACTION_WINDOW_PAT_MANAGER,
  GUI_ACTION_WINDOW_SYS_MANAGER,
  GUI_ACTION_WINDOW_REGISTER_VIEW,
  GUI_ACTION_WINDOW_LOG,
  GUI_ACTION_WINDOW_EFFECT_LIST,
  GUI_ACTION_WINDOW_CHAN_OSC,
  GUI_ACTION_WINDOW_SUBSONGS,
  GUI_ACTION_WINDOW_FIND,
  GUI_ACTION_WINDOW_CLOCK,
  GUI_ACTION_WINDOW_GROOVES,
  GUI_ACTION_WINDOW_XY_OSC,
  GUI_ACTION_WINDOW_MEMORY,
  GUI_ACTION_WINDOW_CS_PLAYER,
  GUI_ACTION_WINDOW_USER_PRESETS,

  GUI_ACTION_COLLAPSE_WINDOW,
  GUI_ACTION_CLOSE_WINDOW,
  GUI_ACTION_GLOBAL_MAX,

  GUI_ACTION_PAT_MIN,
  GUI_ACTION_PAT_NOTE_UP,
  GUI_ACTION_PAT_NOTE_DOWN,
  GUI_ACTION_PAT_OCTAVE_UP,
  GUI_ACTION_PAT_OCTAVE_DOWN,
  GUI_ACTION_PAT_VALUE_UP,
  GUI_ACTION_PAT_VALUE_DOWN,
  GUI_ACTION_PAT_VALUE_UP_COARSE,
  GUI_ACTION_PAT_VALUE_DOWN_COARSE,
  GUI_ACTION_PAT_SELECT_ALL,
  GUI_ACTION_PAT_CUT,
  GUI_ACTION_PAT_COPY,
  GUI_ACTION_PAT_PASTE,
  GUI_ACTION_PAT_PASTE_MIX,
  GUI_ACTION_PAT_PASTE_MIX_BG,
  GUI_ACTION_PAT_PASTE_FLOOD,
  GUI_ACTION_PAT_PASTE_OVERFLOW,
  GUI_ACTION_PAT_CURSOR_UP,
  GUI_ACTION_PAT_CURSOR_DOWN,
  GUI_ACTION_PAT_CURSOR_LEFT,
  GUI_ACTION_PAT_CURSOR_RIGHT,
  GUI_ACTION_PAT_CURSOR_UP_ONE,
  GUI_ACTION_PAT_CURSOR_DOWN_ONE,
  GUI_ACTION_PAT_CURSOR_LEFT_CHANNEL,
  GUI_ACTION_PAT_CURSOR_RIGHT_CHANNEL,
  GUI_ACTION_PAT_CURSOR_NEXT_CHANNEL,
  GUI_ACTION_PAT_CURSOR_PREVIOUS_CHANNEL,
  GUI_ACTION_PAT_CURSOR_BEGIN,
  GUI_ACTION_PAT_CURSOR_END,
  GUI_ACTION_PAT_CURSOR_UP_COARSE,
  GUI_ACTION_PAT_CURSOR_DOWN_COARSE,
  GUI_ACTION_PAT_SELECTION_UP,
  GUI_ACTION_PAT_SELECTION_DOWN,
  GUI_ACTION_PAT_SELECTION_LEFT,
  GUI_ACTION_PAT_SELECTION_RIGHT,
  GUI_ACTION_PAT_SELECTION_UP_ONE,
  GUI_ACTION_PAT_SELECTION_DOWN_ONE,
  GUI_ACTION_PAT_SELECTION_BEGIN,
  GUI_ACTION_PAT_SELECTION_END,
  GUI_ACTION_PAT_SELECTION_UP_COARSE,
  GUI_ACTION_PAT_SELECTION_DOWN_COARSE,
  GUI_ACTION_PAT_MOVE_UP,
  GUI_ACTION_PAT_MOVE_DOWN,
  GUI_ACTION_PAT_MOVE_LEFT_CHANNEL,
  GUI_ACTION_PAT_MOVE_RIGHT_CHANNEL,
  GUI_ACTION_PAT_DELETE,
  GUI_ACTION_PAT_PULL_DELETE,
  GUI_ACTION_PAT_INSERT,
  GUI_ACTION_PAT_MUTE_CURSOR,
  GUI_ACTION_PAT_SOLO_CURSOR,
  GUI_ACTION_PAT_UNMUTE_ALL,
  GUI_ACTION_PAT_NEXT_ORDER,
  GUI_ACTION_PAT_PREV_ORDER,
  GUI_ACTION_PAT_COLLAPSE,
  GUI_ACTION_PAT_INCREASE_COLUMNS,
  GUI_ACTION_PAT_DECREASE_COLUMNS,
  GUI_ACTION_PAT_INTERPOLATE,
  GUI_ACTION_PAT_FADE,
  GUI_ACTION_PAT_INVERT_VALUES,
  GUI_ACTION_PAT_FLIP_SELECTION,
  GUI_ACTION_PAT_COLLAPSE_ROWS,
  GUI_ACTION_PAT_EXPAND_ROWS,
  GUI_ACTION_PAT_COLLAPSE_PAT,
  GUI_ACTION_PAT_EXPAND_PAT,
  GUI_ACTION_PAT_COLLAPSE_SONG,
  GUI_ACTION_PAT_EXPAND_SONG,
  GUI_ACTION_PAT_LATCH,
  GUI_ACTION_PAT_SCROLL_MODE,
  GUI_ACTION_PAT_CLEAR_LATCH,
  GUI_ACTION_PAT_ABSORB_INSTRUMENT,
  GUI_ACTION_PAT_CURSOR_UNDO,
  GUI_ACTION_PAT_CURSOR_REDO,
  GUI_ACTION_PAT_MAX,

  GUI_ACTION_INS_LIST_MIN,
  GUI_ACTION_INS_LIST_ADD,
  GUI_ACTION_INS_LIST_DUPLICATE,
  GUI_ACTION_INS_LIST_OPEN,
  GUI_ACTION_INS_LIST_OPEN_REPLACE,
  GUI_ACTION_INS_LIST_SAVE,
  GUI_ACTION_INS_LIST_SAVE_DMP,
  GUI_ACTION_INS_LIST_MOVE_UP,
  GUI_ACTION_INS_LIST_MOVE_DOWN,
  GUI_ACTION_INS_LIST_DELETE,
  GUI_ACTION_INS_LIST_EDIT,
  GUI_ACTION_INS_LIST_UP,
  GUI_ACTION_INS_LIST_DOWN,
  GUI_ACTION_INS_LIST_DIR_VIEW,
  GUI_ACTION_INS_LIST_SAVE_ALL,
  GUI_ACTION_INS_LIST_MAX,

  GUI_ACTION_WAVE_LIST_MIN,
  GUI_ACTION_WAVE_LIST_ADD,
  GUI_ACTION_WAVE_LIST_DUPLICATE,
  GUI_ACTION_WAVE_LIST_OPEN,
  GUI_ACTION_WAVE_LIST_OPEN_REPLACE,
  GUI_ACTION_WAVE_LIST_SAVE,
  GUI_ACTION_WAVE_LIST_SAVE_DMW,
  GUI_ACTION_WAVE_LIST_SAVE_RAW,
  GUI_ACTION_WAVE_LIST_MOVE_UP,
  GUI_ACTION_WAVE_LIST_MOVE_DOWN,
  GUI_ACTION_WAVE_LIST_DELETE,
  GUI_ACTION_WAVE_LIST_EDIT,
  GUI_ACTION_WAVE_LIST_UP,
  GUI_ACTION_WAVE_LIST_DOWN,
  GUI_ACTION_WAVE_LIST_DIR_VIEW,
  GUI_ACTION_WAVE_LIST_SAVE_ALL,
  GUI_ACTION_WAVE_LIST_MAX,

  GUI_ACTION_SAMPLE_LIST_MIN,
  GUI_ACTION_SAMPLE_LIST_ADD,
  GUI_ACTION_SAMPLE_LIST_DUPLICATE,
  GUI_ACTION_SAMPLE_LIST_OPEN,
  GUI_ACTION_SAMPLE_LIST_OPEN_REPLACE,
  GUI_ACTION_SAMPLE_LIST_OPEN_RAW,
  GUI_ACTION_SAMPLE_LIST_OPEN_REPLACE_RAW,
  GUI_ACTION_SAMPLE_LIST_SAVE,
  GUI_ACTION_SAMPLE_LIST_SAVE_RAW,
  GUI_ACTION_SAMPLE_LIST_MOVE_UP,
  GUI_ACTION_SAMPLE_LIST_MOVE_DOWN,
  GUI_ACTION_SAMPLE_LIST_DELETE,
  GUI_ACTION_SAMPLE_LIST_EDIT,
  GUI_ACTION_SAMPLE_LIST_UP,
  GUI_ACTION_SAMPLE_LIST_DOWN,
  GUI_ACTION_SAMPLE_LIST_PREVIEW,
  GUI_ACTION_SAMPLE_LIST_STOP_PREVIEW,
  GUI_ACTION_SAMPLE_LIST_DIR_VIEW,
  GUI_ACTION_SAMPLE_LIST_MAKE_MAP,
  GUI_ACTION_SAMPLE_LIST_SAVE_ALL,
  GUI_ACTION_SAMPLE_LIST_MAX,

  GUI_ACTION_SAMPLE_MIN,
  GUI_ACTION_SAMPLE_SELECT,
  GUI_ACTION_SAMPLE_DRAW,
  GUI_ACTION_SAMPLE_CUT,
  GUI_ACTION_SAMPLE_COPY,
  GUI_ACTION_SAMPLE_PASTE,
  GUI_ACTION_SAMPLE_PASTE_REPLACE,
  GUI_ACTION_SAMPLE_PASTE_MIX,
  GUI_ACTION_SAMPLE_SELECT_ALL,
  GUI_ACTION_SAMPLE_RESIZE,
  GUI_ACTION_SAMPLE_RESAMPLE,
  GUI_ACTION_SAMPLE_AMPLIFY,
  GUI_ACTION_SAMPLE_NORMALIZE,
  GUI_ACTION_SAMPLE_FADE_IN,
  GUI_ACTION_SAMPLE_FADE_OUT,
  GUI_ACTION_SAMPLE_SILENCE,
  GUI_ACTION_SAMPLE_INSERT,
  GUI_ACTION_SAMPLE_DELETE,
  GUI_ACTION_SAMPLE_TRIM,
  GUI_ACTION_SAMPLE_REVERSE,
  GUI_ACTION_SAMPLE_INVERT,
  GUI_ACTION_SAMPLE_SIGN,
  GUI_ACTION_SAMPLE_FILTER,
  GUI_ACTION_SAMPLE_CROSSFADE_LOOP,
  GUI_ACTION_SAMPLE_PREVIEW,
  GUI_ACTION_SAMPLE_STOP_PREVIEW,
  GUI_ACTION_SAMPLE_ZOOM_IN,
  GUI_ACTION_SAMPLE_ZOOM_OUT,
  GUI_ACTION_SAMPLE_ZOOM_AUTO,
  GUI_ACTION_SAMPLE_MAKE_INS,
  GUI_ACTION_SAMPLE_SET_LOOP,
  GUI_ACTION_SAMPLE_CREATE_WAVE,
  GUI_ACTION_SAMPLE_MAX,

  GUI_ACTION_ORDERS_MIN,
  GUI_ACTION_ORDERS_UP,
  GUI_ACTION_ORDERS_DOWN,
  GUI_ACTION_ORDERS_LEFT,
  GUI_ACTION_ORDERS_RIGHT,
  GUI_ACTION_ORDERS_INCREASE,
  GUI_ACTION_ORDERS_DECREASE,
  GUI_ACTION_ORDERS_EDIT_MODE,
  GUI_ACTION_ORDERS_LINK,
  GUI_ACTION_ORDERS_ADD,
  GUI_ACTION_ORDERS_DUPLICATE,
  GUI_ACTION_ORDERS_DEEP_CLONE,
  GUI_ACTION_ORDERS_DUPLICATE_END,
  GUI_ACTION_ORDERS_DEEP_CLONE_END,
  GUI_ACTION_ORDERS_REMOVE,
  GUI_ACTION_ORDERS_MOVE_UP,
  GUI_ACTION_ORDERS_MOVE_DOWN,
  GUI_ACTION_ORDERS_REPLAY,
  GUI_ACTION_ORDERS_MAX,

  GUI_ACTION_MAX
};

enum FurnaceGUIImages {
  GUI_IMAGE_ICON=0,
  GUI_IMAGE_TALOGO,
  GUI_IMAGE_TACHIP,
  GUI_IMAGE_LOGO,
  GUI_IMAGE_WORDMARK,
  GUI_IMAGE_INTROBG,
  GUI_IMAGE_PAT,

  GUI_IMAGE_MAX
};

enum FurnaceGUIChanOscRef {
  GUI_OSCREF_NONE=0,
  GUI_OSCREF_CENTER,
  GUI_OSCREF_FULL,

  GUI_OSCREF_FREQUENCY,
  GUI_OSCREF_VOLUME,
  GUI_OSCREF_CHANNEL,
  GUI_OSCREF_BRIGHT,

  GUI_OSCREF_NOTE_TRIGGER,

  GUI_OSCREF_MAX
};

enum PasteMode {
  GUI_PASTE_MODE_NORMAL=0,
  GUI_PASTE_MODE_MIX_FG,
  GUI_PASTE_MODE_MIX_BG,
  GUI_PASTE_MODE_FLOOD,
  GUI_PASTE_MODE_OVERFLOW,
  GUI_PASTE_MODE_INS_FG,
  GUI_PASTE_MODE_INS_BG
};

enum NoteCtrl {
  GUI_NOTE_OFF=100,
  GUI_NOTE_OFF_RELEASE=101,
  GUI_NOTE_RELEASE=102
};

#define FURKMOD_CTRL (1U<<31)
#define FURKMOD_SHIFT (1<<29)
#define FURKMOD_META (1<<28)
#define FURKMOD_ALT (1<<27)
#define FURK_MASK 0x40ffffff

#ifdef __APPLE__
#define FURKMOD_CMD FURKMOD_META
#else
#define FURKMOD_CMD FURKMOD_CTRL
#endif

struct SelectionPoint {
  int xCoarse, xFine;
  int y;
  SelectionPoint(int xc, int xf, int yp):
    xCoarse(xc), xFine(xf), y(yp) {}
  SelectionPoint():
    xCoarse(0), xFine(0), y(0) {}
};

struct UndoRegion {
  struct UndoRegionPoint {
    int ord, x, y;
    UndoRegionPoint(int o, int xp, int yp):
      ord(o), x(xp), y(yp) {}
    UndoRegionPoint():
      ord(-1), x(-1), y(-1) {}
  } begin, end;
  UndoRegion(int o0, int x0, int y0, int o1, int x1, int y1):
    begin(o0,x0,y0),
    end(o1,x1,y1) {}
  UndoRegion():
    begin(),
    end() {}
};

enum ActionType {
  GUI_UNDO_CHANGE_ORDER,
  GUI_UNDO_PATTERN_EDIT,
  GUI_UNDO_PATTERN_DELETE,
  GUI_UNDO_PATTERN_PULL,
  GUI_UNDO_PATTERN_PUSH,
  GUI_UNDO_PATTERN_CUT,
  GUI_UNDO_PATTERN_PASTE,
  GUI_UNDO_PATTERN_CHANGE_INS,
  GUI_UNDO_PATTERN_INTERPOLATE,
  GUI_UNDO_PATTERN_FADE,
  GUI_UNDO_PATTERN_SCALE,
  GUI_UNDO_PATTERN_RANDOMIZE,
  GUI_UNDO_PATTERN_INVERT_VAL,
  GUI_UNDO_PATTERN_FLIP,
  GUI_UNDO_PATTERN_COLLAPSE,
  GUI_UNDO_PATTERN_EXPAND,
  GUI_UNDO_PATTERN_COLLAPSE_SONG,
  GUI_UNDO_PATTERN_EXPAND_SONG,
  GUI_UNDO_PATTERN_DRAG,
  GUI_UNDO_REPLACE
};

enum UndoOtherTarget {
  GUI_UNDO_TARGET_SONG,
  GUI_UNDO_TARGET_SUBSONG
};

struct UndoPatternData {
  int subSong, chan, pat, row, col;
  short oldVal, newVal;
  UndoPatternData(int s, int c, int p, int r, int co, short v1, short v2):
    subSong(s),
    chan(c),
    pat(p),
    row(r),
    col(co),
    oldVal(v1),
    newVal(v2) {}
};

struct UndoOrderData {
  int subSong, chan, ord;
  unsigned char oldVal, newVal;
  UndoOrderData(int s, int c, int o, unsigned char v1, unsigned char v2):
    subSong(s),
    chan(c),
    ord(o),
    oldVal(v1),
    newVal(v2) {}
};

struct UndoOtherData {
  UndoOtherTarget target;
  int subtarget;
  size_t off;
  unsigned char oldVal, newVal;
  UndoOtherData(UndoOtherTarget t, int st, size_t o, unsigned char v1, unsigned char v2):
    target(t),
    subtarget(st),
    off(o),
    oldVal(v1),
    newVal(v2) {}
};

struct UndoStep {
  ActionType type;
  SelectionPoint cursor, selStart, selEnd;
  int order;
  bool nibble;
  int oldOrdersLen, newOrdersLen;
  int oldPatLen, newPatLen;
  std::vector<UndoOrderData> ord;
  std::vector<UndoPatternData> pat;
  std::vector<UndoOtherData> other;

  UndoStep():
    type(GUI_UNDO_CHANGE_ORDER),
    cursor(),
    selStart(),
    selEnd(),
    order(0),
    nibble(false),
    oldOrdersLen(0),
    newOrdersLen(0),
    oldPatLen(0),
    newPatLen(0) {}
};

struct CursorJumpPoint {
  SelectionPoint point;
  int order;
  int subSong;
  CursorJumpPoint(const SelectionPoint& p, int o, int ss):
    point(p), order(o), subSong(ss) {}
  CursorJumpPoint():
    point(), order(0), subSong(0) {}
  bool operator== (const CursorJumpPoint& spot) {
    return point.xCoarse==spot.point.xCoarse && point.xFine==spot.point.xFine && point.y==spot.point.y && order==spot.order && subSong==spot.subSong;
  }
  bool operator!= (const CursorJumpPoint& spot) {
    return !(*this == spot);
  }
};

// -1 = any
struct MIDIBind {
  int type, channel, data1, data2;
  int action;
  MIDIBind():
    type(0),
    channel(16),
    data1(128),
    data2(128),
    action(0) {}
};

struct MIDIMap {
  // access method: map[type][channel][data1][data2];
  // channel 16 = any
  // data1 128 = any
  // data2 128 = any
  int**** map;
  std::vector<MIDIBind> binds;

  bool noteInput, volInput, rawVolume, polyInput, directChannel, programChange, midiClock, midiTimeCode, yamahaFMResponse, directProgram;
  // 0: disabled
  //
  // 1: C- C# D- D# E- F- F# G- G# A- A# B-
  // o1    1     3        6     8     A
  //    0     2     4  5     7     9     B
  //    C- C# D- D# E- F- F# G- G# A- A# B-
  // o2    D     F
  //    C     E
  //
  // 2: C- C# D- D# E- F- F# G- G# A- A# B-
  // o1    1     3        6     8     A
  //    0     2     4  5     7     9     B
  //    C- C# D- D# E- F- F# G- G# A- A# B-
  // o2    D     F        2     4     6
  //    C     E     0  1     3     5     7
  //
  // 3: C- C# D- D# E- F- F# G- G# A- A# B-
  // o1    A     B        C     D     E
  //    0     1     2  3     4     5     6
  //    C- C# D- D# E- F- F# G- G# A- A# B-
  // o2    F
  //    7     8     9
  //
  // 4: use dual CC for value input (nibble)
  // 5: use 14-bit CC for value input (MSB/LSB)
  // 6: use single CC for value input (may be imprecise)
  int valueInputStyle;
  int valueInputControlMSB; // on 4
  int valueInputControlLSB; // on 4
  int valueInputControlSingle;

  // 0: disabled
  // 1: use dual CC (nibble)
  // 2: use 14-bit CC (MSB/LSB)
  // 3: use single CC (may be imprecise)
  int valueInputSpecificStyle[18];
  int valueInputSpecificMSB[18];
  int valueInputSpecificLSB[18];
  int valueInputSpecificSingle[18];
  float volExp;

  int valueInputCurMSB, valueInputCurLSB, valueInputCurSingle;
  int valueInputCurMSBS[18];
  int valueInputCurLSBS[18];
  int valueInputCurSingleS[18];

  void compile();
  void deinit();
  int at(const TAMidiMessage& where);
  bool read(String path);
  bool write(String path);
  MIDIMap():
    map(NULL),
    noteInput(true),
    volInput(false),
    rawVolume(false),
    polyInput(false),
    directChannel(false),
    programChange(true),
    midiClock(false),
    midiTimeCode(false),
    yamahaFMResponse(false),
    directProgram(false),
    valueInputStyle(1),
    valueInputControlMSB(0),
    valueInputControlLSB(0),
    valueInputControlSingle(0),
    volExp(2.0f),
    valueInputCurMSB(0),
    valueInputCurLSB(0),
    valueInputCurSingle(0) {
    memset(valueInputSpecificStyle,0,18*sizeof(int));
    memset(valueInputSpecificMSB,0,18*sizeof(int));
    memset(valueInputSpecificLSB,0,18*sizeof(int));
    memset(valueInputSpecificSingle,0,18*sizeof(int));

    memset(valueInputCurMSBS,0,18*sizeof(int));
    memset(valueInputCurLSBS,0,18*sizeof(int));
    memset(valueInputCurSingleS,0,18*sizeof(int));
  }
};

struct Particle {
  ImU32* colors;
  const char* type;
  ImVec2 pos, speed;
  float gravity, friction, life, lifeSpeed;
  bool update(float frameTime);
  Particle(ImU32* color, const char* ty, float x, float y, float sX, float sY, float g, float fr, float l, float lS):
    colors(color),
    type(ty),
    pos(x,y),
    speed(sX,sY),
    gravity(g),
    friction(fr),
    life(l),
    lifeSpeed(lS) {}
};

struct OperationMask {
  bool note, ins, vol, effect, effectVal;
  OperationMask():
    note(true),
    ins(true),
    vol(true),
    effect(true),
    effectVal(true) {}
};

struct TouchPoint {
  // an ID of -1 represents the mouse cursor.
  int id;
  float x, y, z;
  TouchPoint():
    id(-1),
    x(0.0f),
    y(0.0f),
    z(1.0f) {}
  TouchPoint(float xp, float yp):
    id(-1),
    x(xp),
    y(yp),
    z(1.0f) {}
  TouchPoint(int ident, float xp, float yp, float pressure=1.0f):
    id(ident),
    x(xp),
    y(yp),
    z(pressure) {}
};

struct Gradient2DPoint {
  ImVec4 color;
  float x, y, prevX, prevY;
  float spread, distance;
  bool selected, grab;
  Gradient2DPoint(float xPos, float yPos):
    color(1,1,1,1),
    x(xPos),
    y(yPos),
    prevX(0.0f),
    prevY(0.0f),
    spread(0.0f),
    distance(0.5f),
    selected(false),
    grab(false) {}
  Gradient2DPoint():
    color(1,1,1,1),
    x(0.0f),
    y(0.0f),
    spread(0.0f),
    distance(0.5f),
    selected(false),
    grab(false) {}
};

struct Gradient2D {
  ImVec4 bgColor;
  std::vector<Gradient2DPoint> points;
  std::unique_ptr<ImU32[]> grad;
  size_t width, height;

  String toString();
  bool fromString(String val);
  void render();
  ImU32 get(float x, float y);
  Gradient2D(size_t w, size_t h):
    bgColor(0.0f,0.0f,0.0f,0.0f),
    width(w),
    height(h) {
    grad=std::make_unique<ImU32[]>(width*height);
  }
};

struct FurnaceGUISysDefChip {
  DivSystem sys;
  float vol, pan, panFR;
  String flags;
  FurnaceGUISysDefChip(DivSystem s, float v, float p, const char* f, float pf=0.0):
    sys(s),
    vol(v),
    pan(p),
    panFR(pf),
    flags(f) {}
};

struct FurnaceGUISysDef {
  String name;
  String extra;
  String definition;
  std::vector<FurnaceGUISysDefChip> orig;
  std::vector<FurnaceGUISysDef> subDefs;
  void bake();
  FurnaceGUISysDef(const char* n, std::initializer_list<FurnaceGUISysDefChip> def, const char* e=NULL);
  FurnaceGUISysDef(const char* n, const char* def, DivEngine* e);
};

struct FurnaceGUISysCategory {
  const char* name;
  const char* description;
  std::vector<FurnaceGUISysDef> systems;
  FurnaceGUISysCategory(const char* n, const char* d):
    name(n),
    description(d) {}
  FurnaceGUISysCategory():
    name(NULL),
    description(NULL) {}
};

struct FurnaceGUIMacroDesc {
  DivInstrumentMacro* macro;
  int min, max;
  float height;
  const char* displayName;
  const char** bitfieldBits;
  const char* modeName;
  ImVec4 color;
  bool isBitfield, blockMode, bit30;
  String (*hoverFunc)(int,float,void*);
  void* hoverFuncUser;
  bool isArp;
  bool isPitch;

  FurnaceGUIMacroDesc(const char* name, DivInstrumentMacro* m, int macroMin, int macroMax, float macroHeight, ImVec4 col=ImVec4(1.0f,1.0f,1.0f,1.0f), bool block=false, const char* mName=NULL, String (*hf)(int,float,void*)=NULL, bool bitfield=false, const char** bfVal=NULL, bool bit30Special=false, void* hfu=NULL, bool isArp=false, bool isPitch=false):
    macro(m),
    height(macroHeight),
    displayName(name),
    bitfieldBits(bfVal),
    modeName(mName),
    color(col),
    isBitfield(bitfield),
    blockMode(block),
    bit30(bit30Special),
    hoverFunc(hf),
    hoverFuncUser(hfu),
    isArp(isArp),
    isPitch(isPitch) {
    // MSVC -> hell
    this->min=macroMin;
    this->max=macroMax;
  }
};

struct FurnaceGUIMacroEditState {
  int selectedMacro;
  FurnaceGUIMacroEditState():
    selectedMacro(0) {}
};

enum FurnaceGUIFindQueryModes {
  GUI_QUERY_IGNORE=0,
  GUI_QUERY_MATCH,
  GUI_QUERY_MATCH_NOT,
  GUI_QUERY_RANGE,
  GUI_QUERY_RANGE_NOT,
  GUI_QUERY_ANY,
  GUI_QUERY_NONE,

  GUI_QUERY_MAX
};

enum FurnaceGUIFindQueryReplaceModes {
  GUI_QUERY_REPLACE_SET=0,
  GUI_QUERY_REPLACE_ADD,
  GUI_QUERY_REPLACE_ADD_OVERFLOW,
  GUI_QUERY_REPLACE_SCALE,
  GUI_QUERY_REPLACE_CLEAR,

  GUI_QUERY_REPLACE_MAX
};

struct FurnaceGUIFindQuery {
  int noteMode, insMode, volMode, effectCount;
  int effectMode[8];
  int effectValMode[8];
  int note, noteMax;
  unsigned char ins, insMax;
  unsigned char vol, volMax;
  unsigned char effect[8];
  unsigned char effectMax[8];
  unsigned char effectVal[8];
  unsigned char effectValMax[8];

  FurnaceGUIFindQuery():
    noteMode(GUI_QUERY_IGNORE),
    insMode(GUI_QUERY_IGNORE),
    volMode(GUI_QUERY_IGNORE),
    effectCount(0),
    note(0),
    noteMax(0),
    ins(0),
    insMax(0),
    vol(0),
    volMax(0) {
    memset(effectMode,0,8*sizeof(int));
    memset(effectValMode,0,8*sizeof(int));
    memset(effect,0,8);
    memset(effectMax,0,8);
    memset(effectVal,0,8);
    memset(effectValMax,0,8);
  }
};

struct FurnaceGUIQueryResult {
  int subsong, order, x, y;
  signed char effectPos[8];
  FurnaceGUIQueryResult():
    subsong(0),
    order(0),
    x(0),
    y(0),
    effectPos{-1,-1,-1,-1,-1,-1,-1,-1} {}
  FurnaceGUIQueryResult(int ss, int o, int xPos, int yPos, const signed char* fxPos):
    subsong(ss),
    order(o),
    x(xPos),
    y(yPos) {
    memcpy(effectPos,fxPos,8);
  }
};

struct FurnaceGUIWaveSizeEntry {
  short width, height;
  const char* sys;

  FurnaceGUIWaveSizeEntry(short w, short h, const char* s):
    width(w),
    height(h),
    sys(s) {}
  FurnaceGUIWaveSizeEntry():
    width(-1),
    height(-1),
    sys(NULL) {}
};

enum FurnaceGUITextureFormat: unsigned int {
  GUI_TEXFORMAT_UNKNOWN=0,
  GUI_TEXFORMAT_ABGR32=1,
  GUI_TEXFORMAT_ARGB32=2,
  GUI_TEXFORMAT_BGRA32=4,
  GUI_TEXFORMAT_RGBA32=8,
};

class FurnaceGUITexture {
};

struct FurnaceGUIImage {
  unsigned char* data;
  FurnaceGUITexture* tex;
  int width, height, ch;

  FurnaceGUIImage():
   data(NULL),
   tex(NULL),
   width(0),
   height(0),
   ch(0) {}
};

struct FurnaceGUIPerfMetric {
  const char* name;
  int elapsed;
  FurnaceGUIPerfMetric(const char* n, int t):
    name(n),
    elapsed(t) {}
  FurnaceGUIPerfMetric():
    name(NULL),
    elapsed(0) {}
};

struct FurnaceGUIBackupEntry {
  String name;
  uint64_t size;
  struct tm lastEntryTime;
  FurnaceGUIBackupEntry():
    size(0) {
    memset(&lastEntryTime,0,sizeof(struct tm));
  }
};

enum FurnaceGUIBlendMode {
  GUI_BLEND_MODE_NONE=0,
  GUI_BLEND_MODE_BLEND,
  GUI_BLEND_MODE_ADD,
  GUI_BLEND_MODE_MULTIPLY
};

class FurnaceGUIRender {
  public:
    virtual ImTextureID getTextureID(FurnaceGUITexture* which);
    virtual float getTextureU(FurnaceGUITexture* which);
    virtual float getTextureV(FurnaceGUITexture* which);
    virtual FurnaceGUITextureFormat getTextureFormat(FurnaceGUITexture* which);
    virtual bool isTextureValid(FurnaceGUITexture* which);
    virtual bool lockTexture(FurnaceGUITexture* which, void** data, int* pitch);
    virtual bool unlockTexture(FurnaceGUITexture* which);
    virtual bool updateTexture(FurnaceGUITexture* which, void* data, int pitch);
    virtual FurnaceGUITexture* createTexture(bool dynamic, int width, int height, bool interpolate=true, FurnaceGUITextureFormat format=GUI_TEXFORMAT_ABGR32);
    virtual bool destroyTexture(FurnaceGUITexture* which);
    virtual void setTextureBlendMode(FurnaceGUITexture* which, FurnaceGUIBlendMode mode);
    virtual void setBlendMode(FurnaceGUIBlendMode mode);
    virtual void resized(const SDL_Event& ev);
    virtual void clear(ImVec4 color);
    virtual bool newFrame();
    virtual bool canVSync();
    virtual void createFontsTexture();
    virtual void destroyFontsTexture();
    virtual void renderGUI();
    virtual void wipe(float alpha);
    virtual void drawOsc(float* data, size_t len, ImVec2 pos0, ImVec2 pos1, ImVec4 color, ImVec2 canvasSize, float lineWidth);
    virtual void present();
    virtual bool supportsDrawOsc();
    virtual bool areTexturesSquare();
    virtual bool getOutputSize(int& w, int& h);
    virtual int getWindowFlags();
    virtual int getMaxTextureWidth();
    virtual int getMaxTextureHeight();
    virtual unsigned int getTextureFormats();
    virtual const char* getBackendName();
    virtual const char* getVendorName();
    virtual const char* getDeviceName();
    virtual const char* getAPIVersion();
    virtual void setSwapInterval(int swapInterval);
    virtual void preInit(const DivConfig& conf);
    virtual bool init(SDL_Window* win, int swapInterval);
    virtual void initGUI(SDL_Window* win);
    virtual void quitGUI();
    virtual bool quit();
    virtual bool isDead();
    virtual ~FurnaceGUIRender();
};

struct PendingDrawOsc {
  void* gui;
  float* data;
  size_t len;
  ImVec2 pos0;
  ImVec2 pos1;
  ImVec4 color;
  float lineSize;
  PendingDrawOsc():
    gui(NULL),
    data(NULL),
    len(0),
    pos0(0,0),
    pos1(0,0),
    color(0,0,0,0),
    lineSize(0.0f) {}
};

struct FurnaceCV;

class FurnaceGUI {
  DivEngine* e;

  FurnaceGUIRenderBackend renderBackend;
  FurnaceGUIRender* rend;
  FurnaceGUITextureFormat bestTexFormat;

  SDL_Window* sdlWin;
  SDL_Haptic* vibrator;
  bool vibratorAvailable;

  FurnaceCV* cv;
  FurnaceGUITexture* cvTex;

  FurnaceGUITexture* sampleTex;
  int sampleTexW, sampleTexH;
  bool updateSampleTex;

  String workingDir, fileName, clipboard, warnString, errorString, lastError, curFileName, nextFile, sysSearchQuery, newSongQuery, paletteQuery, sampleBankSearchQuery;
  String workingDirSong, workingDirIns, workingDirWave, workingDirSample, workingDirAudioExport;
  String workingDirVGMExport, workingDirROMExport;
  String workingDirFont, workingDirColors, workingDirKeybinds;
  String workingDirLayout, workingDirROM, workingDirTest;
  String workingDirConfig;
  String mmlString[32];
  String mmlStringW, grooveString, grooveListString, mmlStringModTable;
  String mmlStringSNES[DIV_MAX_CHIPS];
  String folderString;

  struct PaletteSearchResult { int id; std::vector<int> highlightChars; };
  std::vector<DivSystem> sysSearchResults;
  std::vector<std::pair<DivSample*,bool>> sampleBankSearchResults;
  std::vector<FurnaceGUISysDef> newSongSearchResults;
  std::vector<PaletteSearchResult> paletteSearchResults;
  FixedQueue<String,32> recentFile;
  std::vector<DivInstrumentType> makeInsTypeList;
  std::vector<FurnaceGUIWaveSizeEntry> waveSizeList;
  std::vector<String> availRenderDrivers;
  std::vector<String> availAudioDrivers;

  bool quit, warnQuit, willCommit, edit, editClone, isPatUnique, modified, displayError, displayExporting, vgmExportLoop, vgmExportPatternHints;
  bool vgmExportDirectStream, displayInsTypeList, displayWaveSizeList;
  bool portrait, injectBackUp, mobileMenuOpen, warnColorPushed;
  bool wantCaptureKeyboard, oldWantCaptureKeyboard, displayMacroMenu;
  bool displayNew, displayExport, displayPalette, fullScreen, preserveChanPos, sysDupCloneChannels, sysDupEnd, noteInputPoly, notifyWaveChange;
  bool wantScrollListIns, wantScrollListWave, wantScrollListSample;
  bool displayPendingIns, pendingInsSingle, displayPendingRawSample, snesFilterHex, modTableHex, displayEditString;
  bool displayPendingSamples, replacePendingSample;
  bool displayExportingROM;
  bool changeCoarse;
  bool mobileEdit;
  bool killGraphics;
  bool safeMode;
  bool midiWakeUp;
  bool makeDrumkitMode;
  bool audioEngineChanged, settingsChanged, debugFFT;
  bool willExport[DIV_MAX_CHIPS];
  int vgmExportVersion;
  int vgmExportTrailingTicks;
  int cvHiScore;
  int drawHalt;
  int macroPointSize;
  int waveEditStyle;
  int displayInsTypeListMakeInsSample;
  int makeDrumkitOctave;
  int mobileEditPage;
  int wheelCalmDown;
  int shallDetectScale;
  int cpuCores;
  float secondTimer;
  unsigned int userEvents;
  float mobileMenuPos, autoButtonSize, mobileEditAnim;
  ImVec2 mobileEditButtonPos, mobileEditButtonSize;
  const int* curSysSection;
  DivInstrumentFM opllPreview;
  short fmPreview[FM_PREVIEW_SIZE];
  bool updateFMPreview, fmPreviewOn, fmPreviewPaused;
  void* fmPreviewOPN;
  void* fmPreviewOPM;
  void* fmPreviewOPL;
  void* fmPreviewOPLL;
  void* fmPreviewOPZ;
  void* fmPreviewOPZInterface;
  void* fmPreviewESFM;
  String* editString;
  SDL_Event userEvent;

  String pendingRawSample;
  int pendingRawSampleDepth, pendingRawSampleChannels, pendingRawSampleRate;
  bool pendingRawSampleUnsigned, pendingRawSampleBigEndian, pendingRawSampleSwapNibbles, pendingRawSampleReplace;

  ImGuiWindowFlags globalWinFlags;

  FurnaceGUIFileDialogs curFileDialog;
  FurnaceGUIWarnings warnAction;
  FurnaceGUIWarnings postWarnAction;
  FurnaceGUIMobileScenes mobScene;

  FurnaceGUIFileDialog* fileDialog;

  int scrW, scrH, scrConfW, scrConfH, canvasW, canvasH;
  int scrX, scrY, scrConfX, scrConfY;
  bool scrMax, sysManagedScale;

  double dpiScale;

  double aboutScroll, aboutSin;
  float aboutHue;

  std::atomic<double> backupTimer;
  std::future<bool> backupTask;
  std::mutex backupLock;
  String backupPath;

  std::vector<FurnaceGUIBackupEntry> backupEntries;
  std::future<bool> backupEntryTask;
  std::mutex backupEntryLock;
  uint64_t totalBackupSize;
  bool refreshBackups;

  std::mutex midiLock;
  FixedQueue<TAMidiMessage,4096> midiQueue;
  MIDIMap midiMap;
  int learning;

  ImFont* mainFont;
  ImFont* iconFont;
  ImFont* furIconFont;
  ImFont* patFont;
  ImFont* bigFont;
  ImFont* headFont;
  ImWchar* fontRange;
  ImWchar* fontRangeB;
  ImVec4 uiColors[GUI_COLOR_MAX];
  ImVec4 volColors[128];
  ImU32 pitchGrad[256];
  ImU32 volGrad[256];
  ImU32 noteGrad[256];
  ImU32 panGrad[256];
  ImU32 insGrad[256];
  ImU32 sysCmd1Grad[256];
  ImU32 sysCmd2Grad[256];

  char noteOffLabel[32];
  char noteRelLabel[32];
  char macroRelLabel[32];
  char emptyLabel[32];
  char emptyLabel2[32];

  std::vector<int> songOrdersLengths; // lengths of all orders (for drawing song export progress)
  int songLength; // length of all the song in rows
  int songLoopedSectionLength; // length of looped part of the song
  int songFadeoutSectionLength; // length of fading part of the song
  bool songHasSongEndCommand; // song has "Song end" command (FFxx)
  int lengthOfOneFile; // length of one rendering pass. song length times num of loops + fadeout
  int totalLength; // total length of render (lengthOfOneFile times num of files for per-channel export)
  float curProgress;
  int totalFiles;

  struct Settings {
    bool settingsChanged;
    int mainFontSize, patFontSize, headFontSize, iconSize;
    int audioEngine;
    int audioQuality;
    int audioHiPass;
    int audioChans;
    int arcadeCore;
    int ym2612Core;
    int snCore;
    int nesCore;
    int fdsCore;
    int c64Core;
    int pokeyCore;
    int opn1Core;
    int opnaCore;
    int opnbCore;
    int opl2Core;
    int opl3Core;
    int opl4Core;
    int esfmCore;
    int opllCore;
    int ayCore;
    int bubsysQuality;
    int dsidQuality;
    int gbQuality;
    int ndsQuality;
    int pceQuality;
    int pnQuality;
    int saaQuality;
    int sccQuality;
    int smQuality; 
    int swanQuality;
    int vbQuality;
    int arcadeCoreRender;
    int ym2612CoreRender;
    int snCoreRender;
    int nesCoreRender;
    int fdsCoreRender;
    int c64CoreRender;
    int pokeyCoreRender;
    int opn1CoreRender;
    int opnaCoreRender;
    int opnbCoreRender;
    int opl2CoreRender;
    int opl3CoreRender;
    int opl4CoreRender;
    int esfmCoreRender;
    int opllCoreRender;
    int ayCoreRender;
    int bubsysQualityRender;
    int dsidQualityRender;
    int gbQualityRender;
    int ndsQualityRender;
    int pceQualityRender;
    int pnQualityRender;
    int saaQualityRender;
    int sccQualityRender;
    int smQualityRender; 
    int swanQualityRender;
    int vbQualityRender;
    int pcSpeakerOutMethod;
    String yrw801Path;
    String tg100Path;
    String mu5Path;
    int mainFont;
    int headFont;
    int patFont;
    int audioRate;
    int audioBufSize;
    int patRowsBase;
    int orderRowsBase;
    int soloAction;
    int ctrlWheelModifier;
    int pullDeleteBehavior;
    int wrapHorizontal;
    int wrapVertical;
    int macroView;
    int fmNames;
    int allowEditDocking;
    int chipNames;
    int overflowHighlight;
    int flatNotes;
    int germanNotation;
    int stepOnDelete;
    int scrollStep;
    int sysSeparators;
    int forceMono;
    int controlLayout;
    int statusDisplay;
    float dpiScale;
    int viewPrevPattern;
    int guiColorsBase;
    int guiColorsShading;
    int avoidRaisingPattern;
    int insFocusesPattern;
    int stepOnInsert;
    int unifiedDataView;
    int sysFileDialog;
    int roundedWindows;
    int roundedButtons;
    int roundedMenus;
    int roundedTabs;
    int roundedScrollbars;
    int loadJapanese;
    int loadChinese;
    int loadChineseTraditional;
    int loadKorean;
    int loadFallback;
    int fmLayout;
    int sampleLayout;
    int waveLayout;
    int susPosition;
    int effectCursorDir;
    int cursorPastePos;
    int titleBarInfo;
    int titleBarSys;
    int frameBorders;
    int effectDeletionAltersValue;
    int oscRoundedCorners;
    int oscTakesEntireWindow;
    int oscBorder;
    int oscEscapesBoundary;
    int oscMono;
    int oscAntiAlias;
    float oscLineSize;
    int separateFMColors;
    int insEditColorize;
    int metroVol;
    int sampleVol;
    int pushNibble;
    int scrollChangesOrder;
    int oplStandardWaveNames;
    int cursorMoveNoScroll;
    int lowLatency;
    int notePreviewBehavior;
    int powerSave;
    int absorbInsInput;
    int eventDelay;
    int moveWindowTitle;
    int hiddenSystems;
    int horizontalDataView;
    int noMultiSystem;
    int oldMacroVSlider;
    int displayAllInsTypes;
    int displayPartial;
    int noteCellSpacing;
    int insCellSpacing;
    int volCellSpacing;
    int effectCellSpacing;
    int effectValCellSpacing;
    int doubleClickColumn;
    int blankIns;
    int dragMovesSelection;
    int cursorFollowsOrder;
    int unsignedDetune;
    int noThreadedInput;
    int saveWindowPos;
    int clampSamples;
    int saveUnusedPatterns;
    int channelColors;
    int channelTextColors;
    int channelStyle;
    int channelVolStyle;
    int channelFeedbackStyle;
    int channelFont;
    int channelTextCenter;
    int midiOutClock;
    int midiOutTime;
    int midiOutProgramChange;
    int midiOutMode;
    int midiOutTimeRate;
    int maxRecentFile;
    int centerPattern;
    int ordersCursor;
    int persistFadeOut;
    int exportLoops;
    double exportFadeOut;
    int macroLayout;
    float doubleClickTime;
    int oneDigitEffects;
    int disableFadeIn;
    int alwaysPlayIntro;
    int iCannotWait;
    int orderButtonPos;
    int compress;
    int newPatternFormat;
    int renderClearPos;
    int insertBehavior;
    int pullDeleteRow;
    int newSongBehavior;
    int memUsageUnit;
    int cursorFollowsWheel;
    int noDMFCompat;
    int removeInsOff;
    int removeVolOff;
    int playOnLoad;
    int insTypeMenu;
    int capitalMenuBar;
    int centerPopup;
    int insIconsStyle;
    int classicChipOptions;
    int exportOptionsLayout;
    int wasapiEx;
    int chanOscThreads;
    int renderPoolThreads;
    int writeInsNames;
    int readInsNames;
    int fontBackend;
    int fontHinting;
    int fontBitmap;
    int fontAutoHint;
    int fontAntiAlias;
    int fontOversample;
    int selectAssetOnLoad;
    int basicColors;
    int playbackTime;
    int shaderOsc;
    int cursorWheelStep;
    int vsync;
    int frameRateLimit;
    int displayRenderTime;
    int inputRepeat;
    int glRedSize;
    int glGreenSize;
    int glBlueSize;
    int glAlphaSize;
    int glDepthSize;
    int glStencilSize;
    int glBufferSize;
    int glDoubleBuffer;
    int backupEnable;
    int backupInterval;
    int backupMaxCopies;
    int autoFillSave;
    int autoMacroStepSize;
    unsigned int maxUndoSteps;
    float vibrationStrength;
    int vibrationLength;
    int s3mOPL3;
    String mainFontPath;
    String headFontPath;
    String patFontPath;
    String audioDevice;
    String midiInDevice;
    String midiOutDevice;
    String renderBackend;
    String renderDriver;
    String initialSysName;
    String noteOffLabel;
    String noteRelLabel;
    String macroRelLabel;
    String emptyLabel;
    String emptyLabel2;
    String sdlAudioDriver;
    String defaultAuthorName;
    String locale;
    DivConfig initialSys;

    Settings():
      settingsChanged(false),
      mainFontSize(GUI_FONT_SIZE_DEFAULT),
      patFontSize(GUI_FONT_SIZE_DEFAULT),
      headFontSize(27),
      iconSize(GUI_ICON_SIZE_DEFAULT),
      audioEngine(DIV_AUDIO_SDL),
      audioQuality(0),
      audioHiPass(1),
      audioChans(2),
      arcadeCore(0),
      ym2612Core(0),
      snCore(0),
      nesCore(0),
      fdsCore(0),
      c64Core(0),
      pokeyCore(1),
      opn1Core(1),
      opnaCore(1),
      opnbCore(1),
      opl2Core(0),
      opl3Core(0),
      opl4Core(0),
      esfmCore(0),
      opllCore(0),
      ayCore(0),
      bubsysQuality(3),
      dsidQuality(3),
      gbQuality(3),
      ndsQuality(3),
      pceQuality(3),
      pnQuality(3),
      saaQuality(3),
      sccQuality(3),
      smQuality(3),
      swanQuality(3),
      vbQuality(3),
      arcadeCoreRender(1),
      ym2612CoreRender(0),
      snCoreRender(0),
      nesCoreRender(0),
      fdsCoreRender(1),
      c64CoreRender(1),
      pokeyCoreRender(1),
      opn1CoreRender(1),
      opnaCoreRender(1),
      opnbCoreRender(1),
      opl2CoreRender(0),
      opl3CoreRender(0),
      opl4CoreRender(0),
      esfmCoreRender(0),
      opllCoreRender(0),
      ayCoreRender(0),
      bubsysQualityRender(3),
      dsidQualityRender(3),
      gbQualityRender(3),
      ndsQualityRender(3),
      pceQualityRender(3),
      pnQualityRender(3),
      saaQualityRender(3),
      sccQualityRender(3),
      smQualityRender(3),
      swanQualityRender(3),
      vbQualityRender(3),
      pcSpeakerOutMethod(0),
      yrw801Path(""),
      tg100Path(""),
      mu5Path(""),
      mainFont(0),
      patFont(0),
      audioRate(44100),
      audioBufSize(1024),
      patRowsBase(0),
      orderRowsBase(1),
      soloAction(0),
      ctrlWheelModifier(0),
      pullDeleteBehavior(1),
      wrapHorizontal(0),
      wrapVertical(0),
      macroView(0),
      fmNames(0),
      allowEditDocking(1),
      chipNames(0),
      overflowHighlight(0),
      germanNotation(0),
      stepOnDelete(0),
      scrollStep(0),
      sysSeparators(1),
      forceMono(0),
      controlLayout(3),
      statusDisplay(0),
      dpiScale(0.0f),
      viewPrevPattern(1),
      guiColorsBase(0),
      guiColorsShading(0),
      avoidRaisingPattern(0),
      insFocusesPattern(1),
      stepOnInsert(0),
      unifiedDataView(0),
      sysFileDialog(1),
      roundedWindows(1),
      roundedButtons(1),
      roundedMenus(0),
      roundedTabs(1),
      roundedScrollbars(1),
      loadJapanese(0),
      loadChinese(0),
      loadChineseTraditional(0),
      loadKorean(0),
      loadFallback(1),
      fmLayout(4),
      sampleLayout(0),
      waveLayout(0),
      susPosition(0),
      effectCursorDir(1),
      cursorPastePos(1),
      titleBarInfo(1),
      titleBarSys(1),
      frameBorders(0),
      effectDeletionAltersValue(1),
      oscRoundedCorners(1),
      oscTakesEntireWindow(0),
      oscBorder(1),
      oscEscapesBoundary(0),
      oscMono(1),
      oscAntiAlias(1),
      oscLineSize(1.0f),
      separateFMColors(0),
      insEditColorize(0),
      metroVol(100),
      sampleVol(50),
      pushNibble(0),
      scrollChangesOrder(0),
      oplStandardWaveNames(0),
      cursorMoveNoScroll(0),
      lowLatency(0),
      notePreviewBehavior(1),
      powerSave(1),
      absorbInsInput(0),
      eventDelay(0),
      moveWindowTitle(1),
      hiddenSystems(0),
      horizontalDataView(0),
      noMultiSystem(0),
      oldMacroVSlider(0),
      displayAllInsTypes(0),
      displayPartial(0),
      noteCellSpacing(0),
      insCellSpacing(0),
      volCellSpacing(0),
      effectCellSpacing(0),
      effectValCellSpacing(0),
      doubleClickColumn(1),
      blankIns(0),
      dragMovesSelection(1),
      cursorFollowsOrder(1),
      unsignedDetune(0),
      noThreadedInput(0),
      clampSamples(0),
      saveUnusedPatterns(0),
      channelColors(1),
      channelTextColors(0),
      channelStyle(1),
      channelVolStyle(0),
      channelFeedbackStyle(1),
      channelFont(1),
      channelTextCenter(1),
      midiOutClock(0),
      midiOutTime(0),
      midiOutProgramChange(0),
      midiOutMode(1),
      midiOutTimeRate(0),
      maxRecentFile(10),
      centerPattern(0),
      ordersCursor(1),
      persistFadeOut(1),
      exportLoops(0),
      exportFadeOut(0.0),
      macroLayout(0),
      doubleClickTime(0.3f),
      oneDigitEffects(0),
      disableFadeIn(0),
      alwaysPlayIntro(0),
      iCannotWait(0),
      orderButtonPos(2),
      compress(1),
      newPatternFormat(1),
      renderClearPos(0),
      insertBehavior(1),
      pullDeleteRow(1),
      newSongBehavior(0),
      memUsageUnit(1),
      cursorFollowsWheel(0),
      noDMFCompat(0),
      removeInsOff(0),
      removeVolOff(0),
      playOnLoad(0),
      insTypeMenu(1),
      capitalMenuBar(0),
      centerPopup(1),
      insIconsStyle(1),
      classicChipOptions(0),
      exportOptionsLayout(1),
      wasapiEx(0),
      chanOscThreads(0),
      renderPoolThreads(0),
      writeInsNames(0),
      readInsNames(1),
      fontBackend(1),
      fontHinting(0),
      fontBitmap(0),
      fontAutoHint(1),
      fontAntiAlias(1),
      fontOversample(GUI_OVERSAMPLE_DEFAULT),
      selectAssetOnLoad(1),
      basicColors(1),
      playbackTime(1),
      shaderOsc(1),
      cursorWheelStep(0),
      vsync(1),
      frameRateLimit(60),
      displayRenderTime(0),
      inputRepeat(1),
      glRedSize(8),
      glGreenSize(8),
      glBlueSize(8),
      glAlphaSize(0),
      glDepthSize(24),
      glStencilSize(0),
      glBufferSize(32),
      glDoubleBuffer(1),
      backupEnable(1),
      backupInterval(30),
      backupMaxCopies(5),
      autoFillSave(0),
      autoMacroStepSize(0),
      maxUndoSteps(100),
      vibrationStrength(0.5f),
      vibrationLength(20),
      s3mOPL3(1),
      mainFontPath(""),
      headFontPath(""),
      patFontPath(""),
      audioDevice(""),
      midiInDevice(""),
      midiOutDevice(""),
      renderBackend(""),
      renderDriver(""),
      initialSysName("Sega Genesis/Mega Drive"),
      noteOffLabel("OFF"),
      noteRelLabel("==="),
      macroRelLabel("REL"),
      emptyLabel("..."),
      emptyLabel2(".."),
      sdlAudioDriver(""),
      defaultAuthorName(""),
      locale("") {}
  } settings;

  struct Tutorial {
    bool introPlayed;
    bool protoWelcome;
    Tutorial():
#ifdef SUPPORT_XP
      introPlayed(true),
#else
      introPlayed(false),
#endif
      protoWelcome(false) {
    }
  } tutorial;

  char finalLayoutPath[4096];

  bool localeRequiresJapanese;
  bool localeRequiresChinese;
  bool localeRequiresChineseTrad;
  bool localeRequiresKorean;
  std::vector<ImWchar> localeExtraRanges;

  DivInstrument* prevInsData;
  DivInstrument cachedCurIns;
  DivInstrument* cachedCurInsPtr;
  bool insEditMayBeDirty;

  unsigned char* pendingLayoutImport;
  size_t pendingLayoutImportLen;
  int pendingLayoutImportStep;
  FixedQueue<bool*,64> pendingLayoutImportReopen;

  int curIns, curWave, curSample, curOctave, curOrder, playOrder, prevIns, oldRow, editStep, editStepCoarse, soloChan, orderEditMode, orderCursor;
  int loopOrder, loopRow, loopEnd, isClipping, newSongCategory, latchTarget;
  int wheelX, wheelY, dragSourceX, dragSourceXFine, dragSourceY, dragDestinationX, dragDestinationXFine, dragDestinationY, oldBeat, oldBar;
  int curGroove, exitDisabledTimer;
  int curPaletteChoice, curPaletteType;
  float soloTimeout;

  int purgeYear, purgeMonth, purgeDay;

  bool patExtraButtons, patChannelNames, patChannelPairs;
  unsigned char patChannelHints;

  bool newSongFirstFrame, paletteFirstFrame, oldRowChanged;
  bool editControlsOpen, ordersOpen, insListOpen, songInfoOpen, patternOpen, insEditOpen;
  bool waveListOpen, waveEditOpen, sampleListOpen, sampleEditOpen, aboutOpen, settingsOpen;
  bool mixerOpen, debugOpen, inspectorOpen, oscOpen, volMeterOpen, statsOpen, compatFlagsOpen;
  bool pianoOpen, notesOpen, channelsOpen, regViewOpen, logOpen, effectListOpen, chanOscOpen;
  bool subSongsOpen, findOpen, spoilerOpen, patManagerOpen, sysManagerOpen, clockOpen, speedOpen;
  bool groovesOpen, xyOscOpen, memoryOpen, csPlayerOpen, cvOpen, userPresetsOpen;

  bool cvNotSerious;

  bool shortIntro;
  bool insListDir, waveListDir, sampleListDir;

  bool clockShowReal, clockShowRow, clockShowBeat, clockShowMetro, clockShowTime;
  float clockMetroTick[16];

  SelectionPoint selStart, selEnd, cursor, cursorDrag, dragStart, dragEnd;
  bool selecting, selectingFull, dragging, curNibble, orderNibble, followOrders, followPattern, changeAllOrders, mobileUI;
  bool collapseWindow, demandScrollX, fancyPattern, firstFrame, tempoView, waveHex, waveSigned, waveGenVisible, lockLayout, editOptsVisible, latchNibble, nonLatchNibble;
  bool keepLoopAlive, keepGrooveAlive, orderScrollLocked, orderScrollTolerance, dragMobileMenu, dragMobileEditButton, wantGrooveListFocus;
  unsigned char lastAssetType;
  FurnaceGUIWindows curWindow, nextWindow, curWindowLast;
  std::atomic<FurnaceGUIWindows> curWindowThreadSafe;
  std::atomic<bool> failedNoteOn;
  float peak[DIV_MAX_OUTPUTS];
  float patChanX[DIV_MAX_CHANS+1];
  float patChanSlideY[DIV_MAX_CHANS+1];
  float lastPatternWidth, longThreshold;
  float buttonLongThreshold;
  String nextDesc;
  String nextDescName;

  OperationMask opMaskDelete, opMaskPullDelete, opMaskInsert, opMaskPaste, opMaskTransposeNote, opMaskTransposeValue;
  OperationMask opMaskInterpolate, opMaskFade, opMaskInvertVal, opMaskScale;
  OperationMask opMaskRandomize, opMaskFlip, opMaskCollapseExpand;
  short latchNote, latchIns, latchVol, latchEffect, latchEffectVal;

  DivWaveSynth wavePreview;
  int wavePreviewLen, wavePreviewHeight;
  bool wavePreviewInit, wavePreviewPaused;

  // bit 31: ctrl
  // bit 30: reserved for SDL scancode mask
  // bit 29: shift
  // bit 28: meta (win)
  // bit 27: alt
  // bit 24-26: reserved
  int actionKeys[GUI_ACTION_MAX];

  std::map<int,int> actionMapGlobal;
  std::map<int,int> actionMapPat;
  std::map<int,int> actionMapOrders;
  std::map<int,int> actionMapSample;
  std::map<int,int> actionMapInsList;
  std::map<int,int> actionMapWaveList;
  std::map<int,int> actionMapSampleList;

  std::vector<DivRegWrite> pgProgram;
  int pgSys, pgAddr, pgVal;

  std::vector<FurnaceGUIFindQuery> curQuery;
  std::vector<FurnaceGUIQueryResult> curQueryResults;
  bool curQueryRangeX, curQueryBackwards;
  int curQueryRangeXMin, curQueryRangeXMax;
  int curQueryRangeY;
  int curQueryEffectPos;

  int queryReplaceEffectCount;
  int queryReplaceEffectPos;
  int queryReplaceNoteMode;
  int queryReplaceInsMode;
  int queryReplaceVolMode;
  int queryReplaceEffectMode[8];
  int queryReplaceEffectValMode[8];
  int queryReplaceNote;
  int queryReplaceIns;
  int queryReplaceVol;
  int queryReplaceEffect[8];
  int queryReplaceEffectVal[8];
  bool queryReplaceNoteDo;
  bool queryReplaceInsDo;
  bool queryReplaceVolDo;
  bool queryReplaceEffectDo[8];
  bool queryReplaceEffectValDo[8];
  bool queryViewingResults;

  struct ActiveNote {
    int chan;
    int note;
    ActiveNote(int c, int n):
      chan(c),
      note(n) {}
  };
  std::vector<ActiveNote> activeNotes;
  std::vector<DivCommand> cmdStream;
  std::vector<Particle> particles;
  std::vector<std::pair<DivInstrument*,bool>> pendingIns;
  std::vector<std::pair<DivSample*,bool>> pendingSamples;

  std::vector<FurnaceGUISysCategory> sysCategories;

  std::vector<String> audioLoadFormats;

  bool wavePreviewOn;
  SDL_Scancode wavePreviewKey;
  int wavePreviewNote;

  bool samplePreviewOn;
  SDL_Scancode samplePreviewKey;
  int samplePreviewNote;

  // SDL_Scancode,int
  std::map<int,int> noteKeys;
  // SDL_Keycode,int
  std::map<int,int> valueKeys;

  // currently active touch points
  std::vector<TouchPoint> activePoints;
  // one frame points
  std::vector<TouchPoint> pressedPoints;
  std::vector<TouchPoint> releasedPoints;

  int sampleMapSelStart;
  int sampleMapSelEnd;
  int sampleMapDigit;
  int sampleMapColumn;
  bool sampleMapFocused, sampleMapWaitingInput;

  ImVec2 macroDragStart;
  ImVec2 macroDragAreaSize;
  unsigned char* macroDragCTarget;
  int* macroDragTarget;
  int macroDragLen;
  int macroDragMin, macroDragMax;
  int macroDragLastX, macroDragLastY;
  int macroDragScroll;
  bool macroDragBitMode;
  bool macroDragInitialValueSet;
  bool macroDragInitialValue;
  bool macroDragChar;
  bool macroDragBit30;
  bool macroDragSettingBit30;
  bool macroDragLineMode;
  bool macroDragMouseMoved;
  ImVec2 macroDragLineInitial;
  ImVec2 macroDragLineInitialV;
  bool macroDragActive;
  FurnaceGUIMacroDesc lastMacroDesc;
  int macroOffX, macroOffY;
  float macroScaleX, macroScaleY;
  int macroRandMin, macroRandMax;

  ImVec2 macroLoopDragStart;
  ImVec2 macroLoopDragAreaSize;
  unsigned char* macroLoopDragTarget;
  int macroLoopDragLen;
  bool macroLoopDragActive;

  FurnaceGUIMacroEditState macroEditStateFM, macroEditStateOP[4], macroEditStateMacros;

  ImVec2 waveDragStart;
  ImVec2 waveDragAreaSize;
  int* waveDragTarget;
  int waveDragLen;
  int waveDragMin, waveDragMax;
  bool waveDragActive;

  int bindSetTarget, bindSetPrevValue;
  bool bindSetActive, bindSetPending;

  float nextScroll, nextAddScroll, orderScroll, orderScrollSlideOrigin;

  ImVec2 orderScrollRealOrigin;
  ImVec2 dragMobileMenuOrigin;

  uint64_t layoutTimeBegin, layoutTimeEnd, layoutTimeDelta;
  uint64_t renderTimeBegin, renderTimeEnd, renderTimeDelta;
  uint64_t drawTimeBegin, drawTimeEnd, drawTimeDelta;
  uint64_t swapTimeBegin, swapTimeEnd, swapTimeDelta;
  uint64_t eventTimeBegin, eventTimeEnd, eventTimeDelta;
  uint64_t nextPresentTime;

  FurnaceGUIPerfMetric perfMetrics[64];
  int perfMetricsLen;

  FurnaceGUIPerfMetric perfMetricsLast[64];
  int perfMetricsLastLen;

  std::map<FurnaceGUIImages,FurnaceGUIImage*> images;

  int chanToMove, sysToMove, sysToDelete, opToMove;
  int assetToMove, dirToMove;

  ImVec2 patWindowPos, patWindowSize;

  // pattern view specific
  ImVec2 fourChars, threeChars, twoChars;
  ImVec2 noteCellSize, insCellSize, volCellSize, effectCellSize, effectValCellSize;
  SelectionPoint sel1, sel2;
  int dummyRows;
  int transposeAmount, randomizeMin, randomizeMax, fadeMin, fadeMax, collapseAmount;
  float scaleMax;
  bool fadeMode, randomMode, haveHitBounds;
  signed char pendingStepUpdate;

  int oldOrdersLen;
  DivOrders oldOrders;
  std::map<unsigned short,DivPattern*> oldPatMap;
  FixedQueue<UndoStep,256> undoHist;
  FixedQueue<UndoStep,256> redoHist;
  FixedQueue<CursorJumpPoint,256> cursorUndoHist;
  FixedQueue<CursorJumpPoint,256> cursorRedoHist;

  // sample editor specific
  double sampleZoom;
  double prevSampleZoom;
  double minSampleZoom;
  int samplePos;
  int resizeSize, silenceSize;
  double resampleTarget;
  int resampleStrat;
  float amplifyVol;
  int sampleSelStart, sampleSelEnd;
  bool sampleInfo, sampleCompatRate;
  bool sampleDragActive, sampleDragMode, sampleDrag16, sampleZoomAuto;
  // 0: start
  // 1: end
  unsigned char sampleSelTarget;
  void* sampleDragTarget;
  ImVec2 sampleDragStart;
  ImVec2 sampleDragAreaSize;
  unsigned int sampleDragLen;
  float sampleFilterL, sampleFilterB, sampleFilterH, sampleFilterRes, sampleFilterCutStart, sampleFilterCutEnd;
  int sampleCrossFadeLoopLength, sampleCrossFadeLoopLaw;
  unsigned char sampleFilterPower;
  short* sampleClipboard;
  size_t sampleClipboardLen;
  bool openSampleResizeOpt, openSampleResampleOpt, openSampleAmplifyOpt, openSampleSilenceOpt, openSampleFilterOpt, openSampleCrossFadeOpt;

  // mixer
  // 0xxx: output
  // 1xxx: input
  unsigned int selectedPortSet;
  // any value not between 0 and 15 are "none"
  int selectedSubPort;
  unsigned int hoveredPortSet;
  int hoveredSubPort;
  bool portDragActive, displayHiddenPorts, displayInternalPorts;
  ImVec2 subPortPos;

  // oscilloscope
  int oscTotal, oscWidth;
  float* oscValues[DIV_MAX_OUTPUTS];
  float* oscValuesAverage;
  float oscZoom;
  float oscWindowSize;
  float oscInput, oscInput1;
  bool oscZoomSlider;

  // per-channel oscilloscope
  int chanOscCols, chanOscAutoColsType, chanOscColorX, chanOscColorY;
  float chanOscWindowSize, chanOscTextX, chanOscTextY, chanOscAmplify, chanOscLineSize;
  bool chanOscWaveCorr, chanOscOptions, updateChanOscGradTex, chanOscUseGrad, chanOscNormalize, chanOscRandomPhase;
  String chanOscTextFormat;
  ImVec4 chanOscColor, chanOscTextColor;
  Gradient2D chanOscGrad;
  FurnaceGUITexture* chanOscGradTex;
  DivWorkPool* chanOscWorkPool;
  float chanOscLP0[DIV_MAX_CHANS];
  float chanOscLP1[DIV_MAX_CHANS];
  float chanOscVol[DIV_MAX_CHANS];
  float chanOscBright[DIV_MAX_CHANS];
  unsigned short lastNeedlePos[DIV_MAX_CHANS];
  unsigned short lastCorrPos[DIV_MAX_CHANS];
  struct ChanOscStatus {
    double* inBuf;
    fftw_complex* outBuf;
    double* corrBuf;
    DivDispatchOscBuffer* relatedBuf;
    size_t inBufPos;
    double inBufPosFrac;
    double waveLen;
    int waveLenBottom, waveLenTop, relatedCh;
    float pitch, windowSize, phaseOff;
    unsigned short needle;
    bool ready, loudEnough, waveCorr;
    fftw_plan plan;
    fftw_plan planI;
    PendingDrawOsc drawOp;
    float oscTex[2048];
    ChanOscStatus():
      inBuf(NULL),
      outBuf(NULL),
      corrBuf(NULL),
      relatedBuf(NULL),
      inBufPos(0),
      inBufPosFrac(0.0f),
      waveLen(0.0),
      waveLenBottom(0),
      waveLenTop(0),
      relatedCh(0),
      pitch(0.0f),
      windowSize(1.0f),
      phaseOff(0.0f),
      needle(0),
      ready(false),
      loudEnough(false),
      waveCorr(false),
      plan(NULL),
      planI(NULL) {}
  } chanOscChan[DIV_MAX_CHANS];

  // x-y oscilloscope
  FurnaceGUITexture* xyOscPointTex;
  bool xyOscOptions;
  int xyOscXChannel;
  bool xyOscXInvert;
  int xyOscYChannel;
  bool xyOscYInvert;
  float xyOscZoom;
  int xyOscSamples;
  float xyOscDecayTime;
  float xyOscIntensity;
  float xyOscThickness;

  // visualizer
  float keyHit[DIV_MAX_CHANS];
  float keyHit1[DIV_MAX_CHANS];
  int lastIns[DIV_MAX_CHANS];

  // log window
  bool followLog;

  // piano
  enum PianoLayoutMode {
    PIANO_LAYOUT_STANDARD = 0,
    PIANO_LAYOUT_CONTINUOUS,
    PIANO_LAYOUT_AUTOMATIC,
    PIANO_LAYOUT_MAX
  };

  enum PianoInputPadMode {
    PIANO_INPUT_PAD_DISABLE = 0,
    PIANO_INPUT_PAD_REPLACE,
    PIANO_INPUT_PAD_SPLIT_AUTO,
    PIANO_INPUT_PAD_SPLIT_VISIBLE,
    PIANO_INPUT_PAD_MAX
  };

  int pianoOctaves, pianoOctavesEdit;
  bool pianoOptions, pianoSharePosition, pianoOptionsSet;
  float pianoKeyHit[180];
  bool pianoKeyPressed[180];
  bool pianoReadonly;
  int pianoOffset, pianoOffsetEdit;
  int pianoView, pianoInputPadMode;
  
  // effect sorting / searching
  bool effectsShow[10];
  ImGuiTextFilter effectSearch;

  // TX81Z
  bool hasACED;
  unsigned char acedData[23];

  // wave generator
  int waveGenBaseShape;
  int waveInterpolation;
  float waveGenDuty;
  int waveGenPower;
  float waveGenInvertPoint;
  float waveGenAmp[16];
  float waveGenPhase[16];
  float waveGenTL[4];
  int fmWaveform[4];
  int waveGenMult[4];
  int waveGenFB[4];
  int waveGenScaleX, waveGenScaleY, waveGenOffsetX, waveGenOffsetY, waveGenSmooth;
  float waveGenAmplify;
  bool waveGenFMCon0[5];
  bool waveGenFMCon1[5];
  bool waveGenFMCon2[5];
  bool waveGenFMCon3[5];
  bool waveGenFMCon4[5];
  bool waveGenFM;

  // intro
  double introPos;
  double introSkip;
  double monitorPos;
  int mustClear;
  float initialScreenWipe;
  bool introSkipDo, introStopped;
  ImVec2 introMin, introMax;

  // tutorial
  int curTutorial, curTutorialStep;

  // command stream player
  ImGuiListClipper csClipper;

  // export options
  DivAudioExportOptions audioExportOptions;
  int dmfExportVersion;
  FurnaceGUIExportTypes curExportType;

  // ROM export specific
  DivROMExportOptions romTarget;
  DivConfig romConfig;
  bool romMultiFile;
  bool romExportSave;
  String romFilterName, romFilterExt;
  String romExportPath;
  DivROMExport* pendingExport;
  bool romExportAvail[DIV_ROM_MAX];
  bool romExportExists;

  // user presets window
  std::vector<int> selectedUserPreset;

  std::vector<String> randomDemoSong;

  void drawExportAudio(bool onWindow=false);
  void drawExportVGM(bool onWindow=false);
  void drawExportROM(bool onWindow=false);
  void drawExportText(bool onWindow=false);
  void drawExportCommand(bool onWindow=false);
  void drawExportDMF(bool onWindow=false);

  void drawSSGEnv(unsigned char type, const ImVec2& size);
  void drawWaveform(unsigned char type, bool opz, const ImVec2& size);
  void drawWaveformSID3(unsigned char type, const ImVec2& size);
  void drawAlgorithm(unsigned char alg, FurnaceGUIFMAlgs algType, const ImVec2& size);
  void drawESFMAlgorithm(DivInstrumentESFM& esfm, const ImVec2& size);
  void drawFMEnv(unsigned char tl, unsigned char ar, unsigned char dr, unsigned char d2r, unsigned char rr, unsigned char sl, unsigned char sus, unsigned char egt, unsigned char algOrGlobalSus, float maxTl, float maxArDr, float maxRr, const ImVec2& size, unsigned short instType);
  void drawSID3Env(unsigned char tl, unsigned char ar, unsigned char dr, unsigned char d2r, unsigned char rr, unsigned char sl, unsigned char sus, unsigned char egt, unsigned char algOrGlobalSus, float maxTl, float maxArDr, float maxRr, const ImVec2& size, unsigned short instType);
  void drawGBEnv(unsigned char vol, unsigned char len, unsigned char sLen, bool dir, const ImVec2& size);
  bool drawSysConf(int chan, int sysPos, DivSystem type, DivConfig& flags, bool modifyOnChange, bool fromMenu=false);
  void kvsConfig(DivInstrument* ins, bool supportsKVS=true);
  void drawFMPreview(const ImVec2& size);
  void renderFMPreview(const DivInstrument* ins, int pos=0);
  void renderFMPreviewOPN(const DivInstrumentFM& params, int pos=0);
  void renderFMPreviewOPM(const DivInstrumentFM& params, int pos=0);
  void renderFMPreviewOPLL(const DivInstrumentFM& params, int pos=0);
  void renderFMPreviewOPL(const DivInstrumentFM& params, int pos=0);
  void renderFMPreviewOPZ(const DivInstrumentFM& params, int pos=0);
  void renderFMPreviewESFM(const DivInstrumentFM& params, const DivInstrumentESFM& esfmParams, int pos=0);

  // combo with locale
  static bool LocalizedComboGetter(void* data, int idx, const char** out_text);

  // these ones offer ctrl-wheel fine value changes.
  bool isCtrlWheelModifierHeld() const;
  bool CWSliderScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format=NULL, ImGuiSliderFlags flags=0);
  bool CWVSliderScalar(const char* label, const ImVec2& size, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format=NULL, ImGuiSliderFlags flags=0);
  bool CWSliderInt(const char* label, int* v, int v_min, int v_max, const char* format="%d", ImGuiSliderFlags flags=0);
  bool CWSliderFloat(const char* label, float* v, float v_min, float v_max, const char* format="%.3f", ImGuiSliderFlags flags=0);
  bool CWVSliderInt(const char* label, const ImVec2& size, int* v, int v_min, int v_max, const char* format="%d", ImGuiSliderFlags flags=0);

  // inverted checkbox
  bool InvCheckbox(const char* label, bool* value);

  // mixer stuff
  ImVec2 calcPortSetSize(String label, int ins, int outs);
  bool portSet(String label, unsigned int portSetID, int ins, int outs, int activeIns, int activeOuts, int& clickedPort, std::map<unsigned int,ImVec2>& portPos);

  void updateWindowTitle();
  void updateROMExportAvail();
  void autoDetectSystem();
  void autoDetectSystemIter(std::vector<FurnaceGUISysDef>& category, bool& isMatch, std::map<DivSystem,int>& defCountMap, std::map<DivSystem,DivConfig>& defConfMap, std::map<DivSystem,int>& sysCountMap, std::map<DivSystem,DivConfig>& sysConfMap);
  void prepareLayout();
  ImVec4 channelColor(int ch);
  ImVec4 channelTextColor(int ch);

  void centerNextWindow(const char* name, float w, float h);

  void readOsc();
  void calcChanOsc();

  void pushAccentColors(const ImVec4& one, const ImVec4& two, const ImVec4& border, const ImVec4& borderShadow);
  void popAccentColors();

  void pushDestColor();
  void popDestColor();
  void pushWarningColor(bool warnCond, bool errorCond=false);
  void popWarningColor();

  void sameLineMaybe(float width=-1.0f);

  float calcBPM(const DivGroovePattern& speeds, float hz, int vN, int vD);

  void patternRow(int i, bool isPlaying, float lineHeight, int chans, int ord, const DivPattern** patCache, bool inhibitSel);

  void drawMacroEdit(FurnaceGUIMacroDesc& i, int totalFit, float availableWidth, int index);
  void drawMacros(std::vector<FurnaceGUIMacroDesc>& macros, FurnaceGUIMacroEditState& state);
  void alterSampleMap(int column, int val);

  void insTabFMModernHeader(DivInstrument* ins);
  void insTabFM(DivInstrument* ins);
  void insTabWavetable(DivInstrument* ins);
  void insTabSample(DivInstrument* ins);

  void drawOrderButtons();

  void actualWaveList();
  void actualSampleList();

  void insListItem(int index, int dir, int asset);
  void waveListItem(int index, float* wavePreview, int dir, int asset);
  void sampleListItem(int index, int dir, int asset);

  void drawSysDefs(std::vector<FurnaceGUISysDef>& category, bool& accepted, std::vector<int>& sysDefStack, bool& alreadyHover);
  void printPresets(std::vector<FurnaceGUISysDef>& items, size_t depth, std::vector<int>& depthStack);
  FurnaceGUISysDef* selectPreset(std::vector<FurnaceGUISysDef>& items);

  void toggleMobileUI(bool enable, bool force=false);

  void pushToggleColors(bool status);
  void popToggleColors();

  void highlightWindow(const char* winName);

  FurnaceGUIImage* getImage(FurnaceGUIImages image);
  FurnaceGUITexture* getTexture(FurnaceGUIImages image, FurnaceGUIBlendMode blendMode=GUI_BLEND_MODE_BLEND);
  void drawImage(ImDrawList* dl, FurnaceGUIImages image, const ImVec2& pos, const ImVec2& scale, double rotate, const ImVec2& uvMin, const ImVec2& uvMax, const ImVec4& imgColor);

  void drawMobileControls();
  void drawMobileOrderSel();
  void drawEditControls();
  void drawSongInfo(bool asChild=false);
  void drawSpeed(bool asChild=false);
  void drawGrooves();
  void drawOrders();
  void drawPattern();
  void drawInsList(bool asChild=false);
  void drawInsEdit();
  void drawInsSID3(DivInstrument* ins);
  void drawWaveList(bool asChild=false);
  void drawWaveEdit();
  void drawSampleList(bool asChild=false);
  void drawSampleEdit();
  void drawMixer();
  void drawOsc();
  void drawChanOsc();
  void drawVolMeter();
  void drawStats();
  void drawMemory();
  void drawCompatFlags();
  void drawPiano();
  void drawNotes(bool asChild=false);
  void drawChannels();
  void drawPatManager();
  void drawSysManager();
  void drawRegView();
  void drawAbout();
  void drawIntro(double introTime, bool monitor=false);
  void drawSettings();
  void drawDebug();
  void drawCSPlayer();
  void drawNewSong();
  void drawPalette();
  void drawExport();
  void drawLog();
  void drawEffectList();
  void drawSubSongs(bool asChild=false);
  void drawFindReplace();
  void drawSpoiler();
  void drawClock();
  void drawTutorial();
  void drawXYOsc();
  void drawUserPresets();

  void parseKeybinds();
  void promptKey(int which);
  void doAction(int what);

  bool importColors(String path);
  bool exportColors(String path);
  bool importKeybinds(String path);
  bool exportKeybinds(String path);
  bool importLayout(String path);
  bool exportLayout(String path);
  bool importConfig(String path);
  bool exportConfig(String path);

  float computeGradPos(int type, int chan);

  void resetColors();
  void resetKeybinds();

  bool splitBackupName(const char* input, String& backupName, struct tm& backupTime);
  void purgeBackups(int year, int month, int day);

  void readConfig(DivConfig& conf, FurnaceGUISettingGroups groups=GUI_SETTINGS_ALL);
  void writeConfig(DivConfig& conf, FurnaceGUISettingGroups groups=GUI_SETTINGS_ALL);

  void syncSettings();
  void commitSettings();
  void syncTutorial();
  void commitTutorial();
  void syncState();
  void commitState(DivConfig& conf);
  void processDrags(int dragX, int dragY);
  void processPoint(SDL_Event& ev);

  void startSelection(int xCoarse, int xFine, int y, bool fullRow=false);
  void updateSelection(int xCoarse, int xFine, int y, bool fullRow=false);
  void finishSelection();
  void finishDrag();

  void moveCursor(int x, int y, bool select);
  void moveCursorPrevChannel(bool overflow);
  void moveCursorNextChannel(bool overflow);
  void moveCursorTop(bool select);
  void moveCursorBottom(bool select);
  void editAdvance();
  void prepareUndo(ActionType action, UndoRegion region=UndoRegion());
  void makeUndo(ActionType action, UndoRegion region=UndoRegion());
  void doSelectAll();
  void doDelete();
  void doPullDelete();
  void doInsert();
  void moveSelected(int x, int y);
  void doTranspose(int amount, OperationMask& mask);
  String doCopy(bool cut, bool writeClipboard, const SelectionPoint& sStart, const SelectionPoint& sEnd);
  void doPasteFurnace(PasteMode mode, int arg, bool readClipboard, String clipb, std::vector<String> data, int startOff, bool invalidData, UndoRegion ur);
  void doPasteMPT(PasteMode mode, int arg, bool readClipboard, String clipb, std::vector<String> data, int mptFormat, UndoRegion ur);
  void doPaste(PasteMode mode=GUI_PASTE_MODE_NORMAL, int arg=0, bool readClipboard=true, String clipb="");
  void doChangeIns(int ins);
  void doInterpolate();
  void doFade(int p0, int p1, bool mode);
  void doInvertValues();
  void doScale(float top);
  void doRandomize(int bottom, int top, bool mode);
  void doFlip();
  void doCollapse(int divider, const SelectionPoint& sStart, const SelectionPoint& sEnd);
  void doExpand(int multiplier, const SelectionPoint& sStart, const SelectionPoint& sEnd);
  void doCollapseSong(int divider);
  void doExpandSong(int multiplier);
  void doAbsorbInstrument();
  void doUndo();
  void doRedo();
  void doFind();
  void doReplace();
  void doDrag();
  void editOptions(bool topMenu);
  DivSystem systemPicker(bool fullWidth);
  void noteInput(int num, int key, int vol=-1);
  void valueInput(int num, bool direct=false, int target=-1);
  void orderInput(int num);

  void doGenerateWave();

  CursorJumpPoint getCurrentCursorJumpPoint();
  void applyCursorJumpPoint(const CursorJumpPoint& spot);
  void makeCursorUndo();
  void doCursorUndo();
  void doCursorRedo();

  void doUndoSample();
  void doRedoSample();

  void checkRecordInstrumentUndoStep();
  void doUndoInstrument();
  void doRedoInstrument();

  void play(int row=0);
  void setOrder(unsigned char order, bool forced=false);
  void stop();
  void endIntroTune();

  void previewNote(int refChan, int note, bool autoNote=false);
  void stopPreviewNote(SDL_Scancode scancode, bool autoNote=false);

  void keyDown(SDL_Event& ev);
  void keyUp(SDL_Event& ev);

  void pointDown(int x, int y, int button);
  void pointUp(int x, int y, int button);
  void pointMotion(int x, int y, int xrel, int yrel);

  void openFileDialog(FurnaceGUIFileDialogs type);
  int save(String path, int dmfVersion);
  int load(String path);
  int loadStream(String path);
  void openRecentFile(String path);
  void pushRecentFile(String path);
  void pushRecentSys(const char* path);
  void exportAudio(String path, DivAudioExportModes mode);
  void delFirstBackup(String name);

  bool parseSysEx(unsigned char* data, size_t len);

  void applyUISettings(bool updateFonts=true);
  void initSystemPresets();

  void initRandomDemoSong();
  bool loadRandomDemoSong();

  bool loadUserPresets(bool redundancy=true, String path="", bool append=false);
  bool saveUserPresets(bool redundancy=true, String path="");

  void encodeMMLStr(String& target, int* macro, int macroLen, int macroLoop, int macroRel, bool hex=false, bool bit30=false);
  void decodeMMLStr(String& source, int* macro, unsigned char& macroLen, unsigned char& macroLoop, int macroMin, int macroMax, unsigned char& macroRel, bool bit30=false);
  void decodeMMLStrW(String& source, int* macro, int& macroLen, int macroMin, int macroMax, bool hex=false);

  String encodeKeyMap(std::map<int,int>& map);
  void decodeKeyMap(std::map<int,int>& map, String source);

  bool initRender();
  bool quitRender();

  ImFont* addFontZlib(const void* data, size_t len, float size_pixels, const ImFontConfig* font_cfg=NULL, const ImWchar* glyph_ranges=NULL);

  const char* getSystemName(DivSystem which);
  const char* getSystemPartNumber(DivSystem sys, DivConfig& flags);

  public:
    void editStr(String* which);
    void showWarning(String what, FurnaceGUIWarnings type);
    void showError(String what);
    String getLastError();
    const char* noteNameNormal(short note, short octave);
    const char* noteName(short note, short octave);
    bool decodeNote(const char* what, short& note, short& octave);
    void bindEngine(DivEngine* eng);
    void enableSafeMode();
    void updateScroll(int amount);
    void addScroll(int amount);
    void setFileName(String name);
    void runBackupThread();
    void pushPartBlend();
    void popPartBlend();
    void runPendingDrawOsc(PendingDrawOsc* which);
    bool detectOutOfBoundsWindow(SDL_Rect& failing);
    int processEvent(SDL_Event* ev);
    bool loop();
    bool finish(bool saveConfig=false);
    bool init();
    bool requestQuit();
    FurnaceGUI();
};

#endif
