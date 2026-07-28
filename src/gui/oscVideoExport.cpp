/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
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

// oscilloscope video export: per-channel oscilloscope rendered to video via
// ffmpeg pipe. waveform extraction logic ported from chanOsc.cpp.

#include "gui.h"
#include "../ta-log.h"
#include "../engine/sfWrapper.h"
#include "misc/cpp/imgui_stdlib.h"
#include <cfloat>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>
#include <chrono>
#include <imgui.h>
#include <fmt/printf.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#ifdef HAVE_SNDFILE
#include <sndfile.h>
#endif

#ifdef _WIN32
# define FURNACE_POPEN  _popen
# define FURNACE_PCLOSE _pclose
# define PIPE_WRITE_MODE "wb"   // binary mode to avoid CR LF translation on Windows
# define NULL_REDIRECT "2>NUL"
#else
# define FURNACE_POPEN  popen
# define FURNACE_PCLOSE pclose
# define PIPE_WRITE_MODE "w"
# define NULL_REDIRECT "2>/dev/null"
#endif

struct OscSoftFont {
  FT_Library ftLib;
  FT_Face ftFace;
  std::vector<unsigned char> data; // raw font bytes, must outlive the FT face
  bool loaded;

  OscSoftFont(): ftLib(NULL),ftFace(NULL),loaded(false) {}
  ~OscSoftFont() { unload(); }

  bool load(const unsigned char* src, size_t sz) {
    unload();
    if (!src||sz==0) return false;
    data.assign(src,src+sz);
    if (FT_Init_FreeType(&ftLib)) { ftLib=NULL; return false; }
    if (FT_New_Memory_Face(ftLib,data.data(),(FT_Long)data.size(),0,&ftFace)) {
      FT_Done_FreeType(ftLib); ftLib=NULL; return false;
    }
    loaded=true;
    return true;
  }

  void setSize(float pixelHeight) {
    if (!loaded) return;
    if (pixelHeight<1.0f) pixelHeight=1.0f;
    FT_Set_Char_Size(ftFace,0,(FT_F26Dot6)(pixelHeight*64.0f),72,72);
  }

  void unload() {
    if (ftFace) { FT_Done_Face(ftFace); ftFace=NULL; }
    if (ftLib)  { FT_Done_FreeType(ftLib); ftLib=NULL; }
    loaded=false;
    data.clear();
  }

  int lineHeight() {
    if (!loaded) return 0;
    return (int)(ftFace->size->metrics.height>>6);
  }

  int ascender() {
    if (!loaded) return 0;
    return (int)(ftFace->size->metrics.ascender>>6);
  }

  void drawText(struct PixBuf& pb, int x, int y, const char* str, unsigned int color, int clipX2=-1);
};

struct PixBuf {
  int w, h;
  std::vector<unsigned int> px; // RGBA packed, R in byte 0 (matches ffmpeg rawvideo rgba)

  void init(int _w, int _h) {
    w=_w; h=_h;
    px.assign(w*h, 0xff000000u);
  }

  void clear(unsigned int color=0xff000000u) {
    std::fill(px.begin(), px.end(), color);
  }

  inline void setPixel(int x, int y, unsigned int color) {
    if ((unsigned)x>=(unsigned)w||(unsigned)y>=(unsigned)h) return;
    px[y*w+x]=color;
  }

  inline void blendPixel(int x, int y, unsigned int color, float a) {
    if ((unsigned)x>=(unsigned)w||(unsigned)y>=(unsigned)h) return;
    unsigned int dst=px[y*w+x];
    float ia=1.0f-a;
    unsigned char sr=color&0xff, sg=(color>>8)&0xff, sb=(color>>16)&0xff;
    unsigned char dr=dst&0xff, dg=(dst>>8)&0xff, db=(dst>>16)&0xff;
    px[y*w+x]=((unsigned int)(unsigned char)(sr*a+dr*ia))
      |(((unsigned int)(unsigned char)(sg*a+dg*ia))<<8)
      |(((unsigned int)(unsigned char)(sb*a+db*ia))<<16)
      |(0xffu<<24);
  }

  // anti-aliased line. thickness below 1px fades out.
  void drawLine(int x0, int y0, int x1, int y1, unsigned int color, float thick) {
    float alphaMul=1.0f;
    if (thick<1.0f) {
      if (thick<=0.0f) return;
      alphaMul=thick;
      thick=1.0f;
    }
    float r=thick*0.5f;
    float dx=(float)(x1-x0), dy=(float)(y1-y0);
    float len2=dx*dx+dy*dy;
    int minX=(int)floorf(((x0<x1)?(float)x0:(float)x1)-r-1.0f);
    int maxX=(int)ceilf(((x0>x1)?(float)x0:(float)x1)+r+1.0f);
    int minY=(int)floorf(((y0<y1)?(float)y0:(float)y1)-r-1.0f);
    int maxY=(int)ceilf(((y0>y1)?(float)y0:(float)y1)+r+1.0f);
    if (minX<0) minX=0;
    if (minY<0) minY=0;
    if (maxX>=w) maxX=w-1;
    if (maxY>=h) maxY=h-1;
    for (int y=minY; y<=maxY; y++) {
      for (int x=minX; x<=maxX; x++) {
        float qx=(float)(x-x0), qy=(float)(y-y0);
        float t=(len2>0.0f)?((qx*dx+qy*dy)/len2):0.0f;
        if (t<0.0f) t=0.0f;
        if (t>1.0f) t=1.0f;
        float ex=qx-t*dx, ey=qy-t*dy;
        float a=r+0.5f-sqrtf(ex*ex+ey*ey);
        if (a<=0.0f) continue;
        if (a>1.0f) a=1.0f;
        blendPixel(x,y,color,a*alphaMul);
      }
    }
  }

};

