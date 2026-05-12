/**
 * Furnace Tracker - multi-system chiptune tracker
 * Discord Rich Presence integration.
 *
 * Presence layout (LEVEL_FULL):
 *   large image:  furnace_logo
 *   small image:  chip-specific badge, derived from song.system[]
 *   details:      "<Editing|Playing|Idling> [\"<song name>\"]"
 *   state:        "<orders> orders • <rows> rows • <channels> channels"
 *   elapsed:      sticky edit-session timer; only resets on song change
 */
#ifndef _FURNACE_DISCORD_RPC_H
#define _FURNACE_DISCORD_RPC_H

#include <cstdint>
#include <string>

class DivEngine;

class FurnaceDiscordRPC {
  public:
    enum Level: unsigned char {
      LEVEL_OFF=0,      ///< RPC disconnected, no IPC opened.
      LEVEL_MINIMAL=1,  ///< Shows only "Furnace" — no song details.
      LEVEL_FULL=2      ///< Shows song + chip + scope + elapsed.
    };

    FurnaceDiscordRPC();
    ~FurnaceDiscordRPC();

    void start(Level level);
    void stop();
    void setLevel(Level level);
    Level getLevel() const { return level; }
    bool isActive() const { return active; }

    /**
     * Sample the engine's current state and update the pending presence.
     * Cheap: only writes internal state and flags dirty. The IPC call
     * happens in tick() so we can throttle.
     */
    void setActivity(DivEngine* e);

    /** Per-frame poll. Drives discord-rpc callbacks and rate-limited flush. */
    void tick();

  private:
    Level level;
    bool active;
    bool dirty;
    int64_t lastFlushMs;

    // last computed presence content (used both for diffing and for the flush)
    std::string details;
    std::string state;
    std::string smallText;
    const char* smallIconKey;

    // edit-session timer. set once at app start, re-stamped only when the
    // song's identity (name + chip combo) changes.
    int64_t editStartTime;
    std::string lastSongIdentity;

    void flush();
    static int64_t nowMs();
};

#endif
