/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2025 tildearrow and contributors
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
#include "../ta-log.h"
#include "IconsFontAwesome4.h"
#include "imgui_internal.h"

#include "gif_load.h"

#ifdef _WIN32
#include <windows.h>
#include "../utfutils.h"
#else
#include <dirent.h>
#endif

#ifndef IS_MOBILE
#define CLICK_TO_OPEN(t) ImGui::TextColored(uiColors[GUI_COLOR_ACCENT_PRIMARY],t); \
  if (ImGui::IsItemHovered()) { \
    ImGui::SetTooltip("click to open"); \
    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand); \
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) { \
      SDL_OpenURL(t); \
    } \
  } \
  ImGui::SameLine(); \
  ImGui::Text(ICON_FA_CLIPBOARD); \
  if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) ImGui::SetTooltip("click to copy"); \
  if (ImGui::IsItemClicked()) { \
    ImGui::SetClipboardText(t); \
    tutorial.popupTimer=0; \
  }
#else
#define CLICK_TO_OPEN(t) ImGui::TextColored(uiColors[GUI_COLOR_ACCENT_PRIMARY],t); if (ImGui::IsItemClicked()) SDL_OpenURL(t);
#endif

enum FurnaceCVObjectTypes {
  CV_NULL=0,
  CV_PLAYER,
  CV_BULLET,
  CV_ENEMY_BULLET,
  CV_BOMB,
  CV_ENEMY_BOMB,
  CV_EXPLOSION,
  CV_ENEMY,
  CV_PLANE,
  CV_FURBALL,
  CV_MINE,
  CV_POWERUP_P,
  CV_POWERUP_S,
  CV_SPECIAL,
  CV_MOD_I,
  CV_MOD_S,
  CV_EXTRA_LIFE
};

struct FurnaceCVObject {
  FurnaceCV* cv;
  unsigned short type;
  unsigned short spriteDef[512];
  unsigned char spriteWidth, spriteHeight;
  bool dead;
  short x, y;
  unsigned char z, prio;
  short collX0, collX1, collY0, collY1;
  short frozen;
  
  virtual void collision(FurnaceCVObject* other);
  virtual void tick();
  FurnaceCVObject(FurnaceCV* p):
    cv(p),
    type(0),
    spriteWidth(2),
    spriteHeight(2),
    dead(false),
    x(0),
    y(0),
    z(0),
    prio(1),
    collX0(0),
    collX1(15),
    collY0(0),
    collY1(15),
    frozen(0) {
    memset(spriteDef,0,512*sizeof(unsigned short));
    spriteDef[0]=4;
    spriteDef[1]=5;
    spriteDef[2]=36;
    spriteDef[3]=37;
  }
  virtual ~FurnaceCVObject() {
  }
};

void FurnaceCVObject::collision(FurnaceCVObject* other) {
}

void FurnaceCVObject::tick() {
}

// special types:
// - 0: nothing
// - 1: "?" one of the following:
//   - 10/30: spawn more enemies
//   - 4/30: downgrades enemies
//   - 3/30: teleports you
//   - 3/30: stops all enemies (momentarily)
//   - 3/30: grants speed and invincible status
//   - 2/30: spawn purple tanks
//   - 2/30: spawn vortices
//   - 1/30: skip to next level
//   - 1/30: 5-up
//   - 1/30: call planes
// - 2: "T" teleports you
// - 3: "X" ripple shot
// - 4: "W" bidirectional shots (until next round)
// - 5: "S" stops all enemies for 10 seconds

struct FurnaceCV {
  SDL_Surface* surface;
  unsigned char* prioBuf;
  DivEngine* e;
  unsigned char* tileData;
  unsigned int tick;
  
  // state
  unsigned short* curStage;
  int stageWidth, stageHeight;
  int stageWidthPx, stageHeightPx;

  const char* typeAddr;
  unsigned char typeDelay;
  int typeX, typeY;
  int typeXStart, typeYStart;
  int typeXEnd, typeYEnd;

  int textWait, curText, transWait;
  int ticksToInit;

  bool inGame, inTransition, newHiScore, playSongs, pleaseInitSongs, gameOver;
  unsigned char lives, respawnTime, stage, shotType, lifeBank, specialType;
  int score;
  int hiScore;
  short lastPlayerX, lastPlayerY;
  short fxChanBase, fxInsBase;
  short speedTicks;
  short planeTime;
  float origSongRate;

  FixedQueue<unsigned char,16> weaponStack;
  
  // graphics
  unsigned short tile0[56][80];
  unsigned short tile1[56][80];
  unsigned short scrollX[2];
  unsigned short scrollY[2];
  unsigned char bgColor;
  std::vector<FurnaceCVObject*> sprite;
  // this offset is applied to sprites.
  int viewX, viewY;

  // input
  unsigned char joyInputPrev;
  unsigned char joyPressed;
  unsigned char joyReleased;
  unsigned char joyInput;

  template<typename T> T* createObject(short x=0, short y=0);
  template<typename T> T* createObjectNoPos();
  void buildStage(int which);

  void putText(int fontBase, bool fontHeight, String text, int x, int y);

  void startTyping(const char* text, int x, int y);

  void soundEffect(int ins, int chan, int note);
  void stopSoundEffect(int ins, int chan, int note);

  void addScore(int amount);

  void typeTick();

  void rasterH(int scanline);
  void render(unsigned char joyIn);
  void tileDataRead(struct GIF_WHDR* data);
  void loadInstruments();
  bool init(DivEngine* eng);
  void unload();

  FurnaceCV():
    surface(NULL),
    e(NULL),
    tileData(NULL),
    tick(0),
    curStage(NULL),
    stageWidth(40),
    stageHeight(28),
    stageWidthPx(320),
    stageHeightPx(224),
    typeAddr(NULL),
    typeDelay(0),
    typeX(0),
    typeY(0),
    typeXStart(0),
    typeYStart(0),
    typeXEnd(39),
    typeYEnd(27),
    textWait(60),
    curText(0),
    transWait(0),
    ticksToInit(2),
    inGame(false),
    inTransition(false),
    newHiScore(false),
    playSongs(true),
    pleaseInitSongs(false),
    gameOver(false),
    lives(5),
    respawnTime(0),
    stage(0),
    shotType(0),
    lifeBank(0),
    specialType(0),
    score(0),
    hiScore(25000),
    lastPlayerX(0),
    lastPlayerY(0),
    fxChanBase(-1),
    fxInsBase(-1),
    speedTicks(0),
    planeTime(0),
    origSongRate(60.0f),
    bgColor(0),
    viewX(0),
    viewY(0),
    joyInputPrev(0),
    joyPressed(0),
    joyReleased(0),
    joyInput(0) {
    memset(tile0,0,80*56*sizeof(short));
    memset(tile1,0,80*56*sizeof(short));

    scrollX[0]=0;
    scrollX[1]=0;
    scrollY[0]=0;
    scrollY[1]=0;
  }
};

struct FurnaceCVPlayer: FurnaceCVObject {
  short subX, subY;
  short speedX, speedY;
  unsigned char shootDir;
  unsigned char animFrame;
  short invincible;
  unsigned char shotTimer;
  bool doubleShot;

  void collision(FurnaceCVObject* other);
  void tick();
  FurnaceCVPlayer(FurnaceCV* p):
    FurnaceCVObject(p),
    subX(0),
    subY(0),
    speedX(0),
    speedY(0),
    shootDir(2),
    animFrame(0),
    invincible(120),
    shotTimer(4),
    doubleShot(false) {
      type=CV_PLAYER;
      spriteWidth=3;
      spriteHeight=3;
      spriteDef[0]=0x10;
      spriteDef[1]=0x11;
      spriteDef[2]=0x12;
      spriteDef[3]=0x30;
      spriteDef[4]=0x31;
      spriteDef[5]=0x32;
      spriteDef[6]=0x50;
      spriteDef[7]=0x51;
      spriteDef[8]=0x52;
      collX0=0;
      collX1=23;
      collY0=0;
      collY1=23;
    }
};

struct FurnaceCVBullet: FurnaceCVObject {
  short subX, subY;
  short speedX, speedY;
  unsigned char bulletType, orient, life;
  
  void killBullet();
  void setType(unsigned char t);
  void collision(FurnaceCVObject* other);
  void tick();
  FurnaceCVBullet(FurnaceCV* p):
    FurnaceCVObject(p),
    subX(0),
    subY(0),
    speedX(0),
    speedY(0),
    bulletType(0),
    orient(0),
    life(0) {
    type=CV_BULLET;
    spriteWidth=1;
    spriteHeight=1;
    spriteDef[0]=6;
    collX0=2;
    collX1=5;
    collY0=2;
    collY1=5;
  }
};

struct FurnaceCVEnemyBullet: FurnaceCVObject {
  short subX, subY;
  short speedX, speedY;
  unsigned char animFrame, bulletType;
  
  void setType(unsigned char type);
  void tick();
  FurnaceCVEnemyBullet(FurnaceCV* p):
    FurnaceCVObject(p),
    subX(0),
    subY(0),
    speedX(0),
    speedY(0),
    animFrame(rand()&0xff),
    bulletType(0) {
    type=CV_ENEMY_BULLET;
    spriteWidth=1;
    spriteHeight=1;
    spriteDef[0]=6;
    collX0=2;
    collX1=5;
    collY0=2;
    collY1=5;
  }
};

struct FurnaceCVEnemy: FurnaceCVObject {
  unsigned char enemyType;
  unsigned char health;
  unsigned char stopped;

  void setType(unsigned char type);
  FurnaceCVEnemy(FurnaceCV* p):
    FurnaceCVObject(p),
    enemyType(0),
    health(1),
    stopped(0) {
    type=CV_ENEMY;
  }
};

struct FurnaceCVEnemy1: FurnaceCVEnemy {
  unsigned char orient;
  unsigned char animFrame;
  short nextTime, shootTime;
  unsigned char shootCooldown;
  short orientCount;

  void collision(FurnaceCVObject* other);

  void tick();
  FurnaceCVEnemy1(FurnaceCV* p):
    FurnaceCVEnemy(p),
    orient(rand()&3),
    animFrame(0),
    nextTime(64+(rand()%600)),
    shootTime(8),
    shootCooldown(0),
    orientCount(0) {
    type=CV_ENEMY;
    spriteDef[0]=0x200;
    spriteDef[1]=0x201;
    spriteDef[2]=0x220;
    spriteDef[3]=0x221;
  }
};

struct FurnaceCVEnemyVortex: FurnaceCVEnemy {
  unsigned char animFrame;
  short nextTime, shootTime, speedX, speedY;

  void collision(FurnaceCVObject* other);

  void tick();
  FurnaceCVEnemyVortex(FurnaceCV* p):
    FurnaceCVEnemy(p),
    animFrame(0),
    nextTime(4+(rand()%140)),
    shootTime(360),
    speedX((rand()%5)-2),
    speedY((rand()%5)-2) {
    type=CV_ENEMY;
    prio=2;
    spriteDef[0]=0x480;
    spriteDef[1]=0x481;
    spriteDef[2]=0x4a0;
    spriteDef[3]=0x4a1;
  }
};

struct FurnaceCVEnemyPlane: FurnaceCVObject {
  unsigned char orient;
  short shootTime, speed;
  bool notifyPlayer;

  void collision(FurnaceCVObject* other);

  void tick();
  FurnaceCVEnemyPlane(FurnaceCV* p):
    FurnaceCVObject(p),
    orient(rand()&3),
    shootTime(40),
    speed(3),
    notifyPlayer(true) {
    type=CV_PLANE;
    speed=2+(p->stage>>2)+(rand()%3);
    prio=3;
    if (speed>8) speed=8;
    switch (orient) {
      case 0: case 2:
        spriteWidth=8;
        spriteHeight=5;
        break;
      case 1: case 3:
        spriteWidth=5;
        spriteHeight=8;
        break;
    }

    switch (orient) {
      case 0:
        for (int i=0; i<40; i++) {
          spriteDef[i]=0x4c0+(i&7)+((i>>3)<<5);
        }
        x=-80;
        y=rand()%(p->stageHeightPx-80);
        break;
      case 1:
        for (int i=0; i<40; i++) {
          spriteDef[i]=0x4d7+(i%5)+((i/5)<<5);
        }
        x=rand()%(p->stageWidthPx-80);
        y=p->stageHeightPx+16;
        break;
      case 2:
        for (int i=0; i<40; i++) {
          spriteDef[i]=0x4c9+(i&7)+((i>>3)<<5);
        }
        x=p->stageWidthPx+16;
        y=rand()%(p->stageHeightPx-80);
        break;
      case 3:
        for (int i=0; i<40; i++) {
          spriteDef[i]=0x4d2+(i%5)+((i/5)<<5);
        }
        x=rand()%(p->stageWidthPx-80);
        y=-80;
        break;
    }
  }
};

struct FurnaceCVExplTiny: FurnaceCVObject {
  unsigned char animFrame;

  void tick();
  FurnaceCVExplTiny(FurnaceCV* p):
    FurnaceCVObject(p),
    animFrame(0) {
    type=CV_EXPLOSION;
    spriteWidth=1;
    spriteHeight=1;
    spriteDef[0]=8;
  }
};

struct FurnaceCVExplMedium: FurnaceCVObject {
  unsigned char animFrame;

  void tick();
  FurnaceCVExplMedium(FurnaceCV* p):
    FurnaceCVObject(p),
    animFrame(0) {
    type=CV_EXPLOSION;
    spriteWidth=3;
    spriteHeight=3;
    spriteDef[0]=0x210;
    spriteDef[1]=0x211;
    spriteDef[2]=0x212;
    spriteDef[3]=0x230;
    spriteDef[4]=0x231;
    spriteDef[5]=0x232;
    spriteDef[6]=0x250;
    spriteDef[7]=0x251;
    spriteDef[8]=0x252;
  }
};

struct FurnaceCVFurBallMedium: FurnaceCVObject {
  unsigned char animFrame;

  void tick();
  FurnaceCVFurBallMedium(FurnaceCV* p):
    FurnaceCVObject(p),
    animFrame(0) {
    type=CV_FURBALL;
    spriteWidth=3;
    spriteHeight=3;
    spriteDef[0]=0x410;
    spriteDef[1]=0x411;
    spriteDef[2]=0x412;
    spriteDef[3]=0x430;
    spriteDef[4]=0x431;
    spriteDef[5]=0x432;
    spriteDef[6]=0x450;
    spriteDef[7]=0x451;
    spriteDef[8]=0x452;
  }
};

struct FurnaceCVFurBallLarge: FurnaceCVObject {
  unsigned char animFrame;

  void tick();
  FurnaceCVFurBallLarge(FurnaceCV* p):
    FurnaceCVObject(p),
    animFrame(0) {
    type=CV_FURBALL;
    spriteWidth=4;
    spriteHeight=4;
    spriteDef[0]=0x390;
    spriteDef[1]=0x391;
    spriteDef[2]=0x392;
    spriteDef[3]=0x393;
    spriteDef[4]=0x3b0;
    spriteDef[5]=0x3b1;
    spriteDef[6]=0x3b2;
    spriteDef[7]=0x3b3;
    spriteDef[8]=0x3d0;
    spriteDef[9]=0x3d1;
    spriteDef[10]=0x3d2;
    spriteDef[11]=0x3d3;
    spriteDef[12]=0x3f0;
    spriteDef[13]=0x3f1;
    spriteDef[14]=0x3f2;
    spriteDef[15]=0x3f3;
  }
};

struct FurnaceCVMine: FurnaceCVObject {
  void collision(FurnaceCVObject* other);
  FurnaceCVMine(FurnaceCV* p):
    FurnaceCVObject(p) {
      type=CV_MINE;
    }
};

struct FurnaceCVPowerupP: FurnaceCVObject {
  unsigned char life;
  void collision(FurnaceCVObject* other);
  void tick();
  FurnaceCVPowerupP(FurnaceCV* p):
    FurnaceCVObject(p),
    life(255) {
      type=CV_POWERUP_P;
    }
};

struct FurnaceCVPowerupS: FurnaceCVObject {
  unsigned char life;
  void collision(FurnaceCVObject* other);
  void tick();
  FurnaceCVPowerupS(FurnaceCV* p):
    FurnaceCVObject(p),
    life(255) {
      type=CV_POWERUP_S;
    }
};

struct FurnaceCVSpecial: FurnaceCVObject {
  unsigned char life;
  unsigned char specialType;
  void collision(FurnaceCVObject* other);
  void tick();
  FurnaceCVSpecial(FurnaceCV* p):
    FurnaceCVObject(p),
    life(255) {
      type=CV_SPECIAL;
      specialType=1+(rand()%5);
    }
};

struct FurnaceCVModI: FurnaceCVObject {
  unsigned char life;
  void collision(FurnaceCVObject* other);
  void tick();
  FurnaceCVModI(FurnaceCV* p):
    FurnaceCVObject(p),
    life(255) {
      type=CV_MOD_I;
    }
};

struct FurnaceCVModS: FurnaceCVObject {
  unsigned char life;
  void collision(FurnaceCVObject* other);
  void tick();
  FurnaceCVModS(FurnaceCV* p):
    FurnaceCVObject(p),
    life(255) {
      type=CV_MOD_S;
    }
};

struct FurnaceCVExtraLife: FurnaceCVObject {
  unsigned char life;
  void collision(FurnaceCVObject* other);
  void tick();
  FurnaceCVExtraLife(FurnaceCV* p):
    FurnaceCVObject(p),
    life(255) {
      type=CV_EXTRA_LIFE;
    }
};

static const char* cvText[]={
  // intro
  _N("Play demo songs?\n"
  "- Down: Play current song\n"
  "- Up: Play demo songs"),

  _N("Welcome to Combat Vehicle!\n\n"
  "Controls:\n"
  "X - Shoot      Arrow Key - Move\n"
  "Z - Special    Esc - Quit"),

  _N("GAME OVER"),

  _N("High Score!"),

  _N("Welcome to Combat Vehicle!\n\n"
  "Controls:\n"
  "B - Shoot      D-Pad - Move\n"
  "A - Special"),
};

void FurnaceGUI::syncTutorial() {
#ifdef SUPPORT_XP
  tutorial.introPlayed=e->getConfBool("tutIntroPlayed",true);
#else
  tutorial.introPlayed=e->getConfBool("tutIntroPlayed",false);
#endif
  tutorial.protoWelcome=e->getConfBool("tutProtoWelcome2",false);
  tutorial.importedMOD=e->getConfBool("tutImportedMOD",false);
  tutorial.importedS3M=e->getConfBool("tutImportedS3M",false);
  tutorial.importedXM=e->getConfBool("tutImportedXM",false);
  tutorial.importedIT=e->getConfBool("tutImportedIT",false);
}

void FurnaceGUI::commitTutorial() {
  e->setConf("tutIntroPlayed",tutorial.introPlayed);
  e->setConf("tutProtoWelcome2",tutorial.protoWelcome);
  e->setConf("tutImportedMOD",tutorial.importedMOD);
  e->setConf("tutImportedS3M",tutorial.importedS3M);
  e->setConf("tutImportedXM",tutorial.importedXM);
  e->setConf("tutImportedIT",tutorial.importedIT);
}

