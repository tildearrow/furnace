#include <map>
#include <string>
#include "locale.h"

class DivLocale;

void DivLocale::addTranslationsRussian()
{
    strings["test"].plurals[0] = "тест";
}