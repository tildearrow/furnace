/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
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

#include "renderGL.h"
#include "../../ta-log.h"
#ifdef USE_GLES
#include "SDL_opengles2.h"
#define PIXEL_FORMAT GL_UNSIGNED_BYTE
#else
#include "SDL_opengl.h"
#define PIXEL_FORMAT GL_UNSIGNED_INT_8_8_8_8_REV
#endif
#include "backends/imgui_impl_opengl3.h"

#define C(x) x; if (glGetError()!=GL_NO_ERROR) logW("OpenGL error in %s:%d: " #x,__FILE__,__LINE__);

PFNGLGENBUFFERSPROC furGenBuffers=NULL;
PFNGLBINDBUFFERPROC furBindBuffer=NULL;
PFNGLBUFFERDATAPROC furBufferData=NULL;
PFNGLVERTEXATTRIBPOINTERPROC furVertexAttribPointer=NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC furEnableVertexAttribArray=NULL;
PFNGLDISABLEVERTEXATTRIBARRAYPROC furDisableVertexAttribArray=NULL;
PFNGLACTIVETEXTUREPROC furActiveTexture=NULL;

PFNGLCREATESHADERPROC furCreateShader=NULL;
PFNGLSHADERSOURCEPROC furShaderSource=NULL;
PFNGLCOMPILESHADERPROC furCompileShader=NULL;
PFNGLGETSHADERIVPROC furGetShaderiv=NULL;
PFNGLATTACHSHADERPROC furAttachShader=NULL;
PFNGLBINDATTRIBLOCATIONPROC furBindAttribLocation=NULL;
PFNGLCREATEPROGRAMPROC furCreateProgram=NULL;
PFNGLLINKPROGRAMPROC furLinkProgram=NULL;
PFNGLGETPROGRAMIVPROC furGetProgramiv=NULL;
PFNGLUSEPROGRAMPROC furUseProgram=NULL;
PFNGLGETUNIFORMLOCATIONPROC furGetUniformLocation=NULL;
PFNGLUNIFORM1FPROC furUniform1f=NULL;
PFNGLUNIFORM2FPROC furUniform2f=NULL;
PFNGLUNIFORM1IPROC furUniform1i=NULL;
PFNGLUNIFORM4FVPROC furUniform4fv=NULL;
PFNGLGETSHADERINFOLOGPROC furGetShaderInfoLog=NULL;

#ifndef USE_GLES
PFNGLGETGRAPHICSRESETSTATUSARBPROC furGetGraphicsResetStatusARB=NULL;
#endif

class FurnaceGLTexture: public FurnaceGUITexture {
  public:
  GLuint id;
  int width, height;
  unsigned char* lockedData;
  FurnaceGLTexture():
    id(0),
    width(0),
    height(0),
    lockedData(NULL) {}
};

#ifdef USE_GLES
const char* sh_wipe_srcV=
  "attribute vec4 fur_position;\n"
  "void main() {\n"
  " gl_Position=fur_position;\n"
  "}\n";

const char* sh_wipe_srcF=
  "uniform float uAlpha;\n"
  "void main() {\n"
  "  gl_FragColor=vec4(0.0,0.0,0.0,uAlpha);\n"
  "}\n";

const char* sh_oscRender_srcV=
  "attribute vec4 fur_position;\n"
  "attribute vec2 fur_texCoord;\n"
  "varying vec2 fur_fragCoord;\n"
  "void main() {\n"
  "  gl_Position=fur_position;\n"
  "  fur_fragCoord=fur_texCoord;\n"
  "}\n";

// thank you akumanatt
const char* sh_oscRender_srcF=
  "uniform vec4 uColor;\n"
  "uniform vec2 uResolution;\n"
  "uniform float uLineWidth;\n"
  "uniform float uAdvance;\n"
  "uniform sampler2D oscVal;\n"
  "varying vec2 fur_fragCoord;\n"
  "void main() {\n"
  "  vec2 uv = fur_fragCoord/uResolution;\n"
  "  vec2 tresh = vec2(uLineWidth)/uResolution;\n"
  "  float x2 = uv.x;\n"
  "  float x3 = uv.x+uAdvance;\n"
  "  float val2 = texture2D(oscVal,vec2(x2,1.0)).x;\n"
  "  float val3 = texture2D(oscVal,vec2(x3,1.0)).x;\n"
  "  float valmax = max(val2,val3);\n"
  "  float valmin = min(val2,val3);\n"
  "  float vald = abs(valmax-valmin);\n"
  "  float alpha = 1.0-abs(uv.y-val2)/max(tresh.y,vald);\n"
  "  if (vald>(1.0/uResolution.y)) {\n"
  "    gl_FragColor = vec4(1.0,0.0,0.0,uColor.w*alpha);\n"
  "  } else {\n"
  "    gl_FragColor = vec4(uColor.xyz,uColor.w*alpha);\n"
  "  }\n"
  "}\n";
