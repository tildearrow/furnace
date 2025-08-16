/************************************************************************/
/*! \defgroup C-interface
    @{

    \brief C interface to realtime MIDI input/output C++ classes.

    RtMidi offers a C-style interface, principally for use in binding
    RtMidi to other programming languages.  All structs, enums, and
    functions listed here have direct analogs (and simply call to)
    items in the C++ RtMidi class and its supporting classes and
    types
*/
/************************************************************************/

/*!
  \file rtmidi_c.h
 */

#include <stdbool.h>
#include <stddef.h>
#ifndef RTMIDI_C_H
#define RTMIDI_C_H

#if defined(RTMIDI_EXPORT)
#if defined _WIN32 || defined __CYGWIN__
#define RTMIDIAPI __declspec(dllexport)
#else
#define RTMIDIAPI __attribute__((visibility("default")))
#endif
#else
#define RTMIDIAPI //__declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

//! \brief Wraps an RtMidi object for C function return statuses.
struct RtMidiWrapper {
    //! The wrapped RtMidi object.
    void* ptr;
    void* data;

    //! True when the last function call was OK.
    bool  ok;

    //! If an error occurred (ok != true), set to an error message.
    const char* msg;
};

//! \brief Typedef for a generic RtMidi pointer.
typedef struct RtMidiWrapper* RtMidiPtr;

//! \brief Typedef for a generic RtMidiIn pointer.
typedef struct RtMidiWrapper* RtMidiInPtr;

//! \brief Typedef for a generic RtMidiOut pointer.
typedef struct RtMidiWrapper* RtMidiOutPtr;

//! \brief MIDI API specifier arguments.  See \ref RtMidi::Api.
enum RtMidiApi {
    RTMIDI_API_UNSPECIFIED,    /*!< Search for a working compiled API. */
    RTMIDI_API_MACOSX_CORE,    /*!< Macintosh OS-X CoreMIDI API. */
    RTMIDI_API_LINUX_ALSA,     /*!< The Advanced Linux Sound Architecture API. */
    RTMIDI_API_UNIX_JACK,      /*!< The Jack Low-Latency MIDI Server API. */
    RTMIDI_API_WINDOWS_MM,     /*!< The Microsoft Multimedia MIDI API. */
    RTMIDI_API_RTMIDI_DUMMY,   /*!< A compilable but non-functional API. */
    RTMIDI_API_WEB_MIDI_API,   /*!< W3C Web MIDI API. */
    RTMIDI_API_WINDOWS_UWP,    /*!< The Microsoft Universal Windows Platform MIDI API. */
    RTMIDI_API_ANDROID,        /*!< The Android MIDI API. */
    RTMIDI_API_NUM             /*!< Number of values in this enum. */
};

//! \brief Defined RtMidiError types. See \ref RtMidiError::Type.
enum RtMidiErrorType {
  RTMIDI_ERROR_WARNING,           /*!< A non-critical error. */
  RTMIDI_ERROR_DEBUG_WARNING,     /*!< A non-critical error which might be useful for debugging. */
  RTMIDI_ERROR_UNSPECIFIED,       /*!< The default, unspecified error type. */
  RTMIDI_ERROR_NO_DEVICES_FOUND,  /*!< No devices found on system. */
  RTMIDI_ERROR_INVALID_DEVICE,    /*!< An invalid device ID was specified. */
  RTMIDI_ERROR_MEMORY_ERROR,      /*!< An error occurred during memory allocation. */
  RTMIDI_ERROR_INVALID_PARAMETER, /*!< An invalid parameter was specified to a function. */
  RTMIDI_ERROR_INVALID_USE,       /*!< The function was called incorrectly. */
  RTMIDI_ERROR_DRIVER_ERROR,      /*!< A system driver error occurred. */
  RTMIDI_ERROR_SYSTEM_ERROR,      /*!< A system error occurred. */
  RTMIDI_ERROR_THREAD_ERROR       /*!< A thread error occurred. */
};

/*! \brief The type of a RtMidi callback function.
 *
 * \param timeStamp   The time at which the message has been received.
 * \param message     The midi message.
 * \param userData    Additional user data for the callback.
 *
 * See \ref RtMidiIn::RtMidiCallback.
 */
typedef void(* RtMidiCCallback) (double timeStamp, const unsigned char* message,
                                 size_t messageSize, void *userData);


/* RtMidi API */

/*! \brief Return the current RtMidi version.
 *! See \ref RtMidi::getVersion().
*/
RTMIDIAPI const char* rtmidi_get_version();

/*! \brief Determine the available compiled MIDI APIs.
 *
 * If the given `apis` parameter is null, returns the number of available APIs.
 * Otherwise, fill the given apis array with the RtMidi::Api values.
 *
 * \param apis  An array or a null value.
 * \param apis_size  Number of elements pointed to by apis
 * \return number of items needed for apis array if apis==NULL, or
 *         number of items written to apis array otherwise.  A negative
 *         return value indicates an error.
 *
 * See \ref RtMidi::getCompiledApi().
*/
RTMIDIAPI int rtmidi_get_compiled_api (enum RtMidiApi *apis, unsigned int apis_size);

//! \brief Return the name of a specified compiled MIDI API.
//! See \ref RtMidi::getApiName().
RTMIDIAPI const char *rtmidi_api_name(enum RtMidiApi api);

//! \brief Return the display name of a specified compiled MIDI API.
//! See \ref RtMidi::getApiDisplayName().
RTMIDIAPI const char *rtmidi_api_display_name(enum RtMidiApi api);

