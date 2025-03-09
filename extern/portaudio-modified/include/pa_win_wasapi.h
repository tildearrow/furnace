#ifndef PA_WIN_WASAPI_H
#define PA_WIN_WASAPI_H
/*
 * $Id:  $
 * PortAudio Portable Real-Time Audio Library
 * WASAPI specific extensions
 *
 * Copyright (c) 1999-2018 Ross Bencina and Phil Burk
 * Copyright (c) 2006-2010 David Viens
 * Copyright (c) 2010-2022 Dmitry Kostjuchenko
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

/** @file
 @ingroup public_header
 @brief WASAPI-specific PortAudio API extension header file.
*/

#include "portaudio.h"
#include "pa_win_waveformat.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* Stream setup flags. */
typedef enum PaWasapiFlags
{
    /* put WASAPI into exclusive mode */
    paWinWasapiExclusive                = (1 << 0),

    /* allow to skip internal PA processing completely */
    paWinWasapiRedirectHostProcessor    = (1 << 1),

    /* assign custom channel mask */
    paWinWasapiUseChannelMask           = (1 << 2),

    /* use Polling method (Event method is the default, see details in the IMPORTANT notes) */
    paWinWasapiPolling                  = (1 << 3),

    /* force custom thread priority setting, must be used if PaWasapiStreamInfo::threadPriority
       is set to a custom value */
    paWinWasapiThreadPriority           = (1 << 4),

    /* force explicit sample format and do not allow PA to select suitable working format, API will
       fail if provided sample format is not supported by audio hardware in Exclusive mode
       or system mixer in Shared mode */
    paWinWasapiExplicitSampleFormat     = (1 << 5),

    /* allow API to insert system-level channel matrix mixer and sample rate converter to allow
       playback formats that do not match the current configured system settings.
       this is in particular required for streams not matching the system mixer sample rate.
       only applies in Shared mode. */
    paWinWasapiAutoConvert              = (1 << 6),

    /* use Passthrough mode for sending encoded audio data in PCM containers to the audio device,
       refer to Microsoft documentation "Representing Formats for IEC 61937 Transmissions" for more
       details about data representation and stream configuration */
    paWinWasapiPassthrough              = (1 << 7),
}
PaWasapiFlags;
#define paWinWasapiExclusive             (paWinWasapiExclusive)
#define paWinWasapiRedirectHostProcessor (paWinWasapiRedirectHostProcessor)
#define paWinWasapiUseChannelMask        (paWinWasapiUseChannelMask)
#define paWinWasapiPolling               (paWinWasapiPolling)
#define paWinWasapiThreadPriority        (paWinWasapiThreadPriority)
#define paWinWasapiExplicitSampleFormat  (paWinWasapiExplicitSampleFormat)
#define paWinWasapiAutoConvert           (paWinWasapiAutoConvert)
#define paWinWasapiPassthrough           (paWinWasapiPassthrough)


/* Stream state.

 @note Multiple states can be united into a bitmask.
 @see  PaWasapiStreamStateCallback, PaWasapi_SetStreamStateHandler
*/
typedef enum PaWasapiStreamState
{
    /* state change was caused by the error:

       Example:
       1) If thread execution stopped due to AUDCLNT_E_RESOURCES_INVALIDATED then state
          value will contain paWasapiStreamStateError|paWasapiStreamStateThreadStop.
    */
    paWasapiStreamStateError         = (1 << 0),

    /* processing thread is preparing to start execution */
    paWasapiStreamStateThreadPrepare = (1 << 1),

    /* processing thread started execution (enters its loop) */
    paWasapiStreamStateThreadStart   = (1 << 2),

    /* processing thread stopped execution */
    paWasapiStreamStateThreadStop    = (1 << 3)
}
PaWasapiStreamState;
#define paWasapiStreamStateError         (paWasapiStreamStateError)
#define paWasapiStreamStateThreadPrepare (paWasapiStreamStateThreadPrepare)
#define paWasapiStreamStateThreadStart   (paWasapiStreamStateThreadStart)
#define paWasapiStreamStateThreadStop    (paWasapiStreamStateThreadStop)


