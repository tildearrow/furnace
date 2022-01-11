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
  GUI_COLOR_INSTR_FM,
  GUI_COLOR_INSTR_STD,
  GUI_COLOR_INSTR_GB,
  GUI_COLOR_INSTR_C64,
  GUI_COLOR_INSTR_AMIGA,
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
  GUI_WINDOW_SAMPLE_EDIT
};

enum FurnaceGUIFileDialogs {
  GUI_FILE_OPEN,
  GUI_FILE_SAVE,
  GUI_FILE_SAMPLE_OPEN,
  GUI_FILE_SAMPLE_SAVE
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

  bool quit, warnQuit, willCommit, edit, modified, displayError;

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

  int mainFontSize, patFontSize;
  size_t maxUndoSteps;

  char finalLayoutPath[4096];

  int curIns, curWave, curSample, curOctave, oldRow, oldOrder, oldOrder1, editStep;
  bool editControlsOpen, ordersOpen, insListOpen, songInfoOpen, patternOpen, insEditOpen;
  bool waveListOpen, waveEditOpen, sampleListOpen, sampleEditOpen, aboutOpen, settingsOpen;
  SelectionPoint selStart, selEnd, cursor;
  bool selecting, curNibble, extraChannelButtons, followOrders, followPattern, changeAllOrders;
  FurnaceGUIWindows curWindow;

  bool noteOffOnRelease;
  SDL_Keycode noteOffOnReleaseKey;
  int noteOffOnReleaseChan;

  std::map<SDL_Keycode,int> noteKeys;
  std::map<SDL_Keycode,int> valueKeys;

  int arpMacroScroll;

  ImVec2 macroDragStart;
  ImVec2 macroDragAreaSize;
  int* macroDragTarget;
  int macroDragLen;
  int macroDragMin, macroDragMax;
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
  void drawAbout();
  void drawSettings();

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
  void doCopy(bool cut);
  void doPaste();
  void doUndo();
  void doRedo();

  void keyDown(SDL_Event& ev);
  void keyUp(SDL_Event& ev);

  void openFileDialog(FurnaceGUIFileDialogs type);
  int save(String path);
  int load(String path);

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