void FurnaceGUI::initRandomDemoSong() {
  std::vector<String> subDirs;
#if !defined(IS_MOBILE) && defined(FURNACE_DATADIR) && defined(SHOW_OPEN_ASSETS_MENU_ENTRY)
  String demoPath=FURNACE_DATADIR;
  demoPath+=DIR_SEPARATOR_STR;
  demoPath+="demos";
#else
#ifdef IS_MOBILE
  String demoPath="/storage/emulated/0/demos";
#else
  String demoPath="demos";
#endif
#endif

  logW("searching for demos in %s...",demoPath);

#ifdef _WIN32
  WIN32_FIND_DATAW de;
  String realDemoPath=demoPath;
  realDemoPath+=DIR_SEPARATOR_STR;
  realDemoPath+="*";
  HANDLE d=FindFirstFileW(utf8To16(realDemoPath.c_str()).c_str(),&de);
  if (d==INVALID_HANDLE_VALUE) {
    realDemoPath="..";
    realDemoPath+=DIR_SEPARATOR_STR;
    realDemoPath+="demos";
    realDemoPath+=DIR_SEPARATOR_STR;
    realDemoPath+="*";
    logW("OH NO");
    HANDLE d=FindFirstFileW(utf8To16(realDemoPath.c_str()).c_str(),&de);
    if (d==INVALID_HANDLE_VALUE) {
      logW("dang it");
      return;
    }
  }
  do {
    String u8Name=utf16To8(de.cFileName);
    if (u8Name==".") continue;
    if (u8Name=="..") continue;
    if (de.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) {
      String newPath=demoPath;
      newPath+=DIR_SEPARATOR_STR;
      newPath+=u8Name;
      logW("adding subdir %s",newPath);
      subDirs.push_back(newPath);
    } else if (strstr(u8Name.c_str(),".fur")!=NULL) {
      String newPath=demoPath;
      newPath+=DIR_SEPARATOR_STR;
      newPath+=u8Name;
      randomDemoSong.push_back(newPath);
    }
  } while (FindNextFileW(d,&de)!=0);
  FindClose(d);
#else
  DIR* d=opendir(demoPath.c_str());
  if (d==NULL) {
    demoPath="..";
    demoPath+=DIR_SEPARATOR_STR;
    demoPath+="demos";
    logW("OH NO");
    d=opendir(demoPath.c_str());
    if (d==NULL) {
      logW("dang it");
      return;
    }
  }

  struct dirent* de=NULL;
  while (true) {
    de=readdir(d);
    if (de==NULL) break;
    if (strcmp(de->d_name,".")==0) continue;
    if (strcmp(de->d_name,"..")==0) continue;
    if (de->d_type==DT_DIR) {
      String newPath=demoPath;
      newPath+=DIR_SEPARATOR_STR;
      newPath+=de->d_name;
      logW("adding subdir %s",newPath);
      subDirs.push_back(newPath);
    } else if (de->d_type==DT_REG && strstr(de->d_name,".fur")!=NULL) {
      String newPath=demoPath;
      newPath+=DIR_SEPARATOR_STR;
      newPath+=de->d_name;
      randomDemoSong.push_back(newPath);
    }
  }
  closedir(d);
#endif

  for (String& i: subDirs) {
#ifdef _WIN32
    WIN32_FIND_DATAW de1;
    String realI=i;
    realI+=DIR_SEPARATOR_STR;
    realI+="*.fur";
    HANDLE d1=FindFirstFileW(utf8To16(realI.c_str()).c_str(),&de1);
    if (d1==INVALID_HANDLE_VALUE) continue;
    do {
      String u8Name=utf16To8(de1.cFileName);
      String newPath=i;
      newPath+=DIR_SEPARATOR_STR;
      newPath+=u8Name;
      randomDemoSong.push_back(newPath);
    } while (FindNextFileW(d1,&de1)!=0);
    FindClose(d1);
#else
    DIR* d1=opendir(i.c_str());
    if (d1==NULL) continue;

    struct dirent* de1=NULL;
    while (true) {
      de1=readdir(d1);
      if (de1==NULL) break;
      if (strcmp(de1->d_name,".")==0) continue;
      if (strcmp(de1->d_name,"..")==0) continue;
      if (de1->d_type==DT_REG && strstr(de1->d_name,".fur")!=NULL) {
        String newPath=i;
        newPath+=DIR_SEPARATOR_STR;
        newPath+=de1->d_name;
        randomDemoSong.push_back(newPath);
      }
    }
    closedir(d1);
#endif
  }
}

bool FurnaceGUI::loadRandomDemoSong() {
  if (randomDemoSong.empty()) return false;
  String which=randomDemoSong[rand()%randomDemoSong.size()];
  logW("RDS LOAD... %s",which);
  load(which);
  return true;
}

void FurnaceGUI::drawTutorial() {
  // welcome
  if (!tutorial.protoWelcome) {
    ImGui::OpenPopup("Welcome");
  }
  if (ImGui::BeginPopupModal("Welcome",NULL,ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoTitleBar)) {
    ImGui::PushFont(bigFont);
    ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x-ImGui::CalcTextSize("Welcome!").x)*0.5);
    ImGui::Text(_("Welcome!"));
    ImGui::PopFont();

    ImGui::Text(_("welcome to Furnace, the biggest open-source chiptune tracker!"));

    ImGui::Separator();

    ImGui::TextWrapped(_("here are some tips to get you started:"));
    
    ImGui::TextWrapped(_(
      "- add an instrument by clicking on + in Instruments\n"
      "- click on the pattern view to focus it\n"
      "- channel columns have the following, in this order: note, instrument, volume and effects\n"
      "- hit space bar while on the pattern to toggle Edit Mode\n"
      "- click on the pattern or use arrow keys to move the cursor\n"
      "- values (instrument, volume, effects and effect values) are in hexadecimal\n"
      "- hit enter to play/stop the song\n"
      "- extend the song by adding more orders in the Orders window\n"
      "- click on the Orders matrix to change the patterns of a channel (left click increases; right click decreases)"
    ));

    ImGui::Separator();

    ImGui::TextWrapped(_(
      "if you are new to trackers, you may check the quick start guide:"
    ));
    CLICK_TO_OPEN("https://github.com/tildearrow/furnace/blob/master/doc/1-intro/quickstart.md")
    ImGui::TextWrapped(_(
      "if you need help, you may:\n"
      "- read the manual (a file called manual.pdf)\n"
      "- ask for help in Discussions"
    ));
    CLICK_TO_OPEN("https://github.com/tildearrow/furnace/discussions")

    ImGui::Separator();

    ImGui::TextWrapped(_("if you find any issues, be sure to report them! the issue tracker is here:"));
    CLICK_TO_OPEN("https://github.com/tildearrow/furnace/issues")

    if (ImGui::Button(_("OK"))) {
      tutorial.protoWelcome=true;
      commitTutorial();
      ImGui::CloseCurrentPopup();
    }

    ImGui::SetWindowPos(ImVec2(
      (canvasW-ImGui::GetWindowSize().x)*0.5,
      (canvasH-ImGui::GetWindowSize().y)*0.5
    ));

    if (tutorial.popupTimer<2.0f) {
      ImDrawList* dl=ImGui::GetForegroundDrawList();
      const ImVec2 winPos=ImGui::GetWindowPos();
      const ImVec2 txtSize=ImGui::CalcTextSize("copied!");
      const ImVec2 winSize=ImGui::GetWindowSize();
      dl->AddText(ImVec2(
        winPos.x+(winSize.x-txtSize.x)/2,
        winPos.y+(winSize.y-txtSize.y*2)
      ),ImGui::ColorConvertFloat4ToU32(uiColors[GUI_COLOR_TOGGLE_ON]),"copied!");
      tutorial.popupTimer+=ImGui::GetIO().DeltaTime;
    }
    ImGui::EndPopup();
  }

  if (cvOpen) {
    ImGui::SetNextWindowPos(ImVec2(0,0));
    ImGui::SetNextWindowSize(ImVec2(canvasW,canvasH));
    if (ImGui::Begin("Combat Vehicle",&cvOpen,ImGuiWindowFlags_NoDocking|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_Modal|ImGuiWindowFlags_NoTitleBar)) {
      ImVec2 dpadLoc=ImVec2(canvasW*0.22,canvasH*0.85); 
      ImVec2 buttonLoc=ImVec2(canvasW*0.78,canvasH*0.85);
      ImVec2 quitLoc=ImVec2(canvasW*0.5,canvasH*0.6);
      float oneUnit=MIN(canvasW,canvasH)*0.12;

      ImVec2 dpadUpStart=ImVec2(
        dpadLoc.x-oneUnit*1.75,
        dpadLoc.y-oneUnit*1.75
      );
      ImVec2 dpadUpEnd=ImVec2(
        dpadLoc.x+oneUnit*1.75,
        dpadLoc.y-oneUnit*0.5
      );
      ImVec2 dpadLeftEnd=ImVec2(
        dpadLoc.x-oneUnit*0.5,
        dpadLoc.y+oneUnit*1.75
      );
      ImVec2 dpadDownStart=ImVec2(
        dpadLoc.x-oneUnit*1.75,
        dpadLoc.y+oneUnit*0.5
      );
      ImVec2 dpadDownEnd=ImVec2(
        dpadLoc.x+oneUnit*1.75,
        dpadLoc.y+oneUnit*1.75
      );
      ImVec2 dpadRightStart=ImVec2(
        dpadLoc.x+oneUnit*0.5,
        dpadLoc.y-oneUnit*1.75
      );

      ImVec2 buttonBStart=ImVec2(
        buttonLoc.x-oneUnit*1.25,
        buttonLoc.y-oneUnit*0.5
      );
      ImVec2 buttonBEnd=ImVec2(
        buttonLoc.x-oneUnit*0.25,
        buttonLoc.y+oneUnit*0.5
      );
      ImVec2 buttonAStart=ImVec2(
        buttonLoc.x+oneUnit*0.25,
        buttonLoc.y-oneUnit*0.5
      );
      ImVec2 buttonAEnd=ImVec2(
        buttonLoc.x+oneUnit*1.25,
        buttonLoc.y+oneUnit*0.5
      );
      ImVec2 buttonQuitStart=ImVec2(
        quitLoc.x-oneUnit*0.5,
        quitLoc.y-oneUnit*0.25
      );
      ImVec2 buttonQuitEnd=ImVec2(
        quitLoc.x+oneUnit*0.5,
        quitLoc.y+oneUnit*0.25
      );

      unsigned char touchControls=0;

      if (mobileUI) {
        for (TouchPoint& i: activePoints) {
          // B
          if (i.x>=buttonBStart.x && i.y>=buttonBStart.y &&
              i.x<=buttonBEnd.x && i.y<=buttonBEnd.y) {
            touchControls|=1;
          }
          // A
          if (i.x>=buttonAStart.x && i.y>=buttonAStart.y &&
              i.x<=buttonAEnd.x && i.y<=buttonAEnd.y) {
            touchControls|=2;
          }
          // up
          if (i.x>=dpadUpStart.x && i.y>=dpadUpStart.y &&
              i.x<=dpadUpEnd.x && i.y<=dpadUpEnd.y) {
            touchControls|=16;
          }
          // down
          if (i.x>=dpadDownStart.x && i.y>=dpadDownStart.y &&
              i.x<=dpadDownEnd.x && i.y<=dpadDownEnd.y) {
            touchControls|=32;
          }
          // left
          if (i.x>=dpadUpStart.x && i.y>=dpadUpStart.y &&
              i.x<=dpadLeftEnd.x && i.y<=dpadLeftEnd.y) {
            touchControls|=64;
          }
          // right
          if (i.x>=dpadRightStart.x && i.y>=dpadRightStart.y &&
              i.x<=dpadDownEnd.x && i.y<=dpadDownEnd.y) {
            touchControls|=128;
          }
          // quit
          if (cv!=NULL) {
            if (i.x>=buttonQuitStart.x && i.y>=buttonQuitStart.y &&
                i.x<=buttonQuitEnd.x && i.y<=buttonQuitEnd.y) {
              cv->unload();
              delete cv;
              cv=NULL;
              cvOpen=false;
              return;
            }
          }
        }
      }

      if (cv==NULL) {
        initRandomDemoSong();
        cv=new FurnaceCV;
        cv->init(e);
        cv->hiScore=cvHiScore;
        lastCVFrame=SDL_GetPerformanceCounter();
        cvFrameTime=100000;
        cvFrameHold=0;
      }
      if (cvTex==NULL) {
        cvTex=rend->createTexture(true,320,224,false,bestTexFormat);
      } else if (!rend->isTextureValid(cvTex)) {
        rend->destroyTexture(cvTex);
        cvTex=rend->createTexture(true,320,224,false,bestTexFormat);
      }

      if (cv->pleaseInitSongs) {
        cv->pleaseInitSongs=false;
        if (cv->playSongs) {
          if (loadRandomDemoSong()) {
            cv->loadInstruments();
          }
        }
        cv->origSongRate=e->getHz();
      }

      WAKE_UP;

      if (cv->inTransition && cv->transWait==1) {
        // load random demo song
        int avgSpeed=0;
        for (int i=0; i<e->curSubSong->speeds.len; i++) {
          avgSpeed+=e->curSubSong->speeds.val[i];
        }
        int oneQuarter=(e->curSubSong->ordersLen*e->curSubSong->patLen*avgSpeed);
        if (e->curSubSong->speeds.len) oneQuarter/=e->curSubSong->speeds.len;
        oneQuarter=(oneQuarter*e->curSubSong->virtualTempoN)/e->curSubSong->virtualTempoD;
        oneQuarter/=e->curSubSong->hz;
        oneQuarter/=4;
        if (cv->playSongs && e->getTotalSeconds()>=oneQuarter) {
          if (loadRandomDemoSong()) {
            cv->loadInstruments();
            e->changeSongP(0);
            e->setOrder(0);
            e->play();
            cv->origSongRate=e->getHz();
          }
        }
      }

      if (cv->speedTicks>0) {
        cv->e->setSongRate(cv->origSongRate*1.5);
      }

      uint64_t nextFrame=SDL_GetPerformanceCounter();
      unsigned int mDivider=SDL_GetPerformanceFrequency()/1000000;
      int delta=(nextFrame-lastCVFrame)/mDivider;
      cvFrameTime=(cvFrameTime*15+delta)/16;
      cvFrameHold+=delta;
      if (cvFrameHold>=16667 || cvFrameTime>15000) {
        cv->render(touchControls);
        cvFrameHold%=16667;
      }
      lastCVFrame=nextFrame;
      
      if (cv->hiScore>cvHiScore) {
        cvHiScore=cv->hiScore;
      }

      if (cvTex!=NULL) {
        SDL_LockSurface(cv->surface);
        rend->updateTexture(cvTex,cv->surface->pixels,320*4);
        SDL_UnlockSurface(cv->surface);

        ImDrawList* dl=ImGui::GetForegroundDrawList();

        ImVec2 p0, p1;

        if (((double)canvasH/(double)canvasW)>0.7) {
          if (mobileUI) {
            p0=ImVec2(0.0,canvasH*0.05);
            p1=ImVec2(canvasW,(canvasH*0.05)+(canvasW*0.7));
          } else {
            p0=ImVec2(0.0,(canvasH-(canvasW*0.7))*0.5);
            p1=ImVec2(canvasW,canvasW*0.7+(canvasH-(canvasW*0.7))*0.5);
          }
        } else {
          p0=ImVec2((canvasW-(canvasH/0.7))*0.5,0.0);
          p1=ImVec2((canvasH/0.7)+(canvasW-(canvasH/0.7))*0.5,canvasH);
        }

        dl->AddRectFilled(ImVec2(0,0),ImVec2(canvasW,canvasH),0xff000000);

        dl->AddImage(rend->getTextureID(cvTex),p0,p1,ImVec2(0,0),ImVec2(rend->getTextureU(cvTex),rend->getTextureV(cvTex)));

        if (mobileUI) {
          ImVec2 chevron[3];

          // up
          chevron[0]=ImLerp(dpadUpStart,dpadUpEnd,ImVec2(0.4,0.65));
          chevron[1]=ImLerp(dpadUpStart,dpadUpEnd,ImVec2(0.5,0.35));
          chevron[2]=ImLerp(dpadUpStart,dpadUpEnd,ImVec2(0.6,0.65));
          dl->AddPolyline(chevron,3,0xffffffff,0,4.0f*dpiScale);

          // left
          chevron[0]=ImLerp(dpadUpStart,dpadLeftEnd,ImVec2(0.65,0.4));
          chevron[1]=ImLerp(dpadUpStart,dpadLeftEnd,ImVec2(0.35,0.5));
          chevron[2]=ImLerp(dpadUpStart,dpadLeftEnd,ImVec2(0.65,0.6));
          dl->AddPolyline(chevron,3,0xffffffff,0,4.0f*dpiScale);

          // down
          chevron[0]=ImLerp(dpadDownStart,dpadDownEnd,ImVec2(0.4,0.35));
          chevron[1]=ImLerp(dpadDownStart,dpadDownEnd,ImVec2(0.5,0.65));
          chevron[2]=ImLerp(dpadDownStart,dpadDownEnd,ImVec2(0.6,0.35));
          dl->AddPolyline(chevron,3,0xffffffff,0,4.0f*dpiScale);

          // right
          chevron[0]=ImLerp(dpadRightStart,dpadDownEnd,ImVec2(0.35,0.4));
          chevron[1]=ImLerp(dpadRightStart,dpadDownEnd,ImVec2(0.65,0.5));
          chevron[2]=ImLerp(dpadRightStart,dpadDownEnd,ImVec2(0.35,0.6));
          dl->AddPolyline(chevron,3,0xffffffff,0,4.0f*dpiScale);

          // A/B
          dl->AddRectFilled(buttonBStart,buttonBEnd,(touchControls&1)?0x4040ffff:0x2040ffff,0,0);
          dl->AddRectFilled(buttonAStart,buttonAEnd,(touchControls&2)?0x4040ffff:0x2040ffff,0,0);
          dl->AddRect(buttonBStart,buttonBEnd,0xff00ffff,0,0,dpiScale);
          dl->AddRect(buttonAStart,buttonAEnd,0xff00ffff,0,0,dpiScale);
          dl->AddText(headFont,settings.headFontSize*dpiScale,ImLerp(buttonBStart,buttonBEnd,ImVec2(0.5,0.5))-(headFont->CalcTextSizeA(settings.headFontSize*dpiScale,FLT_MAX,0,"B")*0.5f),0xff00ffff,"B");
          dl->AddText(headFont,settings.headFontSize*dpiScale,ImLerp(buttonAStart,buttonAEnd,ImVec2(0.5,0.5))-(headFont->CalcTextSizeA(settings.headFontSize*dpiScale,FLT_MAX,0,"A")*0.5f),0xff00ffff,"A");

          // quit
          dl->AddRect(buttonQuitStart,buttonQuitEnd,0xffffffff,0,0,dpiScale);
        }
      }
    }
    ImGui::End();

    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
      if (cv!=NULL) {
        cv->unload();
        delete cv;
        cv=NULL;
      }
      cvOpen=false;
    }
  }
}

// CV
// 320x224

// renderer details:
// two 80x56 tile planes (40x28 is visible)
// 256 colors
// 4096 tiles (image is 256-color, 256x1024)
// each tile is 8x8