/* Host processor.

   Allows to skip internal PA processing completely. paWinWasapiRedirectHostProcessor flag
   must be set to the PaWasapiStreamInfo::flags member in order to have host processor
   redirected to this callback.

   Use with caution! inputFrames and outputFrames depend solely on final device setup.
   To query max values of inputFrames/outputFrames use PaWasapi_GetFramesPerHostBuffer.
*/
typedef void (*PaWasapiHostProcessorCallback) (void *inputBuffer, long inputFrames,
    void *outputBuffer, long outputFrames, void *userData);


/* Stream state handler.

 @param pStream    Pointer to PaStream object.
 @param stateFlags State flags, a collection of values from PaWasapiStreamState enum.
 @param errorId    Error id provided by system API (HRESULT).
 @param userData   Pointer to user data.

 @see   PaWasapiStreamState
*/
typedef void (*PaWasapiStreamStateCallback) (PaStream *pStream, unsigned int stateFlags,
    unsigned int errorId, void *pUserData);


/* Device role. */
typedef enum PaWasapiDeviceRole
{
    eRoleRemoteNetworkDevice = 0,
    eRoleSpeakers,
    eRoleLineLevel,
    eRoleHeadphones,
    eRoleMicrophone,
    eRoleHeadset,
    eRoleHandset,
    eRoleUnknownDigitalPassthrough,
    eRoleSPDIF,
    eRoleHDMI,
    eRoleUnknownFormFactor
}
PaWasapiDeviceRole;


/* Jack connection type. */
typedef enum PaWasapiJackConnectionType
{
    eJackConnTypeUnknown,
    eJackConnType3Point5mm,
    eJackConnTypeQuarter,
    eJackConnTypeAtapiInternal,
    eJackConnTypeRCA,
    eJackConnTypeOptical,
    eJackConnTypeOtherDigital,
    eJackConnTypeOtherAnalog,
    eJackConnTypeMultichannelAnalogDIN,
    eJackConnTypeXlrProfessional,
    eJackConnTypeRJ11Modem,
    eJackConnTypeCombination
}
PaWasapiJackConnectionType;


/* Jack geometric location. */
typedef enum PaWasapiJackGeoLocation
{
    eJackGeoLocUnk = 0,
    eJackGeoLocRear = 0x1, /* matches EPcxGeoLocation::eGeoLocRear */
    eJackGeoLocFront,
    eJackGeoLocLeft,
    eJackGeoLocRight,
    eJackGeoLocTop,
    eJackGeoLocBottom,
    eJackGeoLocRearPanel,
    eJackGeoLocRiser,
    eJackGeoLocInsideMobileLid,
    eJackGeoLocDrivebay,
    eJackGeoLocHDMI,
    eJackGeoLocOutsideMobileLid,
    eJackGeoLocATAPI,
    eJackGeoLocReserved5,
    eJackGeoLocReserved6,
}
PaWasapiJackGeoLocation;


/* Jack general location. */
typedef enum PaWasapiJackGenLocation
{
    eJackGenLocPrimaryBox = 0,
    eJackGenLocInternal,
    eJackGenLocSeparate,
    eJackGenLocOther
}
PaWasapiJackGenLocation;


/* Jack's type of port. */
typedef enum PaWasapiJackPortConnection
{
    eJackPortConnJack = 0,
    eJackPortConnIntegratedDevice,
    eJackPortConnBothIntegratedAndJack,
    eJackPortConnUnknown
}
PaWasapiJackPortConnection;


