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
PFNGLDELETEPROGRAMPROC furDeleteProgram=NULL;
PFNGLDELETESHADERPROC furDeleteShader=NULL;
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
  FurnaceGUITextureFormat format;
  unsigned char* lockedData;
  FurnaceGLTexture():
    id(0),
    width(0),
    height(0),
    format(GUI_TEXFORMAT_UNKNOWN),
    lockedData(NULL) {}
};

#ifdef USE_GLES
const char* sh_wipe_srcV_ES2=
  "attribute vec4 fur_position;\n"
  "void main() {\n"
  " gl_Position=fur_position;\n"
  "}\n";

const char* sh_wipe_srcF_ES2=
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
  "precision highp float;\n"
  "uniform vec4 uColor;\n"
  "uniform vec2 uResolution;\n"
  "uniform float uLineWidth;\n"
  "uniform sampler2D oscVal;\n"
  "varying vec2 fur_fragCoord;\n"
  "const float oneStep=1.0/2048.0;\n"
  "void main() {\n"
  "  float alpha=0.0;\n"
  "  float xMax=ceil(fur_fragCoord.x+uLineWidth);\n"
  "  float valmax=-1024.0;\n"
  "  float valmin=1024.0;\n"
  "  for (float x=floor(fur_fragCoord.x-uLineWidth); x<=xMax; x+=1.0) {\n"
  "    float val=texture2D(oscVal,vec2(x*oneStep,1.0)).x;\n"
  "    if (val>valmax) valmax=val;\n"
  "    if (val<valmin) valmin=val;\n"
  "  }\n"
  "  if ((fur_fragCoord.y-uLineWidth)>valmax*uResolution.y) discard;\n"
  "  if ((fur_fragCoord.y+uLineWidth)<valmin*uResolution.y) discard;\n"
  "  float slope=abs(valmax-valmin)*uResolution.y;\n"
  "  float slopeMul=pow(2.0,ceil(log2(ceil(slope))));\n"
  "  float slopeDiv=min(1.0,1.0/slopeMul);\n"
  "  float xRight=ceil(fur_fragCoord.x+uLineWidth);\n"
  "  for (float x=max(0.0,floor(fur_fragCoord.x-uLineWidth)); x<=xRight; x+=slopeDiv) {\n"
  "    float val0=texture2D(oscVal,vec2(floor(x)*oneStep,1.0)).x;\n"
  "    float val1=texture2D(oscVal,vec2(floor(x+1.0)*oneStep,1.0)).x;\n"
  "    float val=mix(val0,val1,fract(x))*uResolution.y;\n"
  "    alpha+=clamp(uLineWidth-distance(vec2(fur_fragCoord.x,fur_fragCoord.y),vec2(x,val)),0.0,1.0);\n"
  "  }\n"
  "  if (slope>1.0) {\n"
  "    gl_FragColor = vec4(uColor.xyz,uColor.w*clamp(alpha*(1.0+uResolution.y*pow(slope/uResolution.y,2.0))/(uLineWidth*slopeMul),0.0,1.0));\n"
  "  } else {\n"
  "    gl_FragColor = vec4(uColor.xyz,uColor.w*clamp(alpha/uLineWidth,0.0,1.0));\n"
  "  }\n"
  "}\n";
#else
const char* sh_wipe_srcV_130=
  "#version 130\n"
  "in vec4 fur_position;\n"
  "void main() {\n"
  " gl_Position=fur_position;\n"
  "}\n";

const char* sh_wipe_srcF_130=
  "#version 130\n"
  "uniform float uAlpha;\n"
  "out vec4 fur_FragColor;\n"
  "void main() {\n"
  "  fur_FragColor=vec4(0.0,0.0,0.0,uAlpha);\n"
  "}\n";

const char* sh_wipe_srcV_110=
  "#version 110\n"
  "attribute vec4 fur_position;\n"
  "void main() {\n"
  " gl_Position=fur_position;\n"
  "}\n";

