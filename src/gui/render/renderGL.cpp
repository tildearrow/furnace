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
#endif

bool FurnaceGUIRenderGL::createShader(const char* vertexS, const char* fragmentS, int& vertex, int& fragment, int& program) {
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
    return false;
  }

  program=furCreateProgram();
  furAttachShader(program,vertex);
  furAttachShader(program,fragment);
  furBindAttribLocation(program,0,"fur_position");
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
  ImGui_ImplOpenGL3_NewFrame();
  return true;
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

void FurnaceGUIRenderGL::present() {
  SDL_GL_SwapWindow(sdlWin);
  C(glFlush());
}

bool FurnaceGUIRenderGL::getOutputSize(int& w, int& h) {
  SDL_GL_GetDrawableSize(sdlWin,&w,&h);
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
  LOAD_PROC_OPTIONAL(furGetShaderInfoLog,PFNGLGETSHADERINFOLOGPROC,"glGetShaderInfoLog");

#ifndef USE_GLES
  LOAD_PROC_OPTIONAL(furGetGraphicsResetStatusARB,PFNGLGETGRAPHICSRESETSTATUSARBPROC,"glGetGraphicsResetStatusARB");
#endif

  if (createShader(sh_wipe_srcV,sh_wipe_srcF,sh_wipe_vertex,sh_wipe_fragment,sh_wipe_program)) {
    sh_wipe_uAlpha=furGetUniformLocation(sh_wipe_program,"uAlpha");
  }

  C(furGenBuffers(1,&quadBuf));
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
