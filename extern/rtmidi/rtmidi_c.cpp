#include <string.h>
#include <stdlib.h>
#include "rtmidi_c.h"
#include "RtMidi.h"

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
}};

class CallbackProxyUserData
{
  public:
  CallbackProxyUserData (RtMidiCCallback cCallback, void *userData)
    : c_callback (cCallback), user_data (userData)
  {
  }
  RtMidiCCallback c_callback;
  void *user_data;
};

#ifndef RTMIDI_SOURCE_INCLUDED
    extern "C" const enum RtMidiApi rtmidi_compiled_apis[]; // casting from RtMidi::Api[]
#endif
extern "C" const unsigned int rtmidi_num_compiled_apis;

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

void rtmidi_error (MidiApi *api, enum RtMidiErrorType type, const char* errorString)
{
  std::string msg = errorString;
  api->error ((RtMidiError::Type) type, msg);
}

void rtmidi_open_port (RtMidiPtr device, unsigned int portNumber, const char *portName)
{
    std::string name = portName;
    try {
        ((RtMidi*) device->ptr)->openPort (portNumber, name);

    } catch (const RtMidiError & err) {
        device->ok  = false;
        device->msg = err.what ();
    }
}

void rtmidi_open_virtual_port (RtMidiPtr device, const char *portName)
{
    std::string name = portName;
    try {
        ((RtMidi*) device->ptr)->openVirtualPort (name);

    } catch (const RtMidiError & err) {
        device->ok  = false;
        device->msg = err.what ();
    }

}

void rtmidi_close_port (RtMidiPtr device)
{
    try {
        ((RtMidi*) device->ptr)->closePort ();

    } catch (const RtMidiError & err) {
        device->ok  = false;
        device->msg = err.what ();
    }
}

unsigned int rtmidi_get_port_count (RtMidiPtr device)
{
    try {
        return ((RtMidi*) device->ptr)->getPortCount ();

    } catch (const RtMidiError & err) {
        device->ok  = false;
        device->msg = err.what ();
        return -1;
    }
}

int rtmidi_get_port_name (RtMidiPtr device, unsigned int portNumber, char * bufOut, int * bufLen)
{
    if (bufOut == nullptr && bufLen == nullptr) {
        return -1;
    }

    std::string name;
    try {
        name = ((RtMidi*) device->ptr)->getPortName (portNumber);
    } catch (const RtMidiError & err) {
        device->ok  = false;
        device->msg = err.what ();
        return -1;
    }

    if (bufOut == nullptr) {
        *bufLen = static_cast<int>(name.size()) + 1;
        return 0;
    }

    return snprintf(bufOut, static_cast<size_t>(*bufLen), "%s", name.c_str());
}

/* RtMidiIn API */
RtMidiInPtr rtmidi_in_create_default ()
{
    RtMidiWrapper* wrp = new RtMidiWrapper;

    try {
        RtMidiIn* rIn = new RtMidiIn ();

        wrp->ptr = (void*) rIn;
        wrp->data = 0;
        wrp->ok  = true;
        wrp->msg = "";

    } catch (const RtMidiError & err) {
        wrp->ptr = 0;
        wrp->data = 0;
        wrp->ok  = false;
        wrp->msg = err.what ();
    }

    return wrp;
}

RtMidiInPtr rtmidi_in_create (enum RtMidiApi api, const char *clientName, unsigned int queueSizeLimit)
{
    std::string name = clientName;
    RtMidiWrapper* wrp = new RtMidiWrapper;

    try {
        RtMidiIn* rIn = new RtMidiIn ((RtMidi::Api) api, name, queueSizeLimit);

        wrp->ptr = (void*) rIn;
        wrp->data = 0;
        wrp->ok  = true;
        wrp->msg = "";

    } catch (const RtMidiError & err) {
        wrp->ptr = 0;
        wrp->data = 0;
        wrp->ok  = false;
        wrp->msg = err.what ();
    }

    return wrp;
}

void rtmidi_in_free (RtMidiInPtr device)
{
    if (device->data)
      delete (CallbackProxyUserData*) device->data;
    delete (RtMidiIn*) device->ptr;
    delete device;
}

