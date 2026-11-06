// ALSA integration test. USB identity is simulated; all sequencer events are real.
/* Run from the RtMidi directory on Linux with /dev/snd/seq available:
   g++ -std=c++11 -DRTMIDI_API_ALSA tests/alsahotplug.cpp RtMidi.cpp -lasound -pthread \
     -Wl,--wrap=snd_seq_client_info_get_card -Wl,--wrap=realpath \
     -Wl,--wrap=__realpath_chk -Wl,--wrap=pipe2 -Wl,--wrap=pthread_create -Wl,--wrap=snd_seq_open \
     -Wl,--wrap=snd_seq_event_input -Wl,--wrap=snd_seq_subscribe_port \
     -Wl,--wrap=snd_seq_poll_descriptors -Wl,--wrap=snd_seq_query_next_client -o alsahotplug
   ./alsahotplug
   No USB device is unplugged or written to by this test.
*/
#include "../RtMidi.h"
#if defined(RTMIDI_API_ALSA)
#include <alsa/asoundlib.h>
#endif
#include <iostream>
#include <cstdlib>

using namespace rt::midi;

static void check( bool ok, const char *what )
{
  if ( !ok ) { std::cerr << "FAIL: " << what << '\n'; std::exit( 1 ); }
  std::cout << "PASS: " << what << '\n';
}

static void checkUnsupportedBackend()
{
  RtMidiIn input( RtMidi::RTMIDI_DUMMY );
  RtMidiOut output( RtMidi::RTMIDI_DUMMY );
  check( !input.supportsAutoReconnect() && !output.supportsAutoReconnect(), "unsupported backend capability" );
  input.setAutoReconnect( true );
  output.setAutoReconnect( true );
  check( !input.isAutoReconnectEnabled() && !output.isAutoReconnectEnabled(), "unsupported backend ignores enable" );
  input.setAutoReconnect( false );
  output.setAutoReconnect( false );
  check( !input.isAutoReconnectEnabled() && !output.isAutoReconnectEnabled(), "unsupported backend ignores disable" );
}

#if defined(RTMIDI_API_ALSA)
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <functional>
#include <fcntl.h>
#include <dirent.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>

static std::string fixture;
static std::atomic<int> wakeFailures(0), openFailures(0), inputFailure(0), subscriptionFailures(0), descriptorFailures(0);
static std::atomic<int> threadFailures(0);
static std::atomic<int> inputThreadFailures(0), initialSubscriptionFailures(0);
static std::atomic<int> sysexEvents(0);
static std::atomic<int> readFailures(0), enumerationFailures(0);
static std::atomic<bool> monitorCreation(false);
static std::atomic<bool> discardAnnouncements(false);

static bool consume( std::atomic<int> &failures )
{
  int remaining = failures.load();
  while ( remaining > 0 )
    if ( failures.compare_exchange_weak( remaining, remaining - 1 ) ) return true;
  return false;
}

static bool isMonitor( snd_seq_t *seq )
{
  snd_seq_client_info_t *info;
  snd_seq_client_info_alloca( &info );
  return snd_seq_get_client_info( seq, info ) == 0 &&
    strcmp( snd_seq_client_info_get_name( info ), "RtMidi hotplug" ) == 0;
}

extern "C" int __real_pipe2( int fds[2], int flags );
extern "C" int __wrap_pipe2( int fds[2], int flags )
{
  if ( consume( wakeFailures ) ) { errno = EMFILE; return -1; }
  int result = __real_pipe2( fds, flags );
  if ( result == 0 ) monitorCreation = true;
  return result;
}

extern "C" int __real_pthread_create( pthread_t *thread, const pthread_attr_t *attr, void *(*start)(void *), void *arg );
extern "C" int __wrap_pthread_create( pthread_t *thread, const pthread_attr_t *attr, void *(*start)(void *), void *arg )
{
  if ( monitorCreation.exchange(false) ) {
    if ( consume( threadFailures ) ) return EAGAIN;
  }
  else if ( consume( inputThreadFailures ) ) return EAGAIN;
  return __real_pthread_create( thread, attr, start, arg );
}

