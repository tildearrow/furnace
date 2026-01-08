/*
	os2: OS/2 RealTime DART Engine

	copyright 1998-2020 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Samuel Audet
*/

#include "../out123_int.h"

#undef VERSION /* The VERSION macro conflicts with the OS/2 multimedia headers */

#define INCL_OS2MM
#define INCL_DOS
#define INCL_VIO
#define INCL_KBD
#include <os2safe.h>
#include <os2.h>
#include <os2me.h>
#include <stdlib.h>
#include <ctype.h>

#include "../../common/debug.h"

// Only one instance at a time! This all needs to go into userptr!
static BOOL opened = FALSE;

/* complementary audio parameters */
static int numbuffers = 8;     /* total audio buffers, _bare_ minimum = 4 (cuz of prio boost check) */
#define audiobufsize 4096
static int lockdevice = FALSE;
static USHORT volume = 100;
static char *boostprio = NULL;
static char *normalprio = NULL;
static unsigned char boostclass = 3, normalclass = 2;
static signed char   boostdelta = 0, normaldelta = 31;
static unsigned char mmerror[160] = {0};
static int playingframe;

/* audio buffers */
static ULONG ulMCIBuffers;

static MCI_AMP_OPEN_PARMS  maop = {0};
static MCI_MIXSETUP_PARMS  mmp = {0};
static MCI_BUFFER_PARMS    mbp = {0};
static MCI_GENERIC_PARMS   mgp = {0};
static MCI_SET_PARMS       msp = {0};
static MCI_STATUS_PARMS    mstatp = {0};
static MCI_MIX_BUFFER      *MixBuffers = NULL;

struct prebuf
{
	unsigned char d[audiobufsize]; // data area
	int fill; // number of bytes in there
};
// Being lazy for draining.
static unsigned char zbuf[audiobufsize] = {0};

typedef struct
{
	MCI_MIX_BUFFER  *NextBuffer;
	int frameNum;
} BUFFERINFO;

// This static business is EVIL!
// All this needs to go into userptr to allow multiple instances.

static BUFFERINFO *bufferinfo = NULL;


static HEV dataplayed = 0;
static ULONG resetcount;
static BOOL paused = FALSE;

static MCI_MIX_BUFFER *tobefilled, *playingbuffer = NULL, playedbuffer;
static void *pBufferplayed;

static BOOL nomoredata,nobuffermode,justflushed;

static TIB *mainthread; /* thread info to set thread priority */

static ULONG keyboardtid;


static LONG APIENTRY DARTEvent(ULONG ulStatus, MCI_MIX_BUFFER *PlayedBuffer, ULONG ulFlags)
{
	switch(ulFlags)
	{
		case MIX_STREAM_ERROR | MIX_WRITE_COMPLETE:  /* error occur in device */
		
			if ( ulStatus == ERROR_DEVICE_UNDERRUN)
			/* Write buffers to rekick off the amp mixer. */
				mmp.pmixWrite( mmp.ulMixHandle, MixBuffers, ulMCIBuffers );
		break;
		
		case MIX_WRITE_COMPLETE:                     /* for playback  */
			
			playingbuffer = ((BUFFERINFO *) PlayedBuffer->ulUserParm)->NextBuffer;
			
			/* the next three lines are only useful to audio_playing_samples() */
			playedbuffer = *PlayedBuffer;
			playedbuffer.pBuffer = pBufferplayed;
			memcpy(playedbuffer.pBuffer, PlayedBuffer->pBuffer, PlayedBuffer->ulBufferLength);
			
			/* just too bad, the decoder fell behind... here we just keep the
			buffer to be filled in front of the playing one so that when the
			decoder kicks back in, we'll hear it in at the right time */
			if(tobefilled == playingbuffer)
			{
				tobefilled = ((BUFFERINFO *) playingbuffer->ulUserParm)->NextBuffer;
				nomoredata = TRUE;                                               
			}
			else
			{
				playingframe = ((BUFFERINFO *) playingbuffer->ulUserParm)->frameNum;
				
				/* if we're about to be short of decoder's data
				(2nd ahead buffer not filled), let's boost its priority! */
				if(tobefilled == ( (BUFFERINFO *) ((BUFFERINFO *) playingbuffer->ulUserParm)->NextBuffer->ulUserParm)->NextBuffer)
				DosSetPriority(PRTYS_THREAD,boostclass,boostdelta,mainthread->tib_ptib2->tib2_ultid);
			}
			
			/* empty the played buffer in case it doesn't get filled back */
			memset(PlayedBuffer->pBuffer,0,PlayedBuffer->ulBufferLength);
			
			DosPostEventSem(dataplayed);
			
			mmp.pmixWrite( mmp.ulMixHandle, PlayedBuffer, 1 );
		break;
	
	} /* end switch */
	
	return( TRUE );
	
} /* end DARTEvent */


