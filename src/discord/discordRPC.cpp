#include "discordRPC.h"
#include "discordAppID.h"
#include "../ta-log.h"

#ifdef HAVE_DISCORD_RPC
#include <discord_rpc.h>
#include <chrono>
#include <cstring>
#include <cctype>

// Discord guidance is ~5/s; we only send on real state change anyway.
static constexpr int64_t MIN_INTERVAL_MS = 2000;

static const char* LARGE_KEY        = "furnace_logo";
static const char* LARGE_TEXT       = "Furnace — multi-system chiptune tracker";
static const char* CHIP_CUSTOM_KEY  = "chip_custom";

static void onDiscordReady(const DiscordUser* user) {
  logI("Discord RPC: ready (user: %s)",user && user->username ? user->username : "?");
}
static void onDiscordDisconnect(int errCode, const char* msg) {
  logW("Discord RPC: disconnected (%d) %s",errCode,msg ? msg : "");
}
static void onDiscordError(int errCode, const char* msg) {
  logW("Discord RPC: error (%d) %s",errCode,msg ? msg : "");
}

// Map a Furnace system name to one of the chip_<key> asset names uploaded to
// Discord. Falls back to chip_custom. Matching is case-insensitive substring.
//
// Order matters: more specific patterns must come before subset ones
// (e.g. "Master System" before "System", "Game Boy Advance" before "Game Boy").
static const char* chipIconKey(const std::string& systemName) {
  if (systemName.empty()) return CHIP_CUSTOM_KEY;
  std::string s;
  s.reserve(systemName.size());
  for (char c : systemName) s+=(char)std::tolower((unsigned char)c);
  auto has=[&](const char* needle)->bool {
    return s.find(needle)!=std::string::npos;
  };

  if (has("genesis") || has("mega drive"))          return "chip_genesis";
  if (has("master system"))                          return "chip_sms";
  if (has("game boy advance") || has("gba"))         return "chip_gba";
  if (has("virtual boy") || has("vboy"))             return "chip_vboy";
  if (has("game boy"))                               return "chip_gameboy";
  if (has("disk system") || has("fds"))              return "chip_fds";
  if (has("famicom") || has("nes") || has("2a03"))   return "chip_nes";
  if (has("pc engine") || has("turbografx"))         return "chip_pce";
  if (has("super nes") || has("snes") || has("spc")) return "chip_snes";
  if (has("c64") || has("commodore 64") || has("sid")) return "chip_c64";
  if (has("amiga") || has("paula"))                  return "chip_amiga";
  if (has("atari 2600") || has("tia"))               return "chip_tia";
  if (has("pokey") || has("atari 800") || has("atari 8")) return "chip_pokey";
  if (has("zx spectrum") || has("speccy"))           return "chip_zx";
  if (has("msx"))                                    return "chip_msx";
  if (has("wonderswan"))                             return "chip_wonderswan";
  if (has("pc-98") || has("pc98"))                   return "chip_pc98";
  if (has("neo geo") || has("neogeo"))               return "chip_neogeo";
  if (has("x68000"))                                 return "chip_x68000";
  if (has("nintendo 64") || has("n64"))              return "chip_n64";
  if (has("saturn"))                                 return "chip_sat";
  if (has("dave") || has("enterprise"))              return "chip_dave";
  if (has("arcade") || has("cps") || has("capcom") || has("namco") || has("konami"))
                                                     return "chip_arcade";
  // FM-only chips when no console is named
  if (has("yamaha") || has("opl") || has("opm") || has("opn") || has("ym2") || has("ym3"))
                                                     return "chip_yamaha";
  // PSG-only chips when no console is named
  if (has("ay-3-8910") || has("ay8910"))             return "chip_ay";
  return CHIP_CUSTOM_KEY;
}
#endif

int64_t FurnaceDiscordRPC::nowMs() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

FurnaceDiscordRPC::FurnaceDiscordRPC():
  level(LEVEL_OFF),
  active(false),
  dirty(false),
  lastFlushMs(0),
  editStartTime((int64_t)time(nullptr)) {}

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
  Discord_Initialize(DISCORD_APP_ID,&handlers,1 /*autoRegister*/,nullptr);
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
  if (l==LEVEL_OFF) {
    stop();
    level=l;
    return;
  }
  if (!active) {
    start(l);
  } else {
    level=l;
    dirty=true;
  }
}

void FurnaceDiscordRPC::setActivity(const std::string& sName, const std::string& sysName, const std::string& sub) {
  // identity change = new song loaded → reset the sticky edit timer
  std::string identity=sName;
  identity+='\x01';
  identity+=sysName;
  if (identity!=lastSongIdentity) {
    editStartTime=(int64_t)time(nullptr);
    lastSongIdentity=identity;
    dirty=true;
  }

  if (sName==songName && sysName==systemName && sub==subtitle) return;
  songName=sName;
  systemName=sysName;
  subtitle=sub;
  dirty=true;
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

  std::string details, state;

  if (level==LEVEL_MINIMAL) {
    details="Furnace";
    state="multi-system chiptune tracker";
    p.largeImageKey=LARGE_KEY;
    p.largeImageText=LARGE_TEXT;
  } else { // LEVEL_FULL
    details="Editing ";
    if (songName.empty()) {
      details+="untitled";
    } else {
      details+='"';
      details+=songName.substr(0,100);
      details+='"';
    }
    // state line: prefer caller-supplied subtitle (song scope) else the chip
    // name (also redundantly available via the small-icon tooltip).
    state=!subtitle.empty() ? subtitle
        : (systemName.empty() ? "unknown system" : systemName);
    p.startTimestamp=editStartTime;
    p.largeImageKey=LARGE_KEY;
    p.largeImageText=LARGE_TEXT;
    p.smallImageKey=chipIconKey(systemName);
    p.smallImageText=systemName.empty() ? "Furnace" : systemName.c_str();
  }

  p.details=details.c_str();
  p.state=state.c_str();
  Discord_UpdatePresence(&p);

  dirty=false;
  lastFlushMs=nowMs();
}
#else
void FurnaceDiscordRPC::flush() {}
#endif
