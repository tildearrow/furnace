/**
 * Furnace Tracker - multi-system chiptune tracker
 * Discord Rich Presence integration.
 *
 * Presence layout:
 *   large image:  furnace_logo
 *   small image:  chip_<system>  (e.g. chip_genesis), or chip_custom as fallback
 *   details:      'Editing "<song name>"'  or  'Editing untitled'
 *   state:        chip / system name
 *   elapsed:      time since current edit session began (sticky — does not
 *                 reset when playback or export starts; only resets when a
 *                 different song is loaded)
 */
#ifndef _FURNACE_DISCORD_RPC_H
#define _FURNACE_DISCORD_RPC_H

#include <cstdint>
#include <string>

class FurnaceDiscordRPC {
  public:
    /** Privacy levels. */
    enum Level: unsigned char {
      LEVEL_OFF=0,      ///< RPC disconnected, no IPC opened.
      LEVEL_MINIMAL=1,  ///< Shows only "Furnace" — no song name or chip.
      LEVEL_FULL=2      ///< Shows song name + chip + elapsed time.
    };

    FurnaceDiscordRPC();
    ~FurnaceDiscordRPC();

    void start(Level level);
    void stop();
    void setLevel(Level level);
    Level getLevel() const { return level; }
    bool isActive() const { return active; }

    /**
     * Record what's playing. Cheap: only writes internal state and marks dirty.
     * The actual IPC call happens in tick() so we can throttle.
     *
     * @param songName   shown in details line as `Editing "<name>"`
     * @param systemName drives chip-icon lookup + small-image hover text
     * @param subtitle   state line (e.g. "24 orders • 64 rows • 6 channels")
     */
    void setActivity(const std::string& songName, const std::string& systemName, const std::string& subtitle);

    /** Per-frame poll; flushes pending presence subject to rate-limit. */
    void tick();

  private:
    Level level;
    bool active;
    bool dirty;
    int64_t lastFlushMs;

    // current presence content
    std::string songName, systemName, subtitle;

    // edit-session timer. set once at app start, re-stamped only when the
    // song identity (name + system) changes.
    int64_t editStartTime;
    std::string lastSongIdentity;

    void flush();
    static int64_t nowMs();
};

#endif