static void MciError(ULONG ulError, int quiet)
{
	unsigned char buffer[128];
	ULONG rc;
	
	rc = mciGetErrorString(ulError, buffer, sizeof(buffer));
	
	if (rc == MCIERR_SUCCESS)
		sprintf(mmerror,"MCI Error %d: %s",ULONG_LOWD(ulError),buffer);
	else
		sprintf(mmerror,"MCI Error %d: Cannot query error message.",ULONG_LOWD(rc));
	if(!quiet)
		error1("%s",mmerror);
}


static int set_volume(out123_handle *ao, USHORT setvolume)
{
	if(setvolume > 100) setvolume = 100;
	volume = setvolume; /* useful when device is closed and reopened */

	if(maop.usDeviceID)
	{
		memset(&msp,0,sizeof(msp));
		msp.ulAudio = MCI_SET_AUDIO_ALL;
		msp.ulLevel = setvolume;
		
		mciSendCommand(maop.usDeviceID, MCI_SET,
			MCI_WAIT | MCI_SET_AUDIO | MCI_SET_VOLUME,
			&msp, 0);
	}
	return setvolume;
}


int open_os2(out123_handle *ao)
{
	if(opened)
		return -1;

	ULONG rc,i;
	char *temp;
	ULONG openflags;
	PPIB ppib;
	USHORT bits;
	const char *dev = ao->device;
	struct prebuf *pb = NULL;

	if(maop.usDeviceID) return (maop.usDeviceID);
	
	if(!ao) return -1;
	
	if(!dev) dev = "0";
	
	if(ao->rate < 0) ao->rate = 44100;
	if(ao->channels < 0) ao->channels = 2;
	if(ao->format < 0) ao->format = MPG123_ENC_SIGNED_16;
	
	if(ao->format == MPG123_ENC_SIGNED_16)
		bits = 16;
	else if(ao->format == MPG123_ENC_UNSIGNED_8)
		bits = 8;
	else return -1;

	ao->userptr = malloc(sizeof(struct prebuf));
	if(!ao->userptr)
		return -1;
	pb = (struct prebuf*)ao->userptr;
	pb->fill = 0;

	/* open the mixer device */
	memset (&maop, 0, sizeof(maop));
	maop.usDeviceID = 0;
	maop.pszDeviceType = (PSZ) MAKEULONG(MCI_DEVTYPE_AUDIO_AMPMIX, atoi(dev));
	
	openflags = MCI_WAIT | MCI_OPEN_TYPE_ID;
	if(!lockdevice) openflags |= MCI_OPEN_SHAREABLE;
	
	rc = mciSendCommand(0, MCI_OPEN, openflags, &maop, 0);
	
	if (ULONG_LOWD(rc) != MCIERR_SUCCESS)
	{
		MciError(rc, AOQUIET);
		maop.usDeviceID = 0;
		return(-1);
	}
	
	/* volume in ao->gain ?? */
	
	/* Set the MCI_MIXSETUP_PARMS data structure to match the audio stream. */
	
	memset(&mmp, 0, sizeof(mmp));
	
	mmp.ulBitsPerSample = bits;
	mmp.ulFormatTag = MCI_WAVE_FORMAT_PCM;
	mmp.ulSamplesPerSec = ao->rate;
	mmp.ulChannels = ao->channels;
	
	/* Setup the mixer for playback of wave data */
	mmp.ulFormatMode = MCI_PLAY;
	mmp.ulDeviceType = MCI_DEVTYPE_WAVEFORM_AUDIO;
	mmp.pmixEvent    = DARTEvent;
	
	rc = mciSendCommand( maop.usDeviceID,
		MCI_MIXSETUP,
		MCI_WAIT | MCI_MIXSETUP_INIT,
		&mmp,
		0 );
	
	if ( ULONG_LOWD(rc) != MCIERR_SUCCESS )
	{
		MciError(rc, AOQUIET);
		maop.usDeviceID = 0;
		return(-1);
	}
	
	volume = set_volume(ao,volume);
	
	/* Set up the BufferParms data structure and allocate
	* device buffers from the Amp-Mixer  */
	
	memset(&mbp, 0, sizeof(mbp));
	free(MixBuffers);
	free(bufferinfo);
	if(numbuffers < 5) numbuffers = 5;
	if(numbuffers > 200) numbuffers = 200;
	MixBuffers = calloc(numbuffers, sizeof(*MixBuffers));
	bufferinfo = calloc(numbuffers, sizeof(*bufferinfo));
	
	ulMCIBuffers = numbuffers;
	mbp.ulNumBuffers = ulMCIBuffers;
	/*   mbp.ulBufferSize = mmp.ulBufferSize; */
	/* I don't like this... they must be smaller than 64KB or else the
	engine needs major rewrite */
	mbp.ulBufferSize = audiobufsize;
	mbp.pBufList = MixBuffers;
	
	rc = mciSendCommand( maop.usDeviceID,
	MCI_BUFFER,
	MCI_WAIT | MCI_ALLOCATE_MEMORY,
	(PVOID) &mbp,
	0 );
	
	if ( ULONG_LOWD(rc) != MCIERR_SUCCESS )
	{
		MciError(rc, AOQUIET);
		maop.usDeviceID = 0;
		return(-1);
	}
	
	pBufferplayed = playedbuffer.pBuffer = calloc(1,audiobufsize);
	
	ulMCIBuffers = mbp.ulNumBuffers; /* never know! */
	
	/* Fill all device buffers with zeros and set linked list */
	
	for(i = 0; i < ulMCIBuffers; i++)
	{
		MixBuffers[i].ulFlags = 0;
		MixBuffers[i].ulBufferLength = mbp.ulBufferSize;
		memset(MixBuffers[i].pBuffer, 0, MixBuffers[i].ulBufferLength);
		
		MixBuffers[i].ulUserParm = (ULONG) &bufferinfo[i];
		bufferinfo[i].NextBuffer = &MixBuffers[i+1];
	}
	
	bufferinfo[i-1].NextBuffer = &MixBuffers[0];
	
	/* Create a semaphore to know when data has been played by the DART thread */
	DosCreateEventSem(NULL,&dataplayed,0,FALSE);
	
	playingbuffer = &MixBuffers[0];
	tobefilled = &MixBuffers[1];
	playingframe = 0;
	nomoredata = TRUE;
	nobuffermode = FALSE;
	justflushed = FALSE;
	
	if(boostprio)
	{
		temp = alloca(strlen(boostprio)+1);
		strcpy(temp,boostprio);
		
		boostdelta = atoi(temp+1);
		*(temp+1) = 0;
		boostclass = atoi(temp);
	}
	if(boostclass > 4) boostdelta = 3;
	if(boostdelta > 31) boostdelta = 31;
	if(boostdelta < -31) boostdelta = -31;
	
	
	if(normalprio)
	{
		temp = alloca(strlen(normalprio)+1);
		strcpy(temp,normalprio);
		
		normaldelta = atoi(temp+1);
		*(temp+1) = 0;
		normalclass = atoi(temp);
	}
	if(normalclass > 4) normaldelta = 3;
	if(normaldelta > 31) normaldelta = 31;
	if(normaldelta < -31) normaldelta = -31;
	
	
	DosGetInfoBlocks(&mainthread,&ppib); /* ppib not needed, but makes some DOSCALLS.DLL crash */
	DosSetPriority(PRTYS_THREAD,boostclass,boostdelta,mainthread->tib_ptib2->tib2_ultid);
	
	/* Write buffers to kick off the amp mixer. see DARTEvent() */
	rc = mmp.pmixWrite( mmp.ulMixHandle,
		MixBuffers,
		ulMCIBuffers );

	opened = TRUE;
	return maop.usDeviceID;
}

