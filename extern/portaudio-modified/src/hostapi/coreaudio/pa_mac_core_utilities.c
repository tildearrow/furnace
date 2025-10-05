/*
 * Helper and utility functions for pa_mac_core.c (Apple AUHAL implementation)
 *
 * PortAudio Portable Real-Time Audio Library
 * Latest Version at: http://www.portaudio.com
 *
 * Written by Bjorn Roche of XO Audio LLC, from PA skeleton code.
 * Portions copied from code by Dominic Mazzoni (who wrote a HAL implementation)
 *
 * Dominic's code was based on code by Phil Burk, Darren Gibbs,
 * Gord Peters, Stephane Letz, and Greg Pfiel.
 *
 * The following people also deserve acknowledgements:
 *
 * Olivier Tristan for feedback and testing
 * Glenn Zelniker and Z-Systems engineering for sponsoring the Blocking I/O
 * interface.
 *
 *
 * Based on the Open Source API proposed by Ross Bencina
 * Copyright (c) 1999-2002 Ross Bencina, Phil Burk
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * The text above constitutes the entire PortAudio license; however,
 * the PortAudio community also makes the following non-binding requests:
 *
 * Any person wishing to distribute modifications to the Software is
 * requested to send the modifications to the original developer so that
 * they can be incorporated into the canonical version. It is also
 * requested that these non-binding requests be included along with the
 * license above.
 */

/**
 @file
 @ingroup hostapi_src
*/

#include "pa_mac_core_utilities.h"
#include "pa_mac_core_internal.h"
#include <libkern/OSAtomic.h>
#include <strings.h>
#include <pthread.h>
#include <sys/time.h>