static const unsigned char cvPalette[1024]={
  0x00, 0x00, 0x00, 0xff, 0xb3, 0x00, 0x00, 0xff, 0x00, 0xa7, 0x00, 0xff,
  0xe5, 0x93, 0x00, 0xff, 0x00, 0x00, 0xa6, 0xff, 0x77, 0x00, 0x97, 0xff,
  0x12, 0x81, 0xb1, 0xff, 0xbe, 0xbe, 0xbe, 0xff, 0x51, 0x51, 0x51, 0xff,
  0xfc, 0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0xff, 0xff, 0x1d, 0xff,
  0x00, 0x00, 0xfa, 0xff, 0xff, 0x00, 0xff, 0xff, 0x1f, 0xfa, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x01, 0x67, 0xff,
  0x04, 0x00, 0x89, 0xff, 0x00, 0x03, 0xab, 0xff, 0x00, 0x00, 0xdc, 0xff,
  0x04, 0x00, 0xfe, 0xff, 0x00, 0x63, 0x00, 0xff, 0x00, 0x5f, 0x5f, 0xff,
  0x01, 0x62, 0x85, 0xff, 0x05, 0x5d, 0xb6, 0xff, 0x00, 0x61, 0xd9, 0xff,
  0x02, 0x5f, 0xfb, 0xff, 0x00, 0x8a, 0x06, 0xff, 0x00, 0x8a, 0x59, 0xff,
  0x00, 0x85, 0x8a, 0xff, 0x06, 0x87, 0xb1, 0xff, 0x00, 0x8b, 0xd3, 0xff,
  0x02, 0x87, 0xff, 0xff, 0x00, 0xb0, 0x00, 0xff, 0x00, 0xb1, 0x62, 0xff,
  0x00, 0xaf, 0x84, 0xff, 0x02, 0xad, 0xa7, 0xff, 0x00, 0xaf, 0xd8, 0xff,
  0x02, 0xb1, 0xfe, 0xff, 0x00, 0xd8, 0x05, 0xff, 0x00, 0xdb, 0x5c, 0xff,
  0x00, 0xd7, 0x8e, 0xff, 0x03, 0xd5, 0xb0, 0xff, 0x00, 0xd9, 0xd2, 0xff,
  0x00, 0xd4, 0xff, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x66, 0xff,
  0x00, 0xff, 0x88, 0xff, 0x03, 0xff, 0xaa, 0xff, 0x00, 0xff, 0xdb, 0xff,
  0x00, 0xff, 0xfd, 0xff, 0x64, 0x00, 0x00, 0xff, 0x64, 0x00, 0x65, 0xff,
  0x5c, 0x03, 0x87, 0xff, 0x60, 0x01, 0xa9, 0xff, 0x64, 0x00, 0xda, 0xff,
  0x5c, 0x00, 0xfc, 0xff, 0x61, 0x60, 0x00, 0xff, 0x5d, 0x5d, 0x5d, 0xff,
  0x65, 0x5f, 0x83, 0xff, 0x5d, 0x61, 0xb5, 0xff, 0x61, 0x5f, 0xd7, 0xff,
  0x59, 0x63, 0xf9, 0xff, 0x61, 0x88, 0x04, 0xff, 0x5e, 0x87, 0x57, 0xff,
  0x62, 0x83, 0x88, 0xff, 0x5e, 0x8b, 0xaf, 0xff, 0x62, 0x86, 0xe0, 0xff,
  0x5a, 0x8a, 0xff, 0xff, 0x5e, 0xae, 0x00, 0xff, 0x5e, 0xaf, 0x60, 0xff,
  0x62, 0xad, 0x83, 0xff, 0x5a, 0xae, 0xb4, 0xff, 0x5e, 0xac, 0xd6, 0xff,
  0x5a, 0xb5, 0xfc, 0xff, 0x5e, 0xd5, 0x03, 0xff, 0x5e, 0xd9, 0x5b, 0xff,
  0x62, 0xd4, 0x8c, 0xff, 0x5b, 0xd9, 0xae, 0xff, 0x5f, 0xd7, 0xd0, 0xff,
  0x63, 0xd2, 0xff, 0xff, 0x5f, 0xff, 0x00, 0xff, 0x5f, 0xff, 0x64, 0xff,
  0x63, 0xff, 0x86, 0xff, 0x5b, 0xff, 0xa8, 0xff, 0x5f, 0xfe, 0xd9, 0xff,
  0x63, 0xfc, 0xfc, 0xff, 0x88, 0x01, 0x00, 0xff, 0x88, 0x01, 0x62, 0xff,
  0x8c, 0x00, 0x84, 0xff, 0x84, 0x01, 0xb5, 0xff, 0x88, 0x00, 0xd7, 0xff,
  0x8c, 0x00, 0xfa, 0xff, 0x85, 0x60, 0x07, 0xff, 0x81, 0x60, 0x5a, 0xff,
  0x89, 0x5f, 0x8f, 0xff, 0x8d, 0x5e, 0xb2, 0xff, 0x85, 0x62, 0xd4, 0xff,
  0x89, 0x5d, 0xff, 0xff, 0x85, 0x8b, 0x01, 0xff, 0x82, 0x87, 0x63, 0xff,
  0x86, 0x86, 0x86, 0xff, 0x8a, 0x84, 0xa8, 0xff, 0x86, 0x89, 0xdd, 0xff,
  0x8a, 0x87, 0xff, 0xff, 0x82, 0xb1, 0x00, 0xff, 0x82, 0xb2, 0x5e, 0xff,
  0x86, 0xb0, 0x80, 0xff, 0x8a, 0xab, 0xb1, 0xff, 0x82, 0xaf, 0xd3, 0xff,
  0x8a, 0xaf, 0xff, 0xff, 0x82, 0xd8, 0x00, 0xff, 0x82, 0xdc, 0x58, 0xff,
  0x86, 0xd7, 0x89, 0xff, 0x8b, 0xd5, 0xab, 0xff, 0x83, 0xd7, 0xdc, 0xff,
  0x87, 0xd5, 0xff, 0xff, 0x83, 0xff, 0x00, 0xff, 0x83, 0xff, 0x61, 0xff,
  0x87, 0xff, 0x83, 0xff, 0x8b, 0xfd, 0xb4, 0xff, 0x83, 0xff, 0xd7, 0xff,
  0x87, 0xff, 0xf9, 0xff, 0xac, 0x01, 0x07, 0xff, 0xac, 0x04, 0x5f, 0xff,
  0xb0, 0x02, 0x81, 0xff, 0xb4, 0x00, 0xb2, 0xff, 0xac, 0x02, 0xd4, 0xff,
  0xb0, 0x00, 0xff, 0xff, 0xb5, 0x5d, 0x04, 0xff, 0xb1, 0x5d, 0x57, 0xff,
  0xad, 0x62, 0x8d, 0xff, 0xb1, 0x60, 0xaf, 0xff, 0xb5, 0x5c, 0xe0, 0xff,
  0xad, 0x60, 0xff, 0xff, 0xb5, 0x88, 0x00, 0xff, 0xb1, 0x84, 0x60, 0xff,
  0xaa, 0x88, 0x83, 0xff, 0xae, 0x84, 0xb4, 0xff, 0xb6, 0x86, 0xda, 0xff,
  0xae, 0x8a, 0xfc, 0xff, 0xb2, 0xab, 0x03, 0xff, 0xb2, 0xaf, 0x5b, 0xff,
  0xaa, 0xb0, 0x8c, 0xff, 0xae, 0xae, 0xae, 0xff, 0xb2, 0xac, 0xd0, 0xff,
  0xaa, 0xae, 0xff, 0xff, 0xb2, 0xd5, 0x00, 0xff, 0xb2, 0xd6, 0x64, 0xff,
  0xaa, 0xda, 0x86, 0xff, 0xae, 0xd8, 0xa8, 0xff, 0xb3, 0xd4, 0xd9, 0xff,
  0xab, 0xd8, 0xfc, 0xff, 0xb3, 0xff, 0x00, 0xff, 0xb3, 0xff, 0x5e, 0xff,
  0xab, 0xff, 0x80, 0xff, 0xaf, 0xff, 0xb1, 0xff, 0xb3, 0xfe, 0xd4, 0xff,
  0xab, 0xff, 0xff, 0xff, 0xdc, 0x00, 0x05, 0xff, 0xdc, 0x01, 0x5c, 0xff,
  0xd4, 0x02, 0x8d, 0xff, 0xd8, 0x01, 0xaf, 0xff, 0xdc, 0x00, 0xd2, 0xff,
  0xd4, 0x00, 0xff, 0xff, 0xd9, 0x60, 0x01, 0xff, 0xd5, 0x5d, 0x63, 0xff,
  0xd9, 0x5b, 0x86, 0xff, 0xd5, 0x63, 0xac, 0xff, 0xd9, 0x5f, 0xdd, 0xff,
  0xdd, 0x5d, 0xff, 0xff, 0xd5, 0x86, 0x00, 0xff, 0xd5, 0x87, 0x5e, 0xff,
  0xd9, 0x85, 0x80, 0xff, 0xd2, 0x87, 0xb1, 0xff, 0xda, 0x89, 0xd7, 0xff,
  0xde, 0x84, 0xff, 0xff, 0xd6, 0xae, 0x00, 0xff, 0xd6, 0xb1, 0x58, 0xff,
  0xda, 0xad, 0x89, 0xff, 0xd2, 0xb1, 0xab, 0xff, 0xd6, 0xac, 0xdc, 0xff,
  0xda, 0xaa, 0xff, 0xff, 0xd6, 0xd8, 0x00, 0xff, 0xd6, 0xd9, 0x61, 0xff,
  0xda, 0xd7, 0x83, 0xff, 0xd2, 0xd8, 0xb4, 0xff, 0xd7, 0xd7, 0xd7, 0xff,
  0xdb, 0xd5, 0xf9, 0xff, 0xd7, 0xff, 0x04, 0xff, 0xd7, 0xff, 0x5b, 0xff,
  0xdb, 0xfe, 0x8c, 0xff, 0xd3, 0xff, 0xaf, 0xff, 0xd7, 0xff, 0xd1, 0xff,
  0xdb, 0xfc, 0xff, 0xff, 0xff, 0x00, 0x02, 0xff, 0xff, 0x01, 0x68, 0xff,
  0xff, 0x00, 0x8a, 0xff, 0xfc, 0x04, 0xac, 0xff, 0xff, 0x00, 0xde, 0xff,
  0xff, 0x00, 0xff, 0xff, 0xfd, 0x63, 0x00, 0xff, 0xf9, 0x60, 0x60, 0xff,
  0xfd, 0x5e, 0x83, 0xff, 0xff, 0x5d, 0xb8, 0xff, 0xfd, 0x62, 0xda, 0xff,
  0xff, 0x60, 0xfc, 0xff, 0xf9, 0x86, 0x03, 0xff, 0xf9, 0x8a, 0x5b, 0xff,
  0xfd, 0x85, 0x8c, 0xff, 0xff, 0x84, 0xae, 0xff, 0xfa, 0x88, 0xd0, 0xff,
  0xff, 0x87, 0xff, 0xff, 0xfa, 0xb1, 0x00, 0xff, 0xff, 0xab, 0x64, 0xff,
  0xfe, 0xb0, 0x86, 0xff, 0xff, 0xae, 0xa8, 0xff, 0xfa, 0xaf, 0xd9, 0xff,
  0xfe, 0xad, 0xfc, 0xff, 0xfa, 0xdb, 0x00, 0xff, 0xff, 0xd6, 0x5e, 0xff,
  0xfe, 0xda, 0x80, 0xff, 0xff, 0xd5, 0xb1, 0xff, 0xfb, 0xd9, 0xd4, 0xff,
  0xff, 0xd5, 0xff, 0xff, 0xfb, 0xff, 0x01, 0xff, 0xff, 0xf9, 0x63, 0xff,
  0xff, 0xff, 0x89, 0xff, 0xff, 0xff, 0xac, 0xff, 0xfb, 0xff, 0xdd, 0xff,
  0xff, 0xff, 0xff, 0xff, 0x04, 0x04, 0x04, 0xff, 0x0c, 0x0c, 0x0c, 0xff,
  0x18, 0x18, 0x18, 0xff, 0x20, 0x20, 0x20, 0xff, 0x2d, 0x2d, 0x2d, 0xff,
  0x35, 0x35, 0x35, 0xff, 0x41, 0x41, 0x41, 0xff, 0x49, 0x49, 0x49, 0xff,
  0x55, 0x55, 0x55, 0xff, 0x61, 0x61, 0x61, 0xff, 0x69, 0x69, 0x69, 0xff,
  0x71, 0x71, 0x71, 0xff, 0x7d, 0x7d, 0x7d, 0xff, 0x8a, 0x8a, 0x8a, 0xff,
  0x92, 0x92, 0x92, 0xff, 0x9a, 0x9a, 0x9a, 0xff, 0xa6, 0xa6, 0xa6, 0xff,
  0xb2, 0xb2, 0xb2, 0xff, 0xba, 0xba, 0xba, 0xff, 0xc2, 0xc2, 0xc2, 0xff,
  0xce, 0xce, 0xce, 0xff, 0xdb, 0xdb, 0xdb, 0xff, 0xe3, 0xe3, 0xe3, 0xff,
  0xeb, 0xeb, 0xeb, 0xff
};

#include "tileData.h"

#define SE_SHOT1 0, 0, 48
#define SE_EXPL1 1, 1, 67
#define SE_PICKUP1 2, 2, 67
#define SE_PICKUP2 3, 2, 69
#define SE_DEATH_C1 4, 0, 48
#define SE_DEATH_C2 5, 1, 48
#define SE_SHOT2 6, 0, 48
#define SE_TYPEWRITER 7, 0, 48
#define SE_TRANSITION1 8, 2, 43
#define SE_TRANSITION2 9, 2, 48
#define SE_INIT 10, 2, 48
#define SE_RESPAWN 11, 2, 48
#define SE_TANKMOVE 12, 3, 48
#define SE_VORTEXMOVE 13, 3, 55
#define SE_VORTEXSHOOT 14, 3, 48
#define SE_RESIST 15, 1, 48
#define SE_PLANE1 16, 1, 47
#define SE_PLANE2 17, 2, 47
#define SE_EXPL2 18, 1, 60
#define SE_PICKUP3 19, 3, 67
#define SE_TIMEUP 20, 2, 81

template<typename T> T* FurnaceCV::createObject(short x, short y) {
  T* ret=new T(this);
  ret->x=x;
  ret->y=y;
  sprite.push_back(ret);
  return ret;
}

template<typename T> T* FurnaceCV::createObjectNoPos() {
  T* ret=new T(this);
  sprite.push_back(ret);
  return ret;
}

void FurnaceCV::soundEffect(int ins, int chan, int note) {
  e->noteOn(chan+fxChanBase,ins+fxInsBase,note);
  /*
  e->dispatchCmd(DivCommand(DIV_CMD_INSTRUMENT,chan,ins,1));
  e->dispatchCmd(DivCommand(DIV_CMD_NOTE_ON,chan,note));
  */
}

void FurnaceCV::stopSoundEffect(int ins, int chan, int note) {
  e->noteOff(chan+fxChanBase);
  //e->dispatchCmd(DivCommand(DIV_CMD_NOTE_OFF,chan));
}

static const int floorBases[4]={
  0x40, 0x48, 0x60, 0x68
};

void FurnaceCV::buildStage(int which) {
  if (curStage!=NULL) {
    delete[] curStage;
    curStage=NULL;
  }

  if (which>19 || which==4 || which==7 || which==9 || which==11 || which==13 || which==16 || which==17) {
    stageWidth=80;
    stageHeight=56;
  } else {
    stageWidth=40;
    stageHeight=28;    
  }

  stageWidthPx=stageWidth<<3;
  stageHeightPx=stageHeight<<3;

  curStage=new unsigned short[stageWidth*stageHeight];
  
  int floorBase=floorBases[rand()&3];

  for (int i=0; i<stageHeight; i++) {
    for (int j=0; j<stageWidth; j++) {
      curStage[i*stageWidth+j]=floorBase+(rand()&7);
    }
  }

  // render
  for (int i=0; i<stageHeight; i++) {
    for (int j=0; j<stageWidth; j++) {
      tile0[i][j]=curStage[i*stageWidth+j];
    }
  }

  // prepare enemies
  bool busy[28][40];
  memset(busy,0,28*40*sizeof(bool));

  // special stages
  if ((which%10)==9) {
    // vortex
    for (int i=0; i<20+(which>>2); i++) {
      int tries=0;
      while (tries<20) {
        int x=rand()%(stageWidth>>1);
        int y=rand()%(stageHeight>>1);
        int finalX=x<<4;
        int finalY=y<<4;
        if (busy[y][x]) {
          tries++;
          continue;
        }
        createObject<FurnaceCVEnemyVortex>(finalX,finalY);
        createObject<FurnaceCVFurBallMedium>(finalX-4,finalY-4);
        busy[y][x]=true;
        break;
      }
    }
  } else if ((which%10)==19) {
    for (int i=0; i<20+(which>>2); i++) {
      int tries=0;
      while (tries<20) {
        int x=(rand()%(stageWidth>>1))&(~1);
        int y=(rand()%(stageHeight>>1))&(~1);
        int finalX=x<<4;
        int finalY=y<<4;
        if (busy[y][x]) {
          tries++;
          continue;
        }
        FurnaceCVEnemy1* enemy=createObject<FurnaceCVEnemy1>(finalX,finalY);
        createObject<FurnaceCVFurBallLarge>(finalX-4,finalY-4);
        enemy->setType(3);
        busy[y][x]=true;
        busy[y][x+1]=true;
        busy[y+1][x]=true;
        busy[y+1][x+1]=true;
        break;
      }
    }
  } else {
    // large
    if (which>=2) for (int i=0; i<(rand()%3)+which-2; i++) {
      int tries=0;
      while (tries<20) {
        int x=(rand()%(stageWidth>>1))&(~1);
        int y=(rand()%(stageHeight>>1))&(~1);
        int finalX=x<<4;
        int finalY=y<<4;
        if (busy[y][x]) {
          tries++;
          continue;
        }
        FurnaceCVEnemy1* enemy=createObject<FurnaceCVEnemy1>(finalX,finalY);
        createObject<FurnaceCVFurBallLarge>(finalX-4,finalY-4);
        enemy->setType(2);
        if (which>7) {
          enemy->setType((rand()%MAX(3,17-which))==0?3:2);
        }
        busy[y][x]=true;
        busy[y][x+1]=true;
        busy[y+1][x]=true;
        busy[y+1][x+1]=true;
        break;
      }
    }

    // vortex
    if (which>=4) for (int i=0; i<(rand()%(1+which))+(which>>1); i++) {
      int tries=0;
      while (tries<20) {
        int x=rand()%(stageWidth>>1);
        int y=rand()%(stageHeight>>1);
        int finalX=x<<4;
        int finalY=y<<4;
        if (busy[y][x]) {
          tries++;
          continue;
        }
        createObject<FurnaceCVEnemyVortex>(finalX,finalY);
        createObject<FurnaceCVFurBallMedium>(finalX-4,finalY-4);
        busy[y][x]=true;
        break;
      }
    }

    // small
    for (int i=0; i<7+(rand()%4)+((which&3)<<2)+(which>>1); i++) {
      int tries=0;
      while (tries<20) {
        int x=rand()%(stageWidth>>1);
        int y=rand()%(stageHeight>>1);
        int finalX=x<<4;
        int finalY=y<<4;
        if (busy[y][x]) {
          tries++;
          continue;
        }
        FurnaceCVEnemy1* enemy=createObject<FurnaceCVEnemy1>(finalX,finalY);
        createObject<FurnaceCVFurBallMedium>(finalX-4,finalY-4);
        if (which>0) {
          if (which>=20) {
            enemy->setType(1);
          } else {
            enemy->setType((rand()%MAX(2,8-which))==0?1:0);
          }
        }
        busy[y][x]=true;
        break;
      }
    }

    // mines
    for (int i=0; i<7+(rand()%18); i++) {
      int tries=0;
      while (tries<20) {
        int x=rand()%(stageWidth>>1);
        int y=rand()%(stageHeight>>1);
        int finalX=x<<4;
        int finalY=y<<4;
        if (busy[y][x]) {
          tries++;
          continue;
        }
        createObject<FurnaceCVMine>(finalX,finalY);
        busy[y][x]=true;
        break;
      }
    }
  }

  // setup stuff
  planeTime=180+(rand()%400);
}

#define CV_FONTBASE_8x8 0x250
#define CV_FONTBASE_8x8_RED 0x2d0
#define CV_FONTBASE_8x16 0x0

void FurnaceCV::putText(int fontBase, bool fontHeight, String text, int x, int y) {
  int initX=x;
  for (const char* i=text.c_str(); *i; i++) {
    if (*i=='\n') {
      x=initX;
      y+=fontHeight?2:1;
      continue;
    }
    if (y>=28) break;
    tile1[y][x]=fontBase+((*i)&15)+(((*i)&(~15))<<(fontHeight?2:1));
    if (fontHeight) {
      tile1[y+1][x]=fontBase+((*i)&15)+32+(((*i)&(~15))<<(fontHeight?2:1));
    }
    x++;
    if (x>=40) {
      x=initX;
      y+=fontHeight?2:1;
    }
  }
}

void FurnaceCV::startTyping(const char* text, int x, int y) {
  typeAddr=text;
  typeX=x;
  typeY=y;
  typeXStart=x;
  typeYStart=y;
  typeDelay=1;
  soundEffect(SE_TYPEWRITER);
}

void FurnaceCV::typeTick() {
  if (typeAddr==NULL) return;

  unsigned char typeChar=(unsigned char)(*typeAddr);

  if (typeChar<0x20 || typeChar>=0x80) {
    switch (typeChar) {
      case '\n':
        typeX=typeXStart;
        typeY+=2;
        if (typeY>=typeYEnd) typeY=typeYEnd;
        break;
    }
  } else {
    tile1[typeY][typeX]=(typeChar&15)|((typeChar&(~15))<<2);
    tile1[typeY+1][typeX]=32|((typeChar&15)|((typeChar&(~15))<<2));

    if (++typeX>typeXEnd) {
      typeX=typeXStart;
      typeY+=2;

      if (typeY>=typeYEnd) typeY=typeYEnd;
    }
  }

  if (*(++typeAddr)==0) {
    typeAddr=NULL;
    stopSoundEffect(SE_TYPEWRITER);
  }
}

void FurnaceCV::rasterH(int scanline) {

}