// Always write full blocks of audiobufsize. The engine does not like
// things otherwise. Using a local prebuffer for the leftovers.

static int write_os2(out123_handle *ao,unsigned char *buf,int len)
{
	int written = 0;
	struct prebuf *pb = ao->userptr;
	mdebug("write_os2(%p, %p, %d)", ao, buf, len);
	while(len > 0)
	{
		if(len + pb->fill < audiobufsize)
		{
			mdebug("storing into pb %p @ %p, %d on top of %d", pb->d, pb->d+pb->fill, len, pb->fill);
			// just collect until we got a full buffer
			memcpy(pb->d + pb->fill, buf, len);
			pb->fill += len;
			written += len;
			break;
		}
		// Now, we at least got one full buffer to serve.

		/* if we're too quick, let's wait */
		if(nobuffermode)
		{
			MCI_MIX_BUFFER *temp = playingbuffer;

			while(
				(tobefilled != (temp = ((BUFFERINFO *) temp->ulUserParm)->NextBuffer)) &&
				(tobefilled != (temp = ((BUFFERINFO *) temp->ulUserParm)->NextBuffer)) &&
				(tobefilled != (temp = ((BUFFERINFO *) temp->ulUserParm)->NextBuffer)) )
			{
				DosResetEventSem(dataplayed,&resetcount);
				DosWaitEventSem(dataplayed, -1);
				temp = playingbuffer;
			}

		} else {
			while(tobefilled == playingbuffer)
			{
				DosResetEventSem(dataplayed,&resetcount);
				DosWaitEventSem(dataplayed, -1);
			}
		}

		if (justflushed) {
			justflushed = FALSE;
		} else {
			nomoredata = FALSE;
			int got = 0;
			// First the rest from the prebuffer, then the remaining part from the new stuff.
			if(pb->fill)
			{
				mdebug("copy all %d bytes of prebuffer from %p to %p", pb->fill, pb->d, tobefilled->pBuffer);
				memcpy(tobefilled->pBuffer, pb->d, pb->fill);
				got = pb->fill;
				pb->fill = 0;
			}
			int therest = audiobufsize - got;
			// len + got >= audiobufsize!!
			mdebug("copy therest %d from %p to %p", therest, buf, ((unsigned char*)tobefilled->pBuffer)+got);
			memcpy(((unsigned char*)tobefilled->pBuffer)+got, buf, therest);
			debug("done with buffer writes");
			buf += therest;
			len -= therest;
			written += therest;
			tobefilled->ulBufferLength = got+therest; // always == audiobufsize!
			//      ((BUFFERINFO *) tobefilled->ulUserParm)->frameNum = fr->frameNum;

			/* if we're out of the water (3rd ahead buffer filled),
			let's reduce our priority */
			if(tobefilled == ( (BUFFERINFO *) ( (BUFFERINFO *) ((BUFFERINFO *) playingbuffer->ulUserParm)->NextBuffer->ulUserParm)->NextBuffer->ulUserParm)->NextBuffer)
				DosSetPriority(PRTYS_THREAD,normalclass,normaldelta,mainthread->tib_ptib2->tib2_ultid);

			tobefilled = ((BUFFERINFO *) tobefilled->ulUserParm)->NextBuffer;
		}
	}
	return written;
}

