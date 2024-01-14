#include "locale.h"

#include "template.h"
#include "russian.h"

const char* languages[] = 
{
    "English",
    "Русский",
    "TEMPLATE",
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

    getPluralIndex = &getPluralIndexTemplate; //by default we have no plural forms!

    switch(lang)
    {
        case DIV_LANG_RUSSIAN:
        {
            addTranslationsRussian();
            getPluralIndex = &getPluralIndexRussian;
            break;
        }

        case DIV_LANG_TEMPLATE:
        {
            addTranslationsTemplate();
            getPluralIndex = &getPluralIndexTemplate;
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

const char* DivLocale::getTextPlural(const char* text, int p)
{
    if(getPluralIndex == NULL) return text;
    if(text == NULL) return text;
    if(language == DIV_LANG_ENGLISH) return text; //nothing to translate!

    auto iter = strings.find(text);

    if (iter == strings.end()) 
    {
        return text; //fall back to orig string
    }
    else 
    {
        const char* str = iter->second.plurals[getPluralIndex(p)].c_str();
        return strcmp(str, "") == 0 ? text : str; //found string
    }
}