void OscSoftFont::drawText(PixBuf& pb, int x, int y, const char* str, unsigned int color, int clipX2) {
  if (!loaded||!str) return;
  if (clipX2<0) clipX2=pb.w;
  unsigned char cr=color&0xff, cg=(color>>8)&0xff, cb=(color>>16)&0xff;
  int asc=ascender();
  int lh=lineHeight(); if (lh<1) lh=1;
  int cx=x;
  for (const char* p=str; *p; p++) {
    unsigned char c=(unsigned char)*p;
    if (c=='\n') { y+=lh; cx=x; continue; }
    if (cx>=clipX2) break;
    FT_UInt gi=FT_Get_Char_Index(ftFace,c);
    if (FT_Load_Glyph(ftFace,gi,FT_LOAD_RENDER)) continue;
    FT_GlyphSlot slot=ftFace->glyph;
    int bw=(int)slot->bitmap.width, bh=(int)slot->bitmap.rows;
    int bx=cx+slot->bitmap_left, by=y+asc-slot->bitmap_top;
    for (int row=0; row<bh; row++) {
      for (int col=0; col<bw; col++) {
        unsigned char alpha=slot->bitmap.buffer[row*slot->bitmap.pitch+col];
        if (!alpha) continue;
        int px_=bx+col, py_=by+row;
        if (px_>=clipX2) break;
        unsigned int dst=pb.px[(unsigned)py_<(unsigned)pb.h?(unsigned)py_*pb.w+(unsigned)px_:0];
        float a=alpha/255.0f, ia=1.0f-a;
        unsigned char dr=dst&0xff, dg=(dst>>8)&0xff, db=(dst>>16)&0xff;
        unsigned int out=(unsigned char)(cr*a+dr*ia)
                   |((unsigned char)(cg*a+dg*ia)<<8)
                   |((unsigned char)(cb*a+db*ia)<<16)
                   |((unsigned char)(255*a+((dst>>24)&0xff)*ia)<<24);
        pb.setPixel(px_,py_,out);
      }
    }
    cx+=(int)(slot->advance.x>>6);
  }
}

static inline unsigned int packRGBA(float r, float g, float b, float a=1.0f) {
  return (unsigned char)(r*255)
       | ((unsigned char)(g*255)<<8)
       | ((unsigned char)(b*255)<<16)
       | ((unsigned char)(a*255)<<24);
}
static inline unsigned int imvec4ToU(ImVec4 c) {
  return packRGBA(
    c.x<0?0:c.x>1?1:c.x,
    c.y<0?0:c.y>1?1:c.y,
    c.z<0?0:c.z>1?1:c.z,
    c.w<0?0:c.w>1?1:c.w);
}

String oscVideoFormatLabel(DivEngine* e, FurnaceGUI* gui, int ch, const String& fmt) {
  String text;
  bool inFmt=false;
  for (char j: fmt) {
    if (inFmt) {
      char buf[64];
      switch (j) {
        case 'c': text+=e->getChannelName(ch); break;
        case 'C': text+=e->getChannelShortName(ch); break;
        case 'd': snprintf(buf,64,"%d",ch); text+=buf; break;
        case 'D': snprintf(buf,64,"%d",ch+1); text+=buf; break;
        case 'i': { DivChannelState* s=e->getChanState(ch); if(s) text+=e->getIns(s->lastIns)->name; break; }
        case 'I': { DivChannelState* s=e->getChanState(ch); if(s){snprintf(buf,64,"%d",s->lastIns);text+=buf;} break; }
        case 'x': { DivChannelState* s=e->getChanState(ch); if(s){if(s->lastIns<0){text+="??";}else{snprintf(buf,64,"%.2X",s->lastIns);text+=buf;}} break; }
        case 's': text+=e->getSystemName(e->song.sysOfChan[ch]); break;
        case 'p': text+=gui->getSystemPartNumber(e->song.sysOfChan[ch],e->song.systemFlags[e->song.dispatchOfChan[ch]]); break;
        case 'S': { char buf[64]; snprintf(buf,64,"%d",e->song.dispatchOfChan[ch]); text+=buf; break; }
        case 'v': { DivChannelState* s=e->getChanState(ch); if(s){snprintf(buf,64,"%d",s->volume>>8);text+=buf;} break; }
        case 'V': { DivChannelState* s=e->getChanState(ch); if(s){double vm=s->volMax>>8;if(vm<1)vm=1;snprintf(buf,64,"%.1f%%",((double)(s->volume>>8)/vm)*100);text+=buf;} break; }
        case 'b': { DivChannelState* s=e->getChanState(ch); if(s){snprintf(buf,64,"%.2X",s->volume>>8);text+=buf;} break; }
        case 'n': { DivChannelState* s=e->getChanState(ch); if(!s||!s->keyOn){text+="---";}else{text+=gui->noteName(s->note);} break; }
        case 'l': text+='\n'; break;
        case '%': text+='%'; break;
        default: text+='%'; text+=j; break;
      }
      inFmt=false;
    } else {
      if (j=='%') inFmt=true; else text+=j;
    }
  }
  return text;
}

#define OSC_VIDEO_FFT_SIZE 4096

