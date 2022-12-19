#ifndef MZPOKEYSND_H_
#define MZPOKEYSND_H_

#include <stdlib.h>

int MZPOKEYSND_Init(size_t freq17,
                        int playback_freq,
                        int flags,
                        int quality
                        , int clear_regs
                       );

#endif /* MZPOKEYSND_H_ */