#if 0
static int write_os2(out123_handle *ao,unsigned char *buf,int len)
{
	if(len > audiobufsize || !playingbuffer) return -1;
	
	if(mmp.ulBitsPerSample == 16)
		ao->format = MPG123_ENC_SIGNED_16;
	else if(mmp.ulBitsPerSample == 8)
		ao->format = MPG123_ENC_UNSIGNED_8;
	else return -1;
	
	ao->rate = mmp.ulSamplesPerSec;
	ao->channels = mmp.ulChannels;
	
	if(buf && len)
	{
		ULONG rc;
		int upto;
		
		mstatp.ulItem = MCI_STATUS_POSITION;
		
		rc = mciSendCommand( maop.usDeviceID,
				MCI_STATUS,
				MCI_STATUS_ITEM | MCI_WAIT,
				&mstatp,
				0 );
		
		if ( ULONG_LOWD(rc) != MCIERR_SUCCESS )
		{
			MciError(rc, AOQUIET);
			maop.usDeviceID = 0;
			return(-1);
		}
		
		/* this is hypocrite...
		DART returns the value in ulReturn instead of ulValue,
		also it returns in milliseconds and not MMTIME... arg */
		
		upto = (mstatp.ulReturn-playedbuffer.ulTime) * mmp.ulSamplesPerSec / 1000;
		upto *= mmp.ulChannels * (mmp.ulBitsPerSample>>3);
		
		/* if a timing problem occurs, let's at least not crash */
		if(upto > playingbuffer->ulBufferLength)
			upto = playingbuffer->ulBufferLength;
		
		if(len < upto) {
			memcpy(buf,(char *) (playingbuffer->pBuffer)+upto-len, len);
		} else {
			memcpy(buf,(char *) playedbuffer.pBuffer+playedbuffer.ulBufferLength-(len-upto),len-upto);
			memcpy(buf+(len-upto),playingbuffer->pBuffer,upto);
		}
	}
	
	return 0;
}
#endif