struct VideoOscState {
  float dcOff;
  double* fftIn;
  fftw_complex* fftOut;
  double* fftCorr;
  fftw_plan fftPlan;
  fftw_plan fftPlanI;
  bool fftReady;

  VideoOscState():
    dcOff(0.0f), fftIn(NULL), fftOut(NULL), fftCorr(NULL),
    fftPlan(NULL), fftPlanI(NULL), fftReady(false) {}

  VideoOscState(VideoOscState&& o):
    dcOff(o.dcOff), fftIn(o.fftIn), fftOut(o.fftOut), fftCorr(o.fftCorr),
    fftPlan(o.fftPlan), fftPlanI(o.fftPlanI), fftReady(o.fftReady) {
    o.fftIn=NULL; o.fftOut=NULL; o.fftCorr=NULL;
    o.fftPlan=NULL; o.fftPlanI=NULL; o.fftReady=false;
  }

  VideoOscState(const VideoOscState&)=delete;
  VideoOscState& operator=(const VideoOscState&)=delete;

  ~VideoOscState() {
    if (fftPlan) fftw_destroy_plan(fftPlan);
    if (fftPlanI) fftw_destroy_plan(fftPlanI);
    if (fftIn) fftw_free(fftIn);
    if (fftOut) fftw_free(fftOut);
    if (fftCorr) fftw_free(fftCorr);
  }
};

// persists across preview frames so DC correction and FFT plans behave
// like the channel oscilloscope
struct OscVideoPreviewState {
  std::vector<VideoOscState> chans;
};

