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

#ifndef _EFFECT_H
#define _EFFECT_H

#include <stdlib.h>
#include "../ta-utils.h"

class DivEngine;

class DivEffect {
  protected:
    DivEngine* parent;
  public:
    /**
     * fill a buffer with sound data.
     * @param in pointers to input buffers.
     * @param out pointers to output buffers.
     * @param len the amount of samples in input and output.
     */
    virtual void acquire(float** in, float** out, size_t len);

    /**
     * reset the state of this effect.
     */
    virtual void reset();

    /**
     * get the number of inputs this effect requests.
     * @return number of inputs. SHALL NOT be less than zero.
     */
    virtual int getInputCount();

    /**
     * get the number of outputs this effect provides.
     * @return number of outputs. SHALL NOT be less than one.
     */
    virtual int getOutputCount();

    /**
     * called when the sample rate changes.
     * @param rate the new sample rate.
     */
    virtual void rateChanged(double rate);

    /**
     * get the value of a parameter.
     * @param param parameter ID.
     * @return a String with the value.
     * @throws std::out_of_range if the parameter ID is out of range.
     */
    virtual String getParam(size_t param);

    /**
     * set the value of a parameter.
     * @param param parameter ID.
     * @param value the value.
     * @return whether the parameter was set successfully.
     */
    virtual bool setParam(size_t param, String value);

    /**
     * get a list of parameters.
     * @return a C string with a list of parameters, or NULL if there are none.
     * PARAMETER TYPES
     * 
     * Parameter
     *   id:type:name:description:[...]
     *   type may be one of the following:
     *   - s: string
     *   - i: integer
     *   - I: integer slider
     *   - r: integer list (radio button)
     *   - R: integer list (combo box)
     *   - h: integer hex
     *   - f: float
     *   - F: float slider
     *   - d: double
     *   - D: double slider
     *   - b: boolean (checkbox)
     *   - t: boolean (toggle button)
     *   - x: X/Y integer
     *   - X: X/Y float
     *   - c: color (RGB)
     *   - C: color (RGBA)
     *   - B: button
     *
     * SameLine
     *   !s
     *
     * Separator
     *   ---
     *
     * Indent/Unindent
     *   > Indent
     *   < Unindent
     *
     * TreeNode
     *   >> Begin
     *   << End
     *
     * Tabs
     *   >TABBAR   BeginTabBar
     *   >TAB:name Begin
     *   <TAB      End
     *   <TABBAR   EndTabBar
     *
     * Text
     *   TEXT:text (static text)
     *   TEXTF:id (dynamic text)
     *
     * NOTES
     *
     * use a new line to separate parameters.
     * use `\:` if you need a colon.
     */
    virtual const char* getParams();

    /**
     * get the number of parameters.
     * @return count.
     */
    virtual size_t getParamCount();

    /**
     * get a dynamic text.
     * @param id the text ID.
     * @return a String with the text.
     * @throws std::out_of_range if the text ID is out of range.
     */
    virtual String getDynamicText(size_t id);

    /**
     * load effect data.
     * @param version effect data version. may be zero.
     * @param data effect data. may be NULL.
     * @param len effect data length. may be zero.
     * @return whether loading was successful.
     */
    virtual bool load(unsigned short version, const unsigned char* data, size_t len);

    /**
     * save effect data.
     * @param version effect data version.
     * @param len effect data length.
     * @return a pointer to effect data. this must be de-allocated manually.
     *         may also return NULL if it can't save.
     */
    virtual unsigned char* save(unsigned short* version, size_t* len);

    /**
     * initialize this DivEffect.
     * @param parent the parent DivEngine.
     * @param version effect data version. may be zero.
     * @param data effect data. may be NULL.
     * @param len effect data length. may be zero.
     * @return whether initialization was successful.
     */
    virtual bool init(DivEngine* parent, double rate, unsigned short version, const unsigned char* data, size_t len);

    /**
     * quit the DivEffect.
     */
    virtual void quit();

    virtual ~DivEffect();
};

// additional notes:
// - we don't have a GUI API yet, but that will be added when I make the plugin bridge.

#endif