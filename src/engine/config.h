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
  public:
    // config loading/saving
    bool loadFromMemory(const char* buf);
    bool loadFromFile(const char* path, bool createOnFail=true);
    String toString();
    bool save(const char* path);

    // get a config value
    bool getConfBool(String key, bool fallback);
    int getConfInt(String key, int fallback);
    float getConfFloat(String key, float fallback);
    double getConfDouble(String key, double fallback);
    String getConfString(String key, String fallback);

    // set a config value
    void setConf(String key, bool value);
    void setConf(String key, int value);
    void setConf(String key, float value);
    void setConf(String key, double value);
    void setConf(String key, const char* value);
    void setConf(String key, String value);
};

#endif