static void renderFrame(PixBuf& pb, DivEngine* e, FurnaceGUI* gui,
    float windowSizeMs, float amplify, float lineThick,
    int colorMode, unsigned int solidColorU, unsigned int textColorU,
    const String& textFmt, int centerStrat, bool waveCorr,
    const std::vector<unsigned int>& chanColors,
    std::vector<VideoOscState>& chanState,
    OscSoftFont* sf=NULL) {

  int totalChans=e->getTotalChannelCount();

  struct OscEntry { DivDispatchOscBuffer* buf; int ch; };
  std::vector<OscEntry> entries;
  for (int i=0; i<totalChans; i++) {
    DivDispatchOscBuffer* buf=e->getOscBuffer(i);
    if (buf && e->curSubSong->chanShowChanOsc[i])
      entries.push_back({buf,i});
  }
  if (entries.empty()) return;

  int cols=(int)sqrtf((float)entries.size());
  if (cols<1) cols=1;
  if (cols>64) cols=64;
  int rows=((int)entries.size()+(cols-1))/cols;

  int cellW=pb.w/cols;
  int cellH=pb.h/rows;

  const unsigned int sepColor=0xff3a3a3a;
  for (int r=1; r<rows; r++)
    pb.drawLine(0,r*cellH,pb.w-1,r*cellH,sepColor,1.0f);
  for (int c=1; c<cols; c++)
    pb.drawLine(c*cellW,0,c*cellW,pb.h-1,sepColor,1.0f);

  float thick=lineThick;

  float oscTex[1024];

  for (int ei=0; ei<(int)entries.size(); ei++) {
    DivDispatchOscBuffer* buf=entries[ei].buf;
    int ch=entries[ei].ch;

    int col=ei%cols, row=ei/cols;
    int x0=col*cellW+1, y0=row*cellH+2;
    int x1=(col+1)*cellW-1, y1=(row+1)*cellH-2;
    int cw=x1-x0; if (cw<2) cw=2;
    int cH=y1-y0; if (cH<2) cH=2;
    int cy0=y0+cH/2;

    int precision=cw;
    if (precision<1) precision=1;
    if (precision>1024) precision=1024;

    int displaySize=(int)(65536.0f*(windowSizeMs/1000.0f));

    unsigned short rawNeedle=(unsigned short)(buf->needle>>16);
    unsigned short baseNeedle=(unsigned short)(rawNeedle-displaySize);
    unsigned short needle=baseNeedle;

    if (waveCorr) {
      VideoOscState& vstate=chanState[ch];
      if (!vstate.fftReady) {
        vstate.fftIn=(double*)fftw_malloc(OSC_VIDEO_FFT_SIZE*sizeof(double));
        vstate.fftOut=(fftw_complex*)fftw_malloc(OSC_VIDEO_FFT_SIZE*sizeof(fftw_complex));
        vstate.fftCorr=(double*)fftw_malloc(OSC_VIDEO_FFT_SIZE*sizeof(double));
        vstate.fftPlan=fftw_plan_dft_r2c_1d(OSC_VIDEO_FFT_SIZE,vstate.fftIn,vstate.fftOut,FFTW_ESTIMATE);
        vstate.fftPlanI=fftw_plan_dft_c2r_1d(OSC_VIDEO_FFT_SIZE,vstate.fftOut,vstate.fftCorr,FFTW_ESTIMATE);
        if (vstate.fftPlan&&vstate.fftPlanI&&vstate.fftIn&&vstate.fftOut&&vstate.fftCorr)
          vstate.fftReady=true;
      }
      if (vstate.fftReady) {
        int displaySize2=displaySize*2;
        short fftLast=0;
        bool loudEnough=false;
        memset(vstate.fftIn,0,OSC_VIDEO_FFT_SIZE*sizeof(double));
        int fk=0;
        for (unsigned short j=rawNeedle-(unsigned short)displaySize2; j!=rawNeedle; j++,fk++) {
          const int kIn=(fk*OSC_VIDEO_FFT_SIZE)/displaySize2;
          if (kIn>=OSC_VIDEO_FFT_SIZE) break;
          const short nd=buf->data[j];
          if (nd!=-1) fftLast=nd;
          vstate.fftIn[kIn]=(double)fftLast/32768.0;
          if (fabs(vstate.fftIn[kIn])>0.001) loudEnough=true;
          vstate.fftIn[kIn]*=0.55-0.45*cos(M_PI*(double)kIn/(double)(OSC_VIDEO_FFT_SIZE>>1));
        }
        if (loudEnough) {
          fftw_execute(vstate.fftPlan);
          for (int j=0; j<OSC_VIDEO_FFT_SIZE; j++) {
            vstate.fftOut[j][0]/=OSC_VIDEO_FFT_SIZE;
            vstate.fftOut[j][1]/=OSC_VIDEO_FFT_SIZE;
            vstate.fftOut[j][0]=vstate.fftOut[j][0]*vstate.fftOut[j][0]+vstate.fftOut[j][1]*vstate.fftOut[j][1];
            vstate.fftOut[j][1]=0;
          }
          vstate.fftOut[0][0]=vstate.fftOut[0][1]=0;
          vstate.fftOut[1][0]=vstate.fftOut[1][1]=0;
          fftw_execute(vstate.fftPlanI);
          for (int j=0; j<(OSC_VIDEO_FFT_SIZE>>1); j++)
            vstate.fftCorr[j]*=1.0-((double)j/(double)(OSC_VIDEO_FFT_SIZE<<1));
          double candL=DBL_MAX, candH=DBL_MIN;
          int waveLen=OSC_VIDEO_FFT_SIZE-1, waveLenBottom=0;
          for (int j=(OSC_VIDEO_FFT_SIZE>>2); j>2; j--)
            if (vstate.fftCorr[j]<candL) { candL=vstate.fftCorr[j]; waveLenBottom=j; }
          for (int j=(OSC_VIDEO_FFT_SIZE>>1)-1; j>waveLenBottom; j--)
            if (vstate.fftCorr[j]>candH) { candH=vstate.fftCorr[j]; waveLen=j; }
          if (waveLen<(OSC_VIDEO_FFT_SIZE-32)) {
            double waveLenD=(double)waveLen*(double)displaySize2/(double)OSC_VIDEO_FFT_SIZE;
            double dft[2]={0.0,0.0};
            fftLast=0;
            for (int j=(int)rawNeedle-1-displaySize-(int)waveLenD, kk=-(displaySize>>1);
                 (double)kk<waveLenD; j++,kk++) {
              if (buf->data[j&0xffff]!=-1) fftLast=buf->data[j&0xffff];
              if (kk<0) continue;
              double one=(double)fftLast/32768.0;
              double two=(double)kk*(-2.0*M_PI)/waveLenD;
              dft[0]+=one*cos(two);
              dft[1]+=one*sin(two);
            }
            double phase=0.5+atan2(dft[1],dft[0])/(2.0*M_PI);
            needle-=(unsigned short)(unsigned int)(phase*waveLenD);
          }
        }
      }
    }

    float& dcOff=chanState[ch].dcOff;
    float minLevel=1.0f, maxLevel=-1.0f;
    float y=0.0f;

    for (int j=0; j<32768; j++) {
      const short y_s=buf->data[(needle-j)&0xffff];
      if (y_s!=-1) { y=(float)y_s/32768.0f; break; }
    }

    if (centerStrat==0) {
      dcOff=0.0f;
    } else if (centerStrat==1) {
      float y1=y;
      if (minLevel>y1) minLevel=y1;
      if (maxLevel<y1) maxLevel=y1;
      for (unsigned short j=needle; j!=((needle+displaySize)&0xffff); j++) {
        const short y_s=buf->data[j];
        if (y_s!=-1) {
          y1=(float)y_s/32768.0f;
          if (minLevel>y1) minLevel=y1;
          if (maxLevel<y1) maxLevel=y1;
        }
      }
      dcOff=(minLevel+maxLevel)*0.5f;
    }

    if (displaySize<precision) {
      for (int j=0; j<precision; j++) {
        const short y_s=buf->data[(unsigned short)(needle+(j*displaySize/precision))];
        if (y_s!=-1) {
          y=(float)y_s/32768.0f;
          if (minLevel>y) minLevel=y;
          if (maxLevel<y) maxLevel=y;
        }
        float yOut=y-dcOff;
        if (centerStrat==2) dcOff+=(y-dcOff)*0.001f;
        if (yOut<-0.5f) yOut=-0.5f;
        if (yOut>0.5f) yOut=0.5f;
        yOut*=amplify*2.0f;
        oscTex[j]=yOut;
      }
    } else {
      int k=0;
      for (unsigned short j=needle; j!=((unsigned short)(needle+displaySize)); j++,k++) {
        const short y_s=buf->data[j];
        const int kTex=(k*precision)/displaySize;
        if (kTex>=precision) break;
        if (y_s!=-1) {
          y=(float)y_s/32768.0f;
          if (minLevel>y) minLevel=y;
          if (maxLevel<y) maxLevel=y;
        }
        float yOut=y-dcOff;
        if (centerStrat==2) dcOff+=(y-dcOff)*0.001f;
        if (yOut<-0.5f) yOut=-0.5f;
        if (yOut>0.5f) yOut=0.5f;
        yOut*=amplify*2.0f;
        oscTex[kTex]=yOut;
      }
    }

    unsigned int color=(colorMode==1 && ch<(int)chanColors.size()) ? chanColors[ch] : solidColorU;

    pb.drawLine(x0,cy0,x1,cy0,0xff242424,1.0f);

    int prevPX=x0, prevPY=cy0;
    for (int j=0; j<precision; j++) {
      int px_=x0+(j*cw)/precision;
      int py_=cy0-(int)(oscTex[j]*(float)cH*0.5f);
      if (j>0) pb.drawLine(prevPX,prevPY,px_,py_,color,thick);
      prevPX=px_; prevPY=py_;
    }

    if (!textFmt.empty() && sf && sf->loaded) {
      String label=oscVideoFormatLabel(e,gui,ch,textFmt);
      sf->drawText(pb, x0+2, y0+2, label.c_str(), textColorU, x1);
    }
  }
}

