#include "discordRPC.h"
#include "discordAppID.h"
#include "../ta-log.h"
#include "../engine/engine.h"
#include "../engine/song.h"
#include "../engine/sysDef.h"

#ifdef HAVE_DISCORD_RPC
#include <discord_rpc.h>
#endif

#include <chrono>
#include <cstring>
#include <cstdio>

#ifdef HAVE_DISCORD_RPC

static constexpr int64_t MIN_INTERVAL_MS = 2000;

static const char* LARGE_KEY        = "furnace_logo";
static const char* LARGE_TEXT       = "Furnace — multi-system chiptune tracker";
static const char* CHIP_CUSTOM      = "chip_custom";

static void onDiscordReady(const DiscordUser* user) {
  logI("Discord RPC: ready (user: %s)",user && user->username ? user->username : "?");
}
static void onDiscordDisconnect(int errCode, const char* msg) {
  logW("Discord RPC: disconnected (%d) %s",errCode,msg ? msg : "");
}
static void onDiscordError(int errCode, const char* msg) {
  logW("Discord RPC: error (%d) %s",errCode,msg ? msg : "");
}

// ---------------------------------------------------------------------------
// Map e->song.system[] (DivSystem array) to a Discord Art Asset key.
//
// We derive the icon from the chip enum values directly rather than from
// systemName so the choice is robust against the user editing the system
// name string. Multi-chip combos are matched before single-chip fallbacks
// because the user's mental model of "what console is this?" is set by the
// combo (Genesis = YM2612 + SMS), not by any one chip in isolation.
// ---------------------------------------------------------------------------
static const char* chipIconForSystems(const DivSystem* sys, int n) {
  if (n<=0 || sys==NULL) return CHIP_CUSTOM;

  // build a quick flag set
  bool hasYM2612=false, hasSMS=false, hasOPLL=false;
  bool hasNES=false,    hasFDS=false;
  bool hasAY=false,     hasSCC=false;
  bool hasYM2151=false, hasSegaPCM=false;
  bool hasYM2610=false, hasYM2608=false;
  bool hasMSM6258=false, hasMSM6295=false, hasQSound=false;
  bool hasC64=false,    hasOPL=false;
  for (int i=0; i<n; i++) {
    switch (sys[i]) {
      case DIV_SYSTEM_YM2612: case DIV_SYSTEM_YM2612_EXT:
      case DIV_SYSTEM_YM2612_DUALPCM: case DIV_SYSTEM_YM2612_DUALPCM_EXT:
      case DIV_SYSTEM_YM2612_CSM:
        hasYM2612=true; break;
      case DIV_SYSTEM_SMS:
        hasSMS=true; break;
      case DIV_SYSTEM_OPLL: case DIV_SYSTEM_OPLL_DRUMS:
        hasOPLL=true; break;
      case DIV_SYSTEM_NES: case DIV_SYSTEM_5E01:
        hasNES=true; break;
      case DIV_SYSTEM_FDS:
        hasFDS=true; break;
      case DIV_SYSTEM_AY8910: case DIV_SYSTEM_AY8930:
        hasAY=true; break;
      case DIV_SYSTEM_SCC: case DIV_SYSTEM_SCC_PLUS:
        hasSCC=true; break;
      case DIV_SYSTEM_YM2151:
        hasYM2151=true; break;
      case DIV_SYSTEM_SEGAPCM: case DIV_SYSTEM_SEGAPCM_COMPAT:
        hasSegaPCM=true; break;
      case DIV_SYSTEM_YM2610B: case DIV_SYSTEM_YM2610B_EXT:
      case DIV_SYSTEM_YM2610_FULL: case DIV_SYSTEM_YM2610_FULL_EXT:
      case DIV_SYSTEM_YM2610_CRAP: case DIV_SYSTEM_YM2610_CRAP_EXT:
      case DIV_SYSTEM_YM2610_CSM: case DIV_SYSTEM_YM2610B_CSM:
        hasYM2610=true; break;
      case DIV_SYSTEM_YM2608: case DIV_SYSTEM_YM2608_EXT:
      case DIV_SYSTEM_YM2608_CSM:
        hasYM2608=true; break;
      case DIV_SYSTEM_MSM6258: hasMSM6258=true; break;
      case DIV_SYSTEM_MSM6295: hasMSM6295=true; break;
      case DIV_SYSTEM_QSOUND:  hasQSound=true; break;
      case DIV_SYSTEM_C64_6581: case DIV_SYSTEM_C64_8580:
      case DIV_SYSTEM_C64_PCM:
      case DIV_SYSTEM_SID2: case DIV_SYSTEM_SID3:
        hasC64=true; break;
      case DIV_SYSTEM_OPL: case DIV_SYSTEM_OPL2: case DIV_SYSTEM_OPL3:
      case DIV_SYSTEM_OPL4: case DIV_SYSTEM_OPL_DRUMS: case DIV_SYSTEM_OPL2_DRUMS:
      case DIV_SYSTEM_OPL3_DRUMS: case DIV_SYSTEM_OPL4_DRUMS:
      case DIV_SYSTEM_Y8950: case DIV_SYSTEM_Y8950_DRUMS:
        hasOPL=true; break;
      default: break;
    }
  }

  // ---- multi-chip combos (most-specific first) -----------------------------
  if (hasYM2612 && hasSMS)              return "chip_genesis";   // Sega Genesis / Mega Drive
  if (hasSMS && hasOPLL)                return "chip_sms";       // Master System + FM expansion
  if (hasNES && hasFDS)                 return "chip_fds";       // Famicom Disk System
  if (hasYM2610)                        return "chip_neogeo";    // Neo Geo
  if (hasYM2608)                        return "chip_pc98";      // PC-98
  if (hasYM2151 && hasMSM6258)          return "chip_x68000";    // Sharp X68000
  if (hasYM2151 && (hasSegaPCM || hasMSM6295 || hasQSound))
                                        return "chip_arcade";    // arcade FM+sample combos
  if (hasC64)                           return "chip_c64";       // C64 variants (SID2/SID3/PCM)

  // ---- single-chip fallback ------------------------------------------------
  for (int i=0; i<n; i++) {
    switch (sys[i]) {
      case DIV_SYSTEM_GB:                  return "chip_gameboy";
      case DIV_SYSTEM_GBA_DMA:
      case DIV_SYSTEM_GBA_MINMOD:          return "chip_gba";
      case DIV_SYSTEM_VBOY:                return "chip_vboy";
      case DIV_SYSTEM_NES: case DIV_SYSTEM_5E01:
                                           return "chip_nes";
      case DIV_SYSTEM_PCE:                 return "chip_pce";
      case DIV_SYSTEM_SNES:                return "chip_snes";
      case DIV_SYSTEM_AMIGA:               return "chip_amiga";
      case DIV_SYSTEM_TIA:                 return "chip_tia";
      case DIV_SYSTEM_POKEY:               return "chip_pokey";
      case DIV_SYSTEM_AY8910: case DIV_SYSTEM_AY8930:
                                           return "chip_ay";
      case DIV_SYSTEM_SFX_BEEPER:
      case DIV_SYSTEM_SFX_BEEPER_QUADTONE: return "chip_zx";
      case DIV_SYSTEM_SMS: case DIV_SYSTEM_T6W28:
                                           return "chip_sms";
      case DIV_SYSTEM_SWAN:                return "chip_wonderswan";
      case DIV_SYSTEM_DAVE:                return "chip_dave";
      case DIV_SYSTEM_SCC: case DIV_SYSTEM_SCC_PLUS:
                                           return "chip_msx";
      case DIV_SYSTEM_YM2151: case DIV_SYSTEM_OPZ:
      case DIV_SYSTEM_YM2203: case DIV_SYSTEM_YM2203_EXT: case DIV_SYSTEM_YM2203_CSM:
                                           return "chip_yamaha";
      default: break;
    }
  }
  // pure FM that didn't match a console preset
  if (hasOPL || hasOPLL || hasYM2151)     return "chip_yamaha";
  if (hasAY)                              return "chip_ay";
  return CHIP_CUSTOM;
}