/* Thread priority. */
typedef enum PaWasapiThreadPriority
{
    eThreadPriorityNone = 0,
    eThreadPriorityAudio,           //!< Default for Shared mode.
    eThreadPriorityCapture,
    eThreadPriorityDistribution,
    eThreadPriorityGames,
    eThreadPriorityPlayback,
    eThreadPriorityProAudio,        //!< Default for Exclusive mode.
    eThreadPriorityWindowManager
}
PaWasapiThreadPriority;


/* Stream descriptor. */
typedef struct PaWasapiJackDescription
{
    unsigned long              channelMapping;
    unsigned long              color; /* derived from macro: #define RGB(r,g,b) ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16))) */
    PaWasapiJackConnectionType connectionType;
    PaWasapiJackGeoLocation    geoLocation;
    PaWasapiJackGenLocation    genLocation;
    PaWasapiJackPortConnection portConnection;
    unsigned int               isConnected;
}
PaWasapiJackDescription;


/** Stream category.
   Note:
    - values are equal to WASAPI AUDIO_STREAM_CATEGORY enum
    - supported since Windows 8.0, noop on earlier versions
    - values 1,2 are deprecated on Windows 10 and not included into enumeration

 @version Available as of 19.6.0
*/
typedef enum PaWasapiStreamCategory
{
    eAudioCategoryOther           = 0,
    eAudioCategoryCommunications  = 3,
    eAudioCategoryAlerts          = 4,
    eAudioCategorySoundEffects    = 5,
    eAudioCategoryGameEffects     = 6,
    eAudioCategoryGameMedia       = 7,
    eAudioCategoryGameChat        = 8,
    eAudioCategorySpeech          = 9,
    eAudioCategoryMovie           = 10,
    eAudioCategoryMedia           = 11
}
PaWasapiStreamCategory;


/** Stream option.
   Note:
    - values are equal to WASAPI AUDCLNT_STREAMOPTIONS enum
    - supported since Windows 8.1, noop on earlier versions

 @version Available as of 19.6.0
*/
typedef enum PaWasapiStreamOption
{
    eStreamOptionNone        = 0, //!< default
    eStreamOptionRaw         = 1, //!< bypass WASAPI Audio Engine DSP effects, supported since Windows 8.1
    eStreamOptionMatchFormat = 2  //!< force WASAPI Audio Engine into a stream format, supported since Windows 10
}
PaWasapiStreamOption;


/** Passthrough format.

    Format ids are obtained from the Microsoft documentation "Representing Formats for IEC 61937 Transmissions"
    and are composed by such formula where GUID is the guid of passthrough format:
    GUID.Data1 << 16 | GUID.Data2.

 @see PaWasapiStreamPassthrough
 @version Available as of 19.8.0
*/
typedef enum PaWasapiPassthroughFormat
{
    ePassthroughFormatPcmIec60958           = 0x00000000,
    ePassthroughFormatDolbyDigital          = 0x00920000,
    ePassthroughFormatMpeg1                 = 0x00030cea,
    ePassthroughFormatMpeg3                 = 0x00040cea,
    ePassthroughFormatMpeg2                 = 0x00050cea,
    ePassthroughFormatAac                   = 0x00060cea,
    ePassthroughFormatDts                   = 0x00080cea,
    ePassthroughFormatDolbyDigitalPlus      = 0x000a0cea,
    ePassthroughFormatDolbyDigitalPlusAtmos = 0x010a0cea,
    ePassthroughFormatDtsHd                 = 0x000b0cea,
    ePassthroughFormatDtsXE1                = 0x010b0cea,
    ePassthroughFormatDtsXE2                = 0x030b0cea,
    ePassthroughFormatDolbyMlp              = 0x000c0cea,
    ePassthroughFormatDolbyMat20            = 0x010c0cea,
    ePassthroughFormatDolbyMat21            = 0x030c0cea,
    ePassthroughFormatWmaPro                = 0x01640000,
    ePassthroughFormatAtrac                 = 0x00080cea,
    ePassthroughFormatOneBitAudio           = 0x00090cea,
    ePassthroughFormatDst                   = 0x000d0cea,
}
PaWasapiPassthroughFormat;