void FurnaceGUI::clearOscVideoSoftFont() {
  if (oscVideoSoftFont!=NULL) { delete oscVideoSoftFont; oscVideoSoftFont=NULL; }
  if (oscVideoPreviewState!=NULL) { delete oscVideoPreviewState; oscVideoPreviewState=NULL; }
}

void FurnaceGUI::detectOscVideoFfmpeg() {
  String testExe=oscVideoExport.ffmpegPath.empty() ? String("ffmpeg") : oscVideoExport.ffmpegPath;
#ifdef _WIN32
  String testCmd=fmt::sprintf("\"%s\" -version >NUL 2>NUL",testExe);
#else
  String testCmd=fmt::sprintf("\"%s\" -version >/dev/null 2>/dev/null",testExe);
#endif
  oscVideoFfmpegFound=(system(testCmd.c_str())==0);

  if (oscVideoFfmpegFound && oscVideoExport.ffmpegPath.empty()) {
#ifdef _WIN32
    FILE* p=FURNACE_POPEN("where ffmpeg 2>NUL","r");
#else
    FILE* p=FURNACE_POPEN("command -v ffmpeg 2>/dev/null","r");
#endif
    if (p!=NULL) {
      char resolved[4096];
      if (fgets(resolved,sizeof(resolved),p)!=NULL) {
        size_t l=strlen(resolved);
        while (l>0 && (resolved[l-1]=='\n' || resolved[l-1]=='\r')) resolved[--l]=0;
        if (l>0) oscVideoExport.ffmpegPath=resolved;
      }
      FURNACE_PCLOSE(p);
    }
  }
}

void FurnaceGUI::drawOscVideoProgress() {
  centerNextWindow(_("Osc Video Render"),canvasW,canvasH);
  if (ImGui::BeginPopupModal(_("Osc Video Render"),NULL,
      ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|
      ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_AlwaysAutoResize)) {
    WAKE_UP;

    if (!oscVideoExporting) {
      ImGui::CloseCurrentPopup();
    } else if (oscVideoCombining) {
      ImGui::Text(_("Combining..."));
      ImGui::ProgressBar(-1.0f*(float)ImGui::GetTime(),ImVec2(320.0f*dpiScale,0),"");
    } else {
      ImGui::Text(_("Please wait..."));
      double elapsed=(double)oscVideoCurrentFrame.load()/(double)oscVideoExport.fps;
      if (oscVideoTotalTime>0.0) {
        float frac=(float)(elapsed/oscVideoTotalTime);
        if (frac<0.0f) frac=0.0f;
        if (frac>1.0f) frac=1.0f;
        ImGui::ProgressBar(frac,ImVec2(320.0f*dpiScale,0),fmt::sprintf("%.2f%%",frac*100.0f).c_str());
      } else {
        ImGui::ProgressBar(-1.0f*(float)ImGui::GetTime(),ImVec2(320.0f*dpiScale,0),"");
      }
      if (ImGui::Button(_("Abort"),ImVec2(320.0f*dpiScale,0))) {
        oscVideoAbort=true;
      }
    }

    ImGui::EndPopup();
  }
}