const char* sh_wipe_srcF_110=
  "#version 110\n"
  "uniform float uAlpha;\n"
  "void main() {\n"
  "  gl_FragColor=vec4(0.0,0.0,0.0,uAlpha);\n"
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
  "uniform sampler1D oscVal;\n"
  "in vec2 fur_fragCoord;\n"
  "out vec4 fur_FragColor;\n"
  "void main() {\n"
  "  float alpha=0.0;\n"
  "  float xMax=ceil(fur_fragCoord.x+uLineWidth);\n"
  "  float valmax=-1024.0;\n"
  "  float valmin=1024.0;\n"
  "  for (float x=floor(fur_fragCoord.x-uLineWidth); x<=xMax; x+=1.0) {\n"
  "    float val=texelFetch(oscVal,int(x),0).x;\n"
  "    if (val>valmax) valmax=val;\n"
  "    if (val<valmin) valmin=val;\n"
  "  }\n"
  "  if ((fur_fragCoord.y-uLineWidth)>valmax*uResolution.y) discard;\n"
  "  if ((fur_fragCoord.y+uLineWidth)<valmin*uResolution.y) discard;\n"
  "  float slope=abs(valmax-valmin)*uResolution.y;\n"
  "  float slopeMul=pow(2.0,ceil(log2(ceil(slope))));\n"
  "  float slopeDiv=min(1.0,1.0/slopeMul);\n"
  "  float xRight=ceil(fur_fragCoord.x+uLineWidth);\n"
  "  for (float x=max(0.0,floor(fur_fragCoord.x-uLineWidth)); x<=xRight; x+=slopeDiv) {\n"
  "    float val0=texelFetch(oscVal,int(x),0).x;\n"
  "    float val1=texelFetch(oscVal,int(x)+1,0).x;\n"
  "    float val=mix(val0,val1,fract(x))*uResolution.y;\n"
  "    alpha+=max(uLineWidth-distance(vec2(fur_fragCoord.x,fur_fragCoord.y),vec2(x,val)),0.0);\n"
  "  }\n"
  "  if (slope>1.0) {\n"
  "    fur_FragColor = vec4(uColor.xyz,uColor.w*clamp(alpha*(1.0+uResolution.y*pow(slope/uResolution.y,2.0))/(uLineWidth*slopeMul),0.0,1.0));\n"
  "  } else {\n"
  "    fur_FragColor = vec4(uColor.xyz,uColor.w*clamp(alpha/uLineWidth,0.0,1.0));\n"
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

FurnaceGUITextureFormat FurnaceGUIRenderGL::getTextureFormat(FurnaceGUITexture* which) {
  FurnaceGLTexture* t=(FurnaceGLTexture*)which;
  return t->format;
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

FurnaceGUITexture* FurnaceGUIRenderGL::createTexture(bool dynamic, int width, int height, bool interpolate, FurnaceGUITextureFormat format) {
  if (format!=GUI_TEXFORMAT_ABGR32) {
    logE("unsupported texture format!");
    return NULL;
  }
  FurnaceGLTexture* t=new FurnaceGLTexture;
  C(glGenTextures(1,&t->id));
  C(glBindTexture(GL_TEXTURE_2D,t->id));
  if (interpolate) {
    C(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR));
    C(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR));
  } else {
    C(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST));
    C(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST));
  }
  C(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,PIXEL_FORMAT,NULL));
  C(furActiveTexture(GL_TEXTURE0));
  t->width=width;
  t->height=height;
  t->format=format;
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

bool FurnaceGUIRenderGL::canVSync() {
  return swapIntervalSet;
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
  if (!sh_oscRender_have) return;
  if (!furUseProgram) return;
  if (!furUniform4fv) return;
  if (!furUniform1f) return;
  if (!furUniform2f) return;
  if (!furUniform1i) return;

  if (len>2048) {
    logW("len is %d!",len);
    len=2048;
  }

  memcpy(oscData,data,len*sizeof(float));
  if (len<2048) {
    oscData[len]=oscData[len-1];
  }

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

  //float width=fabs(pos1.x-pos0.x);
  float height=fabs(pos1.y-pos0.y)*0.5;

  pos0.x=(2.0f*pos0.x/canvasSize.x)-1.0f;
  pos0.y=1.0f-(2.0f*pos0.y/canvasSize.y);
  pos1.x=(2.0f*pos1.x/canvasSize.x)-1.0f;
  pos1.y=1.0f-(2.0f*pos1.y/canvasSize.y);

  oscVertex[0][0]=pos0.x;
  oscVertex[0][1]=pos1.y;
  oscVertex[0][2]=0.0f;
  oscVertex[0][3]=-height;
  oscVertex[1][0]=pos1.x;
  oscVertex[1][1]=pos1.y;
  oscVertex[1][2]=(float)len;
  oscVertex[1][3]=-height;
  oscVertex[2][0]=pos0.x;
  oscVertex[2][1]=pos0.y;
  oscVertex[2][2]=0.0f;
  oscVertex[2][3]=height;
  oscVertex[3][0]=pos1.x;
  oscVertex[3][1]=pos0.y;
  oscVertex[3][2]=(float)len;
  oscVertex[3][3]=height;

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
  if (lineWidth<=1.0) {
    C(furUniform1f(sh_oscRender_uLineWidth,lineWidth));
  } else {
    C(furUniform1f(sh_oscRender_uLineWidth,0.5+lineWidth*0.5));
  }
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
  return sh_oscRender_have;
}

