/**********************************************************************/
/*! \class RtMidi
    \brief An abstract base class for realtime MIDI input/output.

    This class implements some common functionality for the realtime
    MIDI input/output subclasses RtMidiIn and RtMidiOut.

    RtMidi GitHub site: https://github.com/thestk/rtmidi
    RtMidi WWW site: https://caml.music.mcgill.ca/~gary/rtmidi/

    RtMidi: realtime MIDI i/o C++ classes
    Copyright (c) 2003-2023 Gary P. Scavone

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation files
    (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    Any person wishing to distribute modifications to the Software is
    asked to send the modifications to the original developer so that
    they can be incorporated into the canonical version.  This is,
    however, not a binding provision of this license.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
    ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
    CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/**********************************************************************/

// The old backend and option names start with underscores, which C++ reserves
// (#133). They still work in 6.1.0 but are deprecated and will be removed in 7.0.0.
#if defined(__LINUX_ALSA__) && !defined(RTMIDI_API_ALSA)
  #define RTMIDI_API_ALSA
#endif
#if defined(__UNIX_JACK__) && !defined(RTMIDI_API_JACK)
  #define RTMIDI_API_JACK
#endif
#if defined(__MACOSX_CORE__) && !defined(RTMIDI_API_CORE)
  #define RTMIDI_API_CORE
#endif
#if defined(__WINDOWS_MM__) && !defined(RTMIDI_API_WINMM)
  #define RTMIDI_API_WINMM
#endif
#if defined(__WINDOWS_UWP__) && !defined(RTMIDI_API_UWP)
  #define RTMIDI_API_UWP
#endif
#if defined(__WINDOWS_MIDI_SERVICES__) && !defined(RTMIDI_API_WMS)
  #define RTMIDI_API_WMS
#endif
#if defined(__WEB_MIDI_API__) && !defined(RTMIDI_API_WEBMIDI)
  #define RTMIDI_API_WEBMIDI
#endif
#if defined(__AMIDI__) && !defined(RTMIDI_API_AMIDI)
  #define RTMIDI_API_AMIDI
#endif
#if defined(__RTMIDI_DEBUG__) && !defined(RTMIDI_DEBUG)
  #define RTMIDI_DEBUG
#endif
#if defined(JACK_HAS_PORT_RENAME) && !defined(RTMIDI_JACK_HAS_PORT_RENAME)
  #define RTMIDI_JACK_HAS_PORT_RENAME
#endif

#include "RtMidi.h"
#include <algorithm>
#include <sstream>

using namespace rt::midi;

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#if (TARGET_OS_IPHONE == 1)

    #define AudioGetCurrentHostTime CAHostTimeBase::GetCurrentTime
    #define AudioConvertHostTimeToNanos CAHostTimeBase::ConvertToNanos

    #include <mach/mach_time.h>
    class CTime2nsFactor
    {
    public:
        CTime2nsFactor()
        {
            mach_timebase_info_data_t tinfo;
            mach_timebase_info(&tinfo);
            Factor = (double)tinfo.numer / tinfo.denom;
        }
        static double Factor;
    };
    double CTime2nsFactor::Factor;
    static CTime2nsFactor InitTime2nsFactor;
    #undef AudioGetCurrentHostTime
    #undef AudioConvertHostTimeToNanos
  #define AudioGetCurrentHostTime (uint64_t) mach_absolute_time
  #define AudioConvertHostTimeToNanos(t) t *CTime2nsFactor::Factor
  #define EndianS32_BtoN(n) n

#endif

// Default for Windows is to add an identifier to the port names; this
// flag can be defined (e.g. in your project file) to disable this behaviour.
//#define RTMIDI_DO_NOT_ENSURE_UNIQUE_PORTNAMES

// Default for Windows UWP is to enable a workaround to fix BLE-MIDI IN ports'
// wrong timestamps that occur at least in Windows 10 21H2;
// this flag can be defined (e.g. in your project file)
// to disable this behavior.
//#define RTMIDI_DO_NOT_ENABLE_WORKAROUND_UWP_WRONG_TIMESTAMPS

// **************************************************************** //
//
// MidiInApi and MidiOutApi subclass prototypes.
//
// **************************************************************** //

// The dummy backend is always compiled in.  It is the fallback when no other
// API is available, and applications also select it deliberately: a headless
// build, a CI runner with no MIDI, or a test that needs RtMidi to construct
// without touching hardware.  Before, it existed only when no other backend
// did, so asking for RTMIDI_DUMMY on a machine that had a real backend threw
// "no compiled API support found" instead.
#define RTMIDI_API_DUMMY

#if defined(RTMIDI_API_CORE)
#include <CoreMIDI/CoreMIDI.h>

class MidiInCore: public MidiInApi
{
 public:
  MidiInCore( const std::string &clientName, unsigned int queueSizeLimit );
  ~MidiInCore( void );
  RtMidi::Api getCurrentApi( void ) { return RtMidi::MACOSX_CORE; };
  void openPort( unsigned int portNumber, const std::string &portName );
  void openVirtualPort( const std::string &portName );
  void closePort( void );
  void setClientName( const std::string &clientName );
  void setPortName( const std::string &portName );
  unsigned int getPortCount( void );
  std::string getPortName( unsigned int portNumber );

 protected:
  MIDIClientRef getCoreMidiClientSingleton(const std::string& clientName) throw();
  void initialize( const std::string& clientName );
};

class MidiOutCore: public MidiOutApi
{
 public:
  MidiOutCore( const std::string &clientName );
  ~MidiOutCore( void );
  RtMidi::Api getCurrentApi( void ) { return RtMidi::MACOSX_CORE; };
  void openPort( unsigned int portNumber, const std::string &portName );
  void openVirtualPort( const std::string &portName );
  void closePort( void );
  void setClientName( const std::string &clientName );
  void setPortName( const std::string &portName );
  unsigned int getPortCount( void );
  std::string getPortName( unsigned int portNumber );
  void sendMessage( const unsigned char *message, size_t size );

 protected:
  MIDIClientRef getCoreMidiClientSingleton(const std::string& clientName) throw();
  void initialize( const std::string& clientName );
};

#endif

#if defined(RTMIDI_API_JACK)

class MidiInJack: public MidiInApi
{
 public:
  MidiInJack( const std::string &clientName, unsigned int queueSizeLimit );
  ~MidiInJack( void );
  RtMidi::Api getCurrentApi( void ) { return RtMidi::UNIX_JACK; };
  void openPort( unsigned int portNumber, const std::string &portName );
  void openVirtualPort( const std::string &portName );
  void closePort( void );
  void setClientName( const std::string &clientName );
  void setPortName( const std::string &portName);
  unsigned int getPortCount( void );
  std::string getPortName( unsigned int portNumber );

 protected:
  std::string clientName;

  void connect( void );
  void initialize( const std::string& clientName );
};

class MidiOutJack: public MidiOutApi
{
 public:
  MidiOutJack( const std::string &clientName );
  ~MidiOutJack( void );
  RtMidi::Api getCurrentApi( void ) { return RtMidi::UNIX_JACK; };
  void openPort( unsigned int portNumber, const std::string &portName );
  void openVirtualPort( const std::string &portName );
  void closePort( void );
  void setClientName( const std::string &clientName );
  void setPortName( const std::string &portName);
  unsigned int getPortCount( void );
  std::string getPortName( unsigned int portNumber );
  void sendMessage( const unsigned char *message, size_t size );

 protected:
  std::string clientName;

  void connect( void );
  void initialize( const std::string& clientName );
};

#endif

#if defined(RTMIDI_API_ALSA)

class MidiInAlsa: public MidiInApi
{
 public:
  MidiInAlsa( const std::string &clientName, unsigned int queueSizeLimit );
  ~MidiInAlsa( void );
  RtMidi::Api getCurrentApi( void ) { return RtMidi::LINUX_ALSA; };
  void openPort( unsigned int portNumber, const std::string &portName );
  void openVirtualPort( const std::string &portName );
  void closePort( void );
  void setClientName( const std::string &clientName );
  void setPortName( const std::string &portName);
  unsigned int getPortCount( void );
  std::string getPortName( unsigned int portNumber );

 protected:
  void initialize( const std::string& clientName );
};

class MidiOutAlsa: public MidiOutApi
{
 public:
  MidiOutAlsa( const std::string &clientName );
  ~MidiOutAlsa( void );
  RtMidi::Api getCurrentApi( void ) { return RtMidi::LINUX_ALSA; };
  void openPort( unsigned int portNumber, const std::string &portName );
  void openVirtualPort( const std::string &portName );
  void closePort( void );
  void setClientName( const std::string &clientName );
  void setPortName( const std::string &portName );
  unsigned int getPortCount( void );
  std::string getPortName( unsigned int portNumber );
  void sendMessage( const unsigned char *message, size_t size );

 protected:
  void initialize( const std::string& clientName );
};

#endif

#if defined(RTMIDI_API_WINMM)

class MidiInWinMM: public MidiInApi
{
 public:
  MidiInWinMM( const std::string &clientName, unsigned int queueSizeLimit );
  ~MidiInWinMM( void );
  RtMidi::Api getCurrentApi( void ) { return RtMidi::WINDOWS_MM; };
  void openPort( unsigned int portNumber, const std::string &portName );
  void openVirtualPort( const std::string &portName );
  void closePort( void );
  void setClientName( const std::string &clientName );
  void setPortName( const std::string &portName );
  unsigned int getPortCount( void );
  std::string getPortName( unsigned int portNumber );

 protected:
  void initialize( const std::string& clientName );
};

class MidiOutWinMM: public MidiOutApi
{
 public:
  MidiOutWinMM( const std::string &clientName );
  ~MidiOutWinMM( void );
  RtMidi::Api getCurrentApi( void ) { return RtMidi::WINDOWS_MM; };
  void openPort( unsigned int portNumber, const std::string &portName );
  void openVirtualPort( const std::string &portName );
  void closePort( void );
  void setClientName( const std::string &clientName );
  void setPortName( const std::string &portName );
  unsigned int getPortCount( void );
  std::string getPortName( unsigned int portNumber );
  void sendMessage( const unsigned char *message, size_t size );

 protected:
  void initialize( const std::string& clientName );
};

#endif

#if defined(RTMIDI_API_UWP)

class MidiInWinUWP : public MidiInApi
{
public:
    MidiInWinUWP(const std::string& clientName, unsigned int queueSizeLimit);
    ~MidiInWinUWP(void) override;
    RtMidi::Api getCurrentApi(void) override { return RtMidi::WINDOWS_UWP; };
    void openPort(unsigned int portNumber, const std::string& portName) override;
    void openVirtualPort(const std::string& portName) override;
    void closePort(void) override;
    void setClientName(const std::string& clientName) override;
    void setPortName(const std::string& portName) override;
    unsigned int getPortCount(void) override;
    std::string getPortName(unsigned int portNumber) override;
    double getMessage(std::vector<unsigned char>* message) override;

protected:
    void initialize(const std::string& clientName) override;
};

class MidiOutWinUWP : public MidiOutApi
{
public:
    MidiOutWinUWP(const std::string& clientName);
    ~MidiOutWinUWP(void) override;
    RtMidi::Api getCurrentApi(void) override { return RtMidi::WINDOWS_UWP; };
    void openPort(unsigned int portNumber, const std::string& portName) override;
    void openVirtualPort(const std::string& portName) override;
    void closePort(void) override;
    void setClientName(const std::string& clientName) override;
    void setPortName(const std::string& portName) override;
    unsigned int getPortCount(void) override;
    std::string getPortName(unsigned int portNumber) override;
    void sendMessage(const unsigned char* message, size_t size) override;

protected:
    void initialize(const std::string& clientName) override;
};

#endif

#if defined(RTMIDI_API_WEBMIDI)

class MidiInWeb : public MidiInApi
{
  std::string client_name{};
  std::string web_midi_id{};
  int open_port_number{-1};

 public:
  MidiInWeb(const std::string &/*clientName*/, unsigned int queueSizeLimit );
  ~MidiInWeb( void );
  RtMidi::Api getCurrentApi( void ) { return RtMidi::WEB_MIDI_API; };
  void openPort( unsigned int portNumber, const std::string &portName );
  void openVirtualPort( const std::string &portName );
  void closePort( void );
  void setClientName( const std::string &clientName );
  void setPortName( const std::string &portName );
  unsigned int getPortCount( void );
  std::string getPortName( unsigned int portNumber );

  void onMidiMessage( uint8_t* data, double domHishResTimeStamp );

 protected:
  void initialize( const std::string& clientName );
};

class MidiOutWeb: public MidiOutApi
{
  std::string client_name{};
  std::string web_midi_id{};
  int open_port_number{-1};

 public:
  MidiOutWeb( const std::string &clientName );
  ~MidiOutWeb( void );
  RtMidi::Api getCurrentApi( void ) { return RtMidi::WEB_MIDI_API; };
  void openPort( unsigned int portNumber, const std::string &portName );
  void openVirtualPort( const std::string &portName );
  void closePort( void );
  void setClientName( const std::string &clientName );
  void setPortName( const std::string &portName );
  unsigned int getPortCount( void );
  std::string getPortName( unsigned int portNumber );
  void sendMessage( const unsigned char *message, size_t size );

 protected:
  void initialize( const std::string& clientName );
};

#endif

#if defined(RTMIDI_API_AMIDI)

#define LOG_TAG "RtMidi"
#include <amidi/AMidi.h>
#include <dlfcn.h>
#include <android/log.h>
#include <pthread.h>
#include <atomic>
#include <string>
#include <vector>
#include <jni.h>
#include <unistd.h>
#include <android/api-level.h>
#include <chrono>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <set>

// One entry per PORT, not per device: a device with three ports contributes
// three entries, all sharing its jobject and differing in portIndex.  Android
// reports the counts per device and AMidi*Port_open() takes a port index, so
// without this a multi-port device is reachable only through its first port.
struct AndroidMidiPort {
  jobject device;
  int deviceId;    // MidiDeviceInfo.getId(), new for each connection of a device
  int portIndex;
  bool isOutput;   // as enumerated: true for host->device ports
  // From the device's USB descriptors, when they could be read; otherwise -1.
  int usbInputs;
  int usbOutputs;
  // "<product> <jack name>", built from the descriptors when port names are
  // enabled and the descriptors could be read; otherwise empty.
  std::string usbName;
};

// One openPort() call.  MidiManager.openDevice() reports back on a Binder
// thread, which may happen after openPort() has given up waiting and even
// after the RtMidi object is gone, so the callback is handed this record (via
// a heap-allocated shared_ptr) rather than the object itself.
struct AndroidOpenRequest {
  std::mutex mutex;
  std::condition_variable cond;
  bool done = false;
  bool abandoned = false;     // openPort() timed out and no longer wants it
  bool isOutput = false;
  int portIndex = 0;
  AMidiDevice *device = NULL;
  AMidiInputPort *inputPort = NULL;    // host->device, for MidiOutAndroid
  AMidiOutputPort *outputPort = NULL;  // device->host, for MidiInAndroid
  jobject javaDevice = NULL;           // global ref to the Java MidiDevice
  int deviceId = 0;
  std::string failure;
};

// Long enough for a USB device that is slow to answer, short enough that a
// caller is not left wondering.  On Android 10-12 a device with fewer inputs
// than outputs never answers: the framework blocks forever opening it.
static const std::chrono::milliseconds kAndroidOpenTimeout( 5000 );

// Port names: "<product> <port name>", from the USB descriptors' jack names,
// which needs USB permission the application already holds, or from the port
// names an application's virtual device publishes.  RtMidi 6.0 and earlier named
// every port after its device, so a three-port device showed three identical
// names; RTMIDI_ANDROID_LEGACY_PORT_NAMES restores that.
#if defined(RTMIDI_ANDROID_LEGACY_PORT_NAMES)
static const bool kAndroidPortNames = false;
#else
static const bool kAndroidPortNames = true;
#endif

class MidiInAndroid : public MidiInApi
{
 public:
  MidiInAndroid(const std::string &/*clientName*/, unsigned int queueSizeLimit );
  ~MidiInAndroid( void );
  RtMidi::Api getCurrentApi( void ) { return RtMidi::ANDROID_AMIDI; };
  void openPort( unsigned int portNumber, const std::string &portName );
  void openVirtualPort( const std::string &portName );
  void closePort( void );
  void setClientName( const std::string &clientName );
  void setPortName( const std::string &portName );
  unsigned int getPortCount( void );
  std::string getPortName( unsigned int portNumber );

  void onMidiMessage( uint8_t* data, double domHishResTimeStamp );

  void initialize( const std::string& clientName );
  void connect();
  AMidiDevice* receiveDevice = NULL;
  AMidiOutputPort* midiOutputPort = NULL;
  jobject javaDevice = NULL;  // the Java MidiDevice, closed with the port
  // This object's own port list.  It was once shared by MidiInAndroid and
  // MidiOutAndroid, which rebuilt it with opposite direction filters, so each
  // overwrote the other's list: invisible when a device's input and output
  // counts match, wrong whenever they differ.
  std::vector<AndroidMidiPort> ports_;
  pthread_t readThread;
  std::atomic<bool> reading = ATOMIC_VAR_INIT(false);
  static void* pollMidi(void* context);
  void parse( const uint8_t *data, size_t count, double time );
  void deliver( std::vector<unsigned char> &bytes, double time );
  double lastTime;
  unsigned char runningStatus_ = 0;
};

class MidiOutAndroid: public MidiOutApi
{
 public:
  MidiOutAndroid( const std::string &clientName );
  ~MidiOutAndroid( void );
  RtMidi::Api getCurrentApi( void ) { return RtMidi::ANDROID_AMIDI; };
  void openPort( unsigned int portNumber, const std::string &portName );
  void openVirtualPort( const std::string &portName );
  void closePort( void );
  void setClientName( const std::string &clientName );
  void setPortName( const std::string &portName );
  unsigned int getPortCount( void );
  std::string getPortName( unsigned int portNumber );
  void sendMessage( const unsigned char *message, size_t size );

  void initialize( const std::string& clientName );
  void connect();
  AMidiDevice* sendDevice = NULL;
  AMidiInputPort* midiInputPort = NULL;
  jobject javaDevice = NULL;  // the Java MidiDevice, closed with the port
  bool sysexOpen = false; // a SysEx has started and not yet ended
  // This object's own port list.  It was once shared by MidiInAndroid and
  // MidiOutAndroid, which rebuilt it with opposite direction filters, so each
  // overwrote the other's list: invisible when a device's input and output
  // counts match, wrong whenever they differ.
  std::vector<AndroidMidiPort> ports_;
};

#endif

#if defined(RTMIDI_API_WMS)

class MidiInWinMidi2 : public MidiInApi
{
public:
    MidiInWinMidi2( const std::string& clientName, unsigned int queueSizeLimit );
    ~MidiInWinMidi2( void ) override;
    RtMidi::Api getCurrentApi( void ) override { return RtMidi::WINDOWS_MIDI_SERVICES; }
    void openPort( unsigned int portNumber, const std::string& portName ) override;
    void openVirtualPort( const std::string& portName ) override;
    void closePort( void ) override;
    void setClientName( const std::string& clientName ) override;
    void setPortName( const std::string& portName ) override;
    unsigned int getPortCount( void ) override;
    std::string getPortName( unsigned int portNumber ) override;
    double getMessage( std::vector<unsigned char>* message ) override;
protected:
    void initialize( const std::string& clientName ) override;
};

class MidiOutWinMidi2 : public MidiOutApi
{
public:
    MidiOutWinMidi2( const std::string& clientName );
    ~MidiOutWinMidi2( void ) override;
    RtMidi::Api getCurrentApi( void ) override { return RtMidi::WINDOWS_MIDI_SERVICES; }
    void openPort( unsigned int portNumber, const std::string& portName ) override;
    void openVirtualPort( const std::string& portName ) override;
    void closePort( void ) override;
    void setClientName( const std::string& clientName ) override;
    void setPortName( const std::string& portName ) override;
    unsigned int getPortCount( void ) override;
    std::string getPortName( unsigned int portNumber ) override;
    void sendMessage( const unsigned char* message, size_t size ) override;
protected:
    void initialize( const std::string& clientName ) override;
};

#endif  // RTMIDI_API_WMS

#if defined(RTMIDI_API_DUMMY)

class MidiInDummy: public MidiInApi
{
 public:
 MidiInDummy( const std::string &/*clientName*/, unsigned int queueSizeLimit ) : MidiInApi( queueSizeLimit ) { errorString_ = "MidiInDummy: This class provides no functionality."; error( RtMidiError::WARNING, errorString_ ); }
  RtMidi::Api getCurrentApi( void ) { return RtMidi::RTMIDI_DUMMY; }
  void openPort( unsigned int /*portNumber*/, const std::string &/*portName*/ ) {}
  void openVirtualPort( const std::string &/*portName*/ ) {}
  void closePort( void ) {}
  void setClientName( const std::string &/*clientName*/ ) {};
  void setPortName( const std::string &/*portName*/ ) {};
  unsigned int getPortCount( void ) { return 0; }
  std::string getPortName( unsigned int /*portNumber*/ ) { return ""; }

 protected:
  void initialize( const std::string& /*clientName*/ ) {}
};

class MidiOutDummy: public MidiOutApi
{
 public:
  MidiOutDummy( const std::string &/*clientName*/ ) { errorString_ = "MidiOutDummy: This class provides no functionality."; error( RtMidiError::WARNING, errorString_ ); }
  RtMidi::Api getCurrentApi( void ) { return RtMidi::RTMIDI_DUMMY; }
  void openPort( unsigned int /*portNumber*/, const std::string &/*portName*/ ) {}
  void openVirtualPort( const std::string &/*portName*/ ) {}
  void closePort( void ) {}
  void setClientName( const std::string &/*clientName*/ ) {};
  void setPortName( const std::string &/*portName*/ ) {};
  unsigned int getPortCount( void ) { return 0; }
  std::string getPortName( unsigned int /*portNumber*/ ) { return ""; }
  void sendMessage( const unsigned char * /*message*/, size_t /*size*/ ) {}

 protected:
  void initialize( const std::string& /*clientName*/ ) {}
};

#endif

//*********************************************************************//
//  RtMidi Definitions
//*********************************************************************//

RtMidi :: RtMidi()
  : rtapi_(0)
{
}

RtMidi :: ~RtMidi()
{
  delete rtapi_;
  rtapi_ = 0;
}

RtMidi::RtMidi(RtMidi&& other) noexcept {
    rtapi_ = other.rtapi_;
    other.rtapi_ = nullptr;
}

std::string RtMidi :: getVersion( void ) throw()
{
  return std::string( RTMIDI_VERSION );
}

// Define API names and display names.
// Must be in same order as API enum.
extern "C" {
const char* rtmidi_api_names[][2] = {
  { "unspecified" , "Unknown" },
  { "core"        , "CoreMidi" },
  { "alsa"        , "ALSA" },
  { "jack"        , "Jack" },
  { "winmm"       , "Windows MultiMedia" },
  { "dummy"       , "Dummy" },
  { "web"         , "Web MIDI API" },
  { "winuwp"      , "Windows UWP" },
  { "amidi"       , "Android MIDI API" },
  { "winmidi2"    , "Windows MIDI Services" },
};
const unsigned int rtmidi_num_api_names =
  sizeof(rtmidi_api_names)/sizeof(rtmidi_api_names[0]);

// The order here will control the order of RtMidi's API search in
// the constructor.
extern "C" const RtMidi::Api rtmidi_compiled_apis[] = {
#if defined(RTMIDI_API_CORE)
  RtMidi::MACOSX_CORE,
#endif
#if defined(RTMIDI_API_ALSA)
  RtMidi::LINUX_ALSA,
#endif
#if defined(RTMIDI_API_JACK)
  RtMidi::UNIX_JACK,
#endif
// WMS is listed before WinMM so that UNSPECIFIED auto-detection picks WMS
// when the SDK is installed.  If WMS init fails (SDK not present), its
// getPortCount() returns 0 and the search continues to WinMM.
#if defined(RTMIDI_API_WMS)
  RtMidi::WINDOWS_MIDI_SERVICES,
#endif
#if defined(RTMIDI_API_WINMM)
  RtMidi::WINDOWS_MM,
#endif
#if defined(RTMIDI_API_UWP)
  RtMidi::WINDOWS_UWP,
#endif
#if defined(RTMIDI_API_WEBMIDI)
  RtMidi::WEB_MIDI_API,
#endif
#if defined(RTMIDI_API_AMIDI)
  RtMidi::ANDROID_AMIDI,
#endif
#if defined(RTMIDI_API_DUMMY)
  RtMidi::RTMIDI_DUMMY,
#endif
  RtMidi::UNSPECIFIED,
};
extern "C" const unsigned int rtmidi_num_compiled_apis =
  sizeof(rtmidi_compiled_apis)/sizeof(rtmidi_compiled_apis[0])-1;
}

// This is a compile-time check that rtmidi_num_api_names == RtMidi::NUM_APIS.
// If the build breaks here, check that they match.
template<bool b> class StaticAssert { private: StaticAssert() {} };
template<> class StaticAssert<true>{ public: StaticAssert() {} };
class StaticAssertions { StaticAssertions() {
  StaticAssert<rtmidi_num_api_names == RtMidi::NUM_APIS>();
}};

void RtMidi :: getCompiledApi( std::vector<RtMidi::Api> &apis ) throw()
{
  apis = std::vector<RtMidi::Api>(rtmidi_compiled_apis,
                                  rtmidi_compiled_apis + rtmidi_num_compiled_apis);
}

std::string RtMidi :: getApiName( RtMidi::Api api )
{
  if (api < RtMidi::UNSPECIFIED || api >= RtMidi::NUM_APIS)
    return "";
  return rtmidi_api_names[api][0];
}

std::string RtMidi :: getApiDisplayName( RtMidi::Api api )
{
  if (api < RtMidi::UNSPECIFIED || api >= RtMidi::NUM_APIS)
    return "Unknown";
  return rtmidi_api_names[api][1];
}

RtMidi::Api RtMidi :: getCompiledApiByName( const std::string &name )
{
  unsigned int i=0;
  for (i = 0; i < rtmidi_num_compiled_apis; ++i)
    if (name == rtmidi_api_names[rtmidi_compiled_apis[i]][0])
      return rtmidi_compiled_apis[i];
  return RtMidi::UNSPECIFIED;
}

void RtMidi :: setClientName( const std::string &clientName )
{
  rtapi_->setClientName( clientName );
}

void RtMidi :: setPortName( const std::string &portName )
{
  rtapi_->setPortName( portName );
}


//*********************************************************************//
//  RtMidiIn Definitions
//*********************************************************************//

void RtMidiIn :: openMidiApi( RtMidi::Api api, const std::string &clientName, unsigned int queueSizeLimit )
{
  delete rtapi_;
  rtapi_ = 0;

#if defined(RTMIDI_API_JACK)
  if ( api == UNIX_JACK )
    rtapi_ = new MidiInJack( clientName, queueSizeLimit );
#endif
#if defined(RTMIDI_API_ALSA)
  if ( api == LINUX_ALSA )
    rtapi_ = new MidiInAlsa( clientName, queueSizeLimit );
#endif
#if defined(RTMIDI_API_WINMM)
  if ( api == WINDOWS_MM )
    rtapi_ = new MidiInWinMM( clientName, queueSizeLimit );
#endif
#if defined(RTMIDI_API_UWP)
  if (api == WINDOWS_UWP)
      rtapi_ = new MidiInWinUWP(clientName, queueSizeLimit);
#endif
#if defined(RTMIDI_API_WMS)
  if (api == WINDOWS_MIDI_SERVICES)
      rtapi_ = new MidiInWinMidi2(clientName, queueSizeLimit);
#endif
#if defined(RTMIDI_API_CORE)
  if ( api == MACOSX_CORE )
    rtapi_ = new MidiInCore( clientName, queueSizeLimit );
#endif
#if defined(RTMIDI_API_WEBMIDI)
    if ( api == WEB_MIDI_API )
    rtapi_ = new MidiInWeb( clientName, queueSizeLimit );
#endif
#if defined(RTMIDI_API_AMIDI)
    if ( api == ANDROID_AMIDI )
    rtapi_ = new MidiInAndroid( clientName, queueSizeLimit );
#endif
#if defined(RTMIDI_API_DUMMY)
  if ( api == RTMIDI_DUMMY )
    rtapi_ = new MidiInDummy( clientName, queueSizeLimit );
#endif
}

RTMIDI_DLL_PUBLIC RtMidiIn :: RtMidiIn( RtMidi::Api api, const std::string &clientName, unsigned int queueSizeLimit )
  : RtMidi()
{
  if ( api != UNSPECIFIED ) {
    // Attempt to open the specified API.
    openMidiApi( api, clientName, queueSizeLimit );
    if ( rtapi_ ) return;

    // No compiled support for specified API value.  Issue a warning
    // and continue as if no API was specified.
    std::cerr << "\nRtMidiIn: no compiled support for specified API argument!\n\n" << std::endl;
  }

  // Iterate through the compiled APIs and return as soon as we find one with
  // at least one port.  If none of them has a port right now -- a machine with
  // nothing plugged in yet -- fall back to the first API that opened, so that
  // devices connected later are seen.
  std::vector< RtMidi::Api > apis;
  getCompiledApi( apis );

  // The dummy backend reports no ports and never will, so selecting it here
  // would leave the object permanently deaf (#398).  Drop it from the
  // candidates unless it is the only thing compiled in, which also keeps it
  // from being constructed and announcing itself on a machine with no ports.
  if ( apis.size() > 1 )
    apis.erase( std::remove( apis.begin(), apis.end(), RTMIDI_DUMMY ), apis.end() );
  // Likewise skip an API this system can't run (Windows MIDI Services on older
  // Windows), so a machine with no ports yet doesn't end up on it.
  std::vector< RtMidi::Api > usable;
  for ( unsigned int i=0; i<apis.size(); i++ )
    if ( checkApiAvailability( apis[i] ).available ) usable.push_back( apis[i] );
  if ( !usable.empty() ) apis.swap( usable );

  // Keep the *first* API that opened, not the last: the compiled order is a
  // preference order, so a portless machine must still end up on the most
  // preferred real backend rather than whichever one happened to be tried last.
  RtMidi::Api firstWorkingApi = RtMidi::UNSPECIFIED;
  for ( unsigned int i=0; i<apis.size(); i++ ) {
    openMidiApi( apis[i], clientName, queueSizeLimit );
    if ( rtapi_ && rtapi_->getPortCount() ) return;
    if ( rtapi_ && firstWorkingApi == RtMidi::UNSPECIFIED )
      firstWorkingApi = apis[i];
  }

  // No API had a port.  Reopen the first one that worked, unless that is
  // already the one still open from the last iteration.
  if ( firstWorkingApi != RtMidi::UNSPECIFIED ) {
    if ( rtapi_ && rtapi_->getCurrentApi() == firstWorkingApi ) return;
    openMidiApi( firstWorkingApi, clientName, queueSizeLimit );
    if ( rtapi_ ) return;
  }

  if ( rtapi_ ) return;

  // It should not be possible to get here: the dummy backend is always
  // compiled in, so the loop above always opens something. But just in
  // case something weird happens, we'll throw an error.
  std::string errorText = "RtMidiIn: no compiled API support found ... critical error!!";
  throw( RtMidiError( errorText, RtMidiError::UNSPECIFIED ) );
}

RtMidiIn :: ~RtMidiIn() throw()
{
}


//*********************************************************************//
//  RtMidiOut Definitions
//*********************************************************************//

void RtMidiOut :: openMidiApi( RtMidi::Api api, const std::string &clientName )
{
  delete rtapi_;
  rtapi_ = 0;

#if defined(RTMIDI_API_JACK)
  if ( api == UNIX_JACK )
    rtapi_ = new MidiOutJack( clientName );
#endif
#if defined(RTMIDI_API_ALSA)
  if ( api == LINUX_ALSA )
    rtapi_ = new MidiOutAlsa( clientName );
#endif
#if defined(RTMIDI_API_WINMM)
  if ( api == WINDOWS_MM )
    rtapi_ = new MidiOutWinMM( clientName );
#endif
#if defined(RTMIDI_API_UWP)
  if (api == WINDOWS_UWP)
      rtapi_ = new MidiOutWinUWP(clientName);
#endif
#if defined(RTMIDI_API_WMS)
  if (api == WINDOWS_MIDI_SERVICES)
      rtapi_ = new MidiOutWinMidi2(clientName);
#endif
#if defined(RTMIDI_API_CORE)
  if ( api == MACOSX_CORE )
    rtapi_ = new MidiOutCore( clientName );
#endif
#if defined(RTMIDI_API_WEBMIDI)
    if ( api == WEB_MIDI_API )
    rtapi_ = new MidiOutWeb( clientName );
#endif
#if defined(RTMIDI_API_AMIDI)
    if ( api == ANDROID_AMIDI )
    rtapi_ = new MidiOutAndroid( clientName );
#endif
#if defined(RTMIDI_API_DUMMY)
  if ( api == RTMIDI_DUMMY )
    rtapi_ = new MidiOutDummy( clientName );
#endif
}

RTMIDI_DLL_PUBLIC RtMidiOut :: RtMidiOut( RtMidi::Api api, const std::string &clientName)
{
  if ( api != UNSPECIFIED ) {
    // Attempt to open the specified API.
    openMidiApi( api, clientName );
    if ( rtapi_ ) return;

    // No compiled support for specified API value.  Issue a warning
    // and continue as if no API was specified.
    std::cerr << "\nRtMidiOut: no compiled support for specified API argument!\n\n" << std::endl;
  }

  // Iterate through the compiled APIs and return as soon as we find
  // one with at least one port or we reach the end of the list.
  std::vector< RtMidi::Api > apis;
  getCompiledApi( apis );

  // As in the RtMidiIn constructor above: the dummy is not a candidate unless
  // it is the only thing compiled in, and the first API that opens wins so the
  // compiled preference order is honoured on a portless machine (#398).
  if ( apis.size() > 1 )
    apis.erase( std::remove( apis.begin(), apis.end(), RTMIDI_DUMMY ), apis.end() );
  // Likewise skip an API this system can't run (Windows MIDI Services on older
  // Windows), so a machine with no ports yet doesn't end up on it.
  std::vector< RtMidi::Api > usable;
  for ( unsigned int i=0; i<apis.size(); i++ )
    if ( checkApiAvailability( apis[i] ).available ) usable.push_back( apis[i] );
  if ( !usable.empty() ) apis.swap( usable );

  RtMidi::Api firstWorkingApi = RtMidi::UNSPECIFIED;
  for ( unsigned int i=0; i<apis.size(); i++ ) {
    openMidiApi( apis[i], clientName );
    if ( rtapi_ && rtapi_->getPortCount() ) return;
    if ( rtapi_ && firstWorkingApi == RtMidi::UNSPECIFIED )
      firstWorkingApi = apis[i];
  }

  // No API had a port.  Reopen the first one that worked, unless that is
  // already the one still open from the last iteration.
  if ( firstWorkingApi != RtMidi::UNSPECIFIED ) {
    if ( rtapi_ && rtapi_->getCurrentApi() == firstWorkingApi ) return;
    openMidiApi( firstWorkingApi, clientName );
    if ( rtapi_ ) return;
  }

  if ( rtapi_ ) return;

  // It should not be possible to get here: the dummy backend is always
  // compiled in, so the loop above always opens something. But just in
  // case something weird happens, we'll thrown an error.
  std::string errorText = "RtMidiOut: no compiled API support found ... critical error!!";
  throw( RtMidiError( errorText, RtMidiError::UNSPECIFIED ) );
}

RtMidiOut :: ~RtMidiOut() throw()
{
}

//*********************************************************************//
//  Common MidiApi Definitions
//*********************************************************************//

MidiApi :: MidiApi( void )
  : apiData_( 0 ), connected_( false ), errorCallback_(0), firstErrorOccurred_(false), errorCallbackUserData_(0)
{
}

MidiApi :: ~MidiApi( void )
{
}

void MidiApi :: setErrorCallback( RtMidiErrorCallback errorCallback, void *userData = 0 )
{
    errorCallback_ = errorCallback;
    errorCallbackUserData_ = userData;
}

RtMidiErrorCallback MidiApi :: defaultErrorCallback_ = 0;
void *MidiApi :: defaultErrorCallbackUserData_ = 0;

void MidiApi :: setDefaultErrorCallback( RtMidiErrorCallback errorCallback, void *userData )
{
  defaultErrorCallback_ = errorCallback;
  defaultErrorCallbackUserData_ = userData;
}

void MidiApi :: error( RtMidiError::Type type, std::string errorString )
{
  bool usingDefault = !errorCallback_;
  RtMidiErrorCallback callback = usingDefault ? defaultErrorCallback_ : errorCallback_;
  void *userData = usingDefault ? defaultErrorCallbackUserData_ : errorCallbackUserData_;

  if ( callback ) {

    if ( firstErrorOccurred_ )
      return;

    firstErrorOccurred_ = true;
    const std::string errorMessage = errorString;

    callback( type, errorMessage, userData );
    firstErrorOccurred_ = false;

    // Without its own callback the object may be half-built, so fatal errors still throw.
    if ( !usingDefault || type == RtMidiError::WARNING || type == RtMidiError::DEBUG_WARNING )
      return;
    throw RtMidiError( errorString, type );
  }

  if ( type == RtMidiError::WARNING ) {
    std::cerr << '\n' << errorString << "\n\n";
  }
  else if ( type == RtMidiError::DEBUG_WARNING ) {
#if defined(RTMIDI_DEBUG)
    std::cerr << '\n' << errorString << "\n\n";
#endif
  }
  else {
    std::cerr << '\n' << errorString << "\n\n";
    throw RtMidiError( errorString, type );
  }
}

// The atomic indices in MidiQueue must not change its layout: MidiQueue is a
// member of RtMidiInData, which is the protected inputData_ member of the
// exported MidiInApi, so its size is part of this library's ABI.  On every
// supported platform std::atomic<unsigned int> is lock-free and matches
// unsigned int in size and alignment; assert it rather than assume it.
static_assert( sizeof( std::atomic<unsigned int> ) == sizeof( unsigned int ),
               "std::atomic<unsigned int> must match unsigned int in size" );
static_assert( alignof( std::atomic<unsigned int> ) == alignof( unsigned int ),
               "std::atomic<unsigned int> must match unsigned int in alignment" );

//*********************************************************************//
//  Common MidiInApi Definitions
//*********************************************************************//

MidiInApi :: MidiInApi( unsigned int queueSizeLimit )
  : MidiApi()
{
  // Allocate the MIDI queue.
  inputData_.queue.ringSize = queueSizeLimit;
  if ( inputData_.queue.ringSize > 0 )
    inputData_.queue.ring = new MidiMessage[ inputData_.queue.ringSize ];
}

MidiInApi :: ~MidiInApi( void )
{
  // Delete the MIDI queue.
  if ( inputData_.queue.ringSize > 0 ) delete [] inputData_.queue.ring;
}

// RtMidiInData is part of MidiInApi's layout, and so of the ABI: its atomic
// members must have the size and alignment of the plain types they replaced.
static_assert( sizeof( std::atomic<bool> ) == sizeof( bool ) &&
               alignof( std::atomic<bool> ) == alignof( bool ),
               "std::atomic<bool> would change the layout of RtMidiInData" );
static_assert( sizeof( std::atomic<void *> ) == sizeof( void * ) &&
               alignof( std::atomic<void *> ) == alignof( void * ) &&
               sizeof( std::atomic<RtMidiIn::RtMidiCallback> ) == sizeof( RtMidiIn::RtMidiCallback ) &&
               alignof( std::atomic<RtMidiIn::RtMidiCallback> ) == alignof( RtMidiIn::RtMidiCallback ),
               "std::atomic of a pointer would change the layout of RtMidiInData" );

void MidiInApi :: setCallback( RtMidiIn::RtMidiCallback callback, void *userData )
{
  if ( inputData_.usingCallback ) {
    errorString_ = "MidiInApi::setCallback: a callback function is already set!";
    error( RtMidiError::WARNING, errorString_ );
    return;
  }

  if ( !callback ) {
    errorString_ = "RtMidiIn::setCallback: callback function value is invalid!";
    error( RtMidiError::WARNING, errorString_ );
    return;
  }

  inputData_.userCallback.store( callback, std::memory_order_relaxed );
  inputData_.userData.store( userData, std::memory_order_relaxed );
  // Publishes the two stores above to the input thread (callUserCallback()).
  inputData_.usingCallback.store( true, std::memory_order_release );
}

void MidiInApi :: cancelCallback()
{
  if ( !inputData_.usingCallback ) {
    errorString_ = "RtMidiIn::cancelCallback: no callback function was set!";
    error( RtMidiError::WARNING, errorString_ );
    return;
  }

  // Only the flag: the input thread may have seen it set a moment ago and be
  // about to read userCallback and userData, which must not be null then.
  inputData_.usingCallback = false;
}

// Helpers the backends share for SysEx framing.

// Passes a message to the user callback, or queues it.
static inline void rtmidiDeliver( MidiInApi::RtMidiInData *data, MidiInApi::MidiMessage &message, const char *who )
{
  if ( !data->callUserCallback( message.timeStamp, &message.bytes ) && !data->queue.push( message ) )
    std::cerr << "\n" << who << ": message queue limit reached!!\n\n";
}

// True if ignoreTypes() drops this real-time byte.
static inline bool rtmidiIgnoredRealtime( unsigned char byte, unsigned char ignoreFlags )
{
  if ( byte == 0xF8 || byte == 0xF9 ) return ( ignoreFlags & 0x02 ) != 0;
  if ( byte == 0xFE ) return ( ignoreFlags & 0x04 ) != 0;
  return false;
}

// Real-time bytes may sit inside a SysEx without ending it. Takes those from
// bytes[from] on out of the SysEx and delivers each as its own message.
static inline void rtmidiSplitRealtime( MidiInApi::RtMidiInData *data, std::vector<unsigned char> &bytes,
                                        size_t from, double timeStamp, const char *who )
{
  size_t kept = from;
  for ( size_t i = from; i < bytes.size(); i++ ) {
    unsigned char byte = bytes[i];
    if ( byte < 0xF8 ) { bytes[kept++] = byte; continue; }
    if ( rtmidiIgnoredRealtime( byte, data->ignoreFlags ) ) continue;
    MidiInApi::MidiMessage realtime;
    realtime.bytes.assign( 1, byte );
    realtime.timeStamp = timeStamp;
    rtmidiDeliver( data, realtime, who );
  }
  bytes.resize( kept );
}

// A message that starts with this byte ends an open SysEx: a status byte other
// than F0 (a new SysEx), F7 (its end) or real-time.
static inline bool rtmidiEndsSysex( unsigned char first )
{
  return first >= 0x80 && first < 0xF7 && first != 0xF0;
}

void MidiInApi :: ignoreTypes( bool midiSysex, bool midiTime, bool midiSense )
{
  inputData_.ignoreFlags = 0;
  if ( midiSysex ) inputData_.ignoreFlags = 0x01;
  if ( midiTime ) inputData_.ignoreFlags |= 0x02;
  if ( midiSense ) inputData_.ignoreFlags |= 0x04;
}

double MidiInApi :: getMessage( std::vector<unsigned char> *message )
{
  message->clear();

  if ( inputData_.usingCallback ) {
    errorString_ = "RtMidiIn::getNextMessage: a user callback is currently set for this port.";
    error( RtMidiError::WARNING, errorString_ );
    return 0.0;
  }

  double timeStamp;
  if ( !inputData_.queue.pop( message, &timeStamp ) )
    return 0.0;

  return timeStamp;
}

void MidiInApi :: setBufferSize( unsigned int size, unsigned int count )
{
    inputData_.bufferSize = size;
    inputData_.bufferCount = count;
}

unsigned int MidiInApi::MidiQueue::size( unsigned int *__back,
                                         unsigned int *__front )
{
  // Access back/front members exactly once and make stack copies for
  // size calculation.  Acquiring here pairs with the releases in push() and
  // pop(), so a message stored before an index was published is visible to
  // whichever side reads that index.
  unsigned int _back = back.load( std::memory_order_acquire );
  unsigned int _front = front.load( std::memory_order_acquire );
  unsigned int _size;
  if ( _back >= _front )
    _size = _back - _front;
  else
    _size = ringSize - _front + _back;

  // Return copies of back/front so no new and unsynchronized accesses
  // to member variables are needed.
  if ( __back ) *__back = _back;
  if ( __front ) *__front = _front;
  return _size;
}

// As long as we haven't reached our queue size limit, push the message.
bool MidiInApi::MidiQueue::push( const MidiInApi::MidiMessage& msg )
{
  // Local stack copies of front/back
  unsigned int _back, _front, _size;

  // Get back/front indexes exactly once and calculate current size
  _size = size( &_back, &_front );

  if ( _size < ringSize-1 )
  {
    ring[_back] = msg;
    // Publish the message by releasing the new back index: the store above
    // must not be reordered after it.
    back.store( ( _back + 1 ) % ringSize, std::memory_order_release );
    return true;
  }

  return false;
}

bool MidiInApi::MidiQueue::pop( std::vector<unsigned char> *msg, double* timeStamp )
{
  // Local stack copies of front/back
  unsigned int _back, _front, _size;

  // Get back/front indexes exactly once and calculate current size
  _size = size( &_back, &_front );

  if ( _size == 0 )
    return false;

  // Copy queued message to the vector pointer argument and then "pop" it.
  msg->assign( ring[_front].bytes.begin(), ring[_front].bytes.end() );
  *timeStamp = ring[_front].timeStamp;

  // Release the slot back to the producer only after the copies above have
  // been made.
  front.store( ( _front + 1 ) % ringSize, std::memory_order_release );
  return true;
}

//*********************************************************************//
//  Common MidiOutApi Definitions
//*********************************************************************//

MidiOutApi :: MidiOutApi( void )
  : MidiApi()
{
}

MidiOutApi :: ~MidiOutApi( void )
{
}

// *************************************************** //
//
// OS/API-specific methods.
//
// *************************************************** //

#if defined(RTMIDI_API_CORE)

// The CoreMIDI API is based on the use of a callback function for
// MIDI input.  We convert the system specific time stamps to delta
// time values.

// These are not available on iOS.
#if (TARGET_OS_IPHONE == 0)
  #include <CoreAudio/HostTime.h>
  #include <CoreServices/CoreServices.h>
#endif

// A structure to hold variables related to the CoreMIDI API
// implementation.
struct CoreMidiData {
  MIDIClientRef client;
  MIDIPortRef port;
  MIDIEndpointRef endpoint;
  MIDIEndpointRef destinationId;
  unsigned long long lastTime;
  MIDISysexSendRequest sysexreq;
  bool sysexOpen = false; // output: a SysEx has started and not yet ended
};

static MIDIClientRef CoreMidiClientSingleton = 0;

void RtMidi_setCoreMidiClientSingleton(MIDIClientRef client){
  CoreMidiClientSingleton = client;
}

void RtMidi_disposeCoreMidiClientSingleton(){
  if (CoreMidiClientSingleton == 0){
    return;
  }
  MIDIClientDispose( CoreMidiClientSingleton );
  CoreMidiClientSingleton = 0;
}

//*********************************************************************//
//  API: OS-X
//  Class Definitions: MidiInCore
//*********************************************************************//

static void midiInputCallback( const MIDIPacketList *list, void *procRef, void */*srcRef*/ )
{
  MidiInApi::RtMidiInData *data = static_cast<MidiInApi::RtMidiInData *> (procRef);
  CoreMidiData *apiData = static_cast<CoreMidiData *> (data->apiData);

  unsigned char status;
  unsigned short nBytes, iByte, size;
  unsigned long long time;

  bool& continueSysex = data->continueSysex;
  MidiInApi::MidiMessage& message = data->message;

  const MIDIPacket *packet = &list->packet[0];
  for ( unsigned int i=0; i<list->numPackets; ++i ) {

    // My interpretation of the CoreMIDI documentation: all message
    // types, except sysex, are complete within a packet and there may
    // be several of them in a single packet.  Sysex messages can be
    // broken across multiple packets and PacketLists but are bundled
    // alone within each packet (these packets do not contain other
    // message types).  If sysex messages are split across multiple
    // MIDIPacketLists, they must be handled by multiple calls to this
    // function.

    nBytes = packet->length;
    if ( nBytes == 0 ) {
      packet = MIDIPacketNext( packet );
      continue;
    }

    // A status byte (not F7 or real-time) here means the SysEx lost its end: drop it.
    if ( continueSysex && packet->data[0] >= 0x80 && packet->data[0] < 0xF8 && packet->data[0] != 0xF7 ) {
      continueSysex = false;
      message.bytes.clear();
    }

    // Calculate time stamp.
    if ( data->firstMessage ) {
      message.timeStamp = 0.0;
      data->firstMessage = false;
    }
    else {
      time = packet->timeStamp;
      if ( time == 0 ) { // this happens when receiving asynchronous sysex messages
        time = AudioGetCurrentHostTime();
      }
      time -= apiData->lastTime;
      time = AudioConvertHostTimeToNanos( time );
      if ( !continueSysex )
        message.timeStamp = time * 0.000000001;
    }

    // Track whether any non-filtered messages were found in this
    // packet for timestamp calculation
    bool foundNonFiltered = false;

    iByte = 0;
    if ( continueSysex ) {
      // We have a continuing, segmented sysex message.
      if ( !( data->ignoreFlags & 0x01 ) ) {
        // If we're not ignoring sysex messages, copy the entire packet, then
        // split out any real-time bytes mixed into it (#247): they may sit
        // inside a SysEx without ending it, and are delivered on their own.
        size_t from = message.bytes.size();
        for ( unsigned int j=0; j<nBytes; ++j )
          message.bytes.push_back( packet->data[j] );
        rtmidiSplitRealtime( data, message.bytes, from, message.timeStamp, "MidiInCore" );
      }
      continueSysex = packet->data[nBytes-1] != 0xF7;

      if ( !( data->ignoreFlags & 0x01 ) && !continueSysex ) {
        // If not a continuing sysex message, invoke the user callback function or queue the message.
        rtmidiDeliver( data, message, "MidiInCore" );
        message.bytes.clear();
      }
    }
    else {
      while ( iByte < nBytes ) {
        size = 0;
        // We are expecting that the next byte in the packet is a status byte.
        status = packet->data[iByte];
        if ( !(status & 0x80) ) break;
        // Determine the number of bytes in the MIDI message.
        if ( status < 0xC0 ) size = 3;
        else if ( status < 0xE0 ) size = 2;
        else if ( status < 0xF0 ) size = 3;
        else if ( status == 0xF0 ) {
          // A MIDI sysex
          if ( data->ignoreFlags & 0x01 ) {
            size = 0;
            iByte = nBytes;
          }
          else size = nBytes - iByte;
          continueSysex = packet->data[nBytes-1] != 0xF7;
        }
        else if ( status == 0xF1 ) {
          // A MIDI time code message
          if ( data->ignoreFlags & 0x02 ) {
            size = 0;
            iByte += 2;
          }
          else size = 2;
        }
        else if ( status == 0xF2 ) size = 3;
        else if ( status == 0xF3 ) size = 2;
        else if ( status == 0xF8 && ( data->ignoreFlags & 0x02 ) ) {
          // A MIDI timing tick message and we're ignoring it.
          size = 0;
          iByte += 1;
        }
        else if ( status == 0xFE && ( data->ignoreFlags & 0x04 ) ) {
          // A MIDI active sensing message and we're ignoring it.
          size = 0;
          iByte += 1;
        }
        else size = 1;

        // Copy the MIDI data to our vector.
        if ( size ) {
          foundNonFiltered = true;
          message.bytes.assign( &packet->data[iByte], &packet->data[iByte+size] );
          // A fresh SysEx may have real-time bytes mixed into the same
          // packet (#247): split them out, delivered on their own, before
          // deciding whether the rest is complete.
          if ( status == 0xF0 )
            rtmidiSplitRealtime( data, message.bytes, 0, message.timeStamp, "MidiInCore" );
          if ( !continueSysex ) {
            // If not a continuing sysex message, invoke the user callback function or queue the message.
            rtmidiDeliver( data, message, "MidiInCore" );
            message.bytes.clear();
            // All subsequent messages within same MIDI packet will have time delta 0
            message.timeStamp = 0.0;
          }
          iByte += size;
        }
      }
    }

    // Save the time of the last non-filtered message
    if ( foundNonFiltered ) {
      apiData->lastTime = packet->timeStamp;
      if ( apiData->lastTime == 0 ) { // this happens when receiving asynchronous sysex messages
        apiData->lastTime = AudioGetCurrentHostTime();
      }
    }

    packet = MIDIPacketNext(packet);
  }
}

MidiInCore :: MidiInCore( const std::string &clientName, unsigned int queueSizeLimit )
  : MidiInApi( queueSizeLimit )
{
  MidiInCore::initialize( clientName );
}

MidiInCore :: ~MidiInCore( void )
{
  // Close a connection if it exists.
  MidiInCore::closePort();

  // Cleanup.
  CoreMidiData *data = static_cast<CoreMidiData *> (apiData_);
  if ( data->endpoint ) MIDIEndpointDispose( data->endpoint );
  delete data;
}

MIDIClientRef MidiInCore::getCoreMidiClientSingleton(const std::string& clientName) throw() {

  if (CoreMidiClientSingleton == 0){
      // Set up our client.
      MIDIClientRef client;

      CFStringRef name = CFStringCreateWithCString( NULL, clientName.c_str(), kCFStringEncodingASCII );
      OSStatus result = MIDIClientCreate(name, NULL, NULL, &client );
      if ( result != noErr ) {
        std::ostringstream ost;
        ost << "MidiInCore::initialize: error creating OS-X MIDI client object (" << result << ").";
        errorString_ = ost.str();
        error( RtMidiError::DRIVER_ERROR, errorString_ );
        return 0;
      }
      CFRelease( name );

      CoreMidiClientSingleton = client;
  }

  return CoreMidiClientSingleton;
}

void MidiInCore :: initialize( const std::string& clientName )
{
  // Set up our client.
  MIDIClientRef client = getCoreMidiClientSingleton(clientName);

  // Save our api-specific connection information.
  CoreMidiData *data = (CoreMidiData *) new CoreMidiData;
  data->client = client;
  data->endpoint = 0;
  apiData_ = (void *) data;
  inputData_.apiData = (void *) data;
}

void MidiInCore :: openPort( unsigned int portNumber, const std::string &portName )
{
  if ( connected_ ) {
    errorString_ = "MidiInCore::openPort: a valid connection already exists!";
    error( RtMidiError::WARNING, errorString_ );
    return;
  }

  CFRunLoopRunInMode( kCFRunLoopDefaultMode, 0, false );
  auto nSrc = MIDIGetNumberOfSources();
  if ( nSrc < 1 ) {
    errorString_ = "MidiInCore::openPort: no MIDI input sources found!";
    error( RtMidiError::NO_DEVICES_FOUND, errorString_ );
    return;
  }

  if ( portNumber >= nSrc ) {
    std::ostringstream ost;
    ost << "MidiInCore::openPort: the 'portNumber' argument (" << portNumber << ") is invalid.";
    errorString_ = ost.str();
    error( RtMidiError::INVALID_PARAMETER, errorString_ );
    return;
  }

  MIDIPortRef port;
  CoreMidiData *data = static_cast<CoreMidiData *> (apiData_);
  CFStringRef portNameRef = CFStringCreateWithCString( NULL, portName.c_str(), kCFStringEncodingASCII );
  OSStatus result = MIDIInputPortCreate( data->client,
                                         portNameRef,
                                         midiInputCallback, (void *)&inputData_, &port );
  CFRelease( portNameRef );

  if ( result != noErr ) {
    errorString_ = "MidiInCore::openPort: error creating OS-X MIDI input port.";
    error( RtMidiError::DRIVER_ERROR, errorString_ );
    return;
  }

  // Get the desired input source identifier.
  MIDIEndpointRef endpoint = MIDIGetSource( portNumber );
  if ( endpoint == 0 ) {
    MIDIPortDispose( port );
    errorString_ = "MidiInCore::openPort: error getting MIDI input source reference.";
    error( RtMidiError::DRIVER_ERROR, errorString_ );
    return;
  }

  // Make the connection.
  result = MIDIPortConnectSource( port, endpoint, NULL );
  if ( result != noErr ) {
    MIDIPortDispose( port );
    errorString_ = "MidiInCore::openPort: error connecting OS-X MIDI input port.";
    error( RtMidiError::DRIVER_ERROR, errorString_ );
    return;
  }

  // Save our api-specific port information.
  data->port = port;

  connected_ = true;
}

void MidiInCore :: openVirtualPort( const std::string &portName )
{
  CoreMidiData *data = static_cast<CoreMidiData *> (apiData_);

  // Create a virtual MIDI input destination.
  MIDIEndpointRef endpoint;
  CFStringRef portNameRef = CFStringCreateWithCString( NULL, portName.c_str(), kCFStringEncodingASCII );
  OSStatus result = MIDIDestinationCreate( data->client,
                                           portNameRef,
                                           midiInputCallback, (void *)&inputData_, &endpoint );
  CFRelease( portNameRef );

  if ( result != noErr ) {
    std::ostringstream ost;
    ost << "MidiInCore::openVirtualPort: error creating virtual OS-X MIDI destination (OSStatus " << result << ").";
    errorString_ = ost.str();
    error( RtMidiError::DRIVER_ERROR, errorString_ );
    return;
  }

  // Save our api-specific connection information.
  data->endpoint = endpoint;
}

void MidiInCore :: closePort( void )
{
  CoreMidiData *data = static_cast<CoreMidiData *> (apiData_);

  if ( data->endpoint ) {
    MIDIEndpointDispose( data->endpoint );
    data->endpoint = 0;
  }

  if ( data->port ) {
    MIDIPortDispose( data->port );
    data->port = 0;
  }

  connected_ = false;
}

void MidiInCore :: setClientName ( const std::string& )
{

  errorString_ = "MidiInCore::setClientName: this function is not implemented for the MACOSX_CORE API!";
  error( RtMidiError::WARNING, errorString_ );

}

void MidiInCore :: setPortName ( const std::string& )
{

  errorString_ = "MidiInCore::setPortName: this function is not implemented for the MACOSX_CORE API!";
  error( RtMidiError::WARNING, errorString_ );

}

unsigned int MidiInCore :: getPortCount()
{
  CFRunLoopRunInMode( kCFRunLoopDefaultMode, 0, false );
  return static_cast<unsigned int> (MIDIGetNumberOfSources());
}

// This function was submitted by Douglas Casey Tucker and apparently
// derived largely from PortMidi.
CFStringRef CreateEndpointName( MIDIEndpointRef endpoint, bool isExternal )
{
  CFMutableStringRef result = CFStringCreateMutable( NULL, 0 );
  CFStringRef str;

  // Begin with the endpoint's name.
  str = NULL;
  MIDIObjectGetStringProperty( endpoint, kMIDIPropertyName, &str );
  if ( str != NULL ) {
    CFStringAppend( result, str );
    CFRelease(str);
  }

  // some MIDI devices have a leading space in endpoint name. trim
  CFStringTrim(result, CFSTR(" "));

  MIDIEntityRef entity = 0;
  MIDIEndpointGetEntity( endpoint, &entity );
  if ( entity == 0 )
    // probably virtual
    return result;

  if ( CFStringGetLength( result ) == 0 ) {
    // endpoint name has zero length -- try the entity
    str = NULL;
    MIDIObjectGetStringProperty( entity, kMIDIPropertyName, &str );
    if ( str != NULL ) {
      CFStringAppend( result, str );
    }
  }
  // now consider the device's name
  MIDIDeviceRef device = 0;
  MIDIEntityGetDevice( entity, &device );
  if ( device == 0 )
    return result;

  str = NULL;
  MIDIObjectGetStringProperty( device, kMIDIPropertyName, &str );
  if ( CFStringGetLength( result ) == 0 ) {
      CFRelease( result );
      CFRetain( str );
      return str;
  }
  if ( str != NULL ) {
    // if an external device has only one entity, throw away
    // the endpoint name and just use the device name
    if ( isExternal && MIDIDeviceGetNumberOfEntities( device ) < 2 ) {
      CFRelease( result );
      CFRetain( str );
      return str;
    } else {
      if ( CFStringGetLength( str ) == 0 ) {
        return result;
      }
      // does the entity name already start with the device name?
      // (some drivers do this though they shouldn't)
      // if so, do not prepend
      if ( CFStringCompareWithOptions( result, /* endpoint name */
                                       str /* device name */,
                                       CFRangeMake(0, CFStringGetLength( str ) ), 0 ) != kCFCompareEqualTo ) {
        // prepend the device name to the entity name
        if ( CFStringGetLength( result ) > 0 )
          CFStringInsert( result, 0, CFSTR(" ") );

        CFStringInsert( result, 0, str );
        CFRelease(str);
      }
    }
  }
  return result;
}

// This function was submitted by Douglas Casey Tucker and apparently
// derived largely from PortMidi.
static CFStringRef CreateConnectedEndpointName( MIDIEndpointRef endpoint )
{
  CFMutableStringRef result = CFStringCreateMutable( NULL, 0 );
  CFStringRef str;
  OSStatus err;
  size_t i;

  // Does the endpoint have connections?
  CFDataRef connections = NULL;
  size_t nConnected = 0;
  bool anyStrings = false;
  err = MIDIObjectGetDataProperty( endpoint, kMIDIPropertyConnectionUniqueID, &connections );
  if ( connections != NULL ) {
    // It has connections, follow them
    // Concatenate the names of all connected devices
    nConnected = CFDataGetLength( connections ) / sizeof(MIDIUniqueID);
    if ( nConnected ) {
      const SInt32 *pid = (const SInt32 *)(CFDataGetBytePtr(connections));
      for ( i=0; i<nConnected; ++i, ++pid ) {
        MIDIUniqueID id = EndianS32_BtoN( *pid );
        MIDIObjectRef connObject;
        MIDIObjectType connObjectType;
        err = MIDIObjectFindByUniqueID( id, &connObject, &connObjectType );
        if ( err == noErr ) {
          if ( connObjectType == kMIDIObjectType_ExternalSource  ||
               connObjectType == kMIDIObjectType_ExternalDestination ) {
            // Connected to an external device's endpoint (10.3 and later).
            str = CreateEndpointName( (MIDIEndpointRef)(connObject), true );
          } else {
            // Connected to an external device (10.2) (or something else, catch-
            str = NULL;
            MIDIObjectGetStringProperty( connObject, kMIDIPropertyName, &str );
            if ( str ) CFRetain ( str );
          }
          if ( str != NULL ) {
            if ( anyStrings )
              CFStringAppend( result, CFSTR(", ") );
            else
              anyStrings = true;
            CFStringAppend( result, str );
            CFRelease( str );
          }
        }
      }
    }
    CFRelease( connections );
  }
  if ( anyStrings )
    return result;

  CFRelease( result );

  // Here, either the endpoint had no connections, or we failed to obtain names
  return CreateEndpointName( endpoint, false );
}

std::string MidiInCore :: getPortName( unsigned int portNumber )
{
  CFStringRef nameRef;
  MIDIEndpointRef portRef;
  char name[128];

  std::string stringName;
  CFRunLoopRunInMode( kCFRunLoopDefaultMode, 0, false );
  if ( portNumber >= MIDIGetNumberOfSources() ) {
    std::ostringstream ost;
    ost << "MidiInCore::getPortName: the 'portNumber' argument (" << portNumber << ") is invalid.";
    errorString_ = ost.str();
    error( RtMidiError::WARNING, errorString_ );
    return stringName;
  }

  portRef = MIDIGetSource( portNumber );
  nameRef = CreateConnectedEndpointName( portRef );
  CFStringGetCString( nameRef, name, sizeof(name), kCFStringEncodingUTF8 );
  CFRelease( nameRef );

  return stringName = name;
}

//*********************************************************************//
//  API: OS-X
//  Class Definitions: MidiOutCore
//*********************************************************************//

MidiOutCore :: MidiOutCore( const std::string &clientName )
  : MidiOutApi()
{
  MidiOutCore::initialize( clientName );
}

MidiOutCore :: ~MidiOutCore( void )
{
  // Close a connection if it exists.
  MidiOutCore::closePort();

  // Cleanup.
  CoreMidiData *data = static_cast<CoreMidiData *> (apiData_);
  if ( data->endpoint ) MIDIEndpointDispose( data->endpoint );
  delete data;
}

MIDIClientRef MidiOutCore::getCoreMidiClientSingleton(const std::string& clientName) throw() {

  if (CoreMidiClientSingleton == 0){
      // Set up our client.
      MIDIClientRef client;

      CFStringRef name = CFStringCreateWithCString( NULL, clientName.c_str(), kCFStringEncodingASCII );
      OSStatus result = MIDIClientCreate(name, NULL, NULL, &client );
      if ( result != noErr ) {
        std::ostringstream ost;
        ost << "MidiInCore::initialize: error creating OS-X MIDI client object (" << result << ").";
        errorString_ = ost.str();
        error( RtMidiError::DRIVER_ERROR, errorString_ );
        return 0;
      }
      CFRelease( name );

      CoreMidiClientSingleton = client;
  }

  return CoreMidiClientSingleton;
}

void MidiOutCore :: initialize( const std::string& clientName )
{
  // Set up our client.
  MIDIClientRef client = getCoreMidiClientSingleton(clientName);

  // Save our api-specific connection information.
  CoreMidiData *data = (CoreMidiData *) new CoreMidiData;
  data->client = client;
  data->endpoint = 0;
  apiData_ = (void *) data;
}

unsigned int MidiOutCore :: getPortCount()
{
  CFRunLoopRunInMode( kCFRunLoopDefaultMode, 0, false );
  return static_cast<unsigned int> (MIDIGetNumberOfDestinations());
}

std::string MidiOutCore :: getPortName( unsigned int portNumber )
{
  CFStringRef nameRef;
  MIDIEndpointRef portRef;
  char name[128];

  std::string stringName;
  CFRunLoopRunInMode( kCFRunLoopDefaultMode, 0, false );
  if ( portNumber >= MIDIGetNumberOfDestinations() ) {
    std::ostringstream ost;
    ost << "MidiOutCore::getPortName: the 'portNumber' argument (" << portNumber << ") is invalid.";
    errorString_ = ost.str();
    error( RtMidiError::WARNING, errorString_ );
    return stringName;
  }

  portRef = MIDIGetDestination( portNumber );
  nameRef = CreateConnectedEndpointName(portRef);
  CFStringGetCString( nameRef, name, sizeof(name), kCFStringEncodingUTF8 );
  CFRelease( nameRef );

  return stringName = name;
}

void MidiOutCore :: openPort( unsigned int portNumber, const std::string &portName )
{
  if ( connected_ ) {
    errorString_ = "MidiOutCore::openPort: a valid connection already exists!";
    error( RtMidiError::WARNING, errorString_ );
    return;
  }

  CFRunLoopRunInMode( kCFRunLoopDefaultMode, 0, false );
  auto nDest = MIDIGetNumberOfDestinations();
  if (nDest < 1) {
    errorString_ = "MidiOutCore::openPort: no MIDI output destinations found!";
    error( RtMidiError::NO_DEVICES_FOUND, errorString_ );
    return;
  }

  if ( portNumber >= nDest ) {
    std::ostringstream ost;
    ost << "MidiOutCore::openPort: the 'portNumber' argument (" << portNumber << ") is invalid.";
    errorString_ = ost.str();
    error( RtMidiError::INVALID_PARAMETER, errorString_ );
    return;
  }

  MIDIPortRef port;
  CoreMidiData *data = static_cast<CoreMidiData *> (apiData_);
  CFStringRef portNameRef = CFStringCreateWithCString( NULL, portName.c_str(), kCFStringEncodingASCII );
  OSStatus result = MIDIOutputPortCreate( data->client, portNameRef, &port );
  CFRelease( portNameRef );
  if ( result != noErr ) {
    errorString_ = "MidiOutCore::openPort: error creating OS-X MIDI output port.";
    error( RtMidiError::DRIVER_ERROR, errorString_ );
    return;
  }

  // Get the desired output port identifier.
  MIDIEndpointRef destination = MIDIGetDestination( portNumber );
  if ( destination == 0 ) {
    MIDIPortDispose( port );
    errorString_ = "MidiOutCore::openPort: error getting MIDI output destination reference.";
    error( RtMidiError::DRIVER_ERROR, errorString_ );
    return;
  }

  // Save our api-specific connection information.
  data->port = port;
  data->destinationId = destination;
  connected_ = true;
}

void MidiOutCore :: closePort( void )
{
  CoreMidiData *data = static_cast<CoreMidiData *> (apiData_);

  if ( data->endpoint ) {
    MIDIEndpointDispose( data->endpoint );
    data->endpoint = 0;
  }

  if ( data->port ) {
    MIDIPortDispose( data->port );
    data->port = 0;
  }

  data->sysexOpen = false;
  connected_ = false;
}

void MidiOutCore :: setClientName ( const std::string& )
{

  errorString_ = "MidiOutCore::setClientName: this function is not implemented for the MACOSX_CORE API!";
  error( RtMidiError::WARNING, errorString_ );

}

void MidiOutCore :: setPortName ( const std::string& )
{

  errorString_ = "MidiOutCore::setPortName: this function is not implemented for the MACOSX_CORE API!";
  error( RtMidiError::WARNING, errorString_ );

}

void MidiOutCore :: openVirtualPort( const std::string &portName )
{
  CoreMidiData *data = static_cast<CoreMidiData *> (apiData_);

  if ( data->endpoint ) {
    errorString_ = "MidiOutCore::openVirtualPort: a virtual output port already exists!";
    error( RtMidiError::WARNING, errorString_ );
    return;
  }

  // Create a virtual MIDI output source.
  MIDIEndpointRef endpoint;
  CFStringRef portNameRef = CFStringCreateWithCString( NULL, portName.c_str(), kCFStringEncodingASCII );
  OSStatus result = MIDISourceCreate( data->client, portNameRef, &endpoint );
  CFRelease( portNameRef );

  if ( result != noErr ) {
    std::ostringstream ost;
    ost << "MidiOutCore::openVirtualPort: error creating OS-X virtual MIDI source (OSStatus " << result << ").";
    errorString_ = ost.str();
    error( RtMidiError::DRIVER_ERROR, errorString_ );
    return;
  }

  // Save our api-specific connection information.
  data->endpoint = endpoint;
}

void MidiOutCore :: sendMessage( const unsigned char *message, size_t size )
{
  // We use the MIDISendSysex() function to asynchronously send sysex
  // messages.  Otherwise, we use a single CoreMidi MIDIPacket.
  unsigned int nBytes = static_cast<unsigned int> (size);
  if ( nBytes == 0 ) {
    errorString_ = "MidiOutCore::sendMessage: no data in message argument!";
    error( RtMidiError::WARNING, errorString_ );
    return;
  }

  CoreMidiData *data = static_cast<CoreMidiData *> (apiData_);

  // CoreMIDI's own MIDI1/UMP translation mis-splits a single MIDISend() call
  // whose bytes mix a real-time byte with surrounding data: it inserts an
  // unwanted synthetic F7/F0 pair and loses bytes doing so (confirmed with a
  // raw CoreMIDI probe, independent of this code). Never hand it such a
  // mix: a real-time byte that is not the entire message is sent on its own,
  // splitting the call around it, exactly as if the caller had sent it that
  // way itself, which CoreMIDI transmits faithfully.
  if ( nBytes > 1 ) {
    for ( unsigned int i = 0; i < nBytes; ++i ) {
      if ( message[i] < 0xF8 ) continue;
      if ( i > 0 ) sendMessage( message, i );
      sendMessage( &message[i], 1 );
      if ( i + 1 < nBytes ) sendMessage( message + i + 1, nBytes - i - 1 );
      return;
    }
  }

  MIDITimeStamp timeStamp = AudioGetCurrentHostTime();
  OSStatus result;

  // A message that ends an open SysEx (anything but F0, F7 or real-time)
  // closes the abandoned one first, so every receiver drops it the same
  // way; a real-time message keeps it open and goes out as itself.
  if ( data->sysexOpen && rtmidiEndsSysex( message[0] ) ) {
    static const unsigned char closeSysex[2] = { 0xF0, 0xF7 };
    sendMessage( closeSysex, 2 );
    // If the close failed (already reported), this message would land
    // inside the still-open SysEx.
    if ( data->sysexOpen ) return;
  }

  // Work out the framing state this call leaves behind, but commit it only
  // once every packet has been accepted (below). If a send fails, the state
  // stays as it was, so a caller that retries the same message gets it
  // routed the same way, and an aborted close is not forced shut.
  bool nextSysexOpen = data->sysexOpen;
  for ( unsigned int i = 0; i < nBytes; ++i ) {
    if ( message[i] == 0xF0 ) nextSysexOpen = true;
    else if ( message[i] >= 0x80 && message[i] < 0xF8 ) nextSysexOpen = false;
  }

  ByteCount bufsize = nBytes > 65535 ? 65535 : nBytes;
  std::vector<Byte> buffer( bufsize + 16 ); // pad for other struct members
  ByteCount listSize = buffer.size();
  MIDIPacketList *packetList = (MIDIPacketList*)buffer.data();

  bool accepted = true;
  ByteCount remainingBytes = nBytes;
  while ( remainingBytes ) {
    MIDIPacket *packet = MIDIPacketListInit( packetList );
    // A MIDIPacketList can only contain a maximum of 64K of data, so if our message is longer,
    // break it up into chunks of 64K or less and send out as a MIDIPacketList with only one
    // MIDIPacket. Here, we reuse the memory allocated above on the stack for all.
    ByteCount bytesForPacket = remainingBytes > 65535 ? 65535 : remainingBytes;
    const Byte* dataStartPtr = (const Byte *) &message[nBytes - remainingBytes];
    packet = MIDIPacketListAdd( packetList, listSize, packet, timeStamp, bytesForPacket, dataStartPtr );
    remainingBytes -= bytesForPacket;

    if ( !packet ) {
      errorString_ = "MidiOutCore::sendMessage: could not allocate packet list";
      error( RtMidiError::DRIVER_ERROR, errorString_ );
      return;
    }

    // Send to any destinations that may have connected to us.
    if ( data->endpoint ) {
      result = MIDIReceived( data->endpoint, packetList );
      if ( result != noErr ) {
        accepted = false;
        errorString_ = "MidiOutCore::sendMessage: error sending MIDI to virtual destinations.";
        error( RtMidiError::WARNING, errorString_ );
      }
    }

    // And send to an explicit destination port if we're connected.
    if ( connected_ ) {
      result = MIDISend( data->port, data->destinationId, packetList );
      if ( result != noErr ) {
        accepted = false;
        errorString_ = "MidiOutCore::sendMessage: error sending MIDI message to port.";
        error( RtMidiError::WARNING, errorString_ );
      }
    }
  }

  if ( accepted ) data->sysexOpen = nextSysexOpen;
}

#endif  // RTMIDI_API_CORE


//*********************************************************************//
//  API: LINUX ALSA SEQUENCER
//*********************************************************************//

// API information found at:
//   - http://www.alsa-project.org/documentation.php#Library

#if defined(RTMIDI_API_ALSA)

// The ALSA Sequencer API is based on the use of a callback function for
// MIDI input.
//
// Thanks to Pedro Lopez-Cabanillas for help with the ALSA sequencer
// time stamps and other assorted fixes!!!

// If you don't need timestamping for incoming MIDI events, define the
// preprocessor definition AVOID_TIMESTAMPING to save resources
// associated with the ALSA sequencer queues.

#include <pthread.h>
#include <sys/time.h>

// ALSA header file.
#include <alsa/asoundlib.h>

// USB identity lookup needs snd_seq_client_info_get_card() (ALSA 1.1.1).
#if SND_LIB_VERSION >= 0x010101
#include <algorithm>
#include <fstream>
#include <memory>
#include <mutex>
#include <fcntl.h>
#include <limits.h>

namespace {

class AlsaMidiMonitor;

// Per-connection state; ownership stays with the input/output backend.
class AlsaMidiReconnect
{
 public:
  static bool isSupported() { return true; }
  AlsaMidiReconnect();
  ~AlsaMidiReconnect() { stop(); }
  void prepare( snd_seq_t *seq, snd_seq_port_subscribe_t *subscription, bool input );
  int start();
  void stop();
  int setEnabled( bool enabled );
  bool isEnabled() const;

 private:
  friend class AlsaMidiMonitor;
  AlsaMidiReconnect( const AlsaMidiReconnect & );
  AlsaMidiReconnect &operator=( const AlsaMidiReconnect & );
  static std::string identity( snd_seq_t *seq, const snd_seq_addr_t &address );
  int findPort( snd_seq_t *seq, snd_seq_addr_t &match );
  void reconcile( snd_seq_t *seq );
  void reconnect( snd_seq_t *seq );

  std::shared_ptr<AlsaMidiMonitor> monitor_;
  snd_seq_port_subscribe_t *subscription_;
  snd_seq_addr_t remote_;
  std::string identity_;
  bool input_, missing_, active_, enabled_, check_;
  int lastError_;
};

// One announcement handle and worker for all ALSA input/output connections.
// The registry lock protects subscription lifetime, never MIDI reads/writes.
// Helpers retain shared ownership, including during static-object destruction.
class AlsaMidiMonitor
{
 public:
  static std::shared_ptr<AlsaMidiMonitor> instance() {
    static std::shared_ptr<AlsaMidiMonitor> monitor( new AlsaMidiMonitor );
    return monitor;
  }
  ~AlsaMidiMonitor();
  static void warning( int error, const char *operation, int &previous );

 private:
  friend class AlsaMidiReconnect;
  AlsaMidiMonitor() : seq_(0), wake_{-1, -1}, running_(false), quitting_(false), rescan_(false), retry_(false), lastError_(0) {}
  int start();
  void notify();
  int open();
  void reset( bool retry = false );
  static void *run( void *context );
  void watch();

  std::mutex control_;
  std::vector<AlsaMidiReconnect *> connections_;
  snd_seq_t *seq_;
  int wake_[2];
  pthread_t thread_;
  bool running_, quitting_, rescan_, retry_;
  int lastError_;
};

AlsaMidiReconnect :: AlsaMidiReconnect()
  : monitor_(AlsaMidiMonitor::instance()), subscription_(0), input_(false),
    missing_(false), active_(false), enabled_(true), check_(false), lastError_(0)
{
}

std::string AlsaMidiReconnect :: identity( snd_seq_t *seq, const snd_seq_addr_t &address )
{
  snd_seq_client_info_t *client;
  snd_seq_port_info_t *port;
  snd_seq_client_info_alloca( &client );
  snd_seq_port_info_alloca( &port );
  if ( snd_seq_get_any_client_info( seq, address.client, client ) < 0 ||
       snd_seq_get_any_port_info( seq, address.client, address.port, port ) < 0 ) return "";
  int card = snd_seq_client_info_get_card( client );
  if ( card < 0 ) return "";

  std::ostringstream cardPath;
  cardPath << "/sys/class/sound/card" << card << "/device";
  char resolved[PATH_MAX];
  if ( !realpath( cardPath.str().c_str(), resolved ) ) return "";
  std::string path( resolved );
  while ( !path.empty() ) {
    std::string vendor, product, serial;
    std::ifstream( (path + "/idVendor").c_str() ) >> vendor;
    std::ifstream( (path + "/idProduct").c_str() ) >> product;
    if ( !vendor.empty() && !product.empty() ) {
      std::ifstream serialFile( (path + "/serial").c_str() );
      std::getline( serialFile, serial );
      // Without a serial, identity means the same USB location, not proof of
      // the same physical unit. Never fall back to an ALSA address or name alone.
      std::ostringstream key;
      key << vendor << ':' << product << ':';
      if ( serial.empty() ) key << "path:" << path;
      else key << "serial:" << serial;
      key << ':' << (unsigned int) address.port << ':' << snd_seq_port_info_get_name( port );
      return key.str();
    }
    path.erase( path.find_last_of( '/' ) );
  }
  return "";
}


int AlsaMidiReconnect :: findPort( snd_seq_t *seq, snd_seq_addr_t &match )
{
  snd_seq_client_info_t *client;
  snd_seq_port_info_t *port;
  snd_seq_client_info_alloca( &client );
  snd_seq_port_info_alloca( &port );
  unsigned int matches = 0;
  unsigned int caps = SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE;
  if ( input_ ) caps = SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ;
  snd_seq_client_info_set_client( client, -1 );
  int clientResult, portResult;
  while ( (clientResult = snd_seq_query_next_client( seq, client )) >= 0 ) {
    snd_seq_port_info_set_client( port, snd_seq_client_info_get_client( client ) );
    snd_seq_port_info_set_port( port, -1 );
    while ( (portResult = snd_seq_query_next_port( seq, port )) >= 0 ) {
      unsigned int capability = snd_seq_port_info_get_capability( port );
      if ( (capability & caps) != caps || (capability & SND_SEQ_PORT_CAP_NO_EXPORT) != 0 ) continue;
      snd_seq_addr_t address;
      address.client = snd_seq_port_info_get_client( port );
      address.port = snd_seq_port_info_get_port( port );
      if ( identity( seq, address ) != identity_ ) continue;
      match = address;
      ++matches;
    }
    if ( portResult != -ENOENT ) {
      AlsaMidiMonitor::warning( portResult, "enumerating MIDI ports", lastError_ );
      return portResult;
    }
  }
  if ( clientResult != -ENOENT ) {
    AlsaMidiMonitor::warning( clientResult, "enumerating MIDI clients", lastError_ );
    return clientResult;
  }
  // Duplicate serials or otherwise ambiguous identities must not be guessed.
  return matches;
}


void AlsaMidiReconnect :: prepare( snd_seq_t *seq, snd_seq_port_subscribe_t *subscription, bool input )
{
  stop();
  input_ = input;
  if ( input ) remote_ = *snd_seq_port_subscribe_get_sender( subscription );
  else remote_ = *snd_seq_port_subscribe_get_dest( subscription );
  identity_ = identity( seq, remote_ );
  if ( identity_.empty() ) return;
  snd_seq_addr_t match;
  int matches = findPort( seq, match );
  if ( matches >= 0 && matches != 1 ) return;

  std::lock_guard<std::mutex> lock( monitor_->control_ );
  monitor_->connections_.push_back( this );
  subscription_ = subscription;
  missing_ = false;
  lastError_ = 0;
}

int AlsaMidiReconnect :: start()
{
  std::lock_guard<std::mutex> lock( monitor_->control_ );
  if ( !subscription_ ) return 0;
  active_ = true;
  check_ = true;
  int result = monitor_->start();
  monitor_->notify();
  return result;
}

void AlsaMidiReconnect :: stop()
{
  std::lock_guard<std::mutex> lock( monitor_->control_ );
  if ( !subscription_ ) return;
  std::vector<AlsaMidiReconnect *> &connections = monitor_->connections_;
  connections.erase( std::find( connections.begin(), connections.end(), this ) );
  // No worker can access the subscription after this lock is released.
  subscription_ = 0;
  active_ = false;
  monitor_->notify();
}

int AlsaMidiReconnect :: setEnabled( bool enabled )
{
  std::lock_guard<std::mutex> lock( monitor_->control_ );
  enabled_ = enabled;
  if ( !enabled || !active_ ) return 0;
  check_ = true;
  int result = monitor_->start();
  monitor_->notify();
  return result;
}

bool AlsaMidiReconnect :: isEnabled() const
{
  std::lock_guard<std::mutex> lock( monitor_->control_ );
  return enabled_;
}

void AlsaMidiReconnect :: reconcile( snd_seq_t *seq )
{
  snd_seq_port_subscribe_t *query;
  snd_seq_port_subscribe_alloca( &query );
  snd_seq_port_subscribe_copy( query, subscription_ );
  // After lost announcements, the numeric address alone is not trustworthy.
  if ( snd_seq_get_port_subscription( seq, query ) < 0 || identity( seq, remote_ ) != identity_ )
    missing_ = true;
  check_ = false;
}

void AlsaMidiReconnect :: reconnect( snd_seq_t *seq )
{
  snd_seq_addr_t match;
  if ( findPort( seq, match ) != 1 ) return;
  if ( input_ ) snd_seq_port_subscribe_set_sender( subscription_, &match );
  else snd_seq_port_subscribe_set_dest( subscription_, &match );
  int result = snd_seq_subscribe_port( seq, subscription_ );
  if ( result == 0 || result == -EBUSY ) {
    // EBUSY can mean another application already restored this subscription.
    snd_seq_port_subscribe_t *query;
    snd_seq_port_subscribe_alloca( &query );
    snd_seq_port_subscribe_copy( query, subscription_ );
    int status = snd_seq_get_port_subscription( seq, query );
    if ( status < 0 ) {
      AlsaMidiMonitor::warning( status, "checking restored subscription", lastError_ );
      return;
    }
    remote_ = match;
    missing_ = false;
    lastError_ = 0;
  }
  else AlsaMidiMonitor::warning( result, "restoring MIDI subscription", lastError_ );
}

void AlsaMidiMonitor :: warning( int error, const char *operation, int &previous )
{
  if ( previous == error ) return;
  // Like alsaMidiHandler(), report worker errors without application callbacks:
  // a callback could close a port or destroy RtMidi while this lock is held.
  std::cerr << "RtMidi ALSA hotplug: " << operation << ": " << snd_strerror( error ) << "; retrying.\n";
  previous = error;
}

int AlsaMidiMonitor :: start()
{
  if ( running_ ) return 0;
  int error = 0;
  // No worker exists yet to retry these failures. Retry transient exhaustion
  // here, then let the caller report persistent failure through MidiApi::error.
  for ( int attempt = 0; attempt < 3; ++attempt ) {
    // Like the ALSA input thread, wake through a pipe; never block a notifier.
    if ( pipe2( wake_, O_NONBLOCK | O_CLOEXEC ) < 0 ) error = errno;
    else {
      error = pthread_create( &thread_, 0, run, this );
      if ( error == 0 ) { running_ = true; return 0; }
      close( wake_[0] );
      close( wake_[1] );
      wake_[0] = wake_[1] = -1;
    }
    poll( 0, 0, 10 );
  }
  return -error;
}

void AlsaMidiMonitor :: notify()
{
  if ( wake_[1] < 0 ) return;
  char value = 1;
  ssize_t result;
  do { result = write( wake_[1], &value, sizeof(value) ); } while ( result < 0 && errno == EINTR );
  // EAGAIN means a wake-up is already pending.
}

AlsaMidiMonitor :: ~AlsaMidiMonitor()
{
  {
    std::lock_guard<std::mutex> lock( control_ );
    quitting_ = true;
    notify();
  }
  // Never join while holding the lock the worker needs to exit.
  if ( running_ ) pthread_join( thread_, 0 );
  if ( seq_ ) snd_seq_close( seq_ );
  if ( wake_[0] >= 0 ) close( wake_[0] );
  if ( wake_[1] >= 0 ) close( wake_[1] );
}

void AlsaMidiMonitor :: reset( bool retry )
{
  if ( seq_ ) snd_seq_close( seq_ );
  seq_ = 0;
  rescan_ = true;
  retry_ = retry;
}

int AlsaMidiMonitor :: open()
{
  int result = snd_seq_open( &seq_, "default", SND_SEQ_OPEN_INPUT, SND_SEQ_NONBLOCK );
  if ( result < 0 ) { seq_ = 0; return result; }
  result = snd_seq_set_client_name( seq_, "RtMidi hotplug" );
  if ( result >= 0 ) {
    int port = snd_seq_create_simple_port( seq_, "Announce",
      SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE | SND_SEQ_PORT_CAP_NO_EXPORT, 0 );
    result = port;
    if ( port >= 0 )
      result = snd_seq_connect_from( seq_, port, SND_SEQ_CLIENT_SYSTEM, SND_SEQ_PORT_SYSTEM_ANNOUNCE );
  }
  if ( result < 0 ) reset( true );
  else rescan_ = true;
  return result;
}

void *AlsaMidiMonitor :: run( void *context )
{
  AlsaMidiMonitor *monitor = static_cast<AlsaMidiMonitor *>(context);
  pthread_setname_np( pthread_self(), "RtMidi hotplug" );
  for (;;) {
    try { monitor->watch(); return 0; }
    catch ( const std::exception &error ) {
      std::cerr << "RtMidi ALSA hotplug: " << error.what() << "; retrying.\n";
      std::lock_guard<std::mutex> lock( monitor->control_ );
      monitor->reset( true );
    }
    poll( 0, 0, 100 );
  }
}

void AlsaMidiMonitor :: watch()
{
  for (;;) {
    struct pollfd fds[2] = {};
    fds[0].fd = wake_[0];
    fds[0].events = POLLIN;
    int count = 1, timeout = -1;
    {
      std::lock_guard<std::mutex> lock( control_ );
      if ( quitting_ ) return;
      if ( connections_.empty() ) reset();
      else if ( !seq_ && !retry_ ) {
        int result = open();
        if ( result < 0 ) warning( result, "opening announcement monitor", lastError_ );
      }
      if ( seq_ ) {
        int result = snd_seq_poll_descriptors( seq_, &fds[1], 1, POLLIN );
        if ( result != 1 ) {
          warning( -EIO, "getting announcement descriptor", lastError_ );
          reset( true );
        }
        else {
          count = 2;
          if ( rescan_ || snd_seq_event_input_pending( seq_, 0 ) > 0 ) timeout = 0;
        }
      }
      if ( !connections_.empty() && !seq_ ) timeout = 100;
      for ( AlsaMidiReconnect *connection : connections_ )
        if ( connection->active_ && (connection->check_ || (connection->missing_ && connection->enabled_)) )
          if ( timeout != 0 ) timeout = 100;
    }

    int result = poll( fds, count, timeout );
    int pollError = errno;
    std::lock_guard<std::mutex> lock( control_ );
    if ( quitting_ ) return;
    if ( result < 0 ) {
      if ( pollError != EINTR ) { warning( -pollError, "waiting for announcements", lastError_ ); reset( true ); }
      continue;
    }
    if ( fds[0].revents & POLLIN ) {
      char values[256];
      ssize_t bytes = read( wake_[0], values, sizeof(values) );
      (void) bytes;
    }
    if ( !seq_ ) { retry_ = false; continue; }
    if ( fds[1].revents & (POLLERR | POLLHUP | POLLNVAL) ) {
      warning( -EIO, "reading announcement descriptor", lastError_ );
      reset( true );
      continue;
    }
    // Bound each batch so an event storm cannot starve closePort().
    int inputResult = 0;
    for ( int batch = 0; batch < 1024; ++batch ) {
      snd_seq_event_t *event;
      inputResult = snd_seq_event_input( seq_, &event );
      if ( inputResult < 0 ) break;
      if ( event->type == SND_SEQ_EVENT_PORT_EXIT )
        for ( AlsaMidiReconnect *connection : connections_ )
          if ( event->data.addr.client == connection->remote_.client &&
               event->data.addr.port == connection->remote_.port )
            connection->missing_ = true;
      snd_seq_free_event( event );
    }
    if ( inputResult == -ENOSPC ) {
      warning( inputResult, "announcement overflow; reconciling connections", lastError_ );
      rescan_ = true;
    }
    else if ( inputResult < 0 && inputResult != -EAGAIN && inputResult != -EINTR ) {
      warning( inputResult, "reading announcements", lastError_ );
      reset( true );
      continue;
    }
    else lastError_ = 0;
    for ( AlsaMidiReconnect *connection : connections_ ) {
      if ( !connection->active_ ) continue;
      if ( rescan_ || connection->check_ ) connection->reconcile( seq_ );
      if ( connection->missing_ && connection->enabled_ ) connection->reconnect( seq_ );
    }
    rescan_ = false;
  }
}

#else

namespace {

class AlsaMidiReconnect
{
 public:
  static bool isSupported() { return false; }
  void prepare( snd_seq_t *, snd_seq_port_subscribe_t *, bool ) {}
  int start() { return 0; }
  void stop() {}
  int setEnabled( bool ) { return 0; }
  bool isEnabled() const { return false; }
};

#endif

// A structure to hold variables related to the ALSA API
// implementation.
struct AlsaMidiData {
  AlsaMidiReconnect reconnect;
  snd_seq_t *seq;
  unsigned int portNum;
  int vport;
  snd_seq_port_subscribe_t *subscription;
  snd_midi_event_t *coder;
  unsigned int bufferSize;
  unsigned int requestedBufferSize;
  unsigned char *buffer;
  // The input thread, or dummy_thread_id when there is none to join. Only the
  // thread that owns the MidiInAlsa writes it (see joinInputThread()).
  pthread_t thread;
  pthread_t dummy_thread_id;
  snd_seq_real_time_t lastTime;
  int queue_id; // an input queue is needed to get timestamped events
  int trigger_fds[2];
  // Output-side SysEx state: true once 0xF0 has been seen and no 0xF7 (or
  // other terminating status byte) has arrived yet, so a later call carrying
  // a bare continuation span can be recognized as part of the same message.
  bool sysexInProgress = false;
  MidiInApi *in = nullptr; // input: the object, so the input thread can report errors
};

} // namespace

#define PORT_TYPE( pinfo, bits ) ((snd_seq_port_info_get_capability(pinfo) & (bits)) == (bits))

// Joins the input thread, if one was started and has not been joined yet.
// The input thread must not reset data->thread itself when it exits: a reset
// between the check and the join below would make the owner join
// dummy_thread_id, i.e. the thread that created the MidiInAlsa, which hangs;
// a reset before the check leaves the thread unjoined.
static void joinInputThread( AlsaMidiData *data )
{
  if ( !pthread_equal( data->thread, data->dummy_thread_id ) ) {
    pthread_join( data->thread, NULL );
    data->thread = data->dummy_thread_id;
  }
}

//*********************************************************************//
//  API: LINUX ALSA
//  Class Definitions: MidiInAlsa
//*********************************************************************//

// The time since the last message delivered, from the event's time stamp.
static double alsaDeltaTime( AlsaMidiData *apiData, MidiInApi::RtMidiInData *data, snd_seq_event_t *ev )
{
  double time;

  // Method 1: Use the system time.
  //(void)gettimeofday(&tv, (struct timezone *)NULL);
  //time = (tv.tv_sec * 1000000) + tv.tv_usec;

  // Method 2: Use the ALSA sequencer event time data.
  // (thanks to Pedro Lopez-Cabanillas!).

  // Using method from:
  // https://www.gnu.org/software/libc/manual/html_node/Elapsed-Time.html

  // Perform the carry for the later subtraction by updating y.
  // Temp var y is timespec because computation requires signed types,
  // while snd_seq_real_time_t has unsigned types.
  snd_seq_real_time_t &x( ev->time.time );
  struct timespec y;
  y.tv_nsec = apiData->lastTime.tv_nsec;
  y.tv_sec = apiData->lastTime.tv_sec;
  if ( x.tv_nsec < y.tv_nsec ) {
      int nsec = (y.tv_nsec - (int)x.tv_nsec) / 1000000000 + 1;
      y.tv_nsec -= 1000000000 * nsec;
      y.tv_sec += nsec;
  }
  if ( x.tv_nsec - y.tv_nsec > 1000000000 ) {
      int nsec = ((int)x.tv_nsec - y.tv_nsec) / 1000000000;
      y.tv_nsec += 1000000000 * nsec;
      y.tv_sec -= nsec;
  }

  // Compute the time difference.
  time = (int)x.tv_sec - y.tv_sec + ((int)x.tv_nsec - y.tv_nsec)*1e-9;

  apiData->lastTime = ev->time.time;

  if ( data->firstMessage == true ) {
    data->firstMessage = false;
    return 0.0;
  }
  return time;
}

static void *alsaMidiHandler( void *ptr )
{
  MidiInApi::RtMidiInData *data = static_cast<MidiInApi::RtMidiInData *> (ptr);
  AlsaMidiData *apiData = static_cast<AlsaMidiData *> (data->apiData);

  long nBytes;
  bool continueSysex = false;
  bool doDecode = false;
  MidiInApi::MidiMessage message;
  int poll_fd_count;
  struct pollfd *poll_fds;

  snd_seq_event_t *ev;
  int result;
  result = snd_midi_event_new( 0, &apiData->coder );
  if ( result < 0 ) {
    data->doInput = false;
    std::cerr << "\nMidiInAlsa::alsaMidiHandler: error initializing MIDI event parser!\n\n";
    return 0;
  }
  unsigned char *buffer = (unsigned char *) malloc( apiData->bufferSize );
  if ( buffer == NULL ) {
    data->doInput = false;
    snd_midi_event_free( apiData->coder );
    apiData->coder = 0;
    std::cerr << "\nMidiInAlsa::alsaMidiHandler: error initializing buffer memory!\n\n";
    return 0;
  }
  snd_midi_event_init( apiData->coder );
  snd_midi_event_no_status( apiData->coder, 1 ); // suppress running status messages

  poll_fd_count = snd_seq_poll_descriptors_count( apiData->seq, POLLIN ) + 1;
  poll_fds = (struct pollfd*)alloca( poll_fd_count * sizeof( struct pollfd ));
  snd_seq_poll_descriptors( apiData->seq, poll_fds + 1, poll_fd_count - 1, POLLIN );
  poll_fds[0].fd = apiData->trigger_fds[0];
  poll_fds[0].events = POLLIN;

  while ( data->doInput ) {

    int pending = snd_seq_event_input_pending( apiData->seq, 1 );
    if ( pending == 0 ) {
      // No data pending
      if ( poll( poll_fds, poll_fd_count, -1) >= 0 ) {
        if ( poll_fds[0].revents & POLLIN ) {
          bool dummy;
          int res = read( poll_fds[0].fd, &dummy, sizeof(dummy) );
          (void) res;
        }
      }
      continue;
    }

    // If here, there should be data. An overrun can show up in either call,
    // since input_pending reads from the kernel too.
    result = pending == -ENOSPC ? pending : snd_seq_event_input( apiData->seq, &ev );
    if ( result == -ENOSPC ) {
      // The kernel dropped the queued input, so a SysEx being assembled lost a piece.
      continueSysex = false;
      apiData->in->error( RtMidiError::WARNING, "MidiInAlsa::alsaMidiHandler: MIDI input buffer overrun, messages were lost!" );
      continue;
    }
    else if ( result <= 0 ) {
      std::cerr << "\nMidiInAlsa::alsaMidiHandler: unknown MIDI input error!\n";
      perror("System reports");
      continue;
    }

    // This is a bit weird, but we now have to decode an ALSA MIDI
    // event (back) into MIDI bytes.  We'll ignore non-MIDI types.
    if ( !continueSysex ) message.bytes.clear();

    doDecode = false;
    switch ( ev->type ) {

    case SND_SEQ_EVENT_PORT_SUBSCRIBED:
#if defined(RTMIDI_DEBUG)
      std::cout << "MidiInAlsa::alsaMidiHandler: port connection made!\n";
#endif
      break;

    case SND_SEQ_EVENT_PORT_UNSUBSCRIBED:
      continueSysex = false;
      message.bytes.clear();
      data->firstMessage = true;
      snd_midi_event_reset_decode( apiData->coder );
#if defined(RTMIDI_DEBUG)
      std::cerr << "MidiInAlsa::alsaMidiHandler: port connection has closed!\n";
      std::cout << "sender = " << (int) ev->data.connect.sender.client << ":"
                << (int) ev->data.connect.sender.port
                << ", dest = " << (int) ev->data.connect.dest.client << ":"
                << (int) ev->data.connect.dest.port
                << std::endl;
#endif
      break;

    case SND_SEQ_EVENT_QFRAME: // MIDI time code
      if ( !( data->ignoreFlags & 0x02 ) ) doDecode = true;
      break;

    case SND_SEQ_EVENT_TICK: // 0xF9 ... MIDI timing tick
      if ( !( data->ignoreFlags & 0x02 ) ) doDecode = true;
      break;

    case SND_SEQ_EVENT_CLOCK: // 0xF8 ... MIDI timing (clock) tick
      if ( !( data->ignoreFlags & 0x02 ) ) doDecode = true;
      break;

    case SND_SEQ_EVENT_SENSING: // Active sensing
      if ( !( data->ignoreFlags & 0x04 ) ) doDecode = true;
      break;

    case SND_SEQ_EVENT_SYSEX:
      if ( (data->ignoreFlags & 0x01) ) break;
      // A piece of a SysEx whose start was lost (an overrun, or a sender that never sent
      // the F0) has nothing to join: drop it rather than deliver it as a message.
      if ( !continueSysex && ( ev->data.ext.len == 0 ||
                               static_cast<unsigned char *>( ev->data.ext.ptr )[0] != 0xF0 ) )
        break;
      if ( ev->data.ext.len > apiData->bufferSize ) {
        apiData->bufferSize = ev->data.ext.len;
        free( buffer );
        buffer = (unsigned char *) malloc( apiData->bufferSize );
        if ( buffer == NULL ) {
          data->doInput = false;
          std::cerr << "\nMidiInAlsa::alsaMidiHandler: error resizing buffer memory!\n\n";
          break;
        }
      }
      doDecode = true;
      break;

    default:
      doDecode = true;
    }

    if ( doDecode ) {

      nBytes = snd_midi_event_decode( apiData->coder, buffer, apiData->bufferSize, ev );
      if ( nBytes == 1 && buffer[0] >= 0xF8 && continueSysex ) {
        // A real-time event in the middle of a SysEx: deliver it on its own.
        MidiInApi::MidiMessage realtime;
        realtime.bytes.assign( 1, buffer[0] );
        realtime.timeStamp = alsaDeltaTime( apiData, data, ev );
        rtmidiDeliver( data, realtime, "MidiInAlsa" );
        nBytes = 0;
      }
      if ( nBytes > 0 ) {
        // The ALSA sequencer has a maximum buffer size for MIDI sysex
        // events of 256 bytes.  If a device sends sysex messages larger
        // than this, they are segmented into 256 byte chunks.  So,
        // we'll watch for this and concatenate sysex chunks into a
        // single sysex message if necessary.
        // A status byte (not F7 or real-time) here means the SysEx lost its end: drop it.
        if ( continueSysex && buffer[0] >= 0x80 && buffer[0] < 0xF8 && buffer[0] != 0xF7 )
          continueSysex = false;

        size_t from = continueSysex ? message.bytes.size() : 0;
        if ( !continueSysex )
          message.bytes.assign( buffer, &buffer[nBytes] );
        else
          message.bytes.insert( message.bytes.end(), buffer, &buffer[nBytes] );
        if ( ev->type == SND_SEQ_EVENT_SYSEX )
          for ( size_t i = from; i < message.bytes.size(); i++ )
            if ( message.bytes[i] >= 0xF8 ) {
              rtmidiSplitRealtime( data, message.bytes, from, alsaDeltaTime( apiData, data, ev ), "MidiInAlsa" );
              break;
            }

        continueSysex = ( ( ev->type == SND_SEQ_EVENT_SYSEX ) && !message.bytes.empty() && ( message.bytes.back() != 0xF7 ) );
        if ( !continueSysex ) {

          message.timeStamp = alsaDeltaTime( apiData, data, ev );
        }
        else {
#if defined(RTMIDI_DEBUG)
          std::cerr << "\nMidiInAlsa::alsaMidiHandler: event parsing error or not a MIDI event!\n\n";
#endif
        }
      }
    }

    snd_seq_free_event( ev );
    if ( message.bytes.size() == 0 || continueSysex ) continue;

    if ( !data->callUserCallback( message.timeStamp, &message.bytes ) ) {
      // As long as we haven't reached our queue size limit, push the message.
      if ( !data->queue.push( message ) )
        std::cerr << "\nMidiInAlsa: message queue limit reached!!\n\n";
    }
  }

  if ( buffer ) free( buffer );
  snd_midi_event_free( apiData->coder );
  apiData->coder = 0;
  return 0;
}

MidiInAlsa :: MidiInAlsa( const std::string &clientName, unsigned int queueSizeLimit )
  : MidiInApi( queueSizeLimit )
{
  MidiInAlsa::initialize( clientName );
}

MidiInAlsa :: ~MidiInAlsa()
{
  // Close a connection if it exists.
  MidiInAlsa::closePort();

  // Shutdown the input thread.
  AlsaMidiData *data = static_cast<AlsaMidiData *> (apiData_);
  if ( inputData_.doInput ) {
    inputData_.doInput = false;
    int res = write( data->trigger_fds[1], &inputData_.doInput, sizeof( inputData_.doInput ) );
    (void) res;
  }
  joinInputThread( data );

  // Cleanup.
  close ( data->trigger_fds[0] );
  close ( data->trigger_fds[1] );
  if ( data->vport >= 0 ) snd_seq_delete_port( data->seq, data->vport );
#ifndef AVOID_TIMESTAMPING
  snd_seq_free_queue( data->seq, data->queue_id );
#endif
  snd_seq_close( data->seq );
  delete data;
}

void MidiInAlsa :: initialize( const std::string& clientName )
{
  // Set up the ALSA sequencer client.
  snd_seq_t *seq;
  int result = snd_seq_open( &seq, "default", SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK );
  if ( result < 0 ) {
    errorString_ = "MidiInAlsa::initialize: error creating ALSA sequencer client object.";
    error( RtMidiError::DRIVER_ERROR, errorString_ );
    return;
  }

  // Set client name.
  snd_seq_set_client_name( seq, clientName.c_str() );

  // Save our api-specific connection information.
  AlsaMidiData *data = (AlsaMidiData *) new AlsaMidiData;
  data->seq = seq;
  data->portNum = -1;
  data->vport = -1;
  data->subscription = 0;
  data->dummy_thread_id = pthread_self();
  data->thread = data->dummy_thread_id;
  data->trigger_fds[0] = -1;
  data->trigger_fds[1] = -1;
  data->bufferSize = inputData_.bufferSize;
  data->in = this;
  apiData_ = (void *) data;
  inputData_.apiData = (void *) data;

  if ( pipe(data->trigger_fds) == -1 ) {
    errorString_ = "MidiInAlsa::initialize: error creating pipe objects.";
    error( RtMidiError::DRIVER_ERROR, errorString_ );
    return;
  }

  // Create the input queue
#ifndef AVOID_TIMESTAMPING
  data->queue_id = snd_seq_alloc_named_queue( seq, "RtMidi Queue" );
  // Set arbitrary tempo (mm=100) and resolution (240)
  snd_seq_queue_tempo_t *qtempo;
  snd_seq_queue_tempo_alloca( &qtempo );
  snd_seq_queue_tempo_set_tempo( qtempo, 600000 );
  snd_seq_queue_tempo_set_ppq( qtempo, 240 );
  snd_seq_set_queue_tempo( data->seq, data->queue_id, qtempo );
  snd_seq_drain_output( data->seq );
#endif
}

// This function is used to count or get the pinfo structure for a given port number.
unsigned int portInfo( snd_seq_t *seq, snd_seq_port_info_t *pinfo, unsigned int type, int portNumber )
{
  snd_seq_client_info_t *cinfo;
  int client;
  int count = 0;
  snd_seq_client_info_alloca( &cinfo );

  snd_seq_client_info_set_client( cinfo, -1 );
  while ( snd_seq_query_next_client( seq, cinfo ) >= 0 ) {
    client = snd_seq_client_info_get_client( cinfo );
    if ( client == 0 ) continue;
    // Reset query info
    snd_seq_port_info_set_client( pinfo, client );
    snd_seq_port_info_set_port( pinfo, -1 );
    while ( snd_seq_query_next_port( seq, pinfo ) >= 0 ) {
      unsigned int atyp = snd_seq_port_info_get_type( pinfo );
      if ( ( ( atyp & SND_SEQ_PORT_TYPE_MIDI_GENERIC ) == 0 ) &&
           ( ( atyp & SND_SEQ_PORT_TYPE_SYNTH ) == 0 ) &&
           ( ( atyp & SND_SEQ_PORT_TYPE_APPLICATION ) == 0 ) ) continue;

      unsigned int caps = snd_seq_port_info_get_capability( pinfo );
      if ( ( ( caps & type ) != type ) ||
           ( ( caps & SND_SEQ_PORT_CAP_NO_EXPORT ) != 0 ) ) continue;

      if ( count == portNumber ) return 1;
      ++count;
    }
  }

  // If a negative portNumber was used, return the port count.
  if ( portNumber < 0 ) return count;
  return 0;
}

unsigned int MidiInAlsa :: getPortCount()
{
  snd_seq_port_info_t *pinfo;
  snd_seq_port_info_alloca( &pinfo );

  AlsaMidiData *data = static_cast<AlsaMidiData *> (apiData_);
  return portInfo( data->seq, pinfo, SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ, -1 );
}

std::string MidiInAlsa :: getPortName( unsigned int portNumber )
{
  snd_seq_client_info_t *cinfo;
  snd_seq_port_info_t *pinfo;
  snd_seq_client_info_alloca( &cinfo );
  snd_seq_port_info_alloca( &pinfo );

  std::string stringName;
  AlsaMidiData *data = static_cast<AlsaMidiData *> (apiData_);
  if ( portInfo( data->seq, pinfo, SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ, (int) portNumber ) ) {
    int cnum = snd_seq_port_info_get_client( pinfo );
    snd_seq_get_any_client_info( data->seq, cnum, cinfo );
    std::ostringstream os;
    os << snd_seq_client_info_get_name( cinfo );
    os << ":";
    os << snd_seq_port_info_get_name( pinfo );
    os << " ";                                    // These lines added to make sure devices are listed
    os << snd_seq_port_info_get_client( pinfo );  // with full portnames added to ensure individual device names
    os << ":";
    os << snd_seq_port_info_get_port( pinfo );
    stringName = os.str();
    return stringName;
  }

  // If we get here, we didn't find a match.
  errorString_ = "MidiInAlsa::getPortName: error looking for port name!";
  error( RtMidiError::WARNING, errorString_ );
  return stringName;
}

void MidiInAlsa :: openPort( unsigned int portNumber, const std::string &portName )
{
  if ( connected_ ) {
    errorString_ = "MidiInAlsa::openPort: a valid connection already exists!";
    error( RtMidiError::WARNING, errorString_ );
    return;
  }

  unsigned int nSrc = this->getPortCount();
  if ( nSrc < 1 ) {
    errorString_ = "MidiInAlsa::openPort: no MIDI input sources found!";
    error( RtMidiError::NO_DEVICES_FOUND, errorString_ );
    return;
  }

  snd_seq_port_info_t *src_pinfo;
  snd_seq_port_info_alloca( &src_pinfo );
  AlsaMidiData *data = static_cast<AlsaMidiData *> (apiData_);
  if ( portInfo( data->seq, src_pinfo, SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ, (int) portNumber ) == 0 ) {
    std::ostringstream ost;
    ost << "MidiInAlsa::openPort: the 'portNumber' argument (" << portNumber << ") is invalid.";
    errorString_ = ost.str();
    error( RtMidiError::INVALID_PARAMETER, errorString_ );
    return;
  }

  snd_seq_addr_t sender, receiver;
  sender.client = snd_seq_port_info_get_client( src_pinfo );
  sender.port = snd_seq_port_info_get_port( src_pinfo );
  receiver.client = snd_seq_client_id( data->seq );

  snd_seq_port_info_t *pinfo;
  snd_seq_port_info_alloca( &pinfo );
  if ( data->vport < 0 ) {
    snd_seq_port_info_set_client( pinfo, 0 );
    snd_seq_port_info_set_port( pinfo, 0 );
    snd_seq_port_info_set_capability( pinfo,
                                      SND_SEQ_PORT_CAP_WRITE |
                                      SND_SEQ_PORT_CAP_SUBS_WRITE );
    snd_seq_port_info_set_type( pinfo,
                                SND_SEQ_PORT_TYPE_MIDI_GENERIC |
                                SND_SEQ_PORT_TYPE_APPLICATION );
    snd_seq_port_info_set_midi_channels(pinfo, 16);
#ifndef AVOID_TIMESTAMPING
    snd_seq_port_info_set_timestamping( pinfo, 1 );
    snd_seq_port_info_set_timestamp_real( pinfo, 1 );
    snd_seq_port_info_set_timestamp_queue( pinfo, data->queue_id );
#endif
    snd_seq_port_info_set_name( pinfo,  portName.c_str() );
    data->vport = snd_seq_create_port( data->seq, pinfo );

    if ( data->vport < 0 ) {
      errorString_ = "MidiInAlsa::openPort: ALSA error creating input port.";
      error( RtMidiError::DRIVER_ERROR, errorString_ );
      return;
    }
    data->vport = snd_seq_port_info_get_port( pinfo );
  }

  receiver.port = data->vport;

  if ( !data->subscription ) {
    // Make subscription
    if ( snd_seq_port_subscribe_malloc( &data->subscription ) < 0 ) {
      errorString_ = "MidiInAlsa::openPort: ALSA error allocation port subscription.";
      error( RtMidiError::DRIVER_ERROR, errorString_ );
      return;
    }
    snd_seq_port_subscribe_set_sender( data->subscription, &sender );
    snd_seq_port_subscribe_set_dest( data->subscription, &receiver );
    data->reconnect.prepare( data->seq, data->subscription, true );
    if ( snd_seq_subscribe_port( data->seq, data->subscription ) ) {
      data->reconnect.stop();
      snd_seq_port_subscribe_free( data->subscription );
      data->subscription = 0;
      errorString_ = "MidiInAlsa::openPort: ALSA error making port connection.";
      error( RtMidiError::DRIVER_ERROR, errorString_ );
      return;
    }
  }

  if ( inputData_.doInput == false ) {
    // Wait for old thread to stop, if still running
    joinInputThread( data );

    // Start the input queue
#ifndef AVOID_TIMESTAMPING
    snd_seq_start_queue( data->seq, data->queue_id, NULL );
    snd_seq_drain_output( data->seq );
#endif
    // Start our MIDI input thread.
    pthread_attr_t attr;
    pthread_attr_init( &attr );
    pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE );
    pthread_attr_setschedpolicy( &attr, SCHED_OTHER );

    inputData_.doInput = true;
    int err = pthread_create( &data->thread, &attr, alsaMidiHandler, &inputData_ );
    pthread_attr_destroy( &attr );
    if ( err ) {
      data->thread = data->dummy_thread_id; // undefined after a failed pthread_create()
      data->reconnect.stop();
      snd_seq_unsubscribe_port( data->seq, data->subscription );
      snd_seq_port_subscribe_free( data->subscription );
      data->subscription = 0;
      inputData_.doInput = false;
      errorString_ = "MidiInAlsa::openPort: error starting MIDI input thread!";
      error( RtMidiError::THREAD_ERROR, errorString_ );
      return;
    }
  }

  connected_ = true;
  int result = data->reconnect.start();
  if ( result < 0 ) error( RtMidiError::THREAD_ERROR, "MidiInAlsa: cannot start hotplug monitor." );
}

void MidiInAlsa :: openVirtualPort( const std::string &portName )
{
  AlsaMidiData *data = static_cast<AlsaMidiData *> (apiData_);
  if ( data->vport < 0 ) {
    snd_seq_port_info_t *pinfo;
    snd_seq_port_info_alloca( &pinfo );
    snd_seq_port_info_set_capability( pinfo,
                                      SND_SEQ_PORT_CAP_WRITE |
                                      SND_SEQ_PORT_CAP_SUBS_WRITE );
    snd_seq_port_info_set_type( pinfo,
                                SND_SEQ_PORT_TYPE_MIDI_GENERIC |
                                SND_SEQ_PORT_TYPE_APPLICATION );
    snd_seq_port_info_set_midi_channels( pinfo, 16 );
#ifndef AVOID_TIMESTAMPING
    snd_seq_port_info_set_timestamping( pinfo, 1 );
    snd_seq_port_info_set_timestamp_real( pinfo, 1 );
    snd_seq_port_info_set_timestamp_queue( pinfo, data->queue_id );
#endif
    snd_seq_port_info_set_name( pinfo, portName.c_str() );
    data->vport = snd_seq_create_port( data->seq, pinfo );

    if ( data->vport < 0 ) {
      errorString_ = "MidiInAlsa::openVirtualPort: ALSA error creating virtual port.";
      error( RtMidiError::DRIVER_ERROR, errorString_ );
      return;
    }
    data->vport = snd_seq_port_info_get_port( pinfo );
  }

  if ( inputData_.doInput == false ) {
    // Wait for old thread to stop, if still running
    joinInputThread( data );

    // Start the input queue
#ifndef AVOID_TIMESTAMPING
    snd_seq_start_queue( data->seq, data->queue_id, NULL );
    snd_seq_drain_output( data->seq );
#endif
    // Start our MIDI input thread.
    pthread_attr_t attr;
    pthread_attr_init( &attr );
    pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE );
    pthread_attr_setschedpolicy( &attr, SCHED_OTHER );

    inputData_.doInput = true;
    int err = pthread_create( &data->thread, &attr, alsaMidiHandler, &inputData_ );
    pthread_attr_destroy( &attr );
    if ( err ) {
      data->thread = data->dummy_thread_id; // undefined after a failed pthread_create()
      if ( data->subscription ) {
        snd_seq_unsubscribe_port( data->seq, data->subscription );
        snd_seq_port_subscribe_free( data->subscription );
        data->subscription = 0;
      }
      inputData_.doInput = false;
      errorString_ = "MidiInAlsa::openPort: error starting MIDI input thread!";
      error( RtMidiError::THREAD_ERROR, errorString_ );
      return;
    }
  }
}

void MidiInAlsa :: closePort( void )
{
  AlsaMidiData *data = static_cast<AlsaMidiData *> (apiData_);
  data->reconnect.stop();

  if ( connected_ ) {
    if ( data->subscription ) {
      snd_seq_unsubscribe_port( data->seq, data->subscription );
      snd_seq_port_subscribe_free( data->subscription );
      data->subscription = 0;
    }
    // Stop the input queue
#ifndef AVOID_TIMESTAMPING
    snd_seq_stop_queue( data->seq, data->queue_id, NULL );
    snd_seq_drain_output( data->seq );
#endif
    connected_ = false;
  }

  // Stop thread to avoid triggering the callback, while the port is intended to be closed
  if ( inputData_.doInput ) {
    inputData_.doInput = false;
    int res = write( data->trigger_fds[1], &inputData_.doInput, sizeof( inputData_.doInput ) );
    (void) res;
  }
  joinInputThread( data );
}

void MidiInAlsa :: setClientName( const std::string &clientName )
{

  AlsaMidiData *data = static_cast<AlsaMidiData *> ( apiData_ );
  snd_seq_set_client_name( data->seq, clientName.c_str() );

}

void MidiInAlsa :: setPortName( const std::string &portName )
{
  AlsaMidiData *data = static_cast<AlsaMidiData *> (apiData_);
  snd_seq_port_info_t *pinfo;
  snd_seq_port_info_alloca( &pinfo );
  snd_seq_get_port_info( data->seq, data->vport, pinfo );
  snd_seq_port_info_set_name( pinfo, portName.c_str() );
  snd_seq_set_port_info( data->seq, data->vport, pinfo );
}

//*********************************************************************//
//  API: LINUX ALSA
//  Class Definitions: MidiOutAlsa
//*********************************************************************//

MidiOutAlsa :: MidiOutAlsa( const std::string &clientName ) : MidiOutApi()
{
  MidiOutAlsa::initialize( clientName );
}

MidiOutAlsa :: ~MidiOutAlsa()
{
  // Close a connection if it exists.
  MidiOutAlsa::closePort();

  // Cleanup.
  AlsaMidiData *data = static_cast<AlsaMidiData *> (apiData_);
  if ( data->vport >= 0 ) snd_seq_delete_port( data->seq, data->vport );
  if ( data->coder ) snd_midi_event_free( data->coder );
  if ( data->buffer ) free( data->buffer );
  snd_seq_close( data->seq );
  delete data;
}

void MidiOutAlsa :: initialize( const std::string& clientName )
{
  // Set up the ALSA sequencer client.
  snd_seq_t *seq;
  int result1 = snd_seq_open( &seq, "default", SND_SEQ_OPEN_OUTPUT, SND_SEQ_NONBLOCK );
  if ( result1 < 0 ) {
    errorString_ = "MidiOutAlsa::initialize: error creating ALSA sequencer client object.";
    error( RtMidiError::DRIVER_ERROR, errorString_ );
    return;
  }

  // Set client name.
  snd_seq_set_client_name( seq, clientName.c_str() );

  // Save our api-specific connection information.
  AlsaMidiData *data = (AlsaMidiData *) new AlsaMidiData;
  data->seq = seq;
  data->portNum = -1;
  data->vport = -1;
  data->bufferSize = 32;
  data->coder = 0;
  data->buffer = 0;
  int result = snd_midi_event_new( data->bufferSize, &data->coder );
  if ( result < 0 ) {
    delete data;
    errorString_ = "MidiOutAlsa::initialize: error initializing MIDI event parser!\n\n";
    error( RtMidiError::DRIVER_ERROR, errorString_ );
    return;
  }
  data->buffer = (unsigned char *) malloc( data->bufferSize );
  if ( data->buffer == NULL ) {
    delete data;
    errorString_ = "MidiOutAlsa::initialize: error allocating buffer memory!\n\n";
    error( RtMidiError::MEMORY_ERROR, errorString_ );
    return;
  }
  snd_midi_event_init( data->coder );
  apiData_ = (void *) data;
}

unsigned int MidiOutAlsa :: getPortCount()
{
  snd_seq_port_info_t *pinfo;
  snd_seq_port_info_alloca( &pinfo );

  AlsaMidiData *data = static_cast<AlsaMidiData *> (apiData_);
  return portInfo( data->seq, pinfo, SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE, -1 );
}

std::string MidiOutAlsa :: getPortName( unsigned int portNumber )
{
  snd_seq_client_info_t *cinfo;
  snd_seq_port_info_t *pinfo;
  snd_seq_client_info_alloca( &cinfo );
  snd_seq_port_info_alloca( &pinfo );

  std::string stringName;
  AlsaMidiData *data = static_cast<AlsaMidiData *> (apiData_);
  if ( portInfo( data->seq, pinfo, SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE, (int) portNumber ) ) {
    int cnum = snd_seq_port_info_get_client( pinfo );
    snd_seq_get_any_client_info( data->seq, cnum, cinfo );
    std::ostringstream os;
    os << snd_seq_client_info_get_name( cinfo );
    os << ":";
    os << snd_seq_port_info_get_name( pinfo );
    os << " ";                                    // These lines added to make sure devices are listed
    os << snd_seq_port_info_get_client( pinfo );  // with full portnames added to ensure individual device names
    os << ":";
    os << snd_seq_port_info_get_port( pinfo );
    stringName = os.str();
    return stringName;
  }

  // If we get here, we didn't find a match.
  errorString_ = "MidiOutAlsa::getPortName: error looking for port name!";
  error( RtMidiError::WARNING, errorString_ );
  return stringName;
}

void MidiOutAlsa :: openPort( unsigned int portNumber, const std::string &portName )
{
  if ( connected_ ) {
    errorString_ = "MidiOutAlsa::openPort: a valid connection already exists!";
    error( RtMidiError::WARNING, errorString_ );
    return;
  }

  unsigned int nSrc = this->getPortCount();
  if ( nSrc < 1 ) {
    errorString_ = "MidiOutAlsa::openPort: no MIDI output sources found!";
    error( RtMidiError::NO_DEVICES_FOUND, errorString_ );
    return;
  }

  snd_seq_port_info_t *pinfo;
  snd_seq_port_info_alloca( &pinfo );
  AlsaMidiData *data = static_cast<AlsaMidiData *> (apiData_);
  if ( portInfo( data->seq, pinfo, SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE, (int) portNumber ) == 0 ) {
    std::ostringstream ost;
    ost << "MidiOutAlsa::openPort: the 'portNumber' argument (" << portNumber << ") is invalid.";
    errorString_ = ost.str();
    error( RtMidiError::INVALID_PARAMETER, errorString_ );
    return;
  }

  snd_seq_addr_t sender, receiver;
  receiver.client = snd_seq_port_info_get_client( pinfo );
  receiver.port = snd_seq_port_info_get_port( pinfo );
  sender.client = snd_seq_client_id( data->seq );

  if ( data->vport < 0 ) {
    data->vport = snd_seq_create_simple_port( data->seq, portName.c_str(),
                                              SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ,
                                              SND_SEQ_PORT_TYPE_MIDI_GENERIC|SND_SEQ_PORT_TYPE_APPLICATION );
    if ( data->vport < 0 ) {
      errorString_ = "MidiOutAlsa::openPort: ALSA error creating output port.";
      error( RtMidiError::DRIVER_ERROR, errorString_ );
      return;
    }
  }

  sender.port = data->vport;

  // Make subscription
  if ( snd_seq_port_subscribe_malloc( &data->subscription ) < 0 ) {
    snd_seq_port_subscribe_free( data->subscription );
    errorString_ = "MidiOutAlsa::openPort: error allocating port subscription.";
    error( RtMidiError::DRIVER_ERROR, errorString_ );
    return;
  }
  snd_seq_port_subscribe_set_sender( data->subscription, &sender );
  snd_seq_port_subscribe_set_dest( data->subscription, &receiver );
  snd_seq_port_subscribe_set_time_update( data->subscription, 1 );
  snd_seq_port_subscribe_set_time_real( data->subscription, 1 );
  data->reconnect.prepare( data->seq, data->subscription, false );
  if ( snd_seq_subscribe_port( data->seq, data->subscription ) ) {
    data->reconnect.stop();
    snd_seq_port_subscribe_free( data->subscription );
    data->subscription = 0;
    errorString_ = "MidiOutAlsa::openPort: ALSA error making port connection.";
    error( RtMidiError::DRIVER_ERROR, errorString_ );
    return;
  }

  connected_ = true;
  int result = data->reconnect.start();
  if ( result < 0 ) error( RtMidiError::THREAD_ERROR, "MidiOutAlsa: cannot start hotplug monitor." );
}

void MidiOutAlsa :: closePort( void )
{
  static_cast<AlsaMidiData *>(apiData_)->reconnect.stop();
  if ( connected_ ) {
    AlsaMidiData *data = static_cast<AlsaMidiData *> (apiData_);
    snd_seq_unsubscribe_port( data->seq, data->subscription );
    snd_seq_port_subscribe_free( data->subscription );
    data->subscription = 0;
    data->sysexInProgress = false;
    connected_ = false;
  }
}

void MidiOutAlsa :: setClientName( const std::string &clientName )
{

    AlsaMidiData *data = static_cast<AlsaMidiData *> ( apiData_ );
    snd_seq_set_client_name( data->seq, clientName.c_str() );

}

void MidiOutAlsa :: setPortName( const std::string &portName )
{
  AlsaMidiData *data = static_cast<AlsaMidiData *> (apiData_);
  snd_seq_port_info_t *pinfo;
  snd_seq_port_info_alloca( &pinfo );
  snd_seq_get_port_info( data->seq, data->vport, pinfo );
  snd_seq_port_info_set_name( pinfo, portName.c_str() );
  snd_seq_set_port_info( data->seq, data->vport, pinfo );
}

void MidiOutAlsa :: openVirtualPort( const std::string &portName )
{
  AlsaMidiData *data = static_cast<AlsaMidiData *> (apiData_);
  if ( data->vport < 0 ) {
    data->vport = snd_seq_create_simple_port( data->seq, portName.c_str(),
                                              SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ,
                                              SND_SEQ_PORT_TYPE_MIDI_GENERIC|SND_SEQ_PORT_TYPE_APPLICATION );

    if ( data->vport < 0 ) {
      errorString_ = "MidiOutAlsa::openVirtualPort: ALSA error creating virtual port.";
      error( RtMidiError::DRIVER_ERROR, errorString_ );
    }
  }
}

void MidiOutAlsa :: sendMessage( const unsigned char *message, size_t size )
{
  long result;
  AlsaMidiData *data = static_cast<AlsaMidiData *> (apiData_);
  unsigned int nBytes = static_cast<unsigned int> (size);

  // A large SysEx may be handed to sendMessage() across several calls - an
  // opening F0.. with no F7, bare continuation bytes, then a span ending in
  // F7 - so that the caller can pace the transfer and produce real
  // inter-span gaps on the wire.
  //
  // snd_midi_event_encode() only emits an event for a self-contained message
  // and returns SND_SEQ_EVENT_NONE for any such span, which the loop below
  // reports as "incomplete message!" and drops. ALSA's native transport for a
  // large SysEx is exactly a sequence of SysEx events carrying arbitrary byte
  // spans, which the receiver concatenates until F7 - the mirror of the
  // reassembly MidiInAlsa already performs - so emit those directly and
  // bypass the encoder. The other backends already accept partial SysEx this
  // way; this brings ALSA into line with them.
  if ( nBytes > 0 ) {
    const unsigned char first = message[0];
    if ( data->sysexInProgress && rtmidiEndsSysex( first ) ) {
      // Close the abandoned SysEx first, so every receiver drops it the same way.
      static const unsigned char closeSysex[2] = { 0xF0, 0xF7 };
      sendMessage( closeSysex, 2 );
      // If the close failed (already reported), this message would land inside the open SysEx.
      if ( data->sysexInProgress ) return;
    }
    // A real-time message keeps an open SysEx open, but goes out as itself.
    if ( ( data->sysexInProgress && first < 0xF8 ) || first == 0xF0 ) {
      // Work out the framing state this buffer leaves behind, but commit it
      // only once the event has been accepted (below). If the send fails, the
      // state stays as it was, so a caller that retries the same span gets it
      // routed the same way; otherwise a retried continuation would fall
      // through to the encoder and be rejected as an incomplete message.
      // Real-time bytes (0xF8-0xFF) may appear mid-SysEx without ending it;
      // F7 ends it, and so does any other non-real-time status byte.
      bool nextSysexInProgress = data->sysexInProgress;
      for ( unsigned int i = 0; i < nBytes; ++i ) {
        const unsigned char b = message[i];
        if ( b >= 0xF8 ) continue;
        if ( b == 0xF0 ) { nextSysexInProgress = true; continue; }
        if ( b == 0xF7 ) { nextSysexInProgress = false; continue; }
        if ( b >= 0x80 ) nextSysexInProgress = false;
      }

      snd_seq_event_t ev;
      snd_seq_ev_clear( &ev );
      snd_seq_ev_set_source( &ev, data->vport );
      snd_seq_ev_set_subs( &ev );
      snd_seq_ev_set_direct( &ev );
      snd_seq_ev_set_sysex( &ev, nBytes, const_cast<unsigned char *>( message ) );
      result = snd_seq_event_output( data->seq, &ev );
      if ( result < 0 ) {
        errorString_ = "MidiOutAlsa::sendMessage: error sending MIDI message to port.";
        error( RtMidiError::WARNING, errorString_ );
        return;
      }

      // The event has been accepted, so the framing state moves on.
      data->sysexInProgress = nextSysexInProgress;

      snd_seq_drain_output( data->seq );
      return;
    }
  }

  if ( nBytes > data->bufferSize ) {
    data->bufferSize = nBytes;
    result = snd_midi_event_resize_buffer( data->coder, nBytes );
    if ( result != 0 ) {
      errorString_ = "MidiOutAlsa::sendMessage: ALSA error resizing MIDI event buffer.";
      error( RtMidiError::DRIVER_ERROR, errorString_ );
      return;
    }
    free (data->buffer);
    data->buffer = (unsigned char *) malloc( data->bufferSize );
    if ( data->buffer == NULL ) {
      errorString_ = "MidiOutAlsa::initialize: error allocating buffer memory!\n\n";
      error( RtMidiError::MEMORY_ERROR, errorString_ );
      return;
    }
  }

  for ( unsigned int i=0; i<nBytes; ++i ) data->buffer[i] = message[i];

  unsigned int offset = 0;
  while (offset < nBytes) {
    snd_seq_event_t ev;
    snd_seq_ev_clear( &ev );
    snd_seq_ev_set_source( &ev, data->vport );
    snd_seq_ev_set_subs( &ev );
    snd_seq_ev_set_direct( &ev );
    result = snd_midi_event_encode( data->coder, data->buffer + offset,
                                    (long)(nBytes - offset), &ev );
    if ( result < 0 ) {
      errorString_ = "MidiOutAlsa::sendMessage: event parsing error!";
      error( RtMidiError::WARNING, errorString_ );
      return;
    }

    if ( ev.type == SND_SEQ_EVENT_NONE ) {
      errorString_ = "MidiOutAlsa::sendMessage: incomplete message!";
      error( RtMidiError::WARNING, errorString_ );
      return;
    }

    offset += result;

    // Send the event.
    result = snd_seq_event_output( data->seq, &ev );
    if ( result < 0 ) {
      errorString_ = "MidiOutAlsa::sendMessage: error sending MIDI message to port.";
      error( RtMidiError::WARNING, errorString_ );
      return;
    }
  }
  snd_seq_drain_output( data->seq );
}

#endif // RTMIDI_API_ALSA


//*********************************************************************//
//  API: Windows Multimedia Library (MM)
//*********************************************************************//

// API information deciphered from:
//  - http://msdn.microsoft.com/library/default.asp?url=/library/en-us/multimed/htm/_win32_midi_reference.asp

// Thanks to Jean-Baptiste Berruchon for the sysex code.

#if defined(RTMIDI_API_WINMM)

// The Windows MM API is based on the use of a callback function for
// MIDI input.  We convert the system specific time stamps to delta
// time values.

// Windows MM MIDI header files.
#include <windows.h>
#include <mmsystem.h>
#include <atomic>

// Convert a null-terminated wide string or ANSI-encoded string to UTF-8.
static std::string ConvertToUTF8(const TCHAR *str)
{
  std::string u8str;
  const WCHAR *wstr = L"";
#if defined( UNICODE ) || defined( _UNICODE )
  wstr = str;
#else
  // Convert from ANSI encoding to wide string
  int wlength = MultiByteToWideChar( CP_ACP, 0, str, -1, NULL, 0 );
  std::wstring wstrtemp;
  if ( wlength )
  {
    wstrtemp.assign( wlength - 1, 0 );
    MultiByteToWideChar( CP_ACP, 0, str, -1, &wstrtemp[0], wlength );
    wstr = &wstrtemp[0];
  }
#endif
  // Convert from wide string to UTF-8
  int length = WideCharToMultiByte( CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL );
  if ( length )
  {
    u8str.assign( length - 1, 0 );
    length = WideCharToMultiByte( CP_UTF8, 0, wstr, -1, &u8str[0], length, NULL, NULL );
  }
  return u8str;
}

// A structure to hold variables related to the CoreMIDI API
// implementation.
struct WinMidiData {
  HMIDIIN inHandle;    // Handle to Midi Input Device
  HMIDIOUT outHandle;  // Handle to Midi Output Device
  DWORD lastTime;
  MidiInApi::MidiMessage message;
  std::vector<LPMIDIHDR> sysexBuffer;
  // A SysEx longer than one input buffer arrives one buffer at a time; the
  // pieces collect here until the F7 arrives (#370).
  std::vector<unsigned char> sysexPartial;
  // WinMM calls midiInputCallback() on a thread it owns, because midiInOpen()
  // is called with CALLBACK_FUNCTION, and that callback requeues sysex buffers
  // with midiInAddBuffer().  Meanwhile closePort() runs on the caller's thread
  // and retires the same buffers with midiInUnprepareHeader().  This guards
  // that overlap: it is not protecting against two application threads, but
  // against the driver's own callback thread arriving during teardown.
  // [Patrice]
  CRITICAL_SECTION _mutex;
  // Input-side teardown flag: set while the sysex buffers are being retired so
  // midiInputCallback() stops requeueing them. Checked before the lock is
  // taken, because midiInReset() drives the very callbacks that want it.
  std::atomic<bool> closing;
  // Output-side SysEx framing state: true once 0xF0 has been sent and no 0xF7
  // (or other terminating status byte) has been seen yet. Lets sendMessage()
  // route a continuation span to midiOutLongMsg() regardless of its length.
  bool sysexInProgress = false;
};

// How long to wait for a driver to release a buffer (or to close a port that
// still has buffers queued) before giving up. Deliberately generous: this
// guards against a driver that never lets go, it is not a tuning value.
static const DWORD kWinMMBufferReleaseTimeoutMs = 5000;

//*********************************************************************//
//  API: Windows MM
//  Class Definitions: MidiInWinMM
//*********************************************************************//

// The time since the last message delivered, in seconds; 0 for the first.
static double winmmDeltaTime( MidiInApi::RtMidiInData *data, WinMidiData *apiData, DWORD timestamp )
{
  double delta = 0.0;
  if ( data->firstMessage ) data->firstMessage = false;
  else delta = (double) ( timestamp - apiData->lastTime ) * 0.001;
  apiData->lastTime = timestamp;
  return delta;
}

static void CALLBACK midiInputCallback( HMIDIIN /*hmin*/,
                                        UINT inputStatus,
                                        DWORD_PTR instancePtr,
                                        DWORD_PTR midiMessage,
                                        DWORD timestamp )
{
  if ( inputStatus != MIM_DATA && inputStatus != MIM_LONGDATA && inputStatus != MIM_LONGERROR ) return;

  //MidiInApi::RtMidiInData *data = static_cast<MidiInApi::RtMidiInData *> (instancePtr);
  MidiInApi::RtMidiInData *data = (MidiInApi::RtMidiInData *)instancePtr;
  WinMidiData *apiData = static_cast<WinMidiData *> (data->apiData);

  if ( inputStatus == MIM_DATA ) { // Channel or system message

    // Make sure the first byte is a status byte.
    unsigned char status = (unsigned char) (midiMessage & 0x000000FF);
    if ( !(status & 0x80) ) return;

    // A status byte (not real-time) in the middle of a SysEx means the SysEx lost its end: drop it.
    if ( status < 0xF8 ) apiData->sysexPartial.clear();

    // Determine the number of bytes in the MIDI message.
    unsigned short nBytes = 1;
    if ( status < 0xC0 ) nBytes = 3;
    else if ( status < 0xE0 ) nBytes = 2;
    else if ( status < 0xF0 ) nBytes = 3;
    else if ( status == 0xF1 ) {
      if ( data->ignoreFlags & 0x02 ) return;
      else nBytes = 2;
    }
    else if ( status == 0xF2 ) nBytes = 3;
    else if ( status == 0xF3 ) nBytes = 2;
    else if ( rtmidiIgnoredRealtime( status, data->ignoreFlags ) ) {
      // A timing or active sensing message and we're ignoring it.
      return;
    }

    // Copy bytes to our MIDI message.
    unsigned char *ptr = (unsigned char *) &midiMessage;
    for ( int i=0; i<nBytes; ++i ) apiData->message.bytes.push_back( *ptr++ );
  }
  else { // Sysex message ( MIM_LONGDATA or MIM_LONGERROR )
    MIDIHDR *sysex = ( MIDIHDR *) midiMessage;
    const unsigned char *bytes = (const unsigned char *) sysex->lpData;
    const DWORD nBytes = sysex->dwBytesRecorded;
    std::vector<unsigned char> &partial = apiData->sysexPartial;
    bool complete = false;

    // WinMM returns a buffer when it fills or when the F7 arrives, so a SysEx
    // longer than one buffer comes in pieces.  Join them and deliver the whole
    // message once its F7 arrives, as the other backends do.
    if ( ( data->ignoreFlags & 0x01 ) || inputStatus == MIM_LONGERROR ) {
      partial.clear();
    }
    else if ( nBytes > 0 ) {
      // An F0 starts a new SysEx, so one still pending lost its end: drop it.
      if ( bytes[0] == 0xF0 ) partial.clear();

      // A piece with no start before it (the port opened mid-SysEx) is dropped.
      if ( bytes[0] == 0xF0 || !partial.empty() ) {
        const size_t from = partial.size();
        partial.insert( partial.end(), bytes, bytes + nBytes );

        // The Windows MIDI service hands real-time bytes over as MIM_DATA, but
        // a driver may leave them in the SysEx buffer: deliver those on their own.
        if ( std::find_if( partial.begin() + from, partial.end(),
                           []( unsigned char b ) { return b >= 0xF8; } ) != partial.end() )
          rtmidiSplitRealtime( data, partial, from, winmmDeltaTime( data, apiData, timestamp ), "MidiInWinMM" );
        complete = !partial.empty() && partial.back() == 0xF7;
      }
    }

    // The WinMM API requires that the sysex buffer be requeued after
    // input of each sysex message.  Even if we are ignoring sysex
    // messages, we still need to requeue the buffer in case the user
    // decides to not ignore sysex messages in the future.  However,
    // it seems that WinMM calls this function with an empty sysex
    // buffer when an application closes and in this case, we should
    // avoid requeueing it, else the computer suddenly reboots after
    // one or two minutes.
    //
    // Use the header WinMM handed back rather than indexing sysexBuffer: it is
    // the same object, and the vector is cleared once the port has closed. Do
    // not requeue while the port is closing, since the buffers are about to be
    // unprepared and freed.
    if ( sysex->dwBytesRecorded > 0 ) {

      if ( !apiData->closing )
      {
          EnterCriticalSection( &(apiData->_mutex) );
          MMRESULT result = midiInAddBuffer( apiData->inHandle, sysex, sizeof(MIDIHDR) );
          LeaveCriticalSection( &(apiData->_mutex) );
          if ( result != MMSYSERR_NOERROR )
              std::cerr << "\nRtMidiIn::midiInputCallback: error sending sysex to Midi device!!\n\n";
      }
    }

    if ( !complete ) return;
    apiData->message.bytes.swap( partial );
    partial.clear();
  }

  // Time stamps are taken only for a message that is delivered, so a SysEx in
  // pieces is timed by its last piece, as on ALSA.
  apiData->message.timeStamp = winmmDeltaTime( data, apiData, timestamp );
  rtmidiDeliver( data, apiData->message, "MidiInWinMM" );

  // Clear the vector for the next input message.
  apiData->message.bytes.clear();
}

// Stop input and release everything MidiInWinMM::openPort() acquired: the
// sysex buffers and the input handle. Used by closePort() and by openPort()'s
// own failure paths, so it has to cope with a partly built buffer list.
//
// Every step runs regardless of earlier failures and nothing here throws or
// returns early, so the port can never be left half torn down (see #376).
// Returns the first failure, or MMSYSERR_NOERROR.
static MMRESULT releaseWinMMInput( WinMidiData *data )
{
  // Stop the callback requeueing buffers before retiring them. The flag is
  // atomic and is deliberately set without holding the lock: midiInReset()
  // below drives the very callbacks that take it (see #372).
  data->closing = true;

  midiInReset( data->inHandle );
  midiInStop( data->inHandle );

  MMRESULT firstFailure = MMSYSERR_NOERROR;
  for ( size_t i=0; i < data->sysexBuffer.size(); ++i ) {
    if ( data->sysexBuffer[i] == NULL ) continue;
    MMRESULT result = midiInUnprepareHeader( data->inHandle, data->sysexBuffer[i], sizeof(MIDIHDR) );
    if ( result != MMSYSERR_NOERROR ) {
      // The driver may still own this header, so leak it rather than free it.
      if ( firstFailure == MMSYSERR_NOERROR ) firstFailure = result;
      data->sysexBuffer[i] = NULL;
    }
  }

  MMRESULT result = midiInClose( data->inHandle );
  if ( result != MMSYSERR_NOERROR && firstFailure == MMSYSERR_NOERROR ) firstFailure = result;
  data->inHandle = 0;

  // No callbacks arrive once the handle is closed, so the headers can go now.
  for ( size_t i=0; i < data->sysexBuffer.size(); ++i ) {
    if ( data->sysexBuffer[i] == NULL ) continue;
    delete [] data->sysexBuffer[i]->lpData;
    delete data->sysexBuffer[i];
  }
  data->sysexBuffer.clear();
  data->sysexPartial.clear();
  data->closing = false;

  return firstFailure;
}

MidiInWinMM :: MidiInWinMM( const std::string &clientName, unsigned int queueSizeLimit )
  : MidiInApi( queueSizeLimit )
{
  MidiInWinMM::initialize( clientName );
}

MidiInWinMM :: ~MidiInWinMM()
{
  // Close a connection if it exists.
  MidiInWinMM::closePort();

  WinMidiData *data = static_cast<WinMidiData *> (apiData_);
  DeleteCriticalSection( &(data->_mutex) );

  // Cleanup.
  delete data;
}

void MidiInWinMM :: initialize( const std::string& /*clientName*/ )
{
  // We'll issue a warning here if no devices are available but not
  // throw an error since the user can plugin something later.
  unsigned int nDevices = midiInGetNumDevs();
  if ( nDevices == 0 ) {
    errorString_ = "MidiInWinMM::initialize: no MIDI input devices currently available.";
    error( RtMidiError::WARNING, errorString_ );
  }

  // Save our api-specific connection information.
  WinMidiData *data = (WinMidiData *) new WinMidiData;
  apiData_ = (void *) data;
  inputData_.apiData = (void *) data;
  data->message.bytes.clear();  // needs to be empty for first input message
  data->closing = false;

  if ( !InitializeCriticalSectionAndSpinCount( &(data->_mutex), 0x00000400 ) ) {
    errorString_ = "MidiInWinMM::initialize: InitializeCriticalSectionAndSpinCount failed.";
    error( RtMidiError::WARNING, errorString_ );
  }
}

void MidiInWinMM :: openPort( unsigned int portNumber, const std::string &/*portName*/ )
{
  if ( connected_ ) {
    errorString_ = "MidiInWinMM::openPort: a valid connection already exists!";
    error( RtMidiError::WARNING, errorString_ );
    return;
  }

  unsigned int nDevices = midiInGetNumDevs();
  if (nDevices == 0) {
    errorString_ = "MidiInWinMM::openPort: no MIDI input sources found!";
    error( RtMidiError::NO_DEVICES_FOUND, errorString_ );
    return;
  }

  if ( portNumber >= nDevices ) {
    std::ostringstream ost;
    ost << "MidiInWinMM::openPort: the 'portNumber' argument (" << portNumber << ") is invalid.";
    errorString_ = ost.str();
    error( RtMidiError::INVALID_PARAMETER, errorString_ );
    return;
  }

  WinMidiData *data = static_cast<WinMidiData *> (apiData_);
  MMRESULT result = midiInOpen( &data->inHandle,
                                portNumber,
                                (DWORD_PTR)&midiInputCallback,
                                (DWORD_PTR)&inputData_,
                                CALLBACK_FUNCTION );
  if ( result != MMSYSERR_NOERROR ) {
    errorString_ = "MidiInWinMM::openPort: error creating Windows MM MIDI input port (MMRESULT " +
                   std::to_string( result ) + ").";
    error( RtMidiError::DRIVER_ERROR, errorString_ );
    return;
  }

  // Allocate and init the sysex buffers.
  data->sysexBuffer.resize( inputData_.bufferCount );
  for ( unsigned int i=0; i < inputData_.bufferCount; ++i ) {
    data->sysexBuffer[i] = new MIDIHDR();
    data->sysexBuffer[i]->lpData = new char[ inputData_.bufferSize ];
    data->sysexBuffer[i]->dwBufferLength = inputData_.bufferSize;
    data->sysexBuffer[i]->dwUser = i; // We use the dwUser parameter as buffer indicator
    data->sysexBuffer[i]->dwFlags = 0;

    result = midiInPrepareHeader( data->inHandle, data->sysexBuffer[i], sizeof(MIDIHDR) );
    if ( result != MMSYSERR_NOERROR ) {
      releaseWinMMInput( data );
      errorString_ = "MidiInWinMM::openPort: error starting Windows MM MIDI input port (PrepareHeader, MMRESULT " +
                     std::to_string( result ) + ").";
      error( RtMidiError::DRIVER_ERROR, errorString_ );
      return;
    }

    // Register the buffer.
    result = midiInAddBuffer( data->inHandle, data->sysexBuffer[i], sizeof(MIDIHDR) );
    if ( result != MMSYSERR_NOERROR ) {
      releaseWinMMInput( data );
      errorString_ = "MidiInWinMM::openPort: error starting Windows MM MIDI input port (AddBuffer, MMRESULT " +
                     std::to_string( result ) + ").";
      error( RtMidiError::DRIVER_ERROR, errorString_ );
      return;
    }
  }

  result = midiInStart( data->inHandle );
  if ( result != MMSYSERR_NOERROR ) {
    releaseWinMMInput( data );
    errorString_ = "MidiInWinMM::openPort: error starting Windows MM MIDI input port (MMRESULT " +
                   std::to_string( result ) + ").";
    error( RtMidiError::DRIVER_ERROR, errorString_ );
    return;
  }

  connected_ = true;
}

void MidiInWinMM :: openVirtualPort( const std::string &/*portName*/ )
{
  // This function cannot be implemented for the Windows MM MIDI API.
  errorString_ = "MidiInWinMM::openVirtualPort: cannot be implemented in Windows MM MIDI API!";
  error( RtMidiError::WARNING, errorString_ );
}

void MidiInWinMM :: closePort( void )
{
  if ( connected_ ) {
    WinMidiData *data = static_cast<WinMidiData *> (apiData_);
    // releaseWinMMInput() sets data->closing before anything else, which is
    // what keeps midiInputCallback() out of the buffers being retired.
    MMRESULT result = releaseWinMMInput( data );
    connected_ = false;

    // Reported only once the port is fully closed, and as a warning rather
    // than an error: nothing is left for the caller to act on, and
    // closePort() also runs from the destructor, where a thrown RtMidiError
    // would terminate the process.
    if ( result != MMSYSERR_NOERROR ) {
      errorString_ = "MidiInWinMM::closePort: error closing Windows MM MIDI input port (MMRESULT " +
                     std::to_string( result ) + ").";
      error( RtMidiError::WARNING, errorString_ );
    }
  }
}

void MidiInWinMM :: setClientName ( const std::string& )
{

  errorString_ = "MidiInWinMM::setClientName: this function is not implemented for the WINDOWS_MM API!";
  error( RtMidiError::WARNING, errorString_ );

}

void MidiInWinMM :: setPortName ( const std::string& )
{

  errorString_ = "MidiInWinMM::setPortName: this function is not implemented for the WINDOWS_MM API!";
  error( RtMidiError::WARNING, errorString_ );

}

unsigned int MidiInWinMM :: getPortCount()
{
  return midiInGetNumDevs();
}

std::string MidiInWinMM :: getPortName( unsigned int portNumber )
{
  std::string stringName;
  unsigned int nDevices = midiInGetNumDevs();
  if ( portNumber >= nDevices ) {
    std::ostringstream ost;
    ost << "MidiInWinMM::getPortName: the 'portNumber' argument (" << portNumber << ") is invalid.";
    errorString_ = ost.str();
    error( RtMidiError::WARNING, errorString_ );
    return stringName;
  }

  MIDIINCAPS deviceCaps;
  midiInGetDevCaps( portNumber, &deviceCaps, sizeof(MIDIINCAPS));
  stringName = ConvertToUTF8( deviceCaps.szPname );

  // Next lines added to add the portNumber to the name so that
  // the device's names are sure to be listed with individual names
  // even when they have the same brand name
#ifndef RTMIDI_DO_NOT_ENSURE_UNIQUE_PORTNAMES
  std::ostringstream os;
  os << " ";
  os << portNumber;
  stringName += os.str();
#endif

  return stringName;
}

//*********************************************************************//
//  API: Windows MM
//  Class Definitions: MidiOutWinMM
//*********************************************************************//

MidiOutWinMM :: MidiOutWinMM( const std::string &clientName ) : MidiOutApi()
{
  MidiOutWinMM::initialize( clientName );
}

MidiOutWinMM :: ~MidiOutWinMM()
{
  // Close a connection if it exists.
  MidiOutWinMM::closePort();

  // Cleanup.
  WinMidiData *data = static_cast<WinMidiData *> (apiData_);
  delete data;
}

void MidiOutWinMM :: initialize( const std::string& /*clientName*/ )
{
  // We'll issue a warning here if no devices are available but not
  // throw an error since the user can plug something in later.
  unsigned int nDevices = midiOutGetNumDevs();
  if ( nDevices == 0 ) {
    errorString_ = "MidiOutWinMM::initialize: no MIDI output devices currently available.";
    error( RtMidiError::WARNING, errorString_ );
  }

  // Save our api-specific connection information.
  WinMidiData *data = (WinMidiData *) new WinMidiData;
  apiData_ = (void *) data;
}

unsigned int MidiOutWinMM :: getPortCount()
{
  return midiOutGetNumDevs();
}

std::string MidiOutWinMM :: getPortName( unsigned int portNumber )
{
  std::string stringName;
  unsigned int nDevices = midiOutGetNumDevs();
  if ( portNumber >= nDevices ) {
    std::ostringstream ost;
    ost << "MidiOutWinMM::getPortName: the 'portNumber' argument (" << portNumber << ") is invalid.";
    errorString_ = ost.str();
    error( RtMidiError::WARNING, errorString_ );
    return stringName;
  }

  MIDIOUTCAPS deviceCaps;
  midiOutGetDevCaps( portNumber, &deviceCaps, sizeof( MIDIOUTCAPS ) );
  stringName = ConvertToUTF8( deviceCaps.szPname );

  // Next lines added to add the portNumber to the name so that
  // the device's names are sure to be listed with individual names
  // even when they have the same brand name
  std::ostringstream os;
#ifndef RTMIDI_DO_NOT_ENSURE_UNIQUE_PORTNAMES
  os << " ";
  os << portNumber;
  stringName += os.str();
#endif

  return stringName;
}

void MidiOutWinMM :: openPort( unsigned int portNumber, const std::string &/*portName*/ )
{
  if ( connected_ ) {
    errorString_ = "MidiOutWinMM::openPort: a valid connection already exists!";
    error( RtMidiError::WARNING, errorString_ );
    return;
  }

  unsigned int nDevices = midiOutGetNumDevs();
  if ( nDevices < 1 ) {
    errorString_ = "MidiOutWinMM::openPort: no MIDI output destinations found!";
    error( RtMidiError::NO_DEVICES_FOUND, errorString_ );
    return;
  }

  if ( portNumber >= nDevices ) {
    std::ostringstream ost;
    ost << "MidiOutWinMM::openPort: the 'portNumber' argument (" << portNumber << ") is invalid.";
    errorString_ = ost.str();
    error( RtMidiError::INVALID_PARAMETER, errorString_ );
    return;
  }

  WinMidiData *data = static_cast<WinMidiData *> (apiData_);
  MMRESULT result = midiOutOpen( &data->outHandle,
                                 portNumber,
                                 (DWORD)NULL,
                                 (DWORD)NULL,
                                 CALLBACK_NULL );
  if ( result != MMSYSERR_NOERROR ) {
    errorString_ = "MidiOutWinMM::openPort: error creating Windows MM MIDI output port (MMRESULT " +
                   std::to_string( result ) + ").";
    error( RtMidiError::DRIVER_ERROR, errorString_ );
    return;
  }

  connected_ = true;
}

void MidiOutWinMM :: closePort( void )
{
  if ( connected_ ) {
    WinMidiData *data = static_cast<WinMidiData *> (apiData_);
    // Disabled because midiOutReset triggers 0x7b (if any note was ON) and 0x79 "Reset All
    // Controllers" (to all 16 channels) CC messages which is undesirable (see issue #222)
    // midiOutReset( data->outHandle );
    //
    // Do not restore that call. It is the documented remedy for pending
    // buffers, so it looks like the obvious fix for the retry loop below, and
    // reinstating it reintroduces #222: an All Notes Off / Reset All
    // Controllers blast across all 16 channels on every port close.
    //
    // midiOutClose() returns MIDIERR_STILLPLAYING while buffers are still
    // queued, and in that case the handle is NOT closed and remains valid:
    // midiOutClose() only invalidates the handle on success, which is what
    // makes retrying on that one result correct rather than a use-after-close.
    // It would be safe even if a driver got that wrong: WinMM validates the
    // handle rather than dereferencing it, so a call on an invalidated handle
    // returns MMSYSERR_INVALHANDLE and the loop degrades to a harmless error.
    // The midiOutReset() that would retire the buffers is deliberately
    // disabled above, so give the driver a bounded time to finish instead.
    //
    // The bound is a deadline rather than an iteration count: Sleep( 1 )
    // sleeps for at least one scheduler tick, ~15.6 ms by default, so a loop
    // counting "1 ms" iterations overshoots badly -- a fault-injection build
    // measured 4.99 s across 312 supposed-1 ms retries. See
    // https://randomascii.wordpress.com/2020/10/04/windows-timer-resolution-the-great-rule-change/
    const DWORD start = GetTickCount();
    MMRESULT result;
    for ( ;; ) {
      result = midiOutClose( data->outHandle );
      if ( result != MIDIERR_STILLPLAYING || GetTickCount() - start >= kWinMMBufferReleaseTimeoutMs ) break;
      Sleep( 1 );
    }
    data->outHandle = 0;
    data->sysexInProgress = false;
    connected_ = false;

    // Reported once the object is back in a consistent state, and as a
    // warning rather than an error: closePort() also runs from the
    // destructor, where a thrown RtMidiError would terminate the process.
    if ( result != MMSYSERR_NOERROR ) {
      errorString_ = "MidiOutWinMM::closePort: error closing Windows MM MIDI output port (MMRESULT " +
                     std::to_string( result ) + "); the handle may have leaked.";
      error( RtMidiError::WARNING, errorString_ );
    }
  }
}

void MidiOutWinMM :: setClientName ( const std::string& )
{

  errorString_ = "MidiOutWinMM::setClientName: this function is not implemented for the WINDOWS_MM API!";
  error( RtMidiError::WARNING, errorString_ );

}

void MidiOutWinMM :: setPortName ( const std::string& )
{

  errorString_ = "MidiOutWinMM::setPortName: this function is not implemented for the WINDOWS_MM API!";
  error( RtMidiError::WARNING, errorString_ );

}

void MidiOutWinMM :: openVirtualPort( const std::string &/*portName*/ )
{
  // This function cannot be implemented for the Windows MM MIDI API.
  errorString_ = "MidiOutWinMM::openVirtualPort: cannot be implemented in Windows MM MIDI API!";
  error( RtMidiError::WARNING, errorString_ );
}

void MidiOutWinMM :: sendMessage( const unsigned char *message, size_t size )
{
  if ( !connected_ ) return;

  unsigned int nBytes = static_cast<unsigned int>(size);
  if ( nBytes == 0 ) {
    errorString_ = "MidiOutWinMM::sendMessage: message argument is empty!";
    error( RtMidiError::WARNING, errorString_ );
    return;
  }

  MMRESULT result;
  WinMidiData *data = static_cast<WinMidiData *> (apiData_);

  // Decide whether this buffer belongs on the long-message (SysEx) path.
  //
  // Routing on length alone is wrong: a caller may hand a large SysEx to
  // sendMessage() in several spans so it can pace the transfer, and the final
  // span is frequently short - often a bare 0xF7. Under a size-only test that
  // tail took the midiOutShortMsg() branch, which must not be used for SysEx,
  // so the remaining bytes never reached the device and the transfer never
  // completed. Track the SysEx state instead, so a continuation is routed by
  // what it is rather than by how big it happens to be.
  const unsigned char first = message[0];
  if ( data->sysexInProgress && rtmidiEndsSysex( first ) ) {
    // Close the abandoned SysEx first, so every receiver drops it the same way.
    // On Windows 11 build 26200.9457 the Windows MIDI service dropped everything sent
    // on a handle while a SysEx was open on it; with the October 2026 update (26200.9711)
    // it closes the SysEx with F7 itself, so receivers also get the cut-off part.
    static const unsigned char closeSysex[2] = { 0xF0, 0xF7 };
    sendMessage( closeSysex, 2 );
    // If the close failed (already reported), this message would land inside the open SysEx.
    if ( data->sysexInProgress ) return;
  }
  // A real-time message keeps an open SysEx open, but goes out as itself.
  bool isSysex = ( data->sysexInProgress && first < 0xF8 ) || first == 0xF0;
  if ( !isSysex && nBytes > 3 ) isSysex = true;

  if ( isSysex ) {

    // Work out the framing state this buffer leaves behind, but commit it only
    // once midiOutLongMsg() has accepted the bytes (below). If the send fails,
    // the state stays as it was, so a caller that retries the same span gets
    // it routed the same way; otherwise a retried tail of 1-3 bytes would drop
    // to midiOutShortMsg() and be lost again. Real-time bytes (0xF8-0xFF) may
    // be interleaved mid-SysEx without ending it; 0xF7 ends it, and so does
    // any other non-real-time status byte.
    bool nextSysexInProgress = data->sysexInProgress;
    for ( unsigned int i=0; i<nBytes; ++i ) {
      const unsigned char b = message[i];
      if ( b >= 0xF8 ) continue;
      if ( b == 0xF0 ) { nextSysexInProgress = true; continue; }
      if ( b == 0xF7 ) { nextSysexInProgress = false; continue; }
      if ( b >= 0x80 ) nextSysexInProgress = false;
    }

    // Allocate the MIDIHDR and its buffer on the heap. Once
    // midiOutPrepareHeader() succeeds the driver holds pointers to both, so
    // the failure exits below that may leave the driver owning them leak both
    // on purpose. Leaking only the buffer is not enough: a stack MIDIHDR would
    // be reused as soon as this function returned (#377).
    MIDIHDR *sysex = new MIDIHDR();
    char *buffer = (char *) malloc( nBytes );
    if ( buffer == NULL ) {
      delete sysex;
      errorString_ = "MidiOutWinMM::sendMessage: error allocating sysex message memory!";
      error( RtMidiError::MEMORY_ERROR, errorString_ );
      return;
    }

    // Copy data to buffer.
    for ( unsigned int i=0; i<nBytes; ++i ) buffer[i] = message[i];

    sysex->lpData = (LPSTR) buffer;
    sysex->dwBufferLength = nBytes;
    sysex->dwFlags = 0;
    result = midiOutPrepareHeader( data->outHandle, sysex, sizeof( MIDIHDR ) );
    if ( result != MMSYSERR_NOERROR ) {
      // Nothing was handed to the driver, so both can be freed.
      free( buffer );
      delete sysex;
      errorString_ = "MidiOutWinMM::sendMessage: error preparing sysex header (MMRESULT " +
                     std::to_string( result ) + ").";
      error( RtMidiError::DRIVER_ERROR, errorString_ );
      return;
    }

    // Send the message. From here on, free only what the driver has provably
    // released, i.e. only after midiOutUnprepareHeader() succeeds.
    result = midiOutLongMsg( data->outHandle, sysex, sizeof( MIDIHDR ) );
    if ( result != MMSYSERR_NOERROR ) {
      if ( midiOutUnprepareHeader( data->outHandle, sysex, sizeof( MIDIHDR ) ) == MMSYSERR_NOERROR ) {
        free( buffer );
        delete sysex;
      } // else deliberately leaked: the driver may still hold them
      errorString_ = "MidiOutWinMM::sendMessage: error sending sysex message (MMRESULT " +
                     std::to_string( result ) + ").";
      error( RtMidiError::DRIVER_ERROR, errorString_ );
      return;
    }

    // The driver has accepted the bytes, so the framing state moves on, even
    // if releasing the buffer fails below.
    data->sysexInProgress = nextSysexInProgress;

    // Unprepare the buffer and MIDIHDR.
    //
    // midiOutUnprepareHeader() returns MIDIERR_STILLPLAYING while the driver
    // is not yet done with the buffer, which is the one result worth retrying.
    // Two things matter here: the retry has to be bounded, because how eagerly
    // a driver retires a buffer is not something this library can probe for
    // (third-party MIDI drivers are still supported on Windows); and any other
    // non-zero result has to be treated as a failure rather than as a reason to
    // fall out of the loop and free a buffer the driver may still own. Surprise
    // device removal part-way through a long dump reaches exactly that path.
    //
    // The bound is a deadline rather than an iteration count: Sleep( 1 ) sleeps
    // for at least one scheduler tick, ~15.6 ms by default, so counting 1 ms
    // iterations would overshoot the intended bound many times over.
    const DWORD start = GetTickCount();
    for ( ;; ) {
      result = midiOutUnprepareHeader( data->outHandle, sysex, sizeof ( MIDIHDR ) );
      if ( result == MMSYSERR_NOERROR ) break;
      if ( result != MIDIERR_STILLPLAYING ) {
        errorString_ = "MidiOutWinMM::sendMessage: error unpreparing sysex header (MMRESULT " +
                       std::to_string( result ) + "); buffer not freed.";
        error( RtMidiError::DRIVER_ERROR, errorString_ );
        return; // deliberately leaked: the driver may still hold them
      }
      if ( GetTickCount() - start >= kWinMMBufferReleaseTimeoutMs ) {
        errorString_ = "MidiOutWinMM::sendMessage: timed out waiting for the driver to "
                       "release the sysex buffer; buffer not freed.";
        error( RtMidiError::DRIVER_ERROR, errorString_ );
        return; // deliberately leaked, same reason
      }
      Sleep( 1 );
    }
    free( buffer );
    delete sysex;
  }
  else { // Channel or system message.

    // Pack MIDI bytes into double word. Value-initialize: only nBytes of the
    // four are written, and midiOutShortMsg() should not be handed
    // indeterminate bytes even though it documents the unused ones as ignored.
    DWORD packet = 0;
    unsigned char *ptr = (unsigned char *) &packet;
    for ( unsigned int i=0; i<nBytes; ++i ) {
      *ptr = message[i];
      ++ptr;
    }

    // Send the message immediately.
    result = midiOutShortMsg( data->outHandle, packet );
    if ( result != MMSYSERR_NOERROR ) {
      errorString_ = "MidiOutWinMM::sendMessage: error sending MIDI message (MMRESULT " +
                     std::to_string( result ) + ").";
      error( RtMidiError::DRIVER_ERROR, errorString_ );
    }
  }
}

#endif  // RTMIDI_API_WINMM


//*********************************************************************//
//  API: Universal Windows Platform (UWP)
//*********************************************************************//

// C++/WinRT
//   https://github.com/microsoft/cppwinrt
//
// UWP MIDI API
//   https://docs.microsoft.com/en-us/windows/uwp/audio-video-camera/midi
//
// Example implementation using UWP MIDI in C++/WinRT
//   https://github.com/trueroad/uwp_midiio

#if defined(RTMIDI_API_UWP)

#include <algorithm>
#include <chrono>
#include <regex>
#include <string_view>
#include <mutex>

#include <windows.h>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Devices.Midi.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Security.Cryptography.h>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Midi;
using namespace Windows::Storage::Streams;
using namespace Windows::Security::Cryptography;

// Class for initializing C++/WinRT
class UWPMidiInit
{
public:
    UWPMidiInit()
    {
        winrt::init_apartment();
    }
};

// Class for handling UWP MIDI
class UWPMidiClass
{
public:
    // Structure to store MIDI port name and ID
    struct port
    {
        std::string name;
        std::wstring id;
        std::string hex_id;
        std::string display_name;
    };

    UWPMidiClass(MidiApi& midi_api) :
        midi_api_(midi_api)
    {
    }

    ~UWPMidiClass()
    {
        close();
    }

    // Initialize for MIDI IN
    void in_init(MidiInApi::RtMidiInData* input_data)
    {
        input_data_ = input_data;

        try
        {
            ports_ = list_ports(MidiInPort::GetDeviceSelector());
        }
        catch (hresult_error const& ex)
        {
            raise_hresult_error("UWPMidiClass::in_init: ", ex);
        }
        sort_display_name(ports_);
    }

    // Initialize for MIDI OUT
    void out_init()
    {
        try
        {
            ports_ = list_ports(MidiOutPort::GetDeviceSelector());
            fix_display_name(list_ports(MidiInPort::GetDeviceSelector()), ports_);
        }
        catch (hresult_error const& ex)
        {
            raise_hresult_error("UWPMidiClass::out_init: ", ex);
        }
        sort_display_name(ports_);
    }

    size_t get_num_ports()
    {
        return ports_.size();
    }

    std::string get_port_name(size_t n)
    {
        return ports_[n].display_name;
    }

    bool in_open(size_t port_number);
    bool out_open(size_t port_number);
    void close();

    void midi_in_callback(const MidiInPort&, const MidiMessageReceivedEventArgs& e);
    bool send_buffer(const unsigned char* buff, size_t len);

    // Raise RtMidi error for hresult error
    void raise_hresult_error(std::string_view message, hresult_error const& ex)
    {
        std::ostringstream ss;
        ss << message << "exception HRESULT 0x" << std::hex << ex.code() << ", "
            << utf16_to_utf8(static_cast<std::wstring_view>(ex.message()))
            << "\n";
        midi_api_.error(RtMidiError::DRIVER_ERROR, ss.str());
    }

    // Mutex for MIDI port open/close
    std::mutex mtx_open_close_;
    // Mutex for MIDI IN message queue access
    std::mutex mtx_queue_;

private:
    std::vector<port> list_ports(winrt::hstring device_selector);
    void fix_display_name(const std::vector<port>& in_ports,
        std::vector<port>& out_ports);
    void sort_display_name(std::vector<port>& ports);
    std::string utf16_to_utf8(const std::wstring_view wstr);

    template<class MidiPort_T, class IMidiPort_T>
    IMidiPort_T open(size_t port_number);

    // MidiApi class
    MidiApi& midi_api_;

    // List of MIDI ports
    std::vector<port> ports_;
    // MIDI IN port
    MidiInPort in_port_{ nullptr };
    // MIDI OUT port
    IMidiOutPort out_port_{ nullptr };
    // Output: a SysEx has started and not yet ended
    bool sysex_open_{ false };
    // Backup initial MessageReceived event token
    winrt::event_token before_token_;
    // Input data
    MidiInApi::RtMidiInData* input_data_{ nullptr };
    // Last timestamp
    std::chrono::duration<TimeSpan::rep, TimeSpan::period> last_time_{ 0 };

    // C++/WinRT initializer
    static UWPMidiInit uwp_midi_init_;
    // Regex pattern to extract 8 hex digits from UWP MIDI ID string
    static const std::wregex hex_id_pattern_;

#ifndef RTMIDI_DO_NOT_ENABLE_WORKAROUND_UWP_WRONG_TIMESTAMPS
    // QueryPerformanceFrequency
    LONGLONG qpc_freq_{ 0 };
    // Last QueryPerformanceCounter
    LONGLONG before_qpc_;
    // Weather overflow low occurred or not
    bool b_overflow_low_{ false };

    // BLE-MIDI timestamp periods
    inline constexpr static std::chrono::duration<TimeSpan::rep, TimeSpan::period> ble_midi_period_low_{ std::chrono::milliseconds{128} };
    inline constexpr static std::chrono::duration<TimeSpan::rep, TimeSpan::period> ble_midi_period_high_{ std::chrono::milliseconds{8192} };
    // QPC threshold 4096 ms
    inline constexpr static LONGLONG qpc_threshold_{ 4096 };

    // Regex pattern to detect BLE-MIDI IN
    static const std::wregex ble_midi_pattern_;
#endif
};

// C++/WinRT initializer
UWPMidiInit UWPMidiClass::uwp_midi_init_;
// Regex pattern to extract 8 hex digits from UWP MIDI ID string
const std::wregex UWPMidiClass::hex_id_pattern_{ std::wregex(L"#MIDII_([0-9A-F]{8})\\..+#") };

#ifndef RTMIDI_DO_NOT_ENABLE_WORKAROUND_UWP_WRONG_TIMESTAMPS
const std::wregex UWPMidiClass::ble_midi_pattern_{ std::wregex(L"#MIDII_[0-9A-F]{8}\\.BLE[0-9]{2}#") };
#endif

// Find and create a list of UWP MIDI ports
std::vector<UWPMidiClass::port> UWPMidiClass::list_ports(winrt::hstring device_selector)
{
    const auto devs{ DeviceInformation::FindAllAsync(device_selector).get() };

    std::vector<port> retval;
    for (const auto& d : devs)
    {
        port p;
        p.name = utf16_to_utf8(d.Name());
        p.id = d.Id();

        std::wsmatch m;
        if (std::regex_search(p.id, m, hex_id_pattern_))
        {
            // Ordinary MIDI ports
            // Append hex digits extracted from the UWP MIDI ID string to the port name.
            p.hex_id = utf16_to_utf8(m[1].str());

            std::ostringstream ss;
            ss << p.name
                << " [ "
                << p.hex_id
                << " ]";
            p.display_name = ss.str();
        }
        else
        {
            // Microsoft GS Wavetable Synth etc.
            // Unable to extract hex digits from UWP MIDI ID string.
            // Use the device name as the port name.
            p.display_name = p.name;
        }

        retval.push_back(p);
    }
    return retval;
}

// Fix MIDI OUT port names starting with `MIDI` or `2 - MIDI` to MIDI IN port names with similar ID strings
void UWPMidiClass::fix_display_name(const std::vector<port>& in_ports,
    std::vector<port>& out_ports)
{
    for (auto& outp : out_ports)
    {
        if (outp.hex_id.empty())
            continue;
        if (std::string_view{ outp.name }.substr(0, 4) != "MIDI" && std::string_view{ outp.name }.substr(1, 7) != " - MIDI")
            continue;

        for (const auto& inp : in_ports)
        {
            if (outp.hex_id == inp.hex_id)
            {
                outp.display_name = inp.display_name;
                break;
            }
        }
    }
}

void UWPMidiClass::sort_display_name(std::vector<port>& ports)
{
    std::sort(ports.begin(), ports.end(),
        [](const auto& lhs, const auto& rhs)
    {
        return lhs.display_name < rhs.display_name;
    });
}

std::string UWPMidiClass::utf16_to_utf8(const std::wstring_view wstr)
{
    auto len{ WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr) };
    std::string u8str(len, '\0');
    if (len)
        WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), u8str.data(), len, nullptr, nullptr);
    return u8str;
}

// Open MIDI IN/OUT port
template<class MidiPort_T, class IMidiPort_T>
IMidiPort_T UWPMidiClass::open(size_t port_number)
{
    try
    {
        auto async{ MidiPort_T::FromIdAsync(ports_[port_number].id) };
        // Timeout 3 seconds
        if (async.wait_for(std::chrono::seconds(3)) == AsyncStatus::Completed)
            return async.GetResults();
    }
    catch (hresult_error const& ex)
    {
        raise_hresult_error("UWPMidiClass::open: ", ex);
    }
    return nullptr;
}

// Open MIDI IN port
bool UWPMidiClass::in_open(size_t port_number)
{
    if (in_port_)
        in_port_.Close();

    in_port_ = open<MidiInPort, MidiInPort>(port_number);
    if (!in_port_)
        return false;

#ifndef RTMIDI_DO_NOT_ENABLE_WORKAROUND_UWP_WRONG_TIMESTAMPS
    std::wsmatch m;
    if (std::regex_search(ports_[port_number].id, m, ble_midi_pattern_))
    {
        // BLE-MIDI IN port
        LARGE_INTEGER li;
        if (::QueryPerformanceFrequency(&li))
            qpc_freq_ = li.QuadPart;
    }
#endif

    try
    {
        before_token_ = in_port_.MessageReceived({ this, &UWPMidiClass::midi_in_callback });
    }
    catch (hresult_error const& ex)
    {
        raise_hresult_error("UWPMidiClass::in_open: ", ex);
    }

    return true;
}

// Open MIDI Out port
bool UWPMidiClass::out_open(size_t port_number)
{
    if (out_port_)
        out_port_.Close();

    out_port_ = open<MidiOutPort, IMidiOutPort>(port_number);
    if (!out_port_)
        return false;

    return true;
}

// Close MIDI IN/OUT port
void UWPMidiClass::close()
{
    if (in_port_)
    {
        if (before_token_)
            in_port_.MessageReceived(before_token_);

        in_port_.Close();
        in_port_ = nullptr;
    }
    if (out_port_)
    {
        out_port_.Close();
        out_port_ = nullptr;
    }
    // A SysEx left open here must not be closed on the next port opened.
    sysex_open_ = false;
}

// MessageReceived event handler
void UWPMidiClass::midi_in_callback(const MidiInPort&, const MidiMessageReceivedEventArgs& e)
{
#ifndef RTMIDI_DO_NOT_ENABLE_WORKAROUND_UWP_WRONG_TIMESTAMPS
    LARGE_INTEGER qpc;
    if (qpc_freq_)
    {
        if (!::QueryPerformanceCounter(&qpc))
            qpc_freq_ = 0;
    }
#endif

    const auto& m{ e.Message() };
    if (!m)
        return;

    MidiInApi::MidiMessage message;
    const std::chrono::duration<TimeSpan::rep, TimeSpan::period> duration{ m.Timestamp() };

    // Calculate time stamp.
    if (input_data_->firstMessage == true)
    {
        message.timeStamp = 0.0;
        input_data_->firstMessage = false;
        last_time_ = duration;

#ifndef RTMIDI_DO_NOT_ENABLE_WORKAROUND_UWP_WRONG_TIMESTAMPS
        if (qpc_freq_)
            before_qpc_ = qpc.QuadPart;
#endif
    }
    else
    {
        auto delta{ duration - last_time_ };

#ifndef RTMIDI_DO_NOT_ENABLE_WORKAROUND_UWP_WRONG_TIMESTAMPS
        if (qpc_freq_)
        {
            if (b_overflow_low_)
            {
                if (delta >= ble_midi_period_low_)
                {
                    // Fix after overflow low
                    // https://github.com/trueroad/BLE_MIDI_packet_data_set#page-7-overflow-both
                    delta -= ble_midi_period_low_;
                    b_overflow_low_ = false;
                }
            }
            else
            {
                if ((ble_midi_period_high_ - ble_midi_period_low_) < delta && delta < ble_midi_period_high_ &&
                    ((before_qpc_ - qpc.QuadPart) * 1000 / qpc_freq_) < qpc_threshold_)
                {
                    // Fix overflow low
                    // https://github.com/trueroad/BLE_MIDI_packet_data_set#page-7-overflow-low
                    delta = delta - ble_midi_period_high_ + ble_midi_period_low_;
                    b_overflow_low_ = true;
                }
            }

            before_qpc_ = qpc.QuadPart;
        }
#endif

        const std::chrono::duration<double> sec{ delta };
        message.timeStamp = sec.count();
    }

    if (((input_data_->ignoreFlags & 0x01) &&
            (m.Type() == MidiMessageType::SystemExclusive || m.Type() == MidiMessageType::EndSystemExclusive)) ||
        ((input_data_->ignoreFlags & 0x02) &&
            (m.Type() == MidiMessageType::MidiTimeCode || m.Type() == MidiMessageType::TimingClock)) ||
        ((input_data_->ignoreFlags & 0x04) &&
            m.Type() == MidiMessageType::ActiveSensing))
    {
        return;
    }

    const auto& raw_data{ m.RawData() };
    const size_t len{ raw_data.Length() };

    // Windows passes on a SysEx that lost its end without an F7. Drop it, as
    // the other backends do (#407).
    if (len && raw_data.data()[0] == 0xF0 && raw_data.data()[len - 1] != 0xF7)
        return;

    if (len)
        message.bytes.assign(raw_data.data(), raw_data.data() + len);

    last_time_ = duration;

    if (!input_data_->callUserCallback(message.timeStamp, &message.bytes))
    {
        std::lock_guard<std::mutex> lock(mtx_queue_);

        if (!input_data_->queue.push(message))
        {
            std::cerr << "\nMidiInWinUWP: message queue limit reached!!\n\n";
        }
    }
}

// Send MIDI message
bool UWPMidiClass::send_buffer(const unsigned char* buff, size_t len)
{
    if (!out_port_)
        return false;

    if (len > 0 && sysex_open_ && rtmidiEndsSysex(buff[0]))
    {
        // Close the abandoned SysEx first, so every receiver drops it the same way.
        static const unsigned char closeSysex[2] = { 0xF0, 0xF7 };
        send_buffer(closeSysex, 2);
        // If the close failed (already reported), this message would land inside the open SysEx.
        if (sysex_open_)
            return true;
    }

    try
    {
        out_port_.SendBuffer(CryptographicBuffer::CreateFromByteArray(array_view(buff, buff + len)));
    }
    catch (hresult_error const& ex)
    {
        raise_hresult_error("UWPMidiClass::send_buffer: ", ex);
        return true;
    }

    // F0 opens a SysEx; F7 or any other status byte except real-time ends it.
    for (size_t i = 0; i < len; i++)
    {
        if (buff[i] == 0xF0) sysex_open_ = true;
        else if (buff[i] >= 0x80 && buff[i] < 0xF8) sysex_open_ = false;
    }

    return true;
}

//*********************************************************************//
//  API: Windows UWP
//  Class Definitions: MidiInWinUWP
//*********************************************************************//

MidiInWinUWP::MidiInWinUWP(const std::string& clientName, unsigned int queueSizeLimit)
    : MidiInApi(queueSizeLimit)
{
    MidiInWinUWP::initialize(clientName);
}

MidiInWinUWP :: ~MidiInWinUWP()
{
    // Close a connection if it exists.
    MidiInWinUWP::closePort();

    // Cleanup.
    UWPMidiClass *data = static_cast<UWPMidiClass*> (apiData_);
    delete data;
}

void MidiInWinUWP::initialize(const std::string& /*clientName*/)
{
    // Save our api-specific connection information.
    UWPMidiClass* data{ new UWPMidiClass(*this) };
    data->in_init(&inputData_);
    apiData_ = static_cast<void*>(data);

    // We'll issue a warning here if no devices are available but not
    // throw an error since the user can plugin something later.
    const auto nDevices{ data->get_num_ports() };
    if (nDevices == 0)
    {
        errorString_ = "MidiInWinUWP::initialize: no MIDI input devices currently available.";
        error(RtMidiError::WARNING, errorString_);
    }
}

void MidiInWinUWP::openPort(unsigned int portNumber, const std::string&/*portName*/)
{
    UWPMidiClass* data{ static_cast<UWPMidiClass*>(apiData_) };
    std::lock_guard<std::mutex> lock(data->mtx_open_close_);

    if (connected_)
    {
        errorString_ = "MidiInWinUWP::openPort: a valid connection already exists!";
        error(RtMidiError::WARNING, errorString_);
        return;
    }

    if (data->get_num_ports() == 0)
    {
        errorString_ = "MidiInWinUWP::openPort: no MIDI input sources found!";
        error(RtMidiError::NO_DEVICES_FOUND, errorString_);
        return;
    }

    if (portNumber >= data->get_num_ports())
    {
        std::ostringstream ost;
        ost << "MidiInWinUWP::openPort: the 'portNumber' argument (" << portNumber << ") is invalid.";
        errorString_ = ost.str();
        error(RtMidiError::INVALID_PARAMETER, errorString_);
        return;
    }

    if (!data->in_open(portNumber))
    {
        errorString_ = "MidiInWinUWP::openPort: error creating Windows UWP MIDI input port.";
        error(RtMidiError::DRIVER_ERROR, errorString_);
        return;
    }

    connected_ = true;
}

void MidiInWinUWP::openVirtualPort(const std::string&/*portName*/)
{
    // This function cannot be implemented for the Windows UWP MIDI API.
    errorString_ = "MidiInWinUWP::openVirtualPort: cannot be implemented in Windows UWP MIDI API!";
    error(RtMidiError::WARNING, errorString_);
}

void MidiInWinUWP::closePort(void)
{
    UWPMidiClass* data{ static_cast<UWPMidiClass*>(apiData_) };
    std::lock_guard<std::mutex> lock(data->mtx_open_close_);

    if (connected_)
    {
        data->close();
        connected_ = false;
    }
}

void MidiInWinUWP::setClientName(const std::string&)
{
    errorString_ = "MidiInWinUWP::setClientName: this function is not implemented for the WINDOWS_UWP API!";
    error(RtMidiError::WARNING, errorString_);
}

void MidiInWinUWP::setPortName(const std::string&)
{
    errorString_ = "MidiInWinUWP::setPortName: this function is not implemented for the WINDOWS_UWP API!";
    error(RtMidiError::WARNING, errorString_);
}

unsigned int MidiInWinUWP::getPortCount()
{
    UWPMidiClass* data{ static_cast<UWPMidiClass*>(apiData_) };
    return static_cast<unsigned int>(data->get_num_ports());
}

std::string MidiInWinUWP::getPortName(unsigned int portNumber)
{
    UWPMidiClass* data{ static_cast<UWPMidiClass*>(apiData_) };

    const auto nDevices{ data->get_num_ports() };
    if (portNumber >= nDevices)
    {
        std::ostringstream ost;
        ost << "MidiInWinUWP::getPortName: the 'portNumber' argument (" << portNumber << ") is invalid.";
        errorString_ = ost.str();
        error(RtMidiError::WARNING, errorString_);
        return "";
    }

    return data->get_port_name(portNumber);
}

double MidiInWinUWP::getMessage(std::vector<unsigned char>* message)
{
    UWPMidiClass* data{ static_cast<UWPMidiClass*>(apiData_) };
    std::lock_guard<std::mutex> lock(data->mtx_queue_);

    return MidiInApi::getMessage(message);
}

//*********************************************************************//
//  API: Windows UWP
//  Class Definitions: MidiOutWinUWP
//*********************************************************************//

MidiOutWinUWP::MidiOutWinUWP(const std::string& clientName) : MidiOutApi()
{
    MidiOutWinUWP::initialize(clientName);
}

MidiOutWinUWP :: ~MidiOutWinUWP()
{
    // Close a connection if it exists.
    MidiOutWinUWP::closePort();

    // Cleanup.
    UWPMidiClass* data = static_cast<UWPMidiClass*> (apiData_);
    delete data;
}

void MidiOutWinUWP::initialize(const std::string& /*clientName*/)
{
    // Save our api-specific connection information.
    UWPMidiClass* data{ new UWPMidiClass(*this) };
    data->out_init();
    apiData_ = static_cast<void*>(data);

    // We'll issue a warning here if no devices are available but not
    // throw an error since the user can plug something in later.
    const auto nDevices{ data->get_num_ports() };
    if (nDevices == 0)
    {
        errorString_ = "MidiOutWinUWP::initialize: no MIDI output devices currently available.";
        error(RtMidiError::WARNING, errorString_);
    }
}

unsigned int MidiOutWinUWP::getPortCount()
{
    UWPMidiClass* data{ static_cast<UWPMidiClass*>(apiData_) };
    return static_cast<unsigned int>(data->get_num_ports());
}

std::string MidiOutWinUWP::getPortName(unsigned int portNumber)
{
    UWPMidiClass* data{ static_cast<UWPMidiClass*>(apiData_) };

    const auto nDevices{ data->get_num_ports() };
    if (portNumber >= nDevices)
    {
        std::ostringstream ost;
        ost << "MidiOutWinUWP::getPortName: the 'portNumber' argument (" << portNumber << ") is invalid.";
        errorString_ = ost.str();
        error(RtMidiError::WARNING, errorString_);
        return "";
    }

    return data->get_port_name(portNumber);
}

void MidiOutWinUWP::openPort(unsigned int portNumber, const std::string&/*portName*/)
{
    UWPMidiClass* data{ static_cast<UWPMidiClass*>(apiData_) };
    std::lock_guard<std::mutex> lock(data->mtx_open_close_);

    if (connected_)
    {
        errorString_ = "MidiOutWinUWP::openPort: a valid connection already exists!";
        error(RtMidiError::WARNING, errorString_);
        return;
    }

    if (data->get_num_ports() == 0)
    {
        errorString_ = "MidiOutWinUWP::openPort: no MIDI output destinations found!";
        error(RtMidiError::NO_DEVICES_FOUND, errorString_);
        return;
    }

    if (portNumber >= data->get_num_ports())
    {
        std::ostringstream ost;
        ost << "MidiOutWinUWP::openPort: the 'portNumber' argument (" << portNumber << ") is invalid.";
        errorString_ = ost.str();
        error(RtMidiError::INVALID_PARAMETER, errorString_);
        return;
    }

    if (!data->out_open(portNumber))
    {
        errorString_ = "MidiOutWinUWP::openPort: error creating Windows UWP MIDI output port.";
        error(RtMidiError::DRIVER_ERROR, errorString_);
        return;
    }

    connected_ = true;
}

void MidiOutWinUWP::closePort(void)
{
    UWPMidiClass* data{ static_cast<UWPMidiClass*>(apiData_) };
    std::lock_guard<std::mutex> lock(data->mtx_open_close_);

    if (connected_)
    {
        data->close();
        connected_ = false;
    }
}

void MidiOutWinUWP::setClientName(const std::string&)
{
    errorString_ = "MidiOutWinUWP::setClientName: this function is not implemented for the WINDOWS_UWP API!";
    error(RtMidiError::WARNING, errorString_);
}

void MidiOutWinUWP::setPortName(const std::string&)
{
    errorString_ = "MidiOutWinUWP::setPortName: this function is not implemented for the WINDOWS_UWP API!";
    error(RtMidiError::WARNING, errorString_);
}

void MidiOutWinUWP::openVirtualPort(const std::string&/*portName*/)
{
    // This function cannot be implemented for the Windows UWP MIDI API.
    errorString_ = "MidiOutWinUWP::openVirtualPort: cannot be implemented in Windows UWP MIDI API!";
    error(RtMidiError::WARNING, errorString_);
}

void MidiOutWinUWP::sendMessage(const unsigned char* message, size_t size)
{
    if (!connected_)
        return;

    if (size == 0)
    {
        errorString_ = "MidiOutWinUWP::sendMessage: message argument is empty!";
        error(RtMidiError::WARNING, errorString_);
        return;
    }

    UWPMidiClass* data{ static_cast<UWPMidiClass*>(apiData_) };
    if (!data->send_buffer(message, size))
    {
        errorString_ = "MidiOutWinUWP::sendMessage: error sending message.";
        error(RtMidiError::DRIVER_ERROR, errorString_);
    }
}

#endif  // RTMIDI_API_UWP


//*********************************************************************//
//  API: Windows MIDI Services
//
//  Uses the in-box Windows MIDI Services API, Windows.Devices.Midi2, part
//  of Windows 11 25H2 and later. Elsewhere the backend reports itself
//  unavailable and another API is used.
//  https://github.com/microsoft/MIDI
//
//*********************************************************************//

#if defined(RTMIDI_API_WMS)

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string_view>
#include <thread>

#include <windows.h>
#include <combaseapi.h>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Devices.Midi2.h>
#include <winrt/Windows.Devices.Midi2.Enumeration.h>
#include <winrt/Windows.Devices.Midi2.Enumeration.Legacy.h>

// The in-box Windows MIDI Services API, which Microsoft is rolling out to Windows
// 11 25H2 and later. On a system without it, activating its classes fails with
// REGDB_E_CLASSNOTREG, the backend reports itself unavailable, and RtMidi falls
// back to another API.
namespace midi2      = winrt::Windows::Devices::Midi2;
namespace midi2enum  = winrt::Windows::Devices::Midi2::Enumeration;
namespace midi2legacy = winrt::Windows::Devices::Midi2::Enumeration::Legacy;

// ---------------------------------------------------------------------------
// COM extension interface - inline definition from the IDL.
// Define RTMIDI_USE_WMS_COM_RAW to use SendMidiMessagesRaw instead of
// the WinRT SendMultipleMessagesWordArray path.
//
// #define RTMIDI_USE_WMS_COM_RAW
// ---------------------------------------------------------------------------
struct __declspec(uuid("8087b303-0519-31d1-31d1-000000000020")) IMidiEndpointConnectionRaw : IUnknown
{
    virtual UINT32  __stdcall GetSupportedMaxMidiWordsPerTransmission() = 0;
    virtual BOOL    __stdcall ValidateBufferHasOnlyCompleteUmps(UINT32 wordCount, UINT32* messages) = 0;
    virtual HRESULT __stdcall SendMidiMessagesRaw(UINT64 timestamp, UINT32 wordCount, UINT32* completeMessages) = 0;
    virtual HRESULT __stdcall SetMessagesReceivedCallback(IUnknown* callback) = 0;
    virtual HRESULT __stdcall RemoveMessagesReceivedCallback() = 0;
};

using midi2::MidiClock;
using midi2::MidiSession;
using midi2::MidiEndpointConnection;
using midi2::MidiMessageReceivedEventArgs;
using midi2::IMidiMessageReceivedEventSource;
using midi2enum::MidiEndpointDeviceInformation;
using midi2enum::MidiEndpointDeviceInformationFilters;
using midi2enum::MidiGroupTerminalBlock;
using midi2enum::MidiGroupTerminalBlockDirection;
using midi2enum::MidiEndpointDeviceWatcher;
using midi2enum::MidiEndpointDeviceInformationAddedEventArgs;
using midi2enum::MidiEndpointDeviceInformationUpdatedEventArgs;
using midi2enum::MidiEndpointDeviceInformationRemovedEventArgs;
using midi2enum::Midi1PortFlow;
using midi2legacy::MidiLegacyPortDeviceInformation;

// ---------------------------------------------------------------------------
// Process-wide COM setup and API detection, done on first use.
//
// The backend never initializes COM on a caller's thread: that would fix the
// apartment of a thread the application owns (for example a GUI thread that
// later calls OleInitialize). CoIncrementMTAUsage() keeps the process's
// multi-threaded apartment alive instead, so threads that have not initialized
// COM run in it implicitly, and threads that have are used as they are. The MTA
// usage cookie is deliberately never released.
//
// Detection follows Microsoft's detect-midi-services sample. MidiApi is a static
// runtime class, so it is found through its activation factory. A system that
// has the API can still be set to use the legacy MIDI stack instead; only the
// full Windows MIDI Services mode is used here. In the hybrid mode, devices with
// MIDI 1.0 drivers appear only through the older APIs, so WinMM sees more.
// ---------------------------------------------------------------------------
enum class WmsAvailability
{
    Available,
    NotPresent,          // this version of Windows has no Windows.Devices.Midi2
    ServiceUnavailable,  // the MIDI service is not running and did not start
    LegacyMode           // Windows is set to the legacy or hybrid MIDI stack
};

static WmsAvailability wms_availability()
{
    static std::mutex mtx;
    static CO_MTA_USAGE_COOKIE mta_cookie = nullptr;
    static bool available = false;

    std::lock_guard<std::mutex> lock(mtx);
    if (available)
        return WmsAvailability::Available;

    if (!mta_cookie && FAILED(CoIncrementMTAUsage(&mta_cookie)))
        mta_cookie = nullptr;

    // A failure is not remembered, so a service that starts (or a mode that
    // changes) while the application runs is found by the next RtMidi object.
    try
    {
        winrt::hresult_error error;
        auto statics = winrt::try_get_activation_factory<midi2::MidiApi, midi2::IMidiApiStatics>(error);
        if (!statics)
            return WmsAvailability::NotPresent;

        // Also starts the service if it is not running yet.
        if (!statics.EnsureServiceAvailable())
            return WmsAvailability::ServiceUnavailable;

        if (statics.GetCurrentlySelectedApiMode() != midi2::MidiApiMode::FullWindowsMidiServicesMode)
            return WmsAvailability::LegacyMode;
    }
    catch (winrt::hresult_error const&)
    {
        return WmsAvailability::ServiceUnavailable;
    }

    available = true;
    return WmsAvailability::Available;
}

static bool wms_runtime_available()
{
    return wms_availability() == WmsAvailability::Available;
}

// ---------------------------------------------------------------------------
// MIDI 1.0 byte stream -> UMP word encoder
//
// State persists across calls, so chunked SysEx (F0 in one buffer, F7 in a
// later one), running status, and mixed buffers encode correctly.
//   - Channel voice (0x80-0xEF)     type 2, 32-bit
//   - System common (0xF1-0xF6)     type 1, 32-bit
//   - System real-time (0xF8-0xFF)  type 1, emitted immediately without
//                                   interrupting SysEx or running status
//   - SysEx (0xF0 ... 0xF7)         type 3, 64-bit packets
// ---------------------------------------------------------------------------
class Midi1UmpEncoder
{
public:
    Midi1UmpEncoder() = default;

    //! Encode bytes from buf[0..len-1], appending UMP words to |out|.
    //! |group| is the UMP group index (0-15).
    void encode(const uint8_t* buf, size_t len, uint8_t group,
                std::vector<uint32_t>& out)
    {
        for (size_t i = 0; i < len; ++i)
        {
            const uint8_t b = buf[i];

            // System Realtime (0xF8-0xFF): single-byte, emit immediately.
            // Does NOT cancel running status or interrupt SysEx accumulation.
            if (b >= 0xF8)
            {
                emit_type1(b, 0, 0, group, out);
                continue;
            }

            // Status byte (0x80-0xF7)
            if (b >= 0x80)
            {
                if (b == 0xF0) // SYX_START
                {
                    // SysEx start: discard any partial channel message.
                    data_count_      = 0;
                    running_status_  = 0;
                    in_sysex_        = true;
                    sysex_started_   = false;
                    sysex_buf_count_ = 0;
                }
                else if (b == 0xF7) // SYX_END
                {
                    // SysEx end: emit end/complete with whatever bytes remain
                    // (0-6). A 0-byte "end" is valid UMP and maps to just F7
                    // on the wire for MIDI 1.0 devices.
                    if (in_sysex_)
                    {
                        const uint8_t sn = sysex_started_ ? 0x3u : 0x0u;
                        emit_sysex_packet(sn, sysex_buf_count_, group, out);
                    }
                    in_sysex_        = false;
                    sysex_started_   = false;
                    sysex_buf_count_ = 0;
                    running_status_  = 0;
                    data_count_      = 0;
                }
                else if (b == 0xF4 || b == 0xF5)
                {
                    // Undefined System Common: ignore, no state change.
                }
                else if (b >= 0xF1 && b <= 0xF6)
                {
                    // System Common: cancels running status and SysEx.
                    in_sysex_        = false;
                    sysex_started_   = false;
                    sysex_buf_count_ = 0;
                    data_count_      = 0;
                    const int dlen  = sys_common_data_len(b);
                    if (dlen == 0)
                    {
                        emit_type1(b, 0, 0, group, out);
                        running_status_ = 0;
                    }
                    else
                    {
                        running_status_ = b;
                        data_needed_    = dlen;
                    }
                }
                else
                {
                    // Channel status (0x80-0xEF): set/update running status.
                    // Cancels in-progress SysEx per MIDI spec.
                    in_sysex_        = false;
                    sysex_started_   = false;
                    sysex_buf_count_ = 0;
                    running_status_  = b;
                    data_needed_     = channel_data_len(b);
                    data_count_      = 0;
                }
                continue;
            }

            // Data byte (0x00-0x7F)
            if (in_sysex_)
            {
                // Flush a full 6-byte group immediately as start or continue.
                if (sysex_buf_count_ == 6)
                {
                    emit_sysex_packet(sysex_started_ ? 0x2u : 0x1u, 6, group, out);
                    sysex_started_   = true;
                    sysex_buf_count_ = 0;
                }
                sysex_buf_[sysex_buf_count_++] = b;
                continue;
            }

            if (running_status_ == 0)
                continue;  // stray data byte - ignore

            data_buf_[data_count_++] = b;
            if (data_count_ >= data_needed_)
            {
                const uint8_t d1 = data_buf_[0];
                const uint8_t d2 = (data_needed_ >= 2) ? data_buf_[1] : 0;
                if (running_status_ < 0xF0)
                    emit_type2(running_status_, d1, d2, group, out);
                else
                    emit_type1(running_status_, d1, d2, group, out);
                data_count_ = 0;
                // System common does not support running status.
                if (running_status_ >= 0xF0)
                    running_status_ = 0;
            }
        } // end byte loop

        // Flush any pending partial SysEx bytes so that each send_buffer call
        // produces output proportional to its input rather than accumulating
        // silently until F7 arrives in a later chunk.
        if (in_sysex_ && sysex_buf_count_ > 0)
        {
            emit_sysex_packet(sysex_started_ ? 0x2u : 0x1u,
                              sysex_buf_count_, group, out);
            sysex_started_   = true;
            sysex_buf_count_ = 0;
        }
    }

    //! True between an F0 and the status byte that ends that SysEx.
    bool in_sysex() const { return in_sysex_; }

    //! Discard all accumulated state (call on port close / error recovery).
    void reset()
    {
        in_sysex_        = false;
        sysex_started_   = false;
        sysex_buf_count_ = 0;
        running_status_  = 0;
        data_needed_     = 0;
        data_count_      = 0;
    }

private:
    static int channel_data_len(uint8_t status) noexcept
    {
        const uint8_t type = status >> 4;
        return (type == 0xC || type == 0xD) ? 1 : 2;
    }

    static int sys_common_data_len(uint8_t status) noexcept
    {
        switch (status)
        {
        case 0xF1: return 1;  // MTC Quarter Frame
        case 0xF2: return 2;  // Song Position Pointer
        case 0xF3: return 1;  // Song Select
        default:   return 0;  // 0xF4, 0xF5 (undefined), 0xF6 (Tune Request)
        }
    }

    // Type-2: MIDI 1.0 Channel Voice (32-bit UMP)
    static void emit_type2(uint8_t status, uint8_t d1, uint8_t d2,
                            uint8_t group, std::vector<uint32_t>& out)
    {
        out.push_back((0x2u << 28)
                    | (static_cast<uint32_t>(group)  << 24)
                    | (static_cast<uint32_t>(status) << 16)
                    | (static_cast<uint32_t>(d1)     <<  8)
                    |  static_cast<uint32_t>(d2));
    }

    // Type-1: System Common / Realtime (32-bit UMP)
    static void emit_type1(uint8_t status, uint8_t d1, uint8_t d2,
                            uint8_t group, std::vector<uint32_t>& out)
    {
        out.push_back((0x1u << 28)
                    | (static_cast<uint32_t>(group)  << 24)
                    | (static_cast<uint32_t>(status) << 16)
                    | (static_cast<uint32_t>(d1)     <<  8)
                    |  static_cast<uint32_t>(d2));
    }

    // Type-3: SysEx Data (64-bit UMP pair).
    // sn: 0x0=complete  0x1=start  0x2=continue  0x3=end
    // count: number of valid payload bytes (0-6), read from sysex_buf_.
    void emit_sysex_packet(uint8_t sn, int count, uint8_t group,
                           std::vector<uint32_t>& out)
    {
        uint8_t b[6] = {};
        for (int j = 0; j < count; ++j)
            b[j] = sysex_buf_[j];
        out.push_back((0x3u << 28)
                    | (static_cast<uint32_t>(group) << 24)
                    | (static_cast<uint32_t>(sn)    << 20)
                    | (static_cast<uint32_t>(count) << 16)
                    | (static_cast<uint32_t>(b[0])  <<  8)
                    |  static_cast<uint32_t>(b[1]));
        out.push_back((static_cast<uint32_t>(b[2]) << 24)
                    | (static_cast<uint32_t>(b[3]) << 16)
                    | (static_cast<uint32_t>(b[4]) <<  8)
                    |  static_cast<uint32_t>(b[5]));
    }

    bool    in_sysex_        = false;
    bool    sysex_started_   = false;  // emitted at least one start/continue packet
    uint8_t sysex_buf_[6]    = {};     // pending SysEx bytes not yet emitted
    int     sysex_buf_count_ = 0;      // 0-6
    uint8_t running_status_  = 0;
    int     data_needed_     = 0;
    int     data_count_      = 0;
    uint8_t data_buf_[2]     = {};
};

// Number of 32-bit words in the UMP whose first word is |word0|.
static uint32_t ump_packet_words(uint32_t word0)
{
    static const uint8_t sizes[16] = { 1, 1, 1, 2, 2, 4, 1, 1, 2, 2, 2, 3, 3, 4, 4, 4 };
    return sizes[word0 >> 28];
}

static std::string wstring_to_utf8(const std::wstring_view wstr)
{
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.data(),
                                  static_cast<int>(wstr.size()),
                                  nullptr, 0, nullptr, nullptr);
    std::string out(len, '\0');
    if (len)
        WideCharToMultiByte(CP_UTF8, 0, wstr.data(),
                            static_cast<int>(wstr.size()),
                            out.data(), len, nullptr, nullptr);
    return out;
}

struct WinMidiPort
{
    std::string  display_name;    // UTF-8 for RtMidi consumers
    std::wstring device_id;       // EndpointDeviceId for SDK calls
    uint8_t      group_index = 0; // UMP group (0-15); maps to traditional port number
    std::string  warning;         // reported once per object, empty in the normal case
};

// Used only when an endpoint publishes no MIDI 1.0 ports of its own; see
// wms_legacy_ports(). Windows composes a port name as "<endpoint> <port>"
// unless the port name already carries the endpoint name, and this follows it.
static std::string wms_port_display_name(const std::string& endpoint,
                                         const std::string& block)
{
    if (block.empty())
        return endpoint;
    if (endpoint.empty())
        return block;

    auto lowered = [](std::string s) {
        for (std::size_t i = 0; i < s.size(); ++i)
            s[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(s[i])));
        return s;
    };

    if (lowered(block).find(lowered(endpoint)) != std::string::npos)
        return block;
    return endpoint + " " + block;
}

// Takes the endpoint's MIDI 1.0 ports as the service publishes them, in its own
// port order, and returns how many were found.
//
// These are the names the user already sees: the same strings WinMM reports and
// MIDI Settings shows, with the endpoint and port names combined, duplicates
// disambiguated, and any name the user assigned honoured. Windows composes them
// from the device's blocks by a documented algorithm that has more to it than a
// library should reimplement - naming style is a per-system and per-endpoint
// setting, and custom names are published verbatim. Asking for the result keeps
// RtMidi's port names identical to every other MIDI 1.0 view of the machine.
// https://microsoft.github.io/MIDI/kb/how-midi1-port-names-are-generated/
static std::size_t wms_legacy_ports(std::wstring const& device_id,
                                    Midi1PortFlow flow,
                                    std::vector<WinMidiPort>& target)
{
    std::vector<std::pair<uint32_t, WinMidiPort>> found;   // keyed by port number

    try
    {
        auto ports = MidiLegacyPortDeviceInformation::FindAllForAssociatedEndpoint(
            winrt::hstring(device_id), flow);
        for (auto const& port : ports)
        {
            WinMidiPort p;
            p.display_name = wstring_to_utf8(static_cast<std::wstring_view>(port.Name()));
            p.device_id    = device_id;
            p.group_index  = port.Group().Index();
            found.emplace_back(port.Number(), std::move(p));
        }
    }
    catch (winrt::hresult_error const&)
    {
        return 0;   // no MIDI 1.0 view of this endpoint; the caller falls back
    }

    std::sort(found.begin(), found.end(),
        [](std::pair<uint32_t, WinMidiPort> const& a, std::pair<uint32_t, WinMidiPort> const& b) {
            return a.first < b.first;
        });

    for (auto& entry : found)
        target.push_back(std::move(entry.second));

    return found.size();
}

// Turns one endpoint into RtMidi input and output ports, preferring the MIDI 1.0
// ports the service publishes and falling back to its group terminal blocks for
// an endpoint that has none - a MIDI 2.0 endpoint with no MIDI 1.0 representation.
static void wms_parse_endpoint(MidiEndpointDeviceInformation const& ep,
                               std::vector<WinMidiPort>& in_ports,
                               std::vector<WinMidiPort>& out_ports,
                               bool* used_blocks = nullptr)
{
    if (used_blocks) *used_blocks = false;
    std::string  ep_name   = wstring_to_utf8(static_cast<std::wstring_view>(ep.Name()));
    std::wstring device_id = static_cast<std::wstring>(ep.EndpointDeviceId());

    // A source sends to the host, which is an RtMidi input.
    std::vector<WinMidiPort> published_in, published_out;
    wms_legacy_ports(device_id, Midi1PortFlow::MidiMessageSource, published_in);
    wms_legacy_ports(device_id, Midi1PortFlow::MidiMessageDestination, published_out);

    auto gtbs = ep.GetGroupTerminalBlocks();

    if (gtbs.Size() == 0)
    {
        // Nothing to describe the endpoint's shape, so the published list is all
        // there is; failing that, a single port at group 0 for both directions.
        if (!published_in.empty() || !published_out.empty())
        {
            in_ports  = std::move(published_in);
            out_ports = std::move(published_out);
            return;
        }
        if (used_blocks) *used_blocks = true;
        WinMidiPort p;
        p.display_name = ep_name;
        p.device_id    = device_id;
        p.group_index  = 0;
        in_ports.push_back(p);
        out_ports.push_back(p);
        return;
    }

    // Separate GTBs by direction.
    //   BlockOutput (device->host) = RtMidi IN
    //   BlockInput  (host->device) = RtMidi OUT
    //   Bidirectional             = both
    std::vector<MidiGroupTerminalBlock> in_gtbs, out_gtbs;
    for (auto const& gtb : gtbs)
    {
        auto dir = gtb.Direction();
        if (dir == MidiGroupTerminalBlockDirection::Bidirectional ||
            dir == MidiGroupTerminalBlockDirection::BlockOutput)
            in_gtbs.push_back(gtb);
        if (dir == MidiGroupTerminalBlockDirection::Bidirectional ||
            dir == MidiGroupTerminalBlockDirection::BlockInput)
            out_gtbs.push_back(gtb);
    }

    // Sort + expand a GTB list into port entries.
    auto expand = [&](std::vector<MidiGroupTerminalBlock>& matching, std::vector<WinMidiPort>& target)
    {
        if (matching.empty()) return;

        std::sort(matching.begin(), matching.end(),
            [](MidiGroupTerminalBlock const& a, MidiGroupTerminalBlock const& b) {
                return a.FirstGroup().Index() < b.FirstGroup().Index();
            });

        uint8_t total_groups = 0;
        for (auto const& gtb : matching) total_groups += gtb.GroupCount();

        if (total_groups <= 1)
        {
            std::string gtb_name = wstring_to_utf8(
                static_cast<std::wstring_view>(matching[0].Name()));
            WinMidiPort p;
            p.display_name = wms_port_display_name(ep_name, gtb_name);
            p.device_id    = device_id;
            p.group_index  = matching[0].FirstGroup().Index();
            target.push_back(std::move(p));
        }
        else
        {
            for (auto const& gtb : matching)
            {
                std::string gtb_name = wstring_to_utf8(
                    static_cast<std::wstring_view>(gtb.Name()));
                uint8_t first = gtb.FirstGroup().Index();
                uint8_t count = gtb.GroupCount();
                for (uint8_t g = 0; g < count; ++g)
                {
                    WinMidiPort p;
                    p.display_name = wms_port_display_name(ep_name, gtb_name);
                    p.device_id    = device_id;
                    p.group_index  = first + g;
                    target.push_back(std::move(p));
                }
            }
        }
    };

    std::vector<WinMidiPort> block_in, block_out;
    expand(in_gtbs, block_in);
    expand(out_gtbs, block_out);

    // The published list is preferred, but it only covers the groups the service
    // knows about. If it covers fewer than the endpoint itself describes, some of
    // the device is unreachable, so use the endpoint's own blocks and tell the
    // application why its port names may differ from other MIDI 1.0 software.
    // Compared by the groups they reach, not by how many entries they have: one
    // group may be described by more than one block, which is not a shortfall.
    auto groups_of = [](std::vector<WinMidiPort> const& ports)
    {
        std::vector<uint8_t> groups;
        for (WinMidiPort const& p : ports) groups.push_back(p.group_index);
        std::sort(groups.begin(), groups.end());
        groups.erase(std::unique(groups.begin(), groups.end()), groups.end());
        return groups;
    };

    auto choose = [&](std::vector<WinMidiPort>& published,
                      std::vector<WinMidiPort>& blocks,
                      std::vector<WinMidiPort>& target,
                      const char* direction)
    {
        const std::vector<uint8_t> pub_groups = groups_of(published);
        const std::vector<uint8_t> blk_groups = groups_of(blocks);

        std::vector<uint8_t> missing;
        std::set_difference(blk_groups.begin(), blk_groups.end(),
                            pub_groups.begin(), pub_groups.end(),
                            std::back_inserter(missing));

        if (!published.empty() && missing.empty())
        {
            target = std::move(published);
            return;
        }

        if (used_blocks) *used_blocks = true;

        if (!published.empty())
        {
            std::string note = "Windows MIDI Services lists no MIDI 1.0 " + std::string(direction) +
                " port for group" + (missing.size() > 1 ? "s " : " ");
            for (std::size_t i = 0; i < missing.size(); ++i)
                note += (i ? ", " : "") + std::to_string(missing[i]);
            note += " of \"" + ep_name + "\", which the endpoint describes. Using the endpoint's own "
                "blocks, so these port names may differ from the ones other MIDI 1.0 applications show.";
            for (WinMidiPort& p : blocks) p.warning = note;
        }
        target = std::move(blocks);
    };

    choose(published_in,  block_in,  in_ports,  "input");
    choose(published_out, block_out, out_ports, "output");
}

class WinMidiServicesClass;
static void wms_endpoint_removed(std::wstring const& device_id);

// ---------------------------------------------------------------------------
// Endpoint list shared by all MidiInWinMidi2 / MidiOutWinMidi2 objects.
//
// One MidiEndpointDeviceWatcher keeps the port lists current as endpoints are
// added, updated and removed, so getPortCount() never has to enumerate. The
// first backend object starts it (initial enumeration takes about half a
// second) and it keeps running, so later objects are created without that
// delay. Its handlers are revoked during static destruction (process exit, or
// unloading a DLL that links RtMidi); see ~WinMidiEndpointCache().
// ---------------------------------------------------------------------------
class WinMidiEndpointCache
{
public:
    // Returns the running cache, starting it if needed, or nullptr if the
    // Windows MIDI Services runtime is unavailable.
    static std::shared_ptr<WinMidiEndpointCache> acquire();

    ~WinMidiEndpointCache();

    std::vector<WinMidiPort> ports(bool for_input);
    void recheck_provisional();

private:
    struct Endpoint
    {
        std::wstring             device_id;
        std::vector<WinMidiPort> in_ports;
        std::vector<WinMidiPort> out_ports;

        // True when the ports came from the endpoint's own blocks because the
        // service had published no MIDI 1.0 ports for some group. A device that
        // has just been plugged in is announced before its ports are published,
        // so this is re-checked rather than left as the endpoint's final shape.
        bool provisional = false;

        // When this endpoint first went provisional and stayed that way, and
        // whether it turned up after the library was already running. Together
        // they keep the warning quiet while a device that has just been plugged
        // in settles, which normally takes about a second.
        std::chrono::steady_clock::time_point provisional_since{};
        bool appeared_while_running = false;
    };

    // Data written by the watcher callbacks. Each callback holds its own
    // reference, so a callback still running while the watcher stops never
    // touches freed memory.
    struct State
    {
        std::mutex              mtx;
        std::condition_variable enumerated_cv;
        bool                    enumerated = false;
        std::vector<Endpoint>   endpoints;

        void set_endpoint(MidiEndpointDeviceInformation const& info, bool add_if_missing);
        void remove_endpoint(std::wstring const& device_id);
    };

    WinMidiEndpointCache() = default;
    bool start();
    void revoke_handlers();

    std::shared_ptr<State>    state_ = std::make_shared<State>();
    MidiEndpointDeviceWatcher watcher_{ nullptr };
    winrt::event_token        tok_added_{};
    winrt::event_token        tok_updated_{};
    winrt::event_token        tok_removed_{};
    winrt::event_token        tok_enumerated_{};
};

std::shared_ptr<WinMidiEndpointCache> WinMidiEndpointCache::acquire()
{
    static std::mutex mtx;
    static std::shared_ptr<WinMidiEndpointCache> instance;

    std::lock_guard<std::mutex> lock(mtx);
    if (instance)
        return instance;
    if (!wms_runtime_available())
        return nullptr;

    std::shared_ptr<WinMidiEndpointCache> cache(new WinMidiEndpointCache());
    if (!cache->start())
        return nullptr;
    instance = cache;
    return cache;
}

// The cache is destroyed during static destruction, which for a DLL (including
// rtmidi.dll itself) runs under the loader lock. Releasing the watcher there
// is fatal: the system DeviceWatcher behind it calls CoDecrementMTAUsage(),
// which fails fast under the loader lock. So only the handlers are revoked,
// which keeps callbacks from reaching this code after it is unloaded, and the
// watcher is deliberately leaked.
WinMidiEndpointCache::~WinMidiEndpointCache()
{
    revoke_handlers();
    if (watcher_)
        static_cast<void>(winrt::detach_abi(watcher_));
}

bool WinMidiEndpointCache::start()
{
    try
    {
        watcher_ = MidiEndpointDeviceWatcher::Create(
            MidiEndpointDeviceInformationFilters::StandardNativeMidi1ByteFormat
            | MidiEndpointDeviceInformationFilters::StandardNativeUniversalMidiPacketFormat
            | MidiEndpointDeviceInformationFilters::VirtualDeviceResponder);

        std::shared_ptr<State> state = state_;

        tok_added_ = watcher_.Added(
            [state](MidiEndpointDeviceWatcher const&,
                    MidiEndpointDeviceInformationAddedEventArgs const& args)
            {
                state->set_endpoint(args.AddedDevice(), true);
            });

        // An endpoint's name and group terminal blocks can change after it
        // first appears, for example when UMP endpoint discovery completes.
        tok_updated_ = watcher_.Updated(
            [state](MidiEndpointDeviceWatcher const&,
                    MidiEndpointDeviceInformationUpdatedEventArgs const& args)
            {
                // Added when missing, not only updated: a device that is
                // unplugged and plugged in again is already known to Windows, so
                // its return arrives as an update. Ignoring those left the
                // endpoint out of the list for the life of the process. The
                // watcher's filters decide what reaches here, so anything that
                // does belongs in the list.
                auto info = args.UpdatedDevice();
                if (info)
                    state->set_endpoint(info, true);
            });

        tok_removed_ = watcher_.Removed(
            [state](MidiEndpointDeviceWatcher const&,
                    MidiEndpointDeviceInformationRemovedEventArgs const& args)
            {
                auto removed = args.RemovedDevice();
                if (!removed)
                    return;
                const std::wstring device_id = static_cast<std::wstring>(removed.EndpointDeviceId());
                state->remove_endpoint(device_id);
                wms_endpoint_removed(device_id);
            });

        tok_enumerated_ = watcher_.EnumerationCompleted(
            [state](MidiEndpointDeviceWatcher const&,
                    winrt::Windows::Foundation::IInspectable const&)
            {
                {
                    std::lock_guard<std::mutex> lock(state->mtx);
                    state->enumerated = true;
                }
                state->enumerated_cv.notify_all();
            });

        watcher_.Start();
    }
    catch (winrt::hresult_error const&)
    {
        // Not under the loader lock here, so the watcher can be released.
        revoke_handlers();
        watcher_ = nullptr;
        return false;
    }

    // Wait (at most 2 s) for the initial enumeration, so the first
    // getPortCount() already sees the endpoints that are present.
    std::unique_lock<std::mutex> lock(state_->mtx);
    state_->enumerated_cv.wait_for(lock, std::chrono::seconds(2),
                                   [this] { return state_->enumerated; });
    return true;
}

void WinMidiEndpointCache::revoke_handlers()
{
    if (!watcher_)
        return;
    if (tok_added_)      watcher_.Added(tok_added_);
    if (tok_updated_)    watcher_.Updated(tok_updated_);
    if (tok_removed_)    watcher_.Removed(tok_removed_);
    if (tok_enumerated_) watcher_.EnumerationCompleted(tok_enumerated_);
    tok_added_ = tok_updated_ = tok_removed_ = tok_enumerated_ = {};
}

// Re-reads any endpoint whose ports came from its blocks. A device is announced
// as soon as Windows knows of it, which can be before the service has published
// its MIDI 1.0 ports, and no further notification follows once they appear: an
// endpoint parsed in that moment would otherwise keep its fallback port names
// for the life of the process, so a device that was unplugged and plugged in
// again would come back under different names than every other application
// shows. Only endpoints still marked provisional are re-read, so this costs
// nothing once they settle.
void WinMidiEndpointCache::recheck_provisional()
{
    std::vector<std::wstring> pending;
    {
        std::lock_guard<std::mutex> lock(state_->mtx);
        for (Endpoint const& ep : state_->endpoints)
            if (ep.provisional) pending.push_back(ep.device_id);
    }

    for (std::wstring const& id : pending)
    {
        try
        {
            auto info = MidiEndpointDeviceInformation::CreateFromEndpointDeviceId(winrt::hstring(id));
            if (info)
                state_->set_endpoint(info, false);
        }
        catch (winrt::hresult_error const&)
        {
        }
    }
}

std::vector<WinMidiPort> WinMidiEndpointCache::ports(bool for_input)
{
    recheck_provisional();

    // A device that has just appeared is briefly missing its published ports,
    // which is not worth telling anyone about: hold the warning back until the
    // endpoint has stayed that way longer than settling takes.
    const auto quiet_for = std::chrono::seconds(5);
    const auto now       = std::chrono::steady_clock::now();

    std::vector<WinMidiPort> result;
    std::lock_guard<std::mutex> lock(state_->mtx);
    for (auto const& ep : state_->endpoints)
    {
        auto const& list = for_input ? ep.in_ports : ep.out_ports;
        const bool settling = ep.provisional && ep.appeared_while_running &&
                              ( now - ep.provisional_since ) < quiet_for;
        for (WinMidiPort const& p : list)
        {
            result.push_back(p);
            if (settling) result.back().warning.clear();
        }
    }
    return result;
}

void WinMidiEndpointCache::State::set_endpoint(
    MidiEndpointDeviceInformation const& info, bool add_if_missing)
{
    Endpoint ep;
    ep.device_id = static_cast<std::wstring>(info.EndpointDeviceId());
    wms_parse_endpoint(info, ep.in_ports, ep.out_ports, &ep.provisional);

    std::lock_guard<std::mutex> lock(mtx);

    // Only an endpoint that turns up after the first enumeration is settling.
    // One that was already there when the library started has had all the time
    // it needs, so a short port list on it is worth saying out loud at once.
    ep.provisional_since       = std::chrono::steady_clock::now();
    ep.appeared_while_running  = enumerated;

    auto it = std::find_if(endpoints.begin(), endpoints.end(),
        [&ep](Endpoint const& e) { return e.device_id == ep.device_id; });
    if (it != endpoints.end())
    {
        // Keep the original moment, so an endpoint that has been provisional
        // for a while does not look new after every re-check.
        if (it->provisional && ep.provisional)
        {
            ep.provisional_since      = it->provisional_since;
            ep.appeared_while_running = it->appeared_while_running;
        }
        *it = std::move(ep);
    }
    else if (add_if_missing)
        endpoints.push_back(std::move(ep));
}

void WinMidiEndpointCache::State::remove_endpoint(std::wstring const& device_id)
{
    std::lock_guard<std::mutex> lock(mtx);
    endpoints.erase(std::remove_if(endpoints.begin(), endpoints.end(),
        [&device_id](Endpoint const& e) { return e.device_id == device_id; }),
        endpoints.end());
}

// ---------------------------------------------------------------------------
// Helper class (analogous to UWPMidiClass)
// ---------------------------------------------------------------------------
struct WinMidiSharedConnection;
struct WinMidiReceiveGate;

class WinMidiServicesClass
{
public:
    explicit WinMidiServicesClass(MidiApi& midi_api)
        : midi_api_(midi_api)
    {}

    ~WinMidiServicesClass()
    {
        close();
    }

    // Attaches to the shared endpoint cache and loads the port list. Pass the
    // input data for an input object and nullptr for an output object.
    // Returns false if the Windows MIDI Services runtime is unavailable.
    bool init(MidiInApi::RtMidiInData* input_data);
    bool is_ready() const { return cache_ != nullptr; }

    // Reloads the port list from the endpoint cache.
    void refresh_ports();
    size_t get_num_ports() const { return ports_.size(); }
    std::string get_port_name(size_t n) const { return ports_[n].display_name; }

    // Anything the port list needs to tell the application, each message once
    // per object so a repeated getPortCount() does not repeat it.
    std::vector<std::string> take_new_warnings()
    {
        std::vector<std::string> fresh;
        for (WinMidiPort const& p : ports_)
        {
            if (p.warning.empty()) continue;
            if (std::find(warned_.begin(), warned_.end(), p.warning) != warned_.end()) continue;
            warned_.push_back(p.warning);
            fresh.push_back(p.warning);
        }
        return fresh;
    }

    bool open(size_t port_number);
    void close();

    bool send_buffer(const unsigned char* buf, size_t len);

    // UMP -> raw MIDI 1.0 bytes: fires the RtMidi callback or queue. Called by
    // the shared connection's handler for every message on the endpoint.
    void midi_in_callback(MidiMessageReceivedEventArgs const& args);

    std::mutex mtx_open_close_;
    std::mutex mtx_queue_;

    // Names the process's session if this object opens the first port.
    std::string client_name_;

private:
    MidiApi& midi_api_;

    MidiInApi::RtMidiInData* input_data_{ nullptr };

    std::shared_ptr<WinMidiEndpointCache> cache_;
    std::vector<WinMidiPort> ports_;
    std::vector<std::string> warned_;

    // Set while a port is open: the endpoint's shared connection, and for an
    // input port, its registration as a receiver on that connection.
    std::shared_ptr<WinMidiSharedConnection> shared_;
    std::shared_ptr<WinMidiReceiveGate>      gate_;

    // Active UMP group for the open port
    uint8_t active_group_{ 0 };

    // Stateful MIDI 1.0 -> UMP encoder (persists running status and SysEx
    // framing across send_buffer calls).
    Midi1UmpEncoder encoder_;

    // Input timestamps, in MidiClock ticks
    uint64_t timestamp_frequency_{ 10000000u };
    uint64_t last_timestamp_{ 0 };
    bool first_message_{ true };

    // SysEx 7 reassembly buffer: accumulates Start/Continue fragments
    // until the End packet arrives, then dispatches a single complete message.
    std::vector<uint8_t> sysex_buf_;
};

// ---------------------------------------------------------------------------
// One session, and one connection per endpoint, for the whole process.
//
// Windows MIDI Services wants a single MidiEndpointConnection per endpoint,
// shared by every port object that refers to it (Microsoft's guidance for
// porting MIDI libraries). It is not only a matter of resources: a process that
// holds two connections to one endpoint, for example one for input and one for
// output, and disconnects the one carrying input stops receiving from that
// endpoint, and a connection it opens afterwards never receives either.
//
// So every RtMidi port on an endpoint shares one connection. Output ports send
// through it; input ports register a receiver, and the connection's single
// MessageReceived handler hands each message to every receiver. The connection
// is disconnected when the last port using it closes, or when the endpoint is
// removed, and the next port to open the endpoint gets a new one.
//
// The session is created by the first port opened and named after that
// object's client name.
// ---------------------------------------------------------------------------

// Lets an input port stop message delivery safely. The shared handler holds its
// own reference, so a message arriving after close() finds receiver == nullptr
// instead of a deleted object, and close() waits on mtx for a message that is
// being delivered. mtx is recursive so that a user callback can call closePort().
struct WinMidiReceiveGate
{
    std::recursive_mutex  mtx;
    WinMidiServicesClass* receiver = nullptr;
};

struct WinMidiSharedConnection
{
    std::wstring device_id;

    // connection and raw are used and cleared under send_mtx.
    std::mutex             send_mtx;
    MidiEndpointConnection connection{ nullptr };
#ifdef RTMIDI_USE_WMS_COM_RAW
    winrt::com_ptr<IMidiEndpointConnectionRaw> raw;
#endif
    winrt::event_token     token{};

    // The number of UMP words one send may carry, asked of the service once the
    // connection is open, since it is documented as liable to change. 684
    // (MAXIMUM_LOOPED_UMP_DATASIZE / sizeof(uint32_t) == 2736 / 4) is used only
    // when the service does not answer; max_words_from_service says which.
    uint32_t max_words_per_call{ 684u };
    bool     max_words_from_service{ false };

    std::mutex receivers_mtx;
    std::vector<std::shared_ptr<WinMidiReceiveGate>> receivers;

    // The thread delivering a message, so that a port closed from inside a
    // callback can be recognized. 0 while no message is being delivered.
    std::atomic<DWORD> dispatch_thread{ 0 };

    // Ports using this connection. Guarded by the pool's mutex.
    int users{ 0 };
};

class WinMidiConnectionPool
{
public:
    // Constructs the pool's state. Called when a backend object is created, so
    // the state outlives any static RtMidi object in the program.
    static void touch() { state(); }

    // Returns the endpoint's shared connection with one more user counted,
    // opening it if no port uses it yet. On failure returns nullptr and sets
    // error.
    static std::shared_ptr<WinMidiSharedConnection> acquire(std::wstring const& device_id,
                                                            std::string const& session_name,
                                                            std::string& error);

    // Counts one user fewer, and disconnects when none remain.
    static void release(std::shared_ptr<WinMidiSharedConnection> const& shared);

    // Disconnects the connection to an endpoint that has been removed. Ports
    // still holding it send and receive nothing until they are reopened.
    static void endpoint_removed(std::wstring const& device_id);

private:
    struct State
    {
        std::mutex  mtx;
        MidiSession session{ nullptr };
        std::vector<std::shared_ptr<WinMidiSharedConnection>> connections;
        bool        shut_down{ false };

        ~State();
    };

    static State& state();
    static void disconnect(MidiSession const& session,
                           std::shared_ptr<WinMidiSharedConnection> const& shared);
};

WinMidiConnectionPool::State& WinMidiConnectionPool::state()
{
    static State s;
    return s;
}

// Runs during static destruction, which for a DLL (including rtmidi.dll itself)
// happens under the loader lock; see ~WinMidiEndpointCache(). The handlers are
// revoked so no message reaches code that is being unloaded, and the session
// and connections are deliberately leaked rather than released there.
WinMidiConnectionPool::State::~State()
{
    std::lock_guard<std::mutex> lock(mtx);
    shut_down = true;
    for (auto const& shared : connections)
    {
        std::lock_guard<std::mutex> send_lock(shared->send_mtx);
        if (!shared->connection)
            continue;
        if (shared->token)
            shared->connection.MessageReceived(shared->token);
#ifdef RTMIDI_USE_WMS_COM_RAW
        if (shared->raw)
            static_cast<void>(shared->raw.detach());
#endif
        static_cast<void>(winrt::detach_abi(shared->connection));
    }
    connections.clear();
    if (session)
        static_cast<void>(winrt::detach_abi(session));
}

std::shared_ptr<WinMidiSharedConnection> WinMidiConnectionPool::acquire(
    std::wstring const& device_id, std::string const& session_name, std::string& error)
{
    State& s = state();
    std::lock_guard<std::mutex> lock(s.mtx);

    for (auto const& shared : s.connections)
    {
        if (shared->device_id == device_id)
        {
            ++shared->users;
            return shared;
        }
    }

    try
    {
        if (!s.session)
        {
            s.session = MidiSession::Create(winrt::to_hstring(
                session_name.empty() ? std::string("RtMidi") : session_name));
            if (!s.session)
            {
                error = "could not create a Windows MIDI Services session.";
                return nullptr;
            }
        }

        auto shared = std::make_shared<WinMidiSharedConnection>();
        shared->device_id  = device_id;
        shared->connection = s.session.CreateEndpointConnection(winrt::hstring(device_id));
        if (!shared->connection)
        {
            error = "could not create a connection to the endpoint.";
            return nullptr;
        }

        // The handler is attached before Open(), as the SDK requires, and holds
        // only a weak reference so it does not keep the connection alive.
        std::weak_ptr<WinMidiSharedConnection> weak = shared;
        shared->token = shared->connection.MessageReceived(
            [weak](IMidiMessageReceivedEventSource const&, MidiMessageReceivedEventArgs const& args)
            {
                auto target = weak.lock();
                if (!target)
                    return;

                std::vector<std::shared_ptr<WinMidiReceiveGate>> gates;
                {
                    std::lock_guard<std::mutex> receivers_lock(target->receivers_mtx);
                    gates = target->receivers;
                }

                struct DispatchScope
                {
                    std::atomic<DWORD>& thread;
                    ~DispatchScope() { thread = 0; }
                } scope{ target->dispatch_thread };
                target->dispatch_thread = GetCurrentThreadId();

                for (auto const& gate : gates)
                {
                    std::lock_guard<std::recursive_mutex> gate_lock(gate->mtx);
                    if (gate->receiver)
                        gate->receiver->midi_in_callback(args);
                }
            });

        if (!shared->connection.Open())
        {
            shared->connection.MessageReceived(shared->token);
            s.session.DisconnectEndpointConnection(shared->connection.ConnectionId());
            error = "could not open the connection to the endpoint.";
            return nullptr;
        }

        try
        {
            const uint32_t words = shared->connection.GetSupportedMaxMidiWordsPerTransmission();
            if (words > 0)
            {
                shared->max_words_per_call     = words;
                shared->max_words_from_service = true;
            }
        }
        catch (winrt::hresult_error const&)
        {
        }

#ifdef RTMIDI_USE_WMS_COM_RAW
        // The COM extension interface, for zero-allocation sends.
        shared->raw = shared->connection.as<IMidiEndpointConnectionRaw>();
#endif

        shared->users = 1;
        s.connections.push_back(shared);
        return shared;
    }
    catch (winrt::hresult_error const& ex)
    {
        std::ostringstream ss;
        ss << "HRESULT 0x" << std::hex << static_cast<uint32_t>(ex.code()) << " "
           << wstring_to_utf8(static_cast<std::wstring_view>(ex.message()));
        error = ss.str();
        return nullptr;
    }
}

void WinMidiConnectionPool::release(std::shared_ptr<WinMidiSharedConnection> const& shared)
{
    if (!shared)
        return;

    State& s = state();
    MidiSession session{ nullptr };
    {
        std::lock_guard<std::mutex> lock(s.mtx);
        if (s.shut_down || --shared->users > 0)
            return;
        s.connections.erase(std::remove(s.connections.begin(), s.connections.end(), shared),
                            s.connections.end());
        session = s.session;
    }
    disconnect(session, shared);
}

void WinMidiConnectionPool::endpoint_removed(std::wstring const& device_id)
{
    State& s = state();
    std::shared_ptr<WinMidiSharedConnection> gone;
    MidiSession session{ nullptr };
    {
        std::lock_guard<std::mutex> lock(s.mtx);
        if (s.shut_down)
            return;
        auto it = std::find_if(s.connections.begin(), s.connections.end(),
            [&device_id](std::shared_ptr<WinMidiSharedConnection> const& c) { return c->device_id == device_id; });
        if (it == s.connections.end())
            return;
        gone = *it;
        s.connections.erase(it);
        session = s.session;
    }
    disconnect(session, gone);
}

// Revokes the handler and disconnects. Disconnecting waits for the connection's
// receive thread to finish, so when this runs on that thread (a port closed from
// inside a callback) the work is handed to another thread and completes once the
// callback returns.
void WinMidiConnectionPool::disconnect(MidiSession const& session,
                                       std::shared_ptr<WinMidiSharedConnection> const& shared)
{
    MidiEndpointConnection connection{ nullptr };
    winrt::event_token token{};
    {
        std::lock_guard<std::mutex> lock(shared->send_mtx);
        connection = shared->connection;
        shared->connection = nullptr;
        token = shared->token;
        shared->token = {};
#ifdef RTMIDI_USE_WMS_COM_RAW
        shared->raw = nullptr;
#endif
    }
    if (!connection)
        return;

    auto work = [session, connection, token]()
    {
        try
        {
            if (token)
                connection.MessageReceived(token);
            if (session)
                session.DisconnectEndpointConnection(connection.ConnectionId());
        }
        catch (winrt::hresult_error const&)
        {
        }
    };

    if (shared->dispatch_thread == GetCurrentThreadId())
    {
        try
        {
            std::thread(work).detach();
        }
        catch (std::system_error const&)
        {
        }
    }
    else
    {
        work();
    }
}

static void wms_endpoint_removed(std::wstring const& device_id)
{
    WinMidiConnectionPool::endpoint_removed(device_id);
}

// ---------------------------------------------------------------------------
bool WinMidiServicesClass::init(MidiInApi::RtMidiInData* input_data)
{
    input_data_ = input_data;
    WinMidiConnectionPool::touch();
    if (!cache_)
        cache_ = WinMidiEndpointCache::acquire();
    if (!cache_)
        return false;
    refresh_ports();
    return true;
}

void WinMidiServicesClass::refresh_ports()
{
    if (cache_)
        ports_ = cache_->ports(input_data_ != nullptr);
}

// ---------------------------------------------------------------------------
bool WinMidiServicesClass::open(size_t port_number)
{
    if (!cache_ || port_number >= ports_.size()) return false;

    active_group_  = ports_[port_number].group_index;
    first_message_ = true;

    std::string error;
    std::shared_ptr<WinMidiSharedConnection> shared =
        WinMidiConnectionPool::acquire(ports_[port_number].device_id, client_name_, error);
    if (!shared)
    {
        midi_api_.error(RtMidiError::DRIVER_ERROR, "WinMidiServicesClass::open: " + error);
        return false;
    }

    if (!shared->max_words_from_service)
        midi_api_.error(RtMidiError::DEBUG_WARNING,
                        "WinMidiServicesClass::open: the service did not report its send limit; using 684 words.");

    if (input_data_)
    {
        timestamp_frequency_ = MidiClock::TimestampFrequency();
        if (timestamp_frequency_ == 0)
            timestamp_frequency_ = 10000000u;

        gate_ = std::make_shared<WinMidiReceiveGate>();
        gate_->receiver = this;
        std::lock_guard<std::mutex> lock(shared->receivers_mtx);
        shared->receivers.push_back(gate_);
    }

    shared_ = std::move(shared);
    return true;
}

// ---------------------------------------------------------------------------
// Also releases whatever a failed open() left behind.
//
// closePort() may be called from the input callback, or from another thread,
// but not from both at the same time.
void WinMidiServicesClass::close()
{
    if (gate_)
    {
        {
            // Waits for a message being delivered on another thread, then stops
            // delivery. The mutex is recursive, so a call from the callback
            // itself does not block here.
            std::lock_guard<std::recursive_mutex> lock(gate_->mtx);
            gate_->receiver = nullptr;
        }
        if (shared_)
        {
            std::lock_guard<std::mutex> lock(shared_->receivers_mtx);
            auto& receivers = shared_->receivers;
            receivers.erase(std::remove(receivers.begin(), receivers.end(), gate_), receivers.end());
        }
        gate_.reset();
    }

    if (shared_)
    {
        WinMidiConnectionPool::release(shared_);
        shared_.reset();
    }

    encoder_.reset();
    sysex_buf_.clear();
}

// ---------------------------------------------------------------------------
// Receive callback: convert UMP back to raw MIDI 1.0 bytes
// ---------------------------------------------------------------------------
void WinMidiServicesClass::midi_in_callback(MidiMessageReceivedEventArgs const& args)
{
    if (!input_data_) return;

    uint32_t w0 = 0, w1 = 0, w2 = 0, w3 = 0;
    args.FillWords(w0, w1, w2, w3);

    const uint8_t message_type = static_cast<uint8_t>(w0 >> 28);

    // Filter to the group this port was opened on.
    if (static_cast<uint8_t>((w0 >> 24) & 0x0F) != active_group_) return;

    MidiInApi::MidiMessage message;

    if (message_type == 0x1 || message_type == 0x2)
    {
        // Type 1: system common / real-time. Type 2: MIDI 1.0 channel voice.
        const uint8_t status = static_cast<uint8_t>((w0 >> 16) & 0xFF);
        size_t byte_count;
        if (message_type == 0x1)
        {
            if (status < 0xF1 || status == 0xF7) return;
            if (status == 0xF2)
                byte_count = 3;
            else if (status == 0xF1 || status == 0xF3)
                byte_count = 2;
            else
                byte_count = 1;
        }
        else
        {
            if (status < 0x80 || status > 0xEF) return;
            const uint8_t st = status & 0xF0;
            byte_count = (st == 0xC0 || st == 0xD0) ? 2 : 3;
        }

        if ((input_data_->ignoreFlags & 0x02) && (status == 0xF1 || status == 0xF8))
            return;
        if ((input_data_->ignoreFlags & 0x04) && status == 0xFE)
            return;

        message.bytes.push_back(status);
        if (byte_count >= 2) message.bytes.push_back(static_cast<uint8_t>((w0 >> 8) & 0xFF));
        if (byte_count >= 3) message.bytes.push_back(static_cast<uint8_t>(w0 & 0xFF));
    }
    else if (message_type == 0x3)
    {
        // Type 3: SysEx 7-bit, one 64-bit packet per fragment
        if (input_data_->ignoreFlags & 0x01)
        {
            sysex_buf_.clear();
            return;
        }

        const uint8_t status_nibble = static_cast<uint8_t>((w0 >> 20) & 0x0F);
        uint8_t n = static_cast<uint8_t>((w0 >> 16) & 0x0F);
        if (n > 6) n = 6;

        // Reconstruct raw bytes from the two words (big-endian payload)
        const uint8_t payload[6] = {
            static_cast<uint8_t>((w0 >>  8) & 0xFF),
            static_cast<uint8_t>((w0 >>  0) & 0xFF),
            static_cast<uint8_t>((w1 >> 24) & 0xFF),
            static_cast<uint8_t>((w1 >> 16) & 0xFF),
            static_cast<uint8_t>((w1 >>  8) & 0xFF),
            static_cast<uint8_t>((w1 >>  0) & 0xFF),
        };

        if (status_nibble == 0x0)
        {
            // Complete single-packet SysEx - dispatch immediately.
            message.bytes.push_back(0xF0);
            message.bytes.insert(message.bytes.end(), payload, payload + n);
            message.bytes.push_back(0xF7);
        }
        else if (status_nibble == 0x1)
        {
            // Start - begin accumulation; do not dispatch yet.
            sysex_buf_.clear();
            sysex_buf_.push_back(0xF0);
            sysex_buf_.insert(sysex_buf_.end(), payload, payload + n);
            return;
        }
        else if (status_nibble == 0x2)
        {
            // Continue - append to accumulation buffer; do not dispatch yet.
            if (sysex_buf_.empty()) return; // orphaned continue, discard
            sysex_buf_.insert(sysex_buf_.end(), payload, payload + n);
            return;
        }
        else if (status_nibble == 0x3)
        {
            // End - append final bytes, close with F7, then dispatch.
            if (sysex_buf_.empty()) return; // orphaned end, discard
            sysex_buf_.insert(sysex_buf_.end(), payload, payload + n);
            sysex_buf_.push_back(0xF7);
            message.bytes = std::move(sysex_buf_);
            sysex_buf_.clear();
        }
        else
        {
            return; // unknown status nibble
        }
    }
    else
    {
        // Utility (type 0), MIDI 2.0 channel voice (type 4) and the other UMP
        // message types have no MIDI 1.0 byte form; they are not delivered.
        return;
    }

    // Timestamp: delta since the previous delivered message, in seconds
    const uint64_t ts = args.Timestamp();
    if (first_message_)
    {
        message.timeStamp = 0.0;
        first_message_ = false;
    }
    else
    {
        const uint64_t delta = (ts > last_timestamp_) ? (ts - last_timestamp_) : 0;
        message.timeStamp = static_cast<double>(delta) / static_cast<double>(timestamp_frequency_);
    }
    last_timestamp_ = ts;

    if (!input_data_->callUserCallback(message.timeStamp, &message.bytes))
    {
        std::lock_guard<std::mutex> lock(mtx_queue_);
        if (!input_data_->queue.push(message))
            std::cerr << "\nMidiInWinMidi2: message queue limit reached!!\n\n";
    }
}

// ---------------------------------------------------------------------------
// Send: convert raw MIDI 1.0 bytes to UMP and transmit
// ---------------------------------------------------------------------------
bool WinMidiServicesClass::send_buffer(const unsigned char* buf, size_t len)
{
    if (!shared_ || len == 0) return false;

    // One send at a time on the shared connection, so that one port's message
    // is never split by another port's.
    std::lock_guard<std::mutex> send_lock(shared_->send_mtx);
    if (!shared_->connection)
    {
        midi_api_.error(RtMidiError::DRIVER_ERROR,
                        "WinMidiServicesClass::send_buffer: the endpoint is no longer available.");
        return false;
    }
    const uint32_t max_words_per_call = shared_->max_words_per_call;

    try
    {
        // Encode with a copy of the encoder and keep its state only once every
        // word is sent, so a caller retrying a failed message starts from the
        // same running status and SysEx framing.
        Midi1UmpEncoder encoder = encoder_;
        std::vector<uint32_t> words;
        if (encoder.in_sysex() && rtmidiEndsSysex(buf[0]))
        {
            // Close the abandoned SysEx first, so every receiver drops it the same way.
            static const uint8_t closeSysex[2] = { 0xF0, 0xF7 };
            encoder.encode(closeSysex, 2, active_group_, words);
        }
        encoder.encode(buf, len, active_group_, words);

        const uint32_t total = static_cast<uint32_t>(words.size());
        uint32_t offset = 0;
        while (offset < total)
        {
            // Fill each batch with whole packets only: the service rejects a
            // send that ends partway through a multi-word packet.
            uint32_t batch = 0;
            while (offset + batch < total)
            {
                const uint32_t size = ump_packet_words(words[offset + batch]);
                if (batch > 0 && batch + size > max_words_per_call)
                    break;
                batch += size;
            }
            if (batch > total - offset)
                batch = total - offset;

#ifdef RTMIDI_USE_WMS_COM_RAW
            // Zero-allocation COM path: pointer straight into our local vector.
            HRESULT hr = shared_->raw->SendMidiMessagesRaw(
                0,
                batch,
                words.data() + offset);

            if (FAILED(hr))
            {
                std::ostringstream ss;
                ss << "WinMidiServicesClass::send_buffer: SendMidiMessagesRaw failed"
                   << " at word offset " << offset << " (batch " << batch << " words)"
                   << " HRESULT 0x" << std::hex << static_cast<uint32_t>(hr);
                midi_api_.error(RtMidiError::DRIVER_ERROR, ss.str());
                return false;
            }
#else
            auto result = shared_->connection.SendMultipleMessagesWordArray(
                0,
                offset,
                batch,
                winrt::array_view<uint32_t const>(words.data(), words.data() + total));

            if (!MidiEndpointConnection::SendMessageSucceeded(result))
            {
                std::ostringstream ss;
                ss << "WinMidiServicesClass::send_buffer: SendMultipleMessagesWordArray failed"
                   << " at word offset " << offset << " (batch " << batch << " words)";
                midi_api_.error(RtMidiError::DRIVER_ERROR, ss.str());
                return false;
            }
#endif
            offset += batch;
        }

        encoder_ = encoder;
        return true;
    }
    catch (winrt::hresult_error const& ex)
    {
        std::ostringstream ss;
        ss << "WinMidiServicesClass::send_buffer: HRESULT 0x"
           << std::hex << static_cast<uint32_t>(ex.code()) << " "
           << wstring_to_utf8(static_cast<std::wstring_view>(ex.message()));
        midi_api_.error(RtMidiError::DRIVER_ERROR, ss.str());
        return false;
    }
}

//*********************************************************************//
//  API: Windows MIDI Services - MidiInWinMidi2
//*********************************************************************//

MidiInWinMidi2::MidiInWinMidi2(const std::string& clientName, unsigned int queueSizeLimit)
    : MidiInApi(queueSizeLimit)
{
    MidiInWinMidi2::initialize(clientName);
}

MidiInWinMidi2::~MidiInWinMidi2()
{
    MidiInWinMidi2::closePort();
    delete static_cast<WinMidiServicesClass*>(apiData_);
}

void MidiInWinMidi2::initialize(const std::string& clientName)
{
    WinMidiServicesClass* data = new WinMidiServicesClass(*this);
    data->client_name_ = clientName;
    apiData_ = static_cast<void*>(data);

    if (!data->init(&inputData_))
    {
        // Only a debug warning: an UNSPECIFIED API search constructs this
        // backend on systems without the runtime. openPort() reports it.
        errorString_ = "MidiInWinMidi2::initialize: Windows MIDI Services is not available.";
        error(RtMidiError::DEBUG_WARNING, errorString_);
        return;
    }

    if (data->get_num_ports() == 0)
    {
        errorString_ = "MidiInWinMidi2::initialize: no MIDI input devices currently available.";
        error(RtMidiError::WARNING, errorString_);
    }
}

void MidiInWinMidi2::openPort(unsigned int portNumber, const std::string& /*portName*/)
{
    WinMidiServicesClass* data = static_cast<WinMidiServicesClass*>(apiData_);
    std::lock_guard<std::mutex> lock(data->mtx_open_close_);

    if (connected_)
    {
        errorString_ = "MidiInWinMidi2::openPort: a valid connection already exists!";
        error(RtMidiError::WARNING, errorString_);
        return;
    }

    if (!data->is_ready() && !data->init(&inputData_))
    {
        errorString_ = "MidiInWinMidi2::openPort: Windows MIDI Services is not available "
                       "on this system (the API is missing, the MIDI service is not running, or Windows "
                       "is set to its legacy MIDI stack).";
        error(RtMidiError::DRIVER_NOT_INSTALLED, errorString_);
        return;
    }

    // Refresh the port list from the live watcher cache.  This instance may have
    // been created before the target device appeared (e.g. midi_in is recreated
    // on Windows when a port closes, but the bootloader shows up 700ms later).
    data->refresh_ports();

    if (data->get_num_ports() == 0)
    {
        errorString_ = "MidiInWinMidi2::openPort: no MIDI input sources found!";
        error(RtMidiError::NO_DEVICES_FOUND, errorString_);
        return;
    }
    if (portNumber >= data->get_num_ports())
    {
        std::ostringstream ost;
        ost << "MidiInWinMidi2::openPort: portNumber (" << portNumber
            << ") is invalid (cache has " << data->get_num_ports() << " ports).";
        errorString_ = ost.str();
        error(RtMidiError::INVALID_PARAMETER, errorString_);
        return;
    }
    if (!data->open(portNumber))
    {
        errorString_ = "MidiInWinMidi2::openPort: error opening Windows MIDI Services input port.";
        error(RtMidiError::DRIVER_ERROR, errorString_);
        return;
    }
    connected_ = true;
}

// Virtual ports are not implemented yet, waiting on https://www.github.com/microsoft/MIDI/issues/1047
// to make it's way into the in box version of Windows MIDI Services
void MidiInWinMidi2::openVirtualPort(const std::string& /*portName*/)
{
    errorString_ = "MidiInWinMidi2::openVirtualPort: not implemented yet for Windows MIDI Services.";
    error(RtMidiError::WARNING, errorString_);
}

void MidiInWinMidi2::closePort()
{
    WinMidiServicesClass* data = static_cast<WinMidiServicesClass*>(apiData_);
    std::lock_guard<std::mutex> lock(data->mtx_open_close_);
    if (connected_)
    {
        data->close();
        connected_ = false;
    }
}

void MidiInWinMidi2::setClientName(const std::string&)
{
    errorString_ = "MidiInWinMidi2::setClientName: not implemented for WINDOWS_MIDI_SERVICES.";
    error(RtMidiError::WARNING, errorString_);
}

void MidiInWinMidi2::setPortName(const std::string&)
{
    errorString_ = "MidiInWinMidi2::setPortName: not implemented for WINDOWS_MIDI_SERVICES.";
    error(RtMidiError::WARNING, errorString_);
}

unsigned int MidiInWinMidi2::getPortCount()
{
    WinMidiServicesClass* data = static_cast<WinMidiServicesClass*>(apiData_);
    // Re-read the watcher cache on every call when no port is open, so hot-plug
    // events (e.g. device rebooting into bootloader) are visible - matching the
    // live-query behaviour of the WinMM backend.
    if (!connected_)
        data->refresh_ports();
    for (std::string const& note : data->take_new_warnings())
    {
        errorString_ = "MidiInWinMidi2::getPortCount: " + note;
        error(RtMidiError::WARNING, errorString_);
    }
    return static_cast<unsigned int>(data->get_num_ports());
}

std::string MidiInWinMidi2::getPortName(unsigned int portNumber)
{
    WinMidiServicesClass* data = static_cast<WinMidiServicesClass*>(apiData_);
    if (portNumber >= data->get_num_ports())
    {
        std::ostringstream ost;
        ost << "MidiInWinMidi2::getPortName: portNumber (" << portNumber << ") is invalid.";
        errorString_ = ost.str();
        error(RtMidiError::WARNING, errorString_);
        return "";
    }
    return data->get_port_name(portNumber);
}

double MidiInWinMidi2::getMessage(std::vector<unsigned char>* message)
{
    WinMidiServicesClass* data = static_cast<WinMidiServicesClass*>(apiData_);
    std::lock_guard<std::mutex> lock(data->mtx_queue_);
    return MidiInApi::getMessage(message);
}

//*********************************************************************//
//  API: Windows MIDI Services - MidiOutWinMidi2
//*********************************************************************//

MidiOutWinMidi2::MidiOutWinMidi2(const std::string& clientName) : MidiOutApi()
{
    MidiOutWinMidi2::initialize(clientName);
}

MidiOutWinMidi2::~MidiOutWinMidi2()
{
    MidiOutWinMidi2::closePort();
    delete static_cast<WinMidiServicesClass*>(apiData_);
}

void MidiOutWinMidi2::initialize(const std::string& clientName)
{
    WinMidiServicesClass* data = new WinMidiServicesClass(*this);
    data->client_name_ = clientName;
    apiData_ = static_cast<void*>(data);

    if (!data->init(nullptr))
    {
        // Only a debug warning: an UNSPECIFIED API search constructs this
        // backend on systems without the runtime. openPort() reports it.
        errorString_ = "MidiOutWinMidi2::initialize: Windows MIDI Services is not available.";
        error(RtMidiError::DEBUG_WARNING, errorString_);
        return;
    }

    if (data->get_num_ports() == 0)
    {
        errorString_ = "MidiOutWinMidi2::initialize: no MIDI output devices currently available.";
        error(RtMidiError::WARNING, errorString_);
    }
}

unsigned int MidiOutWinMidi2::getPortCount()
{
    WinMidiServicesClass* data = static_cast<WinMidiServicesClass*>(apiData_);
    // Re-read the watcher cache on every call when no port is open - matches WinMM behaviour.
    if (!connected_)
        data->refresh_ports();
    for (std::string const& note : data->take_new_warnings())
    {
        errorString_ = "MidiOutWinMidi2::getPortCount: " + note;
        error(RtMidiError::WARNING, errorString_);
    }
    return static_cast<unsigned int>(data->get_num_ports());
}

std::string MidiOutWinMidi2::getPortName(unsigned int portNumber)
{
    WinMidiServicesClass* data = static_cast<WinMidiServicesClass*>(apiData_);
    if (portNumber >= data->get_num_ports())
    {
        std::ostringstream ost;
        ost << "MidiOutWinMidi2::getPortName: portNumber (" << portNumber << ") is invalid.";
        errorString_ = ost.str();
        error(RtMidiError::WARNING, errorString_);
        return "";
    }
    return data->get_port_name(portNumber);
}

void MidiOutWinMidi2::openPort(unsigned int portNumber, const std::string& /*portName*/)
{
    WinMidiServicesClass* data = static_cast<WinMidiServicesClass*>(apiData_);
    std::lock_guard<std::mutex> lock(data->mtx_open_close_);

    if (connected_)
    {
        errorString_ = "MidiOutWinMidi2::openPort: a valid connection already exists!";
        error(RtMidiError::WARNING, errorString_);
        return;
    }

    if (!data->is_ready() && !data->init(nullptr))
    {
        errorString_ = "MidiOutWinMidi2::openPort: Windows MIDI Services is not available "
                       "on this system (the API is missing, the MIDI service is not running, or Windows "
                       "is set to its legacy MIDI stack).";
        error(RtMidiError::DRIVER_NOT_INSTALLED, errorString_);
        return;
    }

    // Refresh the port list from the live watcher cache (same reason as MidiInWinMidi2).
    data->refresh_ports();

    if (data->get_num_ports() == 0)
    {
        errorString_ = "MidiOutWinMidi2::openPort: no MIDI output destinations found!";
        error(RtMidiError::NO_DEVICES_FOUND, errorString_);
        return;
    }
    if (portNumber >= data->get_num_ports())
    {
        std::ostringstream ost;
        ost << "MidiOutWinMidi2::openPort: portNumber (" << portNumber
            << ") is invalid (cache has " << data->get_num_ports() << " ports).";
        errorString_ = ost.str();
        error(RtMidiError::INVALID_PARAMETER, errorString_);
        return;
    }
    if (!data->open(portNumber))
    {
        errorString_ = "MidiOutWinMidi2::openPort: error opening Windows MIDI Services output port.";
        error(RtMidiError::DRIVER_ERROR, errorString_);
        return;
    }
    connected_ = true;
}

void MidiOutWinMidi2::closePort()
{
    WinMidiServicesClass* data = static_cast<WinMidiServicesClass*>(apiData_);
    std::lock_guard<std::mutex> lock(data->mtx_open_close_);
    if (connected_)
    {
        data->close();
        connected_ = false;
    }
}

// Not implemented yet; still a stub.
void MidiOutWinMidi2::openVirtualPort(const std::string& /*portName*/)
{
    errorString_ = "MidiOutWinMidi2::openVirtualPort: not implemented yet for Windows MIDI Services.";
    error(RtMidiError::WARNING, errorString_);
}

void MidiOutWinMidi2::setClientName(const std::string&)
{
    errorString_ = "MidiOutWinMidi2::setClientName: not implemented for WINDOWS_MIDI_SERVICES.";
    error(RtMidiError::WARNING, errorString_);
}

void MidiOutWinMidi2::setPortName(const std::string&)
{
    errorString_ = "MidiOutWinMidi2::setPortName: not implemented for WINDOWS_MIDI_SERVICES.";
    error(RtMidiError::WARNING, errorString_);
}

void MidiOutWinMidi2::sendMessage(const unsigned char* message, size_t size)
{
    if (!connected_) return;
    if (size == 0)
    {
        errorString_ = "MidiOutWinMidi2::sendMessage: message is empty!";
        error(RtMidiError::WARNING, errorString_);
        return;
    }
    WinMidiServicesClass* data = static_cast<WinMidiServicesClass*>(apiData_);
    if (!data->send_buffer(message, size))
    {
        errorString_ = "MidiOutWinMidi2::sendMessage: error sending message.";
        error(RtMidiError::DRIVER_ERROR, errorString_);
        return;
    }
    // NOTE: send_buffer() (SendMidiMessagesRaw/SendMultipleMessagesWordArray)
    // returns once WMS accepts the message, NOT once it is actually on the
    // wire - unlike MidiOutWinMM::sendMessage, which blocks until the driver
    // has finished with the buffer. So this backend gives a caller pacing a
    // chunked transfer no real backpressure signal - only whatever gap the
    // caller adds itself. Whether that matters in practice depends on the WMS
    // transport's own internal pacing, which has not been characterized here.
}

// ---------------------------------------------------------------------------
// RtMidi::checkApiAvailability - WMS implementation
// ---------------------------------------------------------------------------
RtMidi::RtMidiApiAvailability RtMidi::checkApiAvailability( RtMidi::Api api )
{
    RtMidiApiAvailability result;
    if ( api != RtMidi::WINDOWS_MIDI_SERVICES )
        return result;  // available = true (default) for all other APIs

    switch ( wms_availability() ) {
    case WmsAvailability::Available:
        break;
    case WmsAvailability::NotPresent:
        result.available = false;
        result.message   = "The Windows MIDI Services API (Windows.Devices.Midi2) is missing: this version of Windows does not include it, and the app does not carry its own copy.";
        break;
    case WmsAvailability::ServiceUnavailable:
        result.available = false;
        result.message   = "The Windows MIDI Service is not running and could not be started.";
        break;
    case WmsAvailability::LegacyMode:
        result.available = false;
        result.message   = "Windows is set to use its legacy MIDI stack rather than Windows MIDI Services.";
        break;
    }
    return result;
}

#endif  // RTMIDI_API_WMS

// Without the Windows MIDI Services backend, no compiled API depends on a
// runtime that can be missing, so only WMS itself is reported as unavailable.
#if !defined(RTMIDI_API_WMS)
RtMidi::RtMidiApiAvailability RtMidi::checkApiAvailability( RtMidi::Api api )
{
    RtMidiApiAvailability result;
    if ( api == RtMidi::WINDOWS_MIDI_SERVICES ) {
        result.available = false;
        result.message   = "This build does not include the Windows MIDI Services backend.";
    }
    return result;
}
#endif  // !RTMIDI_API_WMS


//*********************************************************************//
//  API: UNIX JACK
//
//  Written primarily by Alexander Svetalkin, with updates for delta
//  time by Gary Scavone, April 2011.
//
//  *********************************************************************//

#if defined(RTMIDI_API_JACK)

// JACK header files
#include <jack/jack.h>
#include <jack/midiport.h>
#include <jack/ringbuffer.h>
#include <chrono>
#include <pthread.h>
#include <sched.h>
#ifdef HAVE_SEMAPHORE
  #include <semaphore.h>
#endif

#define JACK_RINGBUFFER_SIZE 16384 // Default size for ringbuffer

struct JackMidiData {
  jack_client_t *client;
  jack_port_t *port;
  jack_ringbuffer_t *buff;
  int buffMaxWrite; // actual writable size, usually 1 less than ringbuffer
  jack_time_t lastTime;
  bool sysexOpen = false; // output: a SysEx has started and not yet ended
#ifdef HAVE_SEMAPHORE
  sem_t sem_cleanup;
  sem_t sem_needpost;
#endif
  MidiInApi :: RtMidiInData *rtMidiIn;
  std::atomic<bool> serverGone{ false }; // output: set by jackShutdown()
  bool serverGoneReported = false;
  bool noPortReported = false;
  };

//*********************************************************************//
//  API: JACK
//  Class Definitions: MidiInJack
//*********************************************************************//

static int jackProcessIn( jack_nframes_t nframes, void *arg )
{
  JackMidiData *jData = (JackMidiData *) arg;
  MidiInApi :: RtMidiInData *rtData = jData->rtMidiIn;
  jack_midi_event_t event;
  jack_time_t time;

  // Is port created?
  if ( jData->port == NULL ) return 0;

  void *buff = jack_port_get_buffer( jData->port, nframes );
  bool& continueSysex = rtData->continueSysex;
  unsigned char& ignoreFlags = rtData->ignoreFlags;

  // We have midi events in buffer
  int evCount = jack_midi_get_event_count( buff );
  for (int j = 0; j < evCount; j++) {
    MidiInApi::MidiMessage& message = rtData->message;
    jack_midi_event_get( &event, buff, j );

    // Compute the delta time.
    time = jack_get_time();
    if ( rtData->firstMessage == true ) {
      message.timeStamp = 0.0;
      rtData->firstMessage = false;
    } else
      message.timeStamp = ( time - jData->lastTime ) * 0.000001;

    jData->lastTime = time;

    // A real-time message in the middle of a SysEx is delivered on its own.
    if ( continueSysex && event.size == 1 && event.buffer[0] >= 0xF8 ) {
      if ( !rtmidiIgnoredRealtime( event.buffer[0], ignoreFlags ) ) {
        MidiInApi::MidiMessage realtime;
        realtime.bytes.assign( 1, event.buffer[0] );
        realtime.timeStamp = message.timeStamp;
        rtmidiDeliver( rtData, realtime, "MidiInJack" );
      }
      continue;
    }

    // A status byte (not F7 or real-time) here means the SysEx lost its end: drop it.
    if ( continueSysex && event.size > 0 && event.buffer[0] >= 0x80 &&
         event.buffer[0] < 0xF8 && event.buffer[0] != 0xF7 )
      continueSysex = false;

    // A data byte with no SysEx in progress is a piece of one whose start was lost: drop it.
    if ( !continueSysex && event.size > 0 && event.buffer[0] < 0x80 )
      continue;

    if ( !continueSysex )
      message.bytes.clear();

    if ( !( ( continueSysex || event.buffer[0] == 0xF0 ) && ( ignoreFlags & 0x01 ) ) ) {
      // Unless this is a (possibly continued) SysEx message and we're ignoring SysEx,
      // copy the event buffer into the MIDI message struct.
      size_t from = message.bytes.size();
      for ( unsigned int i = 0; i < event.size; i++ )
        message.bytes.push_back( event.buffer[i] );
      if ( !message.bytes.empty() && message.bytes[0] == 0xF0 )
        rtmidiSplitRealtime( rtData, message.bytes, from, message.timeStamp, "MidiInJack" );
    }

    switch ( event.buffer[0] ) {
      case 0xF0:
        // Start of a SysEx message
        continueSysex = event.buffer[event.size - 1] != 0xF7;
        if ( ignoreFlags & 0x01 ) continue;
        break;
      case 0xF1:
      case 0xF8:
        // MIDI Time Code or Timing Clock message
        if ( ignoreFlags & 0x02 ) continue;
        break;
      case 0xFE:
        // Active Sensing message
        if ( ignoreFlags & 0x04 ) continue;
        break;
      default:
        if ( continueSysex ) {
          // Continuation of a SysEx message
          continueSysex = event.buffer[event.size - 1] != 0xF7;
          if ( ignoreFlags & 0x01 ) continue;
        }
        // All other MIDI messages
    }

    if ( !continueSysex ) {
      // If not a continuation of a SysEx message,
      // invoke the user callback function or queue the message.
      if ( !rtData->callUserCallback( message.timeStamp, &message.bytes ) ) {
        // As long as we haven't reached our queue size limit, push the message.
        if ( !rtData->queue.push( message ) )
          std::cerr << "\nMidiInJack: message queue limit reached!!\n\n";
      }
    }
  }

  return 0;
}

MidiInJack :: MidiInJack( const std::string &clientName, unsigned int queueSizeLimit )
  : MidiInApi( queueSizeLimit )
{
  MidiInJack::initialize( clientName );
}

void MidiInJack :: initialize( const std::string& clientName )
{
  JackMidiData *data = new JackMidiData;
  apiData_ = (void *) data;

  data->rtMidiIn = &inputData_;
  data->port = NULL;
  data->client = NULL;
  this->clientName = clientName;

  connect();
}

void MidiInJack :: connect()
{
  JackMidiData *data = static_cast<JackMidiData *> (apiData_);
  if ( data->client )
    return;

  // Initialize JACK client
  if (( data->client = jack_client_open( clientName.c_str(), JackNoStartServer, NULL )) == 0) {
    errorString_ = "MidiInJack::initialize: JACK server not running?";
    error( RtMidiError::WARNING, errorString_ );
    return;
  }

  jack_set_process_callback( data->client, jackProcessIn, data );
  jack_activate( data->client );
}

MidiInJack :: ~MidiInJack()
{
  JackMidiData *data = static_cast<JackMidiData *> (apiData_);
  MidiInJack::closePort();

  if ( data->client )
    jack_client_close( data->client );
  delete data;
}

void MidiInJack :: openPort( unsigned int portNumber, const std::string &portName )
{
  JackMidiData *data = static_cast<JackMidiData *> (apiData_);

  connect();
  if ( !data->client ) {
    errorString_ = "MidiInJack::openPort: JACK server not running?";
    error( RtMidiError::DRIVER_ERROR, errorString_ );
    return;
  }

  // Creating new port
  if ( data->port == NULL )
    data->port = jack_port_register( data->client, portName.c_str(),
                                     JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0 );

  if ( data->port == NULL ) {
    errorString_ = "MidiInJack::openPort: JACK error creating port";
    if (portName.size() >= (size_t)jack_port_name_size())
        errorString_ += " (port name too long?)";
    error( RtMidiError::DRIVER_ERROR, errorString_ );
    return;
  }

  // Connecting to the output
  std::string name = getPortName( portNumber );
  jack_connect( data->client, name.c_str(), jack_port_name( data->port ) );

  connected_ = true;
}

void MidiInJack :: openVirtualPort( const std::string &portName )
{
  JackMidiData *data = static_cast<JackMidiData *> (apiData_);

  connect();
  if ( !data->client ) {
    errorString_ = "MidiInJack::openVirtualPort: JACK server not running?";
    error( RtMidiError::DRIVER_ERROR, errorString_ );
    return;
  }
  if ( data->port == NULL )
    data->port = jack_port_register( data->client, portName.c_str(),
                                     JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0 );

  if ( data->port == NULL ) {
    errorString_ = "MidiInJack::openVirtualPort: JACK error creating virtual port";
    if (portName.size() >= (size_t)jack_port_name_size())
        errorString_ += " (port name too long?)";
    error( RtMidiError::DRIVER_ERROR, errorString_ );
  }
}

unsigned int MidiInJack :: getPortCount()
{
  int count = 0;
  JackMidiData *data = static_cast<JackMidiData *> (apiData_);
  connect();
  if ( !data->client )
    return 0;

  // List of available ports
  const char **ports = jack_get_ports( data->client, NULL, JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput );

  if ( ports == NULL ) return 0;
  while ( ports[count] != NULL )
    count++;

  free( ports );

  return count;
}

std::string MidiInJack :: getPortName( unsigned int portNumber )
{
  JackMidiData *data = static_cast<JackMidiData *> (apiData_);
  std::string retStr( "" );

  connect();

  // List of available ports
  const char **ports = jack_get_ports( data->client, NULL,
                                       JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput );

  // Check port validity
  if ( ports == NULL ) {
    errorString_ = "MidiInJack::getPortName: no ports available!";
    error( RtMidiError::WARNING, errorString_ );
    return retStr;
  }

  unsigned int i;
  for ( i=0; i<portNumber && ports[i]; i++ ) {}
  if ( i < portNumber || !ports[portNumber] ) {
    std::ostringstream ost;
    ost << "MidiInJack::getPortName: the 'portNumber' argument (" << portNumber << ") is invalid.";
    errorString_ = ost.str();
    error( RtMidiError::WARNING, errorString_ );
  }
  else retStr.assign( ports[portNumber] );

  jack_free( ports );
  return retStr;
}

void MidiInJack :: closePort()
{
  JackMidiData *data = static_cast<JackMidiData *> (apiData_);

  if ( data->port == NULL ) return;
  // libjack can crash when a port is unregistered while the client runs, so stop it first.
  jack_deactivate( data->client );
  jack_port_unregister( data->client, data->port );
  data->port = NULL;
  jack_activate( data->client );

  connected_ = false;
}

void MidiInJack:: setClientName( const std::string& )
{

  errorString_ = "MidiInJack::setClientName: this function is not implemented for the UNIX_JACK API!";
  error( RtMidiError::WARNING, errorString_ );

}

void MidiInJack :: setPortName( const std::string &portName )
{
  JackMidiData *data = static_cast<JackMidiData *> (apiData_);
#ifdef RTMIDI_JACK_HAS_PORT_RENAME
  jack_port_rename( data->client, data->port, portName.c_str() );
#else
  jack_port_set_name( data->port, portName.c_str() );
#endif
}

//*********************************************************************//
//  API: JACK
//  Class Definitions: MidiOutJack
//*********************************************************************//

// Jack process callback
static int jackProcessOut( jack_nframes_t nframes, void *arg )
{
  JackMidiData *data = (JackMidiData *) arg;
  jack_midi_data_t *midiData;
  int space;

  // Is port created?
  if ( data->port == NULL ) return 0;

  void *buff = jack_port_get_buffer( data->port, nframes );
  jack_midi_clear_buffer( buff );

  while ( jack_ringbuffer_peek( data->buff, (char *) &space, sizeof( space ) ) == sizeof(space) &&
          jack_ringbuffer_read_space( data->buff ) >= sizeof(space) + space ) {
    midiData = jack_midi_event_reserve( buff, 0, space );
    if ( midiData == NULL ) {
      // The port buffer is full: leave the rest for the next period.
      if ( jack_midi_get_event_count( buff ) > 0 ) break;
      // Too big even for an empty buffer, so it would never go out.
      jack_ringbuffer_read_advance( data->buff, sizeof(space) + (size_t) space );
      continue;
    }
    jack_ringbuffer_read_advance( data->buff, sizeof(space) );
    jack_ringbuffer_read( data->buff, (char *) midiData, (size_t) space );
  }

#ifdef HAVE_SEMAPHORE
  // closePort() is waiting: tell it once everything queued has gone out.
  if ( !sem_trywait( &data->sem_needpost ) ) {
    if ( jack_ringbuffer_read_space( data->buff ) == 0 )
      sem_post( &data->sem_cleanup );
    else
      sem_post( &data->sem_needpost );
  }
#endif

  return 0;
}

// The server has shut down or dropped the client; nothing will read the ringbuffer again.
static void jackShutdown( void *arg )
{
  static_cast<JackMidiData *>( arg )->serverGone = true;
}

MidiOutJack :: MidiOutJack( const std::string &clientName ) : MidiOutApi()
{
  MidiOutJack::initialize( clientName );
}

void MidiOutJack :: initialize( const std::string& clientName )
{
  JackMidiData *data = new JackMidiData;
  apiData_ = (void *) data;

  data->port = NULL;
  data->client = NULL;
  data->buff = NULL;
#ifdef HAVE_SEMAPHORE
  sem_init( &data->sem_cleanup, 0, 0 );
  sem_init( &data->sem_needpost, 0, 0 );
#endif
  this->clientName = clientName;

  connect();
}

void MidiOutJack :: connect()
{
  JackMidiData *data = static_cast<JackMidiData *> (apiData_);
  if ( data->client )
    return;

  // Initialize output ringbuffers, once: connect() runs again while the server is down.
  if ( data->buff == NULL ) {
    data->buff = jack_ringbuffer_create( JACK_RINGBUFFER_SIZE );
    data->buffMaxWrite = (int) jack_ringbuffer_write_space( data->buff );
  }

  // Initialize JACK client
  if ( ( data->client = jack_client_open( clientName.c_str(), JackNoStartServer, NULL ) ) == 0 ) {
    errorString_ = "MidiOutJack::initialize: JACK server not running?";
    error( RtMidiError::WARNING, errorString_ );
    return;
  }

  jack_set_process_callback( data->client, jackProcessOut, data );
  jack_on_shutdown( data->client, jackShutdown, data );
  jack_activate( data->client );
}

MidiOutJack :: ~MidiOutJack()
{
  JackMidiData *data = static_cast<JackMidiData *> (apiData_);
  MidiOutJack::closePort();

  // Close the client first: its process callback reads the ringbuffer until then.
  if ( data->client ) {
    jack_client_close( data->client );
  }
  jack_ringbuffer_free( data->buff );

#ifdef HAVE_SEMAPHORE
  sem_destroy( &data->sem_cleanup );
  sem_destroy( &data->sem_needpost );
#endif

  delete data;
}

void MidiOutJack :: openPort( unsigned int portNumber, const std::string &portName )
{
  JackMidiData *data = static_cast<JackMidiData *> (apiData_);
  data->noPortReported = false;

  connect();
  if ( !data->client ) {
    errorString_ = "MidiOutJack::openPort: JACK server not running?";
    error( RtMidiError::DRIVER_ERROR, errorString_ );
    return;
  }

  // Creating new port
  if ( data->port == NULL )
    data->port = jack_port_register( data->client, portName.c_str(),
                                     JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0 );

  if ( data->port == NULL ) {
    errorString_ = "MidiOutJack::openPort: JACK error creating port";
    if (portName.size() >= (size_t)jack_port_name_size())
        errorString_ += " (port name too long?)";
    error( RtMidiError::DRIVER_ERROR, errorString_ );
    return;
  }

  // Connecting to the output
  std::string name = getPortName( portNumber );
  jack_connect( data->client, jack_port_name( data->port ), name.c_str() );

  connected_ = true;
}

void MidiOutJack :: openVirtualPort( const std::string &portName )
{
  JackMidiData *data = static_cast<JackMidiData *> (apiData_);
  data->noPortReported = false;

  connect();
  if ( !data->client ) {
    errorString_ = "MidiOutJack::openVirtualPort: JACK server not running?";
    error( RtMidiError::DRIVER_ERROR, errorString_ );
    return;
  }
  if ( data->port == NULL )
    data->port = jack_port_register( data->client, portName.c_str(),
                                     JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0 );

  if ( data->port == NULL ) {
    errorString_ = "MidiOutJack::openVirtualPort: JACK error creating virtual port";
    if (portName.size() >= (size_t)jack_port_name_size())
        errorString_ += " (port name too long?)";
    error( RtMidiError::DRIVER_ERROR, errorString_ );
  }
}

unsigned int MidiOutJack :: getPortCount()
{
  int count = 0;
  JackMidiData *data = static_cast<JackMidiData *> (apiData_);
  connect();
  if ( !data->client )
    return 0;

  // List of available ports
  const char **ports = jack_get_ports( data->client, NULL,
                                       JACK_DEFAULT_MIDI_TYPE, JackPortIsInput );

  if ( ports == NULL ) return 0;
  while ( ports[count] != NULL )
    count++;

  free( ports );

  return count;
}

std::string MidiOutJack :: getPortName( unsigned int portNumber )
{
  JackMidiData *data = static_cast<JackMidiData *> (apiData_);
  std::string retStr("");

  connect();

  // List of available ports
  const char **ports = jack_get_ports( data->client, NULL,
                                       JACK_DEFAULT_MIDI_TYPE, JackPortIsInput );

  // Check port validity
  if ( ports == NULL ) {
    errorString_ = "MidiOutJack::getPortName: no ports available!";
    error( RtMidiError::WARNING, errorString_ );
    return retStr;
  }

  if ( ports[portNumber] == NULL ) {
    std::ostringstream ost;
    ost << "MidiOutJack::getPortName: the 'portNumber' argument (" << portNumber << ") is invalid.";
    errorString_ = ost.str();
    error( RtMidiError::WARNING, errorString_ );
  }
  else retStr.assign( ports[portNumber] );

  free( ports );
  return retStr;
}

void MidiOutJack :: closePort()
{
  JackMidiData *data = static_cast<JackMidiData *> (apiData_);

  if ( data->port == NULL ) return;

#ifdef HAVE_SEMAPHORE
  // Wait for queued output to go out, unless the server is gone and nothing will send it.
  struct timespec ts;
  if ( !data->serverGone && clock_gettime( CLOCK_REALTIME, &ts ) != -1 ) {
    ts.tv_sec += 1; // wait max one second
    sem_post( &data->sem_needpost );
    sem_timedwait( &data->sem_cleanup, &ts );
  }
#endif

  // libjack can crash when a port is unregistered while the client runs, so stop it first.
  jack_deactivate( data->client );

  // The callback is stopped, so clear what a timed-out wait left behind: the handshake, and
  // output that never went out, which would otherwise go out on the next port opened.
#ifdef HAVE_SEMAPHORE
  while ( sem_trywait( &data->sem_needpost ) == 0 ) {}
  while ( sem_trywait( &data->sem_cleanup ) == 0 ) {}
#endif
  size_t messages = 0, bytes = 0;
  int length;
  while ( jack_ringbuffer_read( data->buff, (char *) &length, sizeof( length ) ) == sizeof( length ) ) {
    jack_ringbuffer_read_advance( data->buff, (size_t) length );
    messages++;
    bytes += (size_t) length;
  }
  if ( messages > 0 ) {
    std::ostringstream ost;
    ost << "MidiOutJack::closePort: " << messages << " message(s), " << bytes
        << " bytes, never went out and were dropped.";
    errorString_ = ost.str();
    error( RtMidiError::WARNING, errorString_ );
  }

  jack_port_unregister( data->client, data->port );
  data->port = NULL;
  data->sysexOpen = false;
  jack_activate( data->client );

  connected_ = false;
}

void MidiOutJack:: setClientName( const std::string& )
{

  errorString_ = "MidiOutJack::setClientName: this function is not implemented for the UNIX_JACK API!";
  error( RtMidiError::WARNING, errorString_ );

}

void MidiOutJack :: setPortName( const std::string &portName )
{
  JackMidiData *data = static_cast<JackMidiData *> (apiData_);
#ifdef RTMIDI_JACK_HAS_PORT_RENAME
  jack_port_rename( data->client, data->port, portName.c_str() );
#else
  jack_port_set_name( data->port, portName.c_str() );
#endif
}

void MidiOutJack :: sendMessage( const unsigned char *message, size_t size )
{
  int nBytes = static_cast<int>(size);
  JackMidiData *data = static_cast<JackMidiData *> (apiData_);

  if ( size + sizeof(nBytes) > (size_t) data->buffMaxWrite ) {
    std::ostringstream ost;
    ost << "MidiOutJack::sendMessage: a message over " << data->buffMaxWrite - sizeof(nBytes)
        << " bytes doesn't fit RtMidi's output buffer; send a large SysEx in pieces. Message dropped.";
    errorString_ = ost.str();
    error( RtMidiError::WARNING, errorString_ );
    return;
  }

  // Only the process callback empties the ringbuffer, so don't queue what it won't send.
  if ( data->port == NULL ) {
    if ( !data->noPortReported ) {
      data->noPortReported = true;
      errorString_ = "MidiOutJack::sendMessage: no port is open; messages are being dropped.";
      error( RtMidiError::WARNING, errorString_ );
    }
    return;
  }
  if ( data->serverGone ) {
    if ( !data->serverGoneReported ) {
      data->serverGoneReported = true;
      errorString_ = "MidiOutJack::sendMessage: the JACK server has shut down; messages are being dropped.";
      error( RtMidiError::WARNING, errorString_ );
    }
    return;
  }

  if ( size > 0 && data->sysexOpen && rtmidiEndsSysex( message[0] ) ) {
    // Close the abandoned SysEx first, so every receiver drops it the same way.
    static const unsigned char closeSysex[2] = { 0xF0, 0xF7 };
    sendMessage( closeSysex, 2 );
  }

  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 1 );
  while ( jack_ringbuffer_write_space(data->buff) < sizeof(nBytes) + size ) {
    if ( data->serverGone ) return;  // reported on the next call
    if ( std::chrono::steady_clock::now() > deadline ) {
      errorString_ = "MidiOutJack::sendMessage: JACK took no data for a second; message dropped.";
      error( RtMidiError::WARNING, errorString_ );
      return;
    }
    sched_yield();
  }

  // Write full message to buffer
  jack_ringbuffer_write( data->buff, ( char * ) &nBytes, sizeof( nBytes ) );
  jack_ringbuffer_write( data->buff, ( const char * ) message, nBytes );

  // Only a message that was queued changes the SysEx state.
  for ( size_t i = 0; i < size; i++ ) {
    if ( message[i] == 0xF0 ) data->sysexOpen = true;
    else if ( message[i] >= 0x80 && message[i] < 0xF8 ) data->sysexOpen = false;
  }
}

#endif  // RTMIDI_API_JACK

//*********************************************************************//
//  API: Web MIDI
//
//  Written primarily by Atsushi Eno, February 2020.
//
//  *********************************************************************//

#if defined(RTMIDI_API_WEBMIDI)

#include <emscripten.h>

//*********************************************************************//
//  API: WEB MIDI
//  Class Definitions: WebMidiAccessShim
//*********************************************************************//

class WebMidiAccessShim
{
public:
  WebMidiAccessShim();
  ~WebMidiAccessShim();
  std::string getPortName( unsigned int portNumber, bool isInput );
};

std::unique_ptr<WebMidiAccessShim> shim{nullptr};

void ensureShim()
{
  if ( shim.get() != nullptr )
    return;
  shim.reset( new WebMidiAccessShim() );
}

bool checkWebMidiAvailability()
{
  ensureShim();

  return MAIN_THREAD_EM_ASM_INT( {
    if ( typeof window._rtmidi_internals_waiting === "undefined" ) {
      console.log ( "Attempted to use Web MIDI API without trying to open it." );
      return false;
    }
    if ( window._rtmidi_internals_waiting ) {
      console.log ( "Attempted to use Web MIDI API while it is being queried." );
      return false;
    }
    if ( _rtmidi_internals_midi_access == null ) {
      console.log ( "Attempted to use Web MIDI API while it already turned out to be unavailable." );
      return false;
    }
    return true;
  } );
}

WebMidiAccessShim::WebMidiAccessShim()
{
  MAIN_THREAD_ASYNC_EM_ASM( {
    if( typeof window._rtmidi_internals_midi_access !== "undefined" )
      return;
    if( typeof window._rtmidi_internals_waiting !== "undefined" ) {
       console.log( "MIDI Access was requested while another request is in progress." );
       return;
    }

    // define functions
    window._rtmidi_internals_get_port_by_number = function( portNumber, isInput ) {
      var midi = window._rtmidi_internals_midi_access;
      var devices = isInput ? midi.inputs : midi.outputs;
      var i = 0;
      for (var device of devices.values()) {
        if ( i == portNumber )
          return device;
        i++;
      }
      console.log( "MIDI " + (isInput ? "input" : "output") + " device of portNumber " + portNumber + " is not found.");
      return null;
    };

    window._rtmidi_internals_waiting = true;
    window.navigator.requestMIDIAccess( {"sysex": true} ).then( (midiAccess) => {
      window._rtmidi_internals_midi_access = midiAccess;
      window._rtmidi_internals_latest_message_timestamp = 0.0;
      window._rtmidi_internals_waiting = false;
      if( midiAccess == null ) {
        console.log ( "Could not get access to MIDI API" );
      }
    } );
  } );
}

WebMidiAccessShim::~WebMidiAccessShim()
{
}

std::string WebMidiAccessShim::getPortName( unsigned int portNumber, bool isInput )
{
  if( !checkWebMidiAvailability() )
    return "";
  char *ret = (char*) MAIN_THREAD_EM_ASM_INT( {
    var port = window._rtmidi_internals_get_port_by_number($0, $1);
    if( port == null)
      return null;
    var length = lengthBytesUTF8(port.name) + 1;
    var ret = _malloc(length);
    stringToUTF8(port.name, ret, length);
    return ret;
  }, portNumber, isInput);
  if (ret == nullptr)
      return "";
  std::string s = ret;
  free(ret);
  return s;
}

//*********************************************************************//
//  API: WEB MIDI
//  Class Definitions: MidiInWeb
//*********************************************************************//

MidiInWeb::MidiInWeb( const std::string &clientName, unsigned int queueSizeLimit )
  : MidiInApi( queueSizeLimit )
{
  initialize( clientName );
}

MidiInWeb::~MidiInWeb( void )
{
  closePort();
}

extern "C" void EMSCRIPTEN_KEEPALIVE rtmidi_onMidiMessageProc( MidiInApi::RtMidiInData* data, uint8_t* inputBytes, int32_t length, double domHighResTimeStamp )
{
  auto &message = data->message;
  message.bytes.resize(message.bytes.size() + length);
  memcpy(message.bytes.data(), inputBytes, length);
  // FIXME: handle timestamp
  data->callUserCallback( message.timeStamp, &message.bytes );
}

void MidiInWeb::openPort( unsigned int portNumber, const std::string &portName )
{
  if( !checkWebMidiAvailability() )
    return;
  if (open_port_number >= 0)
    return;

  MAIN_THREAD_EM_ASM( {
    // In Web MIDI API world, there is no step to open a port, but we have to register the input callback instead.
    var input = window._rtmidi_internals_get_port_by_number($0, true);
    input.onmidimessage = function(e) {
      // In RtMidi world, timestamps are delta time from previous message, while in Web MIDI world
      // timestamps are relative to window creation time (i.e. kind of absolute time with window "epoch" time).
      var rtmidiTimestamp = window._rtmidi_internals_latest_message_timestamp == 0.0 ? 0.0 : e.timeStamp - window._rtmidi_internals_latest_message_timestamp;
      window._rtmidi_internals_latest_message_timestamp = e.timeStamp;
      Module.ccall( 'rtmidi_onMidiMessageProc', 'void', ['number', 'array', 'number', 'number'], [$1, e.data, e.data.length, rtmidiTimestamp] );
    };
  }, portNumber, &inputData_ );
  open_port_number = portNumber;
  connected_ = true;
}

void MidiInWeb::openVirtualPort( const std::string &portName )
{

  errorString_ = "MidiInWeb::openVirtualPort: this function is not implemented for the Web MIDI API!";
  error( RtMidiError::WARNING, errorString_ );

}

void MidiInWeb::closePort( void )
{
  if( open_port_number < 0 )
    return;

  MAIN_THREAD_EM_ASM( {
    var input = _rtmidi_internals_get_port_by_number($0, true);
    if( input == null ) {
      console.log( "Port #" + $0 + " could not be found.");
      return;
    }
    // unregister event handler
    input.onmidimessage = null;
  }, open_port_number );
  open_port_number = -1;
  connected_ = false;
}

void MidiInWeb::setClientName( const std::string &clientName )
{
  client_name = clientName;
}

void MidiInWeb::setPortName( const std::string &portName )
{

  errorString_ = "MidiInWeb::setPortName: this function is not implemented for the Web MIDI API!";
  error( RtMidiError::WARNING, errorString_ );

}

unsigned int MidiInWeb::getPortCount( void )
{
  if( !checkWebMidiAvailability() )
    return 0;
  return MAIN_THREAD_EM_ASM_INT( { return _rtmidi_internals_midi_access.inputs.size; } );
}

std::string MidiInWeb::getPortName( unsigned int portNumber )
{
  if( !checkWebMidiAvailability() )
    return "";
  return shim->getPortName( portNumber, true );
}

void MidiInWeb::initialize( const std::string& clientName )
{
  ensureShim();
  setClientName( clientName );
}

//*********************************************************************//
//  API: WEB MIDI
//  Class Definitions: MidiOutWeb
//*********************************************************************//

MidiOutWeb::MidiOutWeb( const std::string &clientName )
{
  initialize( clientName );
}

MidiOutWeb::~MidiOutWeb( void )
{
  closePort();
}

void MidiOutWeb::openPort( unsigned int portNumber, const std::string &portName )
{
  if( !checkWebMidiAvailability() )
    return;
  if (open_port_number >= 0)
    return;
  // In Web MIDI API world, there is no step to open a port.

  open_port_number = portNumber;
  connected_ = true;
}

void MidiOutWeb::openVirtualPort( const std::string &portName )
{

  errorString_ = "MidiOutWeb::openVirtualPort: this function is not implemented for the Web MIDI API!";
  error( RtMidiError::WARNING, errorString_ );

}

void MidiOutWeb::closePort( void )
{
  // there is really nothing to do for output at JS side.
  open_port_number = -1;
  connected_ = false;
}

void MidiOutWeb::setClientName( const std::string &clientName )
{
  client_name = clientName;
}

void MidiOutWeb::setPortName( const std::string &portName )
{

  errorString_ = "MidiOutWeb::setPortName: this function is not implemented for the Web MIDI API!";
  error( RtMidiError::WARNING, errorString_ );

}

unsigned int MidiOutWeb::getPortCount( void )
{
  if( !checkWebMidiAvailability() )
    return 0;
  return MAIN_THREAD_EM_ASM_INT( { return _rtmidi_internals_midi_access.outputs.size; } );
}

std::string MidiOutWeb::getPortName( unsigned int portNumber )
{
  if( !checkWebMidiAvailability() )
    return "";
  return shim->getPortName( portNumber, false );
}

void MidiOutWeb::sendMessage( const unsigned char *message, size_t size )
{
  if( open_port_number < 0 )
    return;

  MAIN_THREAD_EM_ASM( {
    var output = _rtmidi_internals_get_port_by_number( $0, false );
    if( output == null ) {
      console.log( "Port #" + $0 + " could not be found.");
      return;
    }
    var buf = new ArrayBuffer ($2);
    var msg = new Uint8Array( buf );
    msg.set( new Uint8Array( Module.HEAPU8.buffer.slice( $1, $1 + $2 ) ) );
    output.send( msg );
  }, open_port_number, message, size );
}

void MidiOutWeb::initialize( const std::string& clientName )
{
  if ( shim.get() != nullptr )
    return;
  shim.reset( new WebMidiAccessShim() );
  setClientName( clientName );
}

#endif  // RTMIDI_API_WEBMIDI


//*********************************************************************//
//  API: ANDROID AMIDI
//
//  Written by Yellow Labrador, May 2023.
//  https://github.com/YellowLabrador/rtmidi
//  *********************************************************************//

#if defined(RTMIDI_API_AMIDI)

#include <cstdint>

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

static std::string androidClientName;

//*********************************************************************//
//  API: Android AMIDI
//  Class Definitions: MidiInAndroid
//*********************************************************************//

// The JVM running the app.  Set by JNI_OnLoad when the runtime loads the
// library this backend is built into, and resolved on demand otherwise.
//
// JNI_GetCreatedJavaVMs() is deliberately not linked: it lives in
// libnativehelper, which the NDK ships only from API 31, while AMidi itself is
// available from API 29.  Linking it would raise this backend's minimum to 31
// for no reason, so it is resolved with dlsym() in the cases where JNI_OnLoad
// has not already supplied the pointer.
static JavaVM* androidJvm = NULL;

// The app's com.yellowlab.rtmidi.MidiDeviceOpenedListener (contrib/java), as a
// global ref.  Looked up once, see androidGetListenerClass().
static jclass androidListenerClass = NULL;
static std::mutex androidListenerMutex;

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad( JavaVM* vm, void* /*reserved*/ )
{
  androidJvm = vm;

  // FindClass() sees the app's classes here, but not on a thread attached
  // natively.  An app that never calls openPort() need not bundle the class.
  JNIEnv *env = NULL;
  if ( vm->GetEnv( (void**) &env, JNI_VERSION_1_6 ) == JNI_OK ) {
    jclass listenerClass = env->FindClass( "com/yellowlab/rtmidi/MidiDeviceOpenedListener" );
    if ( listenerClass ) {
      std::lock_guard<std::mutex> lock( androidListenerMutex );
      if ( !androidListenerClass )
        androidListenerClass = (jclass) env->NewGlobalRef( listenerClass );
      env->DeleteLocalRef( listenerClass );
    } else {
      env->ExceptionClear();
    }
  }
  return JNI_VERSION_1_6;
}

static JavaVM* androidGetJvm() {
  if ( androidJvm ) return androidJvm;

  // JNI_OnLoad has not run, so this is a static initializer or a library the
  // runtime did not load itself.
  typedef jint (*GetCreatedJavaVMsFn)( JavaVM**, jsize, jsize* );
  static GetCreatedJavaVMsFn getCreatedJavaVMs =
    (GetCreatedJavaVMsFn) dlsym( RTLD_DEFAULT, "JNI_GetCreatedJavaVMs" );

  if ( !getCreatedJavaVMs ) {
    void *handle = dlopen( "libnativehelper.so", RTLD_NOW );
    if ( handle )
      getCreatedJavaVMs =
        (GetCreatedJavaVMsFn) dlsym( handle, "JNI_GetCreatedJavaVMs" );
  }
  if ( !getCreatedJavaVMs ) {
    LOGE( "Unable to resolve JNI_GetCreatedJavaVMs" );
    return NULL;
  }

  jsize found = 0;
  JavaVM *vm = NULL;
  if ( getCreatedJavaVMs( &vm, 1, &found ) != JNI_OK || found != 1 ) {
    LOGE( "No JVM found" );
    return NULL;
  }
  androidJvm = vm;
  return androidJvm;
}

static JNIEnv* androidGetThreadEnv() {
  JavaVM* pjvms = androidGetJvm();
  jsize jvmsFound = pjvms ? 1 : 0;
  jint result = pjvms ? JNI_OK : JNI_ERR;

  // Something went terribly wrong, no JVM was found
  if (jvmsFound != 1 || result != JNI_OK) {
      LOGE("No JVM found");
      return NULL;
  }

  // Get the JNIEnv for the current thread
  JNIEnv* env = NULL;
  int rc = pjvms->GetEnv((void**)&env, JNI_VERSION_1_6);

  // The current thread was not attached to the JVM. Add it to the JVM
  if (rc == JNI_EDETACHED) {
      pjvms->AttachCurrentThreadAsDaemon(&env, NULL);
  }

  // Neither way to retrieve the JNIEnv worked
  if (env == NULL) {
      LOGE("Unable to retrieve JNI environment");
  }

  return env;
}

static jobject androidGetContext(JNIEnv *env) {
  if (env == NULL) {
    LOGE("androidGetContext: no JNI environment");
    return NULL;
  }

  // Native threads have no frame of their own to free local references.
  if (env->PushLocalFrame(8) != JNI_OK) {
    env->ExceptionClear();
    return NULL;
  }
  auto activityThread = env->FindClass("android/app/ActivityThread");
  auto currentActivityThread = env->GetStaticMethodID(activityThread, "currentActivityThread", "()Landroid/app/ActivityThread;");
  auto at = env->CallStaticObjectMethod(activityThread, currentActivityThread);
  if (at == NULL) {
      LOGE("Unable to locate the global ActivityThread");
      env->PopLocalFrame(NULL);
      return NULL;
  }

  auto getApplication = env->GetMethodID(activityThread, "getApplication", "()Landroid/app/Application;");
  auto context = env->CallObjectMethod(at, getApplication);
  if (context == NULL) {
      LOGE("Application context was NULL");
  }

  return env->PopLocalFrame(context);
}

// The listener class.  When JNI_OnLoad could not find it, it is loaded through
// the app's class loader, which works on any thread.  NULL if it is not bundled.
static jclass androidGetListenerClass(JNIEnv *env) {
  std::lock_guard<std::mutex> lock(androidListenerMutex);
  if (androidListenerClass) return androidListenerClass;

  if (env->PushLocalFrame(8) != JNI_OK) {
    env->ExceptionClear();
    return NULL;
  }
  auto context = androidGetContext(env);
  jobject loader = NULL;
  if (context) {
    auto contextClass = env->FindClass("android/content/Context");
    auto getClassLoader = env->GetMethodID(contextClass, "getClassLoader", "()Ljava/lang/ClassLoader;");
    loader = env->CallObjectMethod(context, getClassLoader);
  }
  if (loader && !env->ExceptionCheck()) {
    auto loaderClass = env->FindClass("java/lang/ClassLoader");
    auto loadClass = env->GetMethodID(loaderClass, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    auto name = env->NewStringUTF("com.yellowlab.rtmidi.MidiDeviceOpenedListener");
    auto listenerClass = env->CallObjectMethod(loader, loadClass, name);
    if (listenerClass && !env->ExceptionCheck())
      androidListenerClass = (jclass) env->NewGlobalRef(listenerClass);
  }
  if (env->ExceptionCheck()) {
    env->ExceptionDescribe();
    env->ExceptionClear();
  }
  env->PopLocalFrame(NULL);
  return androidListenerClass;
}

static jobject androidGetMidiManager(JNIEnv *env, jobject context) {
  // MidiManager midiManager = (MidiManager) getSystemService(Context.MIDI_SERVICE);
  if (env->PushLocalFrame(8) != JNI_OK) {
    env->ExceptionClear();
    return NULL;
  }
  auto contextClass = env->FindClass("android/content/Context");
  auto getServiceMethod = env->GetMethodID(contextClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
  auto manager = env->CallObjectMethod(context, getServiceMethod, env->NewStringUTF("midi"));
  if (env->ExceptionCheck()) {
    env->ExceptionClear();
    manager = NULL;
  }
  return env->PopLocalFrame(manager);
}

// Entries of one device share its global ref, so release each ref once.
static void androidReleasePorts(JNIEnv *env, std::vector<AndroidMidiPort> &ports) {
  jobject previous = NULL;
  for (const AndroidMidiPort &entry : ports) {
    if (entry.device != previous) {
      env->DeleteGlobalRef(entry.device);
      previous = entry.device;
    }
  }
  ports.clear();
}

// Lists a USB MIDI 1.0 device's ports per direction from its configuration
// descriptors, as the iJack string index of each port (0 when unnamed).
//
// Ports are the embedded jacks of each MIDI Streaming endpoint, in descriptor
// order, which is how Linux numbers its substreams and how Android 13 counts
// ports.  An input port (device->host) belongs to an IN endpoint, whose jacks
// are embedded OUT jacks; an output port belongs to an OUT endpoint, whose
// jacks are embedded IN jacks.  Linux names them the same way.  Alternate
// setting 0 only, since a MIDI 2.0 device's UMP interface is alternate setting 1.
static void androidParseUsbMidiJacks(const uint8_t *d, size_t n,
                                     std::vector<int> &inputs, std::vector<int> &outputs) {
  std::map<int, int> inJackString, outJackString;  // jack id -> iJack
  std::vector<int> inEndpointJacks, outEndpointJacks;
  bool midiStreaming = false, endpointIsIn = false;
  for (size_t i = 0; i + 1 < n; ) {
    const size_t len = d[i];
    if (len < 2 || i + len > n) break;
    const uint8_t *x = d + i;
    switch (x[1]) {
      case 0x04:  // INTERFACE: audio class, MIDI Streaming subclass
        midiStreaming = len >= 9 && x[5] == 1 && x[6] == 3 && x[3] == 0;
        break;
      case 0x05:  // ENDPOINT
        if (midiStreaming && len >= 3) endpointIsIn = (x[2] & 0x80) != 0;
        break;
      case 0x24:  // CS_INTERFACE
        if (!midiStreaming || len < 6) break;
        if (x[2] == 0x02)       // MIDI_IN_JACK: iJack follows bJackID
          inJackString[x[4]] = x[5];
        else if (x[2] == 0x03)  // MIDI_OUT_JACK: iJack follows bNrInputPins source pairs
          outJackString[x[4]] = 6 + 2 * (size_t) x[5] < len ? x[6 + 2 * x[5]] : 0;
        break;
      case 0x25:  // CS_ENDPOINT, MS_GENERAL: bNumEmbMIDIJack, baAssocJackID
        if (!midiStreaming || len < 4 || x[2] != 0x01) break;
        for (size_t j = 0; j < x[3] && 4 + j < len; j++)
          (endpointIsIn ? inEndpointJacks : outEndpointJacks).push_back(x[4 + j]);
        break;
    }
    i += len;
  }
  inputs.clear();
  outputs.clear();
  for (int jack : inEndpointJacks)
    inputs.push_back(outJackString.count(jack) ? outJackString[jack] : 0);
  for (int jack : outEndpointJacks)
    outputs.push_back(inJackString.count(jack) ? inJackString[jack] : 0);
}

// What a USB MIDI device's own descriptors say about its ports: one name per
// port, "<product> <jack name>", or for an unnamed jack "<product> MIDI <n>",
// the same as Linux, or just "<product>" when it is the only port in its
// direction.  Names are empty when not requested.
struct AndroidUsbPorts {
  std::vector<std::string> inputs;
  std::vector<std::string> outputs;
};

// Reads a USB string descriptor, or "" when there is none.
static std::string androidUsbString(JNIEnv *env, jobject connection, jmethodID controlTransfer,
                                    int index, int language) {
  if (index == 0) return "";
  jbyteArray buffer = env->NewByteArray(255);
  // GET_DESCRIPTOR (6), device-to-host standard request (0x80), type STRING (3).
  jint n = env->CallIntMethod(connection, controlTransfer, 0x80, 0x06, 0x0300 | index,
                              language, buffer, 255, 1000);
  std::string result;
  if (n >= 2) {
    jbyte bytes[255];
    env->GetByteArrayRegion(buffer, 0, n, bytes);
    jsize length = std::min<jsize>(n, (uint8_t) bytes[0]);
    std::vector<jchar> utf16;
    for (jsize i = 2; i + 1 < length; i += 2)
      utf16.push_back((jchar) ((uint8_t) bytes[i] | ((uint8_t) bytes[i + 1] << 8)));
    jstring text = env->NewString(utf16.data(), (jsize) utf16.size());
    const char *chars = text ? env->GetStringUTFChars(text, NULL) : NULL;
    if (chars) {
      result = chars;
      env->ReleaseStringUTFChars(text, chars);
    }
    if (text) env->DeleteLocalRef(text);
  }
  env->DeleteLocalRef(buffer);
  return result;
}

// Reads a USB MIDI device's ports from its descriptors.  Only with USB
// permission the application already holds: this never prompts.  Returns false
// when the device is not USB or its descriptors cannot be read.
static bool androidReadUsbPorts(JNIEnv *env, jobject context, jobject deviceInfo,
                                bool withNames, AndroidUsbPorts &ports) {
  if (env->PushLocalFrame(32) != JNI_OK) return false;
  bool found = false;

  auto infoClass = env->FindClass("android/media/midi/MidiDeviceInfo");
  auto bundleClass = env->FindClass("android/os/Bundle");
  auto contextClass = env->FindClass("android/content/Context");
  auto managerClass = env->FindClass("android/hardware/usb/UsbManager");
  auto deviceClass = env->FindClass("android/hardware/usb/UsbDevice");
  auto connectionClass = env->FindClass("android/hardware/usb/UsbDeviceConnection");

  // MidiDeviceInfo.TYPE_USB is 1; PROPERTY_USB_DEVICE is "usb_device".
  if (env->CallIntMethod(deviceInfo, env->GetMethodID(infoClass, "getType", "()I")) == 1) {
    auto props = env->CallObjectMethod(deviceInfo,
        env->GetMethodID(infoClass, "getProperties", "()Landroid/os/Bundle;"));
    auto usbDevice = props ? env->CallObjectMethod(props,
        env->GetMethodID(bundleClass, "getParcelable", "(Ljava/lang/String;)Landroid/os/Parcelable;"),
        env->NewStringUTF("usb_device")) : NULL;
    auto manager = usbDevice ? env->CallObjectMethod(context,
        env->GetMethodID(contextClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;"),
        env->NewStringUTF("usb")) : NULL;
    if (manager && env->CallBooleanMethod(manager,
          env->GetMethodID(managerClass, "hasPermission", "(Landroid/hardware/usb/UsbDevice;)Z"), usbDevice)) {
      // Claims no interface, so the MIDI service's use of the device is undisturbed.
      auto connection = env->CallObjectMethod(manager,
          env->GetMethodID(managerClass, "openDevice",
                           "(Landroid/hardware/usb/UsbDevice;)Landroid/hardware/usb/UsbDeviceConnection;"),
          usbDevice);
      if (connection) {
        auto raw = (jbyteArray) env->CallObjectMethod(connection,
            env->GetMethodID(connectionClass, "getRawDescriptors", "()[B"));
        if (raw) {
          jsize n = env->GetArrayLength(raw);
          std::vector<uint8_t> bytes(n);
          env->GetByteArrayRegion(raw, 0, n, (jbyte*) bytes.data());
          std::vector<int> inputJacks, outputJacks;
          androidParseUsbMidiJacks(bytes.data(), bytes.size(), inputJacks, outputJacks);
          found = !inputJacks.empty() || !outputJacks.empty();

          std::string product;
          int language = 0x0409;  // US English, if the device lists no languages
          auto controlTransfer = env->GetMethodID(connectionClass, "controlTransfer", "(IIII[BII)I");
          if (found && withNames) {
            auto jProduct = (jstring) env->CallObjectMethod(usbDevice,
                env->GetMethodID(deviceClass, "getProductName", "()Ljava/lang/String;"));
            const char *chars = jProduct ? env->GetStringUTFChars(jProduct, NULL) : NULL;
            if (chars) {
              product = chars;
              env->ReleaseStringUTFChars(jProduct, chars);
            }
            // String descriptor 0 lists the supported language IDs.
            jbyteArray buffer = env->NewByteArray(4);
            if (env->CallIntMethod(connection, controlTransfer, 0x80, 0x06, 0x0300, 0,
                                   buffer, 4, 1000) >= 4) {
              jbyte id[4];
              env->GetByteArrayRegion(buffer, 0, 4, id);
              language = (uint8_t) id[2] | ((uint8_t) id[3] << 8);
            }
          }
          for (int pass = 0; pass < 2; pass++) {
            const std::vector<int> &jacks = pass == 0 ? inputJacks : outputJacks;
            std::vector<std::string> &names = pass == 0 ? ports.inputs : ports.outputs;
            names.clear();
            for (size_t p = 0; p < jacks.size(); p++) {
              std::string name;
              if (withNames) {
                std::string jack = androidUsbString(env, connection, controlTransfer,
                                                    jacks[p], language);
                if (!jack.empty()) name = product + " " + jack;
                else if (jacks.size() == 1) name = product;  // a lone port needs no number
                else name = product + " MIDI " + std::to_string(p + 1);
              }
              names.push_back(name);
            }
          }
        }
        env->CallVoidMethod(connection, env->GetMethodID(connectionClass, "close", "()V"));
      }
    }
  }

  if (env->ExceptionCheck()) {
    env->ExceptionClear();
    found = false;
  }
  env->PopLocalFrame(NULL);
  return found;
}

// Devices (by MidiDeviceInfo id) whose open timed out.  The framework thread
// opening one may never return, so each further attempt would park another
// Binder thread: refuse them until the open completes or the id is unlisted.
static std::mutex androidTimedOutMutex;
static std::set<int> androidTimedOut;

static bool androidOpenTimedOut(int deviceId) {
  std::lock_guard<std::mutex> lock(androidTimedOutMutex);
  return androidTimedOut.count(deviceId) != 0;
}

static void androidSetOpenTimedOut(int deviceId, bool timedOut) {
  std::lock_guard<std::mutex> lock(androidTimedOutMutex);
  if (timedOut) androidTimedOut.insert(deviceId);
  else androidTimedOut.erase(deviceId);
}

// Forgets devices that are no longer listed: a replugged device gets a new id.
static void androidForgetUnlistedDevices(const std::set<int> &listed) {
  std::lock_guard<std::mutex> lock(androidTimedOutMutex);
  for (auto it = androidTimedOut.begin(); it != androidTimedOut.end(); )
    it = listed.count(*it) ? std::next(it) : androidTimedOut.erase(it);
}

static void androidRefreshMidiDevices(JNIEnv *env, jobject context, bool isOutput,
                                      std::vector<AndroidMidiPort> &ports) {
  androidReleasePorts(env, ports);
  if (env->PushLocalFrame(16) != JNI_OK) {
    env->ExceptionClear();
    return;
  }

  auto midiService = androidGetMidiManager(env, context);

  // MidiDeviceInfo[] devInfos = mMidiManager.getDevices();
  auto midiMgrClass = env->FindClass("android/media/midi/MidiManager");
  auto getDevicesMethod = env->GetMethodID(midiMgrClass, "getDevices", "()[Landroid/media/midi/MidiDeviceInfo;");
  auto jDevices = midiService ? (jobjectArray) env->CallObjectMethod(midiService, getDevicesMethod) : NULL;
  if (env->ExceptionCheck()) {
    env->ExceptionClear();
    jDevices = NULL;
  }
  if (jDevices == NULL) {
    LOGE("androidRefreshMidiDevices: no MIDI service or device list");
    env->PopLocalFrame(NULL);
    return;
  }

  auto deviceInfoClass = env->FindClass("android/media/midi/MidiDeviceInfo");
  auto getIdMethod = env->GetMethodID(deviceInfoClass, "getId", "()I");
  auto getInputPortCountMethod = env->GetMethodID(deviceInfoClass, "getInputPortCount", "()I");
  auto getOutputPortCountMethod = env->GetMethodID(deviceInfoClass, "getOutputPortCount", "()I");
  std::set<int> listed;

  // Android 12 and earlier count a USB device's output ports and report that
  // number for both directions.  Android 13 counts each direction.
  const bool countsBothDirections = android_get_device_api_level() < 33;

  jsize len = env->GetArrayLength((jarray)jDevices);
  for (int i=0; i<len; i++) {
      auto jDeviceInfo = env->GetObjectArrayElement(jDevices, i);
      int deviceId = env->CallIntMethod(jDeviceInfo, getIdMethod);
      listed.insert(deviceId);

      // An output port (host->device) is one of the device's input ports.
      int numPorts = env->CallIntMethod(jDeviceInfo, isOutput ? getInputPortCountMethod : getOutputPortCountMethod);
      int usbInputs = -1, usbOutputs = -1;
      AndroidUsbPorts usb;
      if (numPorts > 0 && (countsBothDirections || kAndroidPortNames) &&
          androidReadUsbPorts(env, context, jDeviceInfo, kAndroidPortNames, usb)) {
          usbInputs = (int) usb.inputs.size();
          usbOutputs = (int) usb.outputs.size();
          if (countsBothDirections && !isOutput && usbInputs < numPorts)
              numPorts = usbInputs;
      }
      const std::vector<std::string> &usbNames = isOutput ? usb.outputs : usb.inputs;
      if (numPorts > 0) {
          auto ref = env->NewGlobalRef(jDeviceInfo);
          for (int p=0; p<numPorts; p++) {
              AndroidMidiPort entry = { ref, deviceId, p, isOutput, usbInputs, usbOutputs,
                                        p < (int) usbNames.size() ? usbNames[p] : "" };
              ports.push_back(entry);
          }
      }
      env->DeleteLocalRef(jDeviceInfo);
  }
  androidForgetUnlistedDevices(listed);
  env->PopLocalFrame(NULL);
}

// Calls MidiDevice.close(), after AMidiDevice_release(), so that the MIDI
// service drops its connection now rather than when the object is collected.
static void androidCloseJavaDevice(JNIEnv *env, jobject device) {
  if (env == NULL || device == NULL) return;
  auto deviceClass = env->GetObjectClass(device);
  env->CallVoidMethod(device, env->GetMethodID(deviceClass, "close", "()V"));
  if (env->ExceptionCheck()) env->ExceptionClear();
  env->DeleteLocalRef(deviceClass);
}

// Releases an open device and its Java MidiDevice (a global ref).
static void androidReleaseDevice(JNIEnv *env, AMidiDevice *device, jobject javaDevice) {
  if (device) AMidiDevice_release(device);
  if (javaDevice && env) {
    androidCloseJavaDevice(env, javaDevice);
    env->DeleteGlobalRef(javaDevice);
  }
}


extern "C"
JNIEXPORT void JNICALL
Java_com_yellowlab_rtmidi_MidiDeviceOpenedListener_midiDeviceOpened(JNIEnv *env, jclass /*clazz*/,
                                                                    jobject midi_device, jlong targetPtr, jboolean /*isOutput*/) {
  // Runs on a Binder thread.  It must not throw and must not touch the RtMidi
  // object, which may be gone; it fills in the request and wakes openPort().
  std::unique_ptr< std::shared_ptr<AndroidOpenRequest> > holder(
      reinterpret_cast< std::shared_ptr<AndroidOpenRequest>* >(targetPtr));
  std::shared_ptr<AndroidOpenRequest> req = *holder;

  AMidiDevice *device = NULL;
  AMidiInputPort *inputPort = NULL;
  AMidiOutputPort *outputPort = NULL;
  std::string failure;

  if (midi_device == NULL) {
    failure = "Android could not open the device";
  } else if (AMidiDevice_fromJava(env, midi_device, &device) != AMEDIA_OK || device == NULL) {
    failure = "AMidiDevice_fromJava() failed";
  } else if (req->isOutput) {
    media_status_t st = AMidiInputPort_open(device, req->portIndex, &inputPort);
    if (st != AMEDIA_OK) failure = "AMidiInputPort_open() failed (" + std::to_string((int) st) + ")";
  } else {
    media_status_t st = AMidiOutputPort_open(device, req->portIndex, &outputPort);
    if (st != AMEDIA_OK) failure = "AMidiOutputPort_open() failed (" + std::to_string((int) st) + ")";
  }

  std::lock_guard<std::mutex> lock(req->mutex);
  if (req->abandoned || !failure.empty()) {
    // Nobody will use these: openPort() timed out, or the open failed part way.
    if (inputPort) AMidiInputPort_close(inputPort);
    if (outputPort) AMidiOutputPort_close(outputPort);
    if (device) AMidiDevice_release(device);
    androidCloseJavaDevice(env, midi_device);
    // A late answer: the device can be tried again.
    if (req->abandoned) androidSetOpenTimedOut(req->deviceId, false);
  } else {
    req->device = device;
    req->inputPort = inputPort;
    req->outputPort = outputPort;
    req->javaDevice = env->NewGlobalRef(midi_device);
  }
  req->failure = failure;
  req->done = true;
  req->cond.notify_all();
}

// Asks MidiManager to open a device.  Returns false, with the reason in
// errorText, when the request could not be made and no answer will come.
static bool androidOpenDevice(jobject deviceInfo, jlong target, bool isOutput,
                              std::string &errorText) {
    auto env = androidGetThreadEnv();
    if (env == NULL) {
      errorText = "no JNI environment";
      return false;
    }
    if (env->PushLocalFrame(8) != JNI_OK) {
      env->ExceptionClear();
      errorText = "out of JNI local references";
      return false;
    }
    auto context = androidGetContext(env);
    auto midiMgr = context ? androidGetMidiManager(env, context) : NULL;
    if (midiMgr == NULL) {
      errorText = "the MIDI service is not available";
      env->PopLocalFrame(NULL);
      return false;
    }

    // openDevice(MidiDeviceInfo deviceInfo, OnDeviceOpenedListener listener, Handler handler)
    auto midiMgrClass = env->GetObjectClass(midiMgr);
    auto openDevicesMethod = env->GetMethodID(midiMgrClass, "openDevice", "(Landroid/media/midi/MidiDeviceInfo;Landroid/media/midi/MidiManager$OnDeviceOpenedListener;Landroid/os/Handler;)V");

    // A null Handler makes MidiManager call onDeviceOpened() on the Binder
    // thread that delivers the result.  It must not be the main looper: the
    // callback calls AMidiDevice_fromJava() and AMidi*Port_open(), which block,
    // so on the main looper the UI freezes, and an application that calls
    // openPort() from the UI thread deadlocks.  Google's MIDI samples do the
    // same.
    jobject handler = NULL;

    jclass listenerClass = androidGetListenerClass(env);
    if (!listenerClass) {
      errorText = "class com.yellowlab.rtmidi.MidiDeviceOpenedListener not found (it must be "
                  "bundled in the APK, from contrib/java)";
      LOGE("%s", errorText.c_str());
      env->PopLocalFrame(NULL);
      return false;
    }

    auto listenerCtor = env->GetMethodID(listenerClass, "<init>", "(JZ)V");
    auto listener = env->NewObject(listenerClass, listenerCtor, target, isOutput);

    if (listener) env->CallVoidMethod(midiMgr, openDevicesMethod, deviceInfo, listener, handler);
    bool requested = listener && !env->ExceptionCheck();
    if (!requested) {
      // Thrown before MidiManager registered the callback, so no answer will come.
      env->ExceptionDescribe();
      env->ExceptionClear();
      errorText = "MidiManager.openDevice() failed (see logcat)";
      LOGE("%s", errorText.c_str());
    }
    env->PopLocalFrame(NULL);
    return requested;
}

// True when opening this port would hang: Android 12 and earlier cannot open a
// USB MIDI device with fewer inputs than outputs (see androidOpenPort()).
// Known only when the device's descriptors could be read.
static bool androidOpenWouldHang(const AndroidMidiPort &port) {
  return port.usbInputs >= 0 && port.usbInputs < port.usbOutputs &&
         android_get_device_api_level() < 33;
}

#if !defined(RTMIDI_ANDROID_LEGACY_PORT_NAMES)
// A port RtMidi refuses to open is named in parentheses, so that a user can
// see it is unavailable before choosing it.
static std::string androidMarkUnopenable(const AndroidMidiPort &port, const std::string &name) {
  return androidOpenWouldHang(port) ? "(" + name + ")" : name;
}
#endif

static std::string androidPortNameInFrame(JNIEnv *env, const std::vector<AndroidMidiPort> &entries,
                                          unsigned int portNumber) {
  if (portNumber >= entries.size()) {
    LOGE("androidPortName: Invalid port number");
    return "";
  }

  // String deviceName = devInfo.getProperties().getString(MidiDeviceInfo.PROPERTY_NAME);
  auto deviceInfoClass = env->FindClass("android/media/midi/MidiDeviceInfo");
  auto getPropsMethod = env->GetMethodID(deviceInfoClass, "getProperties", "()Landroid/os/Bundle;");
  auto bundle = env->CallObjectMethod(entries[portNumber].device, getPropsMethod);

  auto bundleClass = env->FindClass("android/os/Bundle");
  auto getStringMethod = env->GetMethodID(bundleClass, "getString", "(Ljava/lang/String;)Ljava/lang/String;");
  auto jPortName = (jstring) env->CallObjectMethod(bundle, getStringMethod, env->NewStringUTF("name"));
  if (jPortName == nullptr) {
    jPortName = (jstring) env->CallObjectMethod(bundle, getStringMethod, env->NewStringUTF("product"));
    if (jPortName == nullptr) {
      return "";
    }
  }
  
  auto portNameChars = env->GetStringUTFChars(jPortName, NULL);
  if (portNameChars == NULL) return "";
  auto name = std::string(portNameChars);
  env->ReleaseStringUTFChars(jPortName, portNameChars);

  // The bundle names the DEVICE, not the port: Android composes it as
  // manufacturer + " " + product, and passes no port names for USB devices on
  // any version.  With RTMIDI_ANDROID_LEGACY_PORT_NAMES every port keeps exactly
  // that name, as in RtMidi 6.0 and earlier.
#if !defined(RTMIDI_ANDROID_LEGACY_PORT_NAMES)
  const AndroidMidiPort &entry = entries[portNumber];
  if (!entry.usbName.empty()) return androidMarkUnopenable(entry, entry.usbName);

  // The port's own name, which only applications' virtual devices publish.
  auto getPortsMethod = env->GetMethodID(deviceInfoClass, "getPorts",
                                         "()[Landroid/media/midi/MidiDeviceInfo$PortInfo;");
  auto ports = (jobjectArray) env->CallObjectMethod(entry.device, getPortsMethod);
  if (ports) {
    auto portInfoClass = env->FindClass("android/media/midi/MidiDeviceInfo$PortInfo");
    auto getTypeMethod = env->GetMethodID(portInfoClass, "getType", "()I");
    auto getNumberMethod = env->GetMethodID(portInfoClass, "getPortNumber", "()I");
    auto getNameMethod = env->GetMethodID(portInfoClass, "getName", "()Ljava/lang/String;");
    // PortInfo.TYPE_OUTPUT is 2: a port we read from, which is what an input
    // port is from the application's point of view.
    const jint wantType = entry.isOutput ? 1 : 2;
    jsize n = env->GetArrayLength(ports);
    for (jsize i = 0; i < n; i++) {
      auto pi = env->GetObjectArrayElement(ports, i);
      if (env->CallIntMethod(pi, getTypeMethod) != wantType ||
          env->CallIntMethod(pi, getNumberMethod) != entry.portIndex) {
        env->DeleteLocalRef(pi);
        continue;
      }
      auto jn = (jstring) env->CallObjectMethod(pi, getNameMethod);
      env->DeleteLocalRef(pi);
      if (jn) {
        auto c = env->GetStringUTFChars(jn, NULL);
        std::string portName = c ? c : "";
        if (c) env->ReleaseStringUTFChars(jn, c);
        if (!portName.empty()) return androidMarkUnopenable(entry, name + " " + portName);
      }
      break;
    }
  }

  // Nothing names the port, typically a USB device without permission: number
  // it as Linux numbers unnamed ports, unless it is the device's only port in
  // this direction.
  int devicePorts = 0;
  for (const AndroidMidiPort &other : entries)
    if (other.device == entry.device) devicePorts++;
  if (devicePorts > 1) name += " MIDI " + std::to_string(entry.portIndex + 1);
  return androidMarkUnopenable(entry, name);
#endif
  return name;
}

static std::string androidPortName(JNIEnv *env, const std::vector<AndroidMidiPort> &entries,
                                   unsigned int portNumber) {
  if (env->PushLocalFrame(16) != JNI_OK) {
    env->ExceptionClear();
    return "";
  }
  std::string name = androidPortNameInFrame(env, entries, portNumber);
  if (env->ExceptionCheck()) env->ExceptionClear();
  env->PopLocalFrame(NULL);
  return name;
}

MidiInAndroid :: MidiInAndroid( const std::string &clientName, unsigned int queueSizeLimit )
  : MidiInApi( queueSizeLimit ) {
  MidiInAndroid::initialize( clientName );
}

void MidiInAndroid :: initialize( const std::string& clientName ) {
  androidClientName = clientName;
  connect();
}

// Opens one port of a device and waits for the result.  Returns the request on
// success, or NULL after putting the reason in errorText.  Called on the
// caller's thread, so an error raised from openPort() reaches the caller as an
// ordinary RtMidiError rather than escaping from a Binder thread.
static std::shared_ptr<AndroidOpenRequest> androidOpenPort(const AndroidMidiPort &port,
                                                           std::string &errorText) {
  static const char *const kAsymmetric =
      "Android 12 and earlier cannot open a USB MIDI device that has fewer input "
      "ports than output ports: the system hangs opening it, and the device stays "
      "unusable by every application until it is unplugged.  Fixed in Android 13";

  // The descriptors say this device would hang the system: do not try.
  if (androidOpenWouldHang(port)) {
    errorText = "the device has " + std::to_string(port.usbInputs) + " input and " +
                std::to_string(port.usbOutputs) + " output ports.  " + kAsymmetric;
    return NULL;
  }

  const std::string timeout = std::to_string(kAndroidOpenTimeout.count()) + " ms";
  const std::string explanation = android_get_device_api_level() < 33 ?
                                  std::string(".  ") + kAsymmetric : std::string();

  // An earlier open never answered: another would only park another thread.
  if (androidOpenTimedOut(port.deviceId)) {
    errorText = "the device did not answer an earlier attempt to open it within " + timeout +
                ", and is not tried again until it answers or is reconnected" + explanation;
    return NULL;
  }

  auto req = std::make_shared<AndroidOpenRequest>();
  req->isOutput = port.isOutput;
  req->portIndex = port.portIndex;
  req->deviceId = port.deviceId;

  auto holder = new std::shared_ptr<AndroidOpenRequest>(req);
  if (!androidOpenDevice(port.device, reinterpret_cast<jlong>(holder), port.isOutput, errorText)) {
    delete holder;
    return NULL;
  }

  std::unique_lock<std::mutex> lock(req->mutex);
  if (!req->cond.wait_for(lock, kAndroidOpenTimeout, [&]{ return req->done; })) {
    req->abandoned = true;
    androidSetOpenTimedOut(port.deviceId, true);
    errorText = "the device did not open within " + timeout + explanation;
    return NULL;
  }
  if (!req->failure.empty()) {
    errorText = req->failure;
    return NULL;
  }
  return req;
}

void MidiInAndroid :: connect() {
  auto env = androidGetThreadEnv();
  if (env == NULL) return;

  auto context = androidGetContext(env);
  if (context == NULL) return;

  androidRefreshMidiDevices(env, context, false, ports_);

  env->DeleteLocalRef(context);
}

MidiInAndroid :: ~MidiInAndroid() {
  // Attach first: releasing a device on a thread unknown to the JVM aborts.
  auto env = androidGetThreadEnv();

  // Close a connection if it exists.  An open input port's thread would
  // otherwise go on using this object after it is deleted.
  MidiInAndroid::closePort();

  if (env == NULL) return;

  androidReleasePorts(env, ports_);

  androidClientName = "";
}

void MidiInAndroid :: openPort(unsigned int portNumber, const std::string &portName) {
  if (portNumber >= ports_.size()) {
    errorString_ = "MidiInAndroid::openPort: Invalid port number";
    error( RtMidiError::INVALID_PARAMETER, errorString_ );

    return;
  }

  if (connected_) {
    errorString_ = "MidiInAndroid::openPort: A port is already open";
    error( RtMidiError::INVALID_USE, errorString_ );

    return;
  }

  std::string why;
  auto req = androidOpenPort(ports_[portNumber], why);
  if (!req) {
    errorString_ = "MidiInAndroid::openPort: " + why + ".";
    error( RtMidiError::DRIVER_ERROR, errorString_ );
    return;
  }
  receiveDevice = req->device;
  midiOutputPort = req->outputPort;
  javaDevice = req->javaDevice;

  // Set before the thread starts, so a closePort() straight after this sees an
  // open port and tears it down.
  inputData_.message.bytes.clear();
  inputData_.continueSysex = false;
  inputData_.firstMessage = true;
  runningStatus_ = 0;
  reading = true;
  if (pthread_create(&readThread, NULL, MidiInAndroid::pollMidi, this) != 0) {
    reading = false;
    AMidiOutputPort_close(midiOutputPort);
    androidReleaseDevice(androidGetThreadEnv(), receiveDevice, javaDevice);
    midiOutputPort = NULL;
    receiveDevice = NULL;
    javaDevice = NULL;
    errorString_ = "MidiInAndroid::openPort: could not start the input thread.";
    error( RtMidiError::THREAD_ERROR, errorString_ );
    return;
  }
  connected_ = true;
}

void MidiInAndroid :: openVirtualPort(const std::string &portName) {
  errorString_ = "MidiInAndroid::openVirtualPort: this function is not implemented for the Android API!";
  error( RtMidiError::WARNING, errorString_ );
}

unsigned int MidiInAndroid :: getPortCount() {
  connect();
  return ports_.size();
}

std::string MidiInAndroid :: getPortName(unsigned int portNumber) {
  auto env = androidGetThreadEnv();
  if (env == NULL) return "";
  return androidPortName(env, ports_, portNumber);
}

void MidiInAndroid :: closePort() {
  // Don't try to close a port before it was open
  if (!connected_) {
    return;
  }

  // The input thread may already have stopped by itself, when the device went
  // away (see pollMidi()).  The port is still open until here, and the thread
  // still has to be joined.
  reading = false;
  pthread_join(readThread, NULL);

  // AMidiDevice_release() aborts on a thread not attached to the JVM.
  auto env = androidGetThreadEnv();
  if (midiOutputPort) AMidiOutputPort_close(midiOutputPort);
  androidReleaseDevice(env, receiveDevice, javaDevice);
  receiveDevice = NULL;
  midiOutputPort = NULL;
  javaDevice = NULL;
  connected_ = false;
}

void MidiInAndroid:: setClientName(const std::string& clientName) {
  androidClientName = clientName;
}

void MidiInAndroid :: setPortName(const std::string &portName) {
  errorString_ = "MidiInAndroid::setPortName: this function is not implemented for the Android API!";
  error( RtMidiError::WARNING, errorString_ );
}

void* MidiInAndroid :: pollMidi(void* context) {
  auto self = (MidiInAndroid*) context;

  // An AMidi packet carries up to 1015 bytes, and AMidiOutputPort_receive()
  // silently truncates it to the buffer it is given.  A smaller buffer loses
  // the rest of every larger packet, including a SysEx message's closing F7.
  uint8_t incoming[1024];

  while (self->reading) {
    int32_t opcode = 0;
    size_t numBytesReceived = 0;
    int64_t timestamp = 0;
    ssize_t numMessagesReceived = AMidiOutputPort_receive(
        self->midiOutputPort, &opcode, incoming, sizeof(incoming),
        &numBytesReceived, &timestamp);

    if (numMessagesReceived < 0) {
      // WARNING, not SYSTEM_ERROR: this runs on the input thread, and error()
      // throws for any type above DEBUG_WARNING when no error callback is set.
      // An exception leaving a pthread terminates the process, which no caller
      // can catch.  The other backends' input threads do not report at all.
      // Typically the device was disconnected.  The port stays open, with no
      // input, until closePort().
      LOGE("MidiInAndroid::pollMidi: input stopped, AMidiOutputPort_receive() returned %d",
           (int) numMessagesReceived);
      self->errorString_ = "MidiInAndroid::pollMidi: error receiving MIDI data";
      self->error( RtMidiError::WARNING, self->errorString_ );
      self->reading = false;
      break;
    }

    if (numMessagesReceived == 0) {
      // AMidiOutputPort_receive() does not block.  Sleep only when there is
      // nothing queued, so a burst is drained at once.
      usleep(2000);
      continue;
    }

    // AMidi timestamps are nanoseconds; RtMidi reports seconds.
    if (opcode == AMIDI_OPCODE_DATA && numBytesReceived > 0)
      self->parse( incoming, numBytesReceived, timestamp * 1e-9 );
  }

  return NULL;
}

// Length of a message from its status byte, or 0 for SysEx, which ends at F7.
static size_t androidMessageLength( unsigned char status ) {
  if ( status < 0xF0 ) return ( status & 0xE0 ) == 0xC0 ? 2 : 3;
  switch ( status ) {
    case 0xF0: return 0;
    case 0xF1: case 0xF3: return 2;
    case 0xF2: return 3;
    default: return 1;
  }
}

// AMidi delivers a byte stream, not messages: one packet can hold part of a
// SysEx message, or several short messages back to back, with or without
// running status.  Rebuild whole messages, as the other backends' drivers do.
void MidiInAndroid :: parse( const uint8_t *data, size_t count, double time )
{
  std::vector<unsigned char> &message = inputData_.message.bytes;
  const unsigned char ignore = inputData_.ignoreFlags;

  for ( size_t i = 0; i < count; i++ ) {
    const unsigned char byte = data[i];

    // Real-time messages are one byte and may appear inside any other message.
    if ( byte >= 0xF8 ) {
      if ( !rtmidiIgnoredRealtime( byte, ignore ) ) {
        std::vector<unsigned char> realtime( 1, byte );
        deliver( realtime, time );
      }
      continue;
    }

    if ( inputData_.continueSysex ) {
      if ( byte < 0x80 || byte == 0xF7 ) {
        if ( !( ignore & 0x01 ) ) message.push_back( byte );
        if ( byte == 0xF7 ) {
          inputData_.continueSysex = false;
          if ( !( ignore & 0x01 ) ) deliver( message, time );
          message.clear();
        }
        continue;
      }
      // Any other status byte ends an unterminated SysEx message: drop it.
      inputData_.continueSysex = false;
      message.clear();
    }

    if ( byte & 0x80 ) {
      message.clear();
      if ( byte == 0xF0 ) {
        runningStatus_ = 0;  // SysEx cancels running status
        inputData_.continueSysex = true;
        if ( !( ignore & 0x01 ) ) message.push_back( byte );
        continue;
      }
      if ( byte == 0xF7 ) continue;  // a stray end of SysEx
      // System common messages cancel running status.
      runningStatus_ = byte < 0xF0 ? byte : 0;
      message.push_back( byte );
    }
    else {
      if ( message.empty() ) {
        if ( !runningStatus_ ) continue;  // data with no status to belong to
        message.push_back( runningStatus_ );
      }
      message.push_back( byte );
    }

    if ( message.size() == androidMessageLength( message[0] ) ) {
      if ( !( message[0] == 0xF1 && ( ignore & 0x02 ) ) ) deliver( message, time );
      message.clear();
    }
  }
}

void MidiInAndroid :: deliver( std::vector<unsigned char> &bytes, double time )
{
  double delta = 0.0;
  if ( inputData_.firstMessage ) inputData_.firstMessage = false;
  else delta = time - lastTime;
  lastTime = time;

  if ( !inputData_.callUserCallback( delta, &bytes ) ) {
    MidiInApi::MidiMessage queued;
    queued.bytes = bytes;
    queued.timeStamp = delta;
    if ( !inputData_.queue.push( queued ) )
      std::cerr << "\nMidiInAndroid: message queue limit reached!!\n\n";
  }
}


//*********************************************************************//
//  API: Android AMIDI
//  Class Definitions: MidiOutAndroid
//*********************************************************************//


MidiOutAndroid :: MidiOutAndroid( const std::string &clientName ) : MidiOutApi() {
  MidiOutAndroid::initialize( clientName );
}

void MidiOutAndroid :: initialize( const std::string& clientName ) {
  androidClientName = clientName;
  connect();
}

void MidiOutAndroid :: connect() {
  auto env = androidGetThreadEnv();
  if (env == NULL) return;

  auto context = androidGetContext(env);
  if (context == NULL) return;

  androidRefreshMidiDevices(env, context, true, ports_);

  env->DeleteLocalRef(context);
}

MidiOutAndroid :: ~MidiOutAndroid() {
  // Attach first: releasing a device on a thread unknown to the JVM aborts.
  auto env = androidGetThreadEnv();

  // Close a connection if it exists.
  MidiOutAndroid::closePort();

  if (env == NULL) return;

  androidReleasePorts(env, ports_);

  androidClientName = "";
}

void MidiOutAndroid :: openPort( unsigned int portNumber, const std::string &portName ) {
  if (portNumber >= ports_.size()) {
    errorString_ = "MidiOutAndroid::openPort: Invalid port number";
    error( RtMidiError::INVALID_PARAMETER, errorString_ );

    return;
  }

  if (connected_) {
    errorString_ = "MidiOutAndroid::openPort: A port is already open";
    error( RtMidiError::INVALID_USE, errorString_ );

    return;
  }

  std::string why;
  auto req = androidOpenPort(ports_[portNumber], why);
  if (!req) {
    errorString_ = "MidiOutAndroid::openPort: " + why + ".";
    error( RtMidiError::DRIVER_ERROR, errorString_ );
    return;
  }
  sendDevice = req->device;
  midiInputPort = req->inputPort;
  javaDevice = req->javaDevice;
  connected_ = true;
}

void MidiOutAndroid :: openVirtualPort( const std::string &portName ) {
  errorString_ = "MidiOutAndroid::openVirtualPort: this function is not implemented for the Android API!";
  error( RtMidiError::WARNING, errorString_ );
}

unsigned int MidiOutAndroid :: getPortCount() {
  connect();
  return ports_.size();
}

std::string MidiOutAndroid :: getPortName( unsigned int portNumber ) {
  auto env = androidGetThreadEnv();
  if (env == NULL) return "";
  return androidPortName(env, ports_, portNumber);
}

void MidiOutAndroid :: closePort() {
  // AMidiDevice_release() aborts on a thread not attached to the JVM.
  JNIEnv *env = sendDevice || javaDevice ? androidGetThreadEnv() : NULL;
  if (midiInputPort) AMidiInputPort_close(midiInputPort);
  androidReleaseDevice(env, sendDevice, javaDevice);
  sendDevice = NULL;
  midiInputPort = NULL;
  javaDevice = NULL;
  sysexOpen = false;
  connected_ = false;
}

void MidiOutAndroid:: setClientName( const std::string& name ) {
  androidClientName = name;
}

void MidiOutAndroid :: setPortName( const std::string &portName ) {
  errorString_ = "MidiOutAndroid::setPortName: this function is not implemented for the Android API!";
  error( RtMidiError::WARNING, errorString_ );
}

void MidiOutAndroid :: sendMessage( const unsigned char *message, size_t size ) {
  if (midiInputPort == NULL) {
    errorString_ = "MidiOutAndroid::sendMessage: no port is open.";
    error( RtMidiError::WARNING, errorString_ );
    return;
  }

  // A message that ends an open SysEx (anything but F0, F7 or real-time)
  // closes the abandoned one first, so every receiver drops it the same way.
  if (size > 0 && sysexOpen && rtmidiEndsSysex(message[0])) {
    static const unsigned char closeSysex[2] = { 0xF0, 0xF7 };
    sendMessage(closeSysex, 2);
    // If the close failed (already reported), this message would land inside
    // the still-open SysEx.
    if (sysexOpen) return;
  }

  // The framing state this message leaves behind, committed only if it is sent.
  bool nextSysexOpen = sysexOpen;
  for (size_t i = 0; i < size; i++) {
    if (message[i] == 0xF0) nextSysexOpen = true;
    else if (message[i] >= 0x80 && message[i] < 0xF8) nextSysexOpen = false;
  }

  ssize_t sent = AMidiInputPort_send(midiInputPort, (uint8_t*)message, size);
  if (sent != (ssize_t) size) {
    errorString_ = "MidiOutAndroid::sendMessage: AMidiInputPort_send() sent " +
                   std::to_string((long) sent) + " of " + std::to_string(size) + " bytes.";
    LOGE("%s", errorString_.c_str());
    error( RtMidiError::WARNING, errorString_ );
    return;
  }
  sysexOpen = nextSysexOpen;
}

#endif  // RTMIDI_API_AMIDI

// Optional backend features use non-virtual dispatch to preserve the existing
// MidiApi and derived-class vtables. Other backends default to unsupported.
bool MidiApi :: supportsAutoReconnect( void ) const
{
#if defined(RTMIDI_API_ALSA)
  if ( const_cast<MidiApi *>(this)->getCurrentApi() == RtMidi::LINUX_ALSA )
    return AlsaMidiReconnect::isSupported();
#endif
  return false;
}

void MidiApi :: setAutoReconnect( bool enabled )
{
#if defined(RTMIDI_API_ALSA)
  if ( getCurrentApi() == RtMidi::LINUX_ALSA ) {
    int result = static_cast<AlsaMidiData *>(apiData_)->reconnect.setEnabled( enabled );
    if ( result < 0 ) error( RtMidiError::THREAD_ERROR, "RtMidi ALSA: cannot start hotplug monitor." );
  }
#else
  (void) enabled;
#endif
}

bool MidiApi :: isAutoReconnectEnabled( void ) const
{
#if defined(RTMIDI_API_ALSA)
  if ( const_cast<MidiApi *>(this)->getCurrentApi() == RtMidi::LINUX_ALSA )
    return static_cast<AlsaMidiData *>(apiData_)->reconnect.isEnabled();
#endif
  return false;
}