extern "C" int __real_snd_seq_open( snd_seq_t **seq, const char *name, int streams, int mode );
extern "C" int __wrap_snd_seq_open( snd_seq_t **seq, const char *name, int streams, int mode )
{
  if ( streams == SND_SEQ_OPEN_INPUT && consume( openFailures ) ) return -EIO;
  return __real_snd_seq_open( seq, name, streams, mode );
}

extern "C" int __real_snd_seq_event_input( snd_seq_t *seq, snd_seq_event_t **event );
extern "C" int __wrap_snd_seq_event_input( snd_seq_t *seq, snd_seq_event_t **event )
{
  if ( isMonitor( seq ) ) {
    int failure = inputFailure.exchange( 0 );
    if ( consume( readFailures ) ) failure = -EIO;
    if ( discardAnnouncements || failure ) {
      while ( __real_snd_seq_event_input( seq, event ) >= 0 ) snd_seq_free_event( *event );
      if ( failure ) return failure;
      return -EAGAIN;
    }
  }
  int result = __real_snd_seq_event_input( seq, event );
  if ( result >= 0 && (*event)->type == SND_SEQ_EVENT_SYSEX ) ++sysexEvents;
  return result;
}

extern "C" int __real_snd_seq_subscribe_port( snd_seq_t *seq, snd_seq_port_subscribe_t *subscription );
extern "C" int __wrap_snd_seq_subscribe_port( snd_seq_t *seq, snd_seq_port_subscribe_t *subscription )
{
  if ( isMonitor( seq ) ) {
    if ( consume( subscriptionFailures ) ) return -EIO;
  }
  else if ( consume( initialSubscriptionFailures ) ) return -EIO;
  return __real_snd_seq_subscribe_port( seq, subscription );
}

extern "C" int __real_snd_seq_poll_descriptors( snd_seq_t *seq, struct pollfd *fds, unsigned int count, short events );
extern "C" int __wrap_snd_seq_poll_descriptors( snd_seq_t *seq, struct pollfd *fds, unsigned int count, short events )
{
  if ( isMonitor( seq ) && consume( descriptorFailures ) ) return -EIO;
  return __real_snd_seq_poll_descriptors( seq, fds, count, events );
}

extern "C" int __real_snd_seq_query_next_client( snd_seq_t *seq, snd_seq_client_info_t *info );
extern "C" int __wrap_snd_seq_query_next_client( snd_seq_t *seq, snd_seq_client_info_t *info )
{
  if ( isMonitor( seq ) && consume( enumerationFailures ) ) return -EIO;
  return __real_snd_seq_query_next_client( seq, info );
}

static unsigned int monitorThreads()
{
  DIR *tasks = opendir( "/proc/self/task" );
  if ( !tasks ) return 0;
  unsigned int count = 0;
  struct dirent *entry;
  while ( (entry = readdir( tasks )) ) {
    std::string name;
    std::ifstream file( std::string("/proc/self/task/") + entry->d_name + "/comm" );
    std::getline( file, name );
    if ( name == "RtMidi hotplug" ) ++count;
  }
  closedir( tasks );
  return count;
}

struct StartupError {
  RtMidiIn *input;
  unsigned int count;
};

static void startupError( RtMidiError::Type type, const std::string &, void *context )
{
  StartupError *state = static_cast<StartupError *>(context);
  if ( type == RtMidiError::THREAD_ERROR ) {
    ++state->count;
    state->input->closePort(); // Must not run while holding the registry lock.
  }
}

static std::string uniqueName( const char *name )
{
  return std::string( name ) + " " + std::to_string( getpid() );
}