void FurnaceGUI::drawExportOscVideo(bool onWindow) {
  exitDisabledTimer=1;

  ImGui::SeparatorText(_("Video"));
  if (ImGui::InputInt(_("Width##ovw"),&oscVideoExport.width,16,160)) {
    oscVideoExport.width=oscVideoExport.width&~1; // must be even for libx264
    if (oscVideoExport.width<64) oscVideoExport.width=64;
    if (oscVideoExport.width>7680) oscVideoExport.width=7680;
  }
  if (ImGui::InputInt(_("Height##ovh"),&oscVideoExport.height,16,160)) {
    oscVideoExport.height=oscVideoExport.height&~1; // must be even for libx264
    if (oscVideoExport.height<64) oscVideoExport.height=64;
    if (oscVideoExport.height>4320) oscVideoExport.height=4320;
  }
  if (ImGui::InputInt(_("FPS##ovf"),&oscVideoExport.fps,1,10)) {
    if (oscVideoExport.fps<1) oscVideoExport.fps=1;
    if (oscVideoExport.fps>120) oscVideoExport.fps=120;
  }
  ImGui::Text(_("Format:"));
  ImGui::SameLine();
  ImGui::RadioButton(_("MKV (Vorbis)##ovfmt"),&oscVideoExport.outputFormat,0);
  ImGui::SameLine();
  ImGui::RadioButton(_("MP4 (AAC)##ovfmt"),&oscVideoExport.outputFormat,1);
  if (oscVideoExport.outputFormat==1) {
    bool limitSize=(oscVideoExport.targetSizeMB>0);
    if (ImGui::Checkbox(_("Target file size##ovlimit"),&limitSize))
      oscVideoExport.targetSizeMB=limitSize?10:0;
    if (limitSize) {
      ImGui::SameLine();
      ImGui::SetNextItemWidth(100.0f*dpiScale);
      if (ImGui::InputInt(_("MB##ovtargetmb"),&oscVideoExport.targetSizeMB,1,10)) {
        if (oscVideoExport.targetSizeMB<1) oscVideoExport.targetSizeMB=1;
        if (oscVideoExport.targetSizeMB>100000) oscVideoExport.targetSizeMB=100000;
      }
    }
  }

  ImGui::SeparatorText(_("Audio"));
  {
    static const int srVals[]={22050,32000,44100,48000,88200,96000};
    static const char* srLabels[]={"22050 Hz","32000 Hz","44100 Hz","48000 Hz","88200 Hz","96000 Hz"};
    int srIdx=0;
    for (int i=0; i<6; i++) if (oscVideoExport.sampleRate>=srVals[i]) srIdx=i;
    if (ImGui::Combo(_("Sample Rate##ovsr"),&srIdx,srLabels,6))
      oscVideoExport.sampleRate=srVals[srIdx];
  }
  {
    static const int brVals[]={64,80,96,112,128,160,192,224,256,320,500};
    static const char* brLabels[]={"64 kbps","80 kbps","96 kbps","112 kbps","128 kbps","160 kbps","192 kbps","224 kbps","256 kbps","320 kbps","500 kbps"};
    int brIdx=6; // default 192
    for (int i=0; i<11; i++) if (oscVideoExport.audioBitrate>=brVals[i]) brIdx=i;
    if (ImGui::Combo(_("Bitrate##ovab"),&brIdx,brLabels,11))
      oscVideoExport.audioBitrate=brVals[brIdx];
  }

  if (oscVideoExport.outputFormat==1 && oscVideoExport.targetSizeMB==0) {
    if (ImGui::SliderInt(_("Video Quality (CRF)##ovcrf"),&oscVideoExport.videoCRF,0,51)) {
      if (oscVideoExport.videoCRF<0) oscVideoExport.videoCRF=0;
      if (oscVideoExport.videoCRF>51) oscVideoExport.videoCRF=51;
    }
  }

  ImGui::SeparatorText(_("Waveform"));
  if (ImGui::SliderFloat(_("Line Thickness##ovlt"),&oscVideoExport.lineSize,0.5f,8.0f,"%.1f")) {
    if (oscVideoExport.lineSize<0.5f) oscVideoExport.lineSize=0.5f;
    if (oscVideoExport.lineSize>8.0f) oscVideoExport.lineSize=8.0f;
  }
  ImGui::SeparatorText(_("Text"));
  if (ImGui::SliderFloat(_("Text Scale##ovts"),&oscVideoExport.textScale,0.5f,8.0f,"%.1fx")) {
    if (oscVideoExport.textScale<0.5f) oscVideoExport.textScale=0.5f;
    if (oscVideoExport.textScale>8.0f) oscVideoExport.textScale=8.0f;
  }

  ImGui::SeparatorText(_("Preview"));
  float aspect=(float)oscVideoExport.width/(float)oscVideoExport.height;
  float maxPreviewW=ImGui::GetContentRegionAvail().x;
  float previewH=maxPreviewW/aspect;
  if (previewH<60.0f) { previewH=60.0f; }
  if (previewH>280.0f*dpiScale) { previewH=280.0f*dpiScale; }
  float previewW=previewH*aspect;
  if (previewW>maxPreviewW) { previewW=maxPreviewW; previewH=previewW/aspect; }

  int previewPxW=((int)previewW)&~1;
  int previewPxH=((int)previewH)&~1;
  if (previewPxW<2) previewPxW=2;
  if (previewPxH<2) previewPxH=2;

  if (oscVideoPreviewTex==NULL || oscVideoPreviewTexW!=previewPxW || oscVideoPreviewTexH!=previewPxH) {
    if (oscVideoPreviewTex!=NULL) rend->destroyTexture(oscVideoPreviewTex);
    oscVideoPreviewTex=rend->createTexture(true,previewPxW,previewPxH,false,bestTexFormat);
    oscVideoPreviewTexW=previewPxW;
    oscVideoPreviewTexH=previewPxH;
  }

  float contentX=ImGui::GetCursorPosX();
  float offsetX=(ImGui::GetContentRegionAvail().x-previewW)*0.5f;
  if (offsetX>0.0f) ImGui::SetCursorPosX(contentX+offsetX);

  if (oscVideoPreviewTex!=NULL) {
    int totalChans=e->getTotalChannelCount();
    std::vector<unsigned int> chanColors((size_t)totalChans);
    for (int i=0; i<totalChans; i++) chanColors[i]=imvec4ToU(channelColor(i));
    unsigned int solidColorU=imvec4ToU(chanOscColor);
    unsigned int textColorU=imvec4ToU(chanOscTextColor);
    float previewLineSize=oscVideoExport.lineSize*(float)previewPxH/(float)oscVideoExport.height;

    if (oscVideoSoftFont==NULL && !mainFontRawData.empty()) {
      oscVideoSoftFont=new OscSoftFont();
      if (!oscVideoSoftFont->load(mainFontRawData.data(),mainFontRawData.size())) {
        delete oscVideoSoftFont; oscVideoSoftFont=NULL;
      }
    }
    if (oscVideoSoftFont && oscVideoSoftFont->loaded) {
      float px=oscVideoExport.textScale*(float)previewPxH/60.0f;
      if (px<4.0f) px=4.0f;
      oscVideoSoftFont->setSize(px);
    }

    PixBuf pb;
    pb.init(previewPxW,previewPxH);
    pb.clear(0xff000000u);
    if (oscVideoPreviewState==NULL) oscVideoPreviewState=new OscVideoPreviewState();
    if ((int)oscVideoPreviewState->chans.size()!=totalChans) {
      oscVideoPreviewState->chans.clear();
      oscVideoPreviewState->chans.resize((size_t)totalChans);
    }
    renderFrame(pb,e,this,
      chanOscWindowSize,chanOscAmplify,previewLineSize,
      chanOscColorMode,solidColorU,textColorU,
      chanOscTextFormat,chanOscCenterStrat,chanOscWaveCorr,
      chanColors,oscVideoPreviewState->chans,oscVideoSoftFont);

    rend->updateTexture(oscVideoPreviewTex,pb.px.data(),previewPxW*4);
    ImGui::Image(rend->getTextureID(oscVideoPreviewTex),
      ImVec2(previewW,previewH),
      ImVec2(0,0),
      ImVec2(rend->getTextureU(oscVideoPreviewTex),rend->getTextureV(oscVideoPreviewTex)));
  } else {
    ImGui::GetWindowDrawList()->AddRectFilled(
      ImGui::GetCursorScreenPos(),
      ImVec2(ImGui::GetCursorScreenPos().x+previewW,ImGui::GetCursorScreenPos().y+previewH),
      0xff000000);
    ImGui::Dummy(ImVec2(previewW,previewH));
  }

  ImGui::SeparatorText(_("ffmpeg"));
  {
    if (!oscVideoFfmpegChecked) {
      detectOscVideoFfmpeg();
      oscVideoFfmpegChecked=true;
    }
    float detectW=80.0f*dpiScale;
    float browseW=ImGui::CalcTextSize("...").x+ImGui::GetStyle().FramePadding.x*2.0f;
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x-detectW-browseW-ImGui::GetStyle().ItemSpacing.x*2.0f);
    if (ImGui::InputTextWithHint("##ovffmpegpath",_("path to ffmpeg (leave blank for system PATH)"),&oscVideoExport.ffmpegPath))
      oscVideoFfmpegFound=false;
    ImGui::SameLine();
    if (ImGui::Button("...##ovffbrowse"))
      openFileDialog(GUI_FILE_OSC_VIDEO_FFMPEG);
    ImGui::SameLine();
    if (ImGui::Button(_("Detect"),ImVec2(detectW,0)))
      detectOscVideoFfmpeg();
    if (oscVideoFfmpegFound) {
      ImGui::TextColored(uiColors[GUI_COLOR_TOGGLE_ON],_("ffmpeg found"));
    } else {
      ImGui::TextColored(uiColors[GUI_COLOR_WARNING],_("not detected (click Detect to check)"));
    }
  }

  ImGui::Separator();
  if (onWindow) {
    if (ImGui::Button(_("Cancel"),ImVec2(200.0f*dpiScale,0))) ImGui::CloseCurrentPopup();
    ImGui::SameLine();
  }

  if (oscVideoExporting||!oscVideoFfmpegFound) ImGui::BeginDisabled();
  if (ImGui::Button(_("Export"),ImVec2(200.0f*dpiScale,0))) {
    openFileDialog(GUI_FILE_EXPORT_OSC_VIDEO);
    if (onWindow) ImGui::CloseCurrentPopup();
  }
  if (oscVideoExporting||!oscVideoFfmpegFound) ImGui::EndDisabled();
}