/** Passthrough details.

    Passthrough details provide direct link to the additional members in WAVEFORMATEXTENSIBLE_IEC61937.
    Passthrough mode allows to pass encoded data inside the PCM containers to the audio device.

    Detailed description about supported formats and examples are provided in Microsoft documentation
    "Representing Formats for IEC 61937 Transmissions".

 @see paWinWasapiPassthrough
 @version Available as of 19.8.0
*/
typedef struct PaWasapiStreamPassthrough
{
    PaWasapiPassthroughFormat formatId;
    unsigned int encodedSamplesPerSec;
    unsigned int encodedChannelCount;
    unsigned int averageBytesPerSec;
}
PaWasapiStreamPassthrough;


/* Stream descriptor. */
typedef struct PaWasapiStreamInfo
{
    unsigned long size;             /**< sizeof(PaWasapiStreamInfo) */
    PaHostApiTypeId hostApiType;    /**< paWASAPI */
    unsigned long version;          /**< 1 */

    unsigned long flags;            /**< collection of PaWasapiFlags */

    /** Support for WAVEFORMATEXTENSIBLE channel masks. If flags contains
       paWinWasapiUseChannelMask this allows you to specify which speakers
       to address in a multichannel stream. Constants for channelMask
       are specified in pa_win_waveformat.h. Will be used only if
       paWinWasapiUseChannelMask flag is specified.
    */
    PaWinWaveFormatChannelMask channelMask;

    /** Delivers raw data to callback obtained from GetBuffer() methods skipping
       internal PortAudio processing inventory completely. userData parameter will
       be the same that was passed to Pa_OpenStream method. Will be used only if
       paWinWasapiRedirectHostProcessor flag is specified.
    */
    PaWasapiHostProcessorCallback hostProcessorOutput;
    PaWasapiHostProcessorCallback hostProcessorInput;

    /** Specifies thread priority explicitly. Will be used only if paWinWasapiThreadPriority flag
       is specified.

       Please note, if Input/Output streams are opened simultaneously (Full-Duplex mode)
       you shall specify same value for threadPriority or othervise one of the values will be used
       to setup thread priority.
    */
    PaWasapiThreadPriority threadPriority;

    /** Stream category.
     @see PaWasapiStreamCategory
     @version Available as of 19.6.0
    */
    PaWasapiStreamCategory streamCategory;

    /** Stream option.
     @see PaWasapiStreamOption
     @version Available as of 19.6.0
    */
    PaWasapiStreamOption streamOption;

    /** Passthrough details.
     @note paWinWasapiPassthrough flag must be specified in PaWasapiStreamInfo::flags to enable Passthrough mode.
     @see paWinWasapiPassthrough
     @version Available as of 19.7.0
    */
    PaWasapiStreamPassthrough passthrough;
}
PaWasapiStreamInfo;


/** Returns pointer to WASAPI's IAudioClient object of the stream.

 @param pStream      Pointer to PaStream object.
 @param pAudioClient Pointer to pointer of IAudioClient.
 @param bOutput      TRUE (1) for output stream, FALSE (0) for input stream.

 @return Error code indicating success or failure.
*/
PaError PaWasapi_GetAudioClient( PaStream *pStream, void **pAudioClient, int bOutput );


/** Update device list.

    This function is available if PA_WASAPI_MAX_CONST_DEVICE_COUNT is defined during compile time
    with maximum constant WASAPI device count (recommended value - 32).
    If PA_WASAPI_MAX_CONST_DEVICE_COUNT is set to 0 (or not defined) during compile time the implementation
    will not define PaWasapi_UpdateDeviceList() and thus updating device list can only be possible by calling
    Pa_Terminate() and then Pa_Initialize().

 @return Error code indicating success or failure.
*/
PaError PaWasapi_UpdateDeviceList();


