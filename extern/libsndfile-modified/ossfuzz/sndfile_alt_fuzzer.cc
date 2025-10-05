#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sndfile.h>
#include <inttypes.h>

#include "sndfile_fuzz_header.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{   // One byte is needed for deciding which function to target.
    if (size == 0)
       return 0 ;

    const uint8_t decider = *data ;
    data += 1 ;
    size -= 1 ;

    SF_INFO sndfile_info ;
    VIO_DATA vio_data ;
    SF_VIRTUAL_IO vio ;
    SNDFILE *sndfile = NULL ;
    int err = sf_init_file(data, size, &sndfile, &vio_data, &vio, &sndfile_info) ;
    if (err)
      goto EXIT_LABEL ;

    // Just the right number of channels. Create some buffer space for reading.
    switch (decider % 3)
    {   case 0  :
                {
                  short* read_buffer = NULL ;
                  read_buffer = (short*)malloc(sizeof(short) * size);
                  if (read_buffer == NULL)
                    abort() ;

                  while (sf_read_short(sndfile, read_buffer, size))
                  {
                    // Do nothing with the data.
                  }
                  free(read_buffer) ;
                }
                break ;
        case 1  :
                {
                  int* read_buffer = NULL ;
                  read_buffer = (int*)malloc(sizeof(int) * size) ;
                  if (read_buffer == NULL)
                    abort() ;

                  while (sf_read_int(sndfile, read_buffer, size))
                  {
                    // Do nothing with the data.
                  }
                  free(read_buffer) ;
                }
                break ;
        case 2  :
                {
                  double* read_buffer = NULL ;
                  read_buffer = (double*)malloc(sizeof(double) * size) ;
                  if (read_buffer == NULL)
                    abort() ;

                  while (sf_read_double(sndfile, read_buffer, size))
                  {
                    // Do nothing with the data.
                  }
                  free(read_buffer) ;
                }
                break ;
        default :
                break ;
                } ;

  EXIT_LABEL:
    if (sndfile != NULL)
      sf_close(sndfile);

    return 0 ;
}