// ---------------------------------------------------------------------------
// Verb selection from engine state.
// ---------------------------------------------------------------------------
enum Verb { VERB_IDLING, VERB_EDITING, VERB_PLAYING };

static Verb verbForEngine(DivEngine* e) {
  if (e->isPlaying()) return VERB_PLAYING;
  // "Idling" = default empty workspace (no song name, no instruments,
  // wavetables, or samples). As soon as the user creates any asset or names
  // the song we consider them Editing.
  const DivSong& s=e->song;
  if (s.name.empty() && s.ins.empty() && s.wave.empty() && s.sample.empty()) {
    return VERB_IDLING;
  }
  return VERB_EDITING;
}

#endif // HAVE_DISCORD_RPC

// ---------------------------------------------------------------------------

int64_t FurnaceDiscordRPC::nowMs() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

FurnaceDiscordRPC::FurnaceDiscordRPC():
  level(LEVEL_OFF),
  active(false),
  dirty(false),
  lastFlushMs(0),
  smallIconKey(NULL),
  editStartTime((int64_t)time(NULL)) {}

FurnaceDiscordRPC::~FurnaceDiscordRPC() {
  stop();
}

void FurnaceDiscordRPC::start(Level l) {
  level=l;
  if (level==LEVEL_OFF || active) return;
#ifdef HAVE_DISCORD_RPC
  DiscordEventHandlers handlers;
  memset(&handlers,0,sizeof(handlers));
  handlers.ready=onDiscordReady;
  handlers.disconnected=onDiscordDisconnect;
  handlers.errored=onDiscordError;
  Discord_Initialize(DISCORD_APP_ID,&handlers,1,NULL);
  active=true;
  dirty=true;
  lastFlushMs=0;
  logI("Discord RPC: started (app id %s, level %d)",DISCORD_APP_ID,(int)level);
#endif
}

