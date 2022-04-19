#ifndef _NES_FME7_H_
#define _NES_FME7_H_
#include "../device.h"
#include "legacy/emu2149.h"

namespace xgm
{

  class NES_FME7:public ISoundChip
  {
  public:
    enum
    {
      OPT_END
    };
  protected:
    //int option[OPT_END];
    INT32 sm[2][3]; // stereo mix
    INT16 buf[2];
    PSG *psg;
    int divider; // clock divider
    double clock, rate;
    TrackInfoBasic trkinfo[5];
  public:
      NES_FME7 ();
     ~NES_FME7 ();
    virtual void Reset ();
    virtual void Tick (UINT32 clocks);
    virtual UINT32 Render (INT32 b[2]);
    virtual bool Read (UINT32 adr, UINT32 & val, UINT32 id=0);
    virtual bool Write (UINT32 adr, UINT32 val, UINT32 id=0);
    virtual void SetClock (double);
    virtual void SetRate (double);
    virtual void SetOption (int, int);
    virtual void SetMask (int m){ if(psg) PSG_setMask(psg,m); }
    virtual void SetStereoMix (int trk, xgm::INT16 mixl, xgm::INT16 mixr);
    virtual ITrackInfo *GetTrackInfo(int trk);
  };

}                               // namespace

#endif