int FurnaceGUIRenderGL::getWindowFlags() {
  return SDL_WINDOW_OPENGL;
}

int FurnaceGUIRenderGL::getMaxTextureWidth() {
  return maxWidth;
}

int FurnaceGUIRenderGL::getMaxTextureHeight() {
  return maxHeight;
}

unsigned int FurnaceGUIRenderGL::getTextureFormats() {
  return GUI_TEXFORMAT_ABGR32;
}

const char* FurnaceGUIRenderGL::getBackendName() {
  return backendName.c_str();
}

const char* FurnaceGUIRenderGL::getVendorName() {
  return vendorName.c_str();
}

const char* FurnaceGUIRenderGL::getDeviceName() {
  return deviceName.c_str();
}

const char* FurnaceGUIRenderGL::getAPIVersion() {
  return apiVersion.c_str();
}

void FurnaceGUIRenderGL::setSwapInterval(int swapInterval) {
  SDL_GL_SetSwapInterval(swapInterval);
  if (swapInterval>0 && SDL_GL_GetSwapInterval()==0) {
    swapIntervalSet=false;
    logW("tried to enable VSync but couldn't!");
  } else {
    swapIntervalSet=true;
  }
}

void FurnaceGUIRenderGL::preInit(const DivConfig& conf) {
#if defined(USE_GLES)
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,0);
#elif defined(__APPLE__)
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE);
  if (glVer==2) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,0);
  } else {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,2);
  }
#else
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,glVer);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,0);
#endif

  SDL_GL_SetAttribute(SDL_GL_RED_SIZE,conf.getInt("glRedSize",8));
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,conf.getInt("glGreenSize",8));
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,conf.getInt("glBlueSize",8));
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,conf.getInt("glAlphaSize",0));
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,conf.getInt("glDoubleBuffer",1));
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,conf.getInt("glDepthSize",24));
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

bool FurnaceGUIRenderGL::init(SDL_Window* win, int swapInterval) {
  sdlWin=win;
  context=SDL_GL_CreateContext(win);
  if (context==NULL) {
    return false;
  }
  SDL_GL_MakeCurrent(win,context);
  SDL_GL_SetSwapInterval(swapInterval);
  if (swapInterval>0 && SDL_GL_GetSwapInterval()==0) {
    swapIntervalSet=false;
    logW("tried to enable VSync but couldn't!");
  } else {
    swapIntervalSet=true;
  }

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
  LOAD_PROC_OPTIONAL(furDeleteProgram,PFNGLDELETEPROGRAMPROC,"glDeleteProgram");
  LOAD_PROC_OPTIONAL(furDeleteShader,PFNGLDELETESHADERPROC,"glDeleteShader");
  LOAD_PROC_OPTIONAL(furGetUniformLocation,PFNGLGETUNIFORMLOCATIONPROC,"glGetUniformLocation");
  LOAD_PROC_OPTIONAL(furUniform1f,PFNGLUNIFORM1FPROC,"glUniform1f");
  LOAD_PROC_OPTIONAL(furUniform2f,PFNGLUNIFORM2FPROC,"glUniform2f");
  LOAD_PROC_OPTIONAL(furUniform1i,PFNGLUNIFORM1IPROC,"glUniform1i");
  LOAD_PROC_OPTIONAL(furUniform4fv,PFNGLUNIFORM4FVPROC,"glUniform4fv");
  LOAD_PROC_OPTIONAL(furGetShaderInfoLog,PFNGLGETSHADERINFOLOGPROC,"glGetShaderInfoLog");

#ifndef USE_GLES
  LOAD_PROC_OPTIONAL(furGetGraphicsResetStatusARB,PFNGLGETGRAPHICSRESETSTATUSARBPROC,"glGetGraphicsResetStatusARB");
#else
  backendName="OpenGL ES 2.0";
#endif

  // information
  const char* next=(const char*)glGetString(GL_VENDOR);
  if (next==NULL) {
    vendorName="???";
  } else {
    vendorName=next;
  }
  next=(const char*)glGetString(GL_RENDERER);
  if (next==NULL) {
    deviceName="???";
  } else {
    deviceName=next;
  }
  next=(const char*)glGetString(GL_VERSION);
  if (next==NULL) {
    apiVersion="???";
  } else {
    apiVersion=next;
  }

  int maxSize=1024;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxSize);

  maxWidth=maxSize;
  maxHeight=maxSize;

  // texture for osc renderer
  if (glVer==3) {
    C(glGenTextures(1,&oscDataTex));
#ifdef USE_GLES
    C(glBindTexture(GL_TEXTURE_2D,oscDataTex));
    C(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST));
    C(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST));
    C(glTexImage2D(GL_TEXTURE_2D,0,GL_RED_EXT,2048,1,0,GL_RED_EXT,GL_FLOAT,NULL));