void FurnaceCV::render(unsigned char joyIn) {
  // input
  joyInputPrev=joyInput;
  joyInput=joyIn;
  joyInput|=(
    (ImGui::IsKeyDown(ImGuiKey_X)?1:0)|
    (ImGui::IsKeyDown(ImGuiKey_Z)?2:0)|
    (ImGui::IsKeyDown(ImGuiKey_RightShift)?4:0)|
    (ImGui::IsKeyDown(ImGuiKey_Enter)?8:0)|
    (ImGui::IsKeyDown(ImGuiKey_UpArrow)?16:0)|
    (ImGui::IsKeyDown(ImGuiKey_DownArrow)?32:0)|
    (ImGui::IsKeyDown(ImGuiKey_LeftArrow)?64:0)|
    (ImGui::IsKeyDown(ImGuiKey_RightArrow)?128:0)
  );
  joyPressed=(joyInputPrev^0xff)&joyInput;
  joyReleased=(joyInput^0xff)&joyInputPrev;

  // logic
  typeTick();

  int enemyCount=0;

  for (size_t i=0; i<sprite.size(); i++) {
    for (size_t j=i+1; j<sprite.size(); j++) {
      const short s_x0=sprite[i]->x+sprite[i]->collX0;
      const short s_x1=sprite[i]->x+sprite[i]->collX1;
      const short s_y0=sprite[i]->y+sprite[i]->collY0;
      const short s_y1=sprite[i]->y+sprite[i]->collY1;
      const short d_x0=sprite[j]->x+sprite[j]->collX0;
      const short d_x1=sprite[j]->x+sprite[j]->collX1;
      const short d_y0=sprite[j]->y+sprite[j]->collY0;
      const short d_y1=sprite[j]->y+sprite[j]->collY1;
      if (d_y0<s_y1 && d_y1>s_y0 && d_x0<s_x1 && d_x1>s_x0) {
        sprite[i]->collision(sprite[j]);
        sprite[j]->collision(sprite[i]);
      }
    }
    sprite[i]->tick();
    if (sprite[i]->dead) {
      delete sprite[i];
      sprite.erase(sprite.begin()+i);
      i--;
    }
  }

  if (inGame) {
    // planes
    if (--planeTime<=0) {
      planeTime=MAX(10,60-(stage*2))+(rand()%MAX(50,320-stage*4));
      if (stage>=5 && stage!=9 && (stage&1 || stage>=10)) {
        createObjectNoPos<FurnaceCVEnemyPlane>();
      }
    }

    // initialization
    if (ticksToInit>0) {
      if (--ticksToInit<1) {
        e->changeSongP(0);
        e->setOrder(0);
        e->play();
        buildStage(stage);
        createObject<FurnaceCVPlayer>(160,112);
        createObject<FurnaceCVFurBallLarge>(160-4,112-4);
        soundEffect(SE_INIT);
      }
    } else {
      bool hasEnemy=false;
      for (FurnaceCVObject* i: sprite) {
        if (i->type==CV_ENEMY) {
          hasEnemy=true;
          enemyCount++;
        }
      }
      if (!hasEnemy) {
        inTransition=true;
        inGame=false;
        transWait=100;
        soundEffect(SE_TRANSITION1);
        stopSoundEffect(0,0,0);
        stopSoundEffect(0,3,0);
        respawnTime=0;
        for (FurnaceCVObject* i: sprite) {
          if (i->type==CV_EXTRA_LIFE) lifeBank++;
          i->dead=true;
        }
        memset(tile0,0,80*56*sizeof(short));
        memset(tile1,0,80*56*sizeof(short));
      }
    }
    
    if (respawnTime>0) {
      if (--respawnTime<1) {
        if (lives>0) {
          createObject<FurnaceCVPlayer>(lastPlayerX,lastPlayerY);
          createObject<FurnaceCVFurBallLarge>(lastPlayerX-4,lastPlayerY-4);
          soundEffect(SE_RESPAWN);
          lives--;
        } else {
          if (newHiScore) {
            inGame=false;
            inTransition=false;
            for (FurnaceCVObject* i: sprite) {
              i->dead=true;
            }
            memset(tile0,0,80*56*sizeof(short));
            memset(tile1,0,80*56*sizeof(short));
            startTyping(_(cvText[3]),2,3);
            e->setConf("cvHiScore",hiScore);
            e->saveConf();
            curText=4;
            textWait=90;
          } else {
            startTyping(_(cvText[2]),15,13);
          }
          gameOver=true;
        }
      }
    }

    if (gameOver && lifeBank>0 && joyInput==3) {
      lives+=lifeBank;
      respawnTime=1;
      lifeBank=0;
      gameOver=false;
    }

    // draw score
    putText(CV_FONTBASE_8x8_RED,false,"1UP",0,0);
    putText(CV_FONTBASE_8x8,false,fmt::sprintf("%8d",score),3,0);

    putText(CV_FONTBASE_8x8_RED,false,"HI",15,0);
    putText(CV_FONTBASE_8x8,false,fmt::sprintf("%8d",hiScore),17,0);
    tile1[0][31]=0x27e;
    putText(CV_FONTBASE_8x8,false,fmt::sprintf("*%2d",enemyCount),32,0);
    tile1[0][36]=0x27f;
    putText(CV_FONTBASE_8x8,false,fmt::sprintf("*%2d",lives),37,0);

    // arrow overlay
    bool arrowLeft=false;
    bool arrowRight=false;
    bool arrowUp=false;
    bool arrowDown=false;
    if (stageWidthPx>320 || stageHeightPx>224) {
      for (FurnaceCVObject* i: sprite) {
        if (i->type!=CV_ENEMY) continue;
        if ((i->x-viewX+(i->spriteWidth<<3))<0) {
          arrowLeft=true;
        }
        if ((i->x-viewX)>=320) {
          arrowRight=true;
        }
        if ((i->y-viewY+(i->spriteHeight<<3))<0) {
          arrowUp=true;
        }
        if ((i->y-viewY)>=224) {
          arrowDown=true;
        }
      }
    }
    if (arrowLeft && !(tick&8)) {
      tile1[13][1]=0x580;
      tile1[13][2]=0x581;
      tile1[14][1]=0x5a0;
      tile1[14][2]=0x5a1;
    } else {
      tile1[13][1]=0;
      tile1[13][2]=0;
      tile1[14][1]=0;
      tile1[14][2]=0;
    }
    if (arrowRight && !(tick&8)) {
      tile1[13][37]=0x584;
      tile1[13][38]=0x585;
      tile1[14][37]=0x5a4;
      tile1[14][38]=0x5a5;
    } else {
      tile1[13][37]=0;
      tile1[13][38]=0;
      tile1[14][37]=0;
      tile1[14][38]=0;
    }
    if (arrowUp && !(tick&8)) {
      tile1[1][19]=0x582;
      tile1[1][20]=0x583;
      tile1[2][19]=0x5a2;
      tile1[2][20]=0x5a3;
    } else {
      tile1[1][19]=0;
      tile1[1][20]=0;
      tile1[2][19]=0;
      tile1[2][20]=0;
    }
    if (arrowDown && !(tick&8)) {
      tile1[25][19]=0x586;
      tile1[25][20]=0x587;
      tile1[26][19]=0x5a6;
      tile1[26][20]=0x5a7;
    } else {
      tile1[25][19]=0;
      tile1[25][20]=0;
      tile1[26][19]=0;
      tile1[26][20]=0;
    }

    // special stat
    if (specialType>0 && (tick&16)) {
      tile1[24][2]=0x4dc+(((specialType-1)&1)<<1)+(((specialType-1)>>1)<<6);
      tile1[24][3]=0x4dd+(((specialType-1)&1)<<1)+(((specialType-1)>>1)<<6);
      tile1[25][2]=0x4fc+(((specialType-1)&1)<<1)+(((specialType-1)>>1)<<6);
      tile1[25][3]=0x4fd+(((specialType-1)&1)<<1)+(((specialType-1)>>1)<<6);
    } else {
      tile1[24][2]=0;
      tile1[24][3]=0;
      tile1[25][2]=0;
      tile1[25][3]=0;
    }

    // S mod stat
    if (speedTicks>0) {
      speedTicks--;
      if (speedTicks==120) soundEffect(SE_TIMEUP);
      if ((speedTicks<120 && speedTicks&2) || (speedTicks>=120 && speedTicks&16)) {
        tile1[24][36]=0x41e;
        tile1[24][37]=0x41f;
        tile1[25][36]=0x43e;
        tile1[25][37]=0x43f;
      } else {
        tile1[24][36]=0;
        tile1[24][37]=0;
        tile1[25][36]=0;
        tile1[25][37]=0;
      }
      if (speedTicks==0) {
        e->setSongRate(origSongRate);
      }
    } else {
      tile1[24][36]=0;
      tile1[24][37]=0;
      tile1[25][36]=0;
      tile1[25][37]=0;
    }
  } else {
    if (inTransition) {
      if (--transWait<0) {
        stage++;
        logV("stage %d",stage+1);
        soundEffect(SE_TRANSITION2);
        buildStage(stage);
        createObject<FurnaceCVPlayer>(lastPlayerX,lastPlayerY);
        createObject<FurnaceCVFurBallLarge>(lastPlayerX-4,lastPlayerY-4);
        inGame=true;
      }
      if (transWait==40) {
        putText(CV_FONTBASE_8x16,true,fmt::sprintf(_("STAGE %d"),stage+2),16,13);
      } else if (transWait>40) {
        for (int i=1; i<28; i++) {
          for (int j=0; j<40; j++) {
            int result=0x270+(j>>3)+(i>>3)+(transWait>>1)-32;
            tile1[i][j]=(result>=0x270 && result<=0x27b)?result:0;
          }
        }
      } else if (transWait==1) {
        memset(tile1,0,80*56*sizeof(short));
      }
    } else {
      if (curText==1) {
        if (typeAddr==NULL) {
          textWait=1;
          if (joyPressed&32) {
            playSongs=false;
          }
          if (joyPressed) {
            pleaseInitSongs=true;
            textWait=0;
          }
        }
      } else if (joyPressed&15 && curText<3) {
        textWait=1;
        typeAddr=NULL;
      }
      if (typeAddr==NULL) {
        if (--textWait<0) {
          if (curText==4) {
            textWait=90;
          } else if (curText==2) {
            memset(tile1,0,80*56*sizeof(short));
            inGame=true;
          } else {
            memset(tile1,0,80*56*sizeof(short));
            startTyping(_(cvText[curText]),2,3);
            curText++;
            textWait=90;
          }
        }
      }
    }
  }

  tick++;

  // render
  if (surface==NULL) return;

  SDL_LockSurface(surface);

  // render here
  unsigned int* p=(unsigned int*)surface->pixels;
  unsigned int* pCopy=p;
  unsigned int* paletteInt=(unsigned int*)cvPalette;
  unsigned char* pb=prioBuf;
  unsigned short y0=scrollY[0];
  unsigned short y1=scrollY[1];
  for (int i=0; i<224; i++) {
    rasterH(i);

    unsigned short x0=scrollX[0]%stageWidthPx;
    unsigned short x1=scrollX[1]%stageWidthPx;

    y0%=stageHeightPx;
    y1%=stageHeightPx;

    for (int j=0; j<320; j++) {
      unsigned short t0=tile0[y0>>3][x0>>3]&0xfff;
      unsigned short t1=tile1[y1>>3][x1>>3]&0xfff;

      unsigned char td0=tileData[((t0&31)<<3)|(x0&7)|((t0&(~31))<<6)|((y0&7)<<8)];
      unsigned char td1=tileData[((t1&31)<<3)|(x1&7)|((t1&(~31))<<6)|((y1&7)<<8)];

      if (td1) {
        *p++=paletteInt[td1];
        *pb++=2;
      } else if (td0) {
        *p++=paletteInt[td0];
        *pb++=1;
      } else {
        *p++=paletteInt[bgColor];
        *pb++=0;
      }

      if (++x0>=stageWidthPx) x0=0;
      if (++x1>=stageWidthPx) x1=0;
    }

    y0++;
    y1++;
  }

  // draw sprites
  p=pCopy;
  for (FurnaceCVObject* i: sprite) {
    int jk=0;
    for (int j=0; j<i->spriteHeight; j++) {
      for (int k=0; k<i->spriteWidth; k++) {
        int dx=i->x-viewX+(k<<3);
        int dy=i->y-viewY+(j<<3);
        int ptr=dx+dy*320;
        int ptrC=ptr;
        unsigned short spriteTile=i->spriteDef[jk];
        for (int l=0; l<8; l++) {
          dx=i->x-viewX+(k<<3);
          if (dy>=0 && dy<224) for (int m=0; m<8; m++) {
            if (dx>=0 && dx<320) {
              unsigned char data=tileData[((spriteTile&31)<<3)|m|((spriteTile&(~31))<<6)|(l<<8)];
              if (data) {
                if (i->prio>=prioBuf[ptr]) {
                  p[ptr]=paletteInt[data];
                }
              }
            }
            dx++;
            ptr++;
          }
          dy++;
          ptrC+=320;
          ptr=ptrC;
        }
        jk++;
      }
    }
  }

  SDL_UnlockSurface(surface);
}

void FurnaceCV::addScore(int amount) {
  score+=amount;
  if (score>=99999999) score=99999999;
  if (score>hiScore) {
    hiScore=score;
    newHiScore=true;
  }
}

void _tileDataRead(void* user, struct GIF_WHDR* data) {
  ((FurnaceCV*)user)->tileDataRead(data);
}

void FurnaceCV::tileDataRead(struct GIF_WHDR* data) {
  logV("got tile: %d %d",data->xdim,data->ydim);
  logV("frd: %d %d fro: %d %d",data->frxd,data->fryd,data->frxo,data->fryo);
  memcpy(tileData,data->bptr,data->xdim*data->ydim);
}


bool FurnaceCV::init(DivEngine* eng) {
  e=eng;

  tileData=new unsigned char[8*8*4096];
  memset(tileData,0,8*8*4096);
  GIF_Load((void*)tileDataC,tileDataC_len,_tileDataRead,NULL,this,0);

  surface=SDL_CreateRGBSurfaceWithFormat(0,320,224,32,SDL_PIXELFORMAT_ARGB8888);
  if (surface==NULL) {
    logE("FurnaceCV: couldn't create surface!");
    return false;
  }

  prioBuf=new unsigned char[320*224];
  memset(prioBuf,0,320*224);

  loadInstruments();

  return true;
}

void FurnaceCV::unload() {
  SDL_FreeSurface(surface);
  delete[] tileData;
  delete[] prioBuf;

  if (fxInsBase>=0) {
    for (int i=e->song.insLen-1; i>=fxInsBase; i--) {
      e->delInstrument(i);
    }
  }
  if (fxChanBase>=0) {
    e->removeSystem(e->song.systemLen-1);
  }
  e->setSongRate(origSongRate);

  if (curStage!=NULL) {
    delete[] curStage;
    curStage=NULL;
  }
}

#define IS_VISIBLE ((x-cv->viewX+(spriteWidth<<3))>=0 && (x-cv->viewX)<320 && (y-cv->viewY+(spriteHeight<<3))>=0 && (y-cv->viewY)<224)
#define IS_IN_AREA ((x+(spriteWidth<<3))>=0 && (x)<cv->stageWidthPx && (y+(spriteHeight<<3))>=0 && (y)<cv->stageHeightPx)
#define HITS_BORDER (x<0 || y<0 || (x+(spriteWidth<<3))>=cv->stageWidthPx || (y+(spriteHeight<<3))>=cv->stageHeightPx)
#define CONFINE_TO_BORDER \
  if (x<0) x=0; \
  if (y<0) y=0; \
  if ((x+(spriteWidth<<3))>=cv->stageWidthPx) x=(cv->stageWidthPx-1)-(spriteWidth<<3); \
  if ((y+(spriteHeight<<3))>=cv->stageHeightPx) y=(cv->stageHeightPx-1)-(spriteHeight<<3);

// FurnaceCVPlayer IMPLEMENTATION

void FurnaceCVPlayer::collision(FurnaceCVObject* other) {
  if (other->type==CV_ENEMY_BULLET ||
      other->type==CV_MINE ||
      other->type==CV_ENEMY) {
    bool mustDie=!invincible;

    if (other->type==CV_ENEMY_BULLET) {
      const int diffX=abs((other->x+4)-(x+12));
      const int diffY=abs((other->y+4)-(y+12));
      if (diffX>4 || diffY>4) mustDie=false;
    }

    if (mustDie) {
      dead=true;
      cv->speedTicks=0;
      cv->e->setSongRate(cv->origSongRate);
      cv->respawnTime=48;
      cv->specialType=0;
      if (cv->weaponStack.empty()) {
        cv->shotType=0;
      } else {
        cv->shotType=cv->weaponStack.front();
        cv->weaponStack.pop_front();
      }
      cv->soundEffect(SE_DEATH_C1);
      cv->soundEffect(SE_DEATH_C2);
      cv->createObject<FurnaceCVExplMedium>(x-8,y);
      cv->createObject<FurnaceCVExplMedium>(x+8,y);
      cv->createObject<FurnaceCVExplMedium>(x,y-8);
      cv->createObject<FurnaceCVExplMedium>(x,y+8);
      cv->createObject<FurnaceCVExplMedium>(x,y);
    } else if (other->type==CV_ENEMY || other->type==CV_MINE) {
      cv->soundEffect(SE_EXPL1);
    }
  }
}

static const int shootDirOffsX[8]={
  16, 16, 8, 0, 0, 0, 8, 16
};

static const int shootDirOffsY[8]={
  8, 0, 0, 0, 8, 16, 16, 16
};

static const int shootDirOrient[8]={
  0x80, 0, 0, 0, 0x180, 0x100, 0x100, 0x100
};

