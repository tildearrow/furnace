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

#ifndef _DIVCONFIG_H
#define _DIVCONFIG_H

#include "../ta-utils.h"
#include <map>

class DivConfig {
  std::map<String,String> conf;
  void parseLine(const char* line);
  public:
    // config loading/saving
    bool loadFromMemory(const char* buf);
    bool loadFromBase64(const char* buf);
    bool loadFromFile(const char* path, bool createOnFail=true);
    String toString();
    String toBase64();
    bool save(const char* path);

    // get the map
    const std::map<String,String>& configMap();

    // get a config value
    bool getBool(String key, bool fallback) const;
    int getInt(String key, int fallback) const;
    float getFloat(String key, float fallback) const;
    double getDouble(String key, double fallback) const;
    String getString(String key, String fallback) const;

    // check for existence
    bool has(String key);

    // set a config value
    void set(String key, bool value);
    void set(String key, int value);
    void set(String key, float value);
    void set(String key, double value);
    void set(String key, const char* value);
    void set(String key, String value);

    // remove a config value
    bool remove(String key);

    // clear config
    void clear();
};

#endif
