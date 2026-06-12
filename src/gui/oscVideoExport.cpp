/**
 * Furnace Tracker - oscilloscope video export
 * Per-channel oscilloscope rendered to MP4 via ffmpeg pipe.
 * Waveform extraction logic ported directly from chanOsc.cpp.
 */

#include "gui.h"
#include "../ta-log.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>
#include <string>
#include <chrono>
#include <imgui.h>
#include <fmt/printf.h>
#include <ft2build.h>
#include FT_FREETYPE_H

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

  void drawText(struct PixBuf& pb, int x, int y, const char* str, uint32_t color, int clipX2=-1);
};

struct PixBuf {
  int w, h;
  std::vector<uint32_t> px; // RGBA packed, R in byte 0 (matches ffmpeg rawvideo rgba)

  void init(int _w, int _h) {
    w=_w; h=_h;
    px.assign(w*h, 0xff000000u);
  }

  void clear(uint32_t color=0xff000000u) {
    std::fill(px.begin(), px.end(), color);
  }

  inline void setPixel(int x, int y, uint32_t color) {
    if ((unsigned)x>=(unsigned)w||(unsigned)y>=(unsigned)h) return;
    px[y*w+x]=color;
  }

  void drawLine(int x0, int y0, int x1, int y1, uint32_t color, int thick) {
    int dx=abs(x1-x0), sx=(x0<x1)?1:-1;
    int dy=-abs(y1-y0), sy=(y0<y1)?1:-1;
    int err=dx+dy;
    for (;;) {
      if (thick==0) {
        setPixel(x0,y0,color);
      } else {
        for (int ty=-thick; ty<=thick; ty++)
          for (int tx=-thick; tx<=thick; tx++)
            if (tx*tx+ty*ty<=thick*thick+thick)
              setPixel(x0+tx,y0+ty,color);
      }
      if (x0==x1&&y0==y1) break;
      int e2=2*err;
      if (e2>=dy){err+=dy;x0+=sx;}
      if (e2<=dx){err+=dx;y0+=sy;}
    }
  }

};

void OscSoftFont::drawText(PixBuf& pb, int x, int y, const char* str, uint32_t color, int clipX2) {
  if (!loaded||!str) return;
  if (clipX2<0) clipX2=pb.w;
  uint8_t cr=color&0xff, cg=(color>>8)&0xff, cb=(color>>16)&0xff;
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
        uint8_t alpha=slot->bitmap.buffer[row*slot->bitmap.pitch+col];
        if (!alpha) continue;
        int px_=bx+col, py_=by+row;
        if (px_>=clipX2) break;
        uint32_t dst=pb.px[(unsigned)py_<(unsigned)pb.h?(unsigned)py_*pb.w+(unsigned)px_:0];
        float a=alpha/255.0f, ia=1.0f-a;
        uint8_t dr=dst&0xff, dg=(dst>>8)&0xff, db=(dst>>16)&0xff;
        uint32_t out=(uint8_t)(cr*a+dr*ia)
                   |((uint8_t)(cg*a+dg*ia)<<8)
                   |((uint8_t)(cb*a+db*ia)<<16)
                   |((uint8_t)(255*a+((dst>>24)&0xff)*ia)<<24);
        pb.setPixel(px_,py_,out);
      }
    }
    cx+=(int)(slot->advance.x>>6);
  }
}

static inline uint32_t packRGBA(float r, float g, float b, float a=1.0f) {
  return (uint8_t)(r*255)
       | ((uint8_t)(g*255)<<8)
       | ((uint8_t)(b*255)<<16)
       | ((uint8_t)(a*255)<<24);
}
static inline uint32_t imvec4ToU(ImVec4 c) {
  return packRGBA(
    c.x<0?0:c.x>1?1:c.x,
    c.y<0?0:c.y>1?1:c.y,
    c.z<0?0:c.z>1?1:c.z,
    c.w<0?0:c.w>1?1:c.w);
}