#else
const char* sh_wipe_srcV=
  "#version 130\n"
  "in vec4 fur_position;\n"
  "void main() {\n"
  " gl_Position=fur_position;\n"
  "}\n";

const char* sh_wipe_srcF=
  "#version 130\n"
  "uniform float uAlpha;\n"
  "out vec4 fur_FragColor;\n"
  "void main() {\n"
  "  fur_FragColor=vec4(0.0,0.0,0.0,uAlpha);\n"
  "}\n";

const char* sh_oscRender_srcV=
  "#version 130\n"
  "in vec4 fur_position;\n"
  "in vec2 fur_texCoord;\n"
  "out vec2 fur_fragCoord;\n"
  "void main() {\n"
  "  gl_Position=fur_position;\n"
  "  fur_fragCoord=fur_texCoord;\n"
  "}\n";

// thank you akumanatt
const char* sh_oscRender_srcF=
  "#version 130\n"
  "uniform vec4 uColor;\n"
  "uniform vec2 uResolution;\n"
  "uniform float uLineWidth;\n"
  "uniform float uAdvance;\n"
  "uniform sampler1D oscVal;\n"
  "in vec2 fur_fragCoord;\n"
  "out vec4 fur_FragColor;\n"
  "void main() {\n"
  "  vec2 uv = fur_fragCoord/uResolution;\n"
  "  vec2 tresh = vec2(uLineWidth)/uResolution;\n"
  "  float x2 = uv.x;\n"
  "  float x3 = uv.x+uAdvance;\n"
  "  float val2 = texture(oscVal,x2).x;\n"
  "  float val3 = texture(oscVal,x3).x;\n"
  "  float valmax = max(val2,val3);\n"
  "  float valmin = min(val2,val3);\n"
  "  float vald = abs(valmax-valmin);\n"
  "  float alpha = 1.0-abs(uv.y-val2)/max(tresh.y,vald);\n"
  "  if (vald>(1.0/uResolution.y)) {\n"
  "    fur_FragColor = vec4(1.0,0.0,0.0,uColor.w*alpha);\n"
  "  } else {\n"
  "    fur_FragColor = vec4(uColor.xyz,uColor.w*alpha);\n"
  "  }\n"
  "}\n";
#endif

const char* sh_wipe_attrib[]={
  "fur_position",
  NULL
};

const char* sh_oscRender_attrib[]={
  "fur_position",
  "fur_texCoord",
  NULL
};

bool FurnaceGUIRenderGL::createShader(const char* vertexS, const char* fragmentS, int& vertex, int& fragment, int& program, const char** attribNames) {
  int status;
  char infoLog[4096];
  int infoLogLen;

  if (!furCreateShader || !furShaderSource || !furCompileShader || !furGetShaderiv ||
      !furGetShaderInfoLog || !furCreateProgram || !furAttachShader || !furLinkProgram ||
      !furBindAttribLocation || !furGetProgramiv) {
    logW("I can't compile shaders");
    return false;
  }

  vertex=furCreateShader(GL_VERTEX_SHADER);
  furShaderSource(vertex,1,&vertexS,NULL);
  furCompileShader(vertex);
  furGetShaderiv(vertex,GL_COMPILE_STATUS,&status);
  if (!status) {
    logW("failed to compile vertex shader");
    furGetShaderInfoLog(vertex,4095,&infoLogLen,infoLog);
    infoLog[infoLogLen]=0;
    logW("%s",infoLog);
    return false;
  }

  fragment=furCreateShader(GL_FRAGMENT_SHADER);
  furShaderSource(fragment,1,&fragmentS,NULL);
  furCompileShader(fragment);
  furGetShaderiv(fragment,GL_COMPILE_STATUS,&status);
  if (!status) {
    logW("failed to compile fragment shader");
    furGetShaderInfoLog(fragment,4095,&infoLogLen,infoLog);
    infoLog[infoLogLen]=0;
    logW("%s",infoLog);
    return false;
  }

  program=furCreateProgram();
  furAttachShader(program,vertex);
  furAttachShader(program,fragment);
  if (attribNames!=NULL) {
    for (int i=0; attribNames[i]; i++) {
      furBindAttribLocation(program,i,attribNames[i]);
    }
  }
  furLinkProgram(program);
  furGetProgramiv(program,GL_LINK_STATUS,&status);
  if (!status) {
    logW("failed to link shader!");
    return false;
  }
  
  return true;
}

