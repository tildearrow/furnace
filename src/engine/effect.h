/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2023 tildearrow and contributors
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
#include <stdint.h>
#include "../ta-utils.h"

class DivEngine;

enum DivEffectParamType: unsigned char {
  DIV_PARAM_TYPE_DOUBLE,
  DIV_PARAM_TYPE_FLOAT,
  DIV_PARAM_TYPE_S64,
  DIV_PARAM_TYPE_S32,
  DIV_PARAM_TYPE_S16,
  DIV_PARAM_TYPE_S8,
  DIV_PARAM_TYPE_U64,
  DIV_PARAM_TYPE_U32,
  DIV_PARAM_TYPE_U16,
  DIV_PARAM_TYPE_U8,
  DIV_PARAM_TYPE_BOOL,
  DIV_PARAM_TYPE_XY_FLOAT,
  DIV_PARAM_TYPE_XY_INT,
  DIV_PARAM_TYPE_STRING
};

struct DivEffectParam {
  union {
    double d;
    float f;
    uint64_t u64;
    int64_t s64;
    unsigned int u32;
    int s32;
    unsigned short u16;
    short s16;
    unsigned char u8;
    signed char s8;
    bool b;
  } val;
  struct {
    float x, y;
  } valCoordF;
  struct {
    int x, y;
  } valCoordI;
  DivEffectParamType type;
  String valStr;

  DivEffectParam(double v):
    type(DIV_PARAM_TYPE_DOUBLE) {
    val.d=v;
  }
  DivEffectParam(float v):
    type(DIV_PARAM_TYPE_FLOAT) {
    val.f=v;
  }
  DivEffectParam(uint64_t v):
    type(DIV_PARAM_TYPE_U64) {
    val.u64=v;
  }
  DivEffectParam(unsigned int v):
    type(DIV_PARAM_TYPE_U32) {
    val.u32=v;
  }
  DivEffectParam(unsigned short v):
    type(DIV_PARAM_TYPE_U16) {
    val.u16=v;
  }
  DivEffectParam(unsigned char v):
    type(DIV_PARAM_TYPE_U8) {
    val.u8=v;
  }
  DivEffectParam(int64_t v):
    type(DIV_PARAM_TYPE_S64) {
    val.s64=v;
  }
  DivEffectParam(int v):
    type(DIV_PARAM_TYPE_S32) {
    val.s32=v;
  }
  DivEffectParam(short v):
    type(DIV_PARAM_TYPE_S16) {
    val.s16=v;
  }
  DivEffectParam(signed char v):
    type(DIV_PARAM_TYPE_S8) {
    val.s8=v;
  }
  DivEffectParam(bool v):
    type(DIV_PARAM_TYPE_BOOL) {
    val.b=v;
  }
  DivEffectParam(float vx, float vy):
    type(DIV_PARAM_TYPE_XY_FLOAT) {
    valCoordF.x=vx;
    valCoordF.y=vy;
  }
  DivEffectParam(int vx, int vy):
    type(DIV_PARAM_TYPE_XY_INT) {
    valCoordI.x=vx;
    valCoordI.y=vy;
  }
  DivEffectParam(String v):
    type(DIV_PARAM_TYPE_STRING) {
    valStr=v;
  }
};

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
    virtual DivEffectParam getParam(size_t param);

    /**
     * set the value of a parameter.
     * @param param parameter ID.
     * @param value the value.
     * @return whether the parameter was set successfully.
     */
    virtual bool setParam(size_t param, DivEffectParam value);

    /**
     * get a list of parameters.
     * @return a C string with a list of parameters, or NULL if there are none.
     * PARAMETER TYPES
     * 
     * Parameter
     *   id:type:name:description:[...]
     *   type may be one of the following:
     *   - s: string
     *     - hintvalue
     *   - i: integer
     *     - min:max:step
     *   - I: integer slider
     *     - min:max:default
     *   - r: integer list (radio button)
     *     - option1:val1:option2:val2:...
     *   - R: integer list (combo box)
     *     - option1:option2:...
     *   - h: integer hex
     *     - min:max
     *   - f: float
     *     - min:max
     *   - F: float slider
     *     - min:max:default
     *   - d: double
     *     - min:max
     *   - D: double slider
     *     - min:max:default
     *   - b: boolean (checkbox)
     *   - t: boolean (toggle button)
     *   - x: X/Y integer
     *     - xmin:xmax:ymin:ymax
     *   - X: X/Y float
     *     - xmin:xmax:ymin:ymax
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
