#include <string.h>
#include <stdlib.h>
#include "rtmidi_c.h"
#include "RtMidi.h"

// Fixes build error C4996 on Windows 11 with VS2022
#ifdef _MSC_VER
#define strdup _strdup
#endif

/* Compile-time assertions that will break if the enums are changed in
 * the future without synchronizing them properly.  If you get (g++)
 * "error: ‘StaticEnumAssert<b>::StaticEnumAssert() [with bool b = false]’
 * is private within this context", it means enums are not aligned. */
template<bool b> class StaticEnumAssert { private: StaticEnumAssert() {} };
template<> class StaticEnumAssert<true>{ public: StaticEnumAssert() {} };
#define ENUM_EQUAL(x,y) StaticEnumAssert<(int)x==(int)y>()
class StaticEnumAssertions { StaticEnumAssertions() {
    ENUM_EQUAL( RTMIDI_API_UNSPECIFIED,     RtMidi::UNSPECIFIED );
    ENUM_EQUAL( RTMIDI_API_MACOSX_CORE,     RtMidi::MACOSX_CORE );
    ENUM_EQUAL( RTMIDI_API_LINUX_ALSA,      RtMidi::LINUX_ALSA );
    ENUM_EQUAL( RTMIDI_API_UNIX_JACK,       RtMidi::UNIX_JACK );
    ENUM_EQUAL( RTMIDI_API_WINDOWS_MM,      RtMidi::WINDOWS_MM );
    ENUM_EQUAL( RTMIDI_API_ANDROID,         RtMidi::ANDROID_AMIDI );
    ENUM_EQUAL( RTMIDI_API_RTMIDI_DUMMY,    RtMidi::RTMIDI_DUMMY );
    ENUM_EQUAL( RTMIDI_API_WEB_MIDI_API,    RtMidi::WEB_MIDI_API );
    ENUM_EQUAL( RTMIDI_API_WINDOWS_UWP,     RtMidi::WINDOWS_UWP );
    ENUM_EQUAL( RTMIDI_API_WINDOWS_MIDI_SERVICES, RtMidi::WINDOWS_MIDI_SERVICES );

    ENUM_EQUAL( RTMIDI_ERROR_WARNING,            RtMidiError::WARNING );
    ENUM_EQUAL( RTMIDI_ERROR_DEBUG_WARNING,      RtMidiError::DEBUG_WARNING );
    ENUM_EQUAL( RTMIDI_ERROR_UNSPECIFIED,        RtMidiError::UNSPECIFIED );
    ENUM_EQUAL( RTMIDI_ERROR_NO_DEVICES_FOUND,   RtMidiError::NO_DEVICES_FOUND );
    ENUM_EQUAL( RTMIDI_ERROR_INVALID_DEVICE,     RtMidiError::INVALID_DEVICE );
    ENUM_EQUAL( RTMIDI_ERROR_MEMORY_ERROR,       RtMidiError::MEMORY_ERROR );
    ENUM_EQUAL( RTMIDI_ERROR_INVALID_PARAMETER,  RtMidiError::INVALID_PARAMETER );
    ENUM_EQUAL( RTMIDI_ERROR_INVALID_USE,        RtMidiError::INVALID_USE );
    ENUM_EQUAL( RTMIDI_ERROR_DRIVER_ERROR,       RtMidiError::DRIVER_ERROR );
    ENUM_EQUAL( RTMIDI_ERROR_SYSTEM_ERROR,       RtMidiError::SYSTEM_ERROR );
    ENUM_EQUAL( RTMIDI_ERROR_THREAD_ERROR,       RtMidiError::THREAD_ERROR );
    ENUM_EQUAL( RTMIDI_ERROR_DRIVER_NOT_INSTALLED, RtMidiError::DRIVER_NOT_INSTALLED );
}};

template <typename T>
class CallbackProxyUserData
{
  public:
  CallbackProxyUserData (T cCallback, void *userData)
    : c_callback (cCallback), user_data (userData)
  {
  }
  T c_callback;
  void *user_data;
};

#ifndef RTMIDI_SOURCE_INCLUDED
    extern "C" const enum RtMidiApi rtmidi_compiled_apis[]; // casting from RtMidi::Api[]
#endif
extern "C" const unsigned int rtmidi_num_compiled_apis;

static void rtmidi_set_error_msg (RtMidiPtr device, const char *err);
static void rtmidi_clear_error (RtMidiPtr device);