/** Get current audio format of the device assigned to the opened stream.

    Format is represented by PaWinWaveFormat or WAVEFORMATEXTENSIBLE structure.
    Use this function to reconfirm format if PA's processor is overridden and
    paWinWasapiRedirectHostProcessor flag is specified.

 @param pStream    Pointer to PaStream object.
 @param pFormat    Pointer to PaWinWaveFormat or WAVEFORMATEXTENSIBLE structure.
 @param formatSize Size of PaWinWaveFormat or WAVEFORMATEXTENSIBLE structure in bytes.
 @param bOutput    TRUE (1) for output stream, FALSE (0) for input stream.

 @return Non-negative value indicating the number of bytes copied into format descriptor
         or, a PaErrorCode (which is always negative) if PortAudio is not initialized
         or an error is encountered.
*/
int PaWasapi_GetDeviceCurrentFormat( PaStream *pStream, void *pFormat, unsigned int formatSize, int bOutput );


/** Get default audio format for the device in Shared Mode.

    Format is represented by PaWinWaveFormat or WAVEFORMATEXTENSIBLE structure and obtained
    by getting the device property with a PKEY_AudioEngine_DeviceFormat key.

 @param  pFormat    Pointer to PaWinWaveFormat or WAVEFORMATEXTENSIBLE structure.
 @param  formatSize Size of PaWinWaveFormat or WAVEFORMATEXTENSIBLE structure in bytes.
 @param  device     Device index.

 @return Non-negative value indicating the number of bytes copied into format descriptor
         or, a PaErrorCode (which is always negative) if PortAudio is not initialized
         or an error is encountered.
*/
int PaWasapi_GetDeviceDefaultFormat( void *pFormat, unsigned int formatSize, PaDeviceIndex device );


/** Get mix audio format for the device in Shared Mode.

    Format is represented by PaWinWaveFormat or WAVEFORMATEXTENSIBLE structureand obtained by
    IAudioClient::GetMixFormat.

 @param  pFormat    Pointer to PaWinWaveFormat or WAVEFORMATEXTENSIBLE structure.
 @param  formatSize Size of PaWinWaveFormat or WAVEFORMATEXTENSIBLE structure in bytes.
 @param  device     Device index.

 @return Non-negative value indicating the number of bytes copied into format descriptor
         or, a PaErrorCode (which is always negative) if PortAudio is not initialized
         or an error is encountered.
*/
int PaWasapi_GetDeviceMixFormat( void *pFormat, unsigned int formatSize, PaDeviceIndex device );


/** Get device role (PaWasapiDeviceRole enum).

 @param  device Device index.

 @return Non-negative value indicating device role or, a PaErrorCode (which is always negative)
         if PortAudio is not initialized or an error is encountered.
*/
int/*PaWasapiDeviceRole*/ PaWasapi_GetDeviceRole( PaDeviceIndex device );


/** Get device IMMDevice pointer.

 @param device Device index.
 @param pAudioClient Pointer to pointer of IMMDevice.

 @return Error code indicating success or failure.
*/
PaError PaWasapi_GetIMMDevice( PaDeviceIndex device, void **pIMMDevice );


/** Get device loopback state:

    0 - Not loopback,
    1 - Loopback,
    negative - PaErrorCode.

 @param device Device index.

 @return Non-negative value indicating loopback state or, a PaErrorCode (which is always negative)
         if PortAudio is not initialized or an error is encountered.
*/
int PaWasapi_IsLoopback( PaDeviceIndex device );


/** Boost thread priority of calling thread (MMCSS).

    Use it for Blocking Interface only inside the thread which makes calls to Pa_WriteStream/Pa_ReadStream.

 @param  pTask          Handle to pointer to priority task. Must be used with PaWasapi_RevertThreadPriority
                        method to revert thread priority to initial state.

 @param  priorityClass  Id of thread priority of PaWasapiThreadPriority type. Specifying
                        eThreadPriorityNone does nothing.

 @return Error code indicating success or failure.
 @see    PaWasapi_RevertThreadPriority
*/
PaError PaWasapi_ThreadPriorityBoost( void **pTask, PaWasapiThreadPriority priorityClass );