void FurnaceCVPlayer::tick() {
  signed char sdX=0;
  signed char sdY=0;

  int maxSpeed=(cv->speedTicks>0)?128:64;
  int maxSpeedDiagonal=(cv->speedTicks>0)?90:45;

  if (cv->joyInput&16) {
    speedY-=12;
    sdY=-1;
    if (speedY<-maxSpeed) speedY=-maxSpeed;
  } else if (cv->joyInput&32) {
    speedY+=12;
    sdY=1;
    if (speedY>maxSpeed) speedY=maxSpeed;
  } else {
    if (speedY>0) {
      speedY-=12;
      if (speedY<0) speedY=0;
    } else if (speedY<0) {
      speedY+=12;
      if (speedY>0) speedY=0;
    }
  }
  if (cv->joyInput&64) {
    speedX-=12;
    sdX=-1;
    if (speedX<-maxSpeed) speedX=-maxSpeed;
  } else if (cv->joyInput&128) {
    speedX+=12;
    sdX=1;
    if (speedX>maxSpeed) speedX=maxSpeed;
  } else {
    if (speedX>0) {
      speedX-=12;
      if (speedX<0) speedX=0;
    } else if (speedX<0) {
      speedX+=12;
      if (speedX>0) speedX=0;
    }
  }

  // sqrt(2)
  if (speedX && speedY) {
    if (speedX>maxSpeedDiagonal) speedX=maxSpeedDiagonal;
    if (speedX<-maxSpeedDiagonal) speedX=-maxSpeedDiagonal;
    if (speedY>maxSpeedDiagonal) speedY=maxSpeedDiagonal;
    if (speedY<-maxSpeedDiagonal) speedY=-maxSpeedDiagonal;
  }

  subX+=speedX;
  while (subX>=32) {
    x++;
    subX-=32;
  }
  while (subX<0) {
    x--;
    subX+=32;
  }

  subY+=speedY;
  while (subY>=32) {
    y++;
    subY-=32;
  }
  while (subY<0) {
    y--;
    subY+=32;
  }

  if (HITS_BORDER) {
    CONFINE_TO_BORDER;
  }

  cv->lastPlayerX=x;
  cv->lastPlayerY=y;

  if (sdX>0 && sdY>0) shootDir=7;
  if (sdX>0 && sdY<0) shootDir=1;
  if (sdX<0 && sdY>0) shootDir=5;
  if (sdX<0 && sdY<0) shootDir=3;
  if (sdX<0 && sdY==0) shootDir=4;
  if (sdX>0 && sdY==0) shootDir=0;
  if (sdX==0 && sdY<0) shootDir=2;
  if (sdX==0 && sdY>0) shootDir=6;

  if (cv->joyPressed&1) {
    shotTimer=(cv->shotType==2)?5:9;
    if (cv->speedTicks>0) {
      shotTimer=(cv->shotType==2)?2:4;
    }
    if (cv->shotType==1) {
      cv->soundEffect(SE_SHOT2);
    } else {
      cv->soundEffect(SE_SHOT1);
    }
    FurnaceCVBullet* b;

    b=cv->createObject<FurnaceCVBullet>(x+shootDirOffsX[shootDir],y+shootDirOffsY[shootDir]);
    b->orient=shootDir;
    b->setType((cv->shotType==1)?1:0);
    switch (shootDir) {
      case 0:
      case 1:
      case 7:
        b->speedX=160;
        break;
      case 3:
      case 4:
      case 5:
        b->speedX=-160;
        break;
      default:
        b->speedX=0;
        break;
    }
    switch (shootDir) {
      case 1:
      case 2:
      case 3:
        b->speedY=-160;
        break;
      case 5:
      case 6:
      case 7:
        b->speedY=160;
        break;
      default:
        b->speedY=0;
        break;
    }

    if (doubleShot) {
      b=cv->createObject<FurnaceCVBullet>(x+shootDirOffsX[((shootDir+4)&7)],y+shootDirOffsY[((shootDir+4)&7)]);
      b->orient=((shootDir+4)&7);
      b->setType((cv->shotType==1)?1:0);
      switch (((shootDir+4)&7)) {
        case 0:
        case 1:
        case 7:
          b->speedX=160;
          break;
        case 3:
        case 4:
        case 5:
          b->speedX=-160;
          break;
        default:
          b->speedX=0;
          break;
      }
      switch (((shootDir+4)&7)) {
        case 1:
        case 2:
        case 3:
          b->speedY=-160;
          break;
        case 5:
        case 6:
        case 7:
          b->speedY=160;
          break;
        default:
          b->speedY=0;
          break;
      }
    }
  }

  if (cv->joyInput&1) {
    if (--shotTimer<1) {
      shotTimer=(cv->shotType==2)?4:8;
      if (cv->speedTicks>0) {
        shotTimer=(cv->shotType==2)?1:3;
      }
      if (cv->shotType==1) {
        cv->soundEffect(SE_SHOT2);
      } else {
        cv->soundEffect(SE_SHOT1);
      }

      FurnaceCVBullet* b;
      
      b=cv->createObject<FurnaceCVBullet>(x+shootDirOffsX[shootDir],y+shootDirOffsY[shootDir]);
      b->orient=shootDir;
      b->setType((cv->shotType==1)?1:0);
      switch (shootDir) {
        case 0:
        case 1:
        case 7:
          b->speedX=160;
          break;
        case 3:
        case 4:
        case 5:
          b->speedX=-160;
          break;
        default:
          b->speedX=0;
          break;
      }
      switch (shootDir) {
        case 1:
        case 2:
        case 3:
          b->speedY=-160;
          break;
        case 5:
        case 6:
        case 7:
          b->speedY=160;
          break;
        default:
          b->speedY=0;
          break;
      }

      if (cv->shotType==2) {
        b->speedX+=(rand()%64)-32;
        b->speedY+=(rand()%64)-32;
      }

      if (doubleShot) {
        b=cv->createObject<FurnaceCVBullet>(x+shootDirOffsX[((shootDir+4)&7)],y+shootDirOffsY[((shootDir+4)&7)]);
        b->orient=((shootDir+4)&7);
        b->setType((cv->shotType==1)?1:0);
        switch (((shootDir+4)&7)) {
          case 0:
          case 1:
          case 7:
            b->speedX=160;
            break;
          case 3:
          case 4:
          case 5:
            b->speedX=-160;
            break;
          default:
            b->speedX=0;
            break;
        }
        switch (((shootDir+4)&7)) {
          case 1:
          case 2:
          case 3:
            b->speedY=-160;
            break;
          case 5:
          case 6:
          case 7:
            b->speedY=160;
            break;
          default:
            b->speedY=0;
            break;
        }

        if (cv->shotType==2) {
          b->speedX+=(rand()%64)-32;
          b->speedY+=(rand()%64)-32;
        }
      }
    }
  }

  if (cv->joyPressed&2) {
    if (cv->specialType>0) {
      switch (cv->specialType) {
        case 1: // ?
          switch (rand()%30) {
            case 0: case 1: case 2: case 3: case 4:
            case 5: case 6: case 7: case 8: case 9: // spawn enemies
              for (int i=0; i<10; i++) {
                FurnaceCVEnemy1* obj=cv->createObject<FurnaceCVEnemy1>((rand()%(cv->stageWidth-4))<<3,(rand()%(cv->stageHeight-4))<<3);
                obj->setType(rand()%2);
              }
              invincible+=60;
              cv->soundEffect(SE_DEATH_C1);
              break;
            case 10: case 11: case 12: case 13: // downgrade enemies
              for (FurnaceCVObject* i: cv->sprite) {
                if (i->type==CV_ENEMY) {
                  if (((FurnaceCVEnemy*)i)->enemyType>0) {
                    if (((FurnaceCVEnemy*)i)->enemyType>1) {
                      cv->createObject<FurnaceCVFurBallLarge>(i->x-4,i->y-4);
                    } else {
                      cv->createObject<FurnaceCVFurBallMedium>(i->x-4,i->y-4);
                    }
                    if (((FurnaceCVEnemy*)i)->enemyType==2) {
                      i->x+=8;
                      i->y+=8;
                    }
                    ((FurnaceCVEnemy*)i)->setType(((FurnaceCVEnemy*)i)->enemyType-1);
                  }
                }
              }
              cv->soundEffect(SE_EXPL2);
              break;
            case 14: case 15: case 16: // teleport
              cv->createObject<FurnaceCVFurBallLarge>(x-4,y-4);
              invincible=120;
              x=(rand()%(cv->stageWidth-2))<<3;
              y=(rand()%(cv->stageHeight-2))<<3;
              cv->createObject<FurnaceCVFurBallLarge>(x-4,y-4);
              cv->soundEffect(SE_INIT);
              for (FurnaceCVObject* i: cv->sprite) {
                if (i->type==CV_ENEMY_BULLET) {
                  i->dead=true;
                }
              }
              break;
            case 17: case 18: case 19: // stop enemies
              for (FurnaceCVObject* i: cv->sprite) {
                if (i->type==CV_ENEMY) {
                  ((FurnaceCVEnemy*)i)->stopped=true;
                }
              }
              cv->soundEffect(SE_TIMEUP);
              break;
            case 20: case 21: case 22: // speed + invincible
              invincible=600;
              cv->speedTicks=900;
              cv->soundEffect(SE_PICKUP3);
              break;
            case 23: case 24: // purple tank
              for (int i=0; i<6; i++) {
                FurnaceCVEnemy1* obj=cv->createObject<FurnaceCVEnemy1>((rand()%(cv->stageWidth-3))<<3,(rand()%(cv->stageHeight-3))<<3);
                obj->setType(3);
              }
              invincible+=60;
              cv->soundEffect(SE_DEATH_C1);
              break;
            case 25: case 26: // vortex
              for (int i=0; i<12; i++) {
                cv->createObject<FurnaceCVEnemyVortex>((rand()%(cv->stageWidth-2))<<3,(rand()%(cv->stageHeight-2))<<3);
              }
              invincible+=60;
              cv->soundEffect(SE_DEATH_C1);
              break;
            case 27: // next level
              for (FurnaceCVObject* i: cv->sprite) {
                if (i->type==CV_ENEMY) {
                  i->dead=true;
                }
              }
              break;
            case 28: // 5-up
              cv->soundEffect(SE_PICKUP1);
              cv->lives+=5;
              break;
            case 29: // plane
              for (int i=0; i<6; i++) {
                cv->createObjectNoPos<FurnaceCVEnemyPlane>();
              }
              cv->soundEffect(SE_TIMEUP);
              break;
          }
          break;
        case 2: // T
          cv->createObject<FurnaceCVFurBallLarge>(x-4,y-4);
          invincible=120;
          x=(rand()%(cv->stageWidth-2))<<3;
          y=(rand()%(cv->stageHeight-2))<<3;
          cv->createObject<FurnaceCVFurBallLarge>(x-4,y-4);
          cv->soundEffect(SE_INIT);
          for (FurnaceCVObject* i: cv->sprite) {
            if (i->type==CV_ENEMY_BULLET) {
              i->dead=true;
            }
          }
          break;
        case 3: { // X
          for (int i=0; i<64; i++) {
            FurnaceCVBullet* b=cv->createObject<FurnaceCVBullet>(x+4,y+4);
            b->orient=(-i>>3)&7;
            b->setType(1);
            b->speedX=120*cos(M_PI*((float)i/32.0));
            b->speedY=120*sin(M_PI*((float)i/32.0));
          }
          cv->soundEffect(SE_SHOT2);
          break;
        }
        case 4: // W
          doubleShot=true;
          cv->soundEffect(SE_PICKUP3);
          break;
        case 5: // S
          for (FurnaceCVObject* i: cv->sprite) {
            if (i->type==CV_ENEMY) {
              i->frozen=600;
            }
          }
          cv->soundEffect(SE_TIMEUP);
          break;
      }
      cv->specialType=0;
    }
  }

  if (invincible>0) {
    invincible--;
  }

  if (invincible&1) {
    memset(spriteDef,0,9*sizeof(unsigned short));
  } else {
    spriteDef[0]=0x10+((animFrame>>6)<<2)+shootDirOrient[shootDir];
    spriteDef[1]=0x11+((animFrame>>6)<<2)+shootDirOrient[shootDir];
    spriteDef[2]=0x12+((animFrame>>6)<<2)+shootDirOrient[shootDir];
    spriteDef[3]=0x30+((animFrame>>6)<<2)+shootDirOrient[shootDir];
    spriteDef[4]=0x31+((animFrame>>6)<<2)+shootDirOrient[shootDir];
    spriteDef[5]=0x32+((animFrame>>6)<<2)+shootDirOrient[shootDir];
    spriteDef[6]=0x50+((animFrame>>6)<<2)+shootDirOrient[shootDir];
    spriteDef[7]=0x51+((animFrame>>6)<<2)+shootDirOrient[shootDir];
    spriteDef[8]=0x52+((animFrame>>6)<<2)+shootDirOrient[shootDir];
  }

  animFrame+=MAX(fabs(speedX),fabs(speedY));

  // camera
  cv->viewX=x-160+12;
  cv->viewY=y-112+12;
  if (cv->viewX<0) cv->viewX=0;
  if (cv->viewX>cv->stageWidthPx-320) cv->viewX=cv->stageWidthPx-320;
  if (cv->viewY<0) cv->viewY=0;
  if (cv->viewY>cv->stageHeightPx-224) cv->viewY=cv->stageHeightPx-224;
  cv->scrollX[0]=cv->viewX;
  cv->scrollY[0]=cv->viewY;
}

// FurnaceCVBullet IMPLEMENTATION

void FurnaceCVBullet::setType(unsigned char t) {
  bulletType=t;

  if (bulletType==1) {
    spriteWidth=2;
    spriteHeight=2;

    switch (orient) {
      case 0:
        spriteDef[0]=0x496;
        spriteDef[1]=0x497;
        spriteDef[2]=0x4b6;
        spriteDef[3]=0x4b7;
        break;
      case 1:
        spriteDef[0]=0x49c;
        spriteDef[1]=0x49d;
        spriteDef[2]=0x4bc;
        spriteDef[3]=0x4bd;
        break;
      case 2:
        spriteDef[0]=0x494;
        spriteDef[1]=0x495;
        spriteDef[2]=0x4b4;
        spriteDef[3]=0x4b5;
        break;
      case 3:
        spriteDef[0]=0x45e;
        spriteDef[1]=0x45f;
        spriteDef[2]=0x47e;
        spriteDef[3]=0x47f;
        break;
      case 4:
        spriteDef[0]=0x49a;
        spriteDef[1]=0x49b;
        spriteDef[2]=0x4ba;
        spriteDef[3]=0x4bb;
        break;
      case 5:
        spriteDef[0]=0x45c;
        spriteDef[1]=0x45d;
        spriteDef[2]=0x47c;
        spriteDef[3]=0x47d;
        break;
      case 6:
        spriteDef[0]=0x498;
        spriteDef[1]=0x499;
        spriteDef[2]=0x4b8;
        spriteDef[3]=0x4b9;
        break;
      case 7:
        spriteDef[0]=0x49e;
        spriteDef[1]=0x49f;
        spriteDef[2]=0x4be;
        spriteDef[3]=0x4bf;
        break;
    }

    collX0=0;
    collX1=15;
    collY0=0;
    collY1=15;
    
    life=11;
  }
}

void FurnaceCVBullet::killBullet() {
  if (bulletType==1) {
    cv->soundEffect(SE_EXPL1);
    FurnaceCVBullet* b1=cv->createObject<FurnaceCVBullet>(x+4,y+4);
    FurnaceCVBullet* b2=cv->createObject<FurnaceCVBullet>(x+4,y+4);
    FurnaceCVBullet* b3=cv->createObject<FurnaceCVBullet>(x+4,y+4);
    switch (orient) {
      case 0:
        b1->speedX=240;
        b1->speedY=-100;
        b2->speedX=240;
        b2->speedY=0;
        b3->speedX=240;
        b3->speedY=100;
        break;
      case 1:
        b1->speedX=130;
        b1->speedY=-170;
        b2->speedX=170;
        b2->speedY=-170;
        b3->speedX=170;
        b3->speedY=-130;
        break;
      case 2:
        b1->speedX=-100;
        b1->speedY=-240;
        b2->speedX=0;
        b2->speedY=-240;
        b3->speedX=100;
        b3->speedY=-240;
        break;
      case 3:
        b1->speedX=-130;
        b1->speedY=-170;
        b2->speedX=-170;
        b2->speedY=-170;
        b3->speedX=-170;
        b3->speedY=-130;
        break;
      case 4:
        b1->speedX=-240;
        b1->speedY=-100;
        b2->speedX=-240;
        b2->speedY=0;
        b3->speedX=-240;
        b3->speedY=100;
        break;
      case 5:
        b1->speedX=-130;
        b1->speedY=170;
        b2->speedX=-170;
        b2->speedY=170;
        b3->speedX=-170;
        b3->speedY=130;
        break;
      case 6:
        b1->speedX=-100;
        b1->speedY=240;
        b2->speedX=0;
        b2->speedY=240;
        b3->speedX=100;
        b3->speedY=240;
        break;
      case 7:
        b1->speedX=130;
        b1->speedY=170;
        b2->speedX=170;
        b2->speedY=170;
        b3->speedX=170;
        b3->speedY=130;
        break;
    }
    b1->orient=orient;
    b2->orient=orient;
    b3->orient=orient;
    cv->createObject<FurnaceCVExplMedium>(x-8,y-8);
  } else {
    cv->createObject<FurnaceCVExplTiny>(x,y);
  }
}

void FurnaceCVBullet::collision(FurnaceCVObject* other) {
  if (other->type==CV_ENEMY || other->type==CV_MINE) {
    dead=true;
    killBullet();
  }
}

void FurnaceCVBullet::tick() {
  subX+=speedX;
  while (subX>=32) {
    x++;
    subX-=32;
  }
  while (subX<0) {
    x--;
    subX+=32;
  }

  subY+=speedY;
  while (subY>=32) {
    y++;
    subY-=32;
  }
  while (subY<0) {
    y--;
    subY+=32;
  }

  if (life>0) {
    if (--life<=0) {
      dead=true;
      killBullet();
    }
  }

  if (!IS_IN_AREA) {
    dead=true;
  }
}

// FurnaceCVEnemyBullet IMPLEMENTATION

void FurnaceCVEnemyBullet::setType(unsigned char type) {
  bulletType=type;
}

void FurnaceCVEnemyBullet::tick() {
  subX+=speedX;
  while (subX>=32) {
    x++;
    subX-=32;
  }
  while (subX<0) {
    x--;
    subX+=32;
  }

  subY+=speedY;
  while (subY>=32) {
    y++;
    subY-=32;
  }
  while (subY<0) {
    y--;
    subY+=32;
  }

  if (bulletType==1) {
    animFrame+=10+(rand()%12);
    spriteDef[0]=0x28+(animFrame>>6);
  }

  if (!IS_IN_AREA) {
    dead=true;
  }
}

// FurnaceCVEnemy1 IMPLEMENTATION

void FurnaceCVEnemy1::collision(FurnaceCVObject* other) {
  if (other->type==CV_BULLET || other->type==CV_PLAYER) {
    if (--health<=0) {
      dead=true;
      if ((rand()%7)==0 || (enemyType>1 && (rand()%3)==2)) {
        switch (rand()%14) {
          case 0: // extra life
            cv->createObject<FurnaceCVExtraLife>(x+(enemyType>=2?8:0),y+(enemyType>=2?8:0));
            break;
          case 1: case 2: case 3: case 4: // powerup
            cv->createObject<FurnaceCVPowerupP>(x+(enemyType>=2?8:0),y+(enemyType>=2?8:0));
            break;
          case 5: case 6: case 7: case 8: case 9: // powerup
            cv->createObject<FurnaceCVPowerupS>(x+(enemyType>=2?8:0),y+(enemyType>=2?8:0));
            break;
          case 10: case 11: // special
            cv->createObject<FurnaceCVSpecial>(x+(enemyType>=2?8:0),y+(enemyType>=2?8:0));
            break;
          case 12: // mod
            cv->createObject<FurnaceCVModS>(x+(enemyType>=2?8:0),y+(enemyType>=2?8:0));
            break;
          case 13: // mod
            cv->createObject<FurnaceCVModI>(x+(enemyType>=2?8:0),y+(enemyType>=2?8:0));
            break;
        }
      }
      cv->soundEffect(SE_EXPL1);
      switch (enemyType) {
        case 0:
          cv->createObject<FurnaceCVExplMedium>(x-4,y-4);
          cv->addScore(100);
          break;
        case 1:
          cv->createObject<FurnaceCVExplMedium>(x-4,y-4);
          cv->addScore(200);
          break;
        case 2:
          cv->createObject<FurnaceCVExplMedium>(x-8,y);
          cv->createObject<FurnaceCVExplMedium>(x+8,y);
          cv->createObject<FurnaceCVExplMedium>(x,y-8);
          cv->createObject<FurnaceCVExplMedium>(x,y+8);
          cv->createObject<FurnaceCVExplMedium>(x,y);
          cv->addScore(400);
          break;
        case 3:
          cv->createObject<FurnaceCVExplMedium>(x-8,y);
          cv->createObject<FurnaceCVExplMedium>(x+8,y);
          cv->createObject<FurnaceCVExplMedium>(x,y-8);
          cv->createObject<FurnaceCVExplMedium>(x,y+8);
          cv->createObject<FurnaceCVExplMedium>(x,y);
          cv->addScore(800);
          break;
      }
    } else {
      cv->soundEffect(SE_RESIST);
    }
  } else if (other->type==CV_ENEMY || other->type==CV_MINE) {
    // reorient
    orientCount+=2;
    if (orientCount>6) {
      // stuck...
      stopped=true;
      cv->stopSoundEffect(SE_TANKMOVE);
    } else {
      orient=(orient+2)&3;
    }
  }
}