OSStatus PaMacCore_AudioHardwareGetProperty(
        AudioHardwarePropertyID inPropertyID,
        UInt32*                 ioPropertyDataSize,
        void*                   outPropertyData )
{
    AudioObjectPropertyAddress address = { inPropertyID, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
    return AudioObjectGetPropertyData(kAudioObjectSystemObject, &address, 0, NULL, ioPropertyDataSize, outPropertyData);
}

OSStatus PaMacCore_AudioHardwareGetPropertySize(
        AudioHardwarePropertyID inPropertyID,
        UInt32*                 outSize )
{
    AudioObjectPropertyAddress address = { inPropertyID, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
    return AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &address, 0, NULL, outSize);
}

OSStatus PaMacCore_AudioDeviceGetProperty(
        AudioDeviceID         inDevice,
        UInt32                inChannel,
        Boolean               isInput,
        AudioDevicePropertyID inPropertyID,
        UInt32*               ioPropertyDataSize,
        void*                 outPropertyData )
{
    AudioObjectPropertyScope scope = isInput ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput;
    AudioObjectPropertyAddress address = { inPropertyID, scope, inChannel };
    return AudioObjectGetPropertyData(inDevice, &address, 0, NULL, ioPropertyDataSize, outPropertyData);
}

OSStatus PaMacCore_AudioDeviceSetProperty(
        AudioDeviceID         inDevice,
        const AudioTimeStamp* inWhen,
        UInt32                inChannel,
        Boolean               isInput,
        AudioDevicePropertyID inPropertyID,
        UInt32                inPropertyDataSize,
        const void*           inPropertyData )
{
    AudioObjectPropertyScope scope = isInput ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput;
    AudioObjectPropertyAddress address = { inPropertyID, scope, inChannel };
    return AudioObjectSetPropertyData(inDevice, &address, 0, NULL, inPropertyDataSize, inPropertyData);
}

OSStatus PaMacCore_AudioDeviceGetPropertySize(
        AudioDeviceID         inDevice,
        UInt32                inChannel,
        Boolean               isInput,
        AudioDevicePropertyID inPropertyID,
        UInt32*               outSize )
{
    AudioObjectPropertyScope scope = isInput ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput;
    AudioObjectPropertyAddress address = { inPropertyID, scope, inChannel };
    return AudioObjectGetPropertyDataSize(inDevice, &address, 0, NULL, outSize);
}

OSStatus PaMacCore_AudioDeviceAddPropertyListener(
        AudioDeviceID                   inDevice,
        UInt32                          inChannel,
        Boolean                         isInput,
        AudioDevicePropertyID           inPropertyID,
        AudioObjectPropertyListenerProc inProc,
        void*                           inClientData )
{
    AudioObjectPropertyScope scope = isInput ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput;
    AudioObjectPropertyAddress address = { inPropertyID, scope, inChannel };
    return AudioObjectAddPropertyListener(inDevice, &address, inProc, inClientData);
}

OSStatus PaMacCore_AudioDeviceRemovePropertyListener(
        AudioDeviceID                   inDevice,
        UInt32                          inChannel,
        Boolean                         isInput,
        AudioDevicePropertyID           inPropertyID,
        AudioObjectPropertyListenerProc inProc,
        void*                           inClientData )
{
    AudioObjectPropertyScope scope = isInput ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput;
    AudioObjectPropertyAddress address = { inPropertyID, scope, inChannel };
    return AudioObjectRemovePropertyListener(inDevice, &address, inProc, inClientData);
}

OSStatus PaMacCore_AudioStreamGetProperty(
        AudioStreamID         inStream,
        UInt32                inChannel,
        AudioDevicePropertyID inPropertyID,
        UInt32*               ioPropertyDataSize,
        void*                 outPropertyData )
{
    AudioObjectPropertyAddress address = { inPropertyID, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
    return AudioObjectGetPropertyData(inStream, &address, 0, NULL, ioPropertyDataSize, outPropertyData);
}

PaError PaMacCore_SetUnixError( int err, int line )
{
    PaError ret;
    const char *errorText;

    if( err == 0 )
    {
        return paNoError;
    }

    ret = paNoError;
    errorText = strerror( err );

    /** Map Unix error to PaError. Pretty much the only one that maps
        is ENOMEM. */
    if( err == ENOMEM )
        ret = paInsufficientMemory;
    else
        ret = paInternalError;

    DBUG(("%d on line %d: msg='%s'\n", err, line, errorText));
    PaUtil_SetLastHostErrorInfo( paCoreAudio, err, errorText );

    return ret;
}

/*
 * Translates MacOS generated errors into PaErrors
 */
PaError PaMacCore_SetError(OSStatus error, int line, int isError)
{
    /*FIXME: still need to handle possible ComponentResult values.*/
    /*       unfortunately, they don't seem to be documented anywhere.*/
    PaError result;
    const char *errorType;
    const char *errorText;

    switch (error) {
    case kAudioHardwareNoError:
        return paNoError;
    case kAudioHardwareNotRunningError:
        errorText = "Audio Hardware Not Running";
        result = paUnanticipatedHostError;
        break;
    case kAudioHardwareUnspecifiedError:
        errorText = "Unspecified Audio Hardware Error";
        result = paUnanticipatedHostError;
        break;
    case kAudioHardwareUnknownPropertyError:
        errorText = "Audio Hardware: Unknown Property";
        result = paUnanticipatedHostError;
        break;
    case kAudioHardwareBadPropertySizeError:
        errorText = "Audio Hardware: Bad Property Size";
        result = paUnanticipatedHostError;
        break;
    case kAudioHardwareIllegalOperationError:
        errorText = "Audio Hardware: Illegal Operation";
        result = paUnanticipatedHostError;
        break;
    case kAudioHardwareBadDeviceError:
        errorText = "Audio Hardware: Bad Device";
        result = paInvalidDevice;
        break;
    case kAudioHardwareBadStreamError:
        errorText = "Audio Hardware: BadStream";
        result = paUnanticipatedHostError;
        break;
    case kAudioHardwareUnsupportedOperationError:
        errorText = "Audio Hardware: Unsupported Operation";
        result = paUnanticipatedHostError;
        break;
    case kAudioDeviceUnsupportedFormatError:
        errorText = "Audio Device: Unsupported Format";
        result = paSampleFormatNotSupported;
        break;
    case kAudioDevicePermissionsError:
        errorText = "Audio Device: Permissions Error";
        result = paDeviceUnavailable;
        break;
    /* Audio Unit Errors: http://developer.apple.com/documentation/MusicAudio/Reference/CoreAudio/audio_units/chapter_5_section_3.html */
    case kAudioUnitErr_InvalidProperty:
        errorText = "Audio Unit: Invalid Property";
        result = paUnanticipatedHostError;
        break;
    case kAudioUnitErr_InvalidParameter:
        errorText = "Audio Unit: Invalid Parameter";
        result = paUnanticipatedHostError;
        break;
    case kAudioUnitErr_NoConnection:
        errorText = "Audio Unit: No Connection";
        result = paUnanticipatedHostError;
        break;
    case kAudioUnitErr_FailedInitialization:
        errorText = "Audio Unit: Initialization Failed";
        result = paUnanticipatedHostError;
        break;
    case kAudioUnitErr_TooManyFramesToProcess:
        errorText = "Audio Unit: Too Many Frames";
        result = paUnanticipatedHostError;
        break;
    case kAudioUnitErr_InvalidFile:
        errorText = "Audio Unit: Invalid File";
        result = paUnanticipatedHostError;
        break;
    case kAudioUnitErr_UnknownFileType:
        errorText = "Audio Unit: Unknown File Type";
        result = paUnanticipatedHostError;
        break;
    case kAudioUnitErr_FileNotSpecified:
        errorText = "Audio Unit: File Not Specified";
        result = paUnanticipatedHostError;
        break;
    case kAudioUnitErr_FormatNotSupported:
        errorText = "Audio Unit: Format Not Supported";
        result = paUnanticipatedHostError;
        break;
    case kAudioUnitErr_Uninitialized:
        errorText = "Audio Unit: Uninitialized";
        result = paUnanticipatedHostError;
        break;
    case kAudioUnitErr_InvalidScope:
        errorText = "Audio Unit: Invalid Scope";
        result = paUnanticipatedHostError;
        break;
    case kAudioUnitErr_PropertyNotWritable:
        errorText = "Audio Unit: PropertyNotWritable";
        result = paUnanticipatedHostError;
        break;
    case kAudioUnitErr_InvalidPropertyValue:
        errorText = "Audio Unit: Invalid Property Value";
        result = paUnanticipatedHostError;
        break;
    case kAudioUnitErr_PropertyNotInUse:
        errorText = "Audio Unit: Property Not In Use";
        result = paUnanticipatedHostError;
        break;
    case kAudioUnitErr_Initialized:
        errorText = "Audio Unit: Initialized";
        result = paUnanticipatedHostError;
        break;
    case kAudioUnitErr_InvalidOfflineRender:
        errorText = "Audio Unit: Invalid Offline Render";
        result = paUnanticipatedHostError;
        break;
    case kAudioUnitErr_Unauthorized:
        errorText = "Audio Unit: Unauthorized";
        result = paUnanticipatedHostError;
        break;
    case kAudioUnitErr_CannotDoInCurrentContext:
        errorText = "Audio Unit: cannot do in current context";
        result = paUnanticipatedHostError;
        break;
    default:
        errorText = "Unknown Error";
        result = paUnanticipatedHostError;
    }

    if (isError)
        errorType = "Error";
    else
        errorType = "Warning";

    char str[20];
    // see if it appears to be a 4-char-code
    *(UInt32 *)(str + 1) = CFSwapInt32HostToBig(error);
    if (isprint(str[1]) && isprint(str[2]) && isprint(str[3]) && isprint(str[4]))
    {
        str[0] = str[5] = '\'';
        str[6] = '\0';
    } else {
        // no, format it as an integer
        sprintf(str, "%d", (int)error);
    }

    DBUG(("%s on line %d: err='%s', msg=%s\n", errorType, line, str, errorText));

    if (result == paUnanticipatedHostError) {
        PaUtil_SetLastHostErrorInfo( paCoreAudio, error, errorText );
    }

    return result;
}

/*
 * This function computes an appropriate ring buffer size given
 * a requested latency (in seconds), sample rate and framesPerBuffer.
 *
 * The returned ringBufferSize is computed using the following
 * constraints:
 *   - it must be at least 4.
 *   - it must be at least 3x framesPerBuffer.
 *   - it must be at least 2x the suggestedLatency.
 *   - it must be a power of 2.
 * This function attempts to compute the minimum such size.
 *
 * FEEDBACK: too liberal/conservative/another way?
 */
long computeRingBufferSize( const PaStreamParameters *inputParameters,
                            const PaStreamParameters *outputParameters,
                            long inputFramesPerBuffer,
                            long outputFramesPerBuffer,
                            double sampleRate )
{
    long ringSize;
    int index;
    int i;
    double latency ;
    long framesPerBuffer ;

    VVDBUG(( "computeRingBufferSize()\n" ));

    assert( inputParameters || outputParameters );

    if( outputParameters && inputParameters )
    {
        latency = MAX( inputParameters->suggestedLatency, outputParameters->suggestedLatency );
        framesPerBuffer = MAX( inputFramesPerBuffer, outputFramesPerBuffer );
    }
    else if( outputParameters )
    {
        latency = outputParameters->suggestedLatency;
        framesPerBuffer = outputFramesPerBuffer ;
    }
    else /* we have inputParameters  */
    {
        latency = inputParameters->suggestedLatency;
        framesPerBuffer = inputFramesPerBuffer ;
    }

    ringSize = (long) ( latency * sampleRate * 2 + .5);
    VDBUG( ( "suggested latency : %d\n", (int) (latency*sampleRate) ) );
    if( ringSize < framesPerBuffer * 3 )
        ringSize = framesPerBuffer * 3 ;
    VDBUG(("framesPerBuffer:%d\n",(int)framesPerBuffer));
    VDBUG(("Ringbuffer size (1): %d\n", (int)ringSize ));

    /* make sure it's at least 4 */
    ringSize = MAX( ringSize, 4 );

    /* round up to the next power of 2 */
    index = -1;
    for( i=0; i<sizeof(long)*8; ++i )
        if( ringSize >> i & 0x01 )
            index = i;
    assert( index > 0 );
    if( ringSize <= ( 0x01 << index ) )
        ringSize = 0x01 << index ;
    else
        ringSize = 0x01 << ( index + 1 );

    VDBUG(( "Final Ringbuffer size (2): %d\n", (int)ringSize ));
    return ringSize;
}


/*
 * During testing of core audio, I found that serious crashes could occur
 * if properties such as sample rate were changed multiple times in rapid
 * succession. The function below could be used to with a condition variable.
 * to prevent propertychanges from happening until the last property
 * change is acknowledged. Instead, I implemented a busy-wait, which is simpler
 * to implement b/c in second round of testing (nov '09) property changes occurred
 * quickly and so there was no real way to test the condition variable implementation.
 * therefore, this function is not used, but it is aluded to in commented code below,
 * since it represents a theoretically better implementation.
 */

OSStatus propertyProc(
        AudioObjectID inObjectID,
        UInt32 inNumberAddresses,
        const AudioObjectPropertyAddress* inAddresses,
        void* inClientData )
{
    // this is where we would set the condition variable
    return noErr;
}

/* sets the value of the given property and waits for the change to
   be acknowledged, and returns the final value, which is not guaranteed
   by this function to be the same as the desired value. Obviously, this
   function can only be used for data whose input and output are the
   same size and format, and their size and format are known in advance.
   whether or not the call succeeds, if the data is successfully read,
   it is returned in outPropertyData. If it is not read successfully,
   outPropertyData is zeroed, which may or may not be useful in
   determining if the property was read. */
PaError AudioDeviceSetPropertyNowAndWaitForChange(
        AudioDeviceID inDevice,
        UInt32 inChannel,
        Boolean isInput,
        AudioDevicePropertyID inPropertyID,
        UInt32 inPropertyDataSize,
        const void *inPropertyData,
        void *outPropertyData )
{
    OSStatus macErr;
    UInt32 outPropertyDataSize = inPropertyDataSize;

    /* First, see if it already has that value. If so, return. */
    macErr = PaMacCore_AudioDeviceGetProperty( inDevice, inChannel,
                                               isInput, inPropertyID,
                                               &outPropertyDataSize, outPropertyData );
    if( macErr ) {
        memset( outPropertyData, 0, inPropertyDataSize );
        goto failMac;
    }
    if( inPropertyDataSize!=outPropertyDataSize )
        return paInternalError;
    if( 0==memcmp( outPropertyData, inPropertyData, outPropertyDataSize ) )
        return paNoError;

    /* Ideally, we'd use a condition variable to determine changes.
       we could set that up here. */

    /* If we were using a cond variable, we'd do something useful here,
       but for now, this is just to make 10.6 happy. */
    macErr = PaMacCore_AudioDeviceAddPropertyListener( inDevice, inChannel, isInput,
                                                       inPropertyID, propertyProc,
             NULL );
    if( macErr )
        /* we couldn't add a listener. */
        goto failMac;

    /* set property */
    macErr = PaMacCore_AudioDeviceSetProperty( inDevice, NULL, inChannel,
                                               isInput, inPropertyID,
                                               inPropertyDataSize, inPropertyData );
    if( macErr )
        goto failMac;

    /* busy-wait up to 30 seconds for the property to change */
    /* busy-wait is justified here only because the correct alternative (condition variable)
       was hard to test, since most of the waiting ended up being for setting rather than
       getting in OS X 10.5. This was not the case in earlier OS versions. */
    struct timeval tv1, tv2;
    gettimeofday( &tv1, NULL );
    memcpy( &tv2, &tv1, sizeof( struct timeval ) );
    while( tv2.tv_sec - tv1.tv_sec < 30 ) {
        /* now read the property back out */
        macErr = PaMacCore_AudioDeviceGetProperty( inDevice, inChannel,
                 isInput, inPropertyID,
                 &outPropertyDataSize, outPropertyData );
        if( macErr ) {
            memset( outPropertyData, 0, inPropertyDataSize );
            goto failMac;
        }
        /* and compare... */
        if( 0==memcmp( outPropertyData, inPropertyData, outPropertyDataSize ) ) {
            PaMacCore_AudioDeviceRemovePropertyListener( inDevice, inChannel, isInput, inPropertyID, propertyProc, NULL);
            return paNoError;
        }
        /* No match yet, so let's sleep and try again. */
        Pa_Sleep( 100 );
        gettimeofday( &tv2, NULL );
    }
    DBUG( ("Timeout waiting for device setting.\n" ) );

    PaMacCore_AudioDeviceRemovePropertyListener( inDevice, inChannel, isInput, inPropertyID, propertyProc, NULL );
    return paNoError;

failMac:
    PaMacCore_AudioDeviceRemovePropertyListener( inDevice, inChannel, isInput, inPropertyID, propertyProc, NULL );
    return ERR( macErr );
}

/*
 * Sets the sample rate the HAL device.
 * if requireExact: set the sample rate or fail.
 *
 * otherwise      : set the exact sample rate.
 *             If that fails, check for available sample rates, and choose one
 *             higher than the requested rate. If there isn't a higher one,
 *             just use the highest available.
 */
PaError setBestSampleRateForDevice( const AudioDeviceID device,
                                    const bool isOutput,
                                    const bool requireExact,
                                    const Float64 desiredSrate )
{
    const bool isInput = isOutput ? 0 : 1;
    Float64 srate;
    UInt32 propsize = sizeof( Float64 );
    OSErr err;
    AudioValueRange *ranges;
    int i=0;
    Float64 max  = -1; /*the maximum rate available*/
    Float64 best = -1; /*the lowest sample rate still greater than desired rate*/
    VDBUG(("Setting sample rate for device %ld to %g.\n",(long)device,(float)desiredSrate));

    /* -- try setting the sample rate -- */
    srate = 0;
    err = AudioDeviceSetPropertyNowAndWaitForChange(
            device, 0, isInput,
            kAudioDevicePropertyNominalSampleRate,
            propsize, &desiredSrate, &srate );

    /* -- if the rate agrees, and was changed, we are done -- */
    if( srate != 0 && srate == desiredSrate )
        return paNoError;
    /* -- if the rate agrees, and we got no errors, we are done -- */
    if( !err && srate == desiredSrate )
        return paNoError;
    /* -- we've failed if the rates disagree and we are setting input -- */
    if( requireExact )
        return paInvalidSampleRate;

    /* -- generate a list of available sample rates -- */
    err = PaMacCore_AudioDeviceGetPropertySize( device, 0, isInput,
            kAudioDevicePropertyAvailableNominalSampleRates,
            &propsize );
    if( err )
        return ERR( err );
    ranges = (AudioValueRange *)calloc( 1, propsize );
    if( !ranges )
        return paInsufficientMemory;
    err = PaMacCore_AudioDeviceGetProperty( device, 0, isInput,
                                            kAudioDevicePropertyAvailableNominalSampleRates,
                                            &propsize, ranges );
    if( err )
    {
        free( ranges );
        return ERR( err );
    }
    VDBUG(("Requested sample rate of %g was not available.\n", (float)desiredSrate));
    VDBUG(("%lu Available Sample Rates are:\n",propsize/sizeof(AudioValueRange)));
#ifdef MAC_CORE_VERBOSE_DEBUG
    for( i=0; i<propsize/sizeof(AudioValueRange); ++i )
        VDBUG( ("\t%g-%g\n",
                (float) ranges[i].mMinimum,
                (float) ranges[i].mMaximum ) );
#endif
    VDBUG(("-----\n"));

    /* -- now pick the best available sample rate -- */
    for( i=0; i<propsize/sizeof(AudioValueRange); ++i )
    {
        if( ranges[i].mMaximum > max ) max = ranges[i].mMaximum;
        if( ranges[i].mMinimum > desiredSrate ) {
            if( best < 0 )
                best = ranges[i].mMinimum;
            else if( ranges[i].mMinimum < best )
                best = ranges[i].mMinimum;
        }
    }
    if( best < 0 )
        best = max;
    VDBUG( ("Maximum Rate %g. best is %g.\n", max, best ) );
    free( ranges );

    /* -- set the sample rate -- */
    propsize = sizeof( best );
    srate = 0;
    err = AudioDeviceSetPropertyNowAndWaitForChange(
            device, 0, isInput,
            kAudioDevicePropertyNominalSampleRate,
            propsize, &best, &srate );

    /* -- if the set rate matches, we are done -- */
    if( srate != 0 && srate == best )
        return paNoError;

    if( err )
        return ERR( err );

    /* -- otherwise, something weird happened: we didn't set the rate, and we got no errors. Just bail. */
    return paInternalError;
}


/*
   Attempts to set the requestedFramesPerBuffer. If it can't set the exact
   value, it settles for something smaller if available. If nothing smaller
   is available, it uses the smallest available size.
   actualFramesPerBuffer will be set to the actual value on successful return.
   OK to pass NULL to actualFramesPerBuffer.
   The logic is very similar too setBestSampleRate only failure here is
   not usually catastrophic.
*/
PaError setBestFramesPerBuffer( const AudioDeviceID device,
                                const bool isOutput,
                                UInt32 requestedFramesPerBuffer,
                                UInt32 *actualFramesPerBuffer )
{
    UInt32 afpb;
    const bool isInput = !isOutput;
    UInt32 propsize = sizeof(UInt32);
    OSErr err;
    AudioValueRange range;

    if( actualFramesPerBuffer == NULL )
    {
        actualFramesPerBuffer = &afpb;
    }

    /* -- try and set exact FPB -- */
    err = PaMacCore_AudioDeviceSetProperty( device, NULL, 0, isInput,
                                            kAudioDevicePropertyBufferFrameSize,
                                            propsize, &requestedFramesPerBuffer);
    err = PaMacCore_AudioDeviceGetProperty( device, 0, isInput,
                                            kAudioDevicePropertyBufferFrameSize,
                                            &propsize, actualFramesPerBuffer);
    if( err )
    {
        return ERR( err );
    }
    // Did we get the size we asked for?
    if( *actualFramesPerBuffer == requestedFramesPerBuffer )
    {
        return paNoError; /* we are done */
    }

    // Clip requested value against legal range for the device.
    propsize = sizeof(AudioValueRange);
    err = PaMacCore_AudioDeviceGetProperty( device, 0, isInput,
                                            kAudioDevicePropertyBufferFrameSizeRange,
                                            &propsize, &range );
    if( err )
    {
        return ERR( err );
    }
    if( requestedFramesPerBuffer < range.mMinimum )
    {
        requestedFramesPerBuffer = range.mMinimum;
    }
    else if( requestedFramesPerBuffer > range.mMaximum )
    {
        requestedFramesPerBuffer = range.mMaximum;
    }

    /* --- set the buffer size (ignore errors) -- */
    propsize = sizeof( UInt32 );
    err = PaMacCore_AudioDeviceSetProperty( device, NULL, 0, isInput,
                                            kAudioDevicePropertyBufferFrameSize,
                                            propsize, &requestedFramesPerBuffer );
    /* --- read the property to check that it was set -- */
    err = PaMacCore_AudioDeviceGetProperty( device, 0, isInput,
                                            kAudioDevicePropertyBufferFrameSize,
                                            &propsize, actualFramesPerBuffer );

    if( err )
        return ERR( err );

    return paNoError;
}

/**********************
 *
 * XRun stuff
 *
 **********************/

struct PaMacXRunListNode_s {
    PaMacCoreStream *stream;
    struct PaMacXRunListNode_s *next;
} ;

typedef struct PaMacXRunListNode_s PaMacXRunListNode;

/** Always empty, so that it can always be the one returned by
    addToXRunListenerList. note that it's not a pointer. */
static PaMacXRunListNode firstXRunListNode;
static int xRunListSize;
static pthread_mutex_t xrunMutex;

OSStatus xrunCallback(
    AudioObjectID inDevice,
    UInt32 inNumberAddresses,
    const AudioObjectPropertyAddress* inAddresses,
    void * inClientData)
{
    PaMacXRunListNode *node = (PaMacXRunListNode *) inClientData;
    bool isInput = inAddresses->mScope == kAudioDevicePropertyScopeInput;

    int ret = pthread_mutex_trylock( &xrunMutex ) ;

    if( ret == 0 ) {

        node = node->next ; //skip the first node

        for( ; node; node=node->next ) {
            PaMacCoreStream *stream = node->stream;

            if( stream->state != ACTIVE )
                continue; //if the stream isn't active, we don't care if the device is dropping

            if( isInput ) {
                if( stream->inputDevice == inDevice )
                    OSAtomicOr32( paInputOverflow, &stream->xrunFlags );
            } else {
                if( stream->outputDevice == inDevice )
                    OSAtomicOr32( paOutputUnderflow, &stream->xrunFlags );
            }
        }

        pthread_mutex_unlock( &xrunMutex );
    }

    return 0;
}

int initializeXRunListenerList( void )
{
    xRunListSize = 0;
    bzero( (void *) &firstXRunListNode, sizeof(firstXRunListNode) );
    return pthread_mutex_init( &xrunMutex, NULL );
}
int destroyXRunListenerList( void )
{
    PaMacXRunListNode *node;
    node = firstXRunListNode.next;
    while( node ) {
        PaMacXRunListNode *tmp = node;
        node = node->next;
        free( tmp );
    }
    xRunListSize = 0;
    return pthread_mutex_destroy( &xrunMutex );
}

void *addToXRunListenerList( void *stream )
{
    pthread_mutex_lock( &xrunMutex );
    PaMacXRunListNode *newNode;
    // setup new node:
    newNode = (PaMacXRunListNode *) malloc( sizeof( PaMacXRunListNode ) );
    newNode->stream = (PaMacCoreStream *) stream;
    newNode->next = firstXRunListNode.next;
    // insert:
    firstXRunListNode.next = newNode;
    pthread_mutex_unlock( &xrunMutex );

    return &firstXRunListNode;
}

int removeFromXRunListenerList( void *stream )
{
    pthread_mutex_lock( &xrunMutex );
    PaMacXRunListNode *node, *prev;
    prev = &firstXRunListNode;
    node = firstXRunListNode.next;
    while( node ) {
        if( node->stream == stream ) {
            //found it:
            --xRunListSize;
            prev->next = node->next;
            free( node );
            pthread_mutex_unlock( &xrunMutex );
            return xRunListSize;
        }
        prev = prev->next;
        node = node->next;
    }

    pthread_mutex_unlock( &xrunMutex );
    // failure
    return xRunListSize;
}
