#include "../engine/engine.h"
#include "SDL_keycode.h"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include <SDL.h>
#include <deque>
#include <map>

enum FurnaceGUIColors {
  GUI_COLOR_BACKGROUND=0,
  GUI_COLOR_FRAME_BACKGROUND,
  GUI_COLOR_HEADER,
  GUI_COLOR_ITEM_BACKGROUND,
  GUI_COLOR_ITEM_FOREGROUND,
  GUI_COLOR_EDITING,
  GUI_COLOR_INSTR_FM,
  GUI_COLOR_INSTR_STD,
  GUI_COLOR_INSTR_GB,
  GUI_COLOR_INSTR_C64,
  GUI_COLOR_INSTR_AMIGA,
  GUI_COLOR_INSTR_PCE,
  GUI_COLOR_INSTR_AY,
  GUI_COLOR_INSTR_AY8930,
  GUI_COLOR_INSTR_TIA,
  GUI_COLOR_INSTR_SAA1099,
  GUI_COLOR_INSTR_UNKNOWN,
  GUI_COLOR_CHANNEL_FM,
  GUI_COLOR_CHANNEL_PULSE,
  GUI_COLOR_CHANNEL_NOISE,
  GUI_COLOR_CHANNEL_WAVE,
  GUI_COLOR_CHANNEL_PCM,
  GUI_COLOR_CHANNEL_OP,
  GUI_COLOR_CHANNEL_MUTED,
  GUI_COLOR_PATTERN_CURSOR,
  GUI_COLOR_PATTERN_CURSOR_HOVER,
  GUI_COLOR_PATTERN_CURSOR_ACTIVE,
  GUI_COLOR_PATTERN_SELECTION,
  GUI_COLOR_PATTERN_SELECTION_HOVER,
  GUI_COLOR_PATTERN_SELECTION_ACTIVE,
  GUI_COLOR_PATTERN_HI_1,
  GUI_COLOR_PATTERN_HI_2,
  GUI_COLOR_PATTERN_ROW_INDEX,
  GUI_COLOR_PATTERN_ACTIVE,
  GUI_COLOR_PATTERN_INACTIVE,
  GUI_COLOR_PATTERN_INS,
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
  GUI_COLOR_EE_VALUE,
  GUI_COLOR_PLAYBACK_STAT,
  GUI_COLOR_MAX
};

enum FurnaceGUIWindows {
  GUI_WINDOW_NOTHING=0,
  GUI_WINDOW_EDIT_CONTROLS,
  GUI_WINDOW_SONG_INFO,
  GUI_WINDOW_ORDERS,
  GUI_WINDOW_INS_LIST,
  GUI_WINDOW_PATTERN,
  GUI_WINDOW_INS_EDIT,
  GUI_WINDOW_WAVE_LIST,
  GUI_WINDOW_WAVE_EDIT,
  GUI_WINDOW_SAMPLE_LIST,
  GUI_WINDOW_SAMPLE_EDIT,
  GUI_WINDOW_MIXER
};

enum FurnaceGUIFileDialogs {
  GUI_FILE_OPEN,
  GUI_FILE_SAVE,
  GUI_FILE_INS_OPEN,
  GUI_FILE_INS_SAVE,
  GUI_FILE_WAVE_OPEN,
  GUI_FILE_WAVE_SAVE,
  GUI_FILE_SAMPLE_OPEN,
  GUI_FILE_SAMPLE_SAVE,
  GUI_FILE_EXPORT_AUDIO_ONE,
  GUI_FILE_EXPORT_AUDIO_PER_SYS,
  GUI_FILE_EXPORT_AUDIO_PER_CHANNEL,
  GUI_FILE_EXPORT_VGM,
  GUI_FILE_EXPORT_ROM
};

enum FurnaceGUIWarnings {
  GUI_WARN_QUIT,
  GUI_WARN_NEW,
  GUI_WARN_OPEN
};

struct SelectionPoint {
  int xCoarse, xFine;
  int y;
  SelectionPoint():
    xCoarse(0), xFine(0), y(0) {}
};

enum ActionType {
  GUI_ACTION_CHANGE_ORDER,
  GUI_ACTION_PATTERN_EDIT,
  GUI_ACTION_PATTERN_DELETE,
  GUI_ACTION_PATTERN_PULL,
  GUI_ACTION_PATTERN_PUSH,
  GUI_ACTION_PATTERN_CUT,
  GUI_ACTION_PATTERN_PASTE
};

struct UndoPatternData {
  int chan, pat, row, col;
  short oldVal, newVal;
  UndoPatternData(int c, int p, int r, int co, short v1, short v2):
    chan(c),
    pat(p),
    row(r),
    col(co),
    oldVal(v1),
    newVal(v2) {}
};

struct UndoOrderData {
  int chan, ord;
  unsigned char oldVal, newVal;
  UndoOrderData(int c, int o, unsigned char v1, unsigned char v2):
    chan(c),
    ord(o),
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
};

class FurnaceGUI {
  DivEngine* e;

  SDL_Window* sdlWin;
  SDL_Renderer* sdlRend;

  String workingDir, fileName, clipboard, warnString, errorString, lastError, curFileName;

  bool quit, warnQuit, willCommit, edit, modified, displayError, displayExporting;

  FurnaceGUIFileDialogs curFileDialog;
  FurnaceGUIWarnings warnAction;

  int scrW, scrH;

  double dpiScale;

  int aboutScroll, aboutSin;
  float aboutHue;