void FurnaceCVEnemy1::tick() {
  if (frozen>0) {
    if (--frozen>0) return;
  }

  if (!stopped) {
    switch (orient) {
      case 0:
        x+=MAX(1,enemyType);
        break;
      case 1:
        y-=MAX(1,enemyType);
        break;
      case 2:
        x-=MAX(1,enemyType);
        break;
      case 3:
        y+=MAX(1,enemyType);
        break;
    }
    animFrame+=0x15;
  }

  if (--orientCount<=0) orientCount=0;

  if (--nextTime==0) {
    nextTime=64+(rand()%600);

    stopped=((rand()%10)==0);
    if (stopped) {
      nextTime>>=2;
      cv->stopSoundEffect(SE_TANKMOVE);
    } else {
      unsigned char oldOrient=orient;
      orient=rand()&3;
      if (orient==oldOrient) orient=(orient+1)&3;
      cv->soundEffect(SE_TANKMOVE);
    }
  }

  if (shootCooldown>64) {
    shootCooldown+=16*(enemyType+1);
  } else if (--shootTime==0) {
    shootTime=(enemyType>=2)?4:8;
    for (FurnaceCVObject* i: cv->sprite) {
      if (i->type==CV_PLAYER) {
        float dist=sqrt(pow((y+8)-(i->y+12),2.0)+pow((i->x+12)-(x+8),2.0));
        float angle=atan2((y+8)-(i->y+12),(i->x+12)-(x+8))/(M_PI*0.5);
        if (angle<0.0) angle+=4.0;
        float angleDelta=angle-(float)orient;
        if (angleDelta>=2.0) angleDelta-=4.0;
        if (angleDelta<-2.0) angleDelta+=4.0;
        if (fabs(angleDelta)<0.3 && dist<256.0 && IS_VISIBLE) {
          cv->soundEffect(SE_SHOT1);
          shootCooldown+=16-enemyType*2;
          FurnaceCVEnemyBullet* b=cv->createObject<FurnaceCVEnemyBullet>(x+(enemyType>=2?8:4),y+(enemyType>=2?8:4));
          switch (orient) {
            case 0:
              b->speedX=160;
              break;
            case 1:
              b->speedY=-160;
              break;
            case 2:
              b->speedX=-160;
              break;
            case 3:
              b->speedY=160;
              break;
          }

          if (enemyType>0) {
            b->speedX+=(rand()%64)-32;
            b->speedY+=(rand()%64)-32;
          }
        }
        break;
      }
    }
  }

  if (HITS_BORDER) {
    orient=(orient+2)&3;

    CONFINE_TO_BORDER;
  }

  if (enemyType>=2) {
    spriteWidth=3;
    spriteHeight=3;
    collX1=23;
    collY1=23;
    switch (orient) {
      case 0:
        spriteDef[0]=0x300+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[1]=0x301+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[2]=0x302+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[3]=0x320+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[4]=0x321+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[5]=0x322+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[6]=0x340+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[7]=0x341+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[8]=0x342+((animFrame>>7)<<2)+(enemyType==3?8:0);
        break;
      case 1:
        spriteDef[0]=0x280+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[1]=0x281+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[2]=0x282+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[3]=0x2a0+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[4]=0x2a1+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[5]=0x2a2+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[6]=0x2c0+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[7]=0x2c1+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[8]=0x2c2+((animFrame>>7)<<2)+(enemyType==3?8:0);
        break;
      case 2:
        spriteDef[0]=0x400+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[1]=0x401+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[2]=0x402+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[3]=0x420+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[4]=0x421+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[5]=0x422+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[6]=0x440+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[7]=0x441+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[8]=0x442+((animFrame>>7)<<2)+(enemyType==3?8:0);
        break;
      case 3:
        spriteDef[0]=0x380+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[1]=0x381+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[2]=0x382+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[3]=0x3a0+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[4]=0x3a1+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[5]=0x3a2+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[6]=0x3c0+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[7]=0x3c1+((animFrame>>7)<<2)+(enemyType==3?8:0);
        spriteDef[8]=0x3c2+((animFrame>>7)<<2)+(enemyType==3?8:0);
        break;
    }
  } else {
    spriteWidth=2;
    spriteHeight=2;
    collX1=15;
    collY1=15;
    switch (orient) {
      case 0:
        spriteDef[0]=0x204+((animFrame>>7)<<1)+(enemyType==1?0x40:0);
        spriteDef[1]=0x205+((animFrame>>7)<<1)+(enemyType==1?0x40:0);
        spriteDef[2]=0x224+((animFrame>>7)<<1)+(enemyType==1?0x40:0);
        spriteDef[3]=0x225+((animFrame>>7)<<1)+(enemyType==1?0x40:0);
        break;
      case 1:
        spriteDef[0]=0x200+((animFrame>>7)<<1)+(enemyType==1?0x40:0);
        spriteDef[1]=0x201+((animFrame>>7)<<1)+(enemyType==1?0x40:0);
        spriteDef[2]=0x220+((animFrame>>7)<<1)+(enemyType==1?0x40:0);
        spriteDef[3]=0x221+((animFrame>>7)<<1)+(enemyType==1?0x40:0);
        break;
      case 2:
        spriteDef[0]=0x20c+((animFrame>>7)<<1)+(enemyType==1?0x40:0);
        spriteDef[1]=0x20d+((animFrame>>7)<<1)+(enemyType==1?0x40:0);
        spriteDef[2]=0x22c+((animFrame>>7)<<1)+(enemyType==1?0x40:0);
        spriteDef[3]=0x22d+((animFrame>>7)<<1)+(enemyType==1?0x40:0);
        break;
      case 3:
        spriteDef[0]=0x208+((animFrame>>7)<<1)+(enemyType==1?0x40:0);
        spriteDef[1]=0x209+((animFrame>>7)<<1)+(enemyType==1?0x40:0);
        spriteDef[2]=0x228+((animFrame>>7)<<1)+(enemyType==1?0x40:0);
        spriteDef[3]=0x229+((animFrame>>7)<<1)+(enemyType==1?0x40:0);
        break;
    }
  }
}

void FurnaceCVEnemy::setType(unsigned char t) {
  enemyType=t;
  switch (enemyType) {
    case 0:
      health=1;
      break;
    case 1:
      health=2;
      break;
    case 2:
      health=8;
      break;
    case 3:
      health=14;
      break;
  }
}

// FurnaceCVExplTiny IMPLEMENTATION

void FurnaceCVExplTiny::tick() {
  spriteDef[0]=8+(animFrame>>1);

  if (animFrame>=4) {
    spriteDef[0]=0;
    dead=true;
  }

  animFrame++;
}

// FurnaceCVExplMedium IMPLEMENTATION

void FurnaceCVExplMedium::tick() {
  if ((animFrame/3)>=5) {
    memset(spriteDef,0,9*sizeof(unsigned short));
    dead=true;
  } else {
    spriteDef[0]=0x210+(animFrame/3)*3;
    spriteDef[1]=0x211+(animFrame/3)*3;
    spriteDef[2]=0x212+(animFrame/3)*3;
    spriteDef[3]=0x230+(animFrame/3)*3;
    spriteDef[4]=0x231+(animFrame/3)*3;
    spriteDef[5]=0x232+(animFrame/3)*3;
    spriteDef[6]=0x250+(animFrame/3)*3;
    spriteDef[7]=0x251+(animFrame/3)*3;
    spriteDef[8]=0x252+(animFrame/3)*3;
  }

  animFrame++;
}

// FurnaceCVFurBallMedium IMPLEMENTATION

void FurnaceCVFurBallMedium::tick() {
  if ((animFrame/3)>=4) {
    memset(spriteDef,0,9*sizeof(unsigned short));
    dead=true;
  } else {
    spriteDef[0]=0x410+(animFrame/3)*3;
    spriteDef[1]=0x411+(animFrame/3)*3;
    spriteDef[2]=0x412+(animFrame/3)*3;
    spriteDef[3]=0x430+(animFrame/3)*3;
    spriteDef[4]=0x431+(animFrame/3)*3;
    spriteDef[5]=0x432+(animFrame/3)*3;
    spriteDef[6]=0x450+(animFrame/3)*3;
    spriteDef[7]=0x451+(animFrame/3)*3;
    spriteDef[8]=0x452+(animFrame/3)*3;
  }

  animFrame++;
}

// FurnaceCVFurBallLarge IMPLEMENTATION

void FurnaceCVFurBallLarge::tick() {
  if ((animFrame/3)>=4) {
    memset(spriteDef,0,16*sizeof(unsigned short));
    dead=true;
  } else {
    for (int i=0; i<4; i++) {
      spriteDef[i<<2]=(i<<5)+0x390+((animFrame/3)<<2);
      spriteDef[1+(i<<2)]=(i<<5)+0x391+((animFrame/3)<<2);
      spriteDef[2+(i<<2)]=(i<<5)+0x392+((animFrame/3)<<2);
      spriteDef[3+(i<<2)]=(i<<5)+0x393+((animFrame/3)<<2);
    }
  }

  animFrame++;
}

// INSTRUMENTS