static std::string formatLabel(DivEngine* e, FurnaceGUI* gui, int ch, const std::string& fmt) {
  std::string text;
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
        case 'x': { DivChannelState* s=e->getChanState(ch); if(s){snprintf(buf,64,"%.2X",s->lastIns);text+=buf;} break; }
        case 's': text+=e->getSystemName(e->song.sysOfChan[ch]); break;
        case 'v': { DivChannelState* s=e->getChanState(ch); if(s){snprintf(buf,64,"%d",s->volume>>8);text+=buf;} break; }
        case 'V': { DivChannelState* s=e->getChanState(ch); if(s){double vm=s->volMax>>8;if(vm<1)vm=1;snprintf(buf,64,"%.1f%%",((double)(s->volume>>8)/vm)*100);text+=buf;} break; }
        case 'b': { DivChannelState* s=e->getChanState(ch); if(s){snprintf(buf,64,"%.2X",s->volume>>8);text+=buf;} break; }
        case 'n': { DivChannelState* s=e->getChanState(ch); if(!s||!s->keyOn){text+="---";}else{text+=gui->noteName(s->note+60);} break; }
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

struct VideoOscState {
  float dcOff;
  VideoOscState(): dcOff(0.0f) {}
};

static void renderFrame(PixBuf& pb, DivEngine* e, FurnaceGUI* gui,
    float windowSizeMs, float amplify, float lineThick,
    int colorMode, uint32_t solidColorU, uint32_t textColorU,
    const std::string& textFmt, int centerStrat, bool waveCorr,
    const std::vector<uint32_t>& chanColors,
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

  const uint32_t sepColor=0xff3a3a3a;
  for (int r=1; r<rows; r++)
    pb.drawLine(0,r*cellH,pb.w-1,r*cellH,sepColor,0);
  for (int c=1; c<cols; c++)
    pb.drawLine(c*cellW,0,c*cellW,pb.h-1,sepColor,0);

  int thick=(int)roundf(lineThick*0.5f);
  if (thick<0) thick=0;

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
      short prevS=0;
      for (int k=0; k<32768; k++) {
        short s=buf->data[(unsigned short)(baseNeedle-k)];
        if (s!=-1) { prevS=s; break; }
      }
      for (int j=1; j<displaySize; j++) {
        unsigned short pos=(unsigned short)(baseNeedle-j);
        short s=buf->data[pos];
        if (s==-1) continue;
        if (s<=0 && prevS>0) {
          needle=(unsigned short)(pos+1);
          break;
        }
        prevS=s;
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

    uint32_t color=(colorMode==1 && ch<(int)chanColors.size()) ? chanColors[ch] : solidColorU;

    pb.drawLine(x0,cy0,x1,cy0,0xff242424,0);

    int prevPX=x0, prevPY=cy0;
    for (int j=0; j<precision; j++) {
      int px_=x0+(j*cw)/precision;
      int py_=cy0-(int)(oscTex[j]*(float)cH*0.5f);
      if (j>0) pb.drawLine(prevPX,prevPY,px_,py_,color,thick);
      prevPX=px_; prevPY=py_;
    }

    if (!textFmt.empty() && sf && sf->loaded) {
      std::string label=formatLabel(e,gui,ch,textFmt);
      sf->drawText(pb, x0+2, y0+2, label.c_str(), textColorU, x1);
    }
  }
}

void FurnaceGUI::clearOscVideoSoftFont() {
  if (oscVideoSoftFont!=NULL) { delete oscVideoSoftFont; oscVideoSoftFont=NULL; }
}

void FurnaceGUI::detectOscVideoFfmpeg() {
  std::string testExe=oscVideoFfmpegPath.empty() ? std::string("ffmpeg") : std::string(oscVideoFfmpegPath);
#ifdef _WIN32
  std::string testCmd=fmt::sprintf("\"%s\" -version >NUL 2>NUL",testExe);
#else
  std::string testCmd=fmt::sprintf("\"%s\" -version >/dev/null 2>/dev/null",testExe);
#endif
  oscVideoFfmpegFound=(system(testCmd.c_str())==0);
}

void FurnaceGUI::drawOscVideoProgress() {
  centerNextWindow(_("Osc Video Render"),canvasW,canvasH);
  if (ImGui::BeginPopupModal(_("Osc Video Render"),NULL,
      ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|
      ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_AlwaysAutoResize)) {
    WAKE_UP;

    int curOrd=oscVideoCurOrder.load();
    int maxOrd=oscVideoMaxOrder.load();

    if (!oscVideoExporting) {
      ImGui::Text(_("Done!"));
      if (ImGui::Button(_("OK"),ImVec2(200.0f*dpiScale,0)))
        ImGui::CloseCurrentPopup();
    } else {
      ImGui::Text(_("Rendering..."));
      if (maxOrd>0) {
        float frac=(float)curOrd/(float)maxOrd;
        char ovl[64];
        snprintf(ovl,64,"Order %d / %d",curOrd,maxOrd);
        ImGui::ProgressBar(frac,ImVec2(320.0f*dpiScale,0),ovl);
      } else {
        ImGui::ProgressBar(-1.0f*(float)ImGui::GetTime(),ImVec2(320.0f*dpiScale,0),"");
      }
    }

    ImGui::EndPopup();
  }
}

void FurnaceGUI::drawExportOscVideo(bool onWindow) {
  exitDisabledTimer=1;

  ImGui::SeparatorText(_("Video"));
  if (ImGui::InputInt(_("Width##ovw"),&oscVideoWidth,16,160)) {
    oscVideoWidth=oscVideoWidth&~1; // must be even for libx264
    if (oscVideoWidth<64) oscVideoWidth=64;
    if (oscVideoWidth>7680) oscVideoWidth=7680;
  }
  if (ImGui::InputInt(_("Height##ovh"),&oscVideoHeight,16,160)) {
    oscVideoHeight=oscVideoHeight&~1; // must be even for libx264
    if (oscVideoHeight<64) oscVideoHeight=64;
    if (oscVideoHeight>4320) oscVideoHeight=4320;
  }
  if (ImGui::InputInt(_("FPS##ovf"),&oscVideoFps,1,10)) {
    if (oscVideoFps<1) oscVideoFps=1;
    if (oscVideoFps>120) oscVideoFps=120;
  }
  ImGui::Text(_("Format:"));
  ImGui::SameLine();
  ImGui::RadioButton(_("MKV (Vorbis)##ovfmt"),&oscVideoOutputFormat,0);
  ImGui::SameLine();
  ImGui::RadioButton(_("MP4 (AAC)##ovfmt"),&oscVideoOutputFormat,1);
  if (oscVideoOutputFormat==1) {
    bool limitSize=(oscVideoTargetSizeMB>0);
    if (ImGui::Checkbox(_("Target file size##ovlimit"),&limitSize))
      oscVideoTargetSizeMB=limitSize?10:0;
    if (limitSize) {
      ImGui::SameLine();
      ImGui::SetNextItemWidth(100.0f*dpiScale);
      if (ImGui::InputInt(_("MB##ovtargetmb"),&oscVideoTargetSizeMB,1,10)) {
        if (oscVideoTargetSizeMB<1) oscVideoTargetSizeMB=1;
        if (oscVideoTargetSizeMB>100000) oscVideoTargetSizeMB=100000;
      }
    }
  }

  ImGui::SeparatorText(_("Audio"));
  {
    static const int srVals[]={22050,32000,44100,48000,88200,96000};
    static const char* srLabels[]={"22050 Hz","32000 Hz","44100 Hz","48000 Hz","88200 Hz","96000 Hz"};
    int srIdx=0;
    for (int i=0; i<6; i++) if (oscVideoSampleRate>=srVals[i]) srIdx=i;
    if (ImGui::Combo(_("Sample Rate##ovsr"),&srIdx,srLabels,6))
      oscVideoSampleRate=srVals[srIdx];
  }
  {
    static const int brVals[]={64,80,96,112,128,160,192,224,256,320,500};
    static const char* brLabels[]={"64 kbps","80 kbps","96 kbps","112 kbps","128 kbps","160 kbps","192 kbps","224 kbps","256 kbps","320 kbps","500 kbps"};
    int brIdx=6; // default 192
    for (int i=0; i<11; i++) if (oscVideoAudioBitrate>=brVals[i]) brIdx=i;
    if (ImGui::Combo(_("Bitrate##ovab"),&brIdx,brLabels,11))
      oscVideoAudioBitrate=brVals[brIdx];
  }

  ImGui::SeparatorText(_("Text"));
  if (ImGui::SliderFloat(_("Text Scale##ovts"),&oscVideoTextScale,0.5f,8.0f,"%.1fx")) {
    if (oscVideoTextScale<0.5f) oscVideoTextScale=0.5f;
    if (oscVideoTextScale>8.0f) oscVideoTextScale=8.0f;
  }

  ImGui::SeparatorText(_("Preview"));
  float aspect=(float)oscVideoWidth/(float)oscVideoHeight;
  float previewW=ImGui::GetContentRegionAvail().x;
  float previewH=previewW/aspect;
  if (previewH<60.0f) { previewH=60.0f; previewW=previewH*aspect; }
  if (previewH>280.0f*dpiScale) { previewH=280.0f*dpiScale; previewW=previewH*aspect; }

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
    std::vector<uint32_t> chanColors((size_t)totalChans);
    for (int i=0; i<totalChans; i++) chanColors[i]=imvec4ToU(channelColor(i));
    uint32_t solidColorU=imvec4ToU(chanOscColor);
    uint32_t textColorU=imvec4ToU(chanOscTextColor);
    float previewLineSize=chanOscLineSize*(float)previewPxH/(float)oscVideoHeight;

    if (oscVideoSoftFont==NULL && !mainFontRawData.empty()) {
      oscVideoSoftFont=new OscSoftFont();
      if (!oscVideoSoftFont->load(mainFontRawData.data(),mainFontRawData.size())) {
        delete oscVideoSoftFont; oscVideoSoftFont=NULL;
      }
    }
    if (oscVideoSoftFont && oscVideoSoftFont->loaded) {
      float px=oscVideoTextScale*(float)previewPxH/60.0f;
      if (px<4.0f) px=4.0f;
      oscVideoSoftFont->setSize(px);
    }

    PixBuf pb;
    pb.init(previewPxW,previewPxH);
    pb.clear(0xff000000u);
    std::vector<VideoOscState> previewState((size_t)totalChans);
    renderFrame(pb,e,this,
      chanOscWindowSize,chanOscAmplify,previewLineSize,
      chanOscColorMode,solidColorU,textColorU,
      chanOscTextFormat,chanOscCenterStrat,chanOscWaveCorr,
      chanColors,previewState,oscVideoSoftFont);

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
    char ffmpegBuf[1024];
    strncpy(ffmpegBuf,oscVideoFfmpegPath.c_str(),sizeof(ffmpegBuf)-1);
    ffmpegBuf[sizeof(ffmpegBuf)-1]='\0';
    float detectW=80.0f*dpiScale;
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x-detectW-ImGui::GetStyle().ItemSpacing.x);
    if (ImGui::InputText("##ovffmpegpath",ffmpegBuf,sizeof(ffmpegBuf))) {
      oscVideoFfmpegPath=ffmpegBuf;
      oscVideoFfmpegFound=false;
    }
    ImGui::SameLine();
    if (ImGui::Button(_("Detect"),ImVec2(detectW,0)))
      detectOscVideoFfmpeg();
    if (oscVideoFfmpegFound) {
      ImGui::TextColored(ImVec4(0.2f,0.9f,0.2f,1.0f),_("ffmpeg found"));
    } else {
      ImGui::TextColored(ImVec4(1.0f,0.5f,0.1f,1.0f),_("not detected (click Detect to check)"));
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
  int W=oscVideoWidth, H=oscVideoHeight, FPS=oscVideoFps;
  int audioBr=oscVideoAudioBitrate;

  int rate=(int)e->getAudioDescGot().rate;
  if (rate<=0) rate=44100;

  logI("OscVideo: %dx%d @%dfps rate=%d",W,H,FPS,rate);

  std::vector<unsigned char> fontDataCopy=mainFontRawData;
  OscSoftFont threadSf;
  if (!fontDataCopy.empty() && threadSf.load(fontDataCopy.data(),fontDataCopy.size())) {
    float px=oscVideoTextScale*(float)H/60.0f;
    if (px<4.0f) px=4.0f;
    threadSf.setSize(px);
  }

  std::string ffmpegExe=oscVideoFfmpegPath.empty() ? std::string("ffmpeg") : std::string(oscVideoFfmpegPath);
  std::string vidTmp=std::string(path)+".vidtmp.mkv";
  std::string audTmp=std::string(path)+".audtmp.wav";

  std::string vcmd=fmt::sprintf(
    "\"%s\" -y -f rawvideo -pix_fmt rgba -s %dx%d -r %d -i pipe:0 "
    "-c:v libx264 -preset ultrafast -pix_fmt yuv420p -f matroska \"%s\" %s",
    ffmpegExe,W,H,FPS,vidTmp,NULL_REDIRECT);
  FILE* vpipe=FURNACE_POPEN(vcmd.c_str(),PIPE_WRITE_MODE);
  if (!vpipe) {
    logE("OscVideo: popen failed - is ffmpeg installed and on PATH?");
    oscVideoExporting=false;
    return;
  }

  int totalChans=e->getTotalChannelCount();
  std::vector<uint32_t> chanColors((size_t)totalChans);
  for (int i=0; i<totalChans; i++) chanColors[i]=imvec4ToU(channelColor(i));
  uint32_t solidColorU=imvec4ToU(chanOscColor);
  uint32_t textColorU=imvec4ToU(chanOscTextColor);

  float windowSizeMs=chanOscWindowSize;
  float amplify=chanOscAmplify;
  float lineThick=chanOscLineSize;
  int colorMode=chanOscColorMode;
  std::string textFmt=chanOscTextFormat.c_str();
  int centerStrat=chanOscCenterStrat;
  bool waveCorr=chanOscWaveCorr;

  std::vector<VideoOscState> chanState((size_t)totalChans);

  PixBuf pb;
  pb.init(W,H);

  std::vector<float> audioSamples;

  oscVideoMaxOrder.store(e->curSubSong->ordersLen);
  int frameCount=0;
  e->renderOscVideo(FPS, 1,
    [&](int /*frameIdx*/, float** audio, int spf) {
      frameCount++;
      oscVideoCurOrder.store((int)e->getOrder());

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
    [&]()->bool { return false; }
  );

  FURNACE_PCLOSE(vpipe);

  logI("OscVideo: writing audio WAV float32 (%zu stereo samples)",(size_t)audioSamples.size()/2);

  if (!audioSamples.empty()) {
    FILE* af=fopen(audTmp.c_str(),"wb");
    if (af) {
      int dataBytes=(int)audioSamples.size()*4;
      auto w4=[&](uint32_t v){fwrite(&v,4,1,af);};
      auto w2=[&](uint16_t v){fwrite(&v,2,1,af);};
      fwrite("RIFF",1,4,af); w4((uint32_t)(36+dataBytes));
      fwrite("WAVE",1,4,af);
      fwrite("fmt ",1,4,af); w4(16);
      w2(3); w2(2); w4((uint32_t)rate); w4((uint32_t)rate*8); w2(8); w2(32);
      fwrite("data",1,4,af); w4((uint32_t)dataBytes);
      fwrite(audioSamples.data(),4,audioSamples.size(),af);
      fclose(af);
    } else {
      logE("OscVideo: cannot open audio tmp");
    }
  }

  logI("OscVideo: combining...");
  std::string ccmd;
  if (oscVideoOutputFormat==1) {
    if (oscVideoTargetSizeMB>0 && frameCount>0) {
      double dur=(double)frameCount/(double)FPS;
      double targetBits=(double)oscVideoTargetSizeMB*1024.0*1024.0*8.0;
      double audioBits=(double)audioBr*1000.0*dur;
      int vbr=(int)((targetBits-audioBits)/(dur*1000.0));
      if (vbr<50) vbr=50;
      logI("OscVideo: target %d MB - %.1fs, video bitrate %d kbps",oscVideoTargetSizeMB,dur,vbr);
      ccmd=fmt::sprintf(
        "\"%s\" -y -i \"%s\" -i \"%s\" "
        "-c:v libx264 -b:v %dk -maxrate %dk -bufsize %dk -c:a aac -ar %d -b:a %dk \"%s\" %s",
        ffmpegExe,vidTmp,audTmp,vbr,vbr*2,vbr*4,oscVideoSampleRate,audioBr,std::string(path),NULL_REDIRECT);
    } else {
      ccmd=fmt::sprintf(
        "\"%s\" -y -i \"%s\" -i \"%s\" "
        "-c:v copy -c:a aac -ar %d -b:a %dk \"%s\" %s",
        ffmpegExe,vidTmp,audTmp,oscVideoSampleRate,audioBr,std::string(path),NULL_REDIRECT);
    }
  } else {
    ccmd=fmt::sprintf(
      "\"%s\" -y -i \"%s\" -i \"%s\" "
      "-c:v copy -c:a libvorbis -ar %d -b:a %dk \"%s\" %s",
      ffmpegExe,vidTmp,audTmp,oscVideoSampleRate,audioBr,std::string(path),NULL_REDIRECT);
  }
  int ret=system(ccmd.c_str());
  if (ret!=0) logE("OscVideo: combine failed (code %d)",ret);
  else logI("OscVideo: done - %s",path.c_str());

  remove(vidTmp.c_str());
  remove(audTmp.c_str());

  oscVideoExporting=false;
}