/** Boost thread priority of calling thread (MMCSS).

    Use it for Blocking Interface only inside the thread which makes calls to Pa_WriteStream/Pa_ReadStream.

 @param  pTask Task handle obtained by PaWasapi_BoostThreadPriority method.

 @return Error code indicating success or failure.
 @see    PaWasapi_BoostThreadPriority
*/
PaError PaWasapi_ThreadPriorityRevert( void *pTask );


/** Get number of frames per host buffer.

    It is max value of frames of WASAPI buffer which can be locked for operations.
    Use this method as helper to find out max values of inputFrames/outputFrames
    of PaWasapiHostProcessorCallback.

 @param  pStream Pointer to PaStream object.
 @param  pInput  Pointer to variable to receive number of input frames. Can be NULL.
 @param  pOutput Pointer to variable to receive number of output frames. Can be NULL.

 @return Error code indicating success or failure.
 @see    PaWasapiHostProcessorCallback
*/
PaError PaWasapi_GetFramesPerHostBuffer( PaStream *pStream, unsigned int *pInput, unsigned int *pOutput );


/** Get number of jacks associated with a WASAPI device.

    Use this method to determine if there are any jacks associated with the provided WASAPI device.
    Not all audio devices will support this capability. This is valid for both input and output devices.

 @note   Not available on UWP platform.

 @param  device     Device index.
 @param  pJackCount Pointer to variable to receive number of jacks.

 @return Error code indicating success or failure.
 @see    PaWasapi_GetJackDescription
 */
PaError PaWasapi_GetJackCount( PaDeviceIndex device, int *pJackCount );


/** Get the jack description associated with a WASAPI device and jack number.

    Before this function is called, use PaWasapi_GetJackCount to determine the
    number of jacks associated with device.  If jcount is greater than zero, then
    each jack from 0 to jcount can be queried with this function to get the jack
    description.

 @note   Not available on UWP platform.

 @param  device           Device index.
 @param  jackIndex        Jack index.
 @param  pJackDescription Pointer to PaWasapiJackDescription.

 @return Error code indicating success or failure.
 @see PaWasapi_GetJackCount
 */
PaError PaWasapi_GetJackDescription( PaDeviceIndex device, int jackIndex, PaWasapiJackDescription *pJackDescription );


/** Set stream state handler.

 @param  pStream        Pointer to PaStream object.
 @param  fnStateHandler Pointer to state handling function.
 @param  pUserData      Pointer to user data.

 @return Error code indicating success or failure.
*/
PaError PaWasapi_SetStreamStateHandler( PaStream *pStream, PaWasapiStreamStateCallback fnStateHandler, void *pUserData );


/** Set default device Id.

    By default implementation will use the DEVINTERFACE_AUDIO_RENDER and
    DEVINTERFACE_AUDIO_CAPTURE Ids if device Id is not provided explicitly. These default Ids
    will not allow to use Exclusive mode on UWP/WinRT platform and thus you must provide
    device Id explicitly via this API before calling the Pa_OpenStream().

    Device Ids on UWP platform are obtainable via:
    Windows::Media::Devices::MediaDevice::GetDefaultAudioRenderId() or
    Windows::Media::Devices::MediaDevice::GetDefaultAudioCaptureId() API.

    After the call completes, memory referenced by pointers can be freed, as implementation keeps its own copy.

    Call this function before calling Pa_IsFormatSupported() when Exclusive mode is requested.

    See an example in the IMPORTANT notes.

 @note   UWP/WinRT platform only.

 @param  pId     Device Id, pointer to the 16-bit Unicode string (WCHAR). If NULL then device Id
                 will be reset to the default, e.g. DEVINTERFACE_AUDIO_RENDER or DEVINTERFACE_AUDIO_CAPTURE.
 @param  bOutput TRUE (1) for output (render), FALSE (0) for input (capture).

 @return Error code indicating success or failure. Will return paIncompatibleStreamHostApi if library is not compiled
         for UWP/WinRT platform. If Id is longer than PA_WASAPI_DEVICE_ID_LEN characters paBufferTooBig will
         be returned.
*/
PaError PaWasapiWinrt_SetDefaultDeviceId( const unsigned short *pId, int bOutput );


