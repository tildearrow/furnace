#ifndef SNDFILE_FUZZ_HEADER_H
#define SNDFILE_FUZZ_HEADER_H

typedef struct
{
  sf_count_t offset ;
  sf_count_t length ;
  const unsigned char *data ;
} VIO_DATA ;

static sf_count_t vfget_filelen (void *user_data)
{  VIO_DATA *vf = (VIO_DATA *)user_data ;
   return vf->length ;
}

static sf_count_t vfseek (sf_count_t offset, int whence, void *user_data)
{
  VIO_DATA *vf = (VIO_DATA *)user_data ;
  sf_count_t new_offset ;

  switch (whence)
  {   case SEEK_SET :
        new_offset = offset ;
        break ;

    case SEEK_CUR :
        new_offset = vf->offset + offset ;
        break ;

    case SEEK_END :
        new_offset = vf->length + offset ;
        break ;

    default :
        break ;
  }

  /* Ensure you can't seek outside the data */
  if (new_offset > vf->length)
  {  /* Trying to seek past the end of the data */
     printf("vf overseek: new_offset(%" PRId64 ") > vf->length(%" PRId64 ");"
            "  whence(%d), vf->offset(%" PRId64 "), offset(%" PRId64 ")\n",
            new_offset, vf->length, whence, vf->offset, offset) ;
     new_offset = vf->length ;
  }
  else if (new_offset < 0)
  {  /* Trying to seek before the start of the data */
     printf("vf underseek: new_offset(%" PRId64 ") < 0;  whence(%d), vf->offset"
            "(%" PRId64 "), vf->length(%" PRId64 "), offset(%" PRId64 ")\n",
            new_offset, whence, vf->offset, vf->length, offset) ;
     new_offset = 0 ;
  }
  vf->offset = new_offset ;

  return vf->offset ;
}

static sf_count_t vfread (void *ptr, sf_count_t count, void *user_data)
{  VIO_DATA *vf = (VIO_DATA *)user_data ;

   if (vf->offset + count > vf->length)
     count = vf->length - vf->offset ;

   memcpy(ptr, vf->data + vf->offset, count) ;
   vf->offset += count ;

   return count ;
}

static sf_count_t vfwrite (const void *ptr, sf_count_t count, void *user_data)
{
  (void)ptr ;
  (void)count ;
  (void)user_data ;

  // Cannot write to this virtual file.
  return 0;
}

static sf_count_t vftell (void *user_data)
{ VIO_DATA *vf = (VIO_DATA *)user_data ;

  return vf->offset ;
}

int sf_init_file(const uint8_t *data, 
                size_t size, 
                SNDFILE **sndfile, 
                VIO_DATA *vio_data, 
                SF_VIRTUAL_IO *vio, SF_INFO *sndfile_info)
{  float* read_buffer = NULL ;

   // Initialize the virtual IO structure.
   vio->get_filelen = vfget_filelen ;
   vio->seek = vfseek ;
   vio->read = vfread ;
   vio->write = vfwrite ;
   vio->tell = vftell ;

   // Initialize the VIO user data.
   vio_data->data = data ;
   vio_data->length = size ;
   vio_data->offset = 0 ;

   memset(sndfile_info, 0, sizeof(SF_INFO)) ;

   // Try and open the virtual file.
   *sndfile = sf_open_virtual(vio, SFM_READ, sndfile_info, vio_data) ;

   if (sndfile_info->channels == 0)
		 return -1 ;

   if (sndfile_info->channels > 1024 * 1024)
		 return -1 ;

   return 0;
}

#endif
