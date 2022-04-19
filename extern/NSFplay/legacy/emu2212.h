#ifndef _EMU2212_H_
#define _EMU2212_H_

#ifdef EMU2212_DLL_EXPORTS
  #define EMU2212_API __declspec(dllexport)
#elif defined(EMU2212_DLL_IMPORTS)
  #define EMU2212_API __declspec(dllimport)
#else
  #define EMU2212_API
#endif

#ifdef __cplusplus
extern "C" {
#endif
  
#include "emutypes.h"

#define SCC_STANDARD 0
#define SCC_ENHANCED 1

#define SCC_MASK_CH(x) (1<<(x))

typedef struct __SCC {

  e_uint32 clk, rate ,base_incr, quality ;

  e_int32 out, prev, next;
  e_uint32 type ;
  e_uint32 mode ;
  e_uint32 active;
  e_uint32 base_adr;
  e_uint32 mask ;
  
  e_uint32 realstep ;
  e_uint32 scctime ;
  e_uint32 sccstep ;

  e_uint32 incr[5] ;

  e_int8  wave[5][32] ;

  e_uint32 count[5] ;
  e_uint32 freq[5] ;
  e_uint32 phase[5] ;
  e_uint32 volume[5] ;
  e_uint32 offset[5] ;
  e_uint8 reg[0x100-0xC0];

  int ch_enable ;
  int ch_enable_next ;

  int cycle_4bit ;
  int cycle_8bit ;
  int refresh ;
  int rotate[5] ;

} SCC ;


EMU2212_API SCC *SCC_new(e_uint32 c, e_uint32 r) ;
EMU2212_API void SCC_reset(SCC *scc) ;
EMU2212_API void SCC_set_rate(SCC *scc, e_uint32 r);
EMU2212_API void SCC_set_quality(SCC *scc, e_uint32 q) ;
EMU2212_API void SCC_set_type(SCC *scc, e_uint32 type) ;
EMU2212_API void SCC_delete(SCC *scc) ;
EMU2212_API e_int16 SCC_calc(SCC *scc) ;
EMU2212_API void SCC_write(SCC *scc, e_uint32 adr, e_uint32 val) ;
EMU2212_API void SCC_writeReg(SCC *scc, e_uint32 adr, e_uint32 val) ;
EMU2212_API e_uint32 SCC_read(SCC *scc, e_uint32 adr) ;
EMU2212_API e_uint32 SCC_setMask(SCC *scc, e_uint32 adr) ;
EMU2212_API e_uint32 SCC_toggleMask(SCC *scc, e_uint32 adr) ;

#ifdef __cplusplus
}
#endif

#endif
