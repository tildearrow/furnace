#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sndfile.h>
#include <inttypes.h>

#include "sndfile_fuzz_header.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{  VIO_DATA vio_data ;
   SF_VIRTUAL_IO vio ;
   SF_INFO sndfile_info ;
   SNDFILE *sndfile = NULL ;
   float* read_buffer = NULL ;

   int err = sf_init_file(data, size, &sndfile, &vio_data, &vio, &sndfile_info) ;
   if (err)
     goto EXIT_LABEL ;

   // Just the right number of channels. Create some buffer space for reading.
   read_buffer = (float*)malloc(sizeof(float) * sndfile_info.channels);
   if (read_buffer == NULL)
     abort() ;

   while (sf_readf_float(sndfile, read_buffer, 1))
   {
     // Do nothing with the data.
   }

EXIT_LABEL:

   if (sndfile != NULL)
     sf_close(sndfile) ;

   free(read_buffer) ;

   return 0 ;
}
