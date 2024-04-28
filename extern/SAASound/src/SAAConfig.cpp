// Part of SAASound copyright 2020 Dave Hooper <dave@beermex.com>
//
// SAAConfig.cpp: configuration file handler class
//
//////////////////////////////////////////////////////////////////////

#include "defns.h"
#ifdef USE_CONFIG_FILE

#include "SAAConfig.h"
#define INI_READONLY
#define INI_ANSIONLY
#include "minIni/minIni.h"
#include <codecvt>

SAAConfig::SAAConfig()
:
m_bHasReadConfig(false),
m_bGenerateRegisterLogs(false),
m_bGeneratePcmLogs(false),
m_bGeneratePcmSeparateChannels(false),
m_nBoost(DEFAULT_BOOST),
m_bHighpass(false),
m_nOversample(DEFAULT_OVERSAMPLE),
m_minIni(_T(CONFIG_FILE_PATH))
{
}

void SAAConfig::ReadConfig()
{
	// Assume (i.e. require) that the config file is always in UTF-8 .
	// These days, I think that's a good assumption to want to make.
	// It's also easy for people to create UTF-8 configs.
	// Define a helper to let us read from UTF-8 and convert to system locale
	// across platforms (and assume this will be a no-op on *nix)

#if defined(WIN32) && defined(UNICODE)
	// WIN32 support for UNICODE requires different types
	// Convert config file contents from utf8 to wchar_t
	// minIni has been compiled to use plain char for file read-write operations
	// (which supports UTF8) and wchar_t for filenames (which supports unicode filenames
	// on win32)
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
#define wrapped_gets(_mstring, _section, _element, _default) do{ \
		std::string _temp_u8 = m_minIni.gets(u8"" _section "", u8"" _element "", u8"" _default ""); \
		_mstring = converter.from_bytes(_temp_u8.c_str()); \
	} \
	while(0)
#else
	// For *nix I think I'm supposed to convert the text from the
	// file encoding (which I'm assuming is utf8) to system/user locale (if different)
	// which I think that requires converting first to wchar_t and then to current locale
	// For now I'm just going to assume you'using UTF8, and I will do no conversion.
	// minIni has been compiled to use plain char for everything,
	// which is really just a utf8 passthrough assumption.
	// If you're compiling for WIN32 but with UNICODE not defined: you're on your own
	// (it might work but I'm not testing or supporting you..)

#define wrapped_gets(_mstring, _section, _element, _default) \
	_mstring = m_minIni.gets(u8"" _section "", u8"" _element "", u8"" _default "");
#endif

#define _u8ify(x) ( u8"" x "" )
	m_bGenerateRegisterLogs = m_minIni.getbool(u8"Debug", u8"WriteRegisterLog", false);
	if (m_bGenerateRegisterLogs)
	{
		wrapped_gets(m_strRegisterLogPath, "Debug", "RegisterLogPath", DEBUG_SAA_REGISTER_LOG);
	}

	m_bGeneratePcmLogs = m_minIni.getbool(u8"Debug", u8"WritePCMOutput", false);
	if (m_bGeneratePcmLogs)
	{
		wrapped_gets(m_strPcmOutputPath, "Debug", "PCMOutputPath", DEBUG_SAA_PCM_LOG);
		m_bGeneratePcmSeparateChannels = m_minIni.getbool(u8"Debug", u8"PCMSeparateChannels", false);
	}

	m_bHighpass = m_minIni.getbool(u8"Quality", u8"Highpass", false);

	m_nOversample = m_minIni.geti(u8"Quality", u8"Oversample", DEFAULT_OVERSAMPLE);
	if (m_nOversample < 1)
		// oversample of 0 or negative doesn't make sense
		m_nOversample = 1;
	if (m_nOversample > 15)
		// oversample of 16 (=65536x oversample) or more is ridiculous
		// apart from just CPU cycles, our (32-bit)int-based accumulation would overflow
		m_nOversample = 15;

	m_nBoost = m_minIni.getf(u8"Quality", u8"Boost", DEFAULT_BOOST);
	if (m_nBoost < 1)
	{
		// interpret setting Boost=0 as disabling boost i.e. volume multipler = 100% (not 0%)
		// Since "Boost=1" also means volume multipler = 100%, we can just ignore values of
		// Boost between 0 and 1 as meaning 'no boost'
		m_nBoost = 1;
	}
}

t_string SAAConfig::getChannelPcmOutputPath(int i)
{
	// returns a name for the i'th PCM output file when generating
	// single files per channel.  This is really just a helper to localise
	// the string manipulation involved in this.
	// TODO - maybe make the naming convention more flexible?
	t_string filename = m_strPcmOutputPath;
#if defined UNICODE
	filename += std::to_wstring(i);
#else
	filename += std::to_string(i);
#endif
	return filename;
}

#endif // USE_CONFIG_FILE