enum RtMidiApi rtmidi_in_get_current_api (RtMidiPtr device)
{
    try {
        return (RtMidiApi) ((RtMidiIn*) device->ptr)->getCurrentApi ();

    } catch (const RtMidiError & err) {
        device->ok  = false;
        device->msg = err.what ();

        return RTMIDI_API_UNSPECIFIED;
    }
}

static
void callback_proxy (double timeStamp, std::vector<unsigned char> *message, void *userData)
{
  CallbackProxyUserData* data = reinterpret_cast<CallbackProxyUserData*> (userData);
  data->c_callback (timeStamp, message->data (), message->size (), data->user_data);
}

void rtmidi_in_set_callback (RtMidiInPtr device, RtMidiCCallback callback, void *userData)
{
    device->data = (void*) new CallbackProxyUserData (callback, userData);
    try {
        ((RtMidiIn*) device->ptr)->setCallback (callback_proxy, device->data);
    } catch (const RtMidiError & err) {
        device->ok  = false;
        device->msg = err.what ();
        delete (CallbackProxyUserData*) device->data;
        device->data = 0;
    }
}

void rtmidi_in_cancel_callback (RtMidiInPtr device)
{
    try {
        ((RtMidiIn*) device->ptr)->cancelCallback ();
        delete (CallbackProxyUserData*) device->data;
        device->data = 0;
    } catch (const RtMidiError & err) {
        device->ok  = false;
        device->msg = err.what ();
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
    try {
        // FIXME: use allocator to achieve efficient buffering
        std::vector<unsigned char> v;
        double ret = ((RtMidiIn*) device->ptr)->getMessage (&v);

        if (v.size () > 0 && v.size() <= *size) {
            memcpy (message, v.data (), (int) v.size ());
        }

        *size = v.size();
        return ret;
    }
    catch (const RtMidiError & err) {
        device->ok  = false;
        device->msg = err.what ();
        return -1;
    }
    catch (...) {
        device->ok  = false;
        device->msg = "Unknown error";
        return -1;
    }
}

/* RtMidiOut API */
RtMidiOutPtr rtmidi_out_create_default ()
{
    RtMidiWrapper* wrp = new RtMidiWrapper;

    try {
        RtMidiOut* rOut = new RtMidiOut ();

        wrp->ptr = (void*) rOut;
        wrp->data = 0;
        wrp->ok  = true;
        wrp->msg = "";

    } catch (const RtMidiError & err) {
        wrp->ptr = 0;
        wrp->data = 0;
        wrp->ok  = false;
        wrp->msg = err.what ();
    }

    return wrp;
}

RtMidiOutPtr rtmidi_out_create (enum RtMidiApi api, const char *clientName)
{
    RtMidiWrapper* wrp = new RtMidiWrapper;
    std::string name = clientName;

    try {
        RtMidiOut* rOut = new RtMidiOut ((RtMidi::Api) api, name);

        wrp->ptr = (void*) rOut;
        wrp->data = 0;
        wrp->ok  = true;
        wrp->msg = "";

    } catch (const RtMidiError & err) {
        wrp->ptr = 0;
        wrp->data = 0;
        wrp->ok  = false;
        wrp->msg = err.what ();
    }


    return wrp;
}

void rtmidi_out_free (RtMidiOutPtr device)
{
    delete (RtMidiOut*) device->ptr;
    delete device;
}

enum RtMidiApi rtmidi_out_get_current_api (RtMidiPtr device)
{
    try {
        return (RtMidiApi) ((RtMidiOut*) device->ptr)->getCurrentApi ();

    } catch (const RtMidiError & err) {
        device->ok  = false;
        device->msg = err.what ();

        return RTMIDI_API_UNSPECIFIED;
    }
}

int rtmidi_out_send_message (RtMidiOutPtr device, const unsigned char *message, int length)
{
    try {
        ((RtMidiOut*) device->ptr)->sendMessage (message, length);
        return 0;
    }
    catch (const RtMidiError & err) {
        device->ok  = false;
        device->msg = err.what ();
        return -1;
    }
    catch (...) {
        device->ok  = false;
        device->msg = "Unknown error";
        return -1;
    }
}