//! \brief Return the compiled MIDI API having the given name.
//! See \ref RtMidi::getCompiledApiByName().
RTMIDIAPI enum RtMidiApi rtmidi_compiled_api_by_name(const char *name);

//! \internal Report an error.
RTMIDIAPI void rtmidi_error (enum RtMidiErrorType type, const char* errorString);

/*! \brief Open a MIDI port.
 *
 * \param port      Must be greater than 0
 * \param portName  Name for the application port.
 *
 * See RtMidi::openPort().
 */
RTMIDIAPI void rtmidi_open_port (RtMidiPtr device, unsigned int portNumber, const char *portName);

/*! \brief Creates a virtual MIDI port to which other software applications can
 * connect.
 *
 * \param portName  Name for the application port.
 *
 * See RtMidi::openVirtualPort().
 */
RTMIDIAPI void rtmidi_open_virtual_port (RtMidiPtr device, const char *portName);

/*! \brief Close a MIDI connection.
 * See RtMidi::closePort().
 */
RTMIDIAPI void rtmidi_close_port (RtMidiPtr device);

/*! \brief Return the number of available MIDI ports.
 * See RtMidi::getPortCount().
 */
RTMIDIAPI unsigned int rtmidi_get_port_count (RtMidiPtr device);

/*! \brief Access a string identifier for the specified MIDI input port number.
 * 
 * To prevent memory leaks a char buffer must be passed to this function.
 * NULL can be passed as bufOut parameter, and that will write the required buffer length in the bufLen.
 * 
 * See RtMidi::getPortName().
 */
RTMIDIAPI int rtmidi_get_port_name (RtMidiPtr device, unsigned int portNumber, char * bufOut, int * bufLen);

/* RtMidiIn API */

//! \brief Create a default RtMidiInPtr value, with no initialization.
RTMIDIAPI RtMidiInPtr rtmidi_in_create_default (void);

/*! \brief Create a  RtMidiInPtr value, with given api, clientName and queueSizeLimit.
 *
 *  \param api            An optional API id can be specified.
 *  \param clientName     An optional client name can be specified. This
 *                        will be used to group the ports that are created
 *                        by the application.
 *  \param queueSizeLimit An optional size of the MIDI input queue can be
 *                        specified.
 *
 * See RtMidiIn::RtMidiIn().
 */
RTMIDIAPI RtMidiInPtr rtmidi_in_create (enum RtMidiApi api, const char *clientName, unsigned int queueSizeLimit);

//! \brief Free the given RtMidiInPtr.
RTMIDIAPI void rtmidi_in_free (RtMidiInPtr device);

//! \brief Returns the MIDI API specifier for the given instance of RtMidiIn.
//! See \ref RtMidiIn::getCurrentApi().
RTMIDIAPI enum RtMidiApi rtmidi_in_get_current_api (RtMidiPtr device);

//! \brief Set a callback function to be invoked for incoming MIDI messages.
//! See \ref RtMidiIn::setCallback().
RTMIDIAPI void rtmidi_in_set_callback (RtMidiInPtr device, RtMidiCCallback callback, void *userData);

//! \brief Cancel use of the current callback function (if one exists).
//! See \ref RtMidiIn::cancelCallback().
RTMIDIAPI void rtmidi_in_cancel_callback (RtMidiInPtr device);

//! \brief Specify whether certain MIDI message types should be queued or ignored during input.
//! See \ref RtMidiIn::ignoreTypes().
RTMIDIAPI void rtmidi_in_ignore_types (RtMidiInPtr device, bool midiSysex, bool midiTime, bool midiSense);

/*! Fill the user-provided array with the data bytes for the next available
 * MIDI message in the input queue and return the event delta-time in seconds.
 *
 * \param message   Must point to a char* that is already allocated.
 *                  SYSEX messages maximum size being 1024, a statically
 *                  allocated array could
 *                  be sufficient.
 * \param size      Is used to return the size of the message obtained.
 *                  Must be set to the size of \ref message when calling.
 *
 * See RtMidiIn::getMessage().
 */
RTMIDIAPI double rtmidi_in_get_message (RtMidiInPtr device, unsigned char *message, size_t *size);

/* RtMidiOut API */

//! \brief Create a default RtMidiInPtr value, with no initialization.
RTMIDIAPI RtMidiOutPtr rtmidi_out_create_default (void);

/*! \brief Create a RtMidiOutPtr value, with given and clientName.
 *
 *  \param api            An optional API id can be specified.
 *  \param clientName     An optional client name can be specified. This
 *                        will be used to group the ports that are created
 *                        by the application.
 *
 * See RtMidiOut::RtMidiOut().
 */
RTMIDIAPI RtMidiOutPtr rtmidi_out_create (enum RtMidiApi api, const char *clientName);

//! \brief Free the given RtMidiOutPtr.
RTMIDIAPI void rtmidi_out_free (RtMidiOutPtr device);

//! \brief Returns the MIDI API specifier for the given instance of RtMidiOut.
//! See \ref RtMidiOut::getCurrentApi().
RTMIDIAPI enum RtMidiApi rtmidi_out_get_current_api (RtMidiPtr device);

//! \brief Immediately send a single message out an open MIDI output port.
//! See \ref RtMidiOut::sendMessage().
RTMIDIAPI int rtmidi_out_send_message (RtMidiOutPtr device, const unsigned char *message, int length);


#ifdef __cplusplus
}
#endif
#endif

/*! }@ */