static const unsigned char __00_fui[] = {
  0x46, 0x49, 0x4e, 0x53, 0xc4, 0x00, 0x38, 0x00, 0x4e, 0x41, 0x06, 0x00,
  0x73, 0x68, 0x6f, 0x74, 0x31, 0x00, 0x4d, 0x41, 0x8c, 0x00, 0x08, 0x00,
  0x00, 0x0b, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x0f, 0x0b, 0x09, 0x07,
  0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00, 0x01, 0x10, 0xff, 0xff, 0x00,
  0xc1, 0x00, 0x01, 0x42, 0x00, 0x00, 0x40, 0x48, 0x00, 0x00, 0x40, 0x3c,
  0x00, 0x00, 0x40, 0x36, 0x00, 0x00, 0x40, 0x32, 0x00, 0x00, 0x40, 0x2f,
  0x00, 0x00, 0x40, 0x28, 0x00, 0x00, 0x40, 0x24, 0x00, 0x00, 0x40, 0x21,
  0x00, 0x00, 0x40, 0x1f, 0x00, 0x00, 0x40, 0x1f, 0x00, 0x00, 0x40, 0x24,
  0x00, 0x00, 0x40, 0x24, 0x00, 0x00, 0x40, 0x24, 0x00, 0x00, 0x40, 0x24,
  0x00, 0x00, 0x40, 0x21, 0x00, 0x00, 0x40, 0x05, 0x01, 0xff, 0xff, 0x00,
  0x01, 0x00, 0x01, 0x01, 0x0e, 0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01,
  0x00, 0x0f, 0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x0d, 0x10, 0x01,
  0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x0e, 0x13, 0x01, 0xff, 0xff, 0x00,
  0x81, 0x00, 0x01, 0xd8, 0x75, 0xff, 0x50, 0x4e, 0x01, 0x00, 0x03
};
static const unsigned int __00_fui_len = 167;
static const unsigned char __01_fui[] = {
  0x46, 0x49, 0x4e, 0x53, 0xc4, 0x00, 0x38, 0x00, 0x4e, 0x41, 0x0c, 0x00,
  0x65, 0x78, 0x70, 0x6c, 0x31, 0x20, 0x28, 0x47, 0x2d, 0x35, 0x29, 0x00,
  0x4d, 0x41, 0xc5, 0x00, 0x08, 0x00, 0x00, 0x3a, 0xff, 0xff, 0x00, 0x01,
  0x00, 0x01, 0x0a, 0x0e, 0x0e, 0x0d, 0x0d, 0x0c, 0x0e, 0x0f, 0x09, 0x0e,
  0x0d, 0x0e, 0x0d, 0x0c, 0x0c, 0x0c, 0x0b, 0x0b, 0x0b, 0x0a, 0x0a, 0x0a,
  0x0a, 0x09, 0x09, 0x09, 0x08, 0x08, 0x08, 0x08, 0x08, 0x07, 0x07, 0x07,
  0x06, 0x06, 0x06, 0x06, 0x06, 0x05, 0x05, 0x05, 0x05, 0x04, 0x04, 0x04,
  0x03, 0x03, 0x03, 0x03, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
  0x01, 0x25, 0x06, 0xff, 0x00, 0x41, 0x00, 0x01, 0xf5, 0xe9, 0xe7, 0xe5,
  0xe4, 0x04, 0x03, 0xf9, 0xf4, 0x01, 0xfa, 0x03, 0xf3, 0xf2, 0x00, 0xf4,
  0x04, 0x05, 0xf8, 0x00, 0xf9, 0x01, 0xf9, 0xfc, 0xfc, 0xf7, 0x02, 0xff,
  0xf8, 0x04, 0x00, 0xf7, 0xf7, 0xff, 0xfa, 0x00, 0xfb, 0x05, 0x06, 0xff,
  0xff, 0x00, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x0e,
  0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x01, 0x0f, 0x06, 0xff, 0xff,
  0x00, 0x01, 0x00, 0x01, 0x0a, 0x01, 0x01, 0x01, 0x01, 0x06, 0x10, 0x06,
  0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x05, 0x04, 0x04, 0x04, 0x04, 0x0c,
  0x13, 0x06, 0xff, 0xff, 0x00, 0xc1, 0x00, 0x01, 0xad, 0x3a, 0x00, 0x00,
  0xaa, 0xaa, 0x00, 0x00, 0xaa, 0xaa, 0x00, 0x00, 0xaa, 0xaa, 0x00, 0x00,
  0xaa, 0xaa, 0x00, 0x00, 0x20, 0x21, 0x00, 0x00, 0xff, 0x50, 0x4e, 0x01,
  0x00, 0x00
};
static const unsigned int __01_fui_len = 230;
static const unsigned char __02_fui[] = {
  0x46, 0x49, 0x4e, 0x53, 0xc4, 0x00, 0x38, 0x00, 0x4e, 0x41, 0x0e, 0x00,
  0x70, 0x69, 0x63, 0x6b, 0x75, 0x70, 0x31, 0x20, 0x28, 0x47, 0x2d, 0x35,
  0x29, 0x00, 0x4d, 0x41, 0x5d, 0x00, 0x08, 0x00, 0x00, 0x21, 0xff, 0xff,
  0x00, 0x01, 0x00, 0x01, 0x0f, 0x0f, 0x04, 0x04, 0x0c, 0x0c, 0x03, 0x03,
  0x0a, 0x0a, 0x02, 0x02, 0x08, 0x08, 0x01, 0x01, 0x06, 0x06, 0x06, 0x06,
  0x04, 0x04, 0x04, 0x04, 0x02, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01,
  0x00, 0x01, 0x04, 0x00, 0xff, 0x00, 0x01, 0x00, 0x04, 0x00, 0x04, 0x07,
  0x0c, 0x05, 0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x00, 0x0e, 0x01,
  0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x01, 0x0f, 0x01, 0xff, 0xff, 0x00,
  0x01, 0x00, 0x01, 0x01, 0x13, 0x01, 0xff, 0xff, 0x00, 0x81, 0x00, 0x01,
  0x55, 0x55, 0xff, 0x50, 0x4e, 0x01, 0x00, 0x00
};
static const unsigned int __02_fui_len = 128;
static const unsigned char __03_fui[] = {
  0x46, 0x49, 0x4e, 0x53, 0xc4, 0x00, 0x38, 0x00, 0x4e, 0x41, 0x0e, 0x00,
  0x70, 0x69, 0x63, 0x6b, 0x75, 0x70, 0x32, 0x20, 0x28, 0x41, 0x2d, 0x35,
  0x29, 0x00, 0x4d, 0x41, 0x44, 0x00, 0x08, 0x00, 0x00, 0x0a, 0xff, 0xff,
  0x00, 0x01, 0x00, 0x01, 0x0f, 0x0e, 0x04, 0x0d, 0x0c, 0x03, 0x02, 0x02,
  0x01, 0x00, 0x01, 0x02, 0x00, 0xff, 0x00, 0x01, 0x00, 0x03, 0x00, 0x07,
  0x05, 0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x00, 0x0e, 0x01, 0xff,
  0xff, 0x00, 0x01, 0x00, 0x01, 0x01, 0x0f, 0x01, 0xff, 0xff, 0x00, 0x01,
  0x00, 0x01, 0x01, 0x13, 0x01, 0xff, 0xff, 0x00, 0x81, 0x00, 0x01, 0x55,
  0x55, 0xff, 0x50, 0x4e, 0x01, 0x00, 0x00
};
static const unsigned int __03_fui_len = 103;
static const unsigned char __04_fui[] = {
  0x46, 0x49, 0x4e, 0x53, 0xc4, 0x00, 0x38, 0x00, 0x4e, 0x41, 0x0f, 0x00,
  0x64, 0x65, 0x61, 0x74, 0x68, 0x20, 0x63, 0x31, 0x20, 0x28, 0x43, 0x2d,
  0x34, 0x29, 0x00, 0x4d, 0x41, 0xbf, 0x00, 0x08, 0x00, 0x00, 0x50, 0xff,
  0xff, 0x00, 0x01, 0x00, 0x01, 0x0f, 0x0e, 0x0b, 0x09, 0x0f, 0x0f, 0x0f,
  0x0e, 0x0e, 0x0d, 0x0d, 0x0c, 0x0c, 0x0c, 0x0b, 0x0b, 0x0c, 0x0b, 0x09,
  0x08, 0x0d, 0x0d, 0x0c, 0x0c, 0x0b, 0x0b, 0x0b, 0x0a, 0x0a, 0x0a, 0x0a,
  0x09, 0x09, 0x09, 0x08, 0x08, 0x09, 0x09, 0x08, 0x08, 0x07, 0x07, 0x06,
  0x06, 0x06, 0x06, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
  0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x03, 0x03, 0x03, 0x02, 0x02, 0x01,
  0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x00, 0x01, 0x10, 0x00, 0xff, 0x00, 0x41, 0x00, 0x01, 0xff, 0xf7, 0xf0,
  0xef, 0x06, 0x02, 0xfd, 0xfb, 0xf7, 0xf5, 0xf3, 0xf1, 0xf0, 0xee, 0xec,
  0xec, 0x04, 0x01, 0x00, 0xff, 0x01, 0x41, 0x00, 0x01, 0xf6, 0x05, 0x01,
  0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x00, 0x0e, 0x10, 0x00, 0xff, 0x00,
  0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x10, 0x00, 0xff, 0x00,
  0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x05, 0x05, 0x05, 0x05, 0x05,
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x13, 0x01, 0xff, 0xff, 0x00,
  0x81, 0x00, 0x01, 0x55, 0x55, 0xff, 0x50, 0x4e, 0x01, 0x00, 0x00
};
static const unsigned int __04_fui_len = 227;
static const unsigned char __05_fui[] = {
  0x46, 0x49, 0x4e, 0x53, 0xc4, 0x00, 0x38, 0x00, 0x4e, 0x41, 0x0f, 0x00,
  0x64, 0x65, 0x61, 0x74, 0x68, 0x20, 0x63, 0x32, 0x20, 0x28, 0x43, 0x2d,
  0x34, 0x29, 0x00, 0x4d, 0x41, 0xd8, 0x00, 0x08, 0x00, 0x00, 0x51, 0xff,
  0xff, 0x00, 0x01, 0x00, 0x01, 0x08, 0x0b, 0x0b, 0x0c, 0x0f, 0x0f, 0x0f,
  0x0e, 0x0e, 0x0d, 0x0d, 0x0c, 0x0c, 0x0c, 0x0b, 0x0b, 0x05, 0x07, 0x07,
  0x07, 0x09, 0x09, 0x09, 0x08, 0x08, 0x08, 0x08, 0x07, 0x07, 0x07, 0x07,
  0x07, 0x03, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
  0x04, 0x04, 0x04, 0x04, 0x04, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03,
  0x03, 0x03, 0x03, 0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01,
  0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x00, 0x01, 0x10, 0x00, 0xff, 0x00, 0x01, 0x00, 0x01, 0x36, 0x15,
  0x0f, 0x0b, 0x39, 0x37, 0x35, 0x34, 0x33, 0x32, 0x31, 0x31, 0x30, 0x2f,
  0x2f, 0x2e, 0x04, 0x01, 0x00, 0xff, 0x01, 0x41, 0x00, 0x01, 0xf6, 0x05,
  0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x01, 0x0e, 0x10, 0x00, 0xff,
  0x00, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x10, 0x00, 0xff,
  0x00, 0x01, 0x00, 0x01, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d,
  0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x10, 0x10, 0x00, 0xff,
  0x00, 0x01, 0x00, 0x01, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
  0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x13, 0x01, 0xff, 0xff,
  0x00, 0x81, 0x00, 0x01, 0xf5, 0x75, 0xff, 0x50, 0x4e, 0x01, 0x00, 0x00
};
static const unsigned int __05_fui_len = 252;
static const unsigned char __07_fui[] = {
  0x46, 0x49, 0x4e, 0x53, 0xc4, 0x00, 0x38, 0x00, 0x4e, 0x41, 0x0b, 0x00,
  0x74, 0x79, 0x70, 0x65, 0x77, 0x72, 0x69, 0x74, 0x65, 0x72, 0x00, 0x4d,
  0x41, 0x4c, 0x00, 0x08, 0x00, 0x00, 0x04, 0x00, 0xff, 0x00, 0x01, 0x00,
  0x01, 0x06, 0x05, 0x04, 0x00, 0x01, 0x01, 0x00, 0xff, 0x00, 0xc1, 0x00,
  0x01, 0x3e, 0x00, 0x00, 0x40, 0x05, 0x01, 0xff, 0xff, 0x00, 0x01, 0x00,
  0x01, 0x00, 0x0e, 0x04, 0x00, 0xff, 0x00, 0x01, 0x00, 0x01, 0x01, 0x00,
  0x00, 0x00, 0x0f, 0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x0e, 0x10,
  0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x07, 0x13, 0x01, 0xff, 0xff,
  0x00, 0x81, 0x00, 0x01, 0xc5, 0x55, 0xff, 0x50, 0x4e, 0x01, 0x00, 0x03
};
static const unsigned int __07_fui_len = 108;
static const unsigned char __08_fui[] = {
  0x46, 0x49, 0x4e, 0x53, 0xc4, 0x00, 0x38, 0x00, 0x4e, 0x41, 0x12, 0x00,
  0x74, 0x72, 0x61, 0x6e, 0x73, 0x69, 0x74, 0x69, 0x6f, 0x6e, 0x31, 0x20,
  0x28, 0x47, 0x2d, 0x33, 0x29, 0x00, 0x4d, 0x41, 0xdf, 0x00, 0x08, 0x00,
  0x00, 0x5e, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x0c, 0x0b, 0x0b, 0x0a,
  0x09, 0x08, 0x08, 0x08, 0x0f, 0x0d, 0x0d, 0x0c, 0x0b, 0x0b, 0x0a, 0x0a,
  0x0e, 0x0e, 0x0d, 0x0d, 0x0c, 0x0c, 0x0c, 0x0c, 0x0b, 0x0c, 0x0c, 0x0c,
  0x0d, 0x0d, 0x0e, 0x0e, 0x0e, 0x0d, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e,
  0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0d, 0x0d, 0x0c, 0x0c,
  0x0c, 0x0b, 0x0b, 0x0b, 0x0b, 0x0a, 0x0a, 0x09, 0x08, 0x08, 0x08, 0x07,
  0x07, 0x07, 0x07, 0x06, 0x06, 0x06, 0x05, 0x05, 0x04, 0x04, 0x04, 0x04,
  0x04, 0x03, 0x03, 0x03, 0x03, 0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x1b, 0x17, 0xff, 0x00, 0x01,
  0x00, 0x02, 0x00, 0x04, 0x07, 0x04, 0x07, 0x0c, 0x07, 0x0c, 0x10, 0x0c,
  0x10, 0x13, 0x10, 0x13, 0x18, 0x13, 0x18, 0x1c, 0x18, 0x1c, 0x1f, 0x1c,
  0x1f, 0x24, 0x1f, 0x1c, 0x1f, 0x04, 0x26, 0x25, 0xff, 0x01, 0x01, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x0e, 0x05, 0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x00,
  0x0e, 0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x01, 0x0f, 0x01, 0xff,
  0xff, 0x00, 0x01, 0x00, 0x01, 0x01, 0x13, 0x01, 0xff, 0xff, 0x00, 0x81,
  0x00, 0x01, 0x55, 0x55, 0xff, 0x50, 0x4e, 0x01, 0x00, 0x00
};
static const unsigned int __08_fui_len = 262;
static const unsigned char __09_fui[] = {
  0x46, 0x49, 0x4e, 0x53, 0xc4, 0x00, 0x38, 0x00, 0x4e, 0x41, 0x12, 0x00,
  0x74, 0x72, 0x61, 0x6e, 0x73, 0x69, 0x74, 0x69, 0x6f, 0x6e, 0x32, 0x20,
  0x28, 0x43, 0x2d, 0x34, 0x29, 0x00, 0x4d, 0x41, 0x77, 0x00, 0x08, 0x00,
  0x00, 0x3b, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x0f, 0x0e, 0x0c, 0x0b,
  0x0a, 0x09, 0x08, 0x07, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x02, 0x02,
  0x0b, 0x0a, 0x09, 0x08, 0x07, 0x06, 0x06, 0x05, 0x05, 0x04, 0x03, 0x03,
  0x02, 0x02, 0x01, 0x01, 0x07, 0x06, 0x06, 0x05, 0x05, 0x04, 0x04, 0x03,
  0x02, 0x02, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x04, 0x03, 0x03, 0x02,
  0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x04, 0x00, 0xff, 0x00,
  0x01, 0x00, 0x01, 0x04, 0x07, 0x0e, 0x12, 0x05, 0x01, 0xff, 0xff, 0x00,
  0x01, 0x00, 0x01, 0x00, 0x0e, 0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01,
  0x01, 0x0f, 0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x01, 0x13, 0x01,
  0xff, 0xff, 0x00, 0x81, 0x00, 0x01, 0x55, 0x55, 0xff, 0x50, 0x4e, 0x01,
  0x00, 0x00
};
static const unsigned int __09_fui_len = 158;
static const unsigned char __0a_fui[] = {
  0x46, 0x49, 0x4e, 0x53, 0xc4, 0x00, 0x38, 0x00, 0x4e, 0x41, 0x0b, 0x00,
  0x69, 0x6e, 0x69, 0x74, 0x20, 0x28, 0x43, 0x2d, 0x34, 0x29, 0x00, 0x4d,
  0x41, 0x6a, 0x00, 0x08, 0x00, 0x00, 0x29, 0xff, 0xff, 0x00, 0x01, 0x00,
  0x01, 0x0f, 0x0e, 0x0e, 0x0d, 0x0c, 0x0c, 0x0c, 0x0b, 0x0b, 0x0b, 0x0a,
  0x0a, 0x0a, 0x09, 0x09, 0x09, 0x08, 0x08, 0x08, 0x08, 0x07, 0x07, 0x07,
  0x07, 0x06, 0x06, 0x06, 0x06, 0x06, 0x05, 0x05, 0x05, 0x05, 0x04, 0x04,
  0x04, 0x04, 0x03, 0x02, 0x01, 0x00, 0x01, 0x09, 0x03, 0xff, 0x00, 0x01,
  0x00, 0x01, 0x07, 0x13, 0x07, 0x0b, 0x07, 0x04, 0x08, 0x0e, 0x04, 0x05,
  0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x00, 0x0e, 0x01, 0xff, 0xff,
  0x00, 0x01, 0x00, 0x01, 0x01, 0x0f, 0x01, 0xff, 0xff, 0x00, 0x01, 0x00,
  0x01, 0x01, 0x13, 0x01, 0xff, 0xff, 0x00, 0x81, 0x00, 0x01, 0x55, 0x55,
  0xff, 0x50, 0x4e, 0x01, 0x00, 0x00
};
static const unsigned int __0a_fui_len = 138;
static const unsigned char __0b_fui[] = {
  0x46, 0x49, 0x4e, 0x53, 0xc4, 0x00, 0x38, 0x00, 0x4e, 0x41, 0x0e, 0x00,
  0x72, 0x65, 0x73, 0x70, 0x61, 0x77, 0x6e, 0x20, 0x28, 0x43, 0x2d, 0x34,
  0x29, 0x00, 0x4d, 0x41, 0x69, 0x00, 0x08, 0x00, 0x00, 0x29, 0xff, 0xff,
  0x00, 0x01, 0x00, 0x01, 0x0f, 0x0c, 0x0b, 0x0a, 0x05, 0x0f, 0x0c, 0x0b,
  0x0a, 0x05, 0x0f, 0x0c, 0x0b, 0x0a, 0x05, 0x0f, 0x0c, 0x0b, 0x0a, 0x05,
  0x0f, 0x0c, 0x0b, 0x0a, 0x05, 0x0f, 0x0c, 0x0b, 0x0a, 0x05, 0x0f, 0x0c,
  0x0b, 0x08, 0x05, 0x0f, 0x0d, 0x0b, 0x09, 0x05, 0x00, 0x01, 0x08, 0x08,
  0xff, 0x00, 0x01, 0x00, 0x05, 0x10, 0x0f, 0x0d, 0x15, 0x14, 0x0d, 0x0f,
  0x10, 0x05, 0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x00, 0x0e, 0x01,
  0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x01, 0x0f, 0x01, 0xff, 0xff, 0x00,
  0x01, 0x00, 0x01, 0x01, 0x13, 0x01, 0xff, 0xff, 0x00, 0x81, 0x00, 0x01,
  0x55, 0x55, 0xff, 0x50, 0x4e, 0x01, 0x00, 0x00
};
static const unsigned int __0b_fui_len = 140;
static const unsigned char __0c_fui[] = {
  0x46, 0x49, 0x4e, 0x53, 0xc4, 0x00, 0x39, 0x00, 0x4e, 0x41, 0x09, 0x00,
  0x74, 0x61, 0x6e, 0x6b, 0x6d, 0x6f, 0x76, 0x65, 0x00, 0x4d, 0x41, 0x92,
  0x01, 0x08, 0x00, 0x00, 0xff, 0x0b, 0xff, 0x00, 0x01, 0x00, 0x01, 0x04,
  0x05, 0x07, 0x07, 0x07, 0x06, 0x06, 0x06, 0x05, 0x05, 0x05, 0x04, 0x04,
  0x04, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x05, 0x04, 0x05, 0x05, 0x04,
  0x04, 0x04, 0x05, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x04, 0x05, 0x04,
  0x05, 0x05, 0x05, 0x04, 0x05, 0x05, 0x04, 0x05, 0x05, 0x04, 0x04, 0x05,
  0x05, 0x05, 0x04, 0x04, 0x05, 0x05, 0x04, 0x05, 0x05, 0x05, 0x04, 0x05,
  0x05, 0x04, 0x05, 0x04, 0x04, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x05,
  0x04, 0x05, 0x04, 0x05, 0x04, 0x05, 0x05, 0x05, 0x04, 0x05, 0x05, 0x05,
  0x05, 0x05, 0x05, 0x04, 0x05, 0x05, 0x04, 0x04, 0x05, 0x05, 0x04, 0x05,
  0x05, 0x04, 0x05, 0x05, 0x04, 0x04, 0x04, 0x04, 0x04, 0x05, 0x04, 0x04,
  0x04, 0x05, 0x05, 0x04, 0x04, 0x05, 0x04, 0x05, 0x04, 0x05, 0x04, 0x05,
  0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x04, 0x04, 0x04, 0x05, 0x04, 0x04,
  0x04, 0x04, 0x04, 0x04, 0x05, 0x04, 0x04, 0x05, 0x05, 0x05, 0x04, 0x04,
  0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x04, 0x05, 0x05, 0x04, 0x04, 0x04,
  0x05, 0x04, 0x05, 0x05, 0x04, 0x05, 0x05, 0x04, 0x05, 0x04, 0x04, 0x05,
  0x04, 0x04, 0x04, 0x04, 0x05, 0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05,
  0x05, 0x05, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x05, 0x04, 0x04, 0x05,
  0x05, 0x04, 0x05, 0x04, 0x04, 0x05, 0x05, 0x04, 0x05, 0x04, 0x04, 0x04,
  0x04, 0x04, 0x04, 0x05, 0x04, 0x05, 0x04, 0x05, 0x05, 0x05, 0x05, 0x04,
  0x05, 0x04, 0x05, 0x04, 0x04, 0x04, 0x05, 0x05, 0x04, 0x04, 0x04, 0x04,
  0x05, 0x05, 0x04, 0x04, 0x05, 0x05, 0x05, 0x04, 0x05, 0x05, 0x05, 0x05,
  0x05, 0x05, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x05,
  0x04, 0x04, 0x01, 0x02, 0x00, 0xff, 0x00, 0x41, 0x00, 0x01, 0xf4, 0xce,
  0x04, 0x10, 0xff, 0xff, 0x00, 0x81, 0x00, 0x01, 0xb8, 0x00, 0xd9, 0x00,
  0x02, 0x01, 0x18, 0x01, 0xd7, 0x00, 0xa3, 0x00, 0x79, 0x00, 0x59, 0x00,
  0x3e, 0x00, 0x2b, 0x00, 0x1b, 0x00, 0x0f, 0x00, 0x05, 0x00, 0xfe, 0xff,
  0xfe, 0xff, 0xf5, 0xff, 0x05, 0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01,
  0x00, 0x06, 0x02, 0x00, 0xff, 0x00, 0x01, 0x00, 0x01, 0x52, 0x00, 0x07,
  0x02, 0x00, 0xff, 0x00, 0x01, 0x00, 0x01, 0x15, 0x00, 0x0e, 0x01, 0xff,
  0xff, 0x00, 0x01, 0x00, 0x01, 0x01, 0x0f, 0x02, 0x00, 0xff, 0x00, 0x01,
  0x00, 0x01, 0x02, 0x0d, 0x10, 0x02, 0x00, 0xff, 0x00, 0x01, 0x00, 0x01,
  0x0c, 0x0a, 0x11, 0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x01, 0x12,
  0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x0b, 0x13, 0x01, 0x00, 0xff,
  0x00, 0x81, 0x00, 0x01, 0xaa, 0x5a, 0xff, 0x50, 0x4e, 0x01, 0x00, 0x00
};
static const unsigned int __0c_fui_len = 432;
static const unsigned char __0d_fui[] = {
  0x46, 0x49, 0x4e, 0x53, 0xc4, 0x00, 0x39, 0x00, 0x4e, 0x41, 0x0b, 0x00,
  0x76, 0x6f, 0x72, 0x74, 0x65, 0x78, 0x6d, 0x6f, 0x76, 0x65, 0x00, 0x4d,
  0x41, 0x68, 0x00, 0x08, 0x00, 0x00, 0x10, 0xff, 0xff, 0x00, 0x01, 0x00,
  0x01, 0x0a, 0x0a, 0x09, 0x08, 0x07, 0x07, 0x06, 0x06, 0x07, 0x08, 0x09,
  0x0b, 0x0b, 0x0c, 0x0c, 0x00, 0x01, 0x0f, 0xff, 0xff, 0x00, 0x01, 0x00,
  0x01, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
  0x0b, 0x0c, 0x0d, 0x0e, 0x05, 0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01,
  0x00, 0x06, 0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x3a, 0x07, 0x01,
  0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x00, 0x0e, 0x01, 0xff, 0xff, 0x00,
  0x01, 0x00, 0x01, 0x01, 0x11, 0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01,
  0x01, 0x12, 0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x00, 0xff, 0x50,
  0x4e, 0x01, 0x00, 0x00
};
static const unsigned int __0d_fui_len = 136;
static const unsigned char __0e_fui[] = {
  0x46, 0x49, 0x4e, 0x53, 0xc4, 0x00, 0x39, 0x00, 0x4e, 0x41, 0x0c, 0x00,
  0x76, 0x6f, 0x72, 0x74, 0x65, 0x78, 0x73, 0x68, 0x6f, 0x6f, 0x74, 0x00,
  0x4d, 0x41, 0xc1, 0x00, 0x08, 0x00, 0x00, 0x3c, 0xff, 0xff, 0x00, 0x01,
  0x00, 0x01, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
  0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
  0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0e, 0x0e, 0x0d, 0x0d, 0x0c,
  0x0c, 0x0b, 0x0b, 0x0b, 0x0a, 0x0a, 0x09, 0x09, 0x08, 0x08, 0x07, 0x07,
  0x06, 0x06, 0x05, 0x05, 0x05, 0x04, 0x04, 0x03, 0x03, 0x03, 0x02, 0x02,
  0x01, 0x00, 0x01, 0x3c, 0xff, 0xff, 0x00, 0x41, 0x00, 0x01, 0x0b, 0x09,
  0x0b, 0x07, 0x0b, 0x05, 0x0b, 0x03, 0x0b, 0x01, 0x0b, 0xfe, 0x0b, 0xfc,
  0x0b, 0xfa, 0x0b, 0x05, 0x0b, 0x04, 0x0b, 0x03, 0x0b, 0x02, 0x0b, 0x01,
  0x0b, 0x00, 0x0b, 0xff, 0x0b, 0xfe, 0x0b, 0xfd, 0x0b, 0xfc, 0x0b, 0xfb,
  0x0b, 0xfa, 0x0b, 0xf9, 0x0b, 0xf7, 0x0b, 0xf6, 0x0b, 0xf5, 0x0b, 0xf4,
  0x0b, 0xf3, 0x0b, 0xf2, 0x0b, 0xf1, 0x0b, 0xef, 0x0b, 0xee, 0x05, 0x01,
  0x00, 0xff, 0x00, 0x01, 0x00, 0x01, 0x0c, 0x06, 0x01, 0xff, 0xff, 0x00,
  0x01, 0x00, 0x01, 0xff, 0x07, 0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01,
  0x04, 0x0e, 0x01, 0x00, 0xff, 0x00, 0x01, 0x00, 0x01, 0x01, 0x11, 0x01,
  0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x01, 0x12, 0x01, 0xff, 0xff, 0x00,
  0x01, 0x00, 0x01, 0x00, 0xff, 0x50, 0x4e, 0x01, 0x00, 0x00
};
static const unsigned int __0e_fui_len = 226;
static const unsigned char __06_fui[] = {
  0x46, 0x49, 0x4e, 0x53, 0xc4, 0x00, 0x38, 0x00, 0x4d, 0x41, 0xbf, 0x00,
  0x08, 0x00, 0x00, 0x1c, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x0f, 0x0e,
  0x0c, 0x0c, 0x0c, 0x0b, 0x0b, 0x0a, 0x0a, 0x0a, 0x09, 0x09, 0x08, 0x08,
  0x08, 0x08, 0x07, 0x07, 0x07, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
  0x01, 0x00, 0x01, 0x15, 0xff, 0xff, 0x00, 0xc1, 0x00, 0x01, 0x5d, 0x00,
  0x00, 0x40, 0x63, 0x00, 0x00, 0x40, 0x35, 0x00, 0x00, 0x40, 0x35, 0x00,
  0x00, 0x40, 0x31, 0x00, 0x00, 0x40, 0x2e, 0x00, 0x00, 0x40, 0x2b, 0x00,
  0x00, 0x40, 0x29, 0x00, 0x00, 0x40, 0x27, 0x00, 0x00, 0x40, 0x26, 0x00,
  0x00, 0x40, 0x24, 0x00, 0x00, 0x40, 0x23, 0x00, 0x00, 0x40, 0x22, 0x00,
  0x00, 0x40, 0x1e, 0x00, 0x00, 0x40, 0x1d, 0x00, 0x00, 0x40, 0x1c, 0x00,
  0x00, 0x40, 0x1b, 0x00, 0x00, 0x40, 0x1a, 0x00, 0x00, 0x40, 0x1a, 0x00,
  0x00, 0x40, 0x19, 0x00, 0x00, 0x40, 0x18, 0x00, 0x00, 0x40, 0x05, 0x03,
  0x04, 0x04, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x00, 0x0e, 0x01, 0x01,
  0x04, 0x00, 0x01, 0x00, 0x01, 0x01, 0x0f, 0x03, 0x04, 0x04, 0x00, 0x01,
  0x00, 0x01, 0x0d, 0x0d, 0x01, 0x10, 0x01, 0xff, 0xff, 0x00, 0x01, 0x00,
  0x01, 0x0e, 0x13, 0x03, 0x05, 0x04, 0x00, 0xc1, 0x00, 0x01, 0xd8, 0x75,
  0x00, 0x00, 0xaa, 0xea, 0x00, 0x00, 0xaa, 0xaa, 0x00, 0x00, 0xff, 0x50,
  0x4e, 0x01, 0x00, 0x00
};
static const unsigned int __06_fui_len = 208;
static const unsigned char __0f_fui[] = {
  0x46, 0x49, 0x4e, 0x53, 0xc4, 0x00, 0x38, 0x00, 0x4d, 0x41, 0x54, 0x00,
  0x08, 0x00, 0x00, 0x0b, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x0f, 0x09,
  0x06, 0x04, 0x04, 0x03, 0x03, 0x02, 0x01, 0x01, 0x00, 0x01, 0x02, 0x00,
  0xff, 0x00, 0xc1, 0x00, 0x02, 0x3e, 0x00, 0x00, 0x40, 0x4e, 0x00, 0x00,
  0x40, 0x05, 0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x00, 0x0e, 0x01,
  0x06, 0xff, 0x00, 0x01, 0x00, 0x01, 0x01, 0x0f, 0x01, 0xff, 0xff, 0x00,
  0x01, 0x00, 0x01, 0x04, 0x10, 0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01,
  0x09, 0x13, 0x01, 0xff, 0xff, 0x00, 0x81, 0x00, 0x01, 0xc5, 0x55, 0xff,
  0x50, 0x4e, 0x01, 0x00, 0x03
};
static const unsigned int __0f_fui_len = 101;
static const unsigned char __10_fui[] = {
  0x46, 0x49, 0x4e, 0x53, 0xe2, 0x00, 0x38, 0x00, 0x4d, 0x41, 0x94, 0x00,
  0x08, 0x00, 0x00, 0x10, 0xff, 0xff, 0x00, 0x03, 0x00, 0x01, 0x00, 0x0f,
  0x06, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x04, 0x4c, 0x48, 0xff, 0x01, 0x41, 0x00, 0x01, 0xfd, 0xfc,
  0xfb, 0xfb, 0xfb, 0xfa, 0xfa, 0xfa, 0xfa, 0xf8, 0xf7, 0xf6, 0xf6, 0xf5,
  0xf5, 0xf4, 0xf3, 0xf3, 0xf2, 0xf2, 0xf3, 0xf2, 0xf1, 0xf1, 0xf0, 0xf0,
  0xef, 0xef, 0xee, 0xee, 0xee, 0xed, 0xed, 0xed, 0xec, 0xec, 0xec, 0xeb,
  0xeb, 0xeb, 0xeb, 0xeb, 0xeb, 0xea, 0xea, 0xea, 0xea, 0xea, 0xea, 0xea,
  0xeb, 0xeb, 0xeb, 0xeb, 0xeb, 0xec, 0xec, 0xec, 0xed, 0xed, 0xed, 0xed,
  0xee, 0xee, 0xef, 0xef, 0xf0, 0xf0, 0xf1, 0xf1, 0xf2, 0xf2, 0xf2, 0xf3,
  0xf3, 0xf3, 0x05, 0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x00, 0x0e,
  0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x01, 0x0f, 0x01, 0xff, 0xff,
  0x00, 0x01, 0x00, 0x01, 0x01, 0x13, 0x01, 0xff, 0xff, 0x00, 0x81, 0x00,
  0x01, 0x55, 0x55, 0xff, 0x50, 0x4e, 0x01, 0x00, 0x00
};
static const unsigned int __10_fui_len = 165;
static const unsigned char __11_fui[] = {
  0x46, 0x49, 0x4e, 0x53, 0xe2, 0x00, 0x38, 0x00, 0x4d, 0x41, 0x94, 0x00,
  0x08, 0x00, 0x00, 0x10, 0xff, 0xff, 0x00, 0x03, 0x00, 0x01, 0x00, 0x0f,
  0x06, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x04, 0x4c, 0x48, 0xff, 0x01, 0x41, 0x00, 0x01, 0xd7, 0xfd,
  0xfb, 0xfb, 0xfb, 0xfa, 0xfa, 0xfa, 0xfa, 0xf8, 0xf7, 0xf6, 0xf6, 0xf5,
  0xf5, 0xf4, 0xf3, 0xf3, 0xf2, 0xf2, 0xf3, 0xf2, 0xf1, 0xf1, 0xf0, 0xf0,
  0xef, 0xef, 0xee, 0xee, 0xee, 0xed, 0xed, 0xed, 0xec, 0xec, 0xec, 0xeb,
  0xeb, 0xeb, 0xeb, 0xeb, 0xeb, 0xea, 0xea, 0xea, 0xea, 0xea, 0xea, 0xea,
  0xeb, 0xeb, 0xeb, 0xeb, 0xeb, 0xec, 0xec, 0xec, 0xed, 0xed, 0xed, 0xed,
  0xee, 0xee, 0xef, 0xef, 0xf0, 0xf0, 0xf1, 0xf1, 0xf2, 0xf2, 0xf2, 0xf3,
  0xf3, 0xf3, 0x05, 0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x00, 0x0e,
  0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x01, 0x0f, 0x01, 0xff, 0xff,
  0x00, 0x01, 0x00, 0x01, 0x01, 0x13, 0x01, 0xff, 0xff, 0x00, 0x81, 0x00,
  0x01, 0x55, 0x55, 0xff, 0x50, 0x4e, 0x01, 0x00, 0x00
};
static const unsigned int __11_fui_len = 165;
static const unsigned char __12_fui[] = {
  0x46, 0x49, 0x4e, 0x53, 0xe2, 0x00, 0x38, 0x00, 0x4d, 0x41, 0xd0, 0x00,
  0x08, 0x00, 0x00, 0x40, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x0f, 0x0f,
  0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0d, 0x0d, 0x0e, 0x0d, 0x0d,
  0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0b, 0x0b, 0x0b, 0x0b, 0x09, 0x09, 0x07,
  0x07, 0x07, 0x06, 0x06, 0x05, 0x05, 0x05, 0x04, 0x04, 0x04, 0x04, 0x04,
  0x04, 0x04, 0x04, 0x04, 0x03, 0x03, 0x03, 0x03, 0x03, 0x02, 0x02, 0x02,
  0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
  0x00, 0x00, 0x01, 0x3c, 0x06, 0xff, 0x00, 0x41, 0x00, 0x01, 0x28, 0xfe,
  0xe8, 0xf6, 0x2c, 0x21, 0x1d, 0x05, 0x27, 0x04, 0x0b, 0x0b, 0x27, 0x28,
  0x02, 0x0d, 0x0a, 0x04, 0x03, 0x2c, 0x2c, 0x0c, 0xfc, 0x06, 0x1e, 0x2c,
  0xfe, 0xfe, 0x2c, 0x23, 0x2c, 0x24, 0x08, 0x0b, 0x2b, 0x1e, 0x2b, 0xfe,
  0x01, 0x07, 0x07, 0x0a, 0x2b, 0xff, 0x1c, 0x2a, 0x21, 0x16, 0x20, 0x16,
  0x16, 0x1d, 0x1d, 0x1c, 0x09, 0x02, 0x05, 0xfd, 0x09, 0x29, 0x05, 0x06,
  0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01,
  0x0e, 0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x01, 0x0f, 0x03, 0xff,
  0xff, 0x00, 0x01, 0x00, 0x01, 0x0a, 0x01, 0x06, 0x10, 0x03, 0xff, 0xff,
  0x00, 0x01, 0x00, 0x01, 0x05, 0x04, 0x0c, 0x13, 0x03, 0xff, 0xff, 0x00,
  0xc1, 0x00, 0x01, 0xad, 0x3a, 0x00, 0x00, 0xaa, 0xaa, 0x00, 0x00, 0x20,
  0x21, 0x00, 0x00, 0xff, 0x50, 0x4e, 0x01, 0x00, 0x00
};
static const unsigned int __12_fui_len = 225;
static const unsigned char __13_fui[] = {
  0x46, 0x49, 0x4e, 0x53, 0xe2, 0x00, 0x39, 0x00, 0x4d, 0x41, 0x95, 0x00,
  0x08, 0x00, 0x00, 0x52, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x0f, 0x0e,
  0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08, 0x00, 0x0f, 0x0e, 0x0d, 0x0c, 0x0b,
  0x0a, 0x09, 0x08, 0x00, 0x0f, 0x0e, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d,
  0x0d, 0x0d, 0x0c, 0x0c, 0x0c, 0x0c, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0a,
  0x0a, 0x0a, 0x0a, 0x0a, 0x09, 0x09, 0x09, 0x09, 0x08, 0x08, 0x08, 0x08,
  0x07, 0x07, 0x07, 0x07, 0x07, 0x06, 0x06, 0x06, 0x06, 0x06, 0x05, 0x05,
  0x05, 0x05, 0x05, 0x04, 0x04, 0x04, 0x04, 0x04, 0x03, 0x03, 0x02, 0x02,
  0x02, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x0b, 0x06, 0xff,
  0x00, 0x01, 0x00, 0x03, 0x00, 0x04, 0x07, 0x02, 0x06, 0x09, 0x04, 0x08,
  0x0b, 0x10, 0x0b, 0x05, 0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x00,
  0x0e, 0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x01, 0x0f, 0x01, 0xff,
  0xff, 0x00, 0x01, 0x00, 0x01, 0x01, 0x13, 0x01, 0xff, 0xff, 0x00, 0x81,
  0x00, 0x01, 0x55, 0x55, 0xff, 0x50, 0x4e, 0x01, 0x00, 0x00
};
static const unsigned int __13_fui_len = 166;
static const unsigned char __14_fui[] = {
  0x46, 0x49, 0x4e, 0x53, 0xe2, 0x00, 0x38, 0x00, 0x4d, 0x41, 0x4f, 0x00,
  0x08, 0x00, 0x00, 0x1f, 0xff, 0xff, 0x00, 0x01, 0x00, 0x02, 0x0f, 0x0f,
  0x00, 0x00, 0x0f, 0x0f, 0x00, 0x00, 0x0f, 0x0f, 0x00, 0x00, 0x0f, 0x0f,
  0x00, 0x00, 0x0f, 0x0f, 0x00, 0x00, 0x0f, 0x0f, 0x00, 0x00, 0x0f, 0x0f,
  0x00, 0x00, 0x0f, 0x0f, 0x00, 0x05, 0x01, 0xff, 0xff, 0x00, 0x01, 0x00,
  0x01, 0x00, 0x0e, 0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x01, 0x0f,
  0x01, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x01, 0x13, 0x01, 0xff, 0xff,
  0x00, 0x81, 0x00, 0x01, 0x55, 0x55, 0xff, 0x50, 0x4e, 0x01, 0x00, 0x00
};
static const unsigned int __14_fui_len = 96;