/* RtMidi API */
const char* rtmidi_get_version()
{
    return RTMIDI_VERSION;
}

int rtmidi_get_compiled_api (enum RtMidiApi *apis, unsigned int apis_size)
{
    unsigned num = rtmidi_num_compiled_apis;
    if (apis) {
        num = (num < apis_size) ? num : apis_size;
        memcpy(apis, rtmidi_compiled_apis, num * sizeof(enum RtMidiApi));
    }
    return (int)num;
}

extern "C" const char* rtmidi_api_names[][2];
const char *rtmidi_api_name(enum RtMidiApi api) {
    if (api < 0 || api >= RTMIDI_API_NUM)
        return NULL;
    return rtmidi_api_names[api][0];
}

const char *rtmidi_api_display_name(enum RtMidiApi api)
{
    if (api < 0 || api >= RTMIDI_API_NUM)
        return "Unknown";
    return rtmidi_api_names[api][1];
}

enum RtMidiApi rtmidi_compiled_api_by_name(const char *name) {
    RtMidi::Api api = RtMidi::UNSPECIFIED;
    if (name) {
        api = RtMidi::getCompiledApiByName(name);
    }
    return (enum RtMidiApi)api;
}

void rtmidi_open_port (RtMidiPtr device, unsigned int portNumber, const char *portName)
{
    rtmidi_clear_error (device);
    std::string name = portName;
    try {
        ((RtMidi*) device->ptr)->openPort (portNumber, name);

    } catch (const RtMidiError & err) {
        device->ok  = false;
        rtmidi_set_error_msg (device, err.what ());
    }
}

void rtmidi_open_virtual_port (RtMidiPtr device, const char *portName)
{
    rtmidi_clear_error (device);
    std::string name = portName;
    try {
        ((RtMidi*) device->ptr)->openVirtualPort (name);

    } catch (const RtMidiError & err) {
        device->ok  = false;
        rtmidi_set_error_msg (device, err.what ());
    }

}

void rtmidi_close_port (RtMidiPtr device)
{
    rtmidi_clear_error (device);
    try {
        ((RtMidi*) device->ptr)->closePort ();

    } catch (const RtMidiError & err) {
        device->ok  = false;
        rtmidi_set_error_msg (device, err.what ());
    }
}

unsigned int rtmidi_get_port_count (RtMidiPtr device)
{
    rtmidi_clear_error (device);
    try {
        return ((RtMidi*) device->ptr)->getPortCount ();

    } catch (const RtMidiError & err) {
        device->ok  = false;
        rtmidi_set_error_msg (device, err.what ());
        return -1;
    }
}

int rtmidi_get_port_name (RtMidiPtr device, unsigned int portNumber, char * bufOut, int * bufLen)
{
    rtmidi_clear_error (device);
    if (bufOut == nullptr && bufLen == nullptr) {
        return -1;
    }

    std::string name;
    try {
        name = ((RtMidi*) device->ptr)->getPortName (portNumber);
    } catch (const RtMidiError & err) {
        device->ok  = false;
        rtmidi_set_error_msg (device, err.what ());
        return -1;
    }

    if (bufOut == nullptr) {
        *bufLen = static_cast<int>(name.size()) + 1;
        return 0;
    }

    return snprintf(bufOut, static_cast<size_t>(*bufLen), "%s", name.c_str());
}

/* RtMidiIn API */

// The RtMidiIn behind an RtMidiInPtr. It holds on to a message that did not fit
// the caller's buffer, so rtmidi_in_get_message() can return it on the next
// call. Kept here rather than in RtMidiWrapper so that struct does not change.
namespace {
struct RtMidiInC : public RtMidiIn
{
    using RtMidiIn::RtMidiIn;
    std::vector<unsigned char> pending;
    double pendingDelta = 0;
};
}

RtMidiInPtr rtmidi_in_create_default ()
{
    RtMidiWrapper* wrp = new RtMidiWrapper{};

    try {
        RtMidiIn* rIn = new RtMidiInC ();

        wrp->ptr = (void*) rIn;
        wrp->callback_proxy = 0;
        wrp->error_callback_proxy = 0;
        wrp->ok  = true;
        rtmidi_set_error_msg (wrp, "");

    } catch (const RtMidiError & err) {
        wrp->ptr = 0;
        wrp->callback_proxy = 0;
        wrp->error_callback_proxy = 0;
        wrp->ok  = false;
        rtmidi_set_error_msg (wrp, err.what ());
    }

    return wrp;
}