void FurnaceGUI::runOscVideoExport(const String& path) {
  int W=oscVideoExport.width, H=oscVideoExport.height, FPS=oscVideoExport.fps;
  int audioBr=oscVideoExport.audioBitrate;
  oscVideoAbort=false;

  int rate=(int)e->getAudioDescGot().rate;
  if (rate<=0) rate=44100;

  logI("OscVideo: %dx%d @%dfps rate=%d",W,H,FPS,rate);

  std::vector<unsigned char> fontDataCopy=mainFontRawData;
  OscSoftFont threadSf;
  if (!fontDataCopy.empty() && threadSf.load(fontDataCopy.data(),fontDataCopy.size())) {
    float px=oscVideoExport.textScale*(float)H/60.0f;
    if (px<4.0f) px=4.0f;
    threadSf.setSize(px);
  }

  String ffmpegExe=oscVideoExport.ffmpegPath.empty() ? String("ffmpeg") : oscVideoExport.ffmpegPath;
  String vidTmp=String(path)+".vidtmp.mkv";
  String audTmp=String(path)+".audtmp.wav";

  int crf=oscVideoExport.videoCRF;
  String vcmd=fmt::sprintf(
    "\"%s\" -y -f rawvideo -pix_fmt rgba -s %dx%d -r %d -i pipe:0 "
    "-c:v libx264 -crf %d -preset fast -pix_fmt yuv420p -f matroska \"%s\" %s",
    ffmpegExe,W,H,FPS,crf,vidTmp,NULL_REDIRECT);
  FILE* vpipe=FURNACE_POPEN(vcmd.c_str(),PIPE_WRITE_MODE);
  if (!vpipe) {
    logE("OscVideo: popen failed, is ffmpeg installed?");
    oscVideoCombining=false;
    oscVideoExporting=false;
    return;
  }

  int totalChans=e->getTotalChannelCount();
  std::vector<unsigned int> chanColors((size_t)totalChans);
  for (int i=0; i<totalChans; i++) chanColors[i]=imvec4ToU(channelColor(i));
  unsigned int solidColorU=imvec4ToU(chanOscColor);
  unsigned int textColorU=imvec4ToU(chanOscTextColor);

  float windowSizeMs=chanOscWindowSize;
  float amplify=chanOscAmplify;
  float lineThick=oscVideoExport.lineSize;
  int colorMode=chanOscColorMode;
  String textFmt=chanOscTextFormat.c_str();
  int centerStrat=chanOscCenterStrat;
  bool waveCorr=chanOscWaveCorr;

  std::vector<VideoOscState> chanState((size_t)totalChans);

  PixBuf pb;
  pb.init(W,H);

  std::vector<float> audioSamples;

  int frameCount=0;
  e->renderOscVideo(FPS, 1,
    [&](int /*frameIdx*/, float** audio, int spf) {
      frameCount++;
      oscVideoCurrentFrame.store(frameCount);

      for (int i=0; i<spf; i++) {
        audioSamples.push_back(audio[0][i]);
        audioSamples.push_back(audio[1][i]);
      }

      pb.clear(0xff000000u);
      renderFrame(pb,e,this,windowSizeMs,amplify,lineThick,
                  colorMode,solidColorU,textColorU,textFmt,centerStrat,waveCorr,
                  chanColors,chanState,threadSf.loaded?&threadSf:NULL);
      fwrite(pb.px.data(),4,(size_t)(W*H),vpipe);
    },
    [this]()->bool { return oscVideoAbort.load(); }
  );

  FURNACE_PCLOSE(vpipe);

  if (oscVideoAbort.load()) {
    logI("OscVideo: aborted");
    remove(vidTmp.c_str());
    oscVideoCombining=false;
    oscVideoExporting=false;
    return;
  }

  logI("OscVideo: writing audio (%zu stereo samples)",(size_t)audioSamples.size()/2);

#ifdef HAVE_SNDFILE
  if (!audioSamples.empty()) {
    SF_INFO si;
    memset(&si,0,sizeof(SF_INFO));
    si.samplerate=rate;
    si.channels=2;
    si.format=SF_FORMAT_WAV|SF_FORMAT_FLOAT;
    SFWrapper sfWrap;
    SNDFILE* sf=sfWrap.doOpen(audTmp.c_str(),SFM_WRITE,&si);
    if (sf) {
      sf_writef_float(sf,audioSamples.data(),(sf_count_t)(audioSamples.size()/2));
      sfWrap.doClose();
    } else {
      logE("OscVideo: cannot open audio temp file");
    }
  }
#else
  logE("OscVideo: libsndfile not available, skipping audio");
#endif

  oscVideoCombining=true;
  logI("OscVideo: combining...");
  String ccmd;
  if (oscVideoExport.outputFormat==1) {
    if (oscVideoExport.targetSizeMB>0 && frameCount>0) {
      double dur=(double)frameCount/(double)FPS;
      double targetBits=(double)oscVideoExport.targetSizeMB*1024.0*1024.0*8.0;
      double audioBits=(double)audioBr*1000.0*dur;
      int vbr=(int)((targetBits-audioBits)/(dur*1000.0));
      if (vbr<50) vbr=50;
      logI("OscVideo: target %d MB - %.1fs, video bitrate %d kbps",oscVideoExport.targetSizeMB,dur,vbr);
      ccmd=fmt::sprintf(
        "\"%s\" -y -i \"%s\" -i \"%s\" "
        "-c:v libx264 -b:v %dk -maxrate %dk -bufsize %dk -c:a aac -ar %d -b:a %dk \"%s\" %s",
        ffmpegExe,vidTmp,audTmp,vbr,vbr*2,vbr*4,oscVideoExport.sampleRate,audioBr,String(path),NULL_REDIRECT);
    } else {
      ccmd=fmt::sprintf(
        "\"%s\" -y -i \"%s\" -i \"%s\" "
        "-c:v copy -c:a aac -ar %d -b:a %dk \"%s\" %s",
        ffmpegExe,vidTmp,audTmp,oscVideoExport.sampleRate,audioBr,String(path),NULL_REDIRECT);
    }
  } else {
    // vorbis only supports bitrate mode up to 48kHz. use quality mode, which
    // works at any rate. the presets match the vorbis quality levels (q0-q10).
    static const int vorbisBr[]={64,80,96,112,128,160,192,224,256,320,500};
    int vorbisQ=6;
    for (int i=0; i<11; i++) if (audioBr>=vorbisBr[i]) vorbisQ=i;
    ccmd=fmt::sprintf(
      "\"%s\" -y -i \"%s\" -i \"%s\" "
      "-c:v copy -c:a libvorbis -ar %d -q:a %d \"%s\" %s",
      ffmpegExe,vidTmp,audTmp,oscVideoExport.sampleRate,vorbisQ,String(path),NULL_REDIRECT);
  }
  int ret=system(ccmd.c_str());
  if (ret!=0) logE("OscVideo: combine failed (code %d)",ret);
  else logI("OscVideo: done - %s",path.c_str());

  remove(vidTmp.c_str());
  remove(audTmp.c_str());

  oscVideoCombining=false;
  oscVideoExporting=false;
}