#if SND_LIB_VERSION >= 0x010101
extern "C" int __real_snd_seq_client_info_get_card( const snd_seq_client_info_t *info );
extern "C" int __wrap_snd_seq_client_info_get_card( const snd_seq_client_info_t *info )
{
  std::string name = snd_seq_client_info_get_name( const_cast<snd_seq_client_info_t *>(info) );
  if ( name == uniqueName("RtMidi hotplug test A") || name == uniqueName("RtMidi hotplug test duplicate") ) return 900;
  if ( name == uniqueName("RtMidi hotplug test B") ) return 901;
  if ( name == uniqueName("RtMidi hotplug test no serial") ) return 902;
  return __real_snd_seq_client_info_get_card( info );
}
#endif

extern "C" char *__real_realpath( const char *path, char *resolved );
extern "C" char *__wrap_realpath( const char *path, char *resolved )
{
  for ( int card = 900; card <= 902; ++card ) {
    if ( std::string( path ) == "/sys/class/sound/card" + std::to_string( card ) + "/device" ) {
      std::string target = fixture + "/" + std::to_string( card );
      return __real_realpath( target.c_str(), resolved );
    }
  }
  return __real_realpath( path, resolved );
}

#if defined(__GLIBC__)
// Optimized, fortified builds call this instead of realpath(). Keep the
// bounds check while applying exactly the same fixture mapping.
extern "C" char *__wrap___realpath_chk( const char *path, char *resolved, size_t length )
{
  char *result = __wrap_realpath( path, 0 );
  if ( !result ) return 0;
  if ( strlen( result ) >= length ) {
    free( result );
    errno = ERANGE;
    return 0;
  }
  strcpy( resolved, result );
  free( result );
  return resolved;
}
#endif

static bool waitFor( const std::function<bool()> &condition )
{
  for ( int i = 0; i < 200; ++i ) {
    if ( condition() ) return true;
    std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
  }
  return false;
}

struct Device {
  snd_seq_t *seq = 0;
  int port = -1;
  void open( const char *name = "RtMidi hotplug test A" ) {
    check( snd_seq_open( &seq, "default", SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK ) >= 0, "create device" );
    snd_seq_set_client_name( seq, uniqueName( name ).c_str() );
    addPort();
  }
  void addPort() {
    port = snd_seq_create_simple_port( seq, "Keyboard", SND_SEQ_PORT_CAP_READ |
      SND_SEQ_PORT_CAP_SUBS_READ | SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
      SND_SEQ_PORT_TYPE_MIDI_GENERIC );
    check( port >= 0, "create MIDI port" );
  }
  void close() { if ( seq ) snd_seq_close( seq ); seq = 0; }
  ~Device() { close(); }
  bool connected() {
    snd_seq_port_info_t *info;
    snd_seq_port_info_alloca( &info );
    return snd_seq_get_port_info( seq, port, info ) == 0 &&
      snd_seq_port_info_get_read_use( info ) == 1 && snd_seq_port_info_get_write_use( info ) == 1;
  }
  bool disconnected() {
    snd_seq_port_info_t *info;
    snd_seq_port_info_alloca( &info );
    return snd_seq_get_port_info( seq, port, info ) == 0 &&
      snd_seq_port_info_get_read_use( info ) == 0 && snd_seq_port_info_get_write_use( info ) == 0;
  }
  void send() {
    snd_seq_event_t event;
    snd_seq_ev_clear( &event );
    snd_seq_ev_set_source( &event, port );
    snd_seq_ev_set_subs( &event );
    snd_seq_ev_set_direct( &event );
    snd_seq_ev_set_noteon( &event, 0, 60, 64 );
    snd_seq_event_output_direct( seq, &event );
  }
  void sendPartialSysex() {
    unsigned char bytes[] = {0xf0, 0x7d, 0x01};
    snd_seq_event_t event;
    snd_seq_ev_clear( &event );
    snd_seq_ev_set_source( &event, port );
    snd_seq_ev_set_subs( &event );
    snd_seq_ev_set_direct( &event );
    snd_seq_ev_set_sysex( &event, sizeof(bytes), bytes );
    check( snd_seq_event_output_direct( seq, &event ) >= 0, "send unfinished sysex" );
  }
  bool received() {
    snd_seq_event_t *event;
    bool found = false;
    while ( snd_seq_event_input( seq, &event ) >= 0 ) {
      if ( event->type == SND_SEQ_EVENT_NOTEON && event->data.note.note == 60 ) found = true;
      snd_seq_free_event( event );
    }
    return found;
  }
};