ImTextureID FurnaceGUIRenderGL::getTextureID(FurnaceGUITexture* which) {
  intptr_t ret=((FurnaceGLTexture*)which)->id;
  return (ImTextureID)ret;
}

bool FurnaceGUIRenderGL::lockTexture(FurnaceGUITexture* which, void** data, int* pitch) {
  FurnaceGLTexture* t=(FurnaceGLTexture*)which;
  if (t->lockedData!=NULL) return false;
  t->lockedData=new unsigned char[t->width*t->height*4];

  *data=t->lockedData;
  *pitch=t->width*4;
  return true;
}

bool FurnaceGUIRenderGL::unlockTexture(FurnaceGUITexture* which) {
  FurnaceGLTexture* t=(FurnaceGLTexture*)which;
  if (t->lockedData==NULL) return false;

  C(glBindTexture(GL_TEXTURE_2D,t->id));
  C(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,t->width,t->height,0,GL_RGBA,PIXEL_FORMAT,t->lockedData));
  
  C(glFlush());
  delete[] t->lockedData;
  t->lockedData=NULL;

  return true;
}

bool FurnaceGUIRenderGL::updateTexture(FurnaceGUITexture* which, void* data, int pitch) {
  FurnaceGLTexture* t=(FurnaceGLTexture*)which;

  if (t->width*4!=pitch) return false;

  C(glBindTexture(GL_TEXTURE_2D,t->id));
  C(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,t->width,t->height,0,GL_RGBA,PIXEL_FORMAT,data));
  return true;
}

FurnaceGUITexture* FurnaceGUIRenderGL::createTexture(bool dynamic, int width, int height) {
  FurnaceGLTexture* t=new FurnaceGLTexture;
  C(glGenTextures(1,&t->id));
  C(glBindTexture(GL_TEXTURE_2D,t->id));
  C(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR));
  C(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR));
  C(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,PIXEL_FORMAT,NULL));
  C(furActiveTexture(GL_TEXTURE0));
  t->width=width;
  t->height=height;
  return t;
}

bool FurnaceGUIRenderGL::destroyTexture(FurnaceGUITexture* which) {
  FurnaceGLTexture* t=(FurnaceGLTexture*)which;
  C(glDeleteTextures(1,&t->id));
  delete t;
  return true;
}

void FurnaceGUIRenderGL::setTextureBlendMode(FurnaceGUITexture* which, FurnaceGUIBlendMode mode) {
}

void FurnaceGUIRenderGL::setBlendMode(FurnaceGUIBlendMode mode) {
  switch (mode) {
    case GUI_BLEND_MODE_NONE:
      C(glBlendFunc(GL_ONE,GL_ZERO));
      C(glDisable(GL_BLEND));
      break;
    case GUI_BLEND_MODE_BLEND:
      C(glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA));
      C(glEnable(GL_BLEND));
      break;
    case GUI_BLEND_MODE_ADD:
      C(glBlendFunc(GL_SRC_ALPHA,GL_ONE));
      C(glEnable(GL_BLEND));
      break;
    case GUI_BLEND_MODE_MULTIPLY:
      C(glBlendFunc(GL_ZERO,GL_SRC_COLOR));
      C(glEnable(GL_BLEND));
      break;
  }
}

void FurnaceGUIRenderGL::clear(ImVec4 color) {
  SDL_GL_MakeCurrent(sdlWin,context);
  C(glClearColor(color.x,color.y,color.z,color.w));
  C(glClear(GL_COLOR_BUFFER_BIT));
}

bool FurnaceGUIRenderGL::newFrame() {
  return ImGui_ImplOpenGL3_NewFrame();
}

void FurnaceGUIRenderGL::createFontsTexture() {
  ImGui_ImplOpenGL3_CreateFontsTexture();
}

void FurnaceGUIRenderGL::destroyFontsTexture() {
  ImGui_ImplOpenGL3_DestroyFontsTexture();
}

