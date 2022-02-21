/******************************************/
/*
  apinames.cpp
  by Jean Pierre Cimalando, 2018.

  This program tests parts of RtMidi related
  to API names, the conversion from name to API
  and vice-versa.
*/
/******************************************/

#include "RtMidi.h"
#include <cctype>
#include <cstdlib>
#include <iostream>

int test_cpp() {
    std::vector<RtMidi::Api> apis;
    RtMidi::getCompiledApi( apis );

    // ensure the known APIs return valid names
    std::cout << "API names by identifier (C++):\n";
    for ( size_t i = 0; i < apis.size() ; ++i ) {
        const std::string name = RtMidi::getApiName(apis[i]);
        if (name.empty()) {
            std::cout << "Invalid name for API " << (int)apis[i] << "\n";
            exit(1);
        }
        const std::string displayName = RtMidi::getApiDisplayName(apis[i]);
        if (displayName.empty()) {
            std::cout << "Invalid display name for API " << (int)apis[i] << "\n";
            exit(1);
        }
        std::cout << "* " << (int)apis[i] << " '" << name << "': '" << displayName << "'\n";
    }

    // ensure unknown APIs return the empty string
    {
        const std::string name = RtMidi::getApiName((RtMidi::Api)-1);
        if (!name.empty()) {
            std::cout << "Bad string for invalid API '" << name << "'\n";
            exit(1);
        }
        const std::string displayName = RtMidi::getApiDisplayName((RtMidi::Api)-1);
        if (displayName!="Unknown") {
            std::cout << "Bad display string for invalid API '" << displayName << "'\n";
            exit(1);
        }
    }

    // try getting API identifier by name
    std::cout << "API identifiers by name (C++):\n";
    for ( size_t i = 0; i < apis.size() ; ++i ) {
        std::string name = RtMidi::getApiName(apis[i]);
        if ( RtMidi::getCompiledApiByName(name) != apis[i] ) {
            std::cout << "Bad identifier for API '" << name << "'\n";
            exit( 1 );
        }
        std::cout << "* '" << name << "': " << (int)apis[i] << "\n";

        for ( size_t j = 0; j < name.size(); ++j )
            name[j] = (j & 1) ? toupper(name[j]) : tolower(name[j]);
        RtMidi::Api api = RtMidi::getCompiledApiByName(name);
        if ( api != RtMidi::UNSPECIFIED ) {
            std::cout << "Identifier " << (int)api << " for invalid API '" << name << "'\n";
            exit( 1 );
        }
    }

    // try getting an API identifier by unknown name
    {
        RtMidi::Api api;
        api = RtMidi::getCompiledApiByName("");
        if ( api != RtMidi::UNSPECIFIED ) {
            std::cout << "Bad identifier for unknown API name\n";
            exit( 1 );
        }
    }

    return 0;
}

#include "rtmidi_c.h"

int test_c() {
    unsigned api_count = rtmidi_get_compiled_api(NULL, 0);
    std::vector<RtMidiApi> apis(api_count);
    rtmidi_get_compiled_api(apis.data(), api_count);

    // ensure the known APIs return valid names
    std::cout << "API names by identifier (C):\n";
    for ( size_t i = 0; i < api_count; ++i) {
        const std::string name = rtmidi_api_name(apis[i]);
        if (name.empty()) {
            std::cout << "Invalid name for API " << (int)apis[i] << "\n";
            exit(1);
        }
        const std::string displayName = rtmidi_api_display_name(apis[i]);
        if (displayName.empty()) {
            std::cout << "Invalid display name for API " << (int)apis[i] << "\n";
            exit(1);
        }
        std::cout << "* " << (int)apis[i] << " '" << name << "': '" << displayName << "'\n";
    }

    // ensure unknown APIs return the empty string
    {
        const char *s = rtmidi_api_name((RtMidiApi)-1);
        const std::string name(s?s:"");
        if (!name.empty()) {
            std::cout << "Bad string for invalid API '" << name << "'\n";
            exit(1);
        }
        s = rtmidi_api_display_name((RtMidiApi)-1);
        const std::string displayName(s?s:"");
        if (displayName!="Unknown") {
            std::cout << "Bad display string for invalid API '" << displayName << "'\n";
            exit(1);
        }
    }

    // try getting API identifier by name
    std::cout << "API identifiers by name (C):\n";
    for ( size_t i = 0; i < api_count ; ++i ) {
        const char *s = rtmidi_api_name(apis[i]);
        std::string name(s?s:"");
        if ( rtmidi_compiled_api_by_name(name.c_str()) != apis[i] ) {
            std::cout << "Bad identifier for API '" << name << "'\n";
            exit( 1 );
        }
        std::cout << "* '" << name << "': " << (int)apis[i] << "\n";

        for ( size_t j = 0; j < name.size(); ++j )
            name[j] = (j & 1) ? toupper(name[j]) : tolower(name[j]);
        RtMidiApi api = rtmidi_compiled_api_by_name(name.c_str());
        if ( api != RTMIDI_API_UNSPECIFIED ) {
            std::cout << "Identifier " << (int)api << " for invalid API '" << name << "'\n";
            exit( 1 );
        }
    }

    // try getting an API identifier by unknown name
    {
        RtMidiApi api;
        api = rtmidi_compiled_api_by_name("");
        if ( api != RTMIDI_API_UNSPECIFIED ) {
            std::cout << "Bad identifier for unknown API name\n";
            exit( 1 );
        }
    }

    return 0;
}

int main()
{
    test_cpp();
    test_c();
}