/*
static int audio_nobuffermode(out123_handle *ao, int setnobuffermode)
{
   nobuffermode = setnobuffermode;
   return TRUE;
}

int audio_trash_buffers(out123_handle *ao)
{
   int i;

   justflushed = TRUE;

   // Fill all device buffers with zeros 
   for(i = 0; i < ulMCIBuffers; i++)
      memset(MixBuffers[i].pBuffer, 0, MixBuffers[i].ulBufferLength);

   tobefilled = ((BUFFERINFO *) playingbuffer->ulUserParm)->NextBuffer;
   nomoredata = TRUE;

   return TRUE;
}
*/

static int close_os2(out123_handle *ao)
{
	opened = FALSE;
	if(ao && ao->userptr)
	{
		free(ao->userptr);
		ao->userptr = NULL;
	}
	ULONG rc;
	
	if(!maop.usDeviceID)
	return 0;
	
	while(!nomoredata)
	{
		DosResetEventSem(dataplayed,&resetcount);
		DosWaitEventSem(dataplayed, -1);
	}
	
	playingbuffer = NULL;
	
	DosCloseEventSem(dataplayed);
	dataplayed = 0;
	
	free(pBufferplayed);
	
	rc = mciSendCommand( maop.usDeviceID,
			MCI_BUFFER,
			MCI_WAIT | MCI_DEALLOCATE_MEMORY,
			&mbp,
			0 );
	
	if ( ULONG_LOWD(rc) != MCIERR_SUCCESS )
	{
		MciError(rc, AOQUIET);
		return(-1);
	}
	
	free(bufferinfo);
	free(MixBuffers);
	bufferinfo = NULL;
	MixBuffers = NULL;
	
	memset(&mbp, 0, sizeof(mbp));
	
	rc = mciSendCommand( maop.usDeviceID,
			MCI_CLOSE,
			MCI_WAIT ,
			&mgp,
			0 );
	
	if ( ULONG_LOWD(rc) != MCIERR_SUCCESS )
	{
		MciError(rc, AOQUIET);
		return(-1);
	}
	
	memset(&maop, 0, sizeof(maop));
	
	return 0;
}


/*
 * get formats for specific channel/rate parameters
 */