RtMidiInPtr rtmidi_in_create (enum RtMidiApi api, const char *clientName, unsigned int queueSizeLimit)
{
    std::string name = clientName;
    RtMidiWrapper* wrp = new RtMidiWrapper{};

    try {
        RtMidiIn* rIn = new RtMidiInC ((RtMidi::Api) api, name, queueSizeLimit);

        wrp->ptr = (void*) rIn;
        wrp->callback_proxy = 0;
        wrp->error_callback_proxy = 0;
        wrp->ok  = true;
        rtmidi_set_error_msg (wrp, "");

    } catch (const RtMidiError & err) {
        wrp->ptr = 0;
        wrp->callback_proxy = 0;
        wrp->error_callback_proxy = 0;
        wrp->ok  = false;
        rtmidi_set_error_msg (wrp, err.what ());
    }

    return wrp;
}

void rtmidi_in_free (RtMidiInPtr device)
{
    if (device->msg)
      free (device->msg);
    if (device->callback_proxy)
      delete (CallbackProxyUserData<RtMidiCCallback>*) device->callback_proxy;
    if (device->error_callback_proxy)
      delete (CallbackProxyUserData<RtMidiErrorCCallback>*) device->error_callback_proxy;
    delete (RtMidiIn*) device->ptr;
    delete device;
}

enum RtMidiApi rtmidi_in_get_current_api (RtMidiPtr device)
{
    rtmidi_clear_error (device);
    try {
        return (RtMidiApi) ((RtMidiIn*) device->ptr)->getCurrentApi ();

    } catch (const RtMidiError & err) {
        device->ok  = false;
        rtmidi_set_error_msg (device, err.what ());

        return RTMIDI_API_UNSPECIFIED;
    }
}

static
void callback_proxy (double timeStamp, std::vector<unsigned char> *message, void *userData)
{
  CallbackProxyUserData<RtMidiCCallback>* proxy = reinterpret_cast<CallbackProxyUserData<RtMidiCCallback>*> (userData);
  proxy->c_callback (timeStamp, message->data (), message->size (), proxy->user_data);
}

static
void error_callback_proxy (RtMidiError::Type type, const std::string &errorText, void *userData)
{
  CallbackProxyUserData<RtMidiErrorCCallback>* proxy = reinterpret_cast<CallbackProxyUserData<RtMidiErrorCCallback>*> (userData);
  proxy->c_callback (static_cast<RtMidiErrorType>(type), errorText.c_str (), proxy->user_data);
}

void rtmidi_in_set_callback (RtMidiInPtr device, RtMidiCCallback callback, void *userData)
{
    rtmidi_clear_error (device);
    device->callback_proxy = (void*) new CallbackProxyUserData<RtMidiCCallback> (callback, userData);
    try {
        ((RtMidiIn*) device->ptr)->setCallback (callback_proxy, device->callback_proxy);
    } catch (const RtMidiError & err) {
        device->ok  = false;
        rtmidi_set_error_msg (device, err.what ());
        delete (CallbackProxyUserData<RtMidiCCallback>*) device->callback_proxy;
        device->callback_proxy = 0;
    }
}

void rtmidi_in_cancel_callback (RtMidiInPtr device)
{
    rtmidi_clear_error (device);
    try {
        ((RtMidiIn*) device->ptr)->cancelCallback ();
        delete (CallbackProxyUserData<RtMidiCCallback>*) device->callback_proxy;
        device->callback_proxy = 0;
    } catch (const RtMidiError & err) {
        device->ok  = false;
        rtmidi_set_error_msg (device, err.what ());
    }
}

void rtmidi_set_error_callback (RtMidiOutPtr device, RtMidiErrorCCallback callback, void *userData)
{
    device->error_callback_proxy = (void*) new CallbackProxyUserData<RtMidiErrorCCallback> (callback, userData);
    try {
        ((RtMidi*) device->ptr)->setErrorCallback (error_callback_proxy, device->error_callback_proxy);
    } catch (const RtMidiError & err) {
        device->ok  = false;
        rtmidi_set_error_msg (device, err.what ());
        delete (CallbackProxyUserData<RtMidiErrorCCallback>*) device->error_callback_proxy;
        device->error_callback_proxy = 0;
    }
}

void rtmidi_in_ignore_types (RtMidiInPtr device, bool midiSysex, bool midiTime, bool midiSense)
{
  ((RtMidiIn*) device->ptr)->ignoreTypes (midiSysex, midiTime, midiSense);
}