static unsigned int findPort( RtMidi &midi, const char *name )
{
  for ( unsigned int i = 0; i < midi.getPortCount(); ++i )
    if ( midi.getPortName( i ).find( uniqueName( name ) + ":Keyboard " ) == 0 ) return i;
  check( false, "find test device" );
  return 0;
}

static void callback( double, std::vector<unsigned char> *message, void *context )
{
  if ( *message == std::vector<unsigned char>{0x90, 60, 64} )
    ++*static_cast<std::atomic<int> *>(context);
}

static void verifyMidi( Device &device, RtMidiOut &output, std::atomic<int> &received )
{
  check( waitFor( [&] { return device.connected(); } ), "both subscriptions restored" );
  int before = received;
  device.send();
  check( waitFor( [&] { return received == before + 1; } ), "input MIDI delivered" );
  unsigned char note[] = {0x90, 60, 64};
  output.sendMessage( note, sizeof(note) );
  check( waitFor( [&] { return device.received(); } ), "output MIDI delivered" );
}

int main()
{
  checkUnsupportedBackend();
  alarm( 30 ); // Also bound standalone and Autotools runs, including shutdown.
  // Serialize test instances: ALSA client-number allocation is global, even
  // though fixture names are unique. This advisory lock does not block MIDI I/O.
  int sequencer = open( "/dev/snd/seq", O_RDONLY | O_CLOEXEC );
  if ( sequencer < 0 ) {
    std::cout << "SKIP: ALSA sequencer is unavailable\n";
    return 77;
  }
  check( flock( sequencer, LOCK_EX ) == 0, "serialize ALSA integration tests" );
  snd_seq_t *probe = 0;
  if ( snd_seq_open( &probe, "default", SND_SEQ_OPEN_INPUT, SND_SEQ_NONBLOCK ) < 0 ) {
    std::cout << "SKIP: ALSA sequencer is unavailable\n";
    return 77;
  }
  snd_seq_close( probe );
  RtMidiIn input( RtMidi::LINUX_ALSA, "RtMidi hotplug test input" );
  RtMidiOut output( RtMidi::LINUX_ALSA, "RtMidi hotplug test output" );
  const bool inputSupported = input.supportsAutoReconnect();
  const bool outputSupported = output.supportsAutoReconnect();
  if ( !inputSupported || !outputSupported ) {
    std::cout << "SKIP: backend reports automatic reconnection unsupported\n";
    return 77;
  }
  char temporary[] = "/tmp/rtmidi-hotplug-XXXXXX";
  char *directory = mkdtemp( temporary );
  check( directory != 0, "create identity fixtures" );
  fixture = directory;
  for ( int card = 900; card <= 902; ++card ) {
    std::string path = fixture + "/" + std::to_string( card );
    mkdir( path.c_str(), 0700 );
    std::ofstream( path + "/idVendor" ) << "0582\n";
    std::ofstream( path + "/idProduct" ) << "0014\n";
    if ( card != 902 ) std::ofstream( path + "/serial" ) << card << '\n';
  }

  Device device;
  device.open();
  inputThreadFailures = 1;
  try { input.openPort( findPort( input, "RtMidi hotplug test A" ) ); }
  catch ( const RtMidiError &error ) { check( error.getType() == RtMidiError::THREAD_ERROR, "input startup error reported" ); }
  check( inputThreadFailures == 0 && !input.isPortOpen() && device.disconnected(), "input startup failure unregisters recovery" );
  initialSubscriptionFailures = 1;
  try { output.openPort( findPort( output, "RtMidi hotplug test A" ) ); }
  catch ( const RtMidiError &error ) { check( error.getType() == RtMidiError::DRIVER_ERROR, "output subscription error reported" ); }
  check( initialSubscriptionFailures == 0 && !output.isPortOpen() && device.disconnected(), "output startup failure unregisters recovery" );
  check( input.supportsAutoReconnect() && output.supportsAutoReconnect(), "ALSA capability" );
  check( input.isAutoReconnectEnabled() && output.isAutoReconnectEnabled(), "recovery defaults on" );
  std::atomic<int> received(0);
  input.ignoreTypes( false, true, true );
  input.setCallback( callback, &received );
  StartupError startup = {&input, 0};
  input.setErrorCallback( startupError, &startup );
  wakeFailures = 1;
  threadFailures = 2;
  input.openPort( findPort( input, "RtMidi hotplug test A" ) );
  check( startup.count == 1 && !input.isPortOpen(), "startup error callback can close the port" );
  input.setErrorCallback( 0 );
  wakeFailures = 1;
  threadFailures = 1;
  openFailures = 2;
  input.openPort( findPort( input, "RtMidi hotplug test A" ) );
  output.openPort( findPort( output, "RtMidi hotplug test A" ) );
  verifyMidi( device, output, received );
  check( waitFor( [] { return openFailures == 0 && monitorThreads() == 1; } ), "one shared monitoring thread; startup retries" );

  device.sendPartialSysex();
  check( waitFor( [] { return sysexEvents > 0; } ), "input receives unfinished sysex" );
  device.close();
  device.open();
  verifyMidi( device, output, received );

  {
    Device second;
    second.open( "RtMidi hotplug test B" );
    RtMidiIn secondInput( RtMidi::LINUX_ALSA );
    RtMidiOut secondOutput( RtMidi::LINUX_ALSA );
    std::atomic<int> secondReceived(0);
    secondInput.setCallback( callback, &secondReceived );
    secondInput.openPort( findPort( secondInput, "RtMidi hotplug test B" ) );
    secondOutput.openPort( findPort( secondOutput, "RtMidi hotplug test B" ) );
    verifyMidi( second, secondOutput, secondReceived );
    check( monitorThreads() == 1, "all input/output ports share one worker" );
    device.close();
    second.close();
    device.open();
    second.open( "RtMidi hotplug test B" );
    verifyMidi( device, output, received );
    verifyMidi( second, secondOutput, secondReceived );
  }

  discardAnnouncements = true;
  device.close();
  device.open();
  std::this_thread::sleep_for( std::chrono::milliseconds( 150 ) );
  check( device.disconnected(), "simulate lost removal/arrival announcements" );
  inputFailure = -ENOSPC;
  discardAnnouncements = false;
  snd_seq_set_client_name( device.seq, uniqueName("RtMidi hotplug test A").c_str() );
  verifyMidi( device, output, received );
  check( inputFailure == 0, "announcement overflow reconciles every connection" );

  discardAnnouncements = true;
  device.close();
  device.open();
  std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
  openFailures = 2;
  subscriptionFailures = 2;
  enumerationFailures = 2;
  inputFailure = -EIO;
  readFailures = 2;
  discardAnnouncements = false;
  snd_seq_set_client_name( device.seq, uniqueName("RtMidi hotplug test A").c_str() );
  verifyMidi( device, output, received );
  check( openFailures == 0 && subscriptionFailures == 0 && inputFailure == 0 &&
    readFailures == 0 && enumerationFailures == 0,
    "monitor and subscription failures recover automatically" );

  descriptorFailures = 2;
  snd_seq_set_client_name( device.seq, uniqueName("RtMidi hotplug test A").c_str() );
  check( waitFor( [] { return descriptorFailures == 0; } ), "descriptor failure exercised" );
  verifyMidi( device, output, received );

  input.setAutoReconnect( false );
  output.setAutoReconnect( false );
  check( !input.isAutoReconnectEnabled() && !output.isAutoReconnectEnabled(), "runtime disable" );
  verifyMidi( device, output, received );
  device.close();
  device.open();
  std::this_thread::sleep_for( std::chrono::milliseconds( 250 ) );
  check( device.disconnected(), "disabled recovery leaves returning device disconnected" );
  input.setAutoReconnect( true );
  output.setAutoReconnect( true );
  verifyMidi( device, output, received );

  device.close();
  std::ofstream( fixture + "/900/serial" ) << "not-ready\n";
  device.open();
  std::this_thread::sleep_for( std::chrono::milliseconds( 150 ) );
  check( device.disconnected(), "wait for matching identity" );
  std::ofstream( fixture + "/900/serial" ) << "900\n";
  verifyMidi( device, output, received );

  int oldAddress = snd_seq_client_id( device.seq );
  device.close();
  device.open();
  check( snd_seq_client_id( device.seq ) == oldAddress, "same client address" );
  verifyMidi( device, output, received );

  device.close();
  Device wrong;
  wrong.open( "RtMidi hotplug test B" );
  check( snd_seq_client_id( wrong.seq ) == oldAddress, "different serial occupies old address" );
  std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
  check( wrong.disconnected(), "do not connect to wrong serial" );
  device.open();
  check( snd_seq_client_id( device.seq ) != oldAddress, "changed client address" );
  verifyMidi( device, output, received );

  snd_seq_delete_simple_port( device.seq, device.port );
  device.addPort();
  verifyMidi( device, output, received );

  // An explicit patchbay unsubscribe must not itself trigger recovery.
  snd_seq_query_subscribe_t *query;
  snd_seq_query_subscribe_alloca( &query );
  snd_seq_addr_t root;
  root.client = snd_seq_client_id( device.seq );
  root.port = device.port;
  snd_seq_query_subscribe_set_root( query, &root );
  snd_seq_query_subscribe_set_type( query, SND_SEQ_QUERY_SUBS_READ );
  snd_seq_query_subscribe_set_index( query, 0 );
  check( snd_seq_query_port_subscribers( device.seq, query ) == 0, "find input subscription" );
  snd_seq_port_subscribe_t *subscription;
  snd_seq_port_subscribe_alloca( &subscription );
  snd_seq_port_subscribe_set_sender( subscription, &root );
  snd_seq_port_subscribe_set_dest( subscription, snd_seq_query_subscribe_get_addr( query ) );
  check( snd_seq_unsubscribe_port( device.seq, subscription ) == 0, "manual unsubscribe" );
  std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
  check( !device.connected(), "manual unsubscribe remains effective" );

  input.closePort();
  output.closePort();
  device.close();
  device.open();
  std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
  check( device.disconnected(), "explicit close cancels recovery" );
  check( !input.isPortOpen() && !output.isPortOpen(), "closed state" );

  input.setAutoReconnect( false );
  output.setAutoReconnect( false );
  input.openPort( findPort( input, "RtMidi hotplug test A" ) );
  output.openPort( findPort( output, "RtMidi hotplug test A" ) );
  verifyMidi( device, output, received );
  device.close();
  device.open();
  std::this_thread::sleep_for( std::chrono::milliseconds( 150 ) );
  check( device.disconnected(), "disable before open is preserved" );
  device.close();
  input.setAutoReconnect( true );
  output.setAutoReconnect( true );
  device.open();
  verifyMidi( device, output, received );
  input.closePort();
  output.closePort();

  input.openPort( findPort( input, "RtMidi hotplug test A" ) );
  output.openPort( findPort( output, "RtMidi hotplug test A" ) );
  verifyMidi( device, output, received );
  device.close();
  input.closePort();
  output.closePort();
  device.open();
  std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
  check( device.disconnected(), "close while absent cancels recovery" );

  // Exercise no-serial matching separately from serial-number matching.
  device.close();
  device.open( "RtMidi hotplug test no serial" );
  input.openPort( findPort( input, "RtMidi hotplug test no serial" ) );
  output.openPort( findPort( output, "RtMidi hotplug test no serial" ) );
  device.close();
  device.open( "RtMidi hotplug test no serial" );
  verifyMidi( device, output, received );
  input.closePort();
  output.closePort();

  // Software ports lack USB identity and must not be rebound by name.
  device.close();
  device.open( "RtMidi hotplug test software" );
  input.openPort( findPort( input, "RtMidi hotplug test software" ) );
  output.openPort( findPort( output, "RtMidi hotplug test software" ) );
  verifyMidi( device, output, received );
  device.close();
  device.open( "RtMidi hotplug test software" );
  std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
  check( device.disconnected(), "no name-only recovery" );
  input.closePort();
  output.closePort();

  device.close();
  device.open();
  Device duplicate;
  duplicate.open( "RtMidi hotplug test duplicate" );
  input.openPort( findPort( input, "RtMidi hotplug test A" ) );
  output.openPort( findPort( output, "RtMidi hotplug test A" ) );
  verifyMidi( device, output, received );
  device.close();
  device.open();
  std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
  check( device.disconnected() && duplicate.disconnected(), "ambiguous identity disables recovery" );
  input.closePort();
  output.closePort();
  duplicate.close();

  input.openPort( findPort( input, "RtMidi hotplug test A" ) );
  output.openPort( findPort( output, "RtMidi hotplug test A" ) );
  for ( int cycle = 0; cycle < 25; ++cycle ) {
    device.close();
    device.open();
    verifyMidi( device, output, received );
  }
  std::atomic<bool> sending(true);
  std::thread sender( [&] {
    unsigned char note[] = {0x90, 61, 0};
    while ( sending ) {
      output.sendMessage( note, sizeof(note) );
      std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
    }
  } );
  for ( int cycle = 0; cycle < 10; ++cycle ) {
    device.close();
    device.open();
    check( waitFor( [&] { return device.connected(); } ), "hotplug while output is sending" );
  }
  sending = false;
  sender.join();
  input.closePort();
  output.closePort();

  for ( int cycle = 0; cycle < 10; ++cycle ) {
    RtMidiIn temporaryInput( RtMidi::LINUX_ALSA );
    RtMidiOut temporaryOutput( RtMidi::LINUX_ALSA );
    temporaryInput.openPort( findPort( temporaryInput, "RtMidi hotplug test A" ) );
    temporaryOutput.openPort( findPort( temporaryOutput, "RtMidi hotplug test A" ) );
    device.close();
    device.open();
  }
  check( device.disconnected(), "destruction cancels pending recovery" );

  {
    Device other;
    other.open( "RtMidi hotplug test no serial" );
    auto exercise = []( const char *name ) {
      for ( int cycle = 0; cycle < 20; ++cycle ) {
        RtMidiIn in( RtMidi::LINUX_ALSA );
        RtMidiOut out( RtMidi::LINUX_ALSA );
        in.openPort( findPort( in, name ) );
        out.openPort( findPort( out, name ) );
        in.setAutoReconnect( false );
        out.setAutoReconnect( false );
        in.closePort();
        out.closePort();
      }
    };
    std::thread first( exercise, "RtMidi hotplug test A" );
    std::thread second( exercise, "RtMidi hotplug test no serial" );
    first.join();
    second.join();
    check( device.disconnected() && other.disconnected() && monitorThreads() == 1,
      "concurrent open/close leaves one worker and no stale subscriptions" );
  }

  input.openVirtualPort();
  output.openVirtualPort();
  check( !input.isPortOpen() && !output.isPortOpen(), "virtual port semantics unchanged" );
  input.closePort();
  output.closePort();
  for ( int card = 900; card <= 902; ++card ) {
    std::string path = fixture + "/" + std::to_string( card );
    std::remove( (path + "/idVendor").c_str() );
    std::remove( (path + "/idProduct").c_str() );
    std::remove( (path + "/serial").c_str() );
    rmdir( path.c_str() );
  }
  rmdir( fixture.c_str() );
  close( sequencer );
}

#else

int main()
{
  checkUnsupportedBackend();
  return 0;
}

#endif