  ImFont* mainFont;
  ImFont* iconFont;
  ImFont* patFont;
  ImFont* bigFont;
  ImVec4 uiColors[GUI_COLOR_MAX];
  ImVec4 volColors[128];

  struct Settings {
    int mainFontSize, patFontSize, iconSize;
    int audioEngine;
    int audioQuality;
    int arcadeCore;
    int mainFont;
    int patFont;
    int audioRate;
    int audioBufSize;
    int patRowsBase;
    int orderRowsBase;
    int soloAction;
    int pullDeleteBehavior;
    int wrapHorizontal;
    int wrapVertical;
    unsigned int maxUndoSteps;
    String mainFontPath;
    String patFontPath;

    Settings():
      mainFontSize(18),
      patFontSize(18),
      iconSize(16),
      audioEngine(DIV_AUDIO_SDL),
      audioQuality(0),
      arcadeCore(0),
      mainFont(0),
      patFont(0),
      audioRate(44100),
      audioBufSize(1024),
      patRowsBase(0),
      orderRowsBase(1),
      soloAction(0),
      pullDeleteBehavior(1),
      wrapHorizontal(0),
      wrapVertical(0),
      maxUndoSteps(100),
      mainFontPath(""),
      patFontPath("") {}
  } settings;

  char finalLayoutPath[4096];

  int curIns, curWave, curSample, curOctave, oldRow, oldOrder, oldOrder1, editStep, exportLoops, soloChan, soloTimeout, orderEditMode, orderCursor;
  bool editControlsOpen, ordersOpen, insListOpen, songInfoOpen, patternOpen, insEditOpen;
  bool waveListOpen, waveEditOpen, sampleListOpen, sampleEditOpen, aboutOpen, settingsOpen;
  bool mixerOpen;
  SelectionPoint selStart, selEnd, cursor;
  bool selecting, curNibble, orderNibble, extraChannelButtons, followOrders, followPattern, changeAllOrders;
  FurnaceGUIWindows curWindow;

  struct ActiveNote {
    int chan;
    int note;
    ActiveNote(int c, int n):
      chan(c),
      note(n) {}
  };
  std::vector<ActiveNote> activeNotes;

  bool wavePreviewOn;
  SDL_Scancode wavePreviewKey;
  int wavePreviewNote;

  bool samplePreviewOn;
  SDL_Scancode samplePreviewKey;
  int samplePreviewNote;

  std::map<SDL_Scancode,int> noteKeys;
  std::map<SDL_Keycode,int> valueKeys;

  int arpMacroScroll;

  ImVec2 macroDragStart;
  ImVec2 macroDragAreaSize;
  unsigned char* macroDragCTarget;
  int* macroDragTarget;
  int macroDragLen;
  int macroDragMin, macroDragMax;
  int macroDragLastX, macroDragLastY;
  int macroDragBitOff;
  bool macroDragBitMode;
  bool macroDragInitialValueSet;
  bool macroDragInitialValue;
  bool macroDragChar;
  bool macroDragActive;

  ImVec2 macroLoopDragStart;
  ImVec2 macroLoopDragAreaSize;
  signed char* macroLoopDragTarget;
  int macroLoopDragLen;
  bool macroLoopDragActive;

  ImVec2 waveDragStart;
  ImVec2 waveDragAreaSize;
  int* waveDragTarget;
  int waveDragLen;
  int waveDragMin, waveDragMax;
  bool waveDragActive;

  float nextScroll, nextAddScroll;

  ImVec2 patWindowPos, patWindowSize;

  int oldOrdersLen;
  DivOrders oldOrders;
  DivPattern* oldPat[128];
  std::deque<UndoStep> undoHist;
  std::deque<UndoStep> redoHist;

  void updateWindowTitle();
  void prepareLayout();

  void drawEditControls();
  void drawSongInfo();
  void drawOrders();
  void drawPattern();
  void drawInsList();
  void drawInsEdit();
  void drawWaveList();
  void drawWaveEdit();
  void drawSampleList();
  void drawSampleEdit();
  void drawMixer();
  void drawAbout();
  void drawSettings();

  void syncSettings();
  void commitSettings();
  void processDrags(int dragX, int dragY);

  void startSelection(int xCoarse, int xFine, int y);
  void updateSelection(int xCoarse, int xFine, int y);
  void finishSelection();

  void moveCursor(int x, int y);
  void moveCursorTop();
  void moveCursorBottom();
  void editAdvance();
  void prepareUndo(ActionType action);
  void makeUndo(ActionType action);
  void doSelectAll();
  void doDelete();
  void doPullDelete();
  void doInsert();
  void doTranspose(int amount);
  void doCopy(bool cut);
  void doPaste();
  void doUndo();
  void doRedo();

  void play();
  void stop();

  void previewNote(int refChan, int note);
  void stopPreviewNote(SDL_Scancode scancode);

  void keyDown(SDL_Event& ev);
  void keyUp(SDL_Event& ev);

  void openFileDialog(FurnaceGUIFileDialogs type);
  int save(String path);
  int load(String path);
  void exportAudio(String path, DivAudioExportModes mode);

  void showWarning(String what, FurnaceGUIWarnings type);
  void showError(String what);

  public:
    const char* noteName(short note, short octave);
    bool decodeNote(const char* what, short& note, short& octave);
    void bindEngine(DivEngine* eng);
    void updateScroll(int amount);
    void addScroll(int amount);
    bool loop();
    bool finish();
    bool init();
    FurnaceGUI();
};