void FurnaceDiscordRPC::stop() {
  if (!active) return;
#ifdef HAVE_DISCORD_RPC
  Discord_ClearPresence();
  Discord_Shutdown();
#endif
  active=false;
  logI("Discord RPC: stopped");
}

void FurnaceDiscordRPC::setLevel(Level l) {
  if (l==level) return;
  if (l==LEVEL_OFF) { stop(); level=l; return; }
  if (!active) { start(l); } else { level=l; dirty=true; }
}

void FurnaceDiscordRPC::setActivity(DivEngine* e) {
#ifdef HAVE_DISCORD_RPC
  if (!active || e==NULL) return;
  DivSong& s=e->song;

  // identity: name + chip combo. resets the sticky edit timer when changed.
  std::string identity=s.name;
  identity+='\x01';
  for (int i=0; i<s.systemLen; i++) {
    char b[8]; snprintf(b,sizeof(b),"%d,",(int)s.system[i]);
    identity+=b;
  }
  if (identity!=lastSongIdentity) {
    editStartTime=(int64_t)time(NULL);
    lastSongIdentity=identity;
    dirty=true;
  }

  Verb v=verbForEngine(e);
  const char* verbStr=(v==VERB_PLAYING)?"Playing":(v==VERB_IDLING?"Idling":"Editing");

  // details line
  std::string newDetails;
  if (v==VERB_IDLING) {
    newDetails="Idling";
  } else {
    newDetails=verbStr;
    newDetails+=' ';
    if (s.name.empty()) {
      newDetails+="untitled";
    } else {
      newDetails+='"';
      newDetails+=s.name.substr(0,100);
      newDetails+='"';
    }
  }

  // state line: song scope, or a friendly subtitle when idling
  std::string newState;
  if (v==VERB_IDLING) {
    newState="Furnace";
  } else {
    DivSubSong* sub=e->curSubSong;
    int orders=sub?sub->ordersLen:0;
    int patLen=sub?sub->patLen:0;
    int chans=s.chans;
    char buf[96];
    snprintf(buf,sizeof(buf),
      "%d order%s \xe2\x80\xa2 %d row%s \xe2\x80\xa2 %d channel%s",
      orders, orders==1?"":"s",
      patLen, patLen==1?"":"s",
      chans,  chans==1?"":"s");
    newState=buf;
  }

  // small image: chip badge derived from the actual DivSystem array
  const char* newIcon=chipIconForSystems(s.system,s.systemLen);
  // small image hover: the system display string (auto-generated by Furnace)
  std::string newSmallText=s.systemName.empty()?std::string("Furnace"):s.systemName;

  if (newDetails!=details || newState!=state ||
      newIcon!=smallIconKey || newSmallText!=smallText) {
    details=newDetails;
    state=newState;
    smallIconKey=newIcon;
    smallText=newSmallText;
    dirty=true;
  }
#else
  (void)e;
#endif
}

void FurnaceDiscordRPC::tick() {
  if (!active) return;
#ifdef HAVE_DISCORD_RPC
  Discord_RunCallbacks();
  if (dirty && nowMs()-lastFlushMs >= MIN_INTERVAL_MS) flush();
#endif
}

#ifdef HAVE_DISCORD_RPC
void FurnaceDiscordRPC::flush() {
  DiscordRichPresence p;
  memset(&p,0,sizeof(p));
  p.instance=0;

  std::string outDetails, outState;
  if (level==LEVEL_MINIMAL) {
    outDetails="Furnace";
    outState="multi-system chiptune tracker";
    p.largeImageKey=LARGE_KEY;
    p.largeImageText=LARGE_TEXT;
  } else {
    outDetails=details;
    outState=state;
    p.startTimestamp=editStartTime;
    p.largeImageKey=LARGE_KEY;
    p.largeImageText=LARGE_TEXT;
    p.smallImageKey=smallIconKey?smallIconKey:CHIP_CUSTOM;
    p.smallImageText=smallText.empty()?"Furnace":smallText.c_str();
  }
  p.details=outDetails.c_str();
  p.state=outState.c_str();
  Discord_UpdatePresence(&p);

  dirty=false;
  lastFlushMs=nowMs();
}
#else
void FurnaceDiscordRPC::flush() {}
#endif