/** Populate the device list.

    By default the implementation will rely on DEVINTERFACE_AUDIO_RENDER and DEVINTERFACE_AUDIO_CAPTURE as
    default devices. If device Id is provided by PaWasapiWinrt_SetDefaultDeviceId() then those
    device Ids will be used as default and only devices for the device list.

    By populating the device list you can provide an additional available audio devices of the system to PA
    which are obtainable by:
    Windows::Devices::Enumeration::DeviceInformation::FindAllAsync(selector) where selector is obtainable by
    Windows::Media::Devices::MediaDevice::GetAudioRenderSelector() or
    Windows::Media::Devices::MediaDevice::GetAudioCaptureSelector() API.

    After the call completes, memory referenced by pointers can be freed, as implementation keeps its own copy.

    You must call PaWasapi_UpdateDeviceList() to update the internal device list of the implementation after
    calling this function.

    See an example in the IMPORTANT notes.

 @note   UWP/WinRT platform only.

 @param  pId     Array of device Ids, pointer to the array of pointers of 16-bit Unicode string (WCHAR). If NULL
                 and count is also 0 then device Ids will be reset to the default. Required.
 @param  pName   Array of device Names, pointer to the array of pointers of 16-bit Unicode string (WCHAR). Optional.
 @param  pRole   Array of device Roles, see PaWasapiDeviceRole and PaWasapi_GetDeviceRole() for more details. Optional.
 @param  count   Number of devices, the number of array elements (pId, pName, pRole). Maximum count of devices
                 is limited by PA_WASAPI_DEVICE_MAX_COUNT.
 @param  bOutput TRUE (1) for output (render), FALSE (0) for input (capture).

 @return Error code indicating success or failure. Will return paIncompatibleStreamHostApi if library is not compiled
         for UWP/WinRT platform. If Id is longer than PA_WASAPI_DEVICE_ID_LEN characters paBufferTooBig will
         be returned. If Name is longer than PA_WASAPI_DEVICE_NAME_LEN characters paBufferTooBig will
         be returned.
*/
PaError PaWasapiWinrt_PopulateDeviceList( const unsigned short **pId, const unsigned short **pName,
    const PaWasapiDeviceRole *pRole, unsigned int count, int bOutput );


