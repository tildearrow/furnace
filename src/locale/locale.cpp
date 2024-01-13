#include "locale.h"

const char* languages[] = 
{
    "English",
    "Русский",
    NULL
};

const char* DivLocale::getLangName(DivLang lang)
{
    return languages[lang];
}

DivLang DivLocale::getLangIndex()
{
    return language;
}

void DivLocale::setLanguage(DivLang lang)
{
    if(lang >= DIV_LANG_MAX)
    {
        return;
    }

    strings.clear(); //language is valid, so we clear the std::map containing translations. 
    // Lower is a switch that calls the corresponding function to fill it again with translations of specific language.
    language = lang;

    switch(lang)
    {
        case DIV_LANG_RUSSIAN:
        {
            addTranslationsRussian();
            break;
        }

        default: break;
    }
}

const char* DivLocale::getText(const char* text)
{
    if(text == NULL) return text;
    if(language == DIV_LANG_ENGLISH) return text; //nothing to translate!

    auto iter = strings.find(text);

    if (iter == strings.end()) 
    {
        return text; //fall back to orig string
    }
    else 
    {
        return iter->second.plurals[0].c_str(); //found string. The default string (without plural form/context specified) lies in plurals[0]
    }
}