#else
    C(glBindTexture(GL_TEXTURE_1D,oscDataTex));
    C(glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER,GL_NEAREST));
    C(glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER,GL_NEAREST));
    C(glTexImage1D(GL_TEXTURE_1D,0,GL_RED,2048,0,GL_RED,GL_FLOAT,NULL));
#endif
    C(furActiveTexture(GL_TEXTURE0));
  }

  // create shaders
#ifdef USE_GLES
  if ((sh_wipe_have=createShader(sh_wipe_srcV_ES2,sh_wipe_srcF_ES2,sh_wipe_vertex,sh_wipe_fragment,sh_wipe_program,sh_wipe_attrib))==true) {
    sh_wipe_uAlpha=furGetUniformLocation(sh_wipe_program,"uAlpha");
  }

  if ((sh_oscRender_have=createShader(sh_oscRender_srcV,sh_oscRender_srcF,sh_oscRender_vertex,sh_oscRender_fragment,sh_oscRender_program,sh_oscRender_attrib))==true) {
    sh_oscRender_uColor=furGetUniformLocation(sh_oscRender_program,"uColor");
    sh_oscRender_uLineWidth=furGetUniformLocation(sh_oscRender_program,"uLineWidth");
    sh_oscRender_uResolution=furGetUniformLocation(sh_oscRender_program,"uResolution");
    sh_oscRender_oscVal=furGetUniformLocation(sh_oscRender_program,"oscVal");
  }
#else
  if (glVer==3) {
    if ((sh_wipe_have=createShader(sh_wipe_srcV_130,sh_wipe_srcF_130,sh_wipe_vertex,sh_wipe_fragment,sh_wipe_program,sh_wipe_attrib))==true) {
      sh_wipe_uAlpha=furGetUniformLocation(sh_wipe_program,"uAlpha");
    }

    if ((sh_oscRender_have=createShader(sh_oscRender_srcV,sh_oscRender_srcF,sh_oscRender_vertex,sh_oscRender_fragment,sh_oscRender_program,sh_oscRender_attrib))==true) {
      sh_oscRender_uColor=furGetUniformLocation(sh_oscRender_program,"uColor");
      sh_oscRender_uLineWidth=furGetUniformLocation(sh_oscRender_program,"uLineWidth");
      sh_oscRender_uResolution=furGetUniformLocation(sh_oscRender_program,"uResolution");
      sh_oscRender_oscVal=furGetUniformLocation(sh_oscRender_program,"oscVal");
    }
  } else {
    if ((sh_wipe_have=createShader(sh_wipe_srcV_110,sh_wipe_srcF_110,sh_wipe_vertex,sh_wipe_fragment,sh_wipe_program,sh_wipe_attrib))==true) {
      sh_wipe_uAlpha=furGetUniformLocation(sh_wipe_program,"uAlpha");
    }

    sh_oscRender_have=false;
  }
#endif

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

void FurnaceGUIRenderGL::setVersion(unsigned char ver) {
  glVer=ver;
  if (glVer==3) {
    backendName="OpenGL 3.0";
  } else if (glVer==2) {
    backendName="OpenGL 2.0";
  } else {
    backendName="OpenGL BUG.REPORT";
  }
}