#define LOAD_INS(x,y) { \
  DivInstrument* newIns=new DivInstrument; \
  SafeReader reader(x,y); \
  newIns->readInsData(reader,DIV_ENGINE_VERSION,&e->song); \
  newIns->std.volMacro.speed*=macroMul; \
  newIns->std.arpMacro.speed*=macroMul; \
  newIns->std.dutyMacro.speed*=macroMul; \
  newIns->std.waveMacro.speed*=macroMul; \
  newIns->std.pitchMacro.speed*=macroMul; \
  newIns->std.phaseResetMacro.speed*=macroMul; \
  newIns->std.ex1Macro.speed*=macroMul; \
  newIns->std.ex2Macro.speed*=macroMul; \
  newIns->std.ex3Macro.speed*=macroMul; \
  newIns->std.ex4Macro.speed*=macroMul; \
  newIns->std.ex5Macro.speed*=macroMul; \
  newIns->std.ex6Macro.speed*=macroMul; \
  newIns->std.ex7Macro.speed*=macroMul; \
  newIns->std.ex8Macro.speed*=macroMul; \
  e->addInstrumentPtr(newIns); \
}

void FurnaceCV::loadInstruments() {
  logV("loadInstruments");
  fxInsBase=e->song.ins.size();
  fxChanBase=e->getTotalChannelCount();

  int macroMul=(int)((e->curSubSong->hz+20.0)/60.0);

  int sys=e->song.systemLen;
  if (e->addSystem(DIV_SYSTEM_POWERNOISE)) {
    if (e->song.masterVol<0.1) {
      e->song.systemVol[sys]=12.0;
    } else {
      e->song.systemVol[sys]=1.2/e->song.masterVol;
    }
  }
  
  
  LOAD_INS(__00_fui,__00_fui_len);
  LOAD_INS(__01_fui,__01_fui_len);
  LOAD_INS(__02_fui,__02_fui_len);
  LOAD_INS(__03_fui,__03_fui_len);
  LOAD_INS(__04_fui,__04_fui_len);
  LOAD_INS(__05_fui,__05_fui_len);
  LOAD_INS(__06_fui,__06_fui_len);
  LOAD_INS(__07_fui,__07_fui_len);
  LOAD_INS(__08_fui,__08_fui_len);
  LOAD_INS(__09_fui,__09_fui_len);
  LOAD_INS(__0a_fui,__0a_fui_len);
  LOAD_INS(__0b_fui,__0b_fui_len);
  LOAD_INS(__0c_fui,__0c_fui_len);
  LOAD_INS(__0d_fui,__0d_fui_len);
  LOAD_INS(__0e_fui,__0e_fui_len);
  LOAD_INS(__0f_fui,__0f_fui_len);
  LOAD_INS(__10_fui,__10_fui_len);
  LOAD_INS(__11_fui,__11_fui_len);
  LOAD_INS(__12_fui,__12_fui_len);
  LOAD_INS(__13_fui,__13_fui_len);
  LOAD_INS(__14_fui,__14_fui_len);
}

// FurnaceCVMine IMPLEMENTATION

void FurnaceCVMine::collision(FurnaceCVObject* other) {
  if (other->type==CV_BULLET || other->type==CV_PLAYER) {
    dead=true;
    cv->createObject<FurnaceCVExplMedium>(x-4,y-4);
    cv->soundEffect(SE_EXPL1);
    cv->addScore(rand()%50);
  }
}

// FurnaceCVPowerupP IMPLEMENTATION

void FurnaceCVPowerupP::collision(FurnaceCVObject* other) {
  if (other->type==CV_PLAYER) {
    dead=true;
    if (cv->shotType) {
      cv->weaponStack.push_front(cv->shotType);
    }
    cv->shotType=1;
    cv->soundEffect(SE_PICKUP2);
    cv->addScore(200);
  }
}

void FurnaceCVPowerupP::tick() {
  if (--life==0) dead=true;

  if (life>64 || (life&1)) {
    spriteDef[0]=0x490;
    spriteDef[1]=0x491;
    spriteDef[2]=0x4b0;
    spriteDef[3]=0x4b1;
  } else {
    spriteDef[0]=0;
    spriteDef[1]=0;
    spriteDef[2]=0;
    spriteDef[3]=0;
  }
}

// FurnaceCVPowerupS IMPLEMENTATION

void FurnaceCVPowerupS::collision(FurnaceCVObject* other) {
  if (other->type==CV_PLAYER) {
    dead=true;
    if (cv->shotType) {
      cv->weaponStack.push_front(cv->shotType);
    }
    cv->shotType=2;
    cv->soundEffect(SE_PICKUP2);
    cv->addScore(200);
  }
}

void FurnaceCVPowerupS::tick() {
  if (--life==0) dead=true;

  if (life>64 || (life&1)) {
    spriteDef[0]=0x492;
    spriteDef[1]=0x493;
    spriteDef[2]=0x4b2;
    spriteDef[3]=0x4b3;
  } else {
    spriteDef[0]=0;
    spriteDef[1]=0;
    spriteDef[2]=0;
    spriteDef[3]=0;
  }
}

// FurnaceCVExtraLife IMPLEMENTATION

void FurnaceCVExtraLife::collision(FurnaceCVObject* other) {
  if (other->type==CV_PLAYER) {
    dead=true;
    cv->soundEffect(SE_PICKUP1);
    cv->lives++;
  }
}

void FurnaceCVExtraLife::tick() {
  if (--life==0) {
    dead=true;
    cv->lifeBank++;
  }

  if (life>64 || (life&1)) {
    spriteDef[0]=0x0c;
    spriteDef[1]=0x0d;
    spriteDef[2]=0x2c;
    spriteDef[3]=0x2d;
  } else {
    spriteDef[0]=0;
    spriteDef[1]=0;
    spriteDef[2]=0;
    spriteDef[3]=0;
  }
}

// FurnaceCVSpecial IMPLEMENTATION

void FurnaceCVSpecial::collision(FurnaceCVObject* other) {
  if (other->type==CV_PLAYER) {
    dead=true;
    cv->soundEffect(SE_PICKUP2);
    cv->specialType=specialType;
  }
}

void FurnaceCVSpecial::tick() {
  if (--life==0) dead=true;

  if (life>64 || (life&1)) {
    spriteDef[0]=0x4dc+(((specialType-1)&1)<<1)+(((specialType-1)>>1)<<6);
    spriteDef[1]=0x4dd+(((specialType-1)&1)<<1)+(((specialType-1)>>1)<<6);
    spriteDef[2]=0x4fc+(((specialType-1)&1)<<1)+(((specialType-1)>>1)<<6);
    spriteDef[3]=0x4fd+(((specialType-1)&1)<<1)+(((specialType-1)>>1)<<6);
  } else {
    spriteDef[0]=0;
    spriteDef[1]=0;
    spriteDef[2]=0;
    spriteDef[3]=0;
  }
}


// FurnaceCVModI IMPLEMENTATION

void FurnaceCVModI::collision(FurnaceCVObject* other) {
  if (other->type==CV_PLAYER) {
    dead=true;
    cv->soundEffect(SE_PICKUP1);
    cv->addScore(200);
    ((FurnaceCVPlayer*)other)->invincible=600;
  }
}

void FurnaceCVModI::tick() {
  if (--life==0) dead=true;

  if (life>64 || (life&1)) {
    spriteDef[0]=0x41c;
    spriteDef[1]=0x41d;
    spriteDef[2]=0x43c;
    spriteDef[3]=0x43d;
  } else {
    spriteDef[0]=0;
    spriteDef[1]=0;
    spriteDef[2]=0;
    spriteDef[3]=0;
  }
}

// FurnaceCVModS IMPLEMENTATION

void FurnaceCVModS::collision(FurnaceCVObject* other) {
  if (other->type==CV_PLAYER) {
    dead=true;
    cv->soundEffect(SE_PICKUP3);
    cv->addScore(200);
    cv->speedTicks=900;
    cv->e->setSongRate(cv->origSongRate*1.5);
  }
}

void FurnaceCVModS::tick() {
  if (--life==0) dead=true;

  if (life>64 || (life&1)) {
    spriteDef[0]=0x41e;
    spriteDef[1]=0x41f;
    spriteDef[2]=0x43e;
    spriteDef[3]=0x43f;
  } else {
    spriteDef[0]=0;
    spriteDef[1]=0;
    spriteDef[2]=0;
    spriteDef[3]=0;
  }
}

// FurnaceCVEnemyVortex IMPLEMENTATION

void FurnaceCVEnemyVortex::collision(FurnaceCVObject* other) {
  if (other->type==CV_BULLET || other->type==CV_PLAYER) {
    dead=true;
    if ((rand()%2)==0) {
      switch (rand()%14) {
        case 0: // extra life
          cv->createObject<FurnaceCVExtraLife>(x,y);
          break;
        case 1: case 2: case 3: case 4: // powerup
          cv->createObject<FurnaceCVPowerupP>(x,y);
          break;
        case 5: case 6: case 7: case 8: case 9: // powerup
          cv->createObject<FurnaceCVPowerupS>(x,y);
          break;
        case 10: case 11: // special
          cv->createObject<FurnaceCVSpecial>(x,y);
          break;
        case 12: // mod
          cv->createObject<FurnaceCVModS>(x,y);
          break;
        case 13: // mod
          cv->createObject<FurnaceCVModI>(x,y);
          break;
      }
    }
    cv->soundEffect(SE_EXPL1);
    cv->createObject<FurnaceCVFurBallMedium>(x-4,y-4);
    cv->addScore(1000);
  }
}

void FurnaceCVEnemyVortex::tick() {
  if (frozen>0) {
    if (--frozen>0) return;
  }

  x+=speedX;
  y+=speedY;
  animFrame+=0x08;

  if (--nextTime==0) {
    nextTime=10+(rand()%160);
    speedX=(rand()%5)-2;
    speedY=(rand()%5)-2;
    cv->soundEffect(SE_VORTEXMOVE);
  }

  if (--shootTime==0) {
    shootTime=6+(rand()%100);
    cv->soundEffect(SE_VORTEXSHOOT);
    for (int i=0; i<6+cv->stage; i++) {
      float fraction=(float)i/(float)(5+cv->stage);
      float xs=cos(fraction*M_PI*2.0)*28;
      float ys=sin(fraction*M_PI*2.0)*28;
      FurnaceCVEnemyBullet* b=cv->createObject<FurnaceCVEnemyBullet>(x+4,y+4);
      b->setType(1);
      b->speedX=xs;
      b->speedY=ys;
    }
  }

  if (HITS_BORDER) {
    CONFINE_TO_BORDER;
  }

  spriteDef[0]=0x480+((animFrame>>5)<<1);
  spriteDef[1]=0x481+((animFrame>>5)<<1);
  spriteDef[2]=0x4a0+((animFrame>>5)<<1);
  spriteDef[3]=0x4a1+((animFrame>>5)<<1);
}

// FurnaceCVEnemyPlane IMPLEMENTATION

void FurnaceCVEnemyPlane::collision(FurnaceCVObject* other) {
  // ignore completely
}

void FurnaceCVEnemyPlane::tick() {
  switch (orient) {
    case 0:
      x+=speed;
      if (x>cv->stageWidthPx+32) dead=true;
      break;
    case 1:
      y-=speed;
      if (y<-160) dead=true;
      break;
    case 2:
      x-=speed;
      if (x<-160) dead=true;
      break;
    case 3:
      y+=speed;
      if (y>cv->stageHeightPx+32) dead=true;
      break;
    default:
      dead=true;
      logE("plane with invalid orientation %d",orient);
      break;
  }

  if (notifyPlayer) {
    for (FurnaceCVObject* i: cv->sprite) {
      if (i->type==CV_PLAYER) {
        if (abs((x+(spriteWidth<<2))-(i->x+(i->spriteWidth<<2)))<((orient&1)?80:180)) {
          if (abs((y+(spriteHeight<<2))-(i->y+(i->spriteHeight<<2)))<((orient&1)?180:80)) {
            cv->soundEffect(SE_PLANE1);
            cv->soundEffect(SE_PLANE2);
            shootTime=(50-speed*4)+(rand()%20);
            notifyPlayer=false;
          }
        }
        break;
      }
    }
  } else {
    if (--shootTime<=0) {
      shootTime=28-(speed*2);
      cv->soundEffect(SE_EXPL2);
      cv->createObject<FurnaceCVFurBallLarge>(x+(spriteWidth<<2)-16,y+(spriteHeight<<2)-16);
      for (int i=0; i<14; i++) {
        float fraction=(float)i/13.0f;
        float xs=cos(fraction*M_PI*2.0)*28;
        float ys=sin(fraction*M_PI*2.0)*28;
        FurnaceCVEnemyBullet* b=cv->createObject<FurnaceCVEnemyBullet>(x+(spriteWidth<<2)-4,y+(spriteHeight<<2)-4);
        b->speedX=xs;
        b->speedY=ys;
      }
    }
  }
}