double rtmidi_in_get_message (RtMidiInPtr device,
                              unsigned char *message,
                              size_t *size)
{
    rtmidi_clear_error (device);
    try {
        RtMidiInC *in = static_cast<RtMidiInC *> ((RtMidiIn*) device->ptr);

        // Take the next message from the queue, unless one is still waiting
        // from a call whose buffer was too small.
        if (in->pending.empty ()) {
            in->pendingDelta = in->getMessage (&in->pending);
        }
        const size_t length = in->pending.size ();

        // With no buffer, report the length and leave the message for the next call.
        if (message == NULL) {
            *size = length;
            return 0;
        }

        // Too small: keep the message, and say how big the buffer has to be.
        if (length > *size) {
            std::string err = "rtmidi_in_get_message: a " + std::to_string (length)
                + "-byte message did not fit the " + std::to_string (*size)
                + "-byte buffer; it is kept for the next call.";
            *size = length;
            device->ok  = false;
            rtmidi_set_error_msg (device, err.c_str ());
            return -1;
        }

        if (length > 0) {
            memcpy (message, in->pending.data (), length);
        }
        in->pending.clear ();

        *size = length;
        return in->pendingDelta;
    }
    catch (const RtMidiError & err) {
        device->ok  = false;
        rtmidi_set_error_msg (device, err.what ());
        return -1;
    }
    catch (...) {
        device->ok  = false;
        rtmidi_set_error_msg (device, "Unknown error");
        return -1;
    }
}

/* RtMidiOut API */
RtMidiOutPtr rtmidi_out_create_default ()
{
    RtMidiWrapper* wrp = new RtMidiWrapper{};

    try {
        RtMidiOut* rOut = new RtMidiOut ();

        wrp->ptr = (void*) rOut;
        wrp->callback_proxy = 0;
        wrp->error_callback_proxy = 0;
        wrp->ok  = true;
        rtmidi_set_error_msg (wrp, "");

    } catch (const RtMidiError & err) {
        wrp->ptr = 0;
        wrp->callback_proxy = 0;
        wrp->error_callback_proxy = 0;
        wrp->ok  = false;
        rtmidi_set_error_msg (wrp, err.what ());
    }

    return wrp;
}

RtMidiOutPtr rtmidi_out_create (enum RtMidiApi api, const char *clientName)
{
    RtMidiWrapper* wrp = new RtMidiWrapper{};
    std::string name = clientName;

    try {
        RtMidiOut* rOut = new RtMidiOut ((RtMidi::Api) api, name);

        wrp->ptr = (void*) rOut;
        wrp->callback_proxy = 0;
        wrp->error_callback_proxy = 0;
        wrp->ok  = true;
        rtmidi_set_error_msg (wrp, "");

    } catch (const RtMidiError & err) {
        wrp->ptr = 0;
        wrp->callback_proxy = 0;
        wrp->error_callback_proxy = 0;
        wrp->ok  = false;
        rtmidi_set_error_msg (wrp, err.what ());
    }


    return wrp;
}

void rtmidi_out_free (RtMidiOutPtr device)
{
    if (device->msg)
      free (device->msg);
    delete (RtMidiOut*) device->ptr;
    delete device;
}

enum RtMidiApi rtmidi_out_get_current_api (RtMidiPtr device)
{
    rtmidi_clear_error (device);
    try {
        return (RtMidiApi) ((RtMidiOut*) device->ptr)->getCurrentApi ();

    } catch (const RtMidiError & err) {
        device->ok  = false;
        rtmidi_set_error_msg (device, err.what ());

        return RTMIDI_API_UNSPECIFIED;
    }
}

int rtmidi_out_send_message (RtMidiOutPtr device, const unsigned char *message, int length)
{
    rtmidi_clear_error (device);
    try {
        ((RtMidiOut*) device->ptr)->sendMessage (message, length);
        return 0;
    }
    catch (const RtMidiError & err) {
        device->ok  = false;
        rtmidi_set_error_msg (device, err.what ());
        return -1;
    }
    catch (...) {
        device->ok  = false;
        rtmidi_set_error_msg (device, "Unknown error");
        return -1;
    }
}

static void rtmidi_set_error_msg (RtMidiPtr device, const char *err)
{
    if (device->msg) {
        free (device->msg);
    }
    device->msg = strdup(err);
}

static void rtmidi_clear_error (RtMidiPtr device)
{
    device->ok = true;
    if (device->msg) {
        free (device->msg);
        device->msg = 0;
    }
}