void FurnaceGUIRenderGL::renderGUI() {
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void FurnaceGUIRenderGL::wipe(float alpha) {
  C(glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA));
  C(glEnable(GL_BLEND));

  quadVertex[0][0]=-1.0f;
  quadVertex[0][1]=-1.0f;
  quadVertex[0][2]=0.0f;
  quadVertex[1][0]=1.0f;
  quadVertex[1][1]=-1.0f;
  quadVertex[1][2]=0.0f;
  quadVertex[2][0]=-1.0f;
  quadVertex[2][1]=1.0f;
  quadVertex[2][2]=0.0f;
  quadVertex[3][0]=1.0f;
  quadVertex[3][1]=1.0f;
  quadVertex[3][2]=0.0f;

  C(furBindBuffer(GL_ARRAY_BUFFER,quadBuf));
  C(furBufferData(GL_ARRAY_BUFFER,sizeof(quadVertex),quadVertex,GL_STATIC_DRAW));
  C(furVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL));
  C(furEnableVertexAttribArray(0));
  C(furActiveTexture(GL_TEXTURE0));
  C(glBindTexture(GL_TEXTURE_2D,0));
  if (furUseProgram && furUniform1f) {
    C(furUseProgram(sh_wipe_program));
    C(furUniform1f(sh_wipe_uAlpha,alpha));
  }
  C(glDrawArrays(GL_TRIANGLE_STRIP,0,4));
}

void FurnaceGUIRenderGL::drawOsc(float* data, size_t len, ImVec2 pos0, ImVec2 pos1, ImVec4 color, ImVec2 canvasSize, float lineWidth) {
  if (!furUseProgram) return;
  if (!furUniform4fv) return;
  if (!furUniform1f) return;
  if (!furUniform2f) return;
  if (!furUniform1i) return;

  if (len>2048) len=2048;

  memcpy(oscData,data,len*sizeof(float));

  int lastArrayBuf=0;
  int lastElemArrayBuf=0;
  int lastTex=0;
  int lastProgram=0;
  int lastActiveTex=0;
  C(glGetIntegerv(GL_ACTIVE_TEXTURE,&lastActiveTex));
  C(furActiveTexture(GL_TEXTURE0));

#ifdef USE_GLES
  C(glGetIntegerv(GL_TEXTURE_BINDING_2D,&lastTex));
#else
  C(glGetIntegerv(GL_TEXTURE_BINDING_1D,&lastTex));
#endif

#ifdef USE_GLES
  C(glBindTexture(GL_TEXTURE_2D,oscDataTex));
  C(glTexImage2D(GL_TEXTURE_2D,0,GL_RED_EXT,2048,1,0,GL_RED_EXT,GL_FLOAT,oscData));
#else
  C(glBindTexture(GL_TEXTURE_1D,oscDataTex));
  C(glTexImage1D(GL_TEXTURE_1D,0,GL_R32F,2048,0,GL_RED,GL_FLOAT,oscData));
#endif

  //C(glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA));
  //C(glEnable(GL_BLEND));

  float width=fabs(pos1.x-pos0.x);
  float height=fabs(pos1.y-pos0.y)*0.5;

  pos0.x=(2.0f*pos0.x/canvasSize.x)-1.0f;
  pos0.y=1.0f-(2.0f*pos0.y/canvasSize.y);
  pos1.x=(2.0f*pos1.x/canvasSize.x)-1.0f;
  pos1.y=1.0f-(2.0f*pos1.y/canvasSize.y);

  oscVertex[0][0]=pos0.x;
  oscVertex[0][1]=pos1.y;
  oscVertex[0][2]=0.0f;
  oscVertex[0][3]=height;
  oscVertex[1][0]=pos1.x;
  oscVertex[1][1]=pos1.y;
  oscVertex[1][2]=(float)len;
  oscVertex[1][3]=height;
  oscVertex[2][0]=pos0.x;
  oscVertex[2][1]=pos0.y;
  oscVertex[2][2]=0.0f;
  oscVertex[2][3]=-height;
  oscVertex[3][0]=pos1.x;
  oscVertex[3][1]=pos0.y;
  oscVertex[3][2]=(float)len;
  oscVertex[3][3]=-height;

  C(glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&lastArrayBuf));
  C(glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING,&lastElemArrayBuf));

  C(furBindBuffer(GL_ARRAY_BUFFER,oscVertexBuf));
  C(furBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0));
  C(furBufferData(GL_ARRAY_BUFFER,sizeof(oscVertex),oscVertex,GL_STATIC_DRAW));
  C(furVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,4*sizeof(float),NULL));
  C(furVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,4*sizeof(float),(void*)(2*sizeof(float))));
  C(furEnableVertexAttribArray(0));
  C(furEnableVertexAttribArray(1));

  C(glGetIntegerv(GL_CURRENT_PROGRAM,&lastProgram));

  C(furUseProgram(sh_oscRender_program));
  C(furUniform4fv(sh_oscRender_uColor,1,(float*)&color));
  C(furUniform1f(sh_oscRender_uLineWidth,lineWidth));
  C(furUniform1f(sh_oscRender_uAdvance,(1.0f/2048.0f)*((float)len/width)));
  C(furUniform2f(sh_oscRender_uResolution,2048.0f,height));
  C(furUniform1i(sh_oscRender_oscVal,0));

  C(glDrawArrays(GL_TRIANGLE_STRIP,0,4));
  C(furDisableVertexAttribArray(1));

  // restore state
  C(furUseProgram(lastProgram));

  C(furBindBuffer(GL_ARRAY_BUFFER,lastArrayBuf));
  C(furBindBuffer(GL_ELEMENT_ARRAY_BUFFER,lastElemArrayBuf));