/*
    IMPORTANT:

    WASAPI is implemented for Callback and Blocking interfaces. It supports Shared and Exclusive
    share modes.

    Exclusive Mode:

        Exclusive mode allows to deliver audio data directly to hardware bypassing
        software mixing.
        Exclusive mode is specified by 'paWinWasapiExclusive' flag.

    Callback Interface:

        Provides best audio quality with low latency. Callback interface is implemented in
        two versions:

        1) Event-driven:
        It is the most powerful data processing method which provides glitch-free audio with
        around 3 ms latency in Exclusive mode. Lowest possible latency for this mode is
        3 ms for HD Audio class audio chips. For the Shared mode latency can not be
        lower than 20 ms. This method consumes slightly less CPU in comparison to Polling.
        It is the default processing method unless 'paWinWasapiPolling' is specified.

        2) Poll-driven:
        Polling is an alternative to Event-driven processing. Due to its nature Polling consumes
        slightly more CPU. This method is less efficient than Event-driven and its lowest possible
        latency is around 10-13 ms.
        Note: Newer Windows versions (for example 11) allow to achieve similar to Event-driven
        low latency.
        Note: Polling must be used to overcome system bug of Windows Vista (x64) when application
        is WOW64 (32-bit process running on 64-bit OS) that results in WASAPI callback timeout if
        Event-driven method is selected (event handle is never signalled on buffer completion).
        This WOW64 bug does not exist in Windows Vista (x86) or Windows 7 or newer Windows versions.
        Polling can be activated by specifying 'paWinWasapiPolling' flag. Our implementation
        detects WOW64 bug and sets 'paWinWasapiPolling' automatically.

    Thread priority:

        Normally thread priority is set automatically and does not require modification. Although
        if user wants some tweaking thread priority can be modified by setting 'paWinWasapiThreadPriority'
        flag and specifying 'PaWasapiStreamInfo::threadPriority' with value from PaWasapiThreadPriority
        enum.

    Blocking Interface:

        Blocking interface is implemented but due to above described Poll-Driven method can not
        deliver lowest possible latency. Specifying too low latency in Shared mode will result in
        distorted audio although Exclusive mode adds stability.

    8.24 format:

        If paCustomFormat is specified as sample format then the implementation will understand it
        as valid 24-bits inside 32-bit container (e.g. wBitsPerSample = 32, Samples.wValidBitsPerSample = 24).

        By using paCustomFormat there will be small optimization when samples are be copied
        with Copy_24_To_24 by PA processor instead of conversion from packed 3-byte (24-bit) data
        with Int24_To_Int32.

    Pa_IsFormatSupported:

        To check format with correct Share Mode (Exclusive/Shared) you must supply PaWasapiStreamInfo
        with flags paWinWasapiExclusive set through member of PaStreamParameters::hostApiSpecificStreamInfo
        structure.

        If paWinWasapiExplicitSampleFormat flag is provided then implementation will not try to select
        suitable close format and will return an error instead of paFormatIsSupported. By specifying
        paWinWasapiExplicitSampleFormat flag it is possible to find out what sample formats are
        supported by Exclusive or Shared modes.

    Pa_OpenStream:

        To set desired Share Mode (Exclusive/Shared) you must supply
        PaWasapiStreamInfo with flags paWinWasapiExclusive set through member of
        PaStreamParameters::hostApiSpecificStreamInfo structure.

    Coding style for parameters and structure members of the public API:

        1) bXXX - boolean, [1 (TRUE), 0 (FALSE)]
        2) pXXX - pointer
        3) fnXXX - pointer to function
        4) structure members are never prefixed with a type distinguisher


    UWP/WinRT:

        This platform has number of limitations which do not allow to enumerate audio devices without
        an additional external help. Enumeration is possible though from C++/CX, check the related API
        Windows::Devices::Enumeration::DeviceInformation::FindAllAsync().

        The main limitation is an absence of the device enumeration from inside the PA's implementation.
        This problem can be solved by using the following functions:

        PaWasapiWinrt_SetDefaultDeviceId() - to set default input/output device,
        PaWasapiWinrt_PopulateDeviceList() - to populate device list with devices.

        Here is an example of populating the device list which can also be updated dynamically depending on
        whether device was removed from or added to the system:

        ----------------

        std::vector<const UINT16 *> ids, names;
        std::vector<PaWasapiDeviceRole> role;

        ids.resize(count);
        names.resize(count);
        role.resize(count);

        for (UINT32 i = 0; i < count; ++i)
        {
            ids[i]   = (const UINT16 *)device_ids[i].c_str();
            names[i] = (const UINT16 *)device_names[i].c_str();
            role[i]  = eRoleUnknownFormFactor;
        }

        PaWasapiWinrt_SetDefaultDeviceId((const UINT16 *)default_device_id.c_str(), !capture);
        PaWasapiWinrt_PopulateDeviceList(ids.data(), names.data(), role.data(), count, !capture);
        PaWasapi_UpdateDeviceList();

        ----------------
*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PA_WIN_WASAPI_H */