int get_formats_os2(out123_handle *ao)
{
	int fmts = 0;
	ULONG rc;
	MCI_MIXSETUP_PARMS mmptemp = {0};
	
	mmp.ulDeviceType = MCI_DEVTYPE_WAVEFORM_AUDIO;
	mmp.pmixEvent    = DARTEvent;
	
	mmptemp.ulFormatMode = MCI_PLAY;
	mmptemp.ulSamplesPerSec = ao->rate;
	mmptemp.ulChannels = ao->channels;
	
	mmptemp.ulFormatTag = MCI_WAVE_FORMAT_PCM;
	mmptemp.ulBitsPerSample = 16;
	rc = mciSendCommand( maop.usDeviceID,
			MCI_MIXSETUP,
			MCI_WAIT | MCI_MIXSETUP_QUERYMODE,
			&mmptemp,
			0 );

	if((ULONG_LOWD(rc) == MCIERR_SUCCESS) && (rc != 0x4000)) /* undocumented */
	fmts = fmts | MPG123_ENC_SIGNED_16;
	
	mmptemp.ulFormatTag = MCI_WAVE_FORMAT_PCM;
	mmptemp.ulBitsPerSample = 8;
	rc = mciSendCommand( maop.usDeviceID,
			MCI_MIXSETUP,
			MCI_WAIT | MCI_MIXSETUP_QUERYMODE,
			&mmptemp,
			0 );

	if((ULONG_LOWD(rc) == MCIERR_SUCCESS) && (rc != 0x4000)) /* undocumented */
		fmts = fmts | MPG123_ENC_UNSIGNED_8;
	
	mmptemp.ulFormatTag = MCI_WAVE_FORMAT_ALAW;
	mmptemp.ulBitsPerSample = 8;
	rc = mciSendCommand( maop.usDeviceID,
			MCI_MIXSETUP,
			MCI_WAIT | MCI_MIXSETUP_QUERYMODE,
			&mmptemp,
			0 );

	if((ULONG_LOWD(rc) == MCIERR_SUCCESS) && (rc != 0x4000)) /* undocumented */
		fmts = fmts | MPG123_ENC_ALAW_8;
	
	mmptemp.ulFormatTag = MCI_WAVE_FORMAT_MULAW;
	mmptemp.ulBitsPerSample = 8;
	rc = mciSendCommand( maop.usDeviceID,
			MCI_MIXSETUP,
			MCI_WAIT | MCI_MIXSETUP_QUERYMODE,
			&mmptemp,
			0 );

	if((ULONG_LOWD(rc) == MCIERR_SUCCESS) && (rc != 0x4000)) /* undocumented */
		fmts = fmts | MPG123_ENC_ULAW_8;
	
	return fmts;
}

static int get_devices_os2(char *info, int deviceid)
{
	char buffer[128];
	MCI_SYSINFO_PARMS mip;
	
	if(deviceid && info)
	{
		MCI_SYSINFO_LOGDEVICE mid;
		
		mip.pszReturn = buffer;
		mip.ulRetSize = sizeof(buffer);
		mip.usDeviceType = MCI_DEVTYPE_AUDIO_AMPMIX;
		mip.ulNumber = deviceid;
		
		mciSendCommand(0,
			MCI_SYSINFO,
			MCI_WAIT | MCI_SYSINFO_INSTALLNAME,
			&mip,
			0);
		
		mip.ulItem = MCI_SYSINFO_QUERY_DRIVER;
		mip.pSysInfoParm = &mid;
		strcpy(mid.szInstallName,buffer);
		
		mciSendCommand(0,
			MCI_SYSINFO,
			MCI_WAIT | MCI_SYSINFO_ITEM,
			&mip,
			0);
		
		strcpy(info,mid.szProductInfo);
		return deviceid;

	} else {
		int number;
		
		mip.pszReturn = buffer;
		mip.ulRetSize = sizeof(buffer);
		mip.usDeviceType = MCI_DEVTYPE_AUDIO_AMPMIX;
		
		mciSendCommand(0,
			MCI_SYSINFO,
			MCI_WAIT | MCI_SYSINFO_QUANTITY,
			&mip,
			0);
		
		number = atoi(mip.pszReturn);
		return number;
	}
}

// at least drain out our prebuffer, proper draining
// should be (re?)enabled, too
static void drain_os2(out123_handle *ao)
{
	if(!ao)
		return;
	ao->write(ao, zbuf, audiobufsize);
	if(ao->userptr)
		((struct prebuf*)ao->userptr)->fill = 0;
	while(!nomoredata)
	{
		DosResetEventSem(dataplayed,&resetcount);
		DosWaitEventSem(dataplayed, -1);
	}
}

static void flush_os2(out123_handle *ao)
{
	if(ao && ao->userptr)
		((struct prebuf*)ao->userptr)->fill = 0;
}


static int init_os2(out123_handle* ao)
{
	if (ao==NULL) return -1;
	
	/* Set callbacks */
	ao->open = open_os2;
	ao->flush = flush_os2;
	ao->write = write_os2;
	ao->get_formats = get_formats_os2;
	ao->close = close_os2;
	ao->drain = drain_os2;

	/* Success */
	return 0;
}





/* 
	Module information data structure
*/
mpg123_module_t mpg123_output_module_info = {
	/* api_version */	MPG123_MODULE_API_VERSION,
	/* name */			"os2",						
	/* description */	"Audio output for OS2.",
	/* revision */		"$Rev:$",						
	/* handle */		NULL,
	
	/* init_output */	init_os2,						
};