#ifdef USE_GLES
  C(glBindTexture(GL_TEXTURE_2D,lastTex));
#else
  C(glBindTexture(GL_TEXTURE_1D,lastTex));
#endif
  C(furActiveTexture(lastActiveTex));
}

void FurnaceGUIRenderGL::present() {
  SDL_GL_SwapWindow(sdlWin);
  C(glFlush());
}

bool FurnaceGUIRenderGL::getOutputSize(int& w, int& h) {
  SDL_GL_GetDrawableSize(sdlWin,&w,&h);
  return true;
}

bool FurnaceGUIRenderGL::supportsDrawOsc() {
  return true;
}

int FurnaceGUIRenderGL::getWindowFlags() {
  return SDL_WINDOW_OPENGL;
}

void FurnaceGUIRenderGL::preInit() {
#if defined(USE_GLES)
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,0);
#elif defined(__APPLE__)
  // not recommended...
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,2);
#else
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,0);
#endif

  SDL_GL_SetAttribute(SDL_GL_RED_SIZE,8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,0);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,24);
}

#define LOAD_PROC_MANDATORY(_v,_t,_s) \
  _v=(_t)SDL_GL_GetProcAddress(_s); \
  if (!_v) { \
    logE(_s " not found"); \
    return false; \
  }

#define LOAD_PROC_OPTIONAL(_v,_t,_s) \
  _v=(_t)SDL_GL_GetProcAddress(_s); \
  if (!_v) { \
    logW(_s " not found"); \
  }

bool FurnaceGUIRenderGL::init(SDL_Window* win) {
  sdlWin=win;
  context=SDL_GL_CreateContext(win);
  if (context==NULL) {
    return false;
  }
  SDL_GL_MakeCurrent(win,context);
  SDL_GL_SetSwapInterval(1);

  LOAD_PROC_MANDATORY(furGenBuffers,PFNGLGENBUFFERSPROC,"glGenBuffers");
  LOAD_PROC_MANDATORY(furBindBuffer,PFNGLBINDBUFFERPROC,"glBindBuffer");
  LOAD_PROC_MANDATORY(furBufferData,PFNGLBUFFERDATAPROC,"glBufferData");
  LOAD_PROC_MANDATORY(furVertexAttribPointer,PFNGLVERTEXATTRIBPOINTERPROC,"glVertexAttribPointer");
  LOAD_PROC_MANDATORY(furEnableVertexAttribArray,PFNGLENABLEVERTEXATTRIBARRAYPROC,"glEnableVertexAttribArray");
  LOAD_PROC_MANDATORY(furDisableVertexAttribArray,PFNGLDISABLEVERTEXATTRIBARRAYPROC,"glDisableVertexAttribArray");
  LOAD_PROC_MANDATORY(furActiveTexture,PFNGLACTIVETEXTUREPROC,"glActiveTexture");

  LOAD_PROC_OPTIONAL(furCreateShader,PFNGLCREATESHADERPROC,"glCreateShader");
  LOAD_PROC_OPTIONAL(furShaderSource,PFNGLSHADERSOURCEPROC,"glShaderSource");
  LOAD_PROC_OPTIONAL(furCompileShader,PFNGLCOMPILESHADERPROC,"glCompileShader");
  LOAD_PROC_OPTIONAL(furGetShaderiv,PFNGLGETSHADERIVPROC,"glGetShaderiv");
  LOAD_PROC_OPTIONAL(furAttachShader,PFNGLATTACHSHADERPROC,"glAttachShader");
  LOAD_PROC_OPTIONAL(furBindAttribLocation,PFNGLBINDATTRIBLOCATIONPROC,"glBindAttribLocation");
  LOAD_PROC_OPTIONAL(furCreateProgram,PFNGLCREATEPROGRAMPROC,"glCreateProgram");
  LOAD_PROC_OPTIONAL(furLinkProgram,PFNGLLINKPROGRAMPROC,"glLinkProgram");
  LOAD_PROC_OPTIONAL(furGetProgramiv,PFNGLGETPROGRAMIVPROC,"glGetProgramiv");
  LOAD_PROC_OPTIONAL(furUseProgram,PFNGLUSEPROGRAMPROC,"glUseProgram");
  LOAD_PROC_OPTIONAL(furGetUniformLocation,PFNGLGETUNIFORMLOCATIONPROC,"glGetUniformLocation");
  LOAD_PROC_OPTIONAL(furUniform1f,PFNGLUNIFORM1FPROC,"glUniform1f");
  LOAD_PROC_OPTIONAL(furUniform2f,PFNGLUNIFORM2FPROC,"glUniform2f");
  LOAD_PROC_OPTIONAL(furUniform1i,PFNGLUNIFORM1IPROC,"glUniform1i");
  LOAD_PROC_OPTIONAL(furUniform4fv,PFNGLUNIFORM4FVPROC,"glUniform4fv");
  LOAD_PROC_OPTIONAL(furGetShaderInfoLog,PFNGLGETSHADERINFOLOGPROC,"glGetShaderInfoLog");

#ifndef USE_GLES
  LOAD_PROC_OPTIONAL(furGetGraphicsResetStatusARB,PFNGLGETGRAPHICSRESETSTATUSARBPROC,"glGetGraphicsResetStatusARB");
#endif

  // texture for osc renderer
  C(glGenTextures(1,&oscDataTex));
#ifdef USE_GLES
  C(glBindTexture(GL_TEXTURE_2D,oscDataTex));
  C(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR));
  C(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR));
  C(glTexImage2D(GL_TEXTURE_2D,0,GL_RED_EXT,2048,1,0,GL_RED_EXT,GL_FLOAT,NULL));
#else
  C(glBindTexture(GL_TEXTURE_1D,oscDataTex));
  C(glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER,GL_LINEAR));
  C(glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER,GL_LINEAR));
  C(glTexImage1D(GL_TEXTURE_1D,0,GL_RED,2048,0,GL_RED,GL_FLOAT,NULL));
#endif
  C(furActiveTexture(GL_TEXTURE0));

  // create shaders
  if (createShader(sh_wipe_srcV,sh_wipe_srcF,sh_wipe_vertex,sh_wipe_fragment,sh_wipe_program,sh_wipe_attrib)) {
    sh_wipe_uAlpha=furGetUniformLocation(sh_wipe_program,"uAlpha");
  }

  if (createShader(sh_oscRender_srcV,sh_oscRender_srcF,sh_oscRender_vertex,sh_oscRender_fragment,sh_oscRender_program,sh_oscRender_attrib)) {
    sh_oscRender_uColor=furGetUniformLocation(sh_oscRender_program,"uColor");
    sh_oscRender_uAdvance=furGetUniformLocation(sh_oscRender_program,"uAdvance");
    sh_oscRender_uLineWidth=furGetUniformLocation(sh_oscRender_program,"uLineWidth");
    sh_oscRender_uResolution=furGetUniformLocation(sh_oscRender_program,"uResolution");
    sh_oscRender_oscVal=furGetUniformLocation(sh_oscRender_program,"oscVal");
  }

  C(furGenBuffers(1,&quadBuf));
  C(furGenBuffers(1,&oscVertexBuf));
  return true;
}

void FurnaceGUIRenderGL::initGUI(SDL_Window* win) {
  ImGui_ImplSDL2_InitForOpenGL(win,context);
  ImGui_ImplOpenGL3_Init();
}

bool FurnaceGUIRenderGL::quit() {
  if (context==NULL) return false;
  SDL_GL_DeleteContext(context);
  context=NULL;
  return true;
}

void FurnaceGUIRenderGL::quitGUI() { 
  ImGui_ImplOpenGL3_Shutdown();
}

bool FurnaceGUIRenderGL::isDead() {
#ifndef USE_GLES
   if (furGetGraphicsResetStatusARB==NULL) return false;
   return (furGetGraphicsResetStatusARB()!=GL_NO_ERROR);
#else
   // handled by SDL... I think
   return false;
#endif
}
