#include <map>
#include <string>
#include "locale.h"

#include "russian.h"

int getPluralIndexRussian(int n)
{
    return (n%10==1 && n%100!=11) ? 0 : ((n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20)) ? 1 : 2);
    //here you can provide plural forms indices based on the integer.
    //you can find one-liners for common languages here:
    //https://www.gnu.org/software/gettext/manual/html_node/Plural-forms.html
    //these would need some adaptation to work in this code
}

void DivLocale::addTranslationsRussian()
{
    // everything in a string after the ## or ### must remain as is
    // example: Sparkles!##sgab1 means the second instance of "Sparkles!"
    //   in `src/gui/about.cpp`.

    ImGui::LocalizeRegisterEntries(GLocalizationEntriesRuRU, IM_ARRAYSIZE(GLocalizationEntriesRuRU));

    strings["%d apple"].plurals[0] = "%d яблоко";
    strings["%d apple"].plurals[1] = "%d яблока";
    strings["%d apple"].plurals[2] = "%d яблок";

    //src/gui/about.cpp

    strings["and Furnace-B developers##sgab"].plurals[0] = "и разработчики Furnace-B";
    strings["are proud to present##sgab"].plurals[0] = "с гордостью представляют";
    strings["the biggest multi-system chiptune tracker!##sgab"].plurals[0] = "самый большой мультисистемный чиптюн-трекер!";
    strings["featuring DefleMask song compatibility.##sgab"].plurals[0] = "совместим с файлами Deflemask.";

    strings["> CREDITS <##sgab"].plurals[0] = "> ТИТРЫ <";
    strings["-- program --##sgab"].plurals[0] = "-- код --";
    strings["A M 4 N (intro tune)##sgab"].plurals[0] = "A M 4 N (музыка в интро)";
    strings["-- graphics/UI design --##sgab"].plurals[0] = "-- графика/дизайн интерфейса --";
    strings["-- documentation --##sgab"].plurals[0] = "-- документация --";
    strings["-- demo songs --##sgab"].plurals[0] = "-- демо-модули --";
    strings["-- additional feedback/fixes --##sgab"].plurals[0] = "-- обратная связь/фиксы --";
    strings["-- Metal backend test team --##sgab"].plurals[0] = "-- Команда тестирования библиотеки отрисовки Metal --";
    strings["-- translations and related work --##sgab"].plurals[0] = "-- переводы и связанное с ними --";
    strings["LTVA1 (Russian translation)##sgab"].plurals[0] = "LTVA1 (перевод на русский язык)";
    strings["Kagamiin~ (Portuguese translation)##sgab"].plurals[0] = "Kagamiin~ (перевод на португальский язык)";
    strings["freq-mod (Polish translation)##sgab"].plurals[0] = "freq-mod (перевод на польский язык)";

    strings["powered by:##sgab"].plurals[0] = "программа использует:";
    strings["Dear ImGui by Omar Cornut##sgab"].plurals[0] = "Dear ImGui за авторством Omar Cornut";
    strings["SDL2 by Sam Lantinga##sgab"].plurals[0] = "SDL2 за авторством Sam Lantinga";
    strings["zlib by Jean-loup Gailly##sgab"].plurals[0] = "zlib за авторством Jean-loup Gailly";
    strings["and Mark Adler##sgab"].plurals[0] = "и Mark Adler";
    strings["libsndfile by Erik de Castro Lopo##sgab"].plurals[0] = "libsndfile за авторством Erik de Castro Lopo";
    strings["Portable File Dialogs by Sam Hocevar##sgab"].plurals[0] = "Portable File Dialogs за авторством Sam Hocevar";
    strings["Native File Dialog by Frogtoss Games##sgab"].plurals[0] = "Native File Dialog за авторством Frogtoss Games";
    strings["Weak-JACK by x42##sgab"].plurals[0] = "Weak-JACK за авторством x42";
    strings["RtMidi by Gary P. Scavone##sgab"].plurals[0] = "RtMidi за авторством Gary P. Scavone";
    strings["FFTW by Matteo Frigo and Steven G. Johnson##sgab"].plurals[0] = "FFTW за авторством Matteo Frigo и Steven G. Johnson";
    strings["backward-cpp by Google##sgab"].plurals[0] = "backward-cpp за авторством Google";
    strings["adpcm by superctr##sgab"].plurals[0] = "adpcm за авторством superctr";
    strings["adpcm-xq by David Bryant##sgab"].plurals[0] = "adpcm-xq за авторством David Bryant";
    strings["Nuked-OPL3/OPLL/OPM/OPN2/PSG by nukeykt##sgab"].plurals[0] = "Nuked-OPL3/OPLL/OPM/OPN2/PSG за авторством nukeykt";
    strings["YM3812-LLE, YMF262-LLE, YMF276-LLE и YM2608-LLE by nukeykt##sgab"].plurals[0] = "YM3812-LLE, YMF262-LLE, YMF276-LLE и YM2608-LLE за авторством nukeykt";
    strings["ymfm by Aaron Giles##sgab"].plurals[0] = "ymfm за авторством Aaron Giles";
    strings["emu2413 by Digital Sound Antiques##sgab"].plurals[0] = "emu2413 за авторством Digital Sound Antiques";
    strings["MAME SN76496 by Nicola Salmoria##sgab"].plurals[0] = "MAME SN76496 за авторством Nicola Salmoria";
    strings["MAME AY-3-8910 by Couriersud##sgab"].plurals[0] = "MAME AY-3-8910 за авторством Couriersud";
    strings["with AY8930 fixes by Eulous, cam900 and Grauw##sgab"].plurals[0] = "с исправлениями для AY8930 за авторством Eulous, cam900 и Grauw";
    strings["MAME SAA1099 by Juergen Buchmueller and Manuel Abadia##sgab"].plurals[0] = "MAME SAA1099 за авторством Juergen Buchmueller и Manuel Abadia";
    strings["MAME Namco WSG by Nicola Salmoria and Aaron Giles##sgab"].plurals[0] = "MAME Namco WSG за авторством Nicola Salmoria и Aaron Giles";
    strings["MAME RF5C68 core by Olivier Galibert and Aaron Giles##sgab"].plurals[0] = "MAME RF5C68 core за авторством Olivier Galibert и Aaron Giles";
    strings["MAME MSM5232 core by Jarek Burczynski and Hiromitsu Shioya##sgab"].plurals[0] = "MAME MSM5232 core за авторством Jarek Burczynski и Hiromitsu Shioya";
    strings["MAME MSM6258 core by Barry Rodewald##sgab"].plurals[0] = "MAME MSM6258 core за авторством Barry Rodewald";
    strings["MAME YMZ280B core by Aaron Giles##sgab"].plurals[0] = "MAME YMZ280B core за авторством Aaron Giles";
    strings["MAME GA20 core by Acho A. Tang and R. Belmont##sgab"].plurals[0] = "MAME GA20 core за авторством Acho A. Tang и R. Belmont";
    strings["MAME SegaPCM core by Hiromitsu Shioya and Olivier Galibert##sgab"].plurals[0] = "MAME SegaPCM core за авторством Hiromitsu Shioya и Olivier Galibert";
    strings["SAASound by Dave Hooper and Simon Owen##sgab"].plurals[0] = "SAASound за авторством Dave Hooper и Simon Owen";
    strings["SameBoy by Lior Halphon##sgab"].plurals[0] = "SameBoy за авторством Lior Halphon";
    strings["Mednafen PCE, WonderSwan, T6W28 and Virtual Boy audio cores##sgab"].plurals[0] = "Ядра эмуляции Mednafen PCE, WonderSwan, T6W28 и Virtual Boy";
    strings["SNES DSP core by Blargg##sgab"].plurals[0] = "SNES DSP core за авторством Blargg";
    strings["puNES (NES, MMC5 and FDS) by FHorse##sgab"].plurals[0] = "puNES (NES, MMC5 и FDS) за авторством FHorse";
    strings["NSFPlay (NES and FDS) by Brad Smith and Brezza##sgab"].plurals[0] = "NSFPlay (NES and FDS) за авторством Brad Smith и Brezza";
    strings["reSID by Dag Lem##sgab"].plurals[0] = "reSID за авторством Dag Lem";
    strings["reSIDfp by Dag Lem, Antti Lankila##sgab"].plurals[0] = "reSIDfp за авторством Dag Lem, Antti Lankila";
    strings["and Leandro Nini##sgab"].plurals[0] = "и Leandro Nini";
    strings["dSID by DefleMask Team based on jsSID##sgab"].plurals[0] = "dSID за авторством команды DefleMask (на основе jsSID)";
    strings["Stella by Stella Team##sgab"].plurals[0] = "Stella за авторством Stella Team";
    strings["QSound emulator by superctr and Valley Bell##sgab"].plurals[0] = "Эмулятор QSound за авторством superctr и Valley Bell";
    strings["VICE VIC-20 sound core by Rami Rasanen and viznut##sgab"].plurals[0] = "Ядро эмуляции VICE VIC-20 за авторством Rami Rasanen и viznut";
    strings["VICE TED sound core by Andreas Boose, Tibor Biczo##sgab"].plurals[0] = "Ядро эмуляции VICE TED за авторством Andreas Boose, Tibor Biczo";
    strings["and Marco van den Heuvel##sgab"].plurals[0] = "и Marco van den Heuvel";
    strings["VERA sound core by Frank van den Hoef##sgab"].plurals[0] = "Ядро эмуляции VERA за авторством Frank van den Hoef";
    strings["mzpokeysnd POKEY emulator by Michael Borisov##sgab"].plurals[0] = "mzpokeysnd (эмулятор POKEY) за авторством Michael Borisov";
    strings["ASAP POKEY emulator by Piotr Fusik##sgab"].plurals[0] = "ASAP (эмулятор POKEY) за авторством Piotr Fusik";
    strings["ported by laoo to C++##sgab"].plurals[0] = "портирован на C++ laoo";
    strings["vgsound_emu (second version, modified version) by cam900##sgab"].plurals[0] = "vgsound_emu (версия вторая, модифицированная) за авторством cam900";
    strings["SM8521 emulator (modified version) by cam900##sgab"].plurals[0] = "Эмулятор SM8521 (модифицированная версия) за авторством cam900";
    strings["D65010G031 emulator (modified version) by cam900##sgab"].plurals[0] = "Эмулятор D65010G031 (модифицированная версия) за авторством cam900";
    strings["Namco C140/C219 emulator (modified version) by cam900##sgab"].plurals[0] = "Эмулятор C140/C219 (модифицированная версия) за авторством cam900";
    strings["ESFMu emulator (modified version) by Kagamiin~##sgab"].plurals[0] = "Эмулятор ESFMu (модифицированная версия) за авторством Kagamiin~";
    strings["PowerNoise emulator by scratchminer##sgab"].plurals[0] = "Эмулятор PowerNoise за авторством scratchminer";
    strings["ep128emu by Istvan Varga##sgab"].plurals[0] = "ep128emu за авторством Istvan Varga";
    strings["SID2 emulator (modification of reSID) by LTVA##sgab"].plurals[0] = "эмулятор SID2 (модификация reSID) за авторством LTVA";
    strings["5E01 emulator (modification of NSFPlay) by Euly##sgab"].plurals[0] = "эмулятор 5E01 (модификация NSFPlay) за авторством Euly";
    strings["NDS sound emulator by cam900##sgab"].plurals[0] = "NDS sound emulator за авторством cam900";
    strings["FZT sound source by LTVA##sgab"].plurals[0] = "Источник звука FZT за авторством LTVA";

    strings["greetings to:##sgab"].plurals[0] = "передаём привет:";
    strings["NEOART Costa Rica##sgab"].plurals[0] = "NEOART Costa Rica";
    strings["Xenium Demoparty##sgab"].plurals[0] = "Демопати Xenium";
    strings["all members of Deflers of Noice!##sgab"].plurals[0] = "всем участникам Deflers of Noice!";

    strings["copyright © 2021-2024 tildearrow##sgab"].plurals[0] = "Все права защищены © 2021-2024 tildearrow";
    strings["(and contributors).##sgab"].plurals[0] = "(и участники разработки).";
    strings["licensed under GPLv2+! see##sgab"].plurals[0] = "лицензировано по GPLv2+! см.";
    strings["LICENSE for more information.##sgab"].plurals[0] = "LICENSE для доп. информации.";

    strings["help Furnace grow:##sgab"].plurals[0] = "помогите в развитии Furnace:";
    strings["help Furnace-B:##sgab"].plurals[0] = "помогите в развитии Furnace-B:";

    strings["contact tildearrow at:##sgab"].plurals[0] = "вы можете связаться с tildearrow:";

    strings["disclaimer:##sgab"].plurals[0] = "внимание:";
    strings["despite the fact this program works##sgab"].plurals[0] = "несмотря на то что эта программа работает";
    strings["with the .dmf file format, it is NOT##sgab"].plurals[0] = "с файлами формата .dmf, она НЕ";
    strings["affiliated with Delek or DefleMask in##sgab"].plurals[0] = "связана с Delek'ом или программой DefleMask,";
    strings["any way, nor it is a replacement for##sgab"].plurals[0] = "и не является заменой";
    strings["the original program.##sgab"].plurals[0] = "оригинальной программы.";

    strings["it also comes with ABSOLUTELY NO WARRANTY.##sgab"].plurals[0] = "она также предоставляется БЕЗ КАКИХ-ЛИБО ГАРАНТИЙ.";

    strings["thanks to all contributors/bug reporters!##sgab"].plurals[0] = "спасибо всем, кто помогает с разработкой и сообщает о багах!";

    //src/gui/chanOsc.cpp

    strings["Volume##sgco"].plurals[0] = "Громкость";

    strings["Color##sgco1"].plurals[0] = "Цвет";

    strings["Error!##sgco"].plurals[0] = "Ошибка!";
    strings["\nquiet##sgco"].plurals[0] = "\nтихо";

    //   sgcp  src/gui/commandPalette.cpp

    strings["Cancel##sgcp"].plurals[0] = "Отмена";
    strings["cannot add chip! (##sgcp"].plurals[0] = "не могу добавить чип! (";

    //   sgcf  src/gui/compatFlags.cpp

    strings["Ignore duplicate slide effects##sgcf"].plurals[0] = "Игнорировать дублирующиеся эффекты авто-портаменто";
    strings["if this is on, only the first slide of a row in a channel will be considered.##sgcf"].plurals[0] = "при включении этого флага только первый эффект авто-портаменто в данной строке будет эффективен.";
    strings["Ignore 0Dxx on the last order##sgcf"].plurals[0] = "Игнорировать 0Dxx на последней строке матрицы паттернов";
    strings["if this is on, a jump to next row effect will not take place when it is on the last order of a song.##sgcf"].plurals[0] = "при включении флага эффект прыжка на следующий паттерн не будет работать, если паттерн проигрывается в последней строке матрицы паттернов.";
    strings["Don't apply Game Boy envelope on note-less instrument change##sgcf"].plurals[0] = "Не применять огибающую Game Boy при смене инструмента без ноты";
    strings["if this is on, an instrument change will not affect the envelope.##sgcf"].plurals[0] = "при включении этого флага смена инструмента без смены ноты не будет влиять на огибающую.";
    strings["Ignore DAC mode change outside of intended channel in ExtCh mode##sgcf"].plurals[0] = "Игнорировать переключение режима ЦАП, если оно не происходит на соответствующем канале, в режиме расширенного канала";
    strings["if this is on, 17xx has no effect on the operator channels in YM2612.##sgcf"].plurals[0] = "при включении этого флага 17xx не работает, если размещён на каналах операторов расширенного канала (для YM2612).";
    strings["SN76489 duty macro always resets phase##sgcf"].plurals[0] = "Макрос скважности SN76489 всегда сбрасывает фазу";
    strings["when enabled, duty macro will always reset phase, even if its value hasn't changed.##sgcf"].plurals[0] = "при включении этого флага макрос скважности всегда будет сбрасывать фазу, даже если значение скважности не меняется.";
    strings["Don't persist volume macro after it finishes##sgcf"].plurals[0] = "Не удерживать значение макроса громкости после его завершения";
    strings["when enabled, a value in the volume column that happens after the volume macro is done will disregard the macro.##sgcf"].plurals[0] = "при включении этого флага значение в столбце громкости, расположенное после завершения макроса громкости, не будет учитывать значение макроса.";
    strings["Old sample offset effect##sgcf"].plurals[0] = "Старый эффект начального смещения сэмпла";
    strings["behavior changed in 0.6.3##sgcf"].plurals[0] = "поведение изменено в версии 0.6.3";
    strings[".mod import##sgcf"].plurals[0] = "импорт .mod";
    strings["Don't slide on the first tick of a row##sgcf"].plurals[0] = "Не исполнять авто-портаменто в первый шаг движка трекера каждой строки";
    strings["simulates ProTracker's behavior of not applying volume/pitch slides on the first tick of a row.##sgcf"].plurals[0] = "симулирует поведение программы ProTracker, которая не исполняет изменения громкости и частоты на первом шаге движка каждого столбца.";
    strings["Reset arpeggio position on row change##sgcf"].plurals[0] = "Перезапуск позиции арпеджио при продвижении по строкам паттерна";
    strings["simulates ProTracker's behavior of arpeggio being bound to the current tick of a row.##sgcf"].plurals[0] = "симулирует поведение программы ProTracker, которая привязывает исполнение арпеджио к номеру шага движка в каждой строке паттерна.";
    strings["Pitch/Playback##sgcf"].plurals[0] = "Частота/Воспроизведение";
    strings["Pitch linearity:##sgcf"].plurals[0] = "Линейность частоты (в долях полутонов):";
    strings["None##sgcf"].plurals[0] = "Нет (прямая работа с частотой)";
    strings["like ProTracker/FamiTracker##sgcf"].plurals[0] = "как ProTracker/FamiTracker";
    strings["Full##sgcf"].plurals[0] = "Полная";
    strings["like Impulse Tracker##sgcf"].plurals[0] = "как Impulse Tracker";
    strings["Pitch slide speed multiplier##sgcf"].plurals[0] = "Множитель скорости команды авто-портаменто";
    strings["Loop modality:##sgcf"].plurals[0] = "Работа зацикливания:";
    strings["Reset channels##sgcf"].plurals[0] = "Перезапуск каналов";
    strings["select to reset channels on loop. may trigger a voltage click on every loop!##sgcf"].plurals[0] = "выберите для перезапуска каналов каждый раз в начале цикла. может вызывать щелчок в каждом начале цикла из-за смены напряжения!";
    strings["Soft reset channels##sgcf"].plurals[0] = "Мягкий перезапуск каналов";
    strings["select to turn channels off on loop.##sgcf"].plurals[0] = "выберите для отключения каналов в начале цикла.";
    strings["Do nothing##sgcf"].plurals[0] = "Ничего не делать";
    strings["select to not reset channels on loop.##sgcf"].plurals[0] = "выберите, чтобы отключить перезапуск каналов в начале цикла.";
    strings["Cut/delay effect policy:##sgcf"].plurals[0] = "Поведение эффектов заглушения/задержки ноты:";
    strings["Strict##sgcf"].plurals[0] = "Строгое";
    strings["only when time is less than speed (like DefleMask/ProTracker)##sgcf"].plurals[0] = "только в случае, когда параметр меньше скорости (как DefleMask/ProTracker)";
    strings["Lax##sgcf"].plurals[0] = "Нестрогое";
    strings["no checks##sgcf"].plurals[0] = "без проверок";
    strings["Simultaneous jump (0B+0D) treatment:##sgcf"].plurals[0] = "Поведение при одновременном прыжке (0B+0D):";
    strings["Normal##sgcf"].plurals[0] = "Нормальное";
    strings["accept 0B+0D to jump to a specific row of an order##sgcf"].plurals[0] = "принять 0B+0D как прыжок на конкретную строку паттерна на конкретной позиции матрицы паттернов";
    strings["Other##sgcf"].plurals[0] = "Другое";
    strings["Auto-insert one tick gap between notes##sgcf"].plurals[0] = "Автоматически вставлять паузу в 1 шаг движка между нотами";
    strings["when enabled, a one-tick note cut will be inserted between non-legato/non-portamento notes.\nthis simulates the behavior of some Amiga/SNES music engines.\n\nineffective on C64.##sgcf"].plurals[0] = "при включении этого флага между нотами без эффекта легато и авто-портаменто будет вставлено заглушение ноты длиной в один шаг движка.\nэто симуляция повдения некоторых музыкальных драйверов Amiga/SNES.\n\nничего не меняет для C64.";
    strings["Don't reset slides after note off##sgcf"].plurals[0] = "Не переинициализировать авто-портаменто после окончания ноты";
    strings["when enabled, note off will not reset the channel's slide effect.##sgcf"].plurals[0] = "при включении этого флага окончание ноты не будет останавливать авто-портаменто на этом канале.";
    strings["Don't reset portamento after reaching target##sgcf"].plurals[0] = "Не переинициализировать авто-портаменто после достижения цели";
    strings["when enabled, the slide effect will not be disabled after it reaches its target.##sgcf"].plurals[0] = "при включении этого флага эффект авто-портаменто не будет переинициализирован при достижении целевой частоты.";
    strings["Continuous vibrato##sgcf"].plurals[0] = "Непрерывное вибрато";
    strings["when enabled, vibrato phase/position will not be reset on a new note.##sgcf"].plurals[0] = "при включении этого флага фаза/положение вибрато не будут сбрасываться на новой ноте.";
    strings["Pitch macro is not linear##sgcf"].plurals[0] = "Нелинейный макрос частоты";
    strings["when enabled, the pitch macro of an instrument is in frequency/period space.##sgcf"].plurals[0] = "при включении этого флага макрос частоты будет работать относительно периода/частоты, а не долей полутона.";
    strings["Reset arpeggio effect position on new note##sgcf"].plurals[0] = "Сбрасывать положение эффекта арпеджио на новой ноте";
    strings["when enabled, arpeggio effect (00xy) position is reset on a new note.##sgcf"].plurals[0] = "при включении этого флага положение эффекта арпеджио (00xy) сбрасывается на новой ноте.";
    strings["Volume scaling rounds up##sgcf"].plurals[0] = "Масштабирование громкости округляется вверх";
    strings["when enabled, volume macros round up when applied\nthis prevents volume scaling from causing vol=0, which is silent on some chips\n\nineffective on logarithmic channels##sgcf"].plurals[0] = "при включении этого флага значения макросов громкости округляются вверх\nэто предотвращает возникновение ситуации vol=0 при масштабировании громкости, что приводит к заглушению на некоторых чипах\n\nне эффективно на чипах с логарифмическим контролем громкости";
    strings["Stop NES pulse channels hardware sweep on new note##sgcf"].plurals[0] = "Останавливать аппаратное портаменто на каналах импульсов NES при начале новой ноты";
    strings["Do not stop volume slide after reaching zero or full volume##sgcf"].plurals[0] = "Не останавливать эффект изменения громкости при достижении нулевой или максимальной громкости";
    strings["Stop E1xy/E2xy effects on new note##sgcf"].plurals[0] = "Останавливать эффекты E1xy/E2xy на новой ноте";
    strings["Slower 0Axy volume slide##sgcf"].plurals[0] = "Замедление изменения громкости эффектом 0Axy";

    //   sgcs  src/gui/csPlayer.cpp

    strings["Command Stream Player###Command Stream Player"].plurals[0] = "Проигрыватель потока команд###Command Stream Player";
    strings["Load##sgcs"].plurals[0] = "Загрузить";
    strings["Kill##sgcs"].plurals[0] = "Остановить";
    strings["Burn Current Song##sgcs"].plurals[0] = "Сохранить текущий трек";
    strings["Status##sgcs"].plurals[0] = "Статус";
    strings["Hex##sgcs"].plurals[0] = "Шест.";

    //   sgdl  src/gui/dataList.cpp

    strings["Bug!##sgdl"].plurals[0] = "Баг!";
    strings["Unknown##sgdl"].plurals[0] = "Неизвестный тип инструмента";
    strings["duplicate##sgdl0"].plurals[0] = "клонировать";
    strings["replace...##sgdl0"].plurals[0] = "заменить...";
    strings["save##sgdl0"].plurals[0] = "сохранить";
    strings["export (.dmp)##sgdl"].plurals[0] = "экспортировать в .dmp";
    strings["delete##sgdl0"].plurals[0] = "удалить";
    strings["%.2X: <INVALID>##sgdl"].plurals[0] = "%.2X: <НЕДЕЙСТВ.>";
    strings["- None -##sgdl"].plurals[0] = "- Нет -";
    strings["out of memory for this sample!##sgdl"].plurals[0] = "недостаточно памяти для этого сэмпла!";
    strings["make instrument##sgdl"].plurals[0] = "создать инструмент";
    strings["make me a drum kit##sgdl"].plurals[0] = "создать инструмент с набором ударных";
    strings["duplicate##sgdl1"].plurals[0] = "клонировать";
    strings["replace...##sgdl1"].plurals[0] = "заменить...";
    strings["save##sgdl1"].plurals[0] = "сохранить";
    strings["delete##sgdl1"].plurals[0] = "удалить";
    strings["Add##sgdl0"].plurals[0] = "Добавить";
    strings["Duplicate##sgdl2"].plurals[0] = "Клонировать";
    strings["Open##sgdl0"].plurals[0] = "Открыть";
    strings["replace instrument...##sgdl"].plurals[0] = "заменить инструмент...";
    strings["load instrument from TX81Z##sgdl"].plurals[0] = "загрузить инструмент с TX81Z";
    strings["replace wavetable...##sgdl"].plurals[0] = "заменить волновую таблицу...";
    strings["replace sample...##sgdl"].plurals[0] = "заменить сэмпл...";
    strings["import raw sample...##sgdl"].plurals[0] = "загрузить сырые данные сэмпла...";
    strings["import raw sample (replace)...##sgdl"].plurals[0] = "загрузить сырые данные сэмпла (заменить)...";
    strings["replace...##sgdl2"].plurals[0] = "заменить...";
    strings["load from TX81Z##sgdl"].plurals[0] = "загрузить с TX81Z";
    strings["Open (insert; right-click to replace)##sgdl"].plurals[0] = "Открыть (вставить; ПКМ для замены)";
    strings["Save##sgdl2"].plurals[0] = "Сохранить";
    strings["save instrument as .dmp...##sgdl"].plurals[0] = "сохранить инструмент как .dmp...";
    strings["save wavetable as .dmw...##sgdl"].plurals[0] = "сохранить волновую таблицу как .dmw...";
    strings["save raw wavetable...##sgdl"].plurals[0] = "сохранить сырые данные волновой таблицы...";
    strings["save raw sample...##sgdl"].plurals[0] = "сохранить сырые данные сэмпла...";
    strings["save as .dmp...##sgdl"].plurals[0] = "сохранить как .dmp...";
    strings["Toggle folders/standard view##sgdl0"].plurals[0] = "Переключиться между видом с разбиением по папкам и обычным видом";
    strings["Move up##sgdl0"].plurals[0] = "Переместить на одну позицию вверх";
    strings["Move down##sgdl0"].plurals[0] = "Переместить на одну позицию вниз";
    strings["Create##sgdl0"].plurals[0] = "Создать";
    strings["New folder##sgdl0"].plurals[0] = "Новая папка";
    strings["Preview (right click to stop)##sgdl0"].plurals[0] = "Превью (ПКМ для остановки)";
    strings["Delete##sgdl2"].plurals[0] = "Удалить";
    strings["Instruments##sgdl"].plurals[0] = "Инструменты";
    strings["<uncategorized>##sgdl0"].plurals[0] = "<нерассортированные>";
    strings["rename...##sgdl0"].plurals[0] = "переименовать...";
    strings["delete##sgdl3"].plurals[0] = "удалить";
    strings["Wavetables##sgdl"].plurals[0] = "Волновые таблицы";
    strings["Samples##sgdl"].plurals[0] = "Сэмплы";
    strings["Add##sgdl1"].plurals[0] = "Добавить";
    strings["Duplicate##sgdl3"].plurals[0] = "Клонировать";
    strings["Open##sgdl1"].plurals[0] = "Открыть";
    strings["replace...##sgdl3"].plurals[0] = "заменить...";
    strings["Save##sgdl3"].plurals[0] = "Сохранить";
    strings["save as .dmw...##sgdl"].plurals[0] = "сохранить как .dmw...";
    strings["save raw...##sgdl0"].plurals[0] = "сохранить сырые данные...";
    strings["Toggle folders/standard view##sgdl1"].plurals[0] = "Переключиться между видом с разбиением по папкам и обычным видом";
    strings["Move up##sgdl1"].plurals[0] = "Переместить на одну позицию вверх";
    strings["Move down##sgdl1"].plurals[0] = "Переместить на одну позицию вниз";
    strings["Create##sgdl1"].plurals[0] = "Создать";
    strings["New folder##sgdl1"].plurals[0] = "Новая папка";
    strings["Delete##sgdl4"].plurals[0] = "Удалить";
    strings["Add##sgdl2"].plurals[0] = "Добавить";
    strings["Duplicate##sgdl4"].plurals[0] = "Клонировать";
    strings["Open##sgdl2"].plurals[0] = "Открыть";
    strings["replace...##sgdl4"].plurals[0] = "заменить...";
    strings["import raw...##sgdl"].plurals[0] = "импорт сырых данных...";
    strings["import raw (replace)...##sgdl"].plurals[0] = "импорт сырых данных (заменить)...";
    strings["Save##sgdl4"].plurals[0] = "Сохранить";
    strings["save raw...##sgdl1"].plurals[0] = "сохранить сырые данные...";
    strings["Toggle folders/standard view##sgdl2"].plurals[0] = "Переключиться между видом с разбиением по папкам и обычным видом";
    strings["Move up##sgdl2"].plurals[0] = "Переместить на одну позицию вверх";
    strings["Move down##sgdl2"].plurals[0] = "Переместить на одну позицию вниз";
    strings["Create##sgdl2"].plurals[0] = "Создать";
    strings["New folder##sgdl2"].plurals[0] = "Новая папка";
    strings["Preview (right click to stop)##sgdl1"].plurals[0] = "Превью (ПКМ для остановки)";
    strings["Delete##sgdl5"].plurals[0] = "Удалить";
    strings["<uncategorized>##sgdl1"].plurals[0] = "<нерассортированные>";
    strings["rename...##sgdl1"].plurals[0] = "переименовать...";
    strings["delete##sgdl6"].plurals[0] = "удалить";
    strings["rename...##sgdl2"].plurals[0] = "переименовать...";
    strings["Delete##sgdl7"].plurals[0] = "Удалить";

    //src/gui/gui.cpp

    strings["Instrument %d##sggu"].plurals[0] = "Инструмент %d";
    strings["Sample %d"].plurals[0] = "Сэмпл %d";
    strings["the song is over!##sggu0"].plurals[0] = "трек закончился!";
    strings["the song is over!##sggu1"].plurals[0] = "трек закончился!";
    strings["Open File##sggu"].plurals[0] = "Открыть файл";
    strings["compatible files##sggu0"].plurals[0] = "совместимые файлы";
    strings["all files##sggu0"].plurals[0] = "все файлы";
    strings["no backups made yet!##sggu"].plurals[0] = "резервных копий пока нет!";
    strings["Restore Backup##sggu"].plurals[0] = "Загрузить резервную копию";
    strings["Furnace or Furnace-B song##sggu"].plurals[0] = "модуль Furnace или Furnace-B";
    strings["Furnace song##sggu0"].plurals[0] = "модуль Furnace";
    strings["Save File##sggu0"].plurals[0] = "Сохранить файл";
    strings["DefleMask 1.1.3 module##sggu"].plurals[0] = "модуль DefleMask 1.1.3";
    strings["Save File##sggu1"].plurals[0] = "Сохранить файл";
    strings["DefleMask 1.0/legacy module##sggu"].plurals[0] = "модуль DefleMask 1.0/legacy";
    strings["Save File##sggu2"].plurals[0] = "Сохранить файл";
    strings["Furnace-B song##sggu"].plurals[0] = "модуль Furnace-B";
    strings["Load Instrument##sggu"].plurals[0] = "загрузить инструмент";
    strings["all compatible files##sggu1"].plurals[0] = "все совместимые файлы";
    strings["Furnace instrument##sggu0"].plurals[0] = "инструмент Furnace";
    strings["DefleMask preset##sggu0"].plurals[0] = "пресет DefleMask";
    strings["TFM Music Maker instrument##sggu"].plurals[0] = "инструмент из программы TFM Music Maker";
    strings["VGM Music Maker instrument##sggu"].plurals[0] = "инструмент из программы VGM Music Maker";
    strings["Scream Tracker 3 instrument##sggu"].plurals[0] = "инструмент из программы Scream Tracker 3";
    strings["SoundBlaster instrument##sggu"].plurals[0] = "инструмент SoundBlaster";
    strings["Wohlstand OPL instrument##sggu"].plurals[0] = "инструмент из программы Wohlstand OPL";
    strings["Wohlstand OPN instrument##sggu"].plurals[0] = "инструмент из программы Wohlstand OPN";
    strings["Gens KMod patch dump##sggu"].plurals[0] = "дамп патчей Gens KMod";
    strings["BNK file (AdLib)##sggu"].plurals[0] = "файл BNK (AdLib)";
    strings["FF preset bank##sggu"].plurals[0] = "банк пресетов FF";
    strings["2612edit GYB preset bank##sggu"].plurals[0] = "банк пресетов 2612edit GYB";
    strings["VOPM preset bank##sggu"].plurals[0] = "банк пресетов VOPM";
    strings["Wohlstand WOPL bank##sggu"].plurals[0] = "банк Wohlstand WOPL";
    strings["Wohlstand WOPN bank##sggu"].plurals[0] = "банк Wohlstand WOPN";
    strings["all files##sggu1"].plurals[0] = "все файлы";
    strings["Save Instrument##sggu0"].plurals[0] = "Сохранить инструмент";
    strings["Furnace instrument##sggu1"].plurals[0] = "инструмент Furnace";
    strings["Save Instrument##sggu1"].plurals[0] = "Сохранить инструмент";
    strings["DefleMask preset##sggu1"].plurals[0] = "пресет DefleMask";
    strings["Load Wavetable##sggu"].plurals[0] = "Загрузить волновую таблицу";
    strings["compatible files##sggu2"].plurals[0] = "совместимые файлы";
    strings["all files##sggu2"].plurals[0] = "все файлы";
    strings["Save Wavetable##sggu0"].plurals[0] = "Сохранить волновую таблицу";
    strings["Furnace wavetable##sggu"].plurals[0] = "волновая таблица Furnace";
    strings["Save Wavetable##sggu1"].plurals[0] = "Сохранить волновую таблицу";
    strings["DefleMask wavetable##sggu"].plurals[0] = "волновая таблица DefleMask";
    strings["Save Wavetable##sggu2"].plurals[0] = "Сохранить волновую таблицу";
    strings["raw data##sggu"].plurals[0] = "сырые данные";
    strings["Load Sample##sggu"].plurals[0] = "Загрузить сэмпл";
    strings["compatible files##sggu3"].plurals[0] = "совместимые файлы";
    strings["all files##sggu3"].plurals[0] = "все файлы";
    strings["Load Raw Sample##sggu"].plurals[0] = "Загрузить сырые данные сэмпла";
    strings["all files##sggu4"].plurals[0] = "все файлы";
    strings["Save Sample##sggu"].plurals[0] = "Сохранить сэмпл";
    strings["Wave file##sggu0"].plurals[0] = "файл WAV";
    strings["Save Raw Sample##sggu"].plurals[0] = "Сохранить сырые данные сэмпла";
    strings["all files##sggu5"].plurals[0] = "все файлы";
    strings["Export Audio##sggu0"].plurals[0] = "Экспорт аудио";
    strings["Wave file##sggu1"].plurals[0] = "файл WAV";
    strings["Export Audio##sggu1"].plurals[0] = "Экспорт аудио";
    strings["Wave file##sggu2"].plurals[0] = "файл WAV";
    strings["Export Audio##sggu2"].plurals[0] = "Экспорт аудио";
    strings["Wave file##sggu3"].plurals[0] = "файл WAV";
    strings["Export VGM##sggu"].plurals[0] = "Экспорт VGM";
    strings["VGM file##sggu"].plurals[0] = "Файл VGM";
    strings["Export ZSM##sggu"].plurals[0] = "Экспорт ZSM";
    strings["ZSM file##sggu"].plurals[0] = "Файл ZSM";
    strings["Export Command Stream##sggu0"].plurals[0] = "Экспорт потока команд";
    strings["text file##sggu0"].plurals[0] = "текстовый файл";
    strings["Export Command Stream##sggu1"].plurals[0] = "Экспорт потока команд";
    strings["text file##sggu1"].plurals[0] = "текстовый файл";
    strings["Export Command Stream##sggu2"].plurals[0] = "Экспорт потока команд";
    strings["Export FZT module##sggu"].plurals[0] = "Экспорт модуля FZT";
    strings["FZT module##sggu"].plurals[0] = "Модуль FZT";
    strings["binary file##sggu"].plurals[0] = "бинарный файл";
    strings["Export Furnace song##sggu"].plurals[0] = "Экспорт модуля Furnace";
    strings["Furnace song##sggu2"].plurals[0] = "Модуль Furnace";
    strings["Coming soon!##sggu"].plurals[0] = "скоро появится!";
    strings["Select Font##sggu0"].plurals[0] = "Выберите шрифт";
    strings["compatible files##sggu4"].plurals[0] = "совместимые файлы";
    strings["Select Font##sggu1"].plurals[0] = "Выберите шрифт";
    strings["compatible files##sggu5"].plurals[0] = "совместимые файлы";
    strings["Select Font##sggu2"].plurals[0] = "Выберите шрифт";
    strings["compatible files##sggu6"].plurals[0] = "совместимые файлы";
    strings["Select Color File##sggu"].plurals[0] = "Выберите файл с настройками цветов";
    strings["configuration files##sggu0"].plurals[0] = "файлы конфигурации";
    strings["Select Keybind File##sggu"].plurals[0] = "Выберите файл с настройками клавиатуры";
    strings["configuration files##sggu1"].plurals[0] = "файлы конфигурации";
    strings["Select Layout File##sggu"].plurals[0] = "Выберите файл с настройками компоновки окон интерфейса";
    strings[".ini files##sggu0"].plurals[0] = "файлы .ini";
    strings["Select User Presets File##sggu"].plurals[0] = "Выберите файл пользовательских пресетов";
    strings["configuration files##sggu4"].plurals[0] = "файлы конфигурации";
    strings["Select Settings File##sggu"].plurals[0] = "Выберите файл с настройками";
    strings["configuration files##sggu5"].plurals[0] = "файлы конфигурации";
    strings["Export Colors##sggu"].plurals[0] = "Экспортировать настройки цветов";
    strings["configuration files##sggu2"].plurals[0] = "файлы конфигурации";
    strings["Export Keybinds##sggu"].plurals[0] = "Экспортировать настройки клавиатуры";
    strings["configuration files##sggu3"].plurals[0] = "файлы конфигурации";
    strings["Export Layout##sggu"].plurals[0] = "Экспортировать компоновку окон интерфейса";
    strings["Export User Presets##sggu"].plurals[0] = "Экспортировать пользовательские пресеты";
    strings["configuration files##sggu6"].plurals[0] = "файлы конфигурации";
    strings["Export Settings##sggu"].plurals[0] = "Экспортировать настройки";
    strings["configuration files##sggu7"].plurals[0] = "файлы конфигурации";
    strings[".ini files##sggu1"].plurals[0] = "файлы .ini";
    strings["Load ROM##sggu"].plurals[0] = "Загрузить ROM";
    strings["compatible files##sggu7"].plurals[0] = "совместимые файлы";
    strings["all files##sggu6"].plurals[0] = "все файлы";
    strings["Play Command Stream##sggu"].plurals[0] = "Воспроизвести поток команд";
    strings["command stream##sggu"].plurals[0] = "поток команд";
    strings["all files##sggu7"].plurals[0] = "все файлы";
    strings["Open Test##sggu"].plurals[0] = "Открыть (тест)";
    strings["compatible files##sggu8"].plurals[0] = "совместимые файлы";
    strings["another option##sggu0"].plurals[0] = "другая опция";
    strings["all files##sggu8"].plurals[0] = "все файлы";
    strings["Open Test (Multi)##sggu"].plurals[0] = "Открыть (тест, несколько файлов)";
    strings["compatible files##sggu9"].plurals[0] = "совместимые файлы";
    strings["another option##sggu1"].plurals[0] = "другая опция";
    strings["all files##sggu9"].plurals[0] = "все файлы";
    strings["Save Test##sggu"].plurals[0] = "Сохранить (тест)";
    strings["Furnace song##sggu"].plurals[0] = "Модуль Furnace";
    strings["DefleMask module##sggu"].plurals[0] = "Модуль DefleMask";
    strings["you have loaded a backup!\nif you need to, please save it somewhere.\n\nDO NOT RELY ON THE BACKUP SYSTEM FOR AUTO-SAVE!\nFurnace will not save backups of backups.##sggu"].plurals[0] = "вы загрузили резервную копию!\nесли необходимо, сохраните её где-то ещё.\n\nСИСТЕМА РЕЗЕРВНОГО КОПИРОВАНИЯ НЕ ЯВЛЯЕТСЯ СИСТЕМОЙ АВТОСОХРАНЕНИЯ!\nFurnace не сохраняет резервные копии резервных копий.";
    strings["cut##sggu"].plurals[0] = "вырезать";
    strings["copy##sggu"].plurals[0] = "копировать";
    strings["paste##sggu0"].plurals[0] = "вставить";
    strings["paste special...##sggu"].plurals[0] = "вставить...";
    strings["paste mix##sggu"].plurals[0] = "вставить поверх";
    strings["paste mix (background)##sggu"].plurals[0] = "вставить поверх (с заменой существующего)";
    strings["paste with ins (foreground)##sggu"].plurals[0] = "вставить поверх с инстр. (без замены существующего)";
    strings["no instruments available##sggu0"].plurals[0] = "нет доступных инструментов";
    strings["paste with ins (background)##sggu"].plurals[0] = "вставить поверх с инстр. (с заменой существующего)";
    strings["no instruments available##sggu1"].plurals[0] = "нет доступных инструментов";
    strings["paste flood##sggu"].plurals[0] = "вставить с цикл. повт. буфера (до конца патт.)";
    strings["paste overflow##sggu"].plurals[0] = "вставить (с возможным переходом в след. паттерн)";
    strings["delete##sggu0"].plurals[0] = "удалить";
    strings["select all##sggu"].plurals[0] = "выбрать всё";
    strings["operation mask...##sggu"].plurals[0] = "маска операций...";
    strings["delete##sggu1"].plurals[0] = "удаление";
    strings["pull delete##sggu"].plurals[0] = "удал. с подтяг. след. строк";
    strings["insert##sggu"].plurals[0] = "вставка пустой строки";
    strings["paste##sggu1"].plurals[0] = "вставка";
    strings["transpose (note)##sggu"].plurals[0] = "транспонирование (нота)";
    strings["transpose (value)##sggu"].plurals[0] = "транспонирование (других параметров)";
    strings["interpolate##sggu0"].plurals[0] = "интерполяция";
    strings["fade##sggu"].plurals[0] = "градиент/затухание";
    strings["invert values##sggu0"].plurals[0] = "инверсия значений";
    strings["scale##sggu"].plurals[0] = "масштабирование";
    strings["randomize##sggu"].plurals[0] = "заполнение случайными значениями";
    strings["flip##sggu"].plurals[0] = "переворот";
    strings["collapse/expand##sggu"].plurals[0] = "сжать/расширить";
    strings["input latch##sggu"].plurals[0] = "буфер ввода";
    strings["&&: selected instrument\n..: no instrument##sggu"].plurals[0] = "&&: выбранный инструмент\n..: без инструмента";
    strings["Set##sggu"].plurals[0] = "Очистить";
    strings["Reset##sggu"].plurals[0] = "Сбросить";
    strings["note up##sggu"].plurals[0] = "на полутон вверх";
    strings["note down##sggu"].plurals[0] = "на полутон вниз";
    strings["octave up##sggu"].plurals[0] = "на октаву вверх";
    strings["octave down##sggu"].plurals[0] = "на октаву вниз";
    strings["values up##sggu"].plurals[0] = "параметры вверх";
    strings["values down##sggu"].plurals[0] = "параметры вниз";
    strings["values up (+16)##sggu"].plurals[0] = "параметры вверх (+16)";
    strings["values down (-16)##sggu"].plurals[0] = "параметры вниз (-16)";
    strings["transpose##sggu"].plurals[0] = "транспонировать";
    strings["Notes##sggu"].plurals[0] = "Ноты";
    strings["Values##sggu"].plurals[0] = "Параметры";
    strings["interpolate##sggu1"].plurals[0] = "транспонировать";
    strings["change instrument...##sggu"].plurals[0] = "заменить инструмент...";
    strings["no instruments available##sggu"].plurals[0] = "нет доступных инструментов";
    strings["gradient/fade...##sggu"].plurals[0] = "градиент/затухание...";
    strings["Start##sggu"].plurals[0] = "Начало";
    strings["End##sggu"].plurals[0] = "Конец";
    strings["Nibble mode##sggu0"].plurals[0] = "Режим тетрад";
    strings["Go ahead##sggu"].plurals[0] = "Применить";
    strings["scale...##sggu"].plurals[0] = "масштабировать...";
    strings["Scale##sggu"].plurals[0] = "Масштабировать";
    strings["randomize...##sggu"].plurals[0] = "заполнить случайными значениями...";
    strings["Minimum##sggu"].plurals[0] = "Нижняя граница";
    strings["Maximum##sggu"].plurals[0] = "Верхняя граница";
    strings["Nibble mode##sggu1"].plurals[0] = "Режим тетрад";
    strings["Randomize##sggu"].plurals[0] = "Заполнить";
    strings["invert values##sggu1"].plurals[0] = "инвертировать параметры";
    strings["flip selection##sggu"].plurals[0] = "перевернуть выделенную область";
    strings["collapse/expand amount##CollapseAmount"].plurals[0] = "коэффициент сжатия/расширения##CollapseAmount";
    strings["collapse##sggu"].plurals[0] = "сжать";
    strings["expand##sggu"].plurals[0] = "расширить";
    strings["collapse pattern##sggu"].plurals[0] = "сжать паттерн";
    strings["expand pattern##sggu"].plurals[0] = "расширить паттерн";
    strings["collapse song##sggu"].plurals[0] = "сжать трек";
    strings["expand song##sggu"].plurals[0] = "расширить трек";
    strings["find/replace##sggu"].plurals[0] = "найти/заменить";
    strings["clone pattern##sggu"].plurals[0] = "клонировать паттерн";
    strings["Furnace has been started in Safe Mode.\nthis means that:\n\n- software rendering is being used\n- audio output may not work\n- font loading is disabled\n\ncheck any settings which may have made Furnace start up in this mode.\nfont loading is one of these.##sggu"].plurals[0] = "Furnace был запущен в безопасном режиме.\nэто означает:\n\n- используется программная отрисовка\n- может не работать вывод звука\n- отключена загрузка шрифтов\n\nпроверьте, какие настройки могли привести к запуску программы в этом режиме.\nзагрузка шрифтов может быть одной из таких.";
    strings["Unsaved changes! Save changes before opening file?##sggu0"].plurals[0] = "Остались несохранённые изменения! Сохранить их перед открытием файла?";
    strings["Unsaved changes! Save changes before opening file?##sggu2"].plurals[0] = "Остались несохранённые изменения! Сохранить их перед открытием файла?";
    strings["Error while loading file! (%s)##sggu0"].plurals[0] = "Ошибка при загрузке файла! (%s)";
    strings["Unsaved changes! Save changes before quitting?##sggu"].plurals[0] = "Остались несохранённые изменения! Сохранить их перед выходом?";
    strings["error while loading fonts! please check your settings.##sggu0"].plurals[0] = "Ошибка при загрузке шрифтов! Проверьте свои настройки.";
    strings["File##menubar"].plurals[0] = "Файл##menubar";
    strings["file##menubar"].plurals[0] = "файл##menubar";
    strings["new...##sggu"].plurals[0] = "новый...";
    strings["Unsaved changes! Save changes before creating a new song?##sggu"].plurals[0] = "Остались несохранённые изменения! Сохранить их перед созданием нового трека?";
    strings["open...##sggu"].plurals[0] = "открыть...";
    strings["Unsaved changes! Save changes before opening another file?##sggu"].plurals[0] = "Остались несохранённые изменения! Сохранить их перед открытием другого файла?";
    strings["open recent##sggu"].plurals[0] = "открыть недавние";
    strings["Unsaved changes! Save changes before opening file?##sggu1"].plurals[0] = "Остались несохранённые изменения! Сохранить их перед открытием файла?";
    strings["nothing here yet##sggu"].plurals[0] = "пока здесь ничего нет";
    strings["clear history##sggu"].plurals[0] = "очистить историю";
    strings["Are you sure you want to clear the recent file list?##sggu"].plurals[0] = "Вы уверены, что хотите очистить список недавних файлов?";
    strings["save##sggu"].plurals[0] = "сохранить";
    strings["Error while saving file! (%s)##sggu0"].plurals[0] = "Ошибка при сохранении файла! (%s)";
    strings["save as...##sggu"].plurals[0] = "сохранить как...";
    strings["export audio...##sggu0"].plurals[0] = "экспорт аудио...";
    strings["export VGM...##sggu0"].plurals[0] = "экспорт VGM...";
    strings["export .dmf (1.1.3+)...##sggu0"].plurals[0] = "экспорт .dmf (1.1.3+)...";
    strings["export .dmf (1.0/legacy)...##sggu0"].plurals[0] = "экспорт .dmf (1.0/legacy)...";
    strings["export ZSM...##sggu0"].plurals[0] = "экспорт ZSM...";
    strings["export Amiga validation data...##sggu0"].plurals[0] = "экспорт проверочного файла для компьютера Amiga...";
    strings["export text...##sggu0"].plurals[0] = "экспорт текста...";
    strings["export command stream...##sggu0"].plurals[0] = "экспорт потока команд...";
    strings["export FZT module...##sggu"].plurals[0] = "экспорт модуля FZT...";
    strings["export FZT module...##sggu1"].plurals[0] = "экспорт модуля FZT...";
    strings["export Furnace module...##sggu"].plurals[0] = "экспорт модуля Furnace...";
    strings["export audio...##sggu1"].plurals[0] = "экспорт аудио...";
    strings["export VGM...##sggu1"].plurals[0] = "экспорт VGM...";
    strings["export .dmf (1.1.3+)...##sggu1"].plurals[0] = "экспорт .dmf (1.1.3+)...";
    strings["export .dmf (1.0/legacy)...##sggu1"].plurals[0] = "экспорт .dmf (1.0/legacy)...";
    strings["export ZSM...##sggu1"].plurals[0] = "экспорт ZSM...";
    strings["export Amiga validation data...##sggu1"].plurals[0] = "экспорт проверочного файла для компьютера Amiga...";
    strings["export text...##sggu1"].plurals[0] = "экспорт текста...";
    strings["export command stream...##sggu1"].plurals[0] = "экспорт потока команд...";
    strings["export...##sggu"].plurals[0] = "экспорт...";
    strings["manage chips##sggu"].plurals[0] = "менеджер чипов";
    strings["add chip...##sggu"].plurals[0] = "добавить чип...";
    strings["cannot add chip! (##sggu"].plurals[0] = "не могу добавить чип! (";
    strings["configure chip...##sggu"].plurals[0] = "настроить чип...";
    strings["change chip...##sggu"].plurals[0] = "сменить чип...";
    strings["Preserve channel positions##sggu0"].plurals[0] = "Сохранить положение каналов";
    strings["remove chip...##sggu"].plurals[0] = "убрать чип...";
    strings["Preserve channel positions##sggu1"].plurals[0] = "Сохранить положение каналов";
    strings["cannot remove chip! (##sggu"].plurals[0] = "не могу убрать чип! (";
    strings["cannot change chip! (##sggu"].plurals[0] = "не могу сменить чип! (";
    strings["open built-in assets directory##sggu"].plurals[0] = "открыть внутреннюю папку с ресурсами";
    strings["restore backup##sggu"].plurals[0] = "загрузить резервную копию";
    strings["exit...##sggu"].plurals[0] = "выйти...";
    strings["Unsaved changes! Save before quitting?##sggu"].plurals[0] = "Остались несохранённые изменения! Сохранить их перед выходом?";
    strings["Edit##menubar"].plurals[0] = "Правка##menubar";
    strings["edit##menubar"].plurals[0] = "правка##menubar";
    strings["undo##sggu"].plurals[0] = "отменить";
    strings["redo##sggu"].plurals[0] = "вернуть";
    strings["clear...##sggu"].plurals[0] = "очистить...";
    strings["Settings##menubar"].plurals[0] = "Настройки##menubar";
    strings["settings##menubar"].plurals[0] = "настройки##menubar";
    strings["full screen##sggu"].plurals[0] = "полноэкранный режим";
    strings["lock layout##sggu"].plurals[0] = "зафиксировать компоновку окон";
    strings["pattern visualizer##sggu"].plurals[0] = "визуализатор эффектов в паттерне";
    strings["reset layout##sggu"].plurals[0] = "сбросить компоновку окон";
    strings["Are you sure you want to reset the workspace layout?##sggu"].plurals[0] = "Вы уверены, что хотите сброить компоновку окон интерфейса?";
    strings["switch to mobile view##sggu"].plurals[0] = "переключиться на мобильный интерфейс";
    strings["user systems...##sggu"].plurals[0] = "пользовательские системы...";
    strings["settings...##sggu"].plurals[0] = "настройки...";
    strings["Window##menubar"].plurals[0] = "Окно##menubar";
    strings["window##menubar"].plurals[0] = "окно##menubar";
    strings["song information##sggu"].plurals[0] = "о треке";
    strings["subsongs##sggu"].plurals[0] = "подпесни";
    strings["speed##sggu"].plurals[0] = "скорость";
    strings["assets##sggu"].plurals[0] = "ресурсы";
    strings["instruments##sggu"].plurals[0] = "инструменты";
    strings["wavetables##sggu"].plurals[0] = "волновые таблицы";
    strings["samples##sggu"].plurals[0] = "сэмплы";
    strings["orders##sggu"].plurals[0] = "матрица паттернов";
    strings["pattern##sggu"].plurals[0] = "паттерны";
    strings["mixer##sggu"].plurals[0] = "микшер";
    strings["grooves##sggu"].plurals[0] = "ритм-паттерны";
    strings["channels##sggu"].plurals[0] = "каналы";
    strings["pattern manager##sggu"].plurals[0] = "менеджер паттернов";
    strings["chip manager##sggu"].plurals[0] = "менеджер чипов";
    strings["compatibility flags##sggu"].plurals[0] = "флаги совместимости";
    strings["song comments##sggu"].plurals[0] = "комментарии трека";
    strings["song##sggu"].plurals[0] = "трек";
    strings["visualizers##sggu"].plurals[0] = "визуализаторы";
    strings["tempo##sggu"].plurals[0] = "темп";
    strings["debug##sggu"].plurals[0] = "отладка";
    strings["instrument editor##sggu"].plurals[0] = "редактор инструментов";
    strings["wavetable editor##sggu"].plurals[0] = "редактор волновых таблиц";
    strings["sample editor##sggu"].plurals[0] = "редактор сэмплов";
    strings["play/edit controls##sggu"].plurals[0] = "управление редактированием/воспроизведением";
    strings["piano/input pad##sggu"].plurals[0] = "клавиатура пианино/панель ввода";
    strings["oscilloscope (master)##sggu"].plurals[0] = "осциллограф";
    strings["oscilloscope (per-channel)##sggu"].plurals[0] = "осциллографы (для отдельных каналов)";
    strings["oscilloscope (X-Y)##sggu"].plurals[0] = "осциллограф (X-Y)";
    strings["volume meter##sggu"].plurals[0] = "измеритель громкости";
    strings["clock##sggu"].plurals[0] = "часы";
    strings["register view##sggu"].plurals[0] = "регистры";
    strings["log viewer##sggu"].plurals[0] = "просмотр логов";
    strings["statistics##sggu"].plurals[0] = "статистика";
    strings["memory composition##sggu"].plurals[0] = "содержание памяти";
    strings["spoiler##sggu"].plurals[0] = "спойлер";
    strings["Help##menubar"].plurals[0] = "Справка##menubar";
    strings["help##menubar"].plurals[0] = "справка##menubar";
    strings["effect list##sggu"].plurals[0] = "список эффектов";
    strings["debug menu##sggu"].plurals[0] = "отладка";
    strings["inspector##sggu"].plurals[0] = "отладка ImGUI";
    strings["panic##sggu"].plurals[0] = "паника";
    strings["about...##sggu"].plurals[0] = "о программе...";
    strings["| Speed %d:%d##sggu"].plurals[0] = "| Скорость %d:%d";
    strings["| Speed %d##sggu"].plurals[0] = "| Скорость %d";
    strings["| Groove##sggu"].plurals[0] = "| Ритм-паттерн";
    strings[" @ %gHz (%g BPM) ##sggu"].plurals[0] = " @ %g Гц (%g BPM) ";
    strings["| Order %.2X/%.2X ##sggu"].plurals[0] = "| Строка матр. патт. %.2X/%.2X ";
    strings["| Order %d/%d ##sggu"].plurals[0] = "| Строка матр. патт. %d/%d ";
    strings["| Row %.2X/%.2X ##sggu"].plurals[0] = "| Строка %.2X/%.2X ";
    strings["| Row %d/%d ##sggu"].plurals[0] = "| Строка %d/%d ";
    strings["Don't you have anything better to do?##sggu"].plurals[0] = "Вам точно больше нечем заняться?";
    strings["%d years ##sggu"].plurals[0] = "%d год ";
    strings["%d years ##sggu"].plurals[1] = "%d года ";
    strings["%d years ##sggu"].plurals[2] = "%d лет ";
    strings["%d months ##sggu"].plurals[0] = "%d месяц ";
    strings["%d months ##sggu"].plurals[1] = "%d месяца ";
    strings["%d months ##sggu"].plurals[2] = "%d месяцев ";
    strings["%d days ##sggu"].plurals[0] = "%d день ";
    strings["%d days ##sggu"].plurals[1] = "%d дня ";
    strings["%d days ##sggu"].plurals[2] = "%d дней ";
    strings["Note off (cut)##sggu"].plurals[0] = "\"Отпускание клавиши\" (резкое)";
    strings["Note off (release)##sggu"].plurals[0] = "\"Отпускание клавиши\" (с включением фазы затухания огибающей)";
    strings["Macro release only##sggu"].plurals[0] = "\"Отпускание клавиши\" (только для макросов)";
    strings["Note on: %s##sggu"].plurals[0] = "Нота: %s";
    strings["Ins %d: <invalid>##sggu"].plurals[0] = "Инструмент %d: <недейств.>";
    strings["Ins %d: %s##sggu"].plurals[0] = "Инструмент %d: %s";
    strings["Set volume: %d (%.2X, INVALID!)##sggu"].plurals[0] = "Громкость: %d (%.2X, НЕДЕЙСТВИТЕЛЬНА!)";
    strings["Set volume: %d (%.2X, %d%%)##sggu"].plurals[0] = "Громкость: %d (%.2X, %d%%)";
    strings["| modified##sggu"].plurals[0] = "| изменено";
    strings["there was an error in the file dialog! you may want to report this issue to:\nhttps://github.com/tildearrow/furnace/issues\ncheck the Log Viewer (window > log viewer) for more information.\n\nfor now please disable the system file picker in Settings > General.##sggu"].plurals[0] = "возникла ошибка в окне файлового диалога! возможно, вы захотите сообщить об ошибке:\nhttps://github.com/tildearrow/furnace/issues\nвы можете открыть просмотр логов (окно > просмотр логов) для получения дополнительной информации.\n\nпока можете отключить файловый диалог ОС в настройки > основные.";
    strings["can't do anything without Storage permissions!##sggu"].plurals[0] = "не могу ничего сделать без разрешения \"Хранилище\"!";
    strings["Zenity/KDialog not available!\nplease install one of these, or disable the system file picker in Settings > General.##sggu"].plurals[0] = "Zenity/KDialog недоступны!\nпожалуйста, установите один из них, или отключите файловый диалог ОС в настройки > основные.";
    strings["Error while loading file! (%s)##sggu2"].plurals[0] = "Ошибка при загрузке файла! (%s)";
    strings["Error while saving file! (%s)##sggu1"].plurals[0] = "Ошибка при сохранении файла! (%s)";
    strings["Error while loading file! (%s)##sggu3"].plurals[0] = "Ошибка при загрузке файла! (%s)";
    strings["Error while saving file! (%s)##sggu2"].plurals[0] = "Ошибка при сохранении файла! (%s)";
    strings["Error while saving file! (%s)##sggu3"].plurals[0] = "Ошибка при сохранении файла! (%s)";
    strings["error while saving instrument! only the following instrument types are supported:\n- FM (OPN)\n- SN76489/Sega PSG\n- Game Boy\n- PC Engine\n- NES\n- C64\n- FM (OPLL)\n- FDS##sggu"].plurals[0] = "ошибка при сохранении инструмента! поддерживаются только следующие типы инструментов:\n- FM (OPN)\n- SN76489/Sega PSG\n- Game Boy\n- PC Engine\n- NES\n- C64\n- FM (OPLL)\n- FDS";
    strings["there were some errors while loading samples:\n#sggu"].plurals[0] = "при загрузке сэмплов возникли следующие ошибки:\n";
    strings["...but you haven't selected a sample!##sggu0"].plurals[0] = "...но вы не выбрали сэмпл!";
    strings["could not save sample! open Log Viewer for more information.##sggu0"].plurals[0] = "не удалось сохранить сэмпл! откройте просмотрщик логов для получения дополнительной информации.";
    strings["could not save sample! open Log Viewer for more information.##sggu1"].plurals[0] = "не удалось сохранить сэмпл! откройте просмотрщик логов для получения дополнительной информации.";
    strings["there were some warnings/errors while loading instruments:\n#sggu"].plurals[0] = "при загрузке инструментов возникли следующие ошибки и предупреждения:\n";
    strings["> %s: cannot load instrument! (%s)\n#sggu"].plurals[0] = "> %s: не могу загрузить инструмент! (%s)\n";
    strings["...but you haven't selected an instrument!##sggu0"].plurals[0] = "...но вы не выбрали инструмент!";
    strings["cannot load instrument! (##sggu"].plurals[0] = "не могу загрузить инструмент! (";
    strings["congratulations! you managed to load nothing.\nyou are entitled to a bug report.##sggu"].plurals[0] = "поздравляю! вам удалось загрузить ничто.\nвы приглашаетесь к написанию отчёта об ошибке.";
    strings["there were some errors while loading wavetables:\n##sggu"].plurals[0] = "при загрузке волновых таблиц возникли следующие ошибки:\n";
    strings["cannot load wavetable! (##sggu"].plurals[0] = "не могу загрузить волновую таблицу! (";
    strings["...but you haven't selected a wavetable!##sggu"].plurals[0] = "...но вы не выбрали волновую таблицу!";
    strings["could not open file!##sggu"].plurals[0] = "не удалось открыть файл!";
    strings["could not write FZT module!##sggu"].plurals[0] = "не удалось записать модуль FZT!";
    strings["could not import user presets!##sggu"].plurals[0] = "не удалось импортировать пользовательские пресеты!";
    strings["could not import user presets! (%s)##sggu"].plurals[0] = "не удалось импортировать пользовательские пресеты! (%s)";
    strings["Could not write ZSM! (%s)##sggu"].plurals[0] = "Не удалось записать файл ZSM! (%s)";
    strings["could not write text! (%s)##sggu"].plurals[0] = "не удалось записать текстовый файл! (%s)";
    strings["could not write command stream! (%s)##sggu"].plurals[0] = "не удалось записать файл с потоком команд! (%s)";
    strings["could not write tildearrow version Furnace module! (%s)##sggu"].plurals[0] = "не удалось записать файл модуля для версии tildearrow! (%s)";
    strings["Error while loading file! (%s)##sggu4"].plurals[0] = "Ошибка при загрузке файла! (%s)";
    strings["You opened: %s##sggu"].plurals[0] = "Вы открыли: %s";
    strings["You opened:##sggu"].plurals[0] = "Вы открыли:";
    strings["You saved: %s##sggu"].plurals[0] = "Вы сохранили: %s";
    strings["Rendering...###Rendering..."].plurals[0] = "Рендер...###Rendering...";
    strings["Please wait...##sggu"].plurals[0] = "Пожалуйста, подождите...";
    strings["Abort##sggu"].plurals[0] = "Прервать";
    strings["New Song###New Song"].plurals[0] = "Новый трек###New Song";
    strings["Export###Export"].plurals[0] = "Экспорт###Export";
    strings["Error###Error"].plurals[0] = "Ошибка###Error";
    strings["OK##sggu0"].plurals[0] = "ОК";
    strings["Warning###Warning"].plurals[0] = "Внимание###Warning";
    strings["Yes##sggu0"].plurals[0] = "Да";
    strings["No##sggu0"].plurals[0] = "Нет";
    strings["Yes##sggu1"].plurals[0] = "Да";
    strings["No##sggu1"].plurals[0] = "Нет";
    strings["Yes##sggu2"].plurals[0] = "Да";
    strings["No##sggu2"].plurals[0] = "Нет";
    strings["Yes##sggu3"].plurals[0] = "Да";
    strings["No##sggu3"].plurals[0] = "Нет";
    strings["Cancel##sggu0"].plurals[0] = "Отмена";
    strings["Erasing##sggu"].plurals[0] = "Удалить:";
    strings["All subsongs##sggu"].plurals[0] = "Все подпесни";
    strings["Current subsong##sggu"].plurals[0] = "Текущую подпесню";
    strings["Orders##sggu"].plurals[0] = "Матрицу паттернов";
    strings["Pattern##sggu"].plurals[0] = "Паттерны";
    strings["Instruments##sggu"].plurals[0] = "Инструменты";
    strings["Wavetables##sggu"].plurals[0] = "Волновые таблицы";
    strings["Samples##sggu"].plurals[0] = "Сэмплы";
    strings["Optimization##sggu"].plurals[0] = "Оптимизировать:";
    strings["De-duplicate patterns##sggu"].plurals[0] = "Удалить дубликаты паттернов";
    strings["Remove unused instruments##sggu"].plurals[0] = "Удалить неиспользуемые инструменты";
    strings["Remove unused samples##sggu"].plurals[0] = "Удалить неиспользуемые сэмплы";
    strings["Never mind! Cancel##sggu1"].plurals[0] = "Не надо! Отмена";
    strings["Yes##sggu4"].plurals[0] = "Да";
    strings["No##sggu4"].plurals[0] = "Нет";
    strings["Yes##sggu5"].plurals[0] = "Да";
    strings["No##sggu5"].plurals[0] = "Нет";
    strings["Yes##sggu6"].plurals[0] = "Да";
    strings["No##sggu6"].plurals[0] = "Нет";
    strings["Yes##sggu7"].plurals[0] = "Да";
    strings["Yes##sggu8"].plurals[0] = "Да";
    strings["Yes##sggu9"].plurals[0] = "Да";
    strings["Yes##sggu10"].plurals[0] = "Да";
    strings["Yes##sggu11"].plurals[0] = "Да";
    strings["No##sggu8"].plurals[0] = "Нет";
    strings["No##sggu9"].plurals[0] = "Нет";
    strings["No##sggu10"].plurals[0] = "Нет";
    strings["No##sggu11"].plurals[0] = "Нет";
    strings["No##sggu12"].plurals[0] = "Нет";
    strings["Cancel##sggu4"].plurals[0] = "Отмена";
    strings["Cancel##sggu5"].plurals[0] = "Отмена";
    strings["Cancel##sggu6"].plurals[0] = "Отмена";
    strings["Cancel##sggu7"].plurals[0] = "Отмена";
    strings["Cancel##sggu8"].plurals[0] = "Отмена";
    strings["OK##sggu1"].plurals[0] = "ОК";
    strings["Drum kit mode:##sggu"].plurals[0] = "Режим создания набора ударных:";
    strings["Normal##sggu"].plurals[0] = "Обычный";
    strings["12 samples per octave##sggu"].plurals[0] = "12 сэмплов на октаву";
    strings["Starting octave##sggu"].plurals[0] = "Начальная октава";
    strings["too many instruments!##sggu"].plurals[0] = "слишком много инструментов!";
    strings["too many wavetables!##sggu"].plurals[0] = "слишком много волновых таблиц!";
    strings["Select Instrument###Select Instrument"].plurals[0] = "Выберите инструмент###Select Instrument";
    strings["this is an instrument bank! select which one to use:##sggu"].plurals[0] = "это банк инструментов! выберите, какой вы хотите использовать:";
    strings["this is an instrument bank! select which ones to load:##sggu"].plurals[0] = "это банк инструментов! выберите, какой вы хотите загрузить";
    strings["Search...##sggu"].plurals[0] = "Поиск...";
    strings["All##sggu"].plurals[0] = "Все";
    strings["None##sggu"].plurals[0] = "Никакой";
    strings["OK##sggu2"].plurals[0] = "ОК";
    strings["Cancel##sggu2"].plurals[0] = "Отмена";
    strings["...but you haven't selected an instrument!##sggu1"].plurals[0] = "...но вы не выбрали инструмент!";
    strings["Import Raw Sample###Import Raw Sample"].plurals[0] = "Импортировать сырые данные сэмпла###Import Raw Sample";
    strings["Data type:##sggu"].plurals[0] = "Тип данных:";
    strings["Sample rate##sggu"].plurals[0] = "Частота квантования";
    strings["Channels##sggu"].plurals[0] = "Число каналов";
    strings["(will be mixed down to mono)##sggu"].plurals[0] = "(будет сведено в моно)";
    strings["Unsigned##sggu"].plurals[0] = "Беззнаковый";
    strings["Big endian##sggu"].plurals[0] = "Обратный порядок байтов (Big endian)";
    strings["Swap nibbles##sggu"].plurals[0] = "Поменять местами тетрады";
    strings["Swap words##sggu"].plurals[0] = "Поменять местами машинные слова";
    strings["Encoding:##sggu"].plurals[0] = "Кодировка:";
    strings["Reverse bit order##sggu"].plurals[0] = "Обратный порядок битов";
    strings["OK##sggu3"].plurals[0] = "ОК";
    strings["...but you haven't selected a sample!##sggu1"].plurals[0] = "...но вы не выбрали сэмпл!";
    strings["Cancel##sggu3"].plurals[0] = "Отмена";
    strings["Error! No string provided!##sggu"].plurals[0] = "Ошибка! Не предоставлена строка!";
    strings["OK##sggu4"].plurals[0] = "ОК";
    strings["%.0fµs##sggu"].plurals[0] = "%.0f мс";
    strings["error while loading fonts! please check your settings.##sggu1"].plurals[0] = "Ошибка при загрузке шрифтов! пожалуйста, проверьте настройки.";
    strings["it appears I couldn't load these fonts. any setting you can check?##sggu"].plurals[0] = "кажется, я не могу загрузить эти шрифты. проверьте настройки?";
    strings["could not init renderer!\r\nfalling back to software renderer. please restart Furnace.##sggu"].plurals[0] = "не получилось инициализировать движок отрисовки!\r\nперехожу на программную отрисовку. пожалуйста, перезапустите Furnace.";
    strings["could not init renderer! %s\r\nfalling back to software renderer. please restart Furnace.##sggu"].plurals[0] = "не получилось инициализировать движок отрисовки! %s\r\nперехожу на программную отрисовку. пожалуйста, перезапустите Furnace.";
    strings["\r\nfalling back to software renderer. please restart Furnace.##sggu"].plurals[0] = "\r\nперехожу на программную отрисовку. пожалуйста, перезапустите Furnace.";
    strings["could not init renderer! %s##sggu"].plurals[0] = "не получилось инициализировать движок отрисовки! %s";
    strings["\r\nthe render driver has been set to a safe value. please restart Furnace.##sggu"].plurals[0] = "\r\nдвижок отрисовки был сброшен до безопасного. пожалуйста, перезапустите Furnace.";
    strings["could not open window! %s##sggu"].plurals[0] = "не удалось открыть окно! %s";
    strings["error while loading fonts! please check your settings.##sggu2"].plurals[0] = "Ошибка при загрузке шрифтов! пожалуйста, проверьте настройки.";
    strings["could NOT save layout! %s##sggu"].plurals[0] = "Не получилось сохранить компоновку окон! %s";

    //   sggc  src/gui/guiConst.cpp

    strings["Generic Sample##sggc"].plurals[0] = "Типичный сэмпл";
    strings["Beeper##sggc"].plurals[0] = "Пищалка";
    strings["VRC6 (saw)##sggc"].plurals[0] = "VRC6 (пила)";
    strings["OPL (drums)##sggc"].plurals[0] = "OPL (ударные)";
    strings["PowerNoise (noise)##sggc"].plurals[0] = "PowerNoise (шум)";
    strings["PowerNoise (slope)##sggc"].plurals[0] = "PowerNoise (скат)";
    strings["Forward##sggc"].plurals[0] = "Вперёд";
    strings["Backward##sggc"].plurals[0] = "Назад";
    strings["Ping pong##sggc"].plurals[0] = "Туда-обратно";
    strings["1-bit PCM##sggc"].plurals[0] = "1-битная ИКМ";
    strings["1-bit DPCM##sggc"].plurals[0] = "1-битная ДИКМ";
    strings["8-bit PCM##sggc"].plurals[0] = "8-битная ИКМ";
    strings["8-bit µ-law PCM##sggc"].plurals[0] = "8-битная ИКМ (µ-закон)";
    strings["16-bit PCM##sggc"].plurals[0] = "16-битная ИКМ";
    strings["none##sggc"].plurals[0] = "нет";
    strings["linear##sggc"].plurals[0] = "линейная";
    strings["cubic spline##sggc"].plurals[0] = "кубический сплайн";
    strings["blep synthesis##sggc"].plurals[0] = "BLEP-синтез";
    strings["sinc##sggc"].plurals[0] = "sinc";
    strings["best possible##sggc"].plurals[0] = "наилучший";
    strings["Pitch##sggc"].plurals[0] = "Частота";
    strings["Song##sggc"].plurals[0] = "Трек";
    strings["Time##sggc"].plurals[0] = "Время";
    strings["Speed##sggc0"].plurals[0] = "Скорость";
    strings["Panning##sggc"].plurals[0] = "Панорамирование";
    strings["Volume##sggc"].plurals[0] = "Громкость";
    strings["System (Primary)##sggc"].plurals[0] = "Основные эффекты чипа";
    strings["System (Secondary)##sggc"].plurals[0] = "Вспомогательные эффекты чипа";
    strings["Miscellaneous##sggc"].plurals[0] = "Разное";
    strings["Invalid##sggc"].plurals[0] = "Недейств.";

    strings["---Global##sggc"].plurals[0] = "---Global";
    strings["New##sggc"].plurals[0] = "Новый";
    strings["Open file##sggc"].plurals[0] = "Открыть файл";
    strings["Restore backup##sggc"].plurals[0] = "Загрузить резервную копию";
    strings["Save file##sggc"].plurals[0] = "Сохранить файл";
    strings["Save as##sggc"].plurals[0] = "Сохранить как";
    strings["Export##sggc"].plurals[0] = "Экспорт";
    strings["Undo##sggc"].plurals[0] = "Отменить";
    strings["Redo##sggc"].plurals[0] = "Вернуть";
    strings["Exit##sggc"].plurals[0] = "Выход";
    strings["Play/Stop (toggle)##sggc"].plurals[0] = "Старт/стоп (переключение)";
    strings["Play##sggc"].plurals[0] = "Воспроизведение";
    strings["Stop##sggc"].plurals[0] = "Стоп";
    strings["Play (from beginning)##sggc"].plurals[0] = "Воспроизведение (с начала)";
    strings["Play (repeat pattern)##sggc"].plurals[0] = "Воспроизведение (зациклить текущий паттерн)";
    strings["Play from cursor##sggc"].plurals[0] = "Воспроизведение с курсора";
    strings["Step row##sggc"].plurals[0] = "Сделать один шаг по паттерну";
    strings["Octave up##sggc"].plurals[0] = "На октаву вверх";
    strings["Octave down##sggc"].plurals[0] = "На октаву вниз";
    strings["Previous instrument##sggc"].plurals[0] = "Предыдущий инструмент";
    strings["Next instrument##sggc"].plurals[0] = "Следующий инструмент";
    strings["Increase edit step##sggc"].plurals[0] = "Увеличить шаг редактирования";
    strings["Decrease edit step##sggc"].plurals[0] = "Уменьшить шаг редактирования";
    strings["Toggle edit mode##sggc"].plurals[0] = "Переключить режим редактирования";
    strings["Metronome##sggc"].plurals[0] = "Метроном";
    strings["Toggle repeat pattern##sggc"].plurals[0] = "Переключить режим зацикливания паттерна";
    strings["Follow orders##sggc"].plurals[0] = "След. за воспр. в матр. патт.";
    strings["Follow pattern##sggc"].plurals[0] = "След. за воспр. в патт.";
    strings["Toggle full-screen##sggc"].plurals[0] = "Перключить полноэкранный режим";
    strings["Request voice from TX81Z##sggc"].plurals[0] = "Запросить канал у TX81Z";
    strings["Panic##sggc"].plurals[0] = "Паника";
    strings["Clear song data##sggc"].plurals[0] = "Удалить данные трека";
    strings["Command Palette##sggc"].plurals[0] = "Палитра команд";
    strings["Recent files (Palette)##sggc"].plurals[0] = "Недавние файлы (палитра)";
    strings["Instruments (Palette)##sggc"].plurals[0] = "Инструменты (палитра)";
    strings["Samples (Palette)##sggc"].plurals[0] = "Сэмплы (палитра)";
    strings["Change instrument (Palette)##sggc"].plurals[0] = "Сменить инструмент (палитра)";
    strings["Add chip (Palette)##sggc"].plurals[0] = "Добавить чип (палитра)";
    strings["Edit Controls##sggc"].plurals[0] = "Редактирование";
    strings["Orders##sggc"].plurals[0] = "Матрица паттернов";
    strings["Instrument List##sggc"].plurals[0] = "Список инструментов";
    strings["Instrument Editor##sggc"].plurals[0] = "Редактор инструментов";
    strings["Song Information##sggc"].plurals[0] = "Информация о треке";
    strings["Speed##sggc1"].plurals[0] = "Скорость";
    strings["Pattern##sggc"].plurals[0] = "Паттерны";
    strings["Wavetable List##sggc"].plurals[0] = "Список волновых таблиц";
    strings["Wavetable Editor##sggc"].plurals[0] = "Редактор волновых таблиц";
    strings["Sample List##sggc"].plurals[0] = "Список сэмплов";
    strings["Sample Editor##sggc"].plurals[0] = "Редактор сэмплов";
    strings["About##sggc"].plurals[0] = "О программе";
    strings["Settings##sggc"].plurals[0] = "Настройки";
    strings["Mixer##sggc"].plurals[0] = "Микшер";
    strings["Debug Menu##sggc"].plurals[0] = "Отладка";
    strings["Oscilloscope (master)##sggc"].plurals[0] = "Осциллограф";
    strings["Volume Meter##sggc"].plurals[0] = "Измеритель громкости";
    strings["Statistics##sggc"].plurals[0] = "Статистика";
    strings["Compatibility Flags##sggc"].plurals[0] = "Флаги совместимости";
    strings["Piano##sggc"].plurals[0] = "Клавиатура пианино";
    strings["Song Comments##sggc"].plurals[0] = "Комментарии трека";
    strings["Channels##sggc"].plurals[0] = "Каналы";
    strings["Pattern Manager##sggc"].plurals[0] = "Менеджер паттернов";
    strings["Chip Manager##sggc"].plurals[0] = "Менеджер чипов";
    strings["Register View##sggc"].plurals[0] = "Регистры";
    strings["Log Viewer##sggc"].plurals[0] = "Просмотр логов";
    strings["Effect List##sggc"].plurals[0] = "Список эффектов";
    strings["Oscilloscope (per-channel)##sggc"].plurals[0] = "Осциллографы (для отдельных каналов)";
    strings["Subsongs##sggc"].plurals[0] = "Подпесни";
    strings["Find/Replace##sggc"].plurals[0] = "Найти/Заменить";
    strings["Clock##sggc"].plurals[0] = "Часы";
    strings["Grooves##sggc"].plurals[0] = "Ритм-паттерны";
    strings["Oscilloscope (X-Y)##sggc"].plurals[0] = "Осциллограф (X-Y)";
    strings["Memory Composition##sggc"].plurals[0] = "Содержание памяти";
    strings["Command Stream Player##sggc"].plurals[0] = "Проигрыватель потока команд";
    strings["User Presets##sggc"].plurals[0] = "Пользовательские пресеты";
    strings["Collapse/expand current window##sggc"].plurals[0] = "Свернуть/развернуть текущее окно";
    strings["Close current window##sggc"].plurals[0] = "Закрыть текущее окно";

    strings["---Pattern##sggc"].plurals[0] = "---Pattern";
    strings["Transpose (+1)##sggc"].plurals[0] = "Транспонировать (+1)";
    strings["Transpose (-1)##sggc"].plurals[0] = "Транспонировать (-1)";
    strings["Transpose (+1 octave)##sggc"].plurals[0] = "Транспонировать (+1 октава)";
    strings["Transpose (-1 octave)##sggc"].plurals[0] = "Транспонировать (-1 октава)";
    strings["Increase values (+1)##sggc"].plurals[0] = "Увеличить значения (+1)";
    strings["Increase values (-1)##sggc"].plurals[0] = "Уменьшить значения (-1)";
    strings["Increase values (+16)##sggc"].plurals[0] = "Увеличить значения (+16)";
    strings["Increase values (-16)##sggc"].plurals[0] = "Уменьшить значения (-16)";
    strings["Select all##sggc0"].plurals[0] = "Выбрать всё";
    strings["Cut##sggc0"].plurals[0] = "Вырезать";
    strings["Copy##sggc0"].plurals[0] = "Копировать";
    strings["Paste##sggc0"].plurals[0] = "Вставить";
    strings["Paste Mix (foreground)##sggc"].plurals[0] = "Вставить поверх";
    strings["Paste Mix (background)##sggc"].plurals[0] = "Вставить поверх (с заменой существующего)";
    strings["Paste Flood##sggc"].plurals[0] = "Вставить с цикл. повт. буфера (до конца патт.)";
    strings["Paste Overflow##sggc"].plurals[0] = "Вставить (с возможным переходом в след. паттерн)";
    strings["Move cursor up##sggc"].plurals[0] = "Курсор вверх";
    strings["Move cursor down##sggc"].plurals[0] = "Курсор вниз";
    strings["Move cursor left##sggc"].plurals[0] = "Курсор влево";
    strings["Move cursor right##sggc"].plurals[0] = "Курсор вправо";
    strings["Move cursor up by one (override Edit Step)##sggc"].plurals[0] = "Курсор вверх на один шаг (игнорировать шаг редактирования)";
    strings["Move cursor down by one (override Edit Step)##sggc"].plurals[0] = "Курсор вниз на один шаг (игнорировать шаг редактирования)";
    strings["Move cursor to previous channel##sggc"].plurals[0] = "Сдвинуть курсор на предыдущий канал";
    strings["Move cursor to next channel##sggc"].plurals[0] = "Сдвинуть курсор на следующий канал";
    strings["Move cursor to next channel (overflow)##sggc"].plurals[0] = "Сдвинуть курсор на предыдущий канал (с переполнением)";
    strings["Move cursor to previous channel (overflow)##sggc"].plurals[0] = "Сдвинуть курсор на следующий канал (с переполнением)";
    strings["Move cursor to beginning of pattern##sggc"].plurals[0] = "Сдвинуть курсор в начало паттерна";
    strings["Move cursor to end of pattern##sggc"].plurals[0] = "Сдвинуть курсор в конец паттерна";
    strings["Move cursor up (coarse)##sggc"].plurals[0] = "Курсор вверх (грубо)";
    strings["Move cursor down (coarse)##sggc"].plurals[0] = "Курсор вниз (грубо)";
    strings["Expand selection upwards##sggc"].plurals[0] = "Расширить выделенную область вверх";
    strings["Expand selection downwards##sggc"].plurals[0] = "Расширить выделенную область вниз";
    strings["Expand selection to the left##sggc"].plurals[0] = "Расширить выделенную область влево";
    strings["Expand selection to the right##sggc"].plurals[0] = "Расширить выделенную область вправо";
    strings["Expand selection upwards by one (override Edit Step)##sggc"].plurals[0] = "Расширить выделенную область вверх (игнорировать шаг редактирования)";
    strings["Expand selection downwards by one (override Edit Step)##sggc"].plurals[0] = "Расширить выделенную область вниз (игнорировать шаг редактирования)";
    strings["Expand selection to beginning of pattern##sggc"].plurals[0] = "Расширить выделенную область до начала паттерна";
    strings["Expand selection to end of pattern##sggc"].plurals[0] = "Расширить выделенную область до конца паттерна";
    strings["Expand selection upwards (coarse)##sggc"].plurals[0] = "Расширить выделенную область вверх (грубо)";
    strings["Expand selection downwards (coarse)##sggc"].plurals[0] = "Расширить выделенную область вниз (грубо)";
    strings["Move selection up##sggc"].plurals[0] = "Передвинуть выделенную область вверх";
    strings["Move selection down##sggc"].plurals[0] = "Передвинуть выделенную область вниз";
    strings["Move selection to previous channel##sggc"].plurals[0] = "Передвинуть выделенную область на предыдущий канал";
    strings["Move selection to next channel##sggc"].plurals[0] = "Передвинуть выделенную область на следующий канал";
    strings["Delete##sggc"].plurals[0] = "Удалить";
    strings["Pull delete##sggc"].plurals[0] = "Удалить с подтягиванием следующих строк";
    strings["Insert##sggc"].plurals[0] = "Вставить с сдвигом строк вниз";
    strings["Mute channel at cursor##sggc"].plurals[0] = "Заглушить выделенный канал";
    strings["Solo channel at cursor##sggc"].plurals[0] = "Соло выделенного канала";
    strings["Unmute all channels##sggc"].plurals[0] = "Включить все каналы";
    strings["Go to next order##sggc"].plurals[0] = "Перейти на следующую строку матрицы паттернов";
    strings["Go to previous order##sggc"].plurals[0] = "Перейти на предыдущую строку матрицы паттернов";
    strings["Collapse channel at cursor##sggc"].plurals[0] = "Сжать текущий канал";
    strings["Increase effect columns##sggc"].plurals[0] = "Добавить столбец эффектов";
    strings["Decrease effect columns##sggc"].plurals[0] = "Убрать столбец эффектов";
    strings["Interpolate##sggc"].plurals[0] = "Интерполировать";
    strings["Fade##sggc"].plurals[0] = "Затухание/градиент";
    strings["Invert values##sggc"].plurals[0] = "Инвертировать параметры";
    strings["Flip selection##sggc"].plurals[0] = "Перевернуть выделенную область";
    strings["Collapse rows##sggc"].plurals[0] = "Сжать строки";
    strings["Expand rows##sggc"].plurals[0] = "Расширить строки";
    strings["Collapse pattern##sggc"].plurals[0] = "Сжать паттерн";
    strings["Expand pattern##sggc"].plurals[0] = "Расширить паттерн";
    strings["Collapse song##sggc"].plurals[0] = "Сжать трек";
    strings["Expand song##sggc"].plurals[0] = "Расширить трек";
    strings["Set note input latch##sggc"].plurals[0] = "Задать буфер ввода для нот";
    strings["Change mobile scroll mode##sggc"].plurals[0] = "Переключить режим мобильной прокрутки";
    strings["Clear note input latch##sggc"].plurals[0] = "Очистить буфер ввода для нот";
    strings["Clone pattern##sggc"].plurals[0] = "Клонировать паттерн";

    strings["---Instrument list##sggc"].plurals[0] = "---Instrument list";
    strings["Add instrument##sggc0"].plurals[0] = "Добавить инструмент";
    strings["Duplicate instrument##sggc0"].plurals[0] = "Клонировать инструмент";
    strings["Open instrument##sggc0"].plurals[0] = "Открыть инструмент";
    strings["Open instrument (replace current)##sggc0"].plurals[0] = "Открыть инструмент (с заменой выделенного)";
    strings["Save instrument##sggc0"].plurals[0] = "Сохранить инструмент";
    strings["Export instrument (.dmp)##sggc"].plurals[0] = "Экспорт инструмента (.dmp)";
    strings["Move instrument up in list##sggc0"].plurals[0] = "Сдвинуть инструмент вверх в списке";
    strings["Move instrument down in list##sggc0"].plurals[0] = "Сдвинуть инструмент вниз в списке";
    strings["Delete instrument##sggc0"].plurals[0] = "Удалить инструмент";
    strings["Edit instrument##sggc0"].plurals[0] = "Редактировать инструмент";
    strings["Instrument cursor up##sggc0"].plurals[0] = "Курсор в списке инструментов вверх";
    strings["Instrument cursor down##sggc0"].plurals[0] = "Курсор в списке инструментов вниз";
    strings["Instruments: toggle folders/standard view##sggc0"].plurals[0] = "Инструменты: переключиться между видом с разбиением по папкам и обычным видом";
    strings["---Wavetable list##sggc"].plurals[0] = "---Wavetable list";
    strings["Add wavetable##sggc1"].plurals[0] = "Добавить волновую таблицу";
    strings["Duplicate wavetable##sggc1"].plurals[0] = "Клонировать волновую таблицу";
    strings["Open wavetable##sggc1"].plurals[0] = "Открыть волновую таблицу";
    strings["Open wavetable (replace current)##sggc1"].plurals[0] = "Открыть волновую таблицу (с заменой выделенной)";
    strings["Save wavetable##sggc1"].plurals[0] = "Сохранить волновую таблицу";
    strings["Save wavetable (.dmw)##sggc"].plurals[0] = "Сохранить волновую таблицу (.dmw)";
    strings["Save wavetable (raw)##sggc0"].plurals[0] = "Сохранить волновую таблицу (сырые данные)";
    strings["Move wavetable up in list##sggc1"].plurals[0] = "Сдвинуть волновую таблицу вверх в списке";
    strings["Move wavetable down in list##sggc1"].plurals[0] = "Сдвинуть волновую таблицу вниз в списке";
    strings["Delete wavetable##sggc1"].plurals[0] = "Удалить волновую таблицу";
    strings["Edit wavetable##sggc1"].plurals[0] = "Редактировать волновую таблицу";
    strings["Wavetable cursor up##sggc1"].plurals[0] = "Курсор в списке волновых таблиц вверх";
    strings["Wavetable cursor down##sggc1"].plurals[0] = "Курсор в списке волновых таблиц вниз";
    strings["Wavetables: toggle folders/standard view##sggc1"].plurals[0] = "Волновые таблицы: переключиться между видом с разбиением по папкам и обычным видом";
    strings["Paste wavetables from clipboard##sggc"].plurals[0] = "Вставить волновые таблицы из буфера обмена";
    strings["Paste local wavetables from clipboard##sggc1"].plurals[0] = "Вставить локальные волновые таблицы из буфера обмена";
    strings["---Sample list##sggc"].plurals[0] = "---Sample list";
    strings["Add sample##sggc2"].plurals[0] = "Добавить сэмпл";
    strings["Duplicate sample##sggc2"].plurals[0] = "Клонировать сэмпл";
    strings["Open sample##sggc2"].plurals[0] = "Открыть сэмпл";
    strings["Open sample (replace current)##sggc2"].plurals[0] = "Открыть сэмпл (с заменой выделенного)";
    strings["Import raw sample data##sggc"].plurals[0] = "Импорт сырых данных сэмпла";
    strings["Import raw sample data (replace current)##sggc"].plurals[0] = "Импорт сырых данных сэмпла (с заменой выделенного)";
    strings["Save sample##sggc2"].plurals[0] = "Сохранить сэмпл";
    strings["Save sample (raw)##sggc1"].plurals[0] = "Сохранить сэмпл (сырые данные)";
    strings["Move sample up in list##sggc2"].plurals[0] = "Сдвинуть сэмпл вверх в списке";
    strings["Move sample down in list##sggc2"].plurals[0] = "Сдвинуть сэмпл вниз в списке";
    strings["Delete sample##sggc2"].plurals[0] = "Удалить сэмпл";
    strings["Edit sample##sggc2"].plurals[0] = "Редатировать сэмпл";
    strings["Sample cursor up##sggc2"].plurals[0] = "Курсор в списке сэмплов вверх";
    strings["Sample cursor down##sggc2"].plurals[0] = "Курсор в списке сэмплов вниз";
    strings["Sample preview##sggc"].plurals[0] = "Превью сэмпла";
    strings["Stop sample preview##sggc"].plurals[0] = "Остановить превью сэмпла";
    strings["Samples: Toggle folders/standard view##sggc2"].plurals[0] = "Сэмплы: переключиться между видом с разбиением по папкам и обычным видом";
    strings["Samples: Make me a drum kit##sggc"].plurals[0] = "Сэмплы: сделать набор ударных";
    strings["---Sample editor##sggc"].plurals[0] = "---Sample editor";
    strings["Sample editor mode: Select##sggc"].plurals[0] = "Режим редактирования сэмпла: выделение";
    strings["Sample editor mode: Draw##sggc"].plurals[0] = "Режим редактирования сэмпла: рисование";
    strings["Sample editor: Cut##sggc1"].plurals[0] = "Редактор сэмплов: Вырезать";
    strings["Sample editor: Copy##sggc1"].plurals[0] = "Редактор сэмплов: Копировать";
    strings["Sample editor: Paste##sggc1"].plurals[0] = "Редактор сэмплов: Вставить";
    strings["Sample editor: Paste replace##sggc"].plurals[0] = "Редактор сэмплов: Вставить с заменой";
    strings["Sample editor: Paste mix##sggc"].plurals[0] = "Редактор сэмплов: Вставить со смешением";
    strings["Sample editor: Select all##sggc1"].plurals[0] = "Редактор сэмплов: Выбрать всё";
    strings["Sample editor: Resize##sggc"].plurals[0] = "Редактор сэмплов: Изменить размер";
    strings["Sample editor: Resample##sggc"].plurals[0] = "Редактор сэмплов: Изменить частоту дискретизации";
    strings["Sample editor: Amplify##sggc"].plurals[0] = "Редактор сэмплов: Усилить";
    strings["Sample editor: Normalize##sggc"].plurals[0] = "Редактор сэмплов: Нормализовать";
    strings["Sample editor: Fade in##sggc"].plurals[0] = "Редактор сэмплов: Плавное нарастание";
    strings["Sample editor: Fade out##sggc"].plurals[0] = "Редактор сэмплов: Плавное затухание";
    strings["Sample editor: Apply silence##sggc"].plurals[0] = "Редактор сэмплов: Применить тишину";
    strings["Sample editor: Insert silence##sggc"].plurals[0] = "Редактор сэмплов: Вставить тишину";
    strings["Sample editor: Delete##sggc3"].plurals[0] = "Редактор сэмплов: Удалить";
    strings["Sample editor: Trim##sggc"].plurals[0] = "Редактор сэмплов: Обрезать";
    strings["Sample editor: Reverse##sggc"].plurals[0] = "Редактор сэмплов: Реверс";
    strings["Sample editor: Invert##sggc"].plurals[0] = "Редактор сэмплов: Инверсия";
    strings["Sample editor: Signed/unsigned exchange##sggc"].plurals[0] = "Редактор сэмплов: Знаковый <-> беззнаковый";
    strings["Sample editor: Apply filter##sggc"].plurals[0] = "Редактор сэмплов: Применить фильтр";
    strings["Sample editor: Crossfade loop points##sggc"].plurals[0] = "Редактор сэмплов: Сделать плавный переход между началом и концом зацикленной части";
    strings["Sample editor: Preview sample##sggc"].plurals[0] = "Редактор сэмплов: Превью";
    strings["Sample editor: Stop sample preview##sggc"].plurals[0] = "Редактор сэмплов: Остановить превью";
    strings["Sample editor: Zoom in##sggc"].plurals[0] = "Редактор сэмплов: Увеличить масштаб";
    strings["Sample editor: Zoom out##sggc"].plurals[0] = "Редактор сэмплов: Уменьшить масштаб";
    strings["Sample editor: Toggle auto-zoom##sggc"].plurals[0] = "Редактор сэмплов: Переключить авто-увеличение";
    strings["Sample editor: Create instrument from sample##sggc"].plurals[0] = "Редактор сэмплов: Создать инструмент из сэмпла";
    strings["Sample editor: Set loop to selection##sggc"].plurals[0] = "Редактор сэмплов: Зациклить выделенную часть";
    strings["Sample editor: Create wavetable from selection##sggc"].plurals[0] = "Редактор сэмплов: Создать волновую таблицу из выделенной части";
    strings["---Orders##sggc"].plurals[0] = "---Orders";
    strings["Previous order##sggc"].plurals[0] = "Предыдущая строка матрицы паттернов";
    strings["Next order##sggc"].plurals[0] = "Следующая строка матрицы паттернов";
    strings["Order cursor left##sggc"].plurals[0] = "Курсор матрицы паттернов влево";
    strings["Order cursor right##sggc"].plurals[0] = "Курсор матрицы паттернов вправо";
    strings["Increase order value##sggc"].plurals[0] = "Увеличить значение строки матрицы паттернов";
    strings["Decrease order value##sggc"].plurals[0] = "Уменьшить значение строки матрицы паттернов";
    strings["Switch order edit mode##sggc"].plurals[0] = "Переключить режим редактирования матрицы паттернов";
    strings["Order: toggle alter entire row##sggc"].plurals[0] = "Матрица паттернов: Переключить режим изменения всей строки";
    strings["Add order##sggc3"].plurals[0] = "Добавить строку матрицы паттернов";
    strings["Duplicate order##sggc3"].plurals[0] = "Клонировать строку матрицы паттернов";
    strings["Deep clone order##sggc"].plurals[0] = "Клонировать строку матрицы паттернов с выделением новых индексов";
    strings["Copy current order to end of song##sggc"].plurals[0] = "Клонировать текущую строку матрицы паттернов в конец трека";
    strings["Deep clone current order to end of song##sggc"].plurals[0] = "Клонировать текущую строку матрицы паттернов в конец трека с выделением новых индексов";
    strings["Remove order##sggc"].plurals[0] = "Удалить строку матрицы паттернов";
    strings["Move order up##sggc3"].plurals[0] = "Сдвинуть строку матрицы паттернов вверх";
    strings["Move order down##sggc3"].plurals[0] = "Сдвинуть строку матрицы паттернов вниз";
    strings["Replay order##sggc"].plurals[0] = "Воспроизвести строку матрицы паттернов";

    strings["All chips##sggc"].plurals[0] = "Все чипы";
    strings["Square##sggc"].plurals[0] = "Меандр";
    strings["Wavetable##sggc"].plurals[0] = "Волн. табл.";
    strings["Special##sggc"].plurals[0] = "Особые";
    strings["Sample##sggc"].plurals[0] = "Сэмплеры";
    strings["Modern/fantasy##sggc"].plurals[0] = "Совр./вымышл.";

    //   sgda  src/gui/doAction.cpp

    strings["Unsaved changes! Save changes before creating a new song?##sgda"].plurals[0] = "Остались несохранённые изменения! Сохранить их перед созданием нового трека?";
    strings["Unsaved changes! Save changes before opening another file?##sgda"].plurals[0] = "Остались несохранённые изменения! Сохранить их перед открытием другого файла?";
    strings["Unsaved changes! Save changes before opening backup?##sgda"].plurals[0] = "Остались несохранённые изменения! Сохранить их перед открытием резервной копии?";
    strings["Error while saving file! (%s)##sgda"].plurals[0] = "Ошибка при сохранении файла! (%s)";
    strings["Error while sending request (MIDI output not configured?)##sgda"].plurals[0] = "Ошибка при посылке запроса (MIDI вывод не настроен?)";
    strings["Select an option: (cannot be undone!)##sgda"].plurals[0] = "Выберите опцию: (действие не может быть отменено!)";
    strings["too many instruments!##sgda0"].plurals[0] = "слишком много инструментов!";
    strings["too many instruments!##sgda1"].plurals[0] = "слишком много инструментов!";
    strings["too many wavetables!##sgda0"].plurals[0] = "слишком много волновых таблиц!";
    strings["too many wavetables!##sgda1"].plurals[0] = "слишком много волновых таблиц!";
    strings["too many samples!##sgda0"].plurals[0] = "слишком много сэмплов!";
    strings["too many samples!##sgda1"].plurals[0] = "слишком много сэмплов!";
    strings["couldn't paste! make sure your sample is 8 or 16-bit.##sgda"].plurals[0] = "Не получилось вставить сэмпл! убедитесь, что это 8- или 16-битный сэмпл.";
    strings["too many instruments!##sgda2"].plurals[0] = "слишком много инструментов!";
    strings["select at least one sample!##sgda"].plurals[0] = "выберите хотя бы один сэмпл!";
    strings["maximum size is 256 samples!##sgda"].plurals[0] = "максимальный размер равен 256 сэмплам!";
    strings["too many wavetables!##sgda2"].plurals[0] = "слишком много волновых таблиц!";

    //   sgec  src/gui/editControls.cpp

    strings["Mobile Edit###MobileEdit"].plurals[0] = "Мобильное меню редактирования###MobileEdit";
    strings["Mobile Controls###Mobile Controls"].plurals[0] = "Мобильное меню управления###Mobile Controls";
    strings["Mobile Menu###Mobile Menu"].plurals[0] = "Мобильное меню###Mobile Menu";
    strings["Pattern##sgec0"].plurals[0] = "Паттерны";
    strings["Orders##sgec0"].plurals[0] = "МАтрица паттернов";
    strings["Ins##sgec"].plurals[0] = "Инстр.";
    strings["Wave##sgec"].plurals[0] = "Волн. табл.";
    strings["Sample##sgec"].plurals[0] = "Сэмплы";
    strings["Song##sgec"].plurals[0] = "Трек";
    strings["Channels##sgec"].plurals[0] = "Каналы";
    strings["Chips##sgec"].plurals[0] = "Чипы";
    strings["Mixer##sgec"].plurals[0] = "Микшер";
    strings["Other##sgec"].plurals[0] = "Другое";
    strings["New##sgec"].plurals[0] = "Новый";
    strings["Unsaved changes! Save changes before creating a new song?##sgec"].plurals[0] = "Остались несохранённые изменения! Сохранить их перед созданием нового трека?";
    strings["Open##sgec"].plurals[0] = "Открыть";
    strings["Save##sgec"].plurals[0] = "Сохранить";
    strings["Save as...##sgec"].plurals[0] = "Сохранить как...";
    strings["Legacy .dmf##sgec"].plurals[0] = ".dmf (legacy)";
    strings["Export##sgec"].plurals[0] = "Экспорт";
    strings["Restore Backup##sgec"].plurals[0] = "Загрузить резервную копию";
    strings["Song Info##sgec"].plurals[0] = "О треке";
    strings["Subsongs##sgec"].plurals[0] = "Подпесни";
    strings["Speed##sgec"].plurals[0] = "Скорость";
    strings["Channels here...##sgec"].plurals[0] = "Каналы здесь...";
    strings["Chips here...##sgec"].plurals[0] = "Чипы здесь...";
    strings["What the hell...##sgec"].plurals[0] = "Что за хрень...";
    strings["Osc##sgec"].plurals[0] = "Осц.";
    strings["ChanOsc##sgec"].plurals[0] = "Осц-фы кан.";
    strings["RegView##sgec"].plurals[0] = "Регистры";
    strings["Stats##sgec"].plurals[0] = "Стат.";
    strings["Grooves##sgec"].plurals[0] = "Ритм-паттерны";
    strings["Compat Flags##sgec"].plurals[0] = "Флаги совм.";
    strings["XYOsc##sgec"].plurals[0] = "Осц. XY";
    strings["Panic##sgec"].plurals[0] = "Паника";
    strings["Settings##sgec"].plurals[0] = "Настройки";
    strings["Log##sgec"].plurals[0] = "Логи";
    strings["Debug##sgec"].plurals[0] = "Отладка";
    strings["About##sgec"].plurals[0] = "О программе";
    strings["Switch to Desktop Mode##sgec"].plurals[0] = "Переключ. на интерфейс ПК";
    strings["this is NOT ROM export! only use for making sure the\n"
            "Furnace Amiga emulator is working properly by\n"
            "comparing it with real Amiga output."].plurals[0] = 

            "это НЕ экспорт в файл ROM! используйте только для\n"
            "проверки того, что эмулятор Amiga в Furnace работает правильно,\n"
            "сравнивая звук настоящей Amiga и Furnace.";
    strings["Directory##sgec"].plurals[0] = "Папка";
    strings["Bake Data##sgec"].plurals[0] = "Создать данные";
    strings["Done! Baked %d files.##sgec"].plurals[0] = "Готово! Создан %d файл.";
    strings["Done! Baked %d files.##sgec"].plurals[1] = "Готово! Создано %d файла.";
    strings["Done! Baked %d files.##sgec"].plurals[2] = "Готово! Создано %d файлов.";
    strings["Play/Edit Controls###Play/Edit Controls"].plurals[0] = "Управл. воспр./ред.###Play/Edit Controls";
    strings["Octave##sgec0"].plurals[0] = "Октава";
    strings["Coarse Step##sgec0"].plurals[0] = "Грубый шаг редактирования";
    strings["Edit Step##sgec0"].plurals[0] = "Шаг редактирования";
    strings["Play##sgec0"].plurals[0] = "Воспроизвести";
    strings["Stop##sgec0"].plurals[0] = "Стоп";
    strings["Edit##sgec0"].plurals[0] = "Режим редактирования";
    strings["Metronome##sgec0"].plurals[0] = "Метроном";
    strings["Follow##sgec0"].plurals[0] = "Следовать за прогрессом воспроизведения";
    strings["Orders##sgec1"].plurals[0] = "Матрица паттернов";
    strings["Pattern##sgec1"].plurals[0] = "Паттерны";
    strings["Repeat pattern##sgec0"].plurals[0] = "Зациклить текущий паттерн";
    strings["Step one row##sgec0"].plurals[0] = "Сделать один шаг по паттерну";
    strings["Poly##PolyInput"].plurals[0] = "Полифония##PolyInput";
    strings["Mono##PolyInput"].plurals[0] = "Моно##PolyInput";
    strings["Polyphony##sgec0"].plurals[0] = "Полифония превью";
    strings["Stop##sgec1"].plurals[0] = "Стоп";
    strings["Play##sgec1"].plurals[0] = "Воспроизвести";
    strings["Step one row##sgec1"].plurals[0] = "Сделать один шаг по паттерну";
    strings["Repeat pattern##sgec1"].plurals[0] = "Зациклить текущий паттерн";
    strings["Edit##sgec1"].plurals[0] = "Режим редактирования";
    strings["Metronome##sgec1"].plurals[0] = "Метроном";
    strings["Octave##sgec1"].plurals[0] = "Октава";
    strings["Coarse Step##sgec1"].plurals[0] = "Грубый шаг";
    strings["Edit Step##sgec1"].plurals[0] = "Шаг";
    strings["Follow##sgec1"].plurals[0] = "Следовать за прогрессом воспроизведения";
    strings["Orders##sgec2"].plurals[0] = "Матрица паттернов";
    strings["Pattern##sgec2"].plurals[0] = "Паттерны";
    strings["Poly##PolyInput"].plurals[0] = "Полифония##PolyInput";
    strings["Mono##PolyInput"].plurals[0] = "Моно##PolyInput";
    strings["Polyphony##sgec1"].plurals[0] = "Полифония превью";
    strings["Play##sgec2"].plurals[0] = "Воспроизвести";
    strings["Stop##sgec2"].plurals[0] = "const ";
    strings["Step one row##sgec2"].plurals[0] = "Сделать один шаг по паттерну";
    strings["Repeat pattern##sgec2"].plurals[0] = "Зациклить текущий паттерн";
    strings["Edit##sgec2"].plurals[0] = "Режим редактирования";
    strings["Metronome##sgec2"].plurals[0] = "Метроном";
    strings["Oct.##sgec"].plurals[0] = "Окт.";
    strings["Octave##sgec2"].plurals[0] = "Октава";
    strings["Coarse##sgec0"].plurals[0] = "Грубый шаг";
    strings["Step##sgec0"].plurals[0] = "Шаг";
    strings["Foll.##sgec"].plurals[0] = "След.";
    strings["Follow##sgec2"].plurals[0] = "Следовать за прогрессом воспроизведения";
    strings["Ord##FollowOrders"].plurals[0] = "Матр.##FollowOrders";
    strings["Orders##sgec3"].plurals[0] = "Матрица паттернов";
    strings["Pat##FollowPattern"].plurals[0] = "Патт.##FollowPattern";
    strings["Pattern##sgec3"].plurals[0] = "Паттерны";
    strings["Poly##PolyInput"].plurals[0] = "Полифония##PolyInput";
    strings["Mono##PolyInput"].plurals[0] = "Моно##PolyInput";
    strings["Polyphony##sgec2"].plurals[0] = "Полифония превью";
    strings["Play Controls###Play Controls"].plurals[0] = "Управление воспроизведением###Play Controls";
    strings["Stop##sgec3"].plurals[0] = "Стоп";
    strings["Play##sgec3"].plurals[0] = "Воспроизвести";
    strings["Play from the beginning of this pattern##sgec"].plurals[0] = "Воспроизвести с начала этого паттерна";
    strings["Repeat from the beginning of this pattern##sgec"].plurals[0] = "Воспроизвести с начала этого паттерна с зацикливанием паттерна";
    strings["Step one row##sgec3"].plurals[0] = "Сделать один шаг по паттерну";
    strings["Edit##sgec3"].plurals[0] = "Редактировать";
    strings["Metronome##sgec3"].plurals[0] = "Метроном";
    strings["Repeat pattern##sgec3"].plurals[0] = "Зациклить текущий паттерн";
    strings["Poly##PolyInput"].plurals[0] = "Полифония##PolyInput";
    strings["Mono##PolyInput"].plurals[0] = "Моно##PolyInput";
    strings["Polyphony##sgec3"].plurals[0] = "Полифония превью";
    strings["Edit Controls###Edit Controls"].plurals[0] = "Управление редактированием###Edit Controls";
    strings["Octave##sgec3"].plurals[0] = "Октава";
    strings["Coarse##sgec1"].plurals[0] = "Грубый шаг";
    strings["Step##sgec1"].plurals[0] = "Шаг";
    strings["Follow orders##sgec"].plurals[0] = "След. за воспр. в матр. патт.";
    strings["Follow pattern##sgec"].plurals[0] = "След. за воспр. в патт.";

    //   sged  src/gui/editing.cpp

    strings["can't collapse any further!##sged"].plurals[0] = "дальнейшее сжатие невозможно!";
    strings["can't expand any further!##sged"].plurals[0] = "дальнейшее расширение невозможно!";

    //   sgef  src/gui/effectList.cpp

    strings["Effect List###Effect List"].plurals[0] = "Список эффектов###Effect List";
    strings["Chip at cursor: %s##sgef"].plurals[0] = "Курсор на канале чипа: %s";
    strings["Search##sgef"].plurals[0] = "Поиск";
    strings["Effect types to show:##sgef"].plurals[0] = "Показывать типы эффектов:";
    strings["All##sgef"].plurals[0] = "Все";
    strings["None##sgef"].plurals[0] = "Ни одного";
    strings["Name##sgef"].plurals[0] = "Имя";
    strings["Description##sgef"].plurals[0] = "Описание";
    strings["ERROR##sgef"].plurals[0] = "ОШИБКА";

    //   sgeo  src/gui/exportOptions.cpp

    strings["Export type:##sgeo"].plurals[0] = "Тип экспорта:";
    strings["one file##sgeo"].plurals[0] = "один файл";
    strings["multiple files (one per chip)##sgeo"].plurals[0] = "файлы (по одному на чип)";
    strings["multiple files (one per channel)##sgeo"].plurals[0] = "файлы (по одному на канал)";
    strings["Bit depth:##sgeo"].plurals[0] = "Глубина квантования:";
    strings["16-bit integer##sgeo"].plurals[0] = "16-битное целое число";
    strings["32-bit float##sgeo"].plurals[0] = "32-битное число с плавающей запятой";
    strings["Sample rate##sgeo"].plurals[0] = "Частота квантования";
    strings["Channels in file##sgeo"].plurals[0] = "Каналов аудио в файле";
    strings["Loops##sgeo"].plurals[0] = "Повторов трека";
    strings["Fade out (seconds)##sgeo"].plurals[0] = "Затухание (в секундах)";
    strings["Cancel##sgeo0"].plurals[0] = "Отмена";
    strings["Export##sgeo0"].plurals[0] = "Экспорт";
    strings["settings:##sgeo"].plurals[0] = "настройки:";
    strings["format version##sgeo"].plurals[0] = "версия формата";
    strings["loop##sgeo0"].plurals[0] = "зациклить";
    strings["loop trail:##sgeo"].plurals[0] = "маркер конца цикла:";
    strings["auto-detect##sgeo"].plurals[0] = "автоматически";
    strings["add one loop##sgeo1"].plurals[0] = "добавить один цикл";
    strings["custom##sgeo"].plurals[0] = "пользовательский";
    strings["add pattern change hints##sgeo"].plurals[0] = "добавить метки концов паттернов";
    strings["inserts data blocks on pattern changes.\n"
            "useful if you are writing a playback routine.\n\n"
            "the format of a pattern change data block is:\n"
            "67 66 FE ll ll ll ll 01 oo rr pp pp pp ...\n"
            "- ll: length, a 32-bit little-endian number\n"
            "- oo: order\n"
            "- rr: initial row (a 0Dxx effect is able to select a different row)\n"
            "- pp: pattern index (one per channel)\n\n"
            "pattern indexes are ordered as they appear in the song."].plurals[0] = 

            "вставляет блоки данных в местах смены паттернов.\n"
            "полезно, если вы пишете программу для воспроизведения.\n\n"
            "формат блока данных при смене паттерна:\n"
            "67 66 FE ll ll ll ll 01 oo rr pp pp pp ...\n"
            "- ll: длина, 32-битное число, прямой порядок байтов (little endian)\n"
            "- oo: строка матрицы паттернов\n"
            "- rr: начальная строка паттерна (эффект 0Dxx может её поменять)\n"
            "- pp: индекс паттерна (один на канал)\n\n"
            "индексы паттернов рассортированы в том порядке, в котором они\n"
            "встречаются в треке\n";
    strings["direct stream mode##sgeo"].plurals[0] = "запись прямого потока";
    strings["required for DualPCM and MSM6258 export.\n\n"
            "allows for volume/direction changes when playing samples,\n"
            "at the cost of a massive increase in file size."].plurals[0] = 

            "необходим для экспорта DualPCM и MSM6258.\n\n"
            "повзоляет записывать изменение громкости/направления воспроизведения сэмплов\n"
            "ценой сильного увеличения размера файла.";
    strings["chips to export:##sgeo"].plurals[0] = "экспорт следующих чипов:";
    strings["this chip is only available in VGM %d.%.2x and higher!##sgeo"].plurals[0] = "этот чип доступен только в файле VGM версии %d.%.2x и выше!";
    strings["this chip is not supported by the VGM format!##sgeo"].plurals[0] = "этот чип не поддерживается форматом VGM!";
    strings["select the chip you wish to export, but only up to %d of each type.##sgeo"].plurals[0] = "выберите чипы, данные для которых вы хотите включить в файл, но не более %d чипа каждого типа.";
    strings["select the chip you wish to export, but only up to %d of each type.##sgeo"].plurals[1] = "выберите чипы, данные для которых вы хотите включить в файл, но не более %d чипов каждого типа.";
    strings["select the chip you wish to export, but only up to %d of each type.##sgeo"].plurals[2] = "выберите чипы, данные для которых вы хотите включить в файл, но не более %d чипов каждого типа.";
    strings["Cancel##sgeo1"].plurals[0] = "Отмена";
    strings["Export##sgeo1"].plurals[0] = "Экспорт";
    strings["nothing to export##sgeo2"].plurals[0] = "нечего экспортировать";
    strings["Cancel##sgeo2"].plurals[0] = "Отмена";
    strings["Commander X16 Zsound Music File##sgeo"].plurals[0] = "Commander X16 Zsound Music File";
    strings["Tick Rate (Hz)##sgeo"].plurals[0] = "Частота движка (Гц)";
    strings["loop##sgeo2"].plurals[0] = "зациклить";
    strings["optimize size##sgeo"].plurals[0] = "оптимизировать размер";
    strings["Cancel##sgeo3"].plurals[0] = "Отмена";
    strings["Export##sgeo3"].plurals[0] = "Экспорт";
    strings["DefleMask file (1.1.3+)##sgeo"].plurals[0] = "Файл DefleMask (1.1.3+)";
    strings["Cancel##sgeo4"].plurals[0] = "Отмена";
    strings["Export##sgeo4"].plurals[0] = "Экспорт";
    strings["DefleMask file (1.0/legacy)##sgeo"].plurals[0] = "Файл DefleMask (1.0/legacy)";
    strings["Cancel##sgeo5"].plurals[0] = "Отмена";
    strings["Export##sgeo5"].plurals[0] = "Экспорт";
    strings["Directory##sgeo"].plurals[0] = "Папка";
    strings["Cancel##sgeo6"].plurals[0] = "Отмена";
    strings["Bake Data##sgeo"].plurals[0] = "Создать данные";
    strings["Done! Baked %d files.##sgeo"].plurals[0] = "Готово! Создан %d файл.";
    strings["Done! Baked %d files.##sgeo"].plurals[1] = "Готово! Создано %d файла.";
    strings["Done! Baked %d files.##sgeo"].plurals[2] = "Готово! Создано %d файлов.";
    strings["this option exports the song to a text file.\n##sgeo"].plurals[0] = "эта опция позволяет экспортировать трек в текстовый файл.\n";
    strings["Cancel##sgeo7"].plurals[0] = "Отмена";
    strings["Export##sgeo6"].plurals[0] = "Экспорт";
    strings["this option exports a binary file which\n"
            "contains a dump of the internal command stream\n"
            "produced when playing the song.\n\n"
            "technical/development use only!"].plurals[0] = 

            "эта опция позволяет создать бинарный файл,\n"
            "в котором содержится дамп внутренних команд,\n"
            "созданных во время проигрыаания трека.\n\n"
            "используйте только при разработке!";
    strings["Cancel##sgeo8"].plurals[0] = "Отмена";
    strings["Export##sgeo"].plurals[0] = "Экспорт";
    strings["this option exports a Flizzer Tracker module which\n"
    "is meant to be played back on Flipper Zero with\n"
    "Flizzer Tracker app installed."].plurals[0] = 

            "эта опция позволяет экспортировать модуль Flizzer Tracker'а,\n"
            "который можно воспроизвести на Flipper Zero с установленным\n"
            "приложением Flizzer Tracker.";
    strings["Cancel##sgeo9"].plurals[0] = "Отмена";
    strings["Export##sgeo9"].plurals[0] = "Экспорт";
    strings["this option exports a module which is\n"
            "compatible with tildearrow Furnace app.\n\n"

            "not all chips and inst macros will be supported!"].plurals[0] = 

            "эта опция позволяет экспортировать модуль,\n"
            "совместимый с версией Furnace от tildearrow.\n\n"

            "поддерживаются не все чипы и не все макросы инструментов!";
    strings["Cancel##sgeo9"].plurals[0] = "Отмена";
    strings["Export##sgeo7"].plurals[0] = "Экспорт";
    strings["Audio##sgeo"].plurals[0] = "Аудио";
    strings["DMF (1.0/legacy)##sgeo"].plurals[0] = "DMF (1.0/legacy)";
    strings["Amiga Validation##sgeo"].plurals[0] = "Проверка Amiga";
    strings["Text##sgeo"].plurals[0] = "Текст";
    strings["Command Stream##sgeo"].plurals[0] = "Поток команд";
    strings["Furnace##sgeo"].plurals[0] = "Furnace";
    strings["congratulations! you've unlocked a secret panel.##sgeo"].plurals[0] = "поздравляю! вы открыли секретную панель.";
    strings["Toggle hidden systems##sgeo"].plurals[0] = "Включить скрытые системы";
    strings["Toggle all instrument types##sgeo"].plurals[0] = "Включить все типы инструментов";
    strings["Set pitch linearity to Partial##sgeo"].plurals[0] = "Выставить частичную линейность высоты тона";
    strings["Enable multi-threading settings##sgeo"].plurals[0] = "Открыть настройки многопоточности";
    strings["Set fat to max##sgeo"].plurals[0] = "Выкрутить жирность на максимум";
    strings["Set muscle and fat to zero##sgeo"].plurals[0] = "Убрать все мышцы и жир";
    strings["Tell tildearrow this must be a mistake##sgeo"].plurals[0] = "Сказать tildearrow, что, возможно, это ошибка";
    strings["yeah, it's a bug. write a bug report in the GitHub page and tell me how did you get here.##sgeo"].plurals[0] = "да, это баг. заполните отчёт на гитхабе и скажите мне, как вы сюда добрались.";

    //   sgfr  src/gui/findReplace.cpp

    strings["ignore##sgfr"].plurals[0] = "игнорировать";
    strings["equals##sgfr"].plurals[0] = "равно";
    strings["not equal##sgfr"].plurals[0] = "не равно";
    strings["between##sgfr"].plurals[0] = "между";
    strings["not between##sgfr"].plurals[0] = "за пределами интервала";
    strings["any##sgfr"].plurals[0] = "любой";
    strings["none##sgfr"].plurals[0] = "никакого";
    strings["set##sgfr"].plurals[0] = "выставить значение";
    strings["add##sgfr"].plurals[0] = "добавить";
    strings["add (overflow)##sgfr"].plurals[0] = "добавить (с переполнением)";
    strings["scale %##sgfr"].plurals[0] = "масштабировать (в %)";
    strings["clear##sgfr"].plurals[0] = "удалить";

    strings["Find/Replace###Find/Replace"].plurals[0] = "Найти/Заменить###Find/Replace";
    strings["Find##sgfr0"].plurals[0] = "Найти";
    strings["order##sgfr0"].plurals[0] = "строка матрицы паттернов";
    strings["row##sgfr0"].plurals[0] = "строка";
    strings["order##sgfr1"].plurals[0] = "строка матрицы паттернов";
    strings["row##sgfr1"].plurals[0] = "строка";
    strings["channel##sgfr"].plurals[0] = "канал";
    strings["go##sgfr"].plurals[0] = "запустить";
    strings["no matches found!##sgfr"].plurals[0] = "совпадений не найдено!";
    strings["Back##sgfr"].plurals[0] = "Назад";
    strings["Note##sgfr0"].plurals[0] = "Нота";
    strings["Ins##sgfr0"].plurals[0] = "Инструмент";
    strings["Volume##sgfr0"].plurals[0] = "Громкость";
    strings["Effect##sgfr0"].plurals[0] = "Эффект";
    strings["Value##sgfr0"].plurals[0] = "Параметр";
    strings["Delete query##sgfr"].plurals[0] = "Удалить строку";
    strings["Add effect##sgfr0"].plurals[0] = "Добавить эффект";
    strings["Remove effect##sgfr0"].plurals[0] = "Удалить эффект";
    strings["Search range:##sgfr"].plurals[0] = "Диапазон поиска:";
    strings["Song##sgfr"].plurals[0] = "Трек";
    strings["Selection##sgfr"].plurals[0] = "Выделенное";
    strings["Pattern##sgfr"].plurals[0] = "Паттерн";
    strings["Confine to channels##sgfr"].plurals[0] = "Только выбранные каналы";
    strings["From##sgfr"].plurals[0] = "От";
    strings["To##sgfr"].plurals[0] = "До";
    strings["Match effect position:##sgfr"].plurals[0] = "Совпадение с положением эффекта:";
    strings["No##sgfr"].plurals[0] = "Нет";
    strings["match effects regardless of position.##sgfr"].plurals[0] = "эффекты обнаруживаются вне зависимости от положения.";
    strings["Lax##sgfr"].plurals[0] = "Нестрогое";
    strings["match effects only if they appear in-order.##sgfr"].plurals[0] = "эффекты обнаруживаются, если они идут в правильном порядке.";
    strings["Strict##sgfr"].plurals[0] = "Строгое";
    strings["match effects only if they appear exactly as specified.##sgfr"].plurals[0] = "эффекты обнаруживаются, если они полностью соответствуют запросу.";
    strings["Find##sgfr1"].plurals[0] = "Найти";
    strings["Replace##sgfr"].plurals[0] = "Заменить";
    strings["Note##sgfr1"].plurals[0] = "Нота";
    strings["INVALID##sgfr"].plurals[0] = "НЕДЕЙСТВ.";
    strings["Ins##sgfr1"].plurals[0] = "Инструмент";
    strings["Volume##sgfr1"].plurals[0] = "Громкость";
    strings["Effect##sgfr1"].plurals[0] = "Эффект";
    strings["Value##sgfr1"].plurals[0] = "Параметр";
    strings["Add effect##sgfr1"].plurals[0] = "Добавить эффект";
    strings["Remove effect##sgfr1"].plurals[0] = "Удалить эффект";
    strings["Effect replace mode:##sgfr"].plurals[0] = "Режим замены эффектов:";
    strings["Replace matches only##sgfr"].plurals[0] = "Заменять только совпадения";
    strings["Replace matches, then free spaces##sgfr"].plurals[0] = "Заменять совпадения, заполнять пустые ячейки";
    strings["Clear effects##sgfr"].plurals[0] = "Заменять эффекты";
    strings["Insert in free spaces##sgfr"].plurals[0] = "Вставлять эффекты в пустых ячейках";
    strings["Replace##QueryReplace"].plurals[0] = "Заменить##QueryReplace";

    //   sggv  src/gui/grooves.cpp

    strings["Grooves###Grooves"].plurals[0] = "Ритм-паттерны###Grooves";
    strings["use effect 09xx to select a groove pattern.##sggv"].plurals[0] = "используйте эффект 09xx для выбора ритм-паттерна.##sggv";
    strings["pattern##sggv"].plurals[0] = "паттерн##sggv";
    strings["remove##sggv"].plurals[0] = "удалить##sggv";

    //   sgie  src/gui/insEdit.cpp

    strings["Name##sgie"].plurals[0] = "Название";
    strings["Open##sgie0"].plurals[0] = "Открыть";
    strings["Save##sgie"].plurals[0] = "Сохранить";
    strings["export .dmp...##sgie"].plurals[0] = "экспорт .dmp...";
    strings["Type##sgie"].plurals[0] = "Тип";
    strings["Unknown##sgie"].plurals[0] = "Неизвестен";
    strings["none of the currently present chips are able to play this instrument type!##sgie"].plurals[0] = "ни один из добавленных чипов не поддерживает этот тип инструмента!";
    strings["Error##sgie"].plurals[0] = "Ошибка";
    strings["invalid instrument type! change it first.##sgie"].plurals[0] = "недопустимый тип инструмента! сначала поменяйте его.";
    strings["Instrument Editor###Instrument Editor"].plurals[0] = "Ред. инструментов###Instrument Editor";
    strings["waiting...##sgie0"].plurals[0] = "ожидание...";
    strings["waiting...##sgie1"].plurals[0] = "ожидание...";
    strings["no instrument selected##sgie0"].plurals[0] = "не выбрано ни одного инструмента";
    strings["no instrument selected##sgie1"].plurals[0] = "не выбрано ни одного инструмента";
    strings["select one...##sgie"].plurals[0] = "выберите один...";
    strings["or##sgie0"].plurals[0] = "или";
    strings["Open##sgie1"].plurals[0] = "Откройте";
    strings["or##sgie1"].plurals[0] = "или";
    strings["Create New##sgie"].plurals[0] = "Создайте новый";
    strings["copy##sgie"].plurals[0] = "копировать";
    strings["paste##sgie"].plurals[0] = "вставить";
    strings["clear contents##sgie"].plurals[0] = "очистить содержимое";
    strings["offset...##sgie"].plurals[0] = "сместить...";
    strings["offset##sgie"].plurals[0] = "сместить";
    strings["scale...##sgie"].plurals[0] = "масштабировать...";
    strings["scale##sgie"].plurals[0] = "масштабировать";
    strings["randomize...##sgie"].plurals[0] = "заполнить случайными значениями...";
    strings["Min##sgie"].plurals[0] = "Минимум";
    strings["Max##sgie"].plurals[0] = "Максимум";
    strings["randomize##sgie"].plurals[0] = "заполнить";

    //   sgmx  src/gui/mixer.cpp

    strings["input##sgmx"].plurals[0] = "вход";
    strings["output##sgmx"].plurals[0] = "выход";
    strings["Mixer##sgmx"].plurals[0] = "Микшер";
    strings["Master Volume##sgmx"].plurals[0] = "Общая громкость";
    strings["Invert##sgmx"].plurals[0] = "Инвертировать";
    strings["Volume##sgmx"].plurals[0] = "Громкость";
    strings["Panning##sgmx"].plurals[0] = "Панорамирование";
    strings["Front/Rear##sgmx"].plurals[0] = "Передн./задн.";
    strings["Patchbay##sgmx"].plurals[0] = "Соединение каналов";
    strings["Automatic patchbay##sgmx"].plurals[0] = "Автоматически";
    strings["Display hidden ports##sgmx"].plurals[0] = "Отобразить скрытые порты";
    strings["Display internal##sgmx"].plurals[0] = "Внутренние порты";
    strings["System##sgmx0"].plurals[0] = "Система";
    strings["Sample Preview##sgmx"].plurals[0] = "Превью сэмпла";
    strings["Metronome##sgmx"].plurals[0] = "Метроном";
    strings["System##sgmx1"].plurals[0] = "Система";
    strings["disconnect all##sgmx"].plurals[0] = "отсоединить все";

    //   sgns  src/gui/newSong.cpp

    strings["Choose a System!##sgns"].plurals[0] = "Выберите систему!";
    strings["Search...##sgns"].plurals[0] = "Поиск...";
    strings["Categories##sgns"].plurals[0] = "Категории";
    strings["Systems##sgns"].plurals[0] = "Системы";
    strings["no systems here yet!##sgns"].plurals[0] = "пока здесь нет ни одной системы!";
    strings["no results##sgns"].plurals[0] = "ничего не найдено";
    strings["I'm feeling lucky##sgns"].plurals[0] = "Мне повезёт!";
    strings["no categories available! what in the world.##sgns"].plurals[0] = "нет доступных категорий! что происходит...";
    strings["it appears you're extremely lucky today!##sgns"].plurals[0] = "похоже, что вам сегодня очень повезло!";
    strings["Cancel##sgns"].plurals[0] = "Отмена";

    //   sgme  src/gui/memory.cpp

    strings["Memory Composition###Memory Composition"].plurals[0] = "Содержание памяти###Memory Composition";
    strings["bank %d##sgme"].plurals[0] = "банк %d";
    strings["%d-%d ($%x-$%x): %d bytes ($%x)##sgme"].plurals[0] = "%d-%d ($%x-$%x): %d байт ($%x)";
    strings["%d-%d ($%x-$%x): %dK ($%x)##sgme"].plurals[0] = "%d-%d ($%x-$%x): %d КиБ ($%x)";
    strings["no chips with memory##sgme"].plurals[0] = "нет чипов с памятью";

    //   sgor  src/gui/orders.cpp

    strings["Add new order##sgor"].plurals[0] = "Добавить строку";
    strings["Remove order##sgor"].plurals[0] = "Удалить строку";
    strings["Duplicate order (right-click to deep clone)##sgor"].plurals[0] = "Клонировать строку (ПКМ для клонирования с выделением новых индексов)";
    strings["Move order up##sgor"].plurals[0] = "Сдвинуть строку вверх";
    strings["Move order down##sgor"].plurals[0] = "Сдвинуть строку вниз";
    strings["Place copy of current order at end of song (right-click to deep clone)##sgor"].plurals[0] = "Поместить копию строки (ПКМ для копирования с выделением новых индексов) в конец трека";
    strings["Order change mode: entire row##sgor"].plurals[0] = "Режим изменения: вся строка";
    strings["Order change mode: one##sgor"].plurals[0] = "Режим изменения: ячейка";
    strings["Order edit mode: Select and type (scroll vertically)##sgor"].plurals[0] = "Режим редактирования: Выбрать и печатать (вертикальная прокрутка)";
    strings["Order edit mode: Select and type (scroll horizontally)##sgor"].plurals[0] = "Режим редактирования: Выбрать и печатать (горизонтальная прокрутка)";
    strings["Order edit mode: Select and type (don't scroll)##sgor"].plurals[0] = "Режим редактирования: Выбрать и печатать (без прокрутки)";
    strings["Order edit mode: Click to change##sgor"].plurals[0] = "Режим редактирования: нажмите для изменения";

    //   sgos  src/gui/osc.cpp

    strings["Oscilloscope###Oscilloscope"].plurals[0] = "Осциллограф###Oscilloscope";
    strings["zoom: %.2fx (%.1fdB)##sgos"].plurals[0] = "увеличение: %.2fx (%.1f дБ)";
    strings["window size: %.1fms##sgos"].plurals[0] = "ширина окна: %.1f мс";
    strings["(-Infinity)dB##sgos"].plurals[0] = "(минус бесконечность) дБ";

    //   sgpm  src/gui/patManager.cpp

    strings["Pattern Manager###Pattern Manager"].plurals[0] = "Менеджер паттернов###Pattern Manager";
    strings["De-duplicate patterns##sgpm"].plurals[0] = "Удалить дубликаты паттернов";
    strings["Re-arrange patterns##sgpm"].plurals[0] = "Отсортировать паттерны";
    strings["Sort orders##sgpm"].plurals[0] = "Отсортировать столбцы матрицы паттернов";
    strings["Make patterns unique##sgpm"].plurals[0] = "Сделать паттерны уникальными";
    strings["Pattern %.2X\n- not allocated##sgpm"].plurals[0] = "Паттерн %.2X\n- не используется";
    strings["Pattern %.2X\n- use count: %d (%.0f%%)\n\nright-click to erase##sgpm"].plurals[0] = "Паттерн %.2X\n- сколько раз встречается в треке: %d (%.0f%%)\n\nПКМ, чтобы удалить";

    //   sgpa  src/gui/pattern.cpp

    strings["Pattern###Pattern"].plurals[0] = "Паттерны###Pattern";
    strings["there aren't any channels to show.##sgpa"].plurals[0] = "нет каналов для отображения.";
    strings["click for pattern options (effect columns/pattern names/visualizer)##sgpa"].plurals[0] = "нажмите для открытия меню настроек (столбцы эффектов/названия паттернов/визуализатор эффектов)";
    strings["Options:##sgpa"].plurals[0] = "Настройки:";
    strings["Effect columns/collapse##sgpa"].plurals[0] = "Столбцы эффектов и свёртка канала";
    strings["Pattern names##sgpa"].plurals[0] = "Названия паттернов";
    strings["Channel group hints##sgpa"].plurals[0] = "Подсказки о группировке каналов";
    strings["Visualizer##sgpa"].plurals[0] = "Визуализатор эффектов";
    strings["Channel status:##sgpa"].plurals[0] = "Статус канала:";
    strings["No##_PCS0"].plurals[0] = "Нет##_PCS0";
    strings["Yes##_PCS1"].plurals[0] = "Да##_PCS1";
    strings["WARNING!!##sgpa"].plurals[0] = "ВНИМАНИЕ!!!";
    strings["this instrument cannot be previewed because##sgpa"].plurals[0] = "превью этого инструмента невозможно, потому что";
    strings["none of the chips can play it##sgpa"].plurals[0] = "ни один из чипов не совместим с ним";
    strings["your instrument is in peril!! be careful...##sgpa"].plurals[0] = "ваш инструмент под угрозой!! будьте осторожны...";

    //   sgpi  src/gui/piano.cpp

    strings["Piano###Piano"].plurals[0] = "Клав. пианино###Piano";
    strings["Options##sgpi"].plurals[0] = "Настройки";
    strings["Key layout:##sgpi"].plurals[0] = "Компоновка клавиш:";
    strings["Automatic##sgpi"].plurals[0] = "Автоматически";
    strings["Standard##sgpi"].plurals[0] = "Стандартная";
    strings["Continuous##sgpi"].plurals[0] = "Непрерывная";
    strings["Value input pad:##sgpi"].plurals[0] = "Панель ввода значений:";
    strings["Disabled##sgpi"].plurals[0] = "Выкл.";
    strings["Replace piano##sgpi"].plurals[0] = "Заменяет пианино";
    strings["Split (automatic)##sgpi"].plurals[0] = "Разделённая (автоматически)";
    strings["Split (always visible)##sgpi"].plurals[0] = "Разделённая (всегда отображать)";
    strings["Share play/edit offset/range##sgpi"].plurals[0] = "Соблюдать настройки из окна настроек воспроизв./ред.";
    strings["Read-only (can't input notes)##sgpi"].plurals[0] = "Только чтение (нельзя вводить ноты)";
    strings["Input Pad###Input Pad"].plurals[0] = "Панель ввода###Input Pad";

    //   sgpr  src/gui/presets.cpp

    strings["Game consoles##sgpr"].plurals[0] = "Игровые консоли";
    strings["let's play some chiptune making games!##sgpr"].plurals[0] = "давайте поиграем в создание чиптюнов!";
    strings["Sega Genesis (extended channel 3)##sgpr"].plurals[0] = "Sega Genesis (расширенный 3-ий канал)";
    strings["Sega Genesis (DualPCM, extended channel 3)##sgpr"].plurals[0] = "Sega Genesis (DualPCM, расширенный 3-ий канал)";
    strings["Sega Genesis (with Sega CD)##sgpr"].plurals[0] = "Sega Genesis (с Sega CD)";
    strings["Sega Genesis (extended channel 3 with Sega CD)##sgpr"].plurals[0] = "Sega Genesis (расширенный 3-ий канал с Sega CD)";
    strings["Sega Genesis (CSM with Sega CD)##sgpr"].plurals[0] = "Sega Genesis (CSM с Sega CD)";
    strings["Sega Master System (with FM expansion)##sgpr"].plurals[0] = "Sega Master System (с FM-расширением)";
    strings["Sega Master System (with FM expansion in drums mode)##sgpr"].plurals[0] = "Sega Master System (с FM-расширением в режиме ударных)";
    strings["Game Boy Advance (no software mixing)##sgpr"].plurals[0] = "Game Boy Advance (без программного микширования)";
    strings["Game Boy Advance (with MinMod)##sgpr"].plurals[0] = "Game Boy Advance (с драйвером MinMod)";
    strings["Famicom with Konami VRC6##sgpr"].plurals[0] = "Famicom с Konami VRC6";
    strings["Famicom with Konami VRC7##sgpr0"].plurals[0] = "Famicom с Konami VRC7";
    strings["Famicom with MMC5##sgpr"].plurals[0] = "Famicom с MMC5";
    strings["Famicom with Sunsoft 5B##sgpr"].plurals[0] = "Famicom с Sunsoft 5B";
    strings["Famicom with Namco 163##sgpr"].plurals[0] = "Famicom с Namco 163";
    strings["Neo Geo AES (extended channel 2)##sgpr"].plurals[0] = "Neo Geo AES (расширенный 2-ой канал)";
    strings["Neo Geo AES (extended channel 2 and CSM)##sgpr"].plurals[0] = "Neo Geo AES (расширенный 2-ой канал и CSM)";
    strings["Computers##sgpr"].plurals[0] = "Компьютеры";
    strings["let's get to work on chiptune today.##sgpr"].plurals[0] = "давайте сегодня поработаем над чиптюнами.";
    strings["Commodore 64 (C64, 6581 SID + Sound Expander in drums mode)##sgpr"].plurals[0] = "Commodore 64 (C64, 6581 SID + Sound Expander в режиме ударных)";
    strings["Commodore 64 (C64, 8580 SID + Sound Expander in drums mode)##sgpr"].plurals[0] = "Commodore 64 (C64, 8580 SID + Sound Expander в режиме ударных)";
    strings["Commodore 64 (C64, 6581 SID + FM-YAM in drums mode)##sgpr"].plurals[0] = "Commodore 64 (C64, 6581 SID + FM-YAM в режиме ударных)";
    strings["Commodore 64 (C64, 8580 SID + FM-YAM in drums mode)##sgpr"].plurals[0] = "Commodore 64 (C64, 8580 SID + FM-YAM в режиме ударных)";
    strings["MSX + MSX-AUDIO (drums mode)##sgpr"].plurals[0] = "MSX + MSX-AUDIO (в режиме ударных)";
    strings["MSX + MSX-MUSIC (drums mode)##sgpr"].plurals[0] = "MSX + MSX-MUSIC (в режиме ударных)";
    strings["MSX + Neotron (extended channel 2)##sgpr"].plurals[0] = "MSX + Neotron (расширенный 2-ой канал)";
    strings["MSX + Neotron (extended channel 2 and CSM)##sgpr"].plurals[0] = "MSX + Neotron (расширенный 2-ой канал и CSM)";
    strings["MSX + Neotron (with YM2610B)##sgpr"].plurals[0] = "MSX + Neotron (с YM2610B)";
    strings["MSX + Neotron (with YM2610B; extended channel 3)##sgpr"].plurals[0] = "MSX + Neotron (с YM2610B; расширенный 3-ий канал 3)";
    strings["MSX + Neotron (with YM2610B; extended channel 3 and CSM)##sgpr"].plurals[0] = "MSX + Neotron (с YM2610B; расширенный 3-ий канал и CSM)";
    strings["NEC PC-88 (with PC-8801-10)##sgpr"].plurals[0] = "NEC PC-88 (с PC-8801-10)";
    strings["NEC PC-88 (with PC-8801-11)##sgpr"].plurals[0] = "NEC PC-88 (с PC-8801-11)";
    strings["NEC PC-88 (with PC-8801-11; extended channel 3)##sgpr"].plurals[0] = "NEC PC-88 (с PC-8801-11; расширенный 3-ий канал)";
    strings["NEC PC-88 (with PC-8801-11; extended channel 3 and CSM)##sgpr"].plurals[0] = "NEC PC-88 (с PC-8801-11; расширенный 3-ий канал и CSM)";
    strings["NEC PC-88 (with PC-8801-23)##sgpr"].plurals[0] = "NEC PC-88 (с PC-8801-23)";
    strings["NEC PC-88 (with PC-8801-23; extended channel 3)##sgpr"].plurals[0] = "NEC PC-88 (с PC-8801-23; расширенный 3-ий канал)";
    strings["NEC PC-88 (with PC-8801-23; extended channel 3 and CSM)##sgpr"].plurals[0] = "NEC PC-88 (с PC-8801-23; расширенный 3-ий канал и CSM)";
    strings["NEC PC-88 (with HMB-20 HIBIKI-8800)##sgpr"].plurals[0] = "NEC PC-88 (с HMB-20 HIBIKI-8800)";
    strings["NEC PC-8801mk2SR (with PC-8801-10)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (с PC-8801-10)";
    strings["NEC PC-8801mk2SR (with PC-8801-10; extended channel 3)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (с PC-8801-10; расширенный 3-ий канал)";
    strings["NEC PC-8801mk2SR (with PC-8801-10; extended channel 3 and CSM)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (с PC-8801-10; расширенный 3-ий канал и CSM)";
    strings["NEC PC-8801mk2SR (with PC-8801-11)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (с PC-8801-11)";
    strings["NEC PC-8801mk2SR (with PC-8801-11; extended channel 3 on internal OPN)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (с PC-8801-11; расширенный 3-ий канал на внутреннем чипе OPN)";
    strings["NEC PC-8801mk2SR (with PC-8801-11; extended channel 3 on external OPN)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (с PC-8801-11; расширенный 3-ий канал на внешнем чипе OPN)";
    strings["NEC PC-8801mk2SR (with PC-8801-11; extended channel 3 on both OPNs)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (с PC-8801-11; расширенный 3-ий канал на обоих чипах OPN)";
    strings["NEC PC-8801mk2SR (with PC-8801-11; extended channel 3 and CSM on both OPNs)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (с PC-8801-11; расширенный 3-ий канал и CSM на обоих чипах OPN)";
    strings["NEC PC-8801mk2SR (with PC-8801-23)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (с PC-8801-23)";
    strings["NEC PC-8801mk2SR (with PC-8801-23; extended channel 3 on internal OPN)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (с PC-8801-23; расширенный 3-ий канал на внутреннем чипе OPN)";
    strings["NEC PC-8801mk2SR (with PC-8801-23; extended channel 3 and CSM on internal OPN)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (с PC-8801-23; расширенный 3-ий канал и CSM на внутреннем чипе OPN)";
    strings["NEC PC-8801mk2SR (with PC-8801-23; extended channel 3 on external OPN)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (с PC-8801-23; расширенный 3-ий канал на внешнем чипе OPN)";
    strings["NEC PC-8801mk2SR (with PC-8801-23; extended channel 3 and CSM on external OPN)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (с PC-8801-23; расширенный 3-ий канал и CSM на внешнем чипе OPN)";
    strings["NEC PC-8801mk2SR (with PC-8801-23; extended channel 3 on both OPNs)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (с PC-8801-23; extended channel 3 on both OPNs)";
    strings["NEC PC-8801mk2SR (with PC-8801-23; extended channel 3 and CSM on both OPNs)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (с PC-8801-23; расширенный 3-ий канал и CSM на обоих чипах OPN)";
    strings["NEC PC-8801mk2SR (with HMB-20 HIBIKI-8800)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (с HMB-20 HIBIKI-8800)";
    strings["NEC PC-8801mk2SR (with HMB-20 HIBIKI-8800; extended channel 3)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (с HMB-20 HIBIKI-8800; расширенный 3-ий канал)";
    strings["NEC PC-8801mk2SR (with HMB-20 HIBIKI-8800; extended channel 3 and CSM)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (с HMB-20 HIBIKI-8800; расширенный 3-ий канал и CSM)";
    strings["NEC PC-8801FA (with PC-8801-10)##sgpr"].plurals[0] = "NEC PC-8801FA (с PC-8801-10)";
    strings["NEC PC-8801FA (with PC-8801-10; extended channel 3)##sgpr"].plurals[0] = "NEC PC-8801FA (с PC-8801-10; расширенный 3-ий канал)";
    strings["NEC PC-8801FA (with PC-8801-11)##sgpr"].plurals[0] = "NEC PC-8801FA (с PC-8801-11)";
    strings["NEC PC-8801FA (with PC-8801-11; extended channel 3 on internal OPN)##sgpr"].plurals[0] = "NEC PC-8801FA (с PC-8801-11; расширенный 3-ий канал на внутреннем чипе OPN)";
    strings["NEC PC-8801FA (with PC-8801-11; extended channel 3 on external OPN)##sgpr"].plurals[0] = "NEC PC-8801FA (с PC-8801-11; расширенный 3-ий канал на внешнем чипе OPN)";
    strings["NEC PC-8801FA (with PC-8801-11; extended channel 3 and CSM on external OPN)##sgpr"].plurals[0] = "NEC PC-8801FA (с PC-8801-11; расширенный 3-ий канал и CSM на внешнем чипе OPN)";
    strings["NEC PC-8801FA (with PC-8801-11; extended channel 3 on both OPNs)##sgpr"].plurals[0] = "NEC PC-8801FA (с PC-8801-11; расширенный 3-ий канал на обоих чипах OPN)";
    strings["NEC PC-8801FA (with PC-8801-11; extended channel 3 and CSM on both OPNs)##sgpr"].plurals[0] = "NEC PC-8801FA (с PC-8801-11; расширенный 3-ий канал и CSM на обоих чипах OPN)";
    strings["NEC PC-8801FA (with PC-8801-23)##sgpr"].plurals[0] = "NEC PC-8801FA (с PC-8801-23)";
    strings["NEC PC-8801FA (with PC-8801-23; extended channel 3 on internal OPN)##sgpr"].plurals[0] = "NEC PC-8801FA (с PC-8801-23; расширенный 3-ий канал на внутреннем чипе OPN)";
    strings["NEC PC-8801FA (with PC-8801-23; extended channel 3 and CSM on internal OPN)##sgpr"].plurals[0] = "NEC PC-8801FA (с PC-8801-23; расширенный 3-ий канал и CSM на внутреннем чипе OPN)";
    strings["NEC PC-8801FA (with PC-8801-23; extended channel 3 on external OPN)##sgpr"].plurals[0] = "NEC PC-8801FA (с PC-8801-23; расширенный 3-ий канал на внешнем чипе OPN)";
    strings["NEC PC-8801FA (with PC-8801-23; extended channel 3 and CSM on external OPN)##sgpr"].plurals[0] = "NEC PC-8801FA (с PC-8801-23; расширенный 3-ий канал и CSM на внешнем чипе OPN)";
    strings["NEC PC-8801FA (with PC-8801-23; extended channel 3 on both OPNs)##sgpr"].plurals[0] = "NEC PC-8801FA (с PC-8801-23; расширенный 3-ий канал на обоих чипах OPN)";
    strings["NEC PC-8801FA (with PC-8801-23; extended channel 3 and CSM on both OPNs)##sgpr"].plurals[0] = "NEC PC-8801FA (с PC-8801-23; расширенный 3-ий канал и CSM на обоих чипах OPN)";
    strings["NEC PC-8801FA (with HMB-20 HIBIKI-8800)##sgpr"].plurals[0] = "NEC PC-8801FA (с HMB-20 HIBIKI-8800)";
    strings["NEC PC-8801FA (with HMB-20 HIBIKI-8800; extended channel 3)##sgpr"].plurals[0] = "NEC PC-8801FA (с HMB-20 HIBIKI-8800; расширенный 3-ий канал)";
    strings["NEC PC-8801FA (with HMB-20 HIBIKI-8800; extended channel 3 and CSM)##sgpr"].plurals[0] = "NEC PC-8801FA (с HMB-20 HIBIKI-8800; расширенный 3-ий канал и CSM)";
    strings["NEC PC-98 (with PC-9801-26/K)##sgpr"].plurals[0] = "NEC PC-98 (с PC-9801-26/K)";
    strings["NEC PC-98 (with PC-9801-26/K; extended channel 3)##sgpr"].plurals[0] = "NEC PC-98 (с PC-9801-26/K; расширенный 3-ий канал)";
    strings["NEC PC-98 (with PC-9801-26/K; extended channel 3 and CSM)##sgpr"].plurals[0] = "NEC PC-98 (с PC-9801-26/K; расширенный 3-ий канал и CSM)";
    strings["NEC PC-98 (with Sound Orchestra)##sgpr"].plurals[0] = "NEC PC-98 (с Sound Orchestra)";
    strings["NEC PC-98 (with Sound Orchestra; extended channel 3)##sgpr"].plurals[0] = "NEC PC-98 (с Sound Orchestra; расширенный 3-ий канал)";
    strings["NEC PC-98 (with Sound Orchestra; extended channel 3 and CSM)##sgpr"].plurals[0] = "NEC PC-98 (с Sound Orchestra; расширенный 3-ий канал и CSM)";
    strings["NEC PC-98 (with Sound Orchestra in drums mode)##sgpr"].plurals[0] = "NEC PC-98 (с Sound Orchestra в режиме ударных)";
    strings["NEC PC-98 (with Sound Orchestra in drums mode; extended channel 3)##sgpr"].plurals[0] = "NEC PC-98 (с Sound Orchestra в режиме ударных; расширенный 3-ий канал)";
    strings["NEC PC-98 (with Sound Orchestra in drums mode; extended channel 3 and CSM)##sgpr"].plurals[0] = "NEC PC-98 (с Sound Orchestra в режиме ударных; расширенный 3-ий канал и CSM)";
    strings["NEC PC-98 (with Sound Orchestra V)##sgpr"].plurals[0] = "NEC PC-98 (с Sound Orchestra V)";
    strings["NEC PC-98 (with Sound Orchestra V; extended channel 3)##sgpr"].plurals[0] = "NEC PC-98 (с Sound Orchestra V; расширенный 3-ий канал)";
    strings["NEC PC-98 (with Sound Orchestra V; extended channel 3 and CSM)##sgpr"].plurals[0] = "NEC PC-98 (с Sound Orchestra V; расширенный 3-ий канал и CSM)";
    strings["NEC PC-98 (with Sound Orchestra V in drums mode)##sgpr"].plurals[0] = "NEC PC-98 (с Sound Orchestra V в режиме ударных)";
    strings["NEC PC-98 (with Sound Orchestra V in drums mode; extended channel 3)##sgpr"].plurals[0] = "NEC PC-98 (с Sound Orchestra V в режиме ударных; расширенный 3-ий канал)";
    strings["NEC PC-98 (with Sound Orchestra V in drums mode; extended channel 3 and CSM)##sgpr"].plurals[0] = "NEC PC-98 (с Sound Orchestra V в режиме ударных; расширенный 3-ий канал и CSM)";
    strings["NEC PC-98 (with PC-9801-86)##sgpr"].plurals[0] = "NEC PC-98 (с PC-9801-86)";
    strings["NEC PC-98 (with PC-9801-86; extended channel 3)##sgpr"].plurals[0] = "NEC PC-98 (с PC-9801-86; расширенный 3-ий канал)";
    strings["NEC PC-98 (with PC-9801-86; extended channel 3 and CSM)##sgpr"].plurals[0] = "NEC PC-98 (с PC-9801-86; расширенный 3-ий канал и CSM)";
    strings["NEC PC-98 (with PC-9801-86) stereo##sgpr"].plurals[0] = "NEC PC-98 (с PC-9801-86) стерео";
    strings["NEC PC-98 (with PC-9801-86; extended channel 3) stereo##sgpr"].plurals[0] = "NEC PC-98 (с PC-9801-86; расширенный 3-ий канал) стерео";
    strings["NEC PC-98 (with PC-9801-86; extended channel 3 and CSM) stereo##sgpr"].plurals[0] = "NEC PC-98 (с PC-9801-86; расширенный 3-ий канал и CSM) стерео";
    strings["NEC PC-98 (with PC-9801-73)##sgpr"].plurals[0] = "NEC PC-98 (с PC-9801-73)";
    strings["NEC PC-98 (with PC-9801-73; extended channel 3)##sgpr"].plurals[0] = "NEC PC-98 (с PC-9801-73; расширенный 3-ий канал)";
    strings["NEC PC-98 (with PC-9801-73; extended channel 3 and CSM)##sgpr"].plurals[0] = "NEC PC-98 (с PC-9801-73; расширенный 3-ий канал и CSM)";
    strings["NEC PC-98 (with Sound Blaster 16 for PC-9800 w/PC-9801-26/K compatible)##sgpr"].plurals[0] = "NEC PC-98 (с Sound Blaster 16 для PC-9800, совместимый с PC-9801-26/K)";
    strings["NEC PC-98 (with Sound Blaster 16 for PC-9800 w/PC-9801-26/K compatible; extended channel 3)##sgpr"].plurals[0] = "NEC PC-98 (с Sound Blaster 16 для PC-9800, совместимый с PC-9801-26/K; расширенный 3-ий канал)";
    strings["NEC PC-98 (with Sound Blaster 16 for PC-9800 w/PC-9801-26/K compatible; extended channel 3 and CSM)##sgpr"].plurals[0] = "NEC PC-98 (с Sound Blaster 16 для PC-9800, совместимый с PC-9801-26/K; расширенный 3-ий канал и CSM)";
    strings["NEC PC-98 (with Sound Blaster 16 for PC-9800 w/PC-9801-26/K compatible in drums mode)##sgpr"].plurals[0] = "NEC PC-98 (с Sound Blaster 16 для PC-9800, совместимый с PC-9801-26/K в режиме ударных)";
    strings["NEC PC-98 (with Sound Blaster 16 for PC-9800 w/PC-9801-26/K compatible in drums mode; extended channel 3)##sgpr"].plurals[0] = "NEC PC-98 (с Sound Blaster 16 для PC-9800, совместимый с PC-9801-26/K в режиме ударных; расширенный 3-ий канал)";
    strings["NEC PC-98 (with Sound Blaster 16 for PC-9800 w/PC-9801-26/K compatible in drums mode; extended channel 3 and CSM)##sgpr"].plurals[0] = "NEC PC-98 (с Sound Blaster 16 для PC-9800, совместимый с PC-9801-26/K в режиме ударных; расширенный 3-ий канал и CSM)";
    strings["ZX Spectrum (48K, SFX-like engine)##sgpr"].plurals[0] = "ZX Spectrum (48K, драйвер типа SFX)";
    strings["ZX Spectrum (48K, QuadTone engine)##sgpr"].plurals[0] = "ZX Spectrum (48K, драйвер QuadTone)";
    strings["ZX Spectrum (128K) with TurboSound##sgpr"].plurals[0] = "ZX Spectrum (128K) с TurboSound";
    strings["ZX Spectrum (128K) with TurboSound FM##sgpr"].plurals[0] = "ZX Spectrum (128K) с TurboSound FM";
    strings["ZX Spectrum (128K) with TurboSound FM (extended channel 3 on first OPN)##sgpr"].plurals[0] = "ZX Spectrum (128K) с TurboSound FM (расширенный 3-ий канал на первом чипе OPN)";
    strings["ZX Spectrum (128K) with TurboSound FM (extended channel 3 and CSM on first OPN)##sgpr"].plurals[0] = "ZX Spectrum (128K) с TurboSound FM (расширенный 3-ий канал и CSM на первом чипе OPN)";
    strings["ZX Spectrum (128K) with TurboSound FM (extended channel 3 on second OPN)##sgpr"].plurals[0] = "ZX Spectrum (128K) с TurboSound FM (расширенный 3-ий канал на втором чипе OPN)";
    strings["ZX Spectrum (128K) with TurboSound FM (extended channel 3 and CSM on second OPN)##sgpr"].plurals[0] = "ZX Spectrum (128K) с TurboSound FM (расширенный 3-ий канал и CSM на втором чипе OPN)";
    strings["ZX Spectrum (128K) with TurboSound FM (extended channel 3 on both OPNs)##sgpr"].plurals[0] = "ZX Spectrum (128K) с TurboSound FM (расширенный 3-ий канал на обоих чипах OPN)";
    strings["ZX Spectrum (128K) with TurboSound FM (extended channel 3 and CSM on both OPNs)##sgpr"].plurals[0] = "ZX Spectrum (128K) с TurboSound FM (расширенный 3-ий канал и CSM на обоих чипах OPN)";
    strings["Atari 800 (stereo)##sgpr"].plurals[0] = "Atari 800 (стерео)";
    strings["PC (beeper)##sgpr"].plurals[0] = "PC (пищалка)";
    strings["PC + AdLib (drums mode)##sgpr"].plurals[0] = "PC + AdLib (в режиме ударных)";
    strings["PC + Sound Blaster (drums mode)##sgpr"].plurals[0] = "PC + Sound Blaster (в режиме ударных)";
    strings["PC + Sound Blaster w/Game Blaster Compatible##sgpr"].plurals[0] = "PC + Sound Blaster совм. с Game Blaster";
    strings["PC + Sound Blaster w/Game Blaster Compatible (drums mode)##sgpr"].plurals[0] = "PC + Sound Blaster совм. с Game Blaster (в режиме ударных)";
    strings["PC + Sound Blaster Pro (drums mode)##sgpr"].plurals[0] = "PC + Sound Blaster Pro (в режиме ударных)";
    strings["PC + Sound Blaster Pro 2 (drums mode)##sgpr"].plurals[0] = "PC + Sound Blaster Pro 2 (в режиме ударных)";
    strings["PC + ESS AudioDrive ES1488 (native ESFM mode)##sgpr"].plurals[0] = "PC + ESS AudioDrive ES1488 (нативный режим ESFM)";
    strings["Sharp X1 + FM addon##sgpr"].plurals[0] = "Sharp X1 + FM-дополнение";
    strings["FM Towns (extended channel 3)##sgpr"].plurals[0] = "FM Towns (расширенный 3-ий канал)";
    strings["Commander X16 (VERA only)##sgpr0"].plurals[0] = "Commander X16 (только VERA)";
    strings["Commander X16 (with OPM)##sgpr"].plurals[0] = "Commander X16 (с OPM)";
    strings["Commander X16 (with Twin OPL3)##sgpr"].plurals[0] = "Commander X16 (с Twin OPL3)";
    strings["Arcade systems##sgpr"].plurals[0] = "Аркадные автоматы";
    strings["INSERT COIN##sgpr"].plurals[0] = "ВСТАВЬТЕ ЖЕТОН##sgpr";
    strings["Williams/Midway Y/T unit w/ADPCM sound board##sgpr"].plurals[0] = "Автомат Williams/Midway Y/T со звуковой картой АДИКМ";
    strings["Konami Battlantis (drums mode on first OPL2)##sgpr"].plurals[0] = "Konami Battlantis (первый OPL2 в режиме ударных)";
    strings["Konami Battlantis (drums mode on second OPL2)##sgpr"].plurals[0] = "Konami Battlantis (второй OPL2 в режиме ударных)";
    strings["Konami Battlantis (drums mode on both OPL2s)##sgpr"].plurals[0] = "Konami Battlantis (оба OPL2 в режиме ударных)";
    strings["Konami Haunted Castle (drums mode)##sgpr"].plurals[0] = "Konami Haunted Castle (в режиме ударных)";
    strings["Konami S.P.Y. (drums mode)##sgpr"].plurals[0] = "Konami S.P.Y. (в режиме ударных)";
    strings["Konami Rollergames (drums mode)##sgpr"].plurals[0] = "Konami Rollergames (в режиме ударных)";
    strings["Sega System E (with FM expansion)##sgpr"].plurals[0] = "Sega System E (с FM-расширением)";
    strings["Sega System E (with FM expansion in drums mode)##sgpr"].plurals[0] = "Sega System E (с FM-расширением в режиме ударных)";
    strings["Sega Hang-On (extended channel 3)##sgpr"].plurals[0] = "Sega Hang-On (расширенный 3-ий канал)";
    strings["Sega Hang-On (extended channel 3 and CSM)##sgpr"].plurals[0] = "Sega Hang-On (расширенный 3-ий канал и CSM)";
    strings["Sega System 18 (extended channel 3 on first OPN2C)##sgpr"].plurals[0] = "Sega System 18 (расширенный 3-ий канал на первом OPN2C)";
    strings["Sega System 18 (extended channel 3 and CSM on first OPN2C)##sgpr"].plurals[0] = "Sega System 18 (расширенный 3-ий канал и CSM на первом OPN2C)";
    strings["Sega System 18 (extended channel 3 on second OPN2C)##sgpr"].plurals[0] = "Sega System 18 (расширенный 3-ий канал на втором OPN2C)";
    strings["Sega System 18 (extended channel 3 and CSM on second OPN2C)##sgpr"].plurals[0] = "Sega System 18 (расширенный 3-ий канал и CSM на втором OPN2C)";
    strings["Sega System 18 (extended channel 3 on both OPN2Cs)##sgpr"].plurals[0] = "Sega System 18 (расширенный 3-ий канал на обоих чипах OPN2C)";
    strings["Sega System 18 (extended channel 3 and CSM on both OPN2Cs)##sgpr"].plurals[0] = "Sega System 18 (расширенный 3-ий канал и CSM на обоих чипах OPN2C)";
    strings["Sega System 32 (extended channel 3 on first OPN2C)##sgpr"].plurals[0] = "Sega System 32 (расширенный 3-ий канал на первом OPN2C)";
    strings["Sega System 32 (extended channel 3 and CSM on first OPN2C)##sgpr"].plurals[0] = "Sega System 32 (расширенный 3-ий канал и CSM на первом OPN2C)";
    strings["Sega System 32 (extended channel 3 on second OPN2C)##sgpr"].plurals[0] = "Sega System 32 (расширенный 3-ий канал на втором OPN2C)";
    strings["Sega System 32 (extended channel 3 and CSM on second OPN2C)##sgpr"].plurals[0] = "Sega System 32 (расширенный 3-ий канал и CSM на втором OPN2C)";
    strings["Sega System 32 (extended channel 3 on both OPN2Cs)##sgpr"].plurals[0] = "Sega System 32 (расширенный 3-ий канал на обоих чипах OPN2C)";
    strings["Sega System 32 (extended channel 3 and CSM on both OPN2Cs)##sgpr"].plurals[0] = "Sega System 32 (расширенный 3-ий канал и CSM на обоих чипах OPN2C)";
    strings["Capcom Arcade##sgpr"].plurals[0] = "Capcom Arcade";
    strings["Capcom Arcade (extended channel 3 on first OPN)##sgpr"].plurals[0] = "Capcom Arcade (расширенный 3-ий канал на первом OPN)";
    strings["Capcom Arcade (extended channel 3 and CSM on first OPN)##sgpr"].plurals[0] = "Capcom Arcade (расширенный 3-ий канал на втором OPN)";
    strings["Capcom Arcade (extended channel 3 on second OPN)##sgpr"].plurals[0] = "Capcom Arcade (расширенный 3-ий канал и CSM на втором OPN)";
    strings["Capcom Arcade (extended channel 3 and CSM on second OPN)##sgpr"].plurals[0] = "Capcom Arcade (расширенный 3-ий канал и CSM на втором OPN)";
    strings["Capcom Arcade (extended channel 3 on both OPNs)##sgpr"].plurals[0] = "Capcom Arcade (расширенный 3-ий канал на обоих чипах OPN)";
    strings["Capcom Arcade (extended channel 3 and CSM on both OPNs)##sgpr"].plurals[0] = "Capcom Arcade (расширенный 3-ий канал и CSM на обоих чипах OPN)";
    strings["Jaleco Ginga NinkyouDen (drums mode)##sgpr"].plurals[0] = "Jaleco Ginga NinkyouDen (в режиме ударных)";
    strings["NMK 16-bit Arcade##sgpr"].plurals[0] = "NMK 16-bit Arcade";
    strings["NMK 16-bit Arcade (extended channel 3)##sgpr"].plurals[0] = "NMK 16-bit Arcade (расширенный 3-ий канал)";
    strings["NMK 16-bit Arcade (extended channel 3 and CSM)##sgpr"].plurals[0] = "NMK 16-bit Arcade (расширенный 3-ий канал и CSM)";
    strings["NMK 16-bit Arcade (w/NMK112 bankswitching)##sgpr"].plurals[0] = "NMK 16-bit Arcade (со сменой банков NMK112)";
    strings["NMK 16-bit Arcade (w/NMK112 bankswitching, extended channel 3)##sgpr"].plurals[0] = "NMK 16-bit Arcade (со сменой банков NMK112, расширенный 3-ий канал)";
    strings["NMK 16-bit Arcade (w/NMK112 bankswitching, extended channel 3 and CSM)##sgpr"].plurals[0] = "NMK 16-bit Arcade (со сменой банков NMK112, расширенный 3-ий канал и CSM)";
    strings["Atlus Power Instinct 2 (extended channel 3)##sgpr"].plurals[0] = "Atlus Power Instinct 2 (расширенный 3-ий канал)";
    strings["Atlus Power Instinct 2 (extended channel 3 and CSM)##sgpr"].plurals[0] = "Atlus Power Instinct 2 (расширенный 3-ий канал и CSM)";
    strings["Kaneko DJ Boy (extended channel 3)##sgpr"].plurals[0] = "Kaneko DJ Boy (расширенный 3-ий канал)";
    strings["Kaneko DJ Boy (extended channel 3 and CSM)##sgpr"].plurals[0] = "Kaneko DJ Boy (расширенный 3-ий канал и CSM)";
    strings["Kaneko Air Buster (extended channel 3)##sgpr"].plurals[0] = "Kaneko Air Buster (расширенный 3-ий канал)";
    strings["Kaneko Air Buster (extended channel 3 and CSM)##sgpr"].plurals[0] = "Kaneko Air Buster (расширенный 3-ий канал и CSM)";
    strings["Tecmo Ninja Gaiden (extended channel 3 on first OPN)##sgpr"].plurals[0] = "Tecmo Ninja Gaiden (расширенный 3-ий канал на первом OPN)";
    strings["Tecmo Ninja Gaiden (extended channel 3 and CSM on first OPN)##sgpr"].plurals[0] = "Tecmo Ninja Gaiden (расширенный 3-ий канал и CSM на первом OPN)";
    strings["Tecmo Ninja Gaiden (extended channel 3 on second OPN)##sgpr"].plurals[0] = "Tecmo Ninja Gaiden (расширенный 3-ий канал на втором OPN)";
    strings["Tecmo Ninja Gaiden (extended channel 3 and CSM on second OPN)##sgpr"].plurals[0] = "Tecmo Ninja Gaiden (расширенный 3-ий канал и CSM на втором OPN)";
    strings["Tecmo Ninja Gaiden (extended channel 3 on both OPNs)##sgpr"].plurals[0] = "Tecmo Ninja Gaiden (расширенный 3-ий канал на обоих чипах OPN)";
    strings["Tecmo Ninja Gaiden (extended channel 3 and CSM on both OPNs)##sgpr"].plurals[0] = "Tecmo Ninja Gaiden (расширенный 3-ий канал и CSM на обоих чипах OPN)";
    strings["Tecmo System (drums mode)##sgpr"].plurals[0] = "Tecmo System (в режиме ударных)";
    strings["Seibu Kaihatsu Raiden (drums mode)##sgpr"].plurals[0] = "Seibu Kaihatsu Raiden (в режиме ударных)";
    strings["Sunsoft Arcade##sgpr"].plurals[0] = "Sunsoft Arcade";
    strings["Sunsoft Arcade (extended channel 3)##sgpr"].plurals[0] = "Sunsoft Arcade (расширенный 3-ий канал)";
    strings["Sunsoft Arcade (extended channel 3 and CSM)##sgpr"].plurals[0] = "Sunsoft Arcade (расширенный 3-ий канал и CSM)";
    strings["Atari Rampart (drums mode)##sgpr"].plurals[0] = "Atari Rampart (в режиме ударных)";
    strings["Data East Karnov (extended channel 3)##sgpr"].plurals[0] = "Data East Karnov (расширенный 3-ий канал 3)";
    strings["Data East Karnov (extended channel 3 and CSM)##sgpr"].plurals[0] = "Data East Karnov (расширенный 3-ий канал и CSM)";
    strings["Data East Karnov (drums mode)##sgpr"].plurals[0] = "Data East Karnov (в режиме ударных)";
    strings["Data East Karnov (extended channel 3; drums mode)##sgpr"].plurals[0] = "Data East Karnov (расширенный 3-ий канал; в режиме ударных)";
    strings["Data East Karnov (extended channel 3 and CSM; drums mode)##sgpr"].plurals[0] = "Data East Karnov (расширенный 3-ий канал и CSM; в режиме ударных)";
    strings["Data East Arcade##sgpr"].plurals[0] = "Data East Arcade";
    strings["Data East Arcade (extended channel 3)##sgpr"].plurals[0] = "Data East Arcade (расширенный 3-ий канал)";
    strings["Data East Arcade (extended channel 3 and CSM)##sgpr"].plurals[0] = "Data East Arcade (расширенный 3-ий канал и CSM)";
    strings["Data East Arcade (drums mode)##sgpr"].plurals[0] = "Data East Arcade (в режиме ударных)";
    strings["Data East Arcade (extended channel 3; drums mode)##sgpr"].plurals[0] = "Data East Arcade (расширенный 3-ий канал; в режиме ударных)";
    strings["Data East Arcade (extended channel 3 and CSM; drums mode)##sgpr"].plurals[0] = "Data East Arcade (расширенный 3-ий канал и CSM; в режиме ударных)";
    strings["Data East PCX (extended channel 3)##sgpr"].plurals[0] = "Data East PCX (расширенный 3-ий канал)";
    strings["Data East PCX (extended channel 3 and CSM)##sgpr"].plurals[0] = "Data East PCX (расширенный 3-ий канал и CSM)";
    strings["Data East Dark Seal (extended channel 3)##sgpr"].plurals[0] = "Data East Dark Seal (расширенный 3-ий канал)";
    strings["Data East Dark Seal (extended channel 3 and CSM)##sgpr"].plurals[0] = "Data East Dark Seal (расширенный 3-ий канал и CSM)";
    strings["SNK Ikari Warriors (drums mode on first OPL)##sgpr"].plurals[0] = "SNK Ikari Warriors (первый OPL в режиме ударных)";
    strings["SNK Ikari Warriors (drums mode on second OPL)##sgpr"].plurals[0] = "SNK Ikari Warriors (второй OPL в режиме ударных)";
    strings["SNK Ikari Warriors (drums mode on both OPLs)##sgpr"].plurals[0] = "SNK Ikari Warriors (оба чипа OPL в режиме ударных)";
    strings["SNK Triple Z80 (drums mode on Y8950)##sgpr"].plurals[0] = "SNK Triple Z80 (Y8950 в режиме ударных)";
    strings["SNK Triple Z80 (drums mode on OPL)##sgpr"].plurals[0] = "SNK Triple Z80 (OPL в режиме ударных)";
    strings["SNK Triple Z80 (drums mode on Y8950 and OPL)##sgpr"].plurals[0] = "SNK Triple Z80 (Y8950 и OPL в режиме ударных)";
    strings["SNK Chopper I (drums mode on Y8950)##sgpr"].plurals[0] = "SNK Chopper I (Y8950 в режиме ударных)";
    strings["SNK Chopper I (drums mode on OPL2)##sgpr"].plurals[0] = "SNK Chopper I (OPL2 в режиме ударных)";
    strings["SNK Chopper I (drums mode on Y8950 and OPL2)##sgpr"].plurals[0] = "SNK Chopper I (Y8950 и OPL2 в режиме ударных)";
    strings["SNK Touchdown Fever (drums mode on OPL)##sgpr"].plurals[0] = "SNK Touchdown Fever (OPL в режиме ударных)";
    strings["SNK Touchdown Fever (drums mode on Y8950)##sgpr"].plurals[0] = "SNK Touchdown Fever (Y8950 в режиме ударных)";
    strings["SNK Touchdown Fever (drums mode on OPL and Y8950)##sgpr"].plurals[0] = "SNK Touchdown Fever (Y8950 и OPL2 в режиме ударных)";
    strings["Alpha denshi Alpha-68K (extended channel 3)##sgpr"].plurals[0] = "Alpha denshi Alpha-68K (расширенный 3-ий канал)";
    strings["Alpha denshi Alpha-68K (extended channel 3 and CSM)##sgpr"].plurals[0] = "Alpha denshi Alpha-68K (расширенный 3-ий канал и CSM)";
    strings["Alpha denshi Alpha-68K (drums mode)##sgpr"].plurals[0] = "Alpha denshi Alpha-68K (в режиме ударных)";
    strings["Alpha denshi Alpha-68K (extended channel 3; drums mode)##sgpr"].plurals[0] = "Alpha denshi Alpha-68K (расширенный 3-ий канал; в режиме ударных)";
    strings["Alpha denshi Alpha-68K (extended channel 3 and CSM; drums mode)##sgpr"].plurals[0] = "Alpha denshi Alpha-68K (расширенный 3-ий канал и CSM; в режиме ударных)";
    strings["Neo Geo MVS (extended channel 2)##sgpr"].plurals[0] = "Neo Geo MVS (расширенный 2-ой канал)";
    strings["Neo Geo MVS (extended channel 2 and CSM)##sgpr"].plurals[0] = "Neo Geo MVS (расширенный 2-ой канал и CSM)";
    strings["Namco (3-channel WSG)##sgpr"].plurals[0] = "Namco (3-канальный генератор звука на волновых таблицах)";
    strings["Taito Arcade##sgpr"].plurals[0] = "Taito Arcade";
    strings["Taito Arcade (extended channel 3)##sgpr"].plurals[0] = "Taito Arcade (расширенный 3-ий канал)";
    strings["Taito Arcade (extended channel 3 and CSM)##sgpr"].plurals[0] = "Taito Arcade (расширенный 3-ий канал и CSM)";
    strings["Seta 1 + FM addon##sgpr"].plurals[0] = "Seta 1 + FM-дополнение";
    strings["Seta 1 + FM addon (extended channel 3)##sgpr"].plurals[0] = "Seta 1 + FM-дополнение (расширенный 3-ий канал)";
    strings["Seta 1 + FM addon (extended channel 3 and CSM)##sgpr"].plurals[0] = "Seta 1 + FM-дополнение (расширенный 3-ий канал и CSM)";
    strings["Coreland Cyber Tank (drums mode)##sgpr"].plurals[0] = "Coreland Cyber Tank (в режиме ударных)";
    strings["Toaplan 1 (drums mode)##sgpr"].plurals[0] = "Toaplan 1 (в режиме ударных)";
    strings["Dynax/Nakanihon 3rd generation hardware##sgpr"].plurals[0] = "Аппаратная платформа 3-го поколения Dynax/Nakanihon";
    strings["Dynax/Nakanihon 3rd generation hardware (drums mode)##sgpr"].plurals[0] = "Аппаратная платформа 3-го поколения Dynax/Nakanihon (в режиме ударных)";
    strings["Dynax/Nakanihon Real Break (drums mode)##sgpr"].plurals[0] = "Dynax/Nakanihon Real Break (в режиме ударных)";
    strings["User##sgpr"].plurals[0] = "Пользовательские";
    strings["system presets that you have saved.##sgpr"].plurals[0] = "пресеты систем, которые вы сохранили.";
    strings["chips which use frequency modulation (FM) to generate sound.\nsome of these also pack more (like square and sample channels).\nActually \"FM\" here stands for phase modulation,\nbut these two are indistinguishable\nif you use sine waves.##sgpr"].plurals[0] = "чипы, использующую частотную модуляцию (FM-синтез) для генерации звука.\nв некоторых из них присутствуют и другие способы синтеза звука (каналы с квадратными волнами или сэмплами).\nНа самом деле, \"FM\" здесь означает фазовую модуляцию,\nно они неотличимы друг от друга при использовании синусоидальных волн.";
    strings["Yamaha YM2203 (extended channel 3)##sgpr"].plurals[0] = "Yamaha YM2203 (расширенный 3-ий канал)";
    strings["Yamaha YM2203 (extended channel 3 and CSM)##sgpr"].plurals[0] = "Yamaha YM2203 (расширенный 3-ий канал и CSM)";
    strings["Yamaha YM2608 (extended channel 3)##sgpr"].plurals[0] = "Yamaha YM2608 (расширенный 3-ий канал)";
    strings["Yamaha YM2608 (extended channel 3 and CSM)##sgpr"].plurals[0] = "Yamaha YM2608 (расширенный 3-ий канал и CSM)";
    strings["Yamaha YM2610 (extended channel 2)##sgpr"].plurals[0] = "Yamaha YM2610 (расширенный 2-ой канал)";
    strings["Yamaha YM2610 (extended channel 2 and CSM)##sgpr"].plurals[0] = "Yamaha YM2610 (расширенный 2-ой канал и CSM)";
    strings["Yamaha YM2610B (extended channel 3)##sgpr"].plurals[0] = "Yamaha YM2610B (расширенный 3-ий канал)";
    strings["Yamaha YM2610B (extended channel 3 and CSM)##sgpr"].plurals[0] = "Yamaha YM2610B (расширенный 3-ий канал и CSM)";
    strings["Yamaha YM2612 (extended channel 3)##sgpr"].plurals[0] = "Yamaha YM2612 (расширенный 3-ий канал)";
    strings["Yamaha YM2612 (OPN2) with DualPCM##sgpr"].plurals[0] = "Yamaha YM2612 (OPN2) с DualPCM";
    strings["Yamaha YM2612 (extended channel 3) with DualPCM and CSM##sgpr"].plurals[0] = "Yamaha YM2612 (расширенный 3-ий канал) с DualPCM и CSM";
    strings["Yamaha YMF276 (extended channel 3)##sgpr"].plurals[0] = "Yamaha YMF276 (расширенный 3-ий канал)";
    strings["Yamaha YMF276 with DualPCM##sgpr"].plurals[0] = "Yamaha YMF276 с DualPCM";
    strings["Yamaha YMF276 (extended channel 3) with DualPCM and CSM##sgpr"].plurals[0] = "Yamaha YMF276 (расширенный 3-ий канал) с DualPCM и CSM";
    strings["Yamaha YM2413 (drums mode)##sgpr"].plurals[0] = "Yamaha YM2413 (в режиме ударных)";
    strings["Yamaha YM3438 (extended channel 3)##sgpr"].plurals[0] = "Yamaha YM3438 (расширенный 3-ий канал)";
    strings["Yamaha YM3438 (OPN2C) with DualPCM##sgpr"].plurals[0] = "Yamaha YM3438 (OPN2C) с DualPCM";
    strings["Yamaha YM3438 (extended channel 3) with DualPCM and CSM##sgpr"].plurals[0] = "Yamaha YM3438 (расширенный 3-ий канал) с DualPCM и CSM";
    strings["Yamaha YM3526 (drums mode)##sgpr"].plurals[0] = "Yamaha YM3526 (в режиме ударных)";
    strings["Yamaha Y8950 (drums mode)##sgpr"].plurals[0] = "Yamaha Y8950 (в режиме ударных)";
    strings["Yamaha YM3812 (drums mode)##sgpr"].plurals[0] = "Yamaha YM3812 (в режиме ударных)";
    strings["Yamaha YMF262 (drums mode)##sgpr"].plurals[0] = "Yamaha YMF262 (в режиме ударных)";
    strings["Yamaha YMF289B (drums mode)##sgpr"].plurals[0] = "Yamaha YMF289B (в режиме ударных)";
    strings["ESS ES1xxx series (ESFM)##sgpr"].plurals[0] = "ESS серия ES1xxx (ESFM)";
    strings["Square##sgpr"].plurals[0] = "Меандр";
    strings["these chips generate square/pulse tones only (but may include noise).##sgpr"].plurals[0] = "эти чипы способны генерировать лишь квадратные/прямоугольные волны (но могут также генерировать шум).";
    strings["Tandy PSSJ 3-voice sound##sgpr"].plurals[0] = "Tandy PSSJ трёхканальный генератор звука";
    strings["Sega PSG (SN76489-like)##sgpr"].plurals[0] = "Sega PSG (похож на SN76489)";
    strings["Sega PSG (SN76489-like, Stereo)##sgpr"].plurals[0] = "Sega PSG (похож на SN76489, стерео)";
    strings["PC Speaker##sgpr"].plurals[0] = "PC Speaker (пищалка)";
    strings["Sample##sgpr"].plurals[0] = "Сэмплеры";
    strings["chips/systems which use PCM or ADPCM samples for sound synthesis.##sgpr"].plurals[0] = "чипы/системы, которые используют ИКМ- или АДИКМ-сэмплы для воспроизведения звука.";
    strings["Generic PCM DAC##sgpr"].plurals[0] = "Типичный ИКМ ЦАП";
    strings["Wavetable##sgpr"].plurals[0] = "Волн. табл.";
    strings["chips which use user-specified waveforms to generate sound.##sgpr"].plurals[0] = "чипы, которые использует задаваемые пользователем волны для синтеза звука.";
    strings["Namco C15 (8-channel mono)##sgpr"].plurals[0] = "Namco C15 (8-канальный, моно)";
    strings["Namco C30 (8-channel stereo)##sgpr"].plurals[0] = "Namco C30 (8-канальный, стерео)";
    strings["Famicom Disk System (chip)##sgpr"].plurals[0] = "Famicom Disk System (чип)";
    strings["Specialized##sgpr"].plurals[0] = "Особые";
    strings["chips/systems with unique sound synthesis methods.##sgpr"].plurals[0] = "чипы/системы с уникальными методами синтеза звука.";
    strings["Commodore PET (pseudo-wavetable)##sgpr"].plurals[0] = "Commodore PET (псевдо-волновая таблица)";
    strings["ZX Spectrum (beeper only, SFX-like engine)##sgpr"].plurals[0] = "ZX Spectrum (только пищалка, драйвер типа SFX)";
    strings["ZX Spectrum (beeper only, QuadTone engine)##sgpr"].plurals[0] = "ZX Spectrum (только пищалка, драйвер QuadTone)";
    strings["Modern/fantasy##sgpr"].plurals[0] = "Совр./вымышл.";
    strings["chips/systems which do not exist in reality or were made just several years ago.##sgpr"].plurals[0] = "чипы/системы, не имеющие физического воплощения или сделанные всего несколько лет назад.";
    strings["Commander X16 (VERA only)##sgpr1"].plurals[0] = "Commander X16 (только VERA)";
    strings["Flizzer Tracker (FZT) sound source##sgpr"].plurals[0] = "Источник звука Flizzer Tracker (FZT)";
    strings["DefleMask-compatible##sgpr"].plurals[0] = "Совместимые с DefleMask";
    strings["these configurations are compatible with DefleMask.\nselect this if you need to save as .dmf or work with that program.##sgpr"].plurals[0] = "эти пресеты совместимы с DefleMask.\nвыбирайте их, если вам будет нужно будет экспортировать модуль в .dmf или работать с этой программой.";
    strings["Sega Genesis (extended channel 3)##sgpr1"].plurals[0] = "Sega Genesis (расширенный 3-ий канал)";
    strings["Sega Master System (with FM expansion)##sgpr1"].plurals[0] = "Sega Master System (с FM-расширением)";
    strings["Famicom with Konami VRC7##sgpr1"].plurals[0] = "Famicom с Konami VRC7";
    strings["Arcade (YM2151 and SegaPCM)##sgpr1"].plurals[0] = "Arcade (YM2151 и SegaPCM)";
    strings["Neo Geo CD (extended channel 2)##sgpr1"].plurals[0] = "Neo Geo CD (расширенный 2-ой канал)";

    strings["User Systems##sgpr"].plurals[0] = "Пользовательские системы";
    strings["Error! User category does not exist!##sgpr"].plurals[0] = "Ошибка! Категория пользовательских пресетов не существует!";
    strings["Systems##sgpr"].plurals[0] = "Системы";
    strings["New Preset##sgpr"].plurals[0] = "Новый пресет";
    strings["select a preset##sgpr"].plurals[0] = "выберите пресет";
    strings["Name##sgpr"].plurals[0] = "Название";
    strings["Remove##UPresetRemove"].plurals[0] = "Убрать##UPresetRemove";
    strings["Invert##sgpr"].plurals[0] = "Инв.";
    strings["Volume##sgpr"].plurals[0] = "Громкость";
    strings["Panning##sgpr"].plurals[0] = "Панорамирование";
    strings["Front/Rear##sgpr"].plurals[0] = "Передн./задн.";
    strings["Configure##sgpr"].plurals[0] = "Настроить";
    strings["Advanced##sgpr"].plurals[0] = "Дополнительно";
    strings["insert additional settings in `option=value` format.\n"
            "available options:\n"
            "- tickRate##sgpr"].plurals[0] = 
            
            "вставьте дополнительные настройки в формате `параметр=значение`.\n"
            "доступные параметры:\n"
            "- tickRate (частота движка трекера)";
    strings["Save and Close##sgpr"].plurals[0] = "Сохранить и закрыть";
    strings["Import##sgpr"].plurals[0] = "Импорт";
    strings["Import (replace)##sgpr"].plurals[0] = "Импорт (с заменой)";
    strings["Export##sgpr"].plurals[0] = "Экспорт";

    //   sgrv  src/gui/regView.cpp

    strings["Register View###Register View"].plurals[0] = "Регистры###Register View";
    strings["- no register pool available##sgrv"].plurals[0] = "- список регистров недоступен";

    //  sgsed  src/gui/sampleEdit.cpp

    strings["%s: maximum sample rate is %d##sgsed"].plurals[0] = "%s: максимальная частота квантования равна %d";
    strings["%s: minimum sample rate is %d##sgsed"].plurals[0] = "%s: минимальная частота квантования равна %d";
    strings["%s: sample rate must be %d##sgsed"].plurals[0] = "%s: частота квантования должна быть равна %d";
    strings["Sample Editor###Sample Editor"].plurals[0] = "Редактор сэмплов###Sample Editor";
    strings["no sample selected##sgsed"].plurals[0] = "сэмпл не выбран";
    strings["select one...##sgsed"].plurals[0] = "выберите сэмпл...";
    strings["or##sgsed0"].plurals[0] = "или";
    strings["Open##sgsed0"].plurals[0] = "Откройте";
    strings["or##sgsed1"].plurals[0] = "или";
    strings["Create New##sgsed"].plurals[0] = "Создайте новый";
    strings["Invalid##sgsed0"].plurals[0] = "Недейств.";
    strings["Invalid##sgsed1"].plurals[0] = "Недейств.";
    strings["%d: %s"].plurals[0] = "%d: %s";
    strings["Open##sgsed1"].plurals[0] = "Открыть";
    strings["import raw...##sgsed"].plurals[0] = "импорт сырых данных...";
    strings["Save##sgsed"].plurals[0] = "Сохранить";
    strings["save raw...##sgsed"].plurals[0] = "сохранить сырые данные...";
    strings["Name##sgsed"].plurals[0] = "Название";
    strings["SNES: loop start must be a multiple of 16 (try with %d)##sgsed"].plurals[0] = "SNES: начало цикла должно быть кратно 16 (попробуйте %d)";
    strings["SNES: loop end must be a multiple of 16 (try with %d)##sgsed"].plurals[0] = "SNES: конец цикла должен быть кратен 16 (попробуйте %d)";
    strings["SNES: sample length will be padded to multiple of 16##sgsed"].plurals[0] = "SNES: длина сэмпла будет скорректирована до кратности 16";
    strings["QSound: loop cannot be longer than 32767 samples##sgsed"].plurals[0] = "QSound: цикл не может быть длиннее 32767 сэмплов";
    strings["QSound: maximum sample length is 65535##sgsed"].plurals[0] = "QSound: максимальная длина сэмпла: 65535";
    strings["NES: loop start must be a multiple of 512 (try with %d)##sgsed"].plurals[0] = "NES: начало цикла должно быть кратно 512 (попробуйте %d)";
    strings["NES: loop end must be a multiple of 128 (try with %d)##sgsed"].plurals[0] = "NES: конец цикла должен быть кратен 128 (попробуйте %d)";
    strings["NES: maximum DPCM sample length is 32648##sgsed"].plurals[0] = "NES: максимальная длина ДИКМ-сэмпла равна 32648";
    strings["X1-010: samples can't loop##sgsed"].plurals[0] = "X1-010: сэмплы не могут быть зациклены";
    strings["X1-010: maximum sample length is 131072##sgsed"].plurals[0] = "X1-010: максимальная длина сэмпла равна 131072";
    strings["GA20: samples can't loop##sgsed"].plurals[0] = "GA20: сэмплы не могут быть зациклены";
    strings["YM2608: loop point ignored on ADPCM (may only loop entire sample)##sgsed"].plurals[0] = "YM2608: точка зацикливания игнорируется для АДИКМ (зациклить можно только сэмпл целиком)";
    strings["YM2608: sample length will be padded to multiple of 512##sgsed"].plurals[0] = "YM2608: длина сэмпла будет скорректирована до кратности 512";
    strings["YM2610: ADPCM-A samples can't loop##sgsed"].plurals[0] = "YM2610: ADPCM-A сэмплы не могут быть зациклены";
    strings["YM2610: loop point ignored on ADPCM-B (may only loop entire sample)##sgsed"].plurals[0] = "YM2610: точка зацикливания игнорируется для АДИКМ-B (зациклить можно только сэмпл целиком)";
    strings["YM2610: sample length will be padded to multiple of 512##sgsed"].plurals[0] = "YM2610: длина сэмпла будет скорректирована до кратности 512";
    strings["YM2610: maximum ADPCM-A sample length is 2097152##sgsed"].plurals[0] = "YM2610: максимальная длина АДИКМ-A сэмпла равна 2097152";
    strings["Y8950: loop point ignored on ADPCM (may only loop entire sample)##sgsed"].plurals[0] = "Y8950: точка зацикливания игнорируется для АДИКМ (зациклить можно только сэмпл целиком)";
    strings["Y8950: sample length will be padded to multiple of 512##sgsed"].plurals[0] = "Y8950: длина сэмпла будет скорректирована до кратности 512";
    strings["Amiga: loop start must be a multiple of 2##sgsed"].plurals[0] = "Amiga: начало зацикливания должно быть кратно 2";
    strings["Amiga: loop end must be a multiple of 2##sgsed"].plurals[0] = "Amiga: конец зацикливания должен быть кратен 2";
    strings["Amiga: maximum sample length is 131070##sgsed"].plurals[0] = "Amiga: максимальная длина сэмпла равна 131070";
    strings["SegaPCM: maximum sample length is 65280##sgsed"].plurals[0] = "SegaPCM: максимальная длина сэмпла равна 65280";
    strings["K053260: loop point ignored (may only loop entire sample)##sgsed"].plurals[0] = "K053260: точка зацикливания игнорируется (зациклить можно только сэмпл целиком)";
    strings["K053260: maximum sample length is 65535##sgsed"].plurals[0] = "K053260: максимальная длина сэмпла равна 65535";
    strings["C140: maximum sample length is 65535##sgsed"].plurals[0] = "C140: максимальная длина сэмпла равна 65535";
    strings["C219: loop start must be a multiple of 2##sgsed"].plurals[0] = "C219: начало зацикливания должно быть кратно 2";
    strings["C219: loop end must be a multiple of 2##sgsed"].plurals[0] = "C219: конец зацикливания должен быть кратен 2";
    strings["C219: maximum sample length is 131072##sgsed"].plurals[0] = "C219: максимальная длина сэмпла равна 131072";
    strings["MSM6295: samples can't loop##sgsed"].plurals[0] = "MSM6295: сэмплы не могут быть зациклены";
    strings["MSM6295: maximum bankswitched sample length is 129024##sgsed"].plurals[0] = "MSM6295: максимальная длина сэмпла с заменой банков равна 129024";
    strings["GBA DMA: loop start must be a multiple of 4##sgsed"].plurals[0] = "GBA DMA: начало цикла должно быть кратно 4";
    strings["GBA DMA: loop length must be a multiple of 16##sgsed"].plurals[0] = "GBA DMA: длина зацикленной части должна быть кратна 16";
    strings["GBA DMA: sample length will be padded to multiple of 16##sgsed"].plurals[0] = "GBA DMA: длина сэмпла будет скорректирована до кратности 16";
    strings["ES5506: backward loop mode isn't supported##sgsed"].plurals[0] = "ES5506: режим обратного зацикливания не поддерживается";
    strings["backward/ping-pong only supported in Generic PCM DAC\nping-pong also on ES5506##sgsed"].plurals[0] = "режим обратного и \"туда-обратно\" зацикливания поддерживается только для универсального ИКМ ЦАП\n\"туда-обратно\" также поддерживается для ES5506";
    strings["Info##sgsed"].plurals[0] = "Информация";
    strings["Rate##sgsed0"].plurals[0] = "Частота";
    strings["Compat Rate##sgsed"].plurals[0] = "Совместимая частота";
    strings["used in DefleMask-compatible sample mode (17xx), in where samples are mapped to an octave.##sgsed"].plurals[0] = "используется при воспроизведении в режиме совместимости с DefleMask (17xx), в котором сэмплы соответствуют октаве.";
    strings["Loop##sgsed"].plurals[0] = "Цикл";
    strings["Loop (length: %d)##Loop"].plurals[0] = "Цикл (длина: %d)##Loop";
    strings["changing the loop in a BRR sample may result in glitches!##sgsed0"].plurals[0] = "Смена зацикленной части в сэмпле BRR может привести к сбоям!";
    strings["Chips##sgsed"].plurals[0] = "Чипы";
    strings["Type##sgsed"].plurals[0] = "Тип";
    strings["BRR emphasis##sgsed"].plurals[0] = "Постобработка для BRR";
    strings["this is a BRR sample.\nenabling this option will muffle it (only affects non-SNES chips).##sgsed"].plurals[0] = "это сэмпл BRR.\nвключение этой опции \"приглушит\" его (на всех чипах кроме SNES).";
    strings["enable this option to slightly boost high frequencies\nto compensate for the SNES' Gaussian filter's muffle.##sgsed"].plurals[0] = "включите эту опцию для небольшого усиления высоких частот\nдля компенсации фильтрующего свойства гауссовой интерполяции, из-за которого сэмпл \"приглушается\".";
    strings["8-bit dither##sgsed"].plurals[0] = "8-битный дизеринг";
    strings["dither the sample when used on a chip that only supports 8-bit samples.##sgsed"].plurals[0] = "произвести дизеринг для сэмпла при его использовании для чипа, поддерживающего только 8-битные сэмплы.";
    strings["Hz##sgsed"].plurals[0] = "Гц";
    strings["Note##sgsed"].plurals[0] = "Нота";
    strings["%s"].plurals[0] = "%s";
    strings["Fine##sgsed"].plurals[0] = "Расстройка";
    strings["Mode##sgsed"].plurals[0] = "Режим";
    strings["Start##sgsed"].plurals[0] = "Начало";
    strings["changing the loop in a BRR sample may result in glitches!##sgsed1"].plurals[0] = "смена зацикленной части в сэмпле BRR может привести к сбоям!";
    strings["End##sgsed"].plurals[0] = "Конец";
    strings["changing the loop in a BRR sample may result in glitches!##sgsed2"].plurals[0] = "смена зацикленной части в сэмпле BRR может привести к сбоям!";
    strings["%s\n%d bytes free##sgsed"].plurals[0] = "%s\nсвободен %d байт";
    strings["%s\n%d bytes free##sgsed"].plurals[1] = "%s\nсвободно %d байта";
    strings["%s\n%d bytes free##sgsed"].plurals[2] = "%s\nсвободно %d байт";
    strings["%s (%s)\n%d bytes free##sgsed"].plurals[0] = "%s (%s)\nсвободен %d байт";
    strings["%s (%s)\n%d bytes free##sgsed"].plurals[1] = "%s (%s)\nсвободно %d байта";
    strings["%s (%s)\n%d bytes free##sgsed"].plurals[2] = "%s (%s)\nсвободно %d байт";
    strings["\n\nnot enough memory for this sample!##sgsed"].plurals[0] = "\n\nнедостаточно памяти для этого сэмпла!";
    strings["Edit mode: Select##sgsed"].plurals[0] = "Режим редактирования: выделение";
    strings["Edit mode: Draw##sgsed"].plurals[0] = "Режим редактирования: рисование";
    strings["Resize##sgsed0"].plurals[0] = "Масштабировать";
    strings["Samples##sgsed0"].plurals[0] = "Сэмплов (новый размер)";
    strings["Resize##sgsed1"].plurals[0] = "Масштабировать";
    strings["couldn't resize! make sure your sample is 8 or 16-bit.##sgsed"].plurals[0] = "не получилось отмасштабировать! убедитесь, что это 8- или 16-битный сэмпл.";
    strings["Resample##sgsed0"].plurals[0] = "Изменить частоту дискретизации";
    strings["Rate##sgsed1"].plurals[0] = "Частота";
    strings["Factor##sgsed"].plurals[0] = "Коэффициент";
    strings["Filter##sgsed"].plurals[0] = "Фильтр";
    strings["Resample##sgsed1"].plurals[0] = "Применить";
    strings["couldn't resample! make sure your sample is 8 or 16-bit.##sgsed"].plurals[0] = "не получилось изменить частоту дискретизации! убедитесь, что это 8- или 16-битный сэмпл.";
    strings["Undo##sgsed"].plurals[0] = "Отменить";
    strings["Redo##sgsed"].plurals[0] = "Повторить";
    strings["Amplify##sgsed"].plurals[0] = "Усилить";
    strings["Volume##sgsed"].plurals[0] = "Громкость";
    strings["Apply##sgsed0"].plurals[0] = "Применить";
    strings["Normalize##sgsed"].plurals[0] = "Нормализовать";
    strings["Fade in##sgsed"].plurals[0] = "Плавное нарастание";
    strings["Fade out##sgsed"].plurals[0] = "Плавное затухание";
    strings["Insert silence##sgsed"].plurals[0] = "Вставить тишину";
    strings["Samples##sgsed1"].plurals[0] = "Сэмплов";
    strings["Go##sgsed"].plurals[0] = "Применить";
    strings["couldn't insert! make sure your sample is 8 or 16-bit.##sgsed"].plurals[0] = "не получилось вставить! убедитесь, что это 8- или 16-битный сэмпл.";
    strings["Apply silence##sgsed"].plurals[0] = "Применить тишину";
    strings["Delete##sgsed"].plurals[0] = "Удалить";
    strings["Trim##sgsed"].plurals[0] = "Обрезать";
    strings["Reverse##sgsed"].plurals[0] = "Реверс";
    strings["Invert##sgsed"].plurals[0] = "Инвертировать";
    strings["Signed/unsigned exchange##sgsed"].plurals[0] = "Знаковый <-> беззнаковый";
    strings["Apply filter##sgsed"].plurals[0] = "Применить фильтр";
    strings["Cutoff:##sgsed"].plurals[0] = "Частота среза:";
    strings["From##sgsed"].plurals[0] = "От";
    strings["To##sgsed"].plurals[0] = "До";
    strings["Resonance##sgsed"].plurals[0] = "Резонанс (добротность)";
    strings["Power##sgsed"].plurals[0] = "Порядок фильтра";
    strings["Low-pass##sgsed"].plurals[0] = "ФНЧ";
    strings["Band-pass##sgsed"].plurals[0] = "ППФ";
    strings["High-pass##sgsed"].plurals[0] = "ФВЧ";
    strings["Apply##sgsed1"].plurals[0] = "Применить";
    strings["Crossfade loop points##sgsed"].plurals[0] = "Сделать плавный переход между началом и концом зацикленной части";
    strings["Number of samples##sgsed"].plurals[0] = "Число сэмплов";
    strings["Linear <-> Equal power##sgsed"].plurals[0] = "Линейный <-> Одинаковая степень";
    strings["Apply##sgsed2"].plurals[0] = "Применить";
    strings["Crossfade: length would go out of bounds. Aborted...##sgsed"].plurals[0] = "Плавный переход: длина выйдет за границы. Действие отменено.";
    strings["Crossfade: length would overflow loopStart. Try a smaller random value.##sgsed"].plurals[0] = "Плавный переход: длина выйдет за пределы начала цикла. Попробуйте с меньшим случайным значением.";
    strings["Preview sample##sgsed"].plurals[0] = "Превью сэмпла";
    strings["Stop sample preview##sgsed"].plurals[0] = "Остановить превью сэмпла";
    strings["Create instrument from sample##sgsed"].plurals[0] = "Создать инструмент из сэмпла";
    strings["Zoom##sgsed0"].plurals[0] = "Масштаб";
    strings["Zoom##sgsed1"].plurals[0] = "Масштаб";
    strings["%dms"].plurals[0] = "%d мс";
    strings["Auto##sgsed"].plurals[0] = "Автоматически";
    strings["cut##sgsed"].plurals[0] = "вырезать";
    strings["copy##sgsed"].plurals[0] = "копировать";
    strings["paste##sgsed"].plurals[0] = "вставить";
    strings["paste (replace)##sgsed"].plurals[0] = "вставить (с заменой)";
    strings["paste (mix)##sgsed"].plurals[0] = "вставить со смешением";
    strings["select all##sgsed"].plurals[0] = "выбрать всё";
    strings["set loop to selection##sgsed"].plurals[0] = "зациклить по выделенной части";
    strings["create wavetable from selection##sgsed"].plurals[0] = "создать волновую таблицу из выделенной части";
    strings["Draw##sgsed"].plurals[0] = "Рисовать";
    strings["Select##sgsed"].plurals[0] = "Выделить";
    strings["%d samples##sgsed"].plurals[0] = "%d сэмпл";
    strings["%d samples##sgsed"].plurals[1] = "%d сэмпла";
    strings["%d samples##sgsed"].plurals[2] = "%d сэмплов";
    strings["%d bytes##sgsed"].plurals[0] = "%d байт";
    strings["%d bytes##sgsed"].plurals[1] = "%d байта";
    strings["%d bytes##sgsed"].plurals[2] = "%d байтов";
    strings[" (%d-%d: %d samples)##sgsed"].plurals[0] = " (%d-%d: %d сэмпл)";
    strings[" (%d-%d: %d samples)##sgsed"].plurals[1] = " (%d-%d: %d сэмпла)";
    strings[" (%d-%d: %d samples)##sgsed"].plurals[2] = " (%d-%d: %d сэмплов)";
    strings["%.2fHz##sgsed"].plurals[0] = "%.2f Гц";
    strings["Non-8/16-bit samples cannot be edited without prior conversion.##sgsed"].plurals[0] = "Сэмплы в формате, отличающемся от 8- или 16-битной ИКМ, невозможно редактировать\nбез предварительной конвертации в один из этих форматов.";

    //   sgsi  src/gui/songInfo.cpp

    strings["Song Info###Song Information"].plurals[0] = "О треке###Song Information";
    strings["Name##sgsi"].plurals[0] = "Название";
    strings["Author##sgsi"].plurals[0] = "Автор";
    strings["Album##sgsi"].plurals[0] = "Альбом";
    strings["System##sgsi"].plurals[0] = "Система";
    strings["Auto##sgsi"].plurals[0] = "Авто";
    strings["Tuning (A-4)##sgsi"].plurals[0] = "Частота (ля 4-ой октавы)";

    //   sgsn  src/gui/songNotes.cpp

    strings["Song Comments###Song Comments"].plurals[0] = "Комментарии трека###Song Comments";

    //   sgsp  src/gui/speed.cpp

    strings["Speed###Speed"].plurals[0] = "Скорость###Speed";
    strings["Base Tempo##TempoOrHz"].plurals[0] = "Основной темп##TempoOrHz";
    strings["Tick Rate##TempoOrHz"].plurals[0] = "Частота движка##TempoOrHz";
    strings["click to display tick rate##sgsp"].plurals[0] = "нажмите для отображения частоты движка";
    strings["click to display base tempo##sgsp"].plurals[0] = "нажмите для отображения основного темпа";
    strings["Groove##sgsp"].plurals[0] = "Ритм-паттерн";
    strings["click for one speed##sgsp"].plurals[0] = "Нажмите для отображения одной скорости";
    strings["Speeds##sgsp"].plurals[0] = "Скорости";
    strings["click for groove pattern##sgsp"].plurals[0] = "Нажмите для отображения ритм-паттернов";
    strings["Speed##sgsp"].plurals[0] = "Скорость";
    strings["click for two (alternating) speeds##sgsp"].plurals[0] = "Нажмите для отображения двух (чередующихся) скоростей";
    strings["Virtual Tempo##sgsp"].plurals[0] = "Виртуальный темп";
    strings["Numerator##sgsp"].plurals[0] = "Числитель";
    strings["Denominator (set to base tempo)##sgsp"].plurals[0] = "Знаменатель (установите равным основному темпу)";
    strings["Divider##sgsp"].plurals[0] = "Делитель";
    strings["Highlight##sgsp"].plurals[0] = "Подсветка";
    strings["Pattern Length##sgsp"].plurals[0] = "Длина паттерна";
    strings["Song Length##sgsp"].plurals[0] = "Длина трека";

    //   sgst  src/gui/stats.cpp

    strings["Statistics###Statistics"].plurals[0] = "Статистика###Statistics";
    strings["Audio load##sgst"].plurals[0] = "Нагрузка от аудио";

    //   sgss  src/gui/subSongs.cpp

    strings["Subsongs###Subsongs"].plurals[0] = "Подпесни###Subsongs";
    strings["%d. <no name>##sgss0"].plurals[0] = "%d. <без названия>";
    strings["%d. <no name>##sgss1"].plurals[0] = "%d. <без названия>";
    strings["Move up##sgss"].plurals[0] = "Сдвинуть вверх";
    strings["Move down##sgss"].plurals[0] = "Сдвинуть вниз";
    strings["too many subsongs!##sgss0"].plurals[0] = "слишком много подпесен!";
    strings["Add##sgss"].plurals[0] = "Добавить";
    strings["too many subsongs!##sgss1"].plurals[0] = "слишком много подпесен!";
    strings["Duplicate##sgss"].plurals[0] = "Клонировать";
    strings["this is the only subsong!##sgss"].plurals[0] = "это единственная подпесня!";
    strings["are you sure you want to remove this subsong?##sgss"].plurals[0] = "вы действительно хотите удалить эту подпесню?";
    strings["Remove##sgss"].plurals[0] = "Удалить";
    strings["Name##sgss"].plurals[0] = "Название";

    //   sgsc  src/gui/sysConf.cpp

    strings["Clock rate:##sgsc0"].plurals[0] = "Тактовая частота:";
    strings["Chip type:##sgsc0"].plurals[0] = "Тип чипа:";
    strings["YM3438 (9-bit DAC)##sgsc"].plurals[0] = "YM3438 (9-битный ЦАП)";
    strings["YM2612 (9-bit DAC with distortion)##sgsc"].plurals[0] = "YM2612 (9-битный ЦАП с искажениями)";
    strings["YMF276 (external DAC)##sgsc"].plurals[0] = "YMF276 (внешний ЦАП)";
    strings["Disable ExtCh FM macros (compatibility)##sgsc0"].plurals[0] = "Отключить FM-макросы для расширенного канала (совместимость)";
    strings["Ins change in ExtCh operator 2-4 affects FB (compatibility)##sgsc0"].plurals[0] = "Смена инструмента на операторах 2-4 расш. канала меняет FB (совместимость)";
    strings["Clock rate:##sgsc1"].plurals[0] = "Тактовая частота:";
    strings["1.79MHz (Half NTSC)##sgsc"].plurals[0] = "1.79 МГц (половина NTSC)";
    strings["Chip type:##sgsc1"].plurals[0] = "Тип чипа:";
    strings["TI SN76489 with Atari-like short noise##sgsc"].plurals[0] = "TI SN76489 с коротким шумом, похожим на Atari";
    strings["Tandy PSSJ 3-voice sound##sgsc"].plurals[0] = "Tandy PSSJ трёхголосый звук";
    strings["Disable noise period change phase reset##sgsc"].plurals[0] = "Отключить сброс фазы при изменении периода шума";
    strings["Disable easy period to note mapping on upper octaves##sgsc0"].plurals[0] = "Отключить простое преобразование периода в ноту на верхних октавах";
    strings["Pseudo-PAL##sgsc0"].plurals[0] = "Псевдо-PAL";
    strings["Disable anti-click##sgsc0"].plurals[0] = "Отключить анти-щелчок";
    strings["Chip revision:##sgsc0"].plurals[0] = "Ревизия чипа:";
    strings["HuC6280 (original)##sgsc"].plurals[0] = "HuC6280 (оригинальный)";
    strings["CPU rate:##sgsc"].plurals[0] = "Частота ЦП:";
    strings["Sample memory:##sgsc"].plurals[0] = "Память сэмплов:";
    strings["8K (rev A/B/E)##sgsc"].plurals[0] = "8 КиБ (версия A/B/E)";
    strings["64K (rev D/F)##sgsc"].plurals[0] = "64 КиБ (версия D/F)";
    strings["DAC resolution:##sgsc"].plurals[0] = "Разрешение ЦАП:";
    strings["16-bit (rev A/B/D/F)##sgsc"].plurals[0] = "16 бит (версия A/B/D/F)";
    strings["8-bit + TDM (rev C/E)##sgsc"].plurals[0] = "8 бит + мультиплексирование с разделением по времени (версия C/E)";
    strings["Enable echo##sgsc0"].plurals[0] = "Включить эхо";
    strings["Swap echo channels##sgsc"].plurals[0] = "Поменять местами каналы эхо";
    strings["Echo delay:##sgsc0"].plurals[0] = "Задержка эхо:";
    strings["Echo resolution:##sgsc"].plurals[0] = "Разрешение эхо:";
    strings["Echo feedback:##sgsc0"].plurals[0] = "Обратная связь эхо:";
    strings["Echo volume:##sgsc0"].plurals[0] = "Громкость эхо:";
    strings["Disable anti-click##sgsc1"].plurals[0] = "Отключить анти-щелчок";
    strings["Chip revision:##sgsc1"].plurals[0] = "Ревизия чипа:";
    strings["Original (DMG)##sgsc"].plurals[0] = "Оригинальный (DMG)";
    strings["Game Boy Color (rev C)##sgsc"].plurals[0] = "Game Boy Color (версия C)";
    strings["Game Boy Color (rev E)##sgsc"].plurals[0] = "Game Boy Color (версия E)";
    strings["Wave channel orientation:##sgsc"].plurals[0] = "Канал волновых таблиц";
    strings["Normal##sgsc"].plurals[0] = "Нормально";
    strings["Inverted##sgsc"].plurals[0] = "Инвертирован";
    strings["Exact data (inverted)##sgsc"].plurals[0] = "Точные данные (инвертирован)";
    strings["Exact output (normal)##sgsc"].plurals[0] = "Точный вывод (нормально)";
    strings["Pretty please one more compat flag when I use arpeggio and my sound length##sgsc"].plurals[0] = "Ну пожалуйста ещё один флаг совместимости когда я использую арпеджио и малую длину звука на шумовом канале";
    strings["Clock rate:##sgsc2"].plurals[0] = "Тактовая частота:";
    strings["DAC bit depth (reduces output rate):##sgsc"].plurals[0] = "Глубина квантования ЦАП (уменьшает частоту дискретизации):";
    strings["Volume scale:##sgsc"].plurals[0] = "Громкость:";
    strings["Mix buffers (allows longer echo delay):##sgsc"].plurals[0] = "Буферы микширования (позволяет делать более долгую задержку эха):";
    strings["Channel limit:##sgsc"].plurals[0] = "Лимит числа каналов:";
    strings["Sample rate:##sgsc"].plurals[0] = "Частота дискретизации:";
    strings["Actual sample rate: %d Hz##sgsc"].plurals[0] = "Настоящая частота дискретизации: %d Гц";
    strings["Max mixer CPU usage: %.0f%%##sgsc"].plurals[0] = "Максимальное использование ЦП микшером: %.0f%%";
    strings["Arcade (4MHz)##sgsc"].plurals[0] = "Arcade (4 МГц)";
    strings["Half NTSC (1.79MHz)##sgsc"].plurals[0] = "Половина NTSC (1.79 МГц)";
    strings["Patch set:##sgsc"].plurals[0] = "Набор патчей:";
    strings["Ignore top/hi-hat frequency changes##sgsc"].plurals[0] = "Игнорировать смену частоты тарелок/хай-хэтов";
    strings["Apply fixed frequency to all drums at once##sgsc"].plurals[0] = "Применять режим фиксированной частоты ко всем ударным";
    strings["Broken pitch macro/slides (compatibility)##sgsc0"].plurals[0] = "Сломанное портаменто эффектов и макросов (совместимость)";
    strings["Pseudo-PAL##sgsc1"].plurals[0] = "Псевдо-PAL";
    strings["Broken pitch macro/slides (compatibility)##sgsc1"].plurals[0] = "Сломанное портаменто эффектов и макросов (совместимость)";
    strings["Clock rate:##sgsc20"].plurals[0] = "Тактовая частота:";
    strings["DPCM channel mode:##sgsc"].plurals[0] = "Режим ДИКМ-канала:";
    strings["DPCM (muffled samples; low CPU usage)##sgsc"].plurals[0] = "ДИКМ (приглушённые сэмплы; малая нагрузка на ЦП)";
    strings["PCM (crisp samples; high CPU usage)##sgsc"].plurals[0] = "ИКМ (чёткие сэмплы; большая нагрузка на ЦП)";
    strings["Clock rate:##sgsc18"].plurals[0] = "Тактовая частота:";
    strings["Clock rate:##sgsc19"].plurals[0] = "Тактовая частота:";
    strings["Global parameter priority:##sgsc0"].plurals[0] = "Приоритет глобальных параметров:";
    strings["Left to right##sgsc0"].plurals[0] = "Слева направо";
    strings["Last used channel##sgsc0"].plurals[0] = "Последний использованный канал";
    strings["Hard reset envelope:##sgsc"].plurals[0] = "Огибающая при жёстком перезапуске:";
    strings["Attack##sgsc"].plurals[0] = "Атака";
    strings["Decay##sgsc"].plurals[0] = "Спад";
    strings["Sustain##sgsc"].plurals[0] = "Сустейн";
    strings["Release##sgsc"].plurals[0] = "Релиз";
    strings["Envelope reset time:##sgsc"].plurals[0] = "Время сброса огибающей:";
    strings["- 0 disables envelope reset. not recommended!\n- 1 may trigger SID envelope bugs.\n- values that are too high may result in notes being skipped.##sgsc"].plurals[0] = "- 0 отключает сброс огибающей. не рекомендуется!\n- 1 может привести к появлению сбоев огибающей SID.\n- слишком большие значения могут привести к пропуску коротких нот.";
    strings["Disable 1Exy env update (compatibility)##sgsc"].plurals[0] = "Отключить обновление огибающей при применении эффекта 1Exy (совместимость)";
    strings["Relative duty and cutoff macros are coarse (compatibility)##sgsc"].plurals[0] = "Макросы скважности и частоты среза в относительном режиме имеют меньшее разрешение (совместимость)";
    strings["Cutoff macro race conditions (compatibility)##sgsc"].plurals[0] = "Состояние гонки макроса скважности (совместимость)";
    strings["Disable ExtCh FM macros (compatibility)##sgsc1"].plurals[0] = "Отключить FM-макросы для расширенного канала (совместимость)";
    strings["Ins change in ExtCh operator 2-4 affects FB (compatibility)##sgsc1"].plurals[0] = "Смена инструмента на операторах 2-4 расш. канала меняет FB (совместимость)";
    strings["SSG Volume##sgsc0"].plurals[0] = "Громкость SSG";
    strings["FM/ADPCM Volume##sgsc0"].plurals[0] = "Громкость FM/АДИКМ";
    strings["Clock rate:##sgsc3"].plurals[0] = "Тактовая частота:";
    strings["0.83MHz (Pre-divided Sunsoft 5B on PAL)##sgsc"].plurals[0] = "0.83 МГц (Sunsoft 5B с предварительным делителем частоты, PAL)";
    strings["0.89MHz (Pre-divided Sunsoft 5B)##sgsc"].plurals[0] = "0.89 МГц (Sunsoft 5B с предварительным делителем частоты)";
    strings["Chip type:##sgsc2"].plurals[0] = "Тип чипа:";
    strings["note: AY-3-8914 is not supported by the VGM format!##sgsc"].plurals[0] = "примечание: AY-3-8914 не поддерживается в формате VGM!";
    strings["Stereo##_AY_STEREO"].plurals[0] = "Стерео##_AY_STEREO";
    strings["Separation##sgsc"].plurals[0] = "Разделение в стерео";
    strings["Half Clock divider##_AY_CLKSEL"].plurals[0] = "Делить тактовую частоту на 2##_AY_CLKSEL";
    strings["Stereo separation:##sgsc"].plurals[0] = "Разделение в стерео:";
    strings["Model:##sgsc"].plurals[0] = "Модель:";
    strings["Chip memory:##sgsc"].plurals[0] = "Память чипа:";
    strings["2MB (ECS/AGA max)##sgsc"].plurals[0] = "2 МиБ (максимум для ECS/AGA)";
    strings["512KB (OCS max)##sgsc"].plurals[0] = "512 КиБ (максимум для OCS)";
    strings["Bypass frequency limits##sgsc"].plurals[0] = "Игнорировать ограничения частоты";
    strings["Mixing mode:##sgsc"].plurals[0] = "Режим микширования:";
    strings["Mono##sgsc"].plurals[0] = "Моно";
    strings["Mono (no distortion)##sgsc"].plurals[0] = "Моно (без искажений)";
    strings["Stereo##sgsc0"].plurals[0] = "Стерео";
    strings["Clock rate:##sgsc4"].plurals[0] = "Тактовая частота:";
    strings["Speaker type:##sgsc"].plurals[0] = "Тип пищалки:";
    strings["Unfiltered##sgsc"].plurals[0] = "Без фильтрации";
    strings["Cone##sgsc"].plurals[0] = "Диффузор";
    strings["Piezo##sgsc"].plurals[0] = "Пьезопищалка";
    strings["Use system beeper (Linux only!)##sgsc"].plurals[0] = "Использовать пищалку на материнской плате (только для Linux!)";
    strings["Reset phase on frequency change##sgsc"].plurals[0] = "Сбор фазы при изменении частоты";
    strings["Echo delay:##sgsc1"].plurals[0] = "Задержка эхо:";
    strings["Echo feedback:##sgsc1"].plurals[0] = "Обратная связь эхо:";
    strings["Clock rate:##sgsc5"].plurals[0] = "Тактовая частота:";
    strings["Stereo##sgsc1"].plurals[0] = "Стерео";
    strings["Bankswitched (Seta 2)##sgsc"].plurals[0] = "Со сменой банков памяти (Seta 2)";
    strings["Clock rate:##sgsc6"].plurals[0] = "Тактовая частота:";
    strings["Initial channel limit:##sgsc0"].plurals[0] = "Изначальный лимит числа каналов:";
    strings["Disable hissing##sgsc"].plurals[0] = "Отключить высокочастотное пищание";
    strings["Scale frequency to wave length##sgsc"].plurals[0] = "Масштабировать частоту под длину волны";
    strings["Initial channel limit:##sgsc1"].plurals[0] = "Изначальный лимит числа каналов:";
    strings["Volume scale:##sgsc0"].plurals[0] = "Масштабирование громкости:";
    strings["Clock rate:##sgsc7"].plurals[0] = "Тактовая частота:";
    strings["Output rate:##sgsc0"].plurals[0] = "Частота квантования на выходе:";
    strings["FM: clock / 72, SSG: clock / 16##sgsc0"].plurals[0] = "FM: такт. част. / 72, SSG: такт. част. / 16";
    strings["FM: clock / 36, SSG: clock / 8##sgsc"].plurals[0] = "FM: такт. част. / 36, SSG: такт. част. / 8";
    strings["FM: clock / 24, SSG: clock / 4##sgsc"].plurals[0] = "FM: такт. част. / 24, SSG: такт. част. / 4";
    strings["SSG Volume##sgsc1"].plurals[0] = "Громкость SSG";
    strings["FM Volume##sgsc"].plurals[0] = "Громкость FM";
    strings["Disable ExtCh FM macros (compatibility)##sgsc2"].plurals[0] = "Отключить FM-макросы для расширенного канала (совместимость)";
    strings["Ins change in ExtCh operator 2-4 affects FB (compatibility)##sgsc2"].plurals[0] = "Смена инструмента на операторах 2-4 расш. канала меняет FB (совместимость)";
    strings["Clock rate:##sgsc8"].plurals[0] = "Тактовая частота:";
    strings["8MHz (Arcade)##sgsc"].plurals[0] = "8 МГц (Arcade)";
    strings["Output rate:##sgsc1"].plurals[0] = "Частота квантования на выходе:";
    strings["FM: clock / 144, SSG: clock / 32##sgsc"].plurals[0] = "FM: такт. част. / 144, SSG: такт. част. / 32";
    strings["FM: clock / 72, SSG: clock / 16##sgsc1"].plurals[0] = "FM: такт. част. / 72, SSG: такт. част. / 16";
    strings["FM: clock / 48, SSG: clock / 8##sgsc"].plurals[0] = "FM: такт. част. / 48, SSG: такт. част. / 8";
    strings["SSG Volume##sgsc2"].plurals[0] = "Громкость SSG";
    strings["FM/ADPCM Volume##sgsc1"].plurals[0] = "Громкость FM/АДИКМ";
    strings["Disable ExtCh FM macros (compatibility)##sgsc3"].plurals[0] = "Отключить FM-макросы для расширенного канала (совместимость)";
    strings["Ins change in ExtCh operator 2-4 affects FB (compatibility)##sgsc3"].plurals[0] = "Смена инструмента на операторах 2-4 расш. канала меняет FB (совместимость)";
    strings["Clock rate:##sgsc9"].plurals[0] = "Тактовая частота:";
    strings["Chip type:##sgsc3"].plurals[0] = "Тип чипа:";
    strings["RF5C68 (10-bit output)##sgsc"].plurals[0] = "RF5C68 (10-битный звук)";
    strings["RF5C164 (16-bit output)##sgsc"].plurals[0] = "RF5C164 (16-битный звук)";
    strings["Clock rate:##sgsc10"].plurals[0] = "Тактовая частота:";
    strings["Sample rate table:##sgsc"].plurals[0] = "Таблица частот квантования:";
    strings["divider \\ clock##sgsc"].plurals[0] = "Делитель тактовой частоты";
    strings["full##sgsc"].plurals[0] = "полная";
    strings["half##sgsc"].plurals[0] = "половина";
    strings["Clock rate:##sgsc11"].plurals[0] = "Тактовая частота:";
    strings["Output rate:##sgsc2"].plurals[0] = "Частота квантования на выходе:";
    strings["clock / 132##sgsc"].plurals[0] = "такт. част. / 132";
    strings["clock / 165##sgsc"].plurals[0] = "такт. част. / 165";
    strings["Bankswitched (NMK112)##sgsc"].plurals[0] = "Смена банков (NMK112)";
    strings["Clock rate:##sgsc12"].plurals[0] = "Тактовая частота:";
    strings["1.5MHz (Arcade)##sgsc"].plurals[0] = "1.5 МГц (Arcade)";
    strings["Consistent frequency across all duties##sgsc"].plurals[0] = "Стабильная частота для всех скважностей";
    strings["note: only works for an initial LFSR value of 0!##sgsc"].plurals[0] = "примечание: работает только для нулевого начального состояния РСЛОС!";
    strings["Clock rate:##sgsc13"].plurals[0] = "Тактовая частота:";
    strings["Clock rate:##sgsc14"].plurals[0] = "Тактовая частота:";
    strings["Chip type:##sgsc4"].plurals[0] = "Тип чипа:";
    strings["Compatible panning (0800)##sgsc"].plurals[0] = "Совместимое панорамирование (0800)";
    strings["Clock rate:##sgsc15"].plurals[0] = "Тактовая частота:";
    strings["Output rate:##sgsc3"].plurals[0] = "Частота квантования на выходе:";
    strings["Output bit depth:##sgsc"].plurals[0] = "Глубина квантования на выходе:";
    strings["Stereo##sgsc2"].plurals[0] = "Стерео";
    strings["Interpolation:##sgsc"].plurals[0] = "Интерполяция:";
    strings["None##sgsc"].plurals[0] = "Выкл.";
    strings["Linear##sgsc"].plurals[0] = "Линейная";
    strings["Cubic##sgsc"].plurals[0] = "Кубическая";
    strings["Sinc##sgsc"].plurals[0] = "Sinc";
    strings["Volume scale:##sgsc1"].plurals[0] = "Масштабирование громкости:";
    strings["Left##VolScaleL"].plurals[0] = "Слева##VolScaleL";
    strings["Right##VolScaleL"].plurals[0] = "Справа##VolScaleL";
    strings["Enable echo##sgsc1"].plurals[0] = "Включить эхо";
    strings["Initial echo state:##sgsc"].plurals[0] = "Начальное состояние эхо:";
    strings["Delay##EchoDelay"].plurals[0] = "Задержка##EchoDelay";
    strings["Feedback##EchoFeedback"].plurals[0] = "Обр. связь##EchoFeedback";
    strings["Echo volume:##sgsc1"].plurals[0] = "Громкость эхо:";
    strings["Left##EchoVolL"].plurals[0] = "Слева##EchoVolL";
    strings["Right##EchoVolL"].plurals[0] = "Справа##EchoVolL";
    strings["Echo filter:##sgsc"].plurals[0] = "Фильтр эхо:";
    strings["Hex##SNESFHex"].plurals[0] = "Шест.##SNESFHex";
    strings["Dec##SNESFHex"].plurals[0] = "Дес.##SNESFHex";
    strings["sum: %d##sgsc"].plurals[0] = "сумма: %d";
    strings["Detune##sgsc"].plurals[0] = "Расстройка";
    strings["Capacitor values (nF):##sgsc"].plurals[0] = "Ёмкость конденсаторов (нФ):";
    strings["Initial part volume (channel 1-4):##sgsc"].plurals[0] = "Начальная громкость (каналы 1-4):";
    strings["Initial part volume (channel 5-8):##sgsc"].plurals[0] = "Начальная громкость (каналы 5-8):";
    strings["Envelope mode (channel 1-4):##sgsc"].plurals[0] = "Режим огибающей (каналы 1-4):";
    strings["Capacitor (attack/decay)##EM00"].plurals[0] = "Конденсатор (атака/спад)##EM00";
    strings["External (volume macro)##EM01"].plurals[0] = "Внешняя (макрос громкости)##EM01";
    strings["Envelope mode (channel 5-8):##sgsc"].plurals[0] = "Режим огибающей (каналы 5-8):";
    strings["Capacitor (attack/decay)##EM10"].plurals[0] = "Конденсатор (атака/спад)##EM10";
    strings["External (volume macro)##EM11"].plurals[0] = "Внешняя (макрос громкости)##EM11";
    strings["Global vibrato:##sgsc"].plurals[0] = "Глобальное вибрато:";
    strings["Speed##sgsc"].plurals[0] = "Скорость";
    strings["Depth##sgsc"].plurals[0] = "Глубина";
    strings["Disable easy period to note mapping on upper octaves##sgsc1"].plurals[0] = "Отключить простое преобразование периода в ноту на верхних октавах";
    strings["Stereo##sgsc3"].plurals[0] = "Стерео";
    strings["Waveform storage mode:##sgsc0"].plurals[0] = "Способ хранения волн:";
    strings["RAM##sgsc"].plurals[0] = "ОЗУ";
    strings["ROM (up to 8 waves)##sgsc"].plurals[0] = "ПЗУ (до 8 волн)";
    strings["Compatible noise frequencies##sgsc"].plurals[0] = "Совместимые частоты шума";
    strings["Legacy slides and pitch (compatibility)##sgsc"].plurals[0] = "Старые частоты и портаменто (совместимость)";
    strings["Clock rate:##sgsc16"].plurals[0] = "Тактовая частота:";
    strings["Clock rate:##sgsc17"].plurals[0] = "Тактовая частота:";
    strings["Global parameter priority:##sgsc1"].plurals[0] = "Приоритет глобальных параметров:";
    strings["Left to right##sgsc1"].plurals[0] = "Слева направо";
    strings["Last used channel##sgsc1"].plurals[0] = "Последний использованный канал";
    strings["Banking style:##sgsc"].plurals[0] = "Стиль переключения банков:";
    strings["Raw (16MB; no VGM export!)##sgsc"].plurals[0] = "Сырой (16 МиБ; не поддерживается при экспорте в VGM!)";
    strings["Waveform storage mode:##sgsc1"].plurals[0] = "Способ хранения волн:";
    strings["Dynamic (unconfirmed)##sgsc"].plurals[0] = "Динамический (не подтверждён)";
    strings["Static (up to 5 waves)##sgsc"].plurals[0] = "Статический (до 5 волн)";
    strings["DS (4MB RAM)##sgsc"].plurals[0] = "DS (4 МиБ ОЗУ)";
    strings["DSi (16MB RAM)##sgsc"].plurals[0] = "DSi (16 МиБ ОЗУ)";
    strings["nothing to configure##sgsc"].plurals[0] = "настраивать нечего";
    strings["Downmix chip output to mono##sgsc"].plurals[0] = "Свести звук чипа в моно";
    strings["Reserved blocks for wavetables:##sgsc"].plurals[0] = "Зарезервированных блоков под волновые таблицы:";
    strings["Reserve this many blocks 256 bytes each in sample memory.\nEach block holds one wavetable (is used for one wavetable channel),\nso reserve as many as you need.##sgsc"].plurals[0] = "Зарезервировать столько блоков, каждый по 256 байт, в памяти сэмплов.\nВ каждый блок помещается одна волновая таблица (он используется для одного канала в режиме волновых таблиц),\nтак что выделяйте столько, сколько вам нужно.";
    strings["Custom clock rate##sgsc"].plurals[0] = "Пользовательская тактовая частота";
    strings["Hz##sgscHz"].plurals[0] = "Гц##sgscHz";
    strings["1MB##sgsc"].plurals[0] = "1 МиБ";
    strings["256KB##sgsc"].plurals[0] = "256 КиБ";
    strings["Namco System 2 (2MB)##sgsc"].plurals[0] = "Namco System 2 (2 МиБ)";
    strings["Namco System 21 (4MB)##sgsc"].plurals[0] = "Namco System 21 (4 МиБ)";

    //   sgsm  src/gui/sysManager.cpp

    strings["Chip Manager###Chip Manager"].plurals[0] = "Менеджер чипов###Chip Manager";
    strings["Preserve channel order##sgsm"].plurals[0] = "Сохранить порядок каналов";
    strings["Clone channel data##sgsm"].plurals[0] = "Клонировать данные каналов";
    strings["Clone at end##sgsm"].plurals[0] = "Клонировать в конец";
    strings["Name##sgsm"].plurals[0] = "Название";
    strings["Actions##sgsm"].plurals[0] = "Действия";
    strings["(drag to swap chips)##sgsm"].plurals[0] = "(перетащите, чтобы поменять местами каналы)";
    strings["Clone##SysDup"].plurals[0] = "Клонировать##SysDup";
    strings["cannot duplicate chip! (##sgsm"].plurals[0] = "не получилось склонировать чип! (";
    strings["max number of systems is %d##sgsm"].plurals[0] = "максимальное число чипов/систем не может превышать %d";
    strings["max number of total channels is %d##sgsm"].plurals[0] = "максимальное общее число каналов не может превышать %d";
    strings["Change##SysChange"].plurals[0] = "Сменить##SysChange";
    strings["Are you sure you want to remove this chip?##sgsm"].plurals[0] = "Вы действительно хотите удалить этот чип?";
    strings["Remove##sgsm"].plurals[0] = "Удалить";
    strings["cannot add chip! (##sgsm"].plurals[0] = "не могу добавить чип! (";

    //   sgsa  src/gui/sysPartNumber.cpp

    strings["ZXS Beeper##sgsa"].plurals[0] = "ZXS (пищалка)";

    //   sgsp  src/gui/sysPicker.cpp

    strings["Search...##sgsp"].plurals[0] = "Поиск...";

    // # sgvm  src/gui/volMeter.cpp

    strings["Volume Meter###Volume Meter"].plurals[0] = "Измеритель громкости###Volume Meter";

    //   sgwe  src/gui/waveEdit.cpp

    strings["Sine##sgwe0"].plurals[0] = "Синус";
    strings["Triangle##sgwe0"].plurals[0] = "Треуг. волна";
    strings["Saw##sgwe0"].plurals[0] = "Пила";
    strings["Pulse##sgwe"].plurals[0] = "Меандр";

    strings["None##sgwe"].plurals[0] = "Нет";
    strings["Linear##sgwe"].plurals[0] = "Линейная";
    strings["Cosine##sgwe"].plurals[0] = "Косинусоидная";
    strings["Cubic##sgwe"].plurals[0] = "Кубическая";

    strings["Sine##sgwe1"].plurals[0] = "Синус";
    strings["Rect. Sine##sgwe"].plurals[0] = "Выпрямл. синус";
    strings["Abs. Sine##sgwe"].plurals[0] = "Модуль синуса";
    strings["Quart. Sine##sgwe"].plurals[0] = "Четвертинки синуса";
    strings["Squish. Sine##sgwe"].plurals[0] = "Сжатый синус";
    strings["Abs. Squish. Sine##sgwe"].plurals[0] = "Модуль сж. синуса";
    strings["Square##sgwe"].plurals[0] = "Меандр";
    strings["rectSquare##sgwe"].plurals[0] = "Выпрямл. меандр";
    strings["Saw##sgwe1"].plurals[0] = "Пила";
    strings["Rect. Saw##sgwe"].plurals[0] = "Выпрямл. пила";
    strings["Abs. Saw##sgwe"].plurals[0] = "Модуль пилы";
    strings["Cubed Saw##sgwe"].plurals[0] = "Пила в кубе";
    strings["Rect. Cubed Saw##sgwe"].plurals[0] = "Выпрямл. пила в кубе";
    strings["Abs. Cubed Saw##sgwe"].plurals[0] = "Модуль пилы в кубе";
    strings["Cubed Sine##sgwe"].plurals[0] = "Куб синуса";
    strings["Rect. Cubed Sine##sgwe"].plurals[0] = "Выпрямл. куб синуса";
    strings["Abs. Cubed Sine##sgwe"].plurals[0] = "Модуль куба синуса";
    strings["Quart. Cubed Sine##sgwe"].plurals[0] = "Четвертинки куба синуса";
    strings["Squish. Cubed Sine##sgwe"].plurals[0] = "Сжатый куб синуса";
    strings["Squish. Abs. Cub. Sine##sgwe"].plurals[0] = "Модуль сж. куба синуса";
    strings["Triangle##sgwe1"].plurals[0] = "Треуг. волна";
    strings["Rect. Triangle##sgwe"].plurals[0] = "Выпрямл. треуг. волна";
    strings["Abs. Triangle##sgwe"].plurals[0] = "Модуль треуг. волны";
    strings["Quart. Triangle##sgwe"].plurals[0] = "Четвертинки треуг. волны";
    strings["Squish. Triangle##sgwe"].plurals[0] = "Сжатая треуг. волна";
    strings["Abs. Squish. Triangle##sgwe"].plurals[0] = "Модуль сж. треуг. волны";
    strings["Cubed Triangle##sgwe"].plurals[0] = "Куб треуг. волны";
    strings["Rect. Cubed Triangle##sgwe"].plurals[0] = "Выпрямл. куб треуг. волны";
    strings["Abs. Cubed Triangle##sgwe"].plurals[0] = "Модуль куба треуг. волны";
    strings["Quart. Cubed Triangle##sgwe"].plurals[0] = "Четвертинки куба треуг. волны";
    strings["Squish. Cubed Triangle##sgwe"].plurals[0] = "Сжатый куб треуг. волны";
    strings["Squish. Abs. Cub. Triangle##sgwe"].plurals[0] = "Сжатый модуль куба треуг. волны";

    strings["Wavetable Editor###Wavetable Editor"].plurals[0] = "Редактор волновых таблиц###Wavetable Editor";
    strings["no wavetable selected##sgwe0"].plurals[0] = "не выбрано ни одной волновой таблицы";
    strings["no wavetable selected##sgwe1"].plurals[0] = "не выбрано ни одной волновой таблицы";
    strings["select one...##sgwe"].plurals[0] = "выберите волновую таблицу...";
    strings["or##sgwe0"].plurals[0] = "или";
    strings["Open##sgwe0"].plurals[0] = "Откройте";
    strings["or##sgwe1"].plurals[0] = "или";
    strings["Create New##sgwe"].plurals[0] = "Создайте новую";
    strings["Open##sgwe1"].plurals[0] = "Открыть";
    strings["Save##sgwe"].plurals[0] = "Сохранить";
    strings["export .dmw...##sgwe"].plurals[0] = "экспорт .dmw...";
    strings["export raw...##sgwe"].plurals[0] = "экспорт сырых данных...";
    strings["Steps##sgwe"].plurals[0] = "Уровни";
    strings["Lines##sgwe"].plurals[0] = "Линии";
    strings["Width##sgwe"].plurals[0] = "Длина";
    strings["use a width of:\n- any on Amiga/N163\n- 32 on Game Boy, PC Engine, SCC, Konami Bubble System, Namco WSG, Virtual Boy and WonderSwan\n- 64 on FDS\n- 128 on X1-010\n- 256 for ES5503\nany other widths will be scaled during playback.##sgwe"].plurals[0] = "используйте следующие длины:\n- любую для Amiga/N163\n- 32 для Game Boy, PC Engine, SCC, Konami Bubble System, Namco WSG, Virtual Boy и WonderSwan\n- 64 для FDS\n- 128 для X1-010\n- 256 для ES5503\nлюбые другие длины будут отмасштабированы во время воспроизведения.";
    strings["Height##sgwe"].plurals[0] = "Высота";
    strings["use a height of:\n- 16 for Game Boy, WonderSwan, Namco WSG, Konami Bubble System, X1-010 Envelope shape and N163\n- 32 for PC Engine\n- 64 for FDS and Virtual Boy\n- 256 for X1-010, SCC and ES5503\nany other heights will be scaled during playback.##sgwe"].plurals[0] = "используйте следующие высоты:\n- 16 для Game Boy, WonderSwan, Namco WSG, Konami Bubble System, формы огибающей X1-010 и N163\n- 32 для PC Engine\n- 64 для FDS и Virtual Boy\n- 256 для X1-010, SCC и ES5503\nлюбые другие высоты будут отмасштабированы во время воспроизведения.";
    strings["Shapes##sgwe"].plurals[0] = "Волны";
    strings["Duty##sgwe"].plurals[0] = "Скважность";
    strings["Exponent##sgwe"].plurals[0] = "Пок. степени";
    strings["XOR Point##sgwe"].plurals[0] = "Точка XOR";
    strings["Amplitude/Phase##sgwe"].plurals[0] = "Амплитуда/фаза";
    strings["Op##sgwe0"].plurals[0] = "Опер.";
    strings["Level##sgwe"].plurals[0] = "Громкость";
    strings["Mult##sgwe"].plurals[0] = "Множитель";
    strings["FB##sgwe"].plurals[0] = "Обр. св.";
    strings["Op##sgwe1"].plurals[0] = "Опер.";
    strings["Waveform##sgwe"].plurals[0] = "Волна";
    strings["Connection Diagram##sgwe0"].plurals[0] = "Матрица модуляции";
    strings["Connection Diagram##sgwe1"].plurals[0] = "Матрица модуляции";
    strings["Out##sgwe"].plurals[0] = "Вывод";
    strings["WaveTools##sgwe"].plurals[0] = "Инструменты";
    strings["Scale X##sgwe"].plurals[0] = "Масштаб X";
    strings["wavetable longer than 256 samples!##sgwe"].plurals[0] = "волновая таблица длинее 256 шагов!";
    strings["Scale Y##sgwe"].plurals[0] = "Масштаб Y";
    strings["Offset X##sgwe"].plurals[0] = "Сдвиг X";
    strings["Offset Y##sgwe"].plurals[0] = "Сдвиг Y";
    strings["Smooth##sgwe"].plurals[0] = "Сгладить";
    strings["Amplify##sgwe"].plurals[0] = "Усилить";
    strings["Normalize##sgwe"].plurals[0] = "Нормализовать";
    strings["Invert##sgwe"].plurals[0] = "Инвертировать";
    strings["Reverse##sgwe"].plurals[0] = "Реверс";
    strings["Half##sgwe"].plurals[0] = "Сократить 2х";
    strings["Double##sgwe"].plurals[0] = "Растянуть 2х";
    strings["Convert Signed/Unsigned##sgwe"].plurals[0] = "Конверт. знаковая <-> беззнаковая";
    strings["Randomize##sgwe"].plurals[0] = "Заполнить случ. знач.";
    strings["Dec##sgwe"].plurals[0] = "Дес.";
    strings["Hex##sgwe"].plurals[0] = "Шест.";
    strings["Signed/Unsigned##sgwe"].plurals[0] = "Знаковая <-> беззнаковая";

    //   sgxy  src/gui/xyOsc.cpp

    strings["Oscilloscope (X-Y)###Oscilloscope (X-Y)"].plurals[0] = "Осциллограф (X-Y)###Oscilloscope (X-Y)";
    strings["X Channel##sgxy"].plurals[0] = "Канал оси X";
    strings["Invert##X"].plurals[0] = "Инвертировать";
    strings["Y Channel##sgxy"].plurals[0] = "Канал оси Y";
    strings["Invert##Y"].plurals[0] = "Инвертировать";
    strings["Zoom##sgxy"].plurals[0] = "Масштаб";
    strings["Samples##sgxy"].plurals[0] = "Сэмплов";
    strings["Decay Time (ms)##sgxy"].plurals[0] = "Время затухания (мс)";
    strings["Intensity##sgxy"].plurals[0] = "Яркость";
    strings["Line Thickness##sgxy"].plurals[0] = "Толщина линий";
    strings["OK##sgxy"].plurals[0] = "ОК";
    strings["(-Infinity)dB,(-Infinity)dB##sgxy"].plurals[0] = "(минус бесконечность) дБ,(минус бесконечность) дБ";
    strings["(-Infinity)dB,%.1fdB##sgxy"].plurals[0] = "(минус бесконечность) дБ,%.1f дБ";
    strings["%.1fdB,(-Infinity)dB##sgxy"].plurals[0] = "%.1f дБ,(минус бесконечность) дБ";

    //WINDOW NAMES

    strings["Orders###Orders"].plurals[0] = "Матрица паттернов###Orders";
    strings["About Furnace###About Furnace"].plurals[0] = "О Furnace###About Furnace";
    strings["Channels###Channels"].plurals[0] = "Каналы###Channels";
    strings["Oscilloscope (per-channel)###Oscilloscope (per-channel)"].plurals[0] = "Осциллографы (отд. кан.)###Oscilloscope (per-channel)";
    strings["Instruments###Instruments"].plurals[0] = "Инструменты###Instruments";
    strings["Wavetables###Wavetables"].plurals[0] = "Волновые таблицы###Wavetables";
    strings["Debug###Debug"].plurals[0] = "Отладка###Debug";
    strings["Samples###Samples"].plurals[0] = "Сэмплы###Samples";
    strings["MobileEdit###MobileEdit"].plurals[0] = "Моб. меню ред.###MobileEdit";
    strings["Log Viewer###Log Viewer"].plurals[0] = "Просмотр логов###Log Viewer";
    strings["Mixer###Mixer"].plurals[0] = "Микшер###Mixer";
    strings["OrderSel###OrderSel"].plurals[0] = "Выб. матр. пат.###OrderSel";
    strings["Spoiler###Spoiler"].plurals[0] = "Спойлер###Spoiler";
    //popups
    strings["Warning###Warning"].plurals[0] = "Внимание###Warning";
    strings["Error###Error"].plurals[0] = "Ошибка###Error";
    strings["Select Instrument###Select Instrument"].plurals[0] = "Выберите инструмент###Select Instrument";
    strings["Import Raw Sample###Import Raw Sample"].plurals[0] = "Импорт сырых данных сэмпла###Import Raw Sample";
    strings["Rendering...###Rendering..."].plurals[0] = "Рендер...###Rendering...";
    
    //MACRO EDITOR

    //macro hover notes

    strings["exponential##sgmu"].plurals[0] = "экспоненциальное";
    strings["linear##sgmu"].plurals[0] = "линейное";
    strings["direct##sgmu"].plurals[0] = "прямое";

    strings["Release"].plurals[0] = "Релиз";
    strings["Loop"].plurals[0] = "Цикл";

    strings["HP/K2, HP/K2##sgmu"].plurals[0] = "ФВЧ/K2, ФВЧ/K2";
    strings["HP/K2, LP/K1##sgmu"].plurals[0] = "ФВЧ/K2, ФНЧ/K1";
    strings["LP/K2, LP/K2##sgmu"].plurals[0] = "ФНЧ/K2, ФНЧ/K2";
    strings["LP/K2, LP/K1##sgmu"].plurals[0] = "ФНЧ/K2, ФНЧ/K1";

    strings["Saw##sgmu"].plurals[0] = "Пила";
    strings["Square##sgmu"].plurals[0] = "Меандр";
    strings["Triangle##sgmu"].plurals[0] = "Треугольная волна";
    strings["Random##sgmu"].plurals[0] = "Шум";

    //src/gui/settings.cpp

    strings["<Use system font>##sgse0"].plurals[0] = "<Использовать системный шрифт>";
    strings["<Custom...>##sgse0"].plurals[0] = "<Внешний...>";
    strings["<Use system font>##sgse1"].plurals[0] = "<Использовать системный шрифт>";
    strings["<Custom...>##sgse1"].plurals[0] = "<Внешний...>";
    strings["<Use system font>##sgse2"].plurals[0] = "<Использовать системный шрифт>";
    strings["<Custom...>##sgse2"].plurals[0] = "<Внешний...>";
    strings["High##sgse"].plurals[0] = "Высокое";
    strings["Low##sgse"].plurals[0] = "Низкое";
    strings["ASAP (C++ port)##sgse"].plurals[0] = "ASAP (портирован на C++)";
    strings["ESFMu (fast)##sgse"].plurals[0] = "ESFMu (быстрый)";
    strings["Lower##sgse"].plurals[0] = "Очень низкое";
    strings["Low##sgse1"].plurals[0] = "Низкое";
    strings["Medium##sgse"].plurals[0] = "Среднее";
    strings["High##sgse"].plurals[0] = "Высокое";
    strings["Ultra##sgse"].plurals[0] = "Очень высокое";
    strings["Ultimate##sgse"].plurals[0] = "Максимальное";
    strings["KIOCSOUND on /dev/tty1##sgse"].plurals[0] = "KIOCSOUND в /dev/tty1";
    strings["KIOCSOUND on standard output##sgse"].plurals[0] = "KIOCSOUND в стандартном выводе";
    strings["Disabled/custom##sgse0"].plurals[0] = "Выкл./пользовательский";
    strings["Two octaves (0 is C-4, F is D#5)##sgse"].plurals[0] = "Две октавы (0 = C-4, F = D#5)";
    strings["Raw (note number is value)##sgse"].plurals[0] = "Сырой ввод (номер ноты - само значение)";
    strings["Two octaves alternate (lower keys are 0-9, upper keys are A-F)##sgse"].plurals[0] = "Альтернативный ввод двух октав (нижние клавиши 0-9, верхние - A-F)";
    strings["Use dual control change (one for each nibble)##sgse0"].plurals[0] = "Использовать двойную смену значения контроллера (по одному на тетраду)";
    strings["Use 14-bit control change##sgse0"].plurals[0] = "Использовать 14-битную смену значения контроллера";
    strings["Use single control change (imprecise)##sgse0"].plurals[0] = "Использовать однократную смену значения контроллера (малая точность)";
    strings["Disabled/custom##sgse1"].plurals[0] = "Выкл./пользовательский";
    strings["Use dual control change (one for each nibble)##sgse1"].plurals[0] = "Использовать двойную смену значения контроллера (по одному на тетраду)";
    strings["Use 14-bit control change##sgse1"].plurals[0] = "Использовать 14-битную смену значения контроллера";
    strings["Use single control change (imprecise)##sgse1"].plurals[0] = "Использовать однократную смену значения контроллера (малая точность)";
    strings["--select--##sgse"].plurals[0] = "--выберите--";
    strings["Note Off##sgse"].plurals[0] = "Отпускание клавиши";
    strings["Note On##sgse"].plurals[0] = "Нажатие клавиши";
    strings["Aftertouch##sgse"].plurals[0] = "Давление на клавишу после нажатия";
    strings["Control##sgse"].plurals[0] = "Значение контроллера";
    strings["Program##sgse0"].plurals[0] = "Программа";
    strings["ChanPressure##sgse"].plurals[0] = "Одинаковое давление на все нажатые клавиши в одном канале";
    strings["Pitch Bend##sgse"].plurals[0] = "Смена высоты тона";
    strings["SysEx##sgse"].plurals[0] = "SysEx";
    strings["Instrument##sgse0"].plurals[0] = "Инструмент";
    strings["Volume##sgse0"].plurals[0] = "Громкость";
    strings["Effect 1 type##sgse"].plurals[0] = "Индекс эффекта №1";
    strings["Effect 1 value##sgse"].plurals[0] = "Параметр эффекта №1";
    strings["Effect 2 type##sgse"].plurals[0] = "Индекс эффекта №2";
    strings["Effect 2 value##sgse"].plurals[0] = "Параметр эффекта №2";
    strings["Effect 3 type##sgse"].plurals[0] = "Индекс эффекта №3";
    strings["Effect 3 value##sgse"].plurals[0] = "Параметр эффекта №3";
    strings["Effect 4 type##sgse"].plurals[0] = "Индекс эффекта №4";
    strings["Effect 4 value##sgse"].plurals[0] = "Параметр эффекта №4";
    strings["Effect 5 type##sgse"].plurals[0] = "Индекс эффекта №5";
    strings["Effect 5 value##sgse"].plurals[0] = "Параметр эффекта №5";
    strings["Effect 6 type##sgse"].plurals[0] = "Индекс эффекта №6";
    strings["Effect 6 value##sgse"].plurals[0] = "Параметр эффекта №6";
    strings["Effect 7 type##sgse"].plurals[0] = "Индекс эффекта №7";
    strings["Effect 7 value##sgse"].plurals[0] = "Параметр эффекта №7";
    strings["Effect 8 type##sgse"].plurals[0] = "Индекс эффекта №8";
    strings["Effect 8 value##sgse"].plurals[0] = "Параметр эффекта №8";

    strings["Press key...##sgse"].plurals[0] = "Нажмите клавишу...";
    strings["Settings###Settings"].plurals[0] = "Настройки###Settings";
    strings["Do you want to save your settings?##sgse"].plurals[0] = "Вы хотите сохранить свои настройки?";

    strings["General##sgse"].plurals[0] = "Основные";
    strings["Program##sgse1"].plurals[0] = "Программа";
    strings["Render backend##sgse"].plurals[0] = "Библиотека отрисовки";
    strings["Software##sgse"].plurals[0] = "Программная отрисовка";
    strings["Software"].plurals[0] = "Программная отрисовка"; //sigh
    strings["you may need to restart Furnace for this setting to take effect.##sgse0"].plurals[0] = "возможно, вам потребуется перезапустить Furnace, чтобы эта настрока применилась.";
    strings["Advanced render backend settings##sgse"].plurals[0] = "Продвинутые настройки библиотеки отрисовки";
    strings["beware: changing these settings may render Furnace unusable! do so at your own risk.\nstart Furnace with -safemode if you mess something up.##sgse"].plurals[0] = "внимание: изменение этих настроек может нарушить работу Furnace! изменяйте их на свой страх и риск.\nвы можете перезапустить Furnace с опцией -safemode, если вы что-то сломали.";
    strings["Render driver##sgse"].plurals[0] = "Драйвер отрисовки";
    strings["Automatic##sgse0"].plurals[0] = "Выбирать автоматически";
    strings["Automatic##sgse1"].plurals[0] = "Выбирать автоматически";
    strings["you may need to restart Furnace for this setting to take effect.##sgse1"].plurals[0] = "возможно, вам потребуется перезапустить Furnace, чтобы эта настрока применилась.";
    strings["Red bits##sgse"].plurals[0] = "Биты красной составляющей";
    strings["Green bits##sgse"].plurals[0] = "Биты зелёной составляющей";
    strings["Blue bits##sgse"].plurals[0] = "Биты синей составляющей";
    strings["Alpha bits##sgse"].plurals[0] = "Биты составляющей прозрачности";
    strings["Color depth##sgse"].plurals[0] = "Глубина цвета";
    strings["Stencil buffer size##sgse"].plurals[0] = "Размер трафаретного буфера";
    strings["Buffer size##sgse"].plurals[0] = "Размер буфера";
    strings["Double buffer##sgse"].plurals[0] = "Двойная буферизация";
    strings["the following values are common (in red, green, blue, alpha order):\n- 24 bits: 8, 8, 8, 0\n- 16 bits: 5, 6, 5, 0\n- 32 bits (with alpha): 8, 8, 8, 8\n- 30 bits (deep): 10, 10, 10, 0##sgse"].plurals[0] = "распространёнными являются следующие значения (в порядке красный, зелёный, синий, прозрачность):\n- 24 бита: 8, 8, 8, 0\n- 16 бит: 5, 6, 5, 0\n- 32 бита (с прозрачностью): 8, 8, 8, 8\n- 30 бит (глубокое квантование цвета): 10, 10, 10, 0";
    strings["nothing to configure##sgse"].plurals[0] = "нет настроек";
    strings["current backend: %s\n%s\n%s\n%s##sgse"].plurals[0] = "текущая библиотека отрисовки: %s\n%s\n%s\n%s";
    strings["VSync##sgse"].plurals[0] = "Вертикальная синхронизация";
    strings["Frame rate limit##sgse"].plurals[0] = "Ограничение частоты кадров";
    strings["only applies when VSync is disabled.##sgse"].plurals[0] = "применяется только при отключённой вертикальной синхронизации.";
    strings["Display render time##sgse"].plurals[0] = "Отображать время отрисовки";
    strings["Late render clear##sgse"].plurals[0] = "Запаздывающая очистка буфера отрисовщика";
    strings["calls rend->clear() after rend->present(). might reduce UI latency by one frame in some drivers.##sgse"].plurals[0] = "вызывает rend->clear() после rend->present(). может устранить запаздывание отрисовки интерфейса на один кадр для некоторых драйверов.";
    strings["Power-saving mode##sgse"].plurals[0] = "Режим энергосбережения";
    strings["saves power by lowering the frame rate to 2fps when idle.\nmay cause issues under Mesa drivers!##sgse"].plurals[0] = "уменьшает энергопотребление при помощи уменьшения частоты отрисовки до двух кадров в секунду в режиме ожидания.\nможет приводить к проблемам на драйверах Mesa!";
    strings["Disable threaded input (restart after changing!)##sgse"].plurals[0] = "Отключить обработку нажатий для превью инструмента в отдельном потоке (перезагрузите программу после изменения!)";
    strings["threaded input processes key presses for note preview on a separate thread (on supported platforms), which reduces latency.\nhowever, crashes have been reported when threaded input is on. enable this option if that is the case.##sgse"].plurals[0] = "обработка нажатий клавиш для превью инструмента происходит в отдельном потоке (на поддерживаемых платформах), что позволяет уменьшить задержку ввода.\nтем не менее, есть сообщения о вылетах программы при выключённой настройке. включите её, если у вас программа вылетает.";
    strings["Enable event delay##sgse"].plurals[0] = "Включить задержку событий";
    strings["may cause issues with high-polling-rate mice when previewing notes.##sgse"].plurals[0] = "может привести к проблемам во время превью инструмента, если подключена мышь с большой частотой обновления.";
    strings["Per-channel oscilloscope threads##sgse"].plurals[0] = "Потоки исполнения осциллографов для отдельных каналов";
    strings["you're being silly, aren't you? that's enough.##sgse"].plurals[0] = "может, хватит уже хернёй страдать? этого достаточно.";
    strings["what are you doing? stop!##sgse"].plurals[0] = "ты чё делаешь? хватит!";
    strings["it is a bad idea to set this number higher than your CPU core count (%d)!##sgse"].plurals[0] = "не рекомендуется выставлять здесь значение, большее количества ядер вашего ЦП (%d)!";
    strings["Oscilloscope rendering engine:##sgse"].plurals[0] = "Движок отрисовки осциллографов:";
    strings["ImGui line plot##sgse"].plurals[0] = "Отрисовка линий от ImGui";
    strings["render using Dear ImGui's built-in line drawing functions.##sgse"].plurals[0] = "отрисовывать при помощи встроенных функций отрисовки линий Dear ImGui.";
    strings["GLSL (if available)##sgse"].plurals[0] = "GLSL (при наличии)";
    strings["render using shaders that run on the graphics card.\nonly available in OpenGL ES 2.0 render backend.##sgse"].plurals[0] = "отрисовывать при помощи шейдеров, исполняемых на ГП.\nработает только при выборе OpenGL ES 2.0 в качестве библиотеки отрисовки.";
    strings["render using shaders that run on the graphics card.\nonly available in OpenGL 3.0 render backend.##sgse"].plurals[0] = "отрисовывать при помощи шейдеров, исполняемых на ГП.\nработает только при выборе OpenGL 3.0 в качестве библиотеки отрисовки.";
    strings["File##sgse"].plurals[0] = "Файл";
    strings["Vibration##sgse"].plurals[0] = "Вибрация";
    strings["Strength##sgse"].plurals[0] = "Сила вибрации";
    strings["Length##sgse"].plurals[0] = "Длина вибрации";
    strings["Use system file picker##sgse"].plurals[0] = "Использовать диалоговое окно выбора файлов ОС";
    strings["Number of recent files##sgse"].plurals[0] = "Количество недавних файлов";
    strings["Compress when saving##sgse"].plurals[0] = "Сжимать сохраняемые файлы";
    strings["use zlib to compress saved songs.##sgse"].plurals[0] = "использовать библиотеку zlib для сжатия сохраняемых модулей.";
    strings["Save unused patterns##sgse"].plurals[0] = "Сохранять неиспользуемые паттерны";
    strings["Use new pattern format when saving##sgse"].plurals[0] = "Использовать новый формат сохранения паттернов";
    strings["use a packed format which saves space when saving songs.\ndisable if you need compatibility with older Furnace and/or tools\nwhich do not support this format.##sgse"].plurals[0] = "использовать сжатый формат сохранения паттернов, что позволяет уменьшить размер файла с модулем.\nотключите, если нужна совместимость со старыми версиями Furnace и/или другими программами,\nкоторые не поддерживают новый формат.";
    strings["Don't apply compatibility flags when loading .dmf##sgse"].plurals[0] = "Не применять флаги совместимости при загрузке .dmf";
    strings["do not report any issues arising from the use of this option!##sgse"].plurals[0] = "не жалуйтесь на проблемы, которые возникнут после включения этой настройки!";
    strings["Play after opening song:##sgse"].plurals[0] = "Проигрывание модуля после его загрузки:";
    strings["No##pol0"].plurals[0] = "Нет##pol0";
    strings["Only if already playing##pol1"].plurals[0] = "Только если до этого уже играл##pol1";
    strings["Yes##pol0"].plurals[0] = "Да##pol0";
    strings["Audio export loop/fade out time:##sgse"].plurals[0] = "Количество циклов проигрывания и время затухания при экспорте аудио:";
    strings["Set to these values on start-up:##fot0"].plurals[0] = "Выставить эти значения при запуске:##fot0";
    strings["Loops##sgse"].plurals[0] = "Циклы";
    strings["Fade out (seconds)##sgse"].plurals[0] = "Затухание (в секундах)";
    strings["Remember last values##fot1"].plurals[0] = "Запоминать предыдущие значения##fot1";
    strings["Store instrument name in .fui##sgse"].plurals[0] = "Сохранять название инструмента в файле .fui";
    strings["when enabled, saving an instrument will store its name.\nthis may increase file size.##sgse"].plurals[0] = "При включении имя инструмента будет сохраняться в файле.\nэто может увеличить размер файла.";
    strings["Load instrument name from .fui##sgse"].plurals[0] = "Загружать имя инструмента из файла .fui";
    strings["when enabled, loading an instrument will use the stored name (if present).\notherwise, it will use the file name.##sgse"].plurals[0] = "При включении имя инструмента будет загружаться из файла (при наличии имени в файле).\nВ противном случае будет использоваться имя файла.";
    strings["Auto-fill file name when saving##sggu"].plurals[0] = "Автоматически подставлять имя файла при сохранении";
    strings["fill the file name field with an appropriate file name when saving or exporting.##sggu"].plurals[0] = "подставлять соответствующее имя файла при экспорте или сохранении.";
    strings["New Song##sgse"].plurals[0] = "Новая композиция";
    strings["Initial system:##sgse"].plurals[0] = "Система по умолчанию:";
    strings["Current system##sgse"].plurals[0] = "Текущая";
    strings["Randomize##sgse"].plurals[0] = "Выбрать случайно";
    strings["Reset to defaults##sgse"].plurals[0] = "Устан. по умолчанию";
    strings["Name##sgse"].plurals[0] = "Название";
    strings["Invert##sgse0"].plurals[0] = "Обр.";
    strings["Invert##sgse1"].plurals[0] = "Обр.";
    strings["Volume##sgse1"].plurals[0] = "Громкость";
    strings["Panning##sgse"].plurals[0] = "Панорамирование";
    strings["Front/Rear##sgse"].plurals[0] = "Передн./задн.";
    strings["Configure##sgse"].plurals[0] = "Настроить";
    strings["When creating new song:##sgse"].plurals[0] = "При создании новой композиции:";
    strings["Display system preset selector##NSB0"].plurals[0] = "Отобразить окно выбора пресета системы##NSB0";
    strings["Start with initial system##NSB1"].plurals[0] = "Начать с системы по умолчанию##NSB1";
    strings["Default author name##sgse"].plurals[0] = "Имя автора по умолчанию";
    strings["Start-up##sgse"].plurals[0] = "Запуск";
    strings["Disable fade-in during start-up##sgse"].plurals[0] = "Отключить плавное появление интерфейса при запуске";
    strings["About screen party time##sgse"].plurals[0] = "Вечеринка на экране \"О программе\"";
    strings["Warning: may cause epileptic seizures.##sgse"].plurals[0] = "Внимание: может вызвать эпилептические приступы.";
    strings["Behavior##sgse"].plurals[0] = "Поведение программы";
    strings["New instruments are blank##sgse"].plurals[0] = "Пустые новые инструменты";
    strings["Language##sgse"].plurals[0] = "Язык";
    strings["GUI language##sgse"].plurals[0] = "Язык интерфейса";
    strings["Translate channel names in pattern header##sgse"].plurals[0] = "Переводить имена каналов в заголовках паттернов";
    strings["Translate channel names in channel oscilloscope label##sgse"].plurals[0] = "Переводить имена каналов в надписях на осциллографах отдельных каналов";
    strings["Translate short channel names (in orders and other places)##sgse"].plurals[0] = "Переводить короткие имена каналов (в матрице паттернов и других местах)";
    strings["Configuration##sgse0"].plurals[0] = "Настройки программы";
    strings["Import##sgse"].plurals[0] = "Импорт";
    strings["Export##sgse"].plurals[0] = "Экспорт";
    strings["Factory Reset##sgse"].plurals[0] = "Сброс на заводские настройки";
    strings["Are you sure you want to reset all Furnace settings?\nYou must restart Furnace after doing so.##sgse"].plurals[0] = "Вы уверены, что хотите сбросить все настройки Furnace?\nВам нужно будет перезапустить Furnace после этого.";
    strings["Audio##sgse"].plurals[0] = "Аудио";
    strings["Output##sgse"].plurals[0] = "Вывод";
    strings["Backend##sgse"].plurals[0] = "Интерфейс";
    strings["Driver##sgse"].plurals[0] = "Драйвер";
    strings["Automatic##sgse2"].plurals[0] = "Автоматически";
    strings["you may need to restart Furnace for this setting to take effect.##sgse2"].plurals[0] = "возможно, вам придётся перезапустить Furnace для применения настройки.";
    strings["Device##sgse"].plurals[0] = "Устройство вывода";
    strings["<click on OK or Apply first>##sgse"].plurals[0] = "<сначала нажмите на кнопки \"ОК\" или \"Применить\">";
    strings["ALERT - TRESPASSER DETECTED##sgse"].plurals[0] = "ВНИМАНИЕ - ОБНАРУЖЕН НАРУШИТЕЛЬ";
    strings["you have been arrested for trying to engage with a disabled combo box.##sgse"].plurals[0] = "вы были арестованы за попытку взаимодействия с выключенным выпадающим списком.";
    strings["<System default>##sgse0"].plurals[0] = "<По умолчанию>";
    strings["<System default>##sgse1"].plurals[0] = "<По умолчанию>";
    strings["Sample rate##sgse"].plurals[0] = "Частота дискретизации";
    strings["Outputs##sgse"].plurals[0] = "Выводы";
    strings["common values:\n- 1 for mono\n- 2 for stereo\n- 4 for quadraphonic\n- 6 for 5.1 surround\n- 8 for 7.1 surround##sgse"].plurals[0] = "распространённые значения:\n- 1 для моно\n- 2 для стерео\n- 4 для квадрафонического звука\n- 6 для 5.1 объёмного звука\n- 8 для 7.1 объёмного звука";
    strings["Channels##sgse"].plurals[0] = "Число каналов";
    strings["What?##sgse3"].plurals[0] = "Что?";
    strings["Buffer size##sgse"].plurals[0] = "Размер буфера";
    strings["%d (latency: ~%.1fms)##sgse"].plurals[0] = "%d (задержка: ~%.1f мс)";
    strings["Multi-threaded (EXPERIMENTAL)##sgse"].plurals[0] = "Многопоточность (ЭКСПЕРИМЕНТАЛЬНАЯ)";
    strings["runs chip emulation on separate threads.\nmay increase performance when using heavy emulation cores.\n\nwarnings:\n- experimental!\n- only useful on multi-chip songs.##sgse"].plurals[0] = "исполняет эмуляторы чипов в отдельных потоках.\nможет повысить производительность при использовании тяжёлых эмуляторов.\n\nвнимание:\n- экспериментальная функция!\n- полезна только для композиций, использующих несколько чипов.";
    strings["Number of threads##sgse"].plurals[0] = "Количество потоков";
    strings["that's the limit!##sgse"].plurals[0] = "это предел!";
    strings["it is a VERY bad idea to set this number higher than your CPU core count (%d)!##sgse"].plurals[0] = "это ОЧЕНЬ плохая идея - устанавливать это значение большим, чем колчество ядер ЦП (%d)!";
    strings["Low-latency mode##sgse"].plurals[0] = "Режим малой задержки";
    strings["reduces latency by running the engine faster than the tick rate.\nuseful for live playback/jam mode.\n\nwarning: only enable if your buffer size is small (10ms or less).##sgse"].plurals[0] = "уменьшает задержку, исполняя код движка трекера быстрее, чем указано в настройках.\nполезно для режима исполнения в реальном времени.\n\nвнимание: включайте только если размер вашего аудиобуфера мал (10 мс или меньше).";
    strings["Force mono audio##sgse"].plurals[0] = "Принудительно сводить в моно";
    strings["Exclusive mode##sgse"].plurals[0] = "Исключительный режим";
    strings["want: %d samples @ %.0fHz (%d %s)##sgse"].plurals[0] = "запрошено: %d сэмплов @ %.0f Гц (%d %s)";
    strings["channel##sgse"].plurals[0] = "канал";
    strings["channel##sgse"].plurals[1] = "канала";
    strings["channel##sgse"].plurals[2] = "каналов";
    strings["got: %d samples @ %.0fHz (%d %s)##sgse"].plurals[0] = "получено: %d сэмплов @ %.0f Гц (%d %s)";
    strings["Mixing##sgse"].plurals[0] = "Микширование";
    strings["Quality##sgse"].plurals[0] = "Качество";
    strings["Software clipping##sgse"].plurals[0] = "Программное ограничение сигнала";
    strings["DC offset correction##sgse"].plurals[0] = "Коррекция смещения пост. составляющей";
    strings["Metronome##sgse"].plurals[0] = "Метроном";
    strings["Volume##sgse2"].plurals[0] = "Громкость метронома";
    strings["Sample preview##sgse"].plurals[0] = "Превью сэмпла";
    strings["Volume##sgse3"].plurals[0] = "Громкость";
    strings["MIDI##sgse"].plurals[0] = "MIDI";
    strings["MIDI input##sgse0"].plurals[0] = "Ввод MIDI";
    strings["MIDI input##sgse1"].plurals[0] = "MIDI вход";
    strings["<disabled>##sgse0"].plurals[0] = "<выкл.>";
    strings["<disabled>##sgse1"].plurals[0] = "<выкл.>";
    strings["Re-scan MIDI devices##sgse"].plurals[0] = "Перезап. скан. для обнаруж. MIDI-устройств";
    strings["Note input##sgse0"].plurals[0] = "Ввод нот";
    strings["Velocity input##sgse"].plurals[0] = "Ввод скорости нажатия";
    strings["Map MIDI channels to direct channels##sgse"].plurals[0] = "Привязать MIDI-каналы к прямым каналам";
    strings["Program change pass-through##sgse"].plurals[0] = "Пропускать на выход сообщения об изменении программы";
    strings["Map Yamaha FM voice data to instruments##sgse"].plurals[0] = "Привязать данные тембров Yamaha FM к инструментам";
    strings["Program change is instrument selection##sgse"].plurals[0] = "Смена программы = выбор инструмента";
    strings["Listen to MIDI clock##sgse"].plurals[0] = "Следить за тактовой частотой MIDI";
    strings["Listen to MIDI time code##sgse"].plurals[0] = "Следить за временным кодом MIDI";
    strings["Value input style##sgse0"].plurals[0] = "Стиль ввода значений";
    strings["Value input style##sgse1"].plurals[0] = "Стиль ввода значений";
    strings["Control##valueCCS"].plurals[0] = "Управление##valueCCS";
    strings["CC of upper nibble##valueCC1"].plurals[0] = "Управляющая команда для верхней тетрады##valueCC1";
    strings["MSB CC##valueCC1"].plurals[0] = "Управл. ком. для старш. бита##valueCC1";
    strings["CC of lower nibble##valueCC2"].plurals[0] = "Управляющая команда для нижней тетрады##valueCC2";
    strings["LSB CC##valueCC2"].plurals[0] = "Управл. ком. для младш. бита##valueCC2";
    strings["Per-column control change##sgse"].plurals[0] = "Изменение управления для каждого столбца";
    strings["Control##valueCCS"].plurals[0] = "Управление##valueCCS";
    strings["CC of upper nibble##valueCC1"].plurals[0] = "Управляющая команда для верхней тетрады##valueCC1";
    strings["MSB CC##valueCC1"].plurals[0] = "Управл. ком. для старш. бита##valueCC1";
    strings["CC of lower nibble##valueCC2"].plurals[0] = "Управляющая команда для нижней тетрады##valueCC2";
    strings["LSB CC##valueCC2"].plurals[0] = "Управл. ком. для младш. бита##valueCC2";
    strings["Volume curve##sgse0"].plurals[0] = "Кривая громкости";
    strings["Volume curve##sgse1"].plurals[0] = "Кривая громкости";
    strings["Actions:##sgse"].plurals[0] = "Действия:";
    strings["(learning! press a button or move a slider/knob/something on your device.)##sgse"].plurals[0] = "(обучение! нажмите кнопку или подвигайте что-то на вашем устройстве.)";
    strings["Type##sgse0"].plurals[0] = "Тип";
    strings["Channel##sgse0"].plurals[0] = "Канал";
    strings["Note/Control##sgse"].plurals[0] = "Нота/управление";
    strings["Velocity/Value##sgse"].plurals[0] = "Скор. наж./парам.";
    strings["Action##sgse"].plurals[0] = "Действие";
    strings["Any##sgse0"].plurals[0] = "Любой";
    strings["Any##sgse1"].plurals[0] = "Любая";
    strings["Any##sgse2"].plurals[0] = "Любая";
    strings["Any##sgse3"].plurals[0] = "Любое";
    strings["--none--##sgse"].plurals[0] = "--нет--";
    strings["waiting...##BLearn"].plurals[0] = "ожидание...##BLearn";
    strings["Learn##BLearn"].plurals[0] = "Запомнить##BLearn";
    strings["MIDI output##sgse0"].plurals[0] = "MIDI вывод";
    strings["MIDI output##sgse1"].plurals[0] = "Устройство вывода MIDI";
    strings["<disabled>##sgse2"].plurals[0] = "<выкл.>";
    strings["<disabled>##sgse3"].plurals[0] = "<выкл.>";
    strings["Output mode:##sgse"].plurals[0] = "Режим вывода:";
    strings["Off (use for TX81Z)##sgse"].plurals[0] = "Выкл. (используйте для TX81Z)";
    strings["Melodic##sgse"].plurals[0] = "Мелодия";
    //strings["Light Show (use for Launchpad)##sgse"].plurals[0] = "Light Show (use for Launchpad)";
    strings["Send Program Change##sgse"].plurals[0] = "Посылать команду изменения программы";
    strings["Send MIDI clock##sgse"].plurals[0] = "Посылать тактовую частоту MIDI";
    strings["Send MIDI timecode##sgse"].plurals[0] = "Посылать временной код MIDI";
    strings["Timecode frame rate:##sgse"].plurals[0] = "Частота посылок временного кода:";
    strings["Closest to Tick Rate##sgse"].plurals[0] = "Ближайшая к частоте движка трекера";
    strings["Film (24fps)##sgse"].plurals[0] = "Киноплёнка (24 кадра в секунду)";
    strings["PAL (25fps)##sgse"].plurals[0] = "PAL (25 кадров в секунду)";
    strings["NTSC drop (29.97fps)##sgse"].plurals[0] = "NTSC с выпадением кадров (29.97 кадров в секунду)";
    strings["NTSC non-drop (30fps)##sgse"].plurals[0] = "NTSC без выпадения кадров (30 кадров в секунду)";
    strings["Emulation##sgse"].plurals[0] = "Эмуляция";
    strings["Cores##sgse"].plurals[0] = "Ядра эмуляции";
    strings["System##sgse"].plurals[0] = "Система/чип";
    strings["Playback Core(s)##sgse"].plurals[0] = "Ядро(-а) воспроизведения";
    strings["used for playback##sgse"].plurals[0] = "используется(-ются) для воспроизведения";
    strings["Render Core(s)##sgse"].plurals[0] = "Ядро(-а) рендера";
    strings["used in audio export##sgse"].plurals[0] = "используется(-ются) при рендере звука в аудиофайл";
    strings["Quality##sgse1"].plurals[0] = "Качество";
    strings["Playback##sgse"].plurals[0] = "Воспроизведение";
    strings["Render##sgse"].plurals[0] = "Рендер";
    strings["Other##sgse"].plurals[0] = "Разное";
    strings["PC Speaker strategy##sgse"].plurals[0] = "Взаимодействие с PC Speaker";
    strings["Sample ROMs:##sgse"].plurals[0] = "Образы ROM сэмплов:";
    strings["OPL4 YRW801 path##sgse"].plurals[0] = "Путь к OPL4 YRW801";
    strings["MultiPCM TG100 path##sgse"].plurals[0] = "Путь к MultiPCM TG100";
    strings["MultiPCM MU5 path##sgse"].plurals[0] = "Путь к MultiPCM MU5";
    strings["Keyboard##sgse0"].plurals[0] = "Клавиатура";
    strings["Keyboard##sgse1"].plurals[0] = "Привязка клавиш";
    strings["Import##sgse0"].plurals[0] = "Импорт";
    strings["Export##sgse0"].plurals[0] = "Экспорт";
    strings["Reset defaults##sgse0"].plurals[0] = "Сбросить до настроек по умолчанию";
    strings["Are you sure you want to reset the keyboard settings?##sgse"].plurals[0] = "Вы действительно хотите сбросить настройки привязки клавиш?";
    strings["Global hotkeys##sgse"].plurals[0] = "Глобальные горячие клавиши";
    strings["Window activation##sgse"].plurals[0] = "Активация окон";
    strings["Note input##sgse1"].plurals[0] = "Ввод нот";
    strings["Key##sgse"].plurals[0] = "Клавиша";
    strings["Type##sgse1"].plurals[0] = "Тип";
    strings["Value##sgse"].plurals[0] = "Значение";
    strings["Remove##sgse"].plurals[0] = "Убрать";
    strings["Macro release##SNType_%d"].plurals[0] = "\"Отпускание клавиши\" (только для макросов)##SNType_%d";
    strings["Note release##SNType_%d"].plurals[0] = "\"Отпускание клавиши\" (с включением фазы затухания огибающей)##SNType_%d";
    strings["Note off##SNType_%d"].plurals[0] = "\"Отпускание клавиши\" (резкое)##SNType_%d";
    strings["Note##SNType_%d"].plurals[0] = "Нота##SNType_%d";
    strings["Add...##sgse"].plurals[0] = "Добавить...";
    strings["Pattern##sgse0"].plurals[0] = "Паттерн";
    strings["Instrument list##sgse"].plurals[0] = "Список инструментов";
    strings["Wavetable list##sgse"].plurals[0] = "Список волновых таблиц";
    strings["Local wavetables list##sgse"].plurals[0] = "Список локальных волновых таблиц";
    strings["Sample list##sgse"].plurals[0] = "Список сэмплов";
    strings["Orders##sgse0"].plurals[0] = "Матрица паттернов";
    strings["Sample editor##sgse"].plurals[0] = "Редактор сэмплов";
    strings["Interface##sgse0"].plurals[0] = "Интерфейс";
    strings["Layout##sgse"].plurals[0] = "Компоновка интерфейса";
    strings["Workspace layout:##sgse"].plurals[0] = "Расположение окон интерфейса:";
    strings["Import##sgse1"].plurals[0] = "Импорт";
    strings["Export##sgse1"].plurals[0] = "Экспорт";
    strings["Reset##sgse"].plurals[0] = "Сбросить";
    strings["Are you sure you want to reset the workspace layout?##sgse"].plurals[0] = "Вы действительно хотите сбросить компоновку окон интерфейса?";
    strings["Allow docking editors##sgse"].plurals[0] = "Разрешить стыковать окна редакторов";
    strings["Remember window position##sgse"].plurals[0] = "Запоминать положение окон";
    strings["remembers the window's last position on start-up.##sgse"].plurals[0] = "при запуске программы восстанавливает последнее положение каждого окна.";
    strings["Only allow window movement when clicking on title bar##sgse"].plurals[0] = "Разрешать перемещение окон только при нажатии на их полосу заголовка";
    strings["Center pop-up windows##sgse"].plurals[0] = "Центрировать всплывающие окна";
    strings["Play/edit controls layout:##sgse"].plurals[0] = "Компоновка управления воспроизведением/редактированием:";
    strings["Classic##ecl0"].plurals[0] = "Классическая##ecl0";
    strings["Compact##ecl1"].plurals[0] = "Компактная##ecl1";
    strings["Compact (vertical)##ecl2"].plurals[0] = "Компактная (вертикальная)##ecl2";
    strings["Split##ecl3"].plurals[0] = "С разделением на два окна##ecl3";
    strings["Position of buttons in Orders:##sgse"].plurals[0] = "Позиция кнопок в редакторе матрицы паттернов:";
    strings["Top##obp0"].plurals[0] = "Сверху##obp0";
    strings["Left##obp1"].plurals[0] = "Слева##obp1";
    strings["Right##obp2"].plurals[0] = "Справа##obp2";
    strings["Mouse##sgse"].plurals[0] = "Мышь";
    strings["Double-click time (seconds)##sgse"].plurals[0] = "Время двойного нажатия (в секундах)";
    strings["Don't raise pattern editor on click##sgse"].plurals[0] = "Не поднимать редактор паттернов при нажатии";
    strings["Focus pattern editor when selecting instrument##sgse"].plurals[0] = "Переместить фокус на редактор паттернов при выборе инструмента";
    strings["Note preview behavior:##sgse"].plurals[0] = "Превью нот:";
    strings["Never##npb0"].plurals[0] = "Никогда##npb0";
    strings["When cursor is in Note column##npb1"].plurals[0] = "Когда курсор находится в столбце нот##npb1";
    strings["When cursor is in Note column or not in edit mode##npb2"].plurals[0] = "Когда курсор находится в столбце нот или не включён режим редактирования##npb2";
    strings["Always##npb3"].plurals[0] = "Всегда##npb3";
    strings["Allow dragging selection:##sgse"].plurals[0] = "Разрешить перемещение выделенного фрагмента:";
    strings["No##dms0"].plurals[0] = "Нет##dms0";
    strings["Yes##dms1"].plurals[0] = "Да##dms1";
    strings["Yes (while holding Ctrl only)##dms2"].plurals[0] = "Да (только при нажатой клавише Ctrl)##dms2";
    strings["Toggle channel solo on:##sgse"].plurals[0] = "Включать режим соло для канала:";
    strings["Right-click or double-click##soloA"].plurals[0] = "ПКМ или двойное нажатие##soloA";
    strings["Right-click##soloR"].plurals[0] = "ПКМ##soloR";
    strings["Double-click##soloD"].plurals[0] = "Двойное нажатие##soloD";
    strings["Double click selects entire column##sgse"].plurals[0] = "Двойное нажатие выделяет весь столбец";
    strings["Cursor behavior##sgse"].plurals[0] = "Поведение курсора";
    strings["Insert pushes entire channel row##sgse"].plurals[0] = "Клавиша Insert сдвигает паттерны на всех каналах";
    strings["Pull delete affects entire channel row##sgse"].plurals[0] = "Удаление с подтягиванием следующих строк сдвигает паттерны на всех каналах";
    strings["Push value when overwriting instead of clearing it##sgse"].plurals[0] = "Отодвинуть значение ячейки в соседнюю вместо удаления при перезаписи ячейки";
    strings["Keyboard note/value input repeat (hold key to input continuously)##sgse"].plurals[0] = "Повторяющийся ввод нот/значений с клавиатуры (удерживайте клавишу для постоянного ввода";
    strings["Effect input behavior:##sgse"].plurals[0] = "Ввод эффектов:";
    strings["Move down##eicb0"].plurals[0] = "перемещаться вниз##eicb0";
    strings["Move to effect value (otherwise move down)##eicb1"].plurals[0] = "Перепрыгнуть на параметр эффекта (иначе перемещаться вниз)##eicb1";
    strings["Move to effect value/next effect and wrap around##eicb2"].plurals[0] = "Перепрыгнуть на параметр эффекта/следующий эффект, в конце строки перепрыгнуть в начало строки эффектов##eicb2";
    strings["Delete effect value when deleting effect##sgse"].plurals[0] = "Удалять параметр эффекта при удалении эффекта";
    strings["Change current instrument when changing instrument column (absorb)##sgse"].plurals[0] = "Изменять выделенный инструмент при редактировании столбца инструмента";
    strings["Remove instrument value when inserting note off/release##sgse"].plurals[0] = "Удалять значение столбца инструмента при вводе ноты OFF/===";
    strings["Remove volume value when inserting note off/release##sgse"].plurals[0] = "Удалять значение столбца громкости при вводе ноты OFF/===";
    strings["Cursor movement##sgse"].plurals[0] = "Передвижение курсора";
    strings["Wrap horizontally:##sgse"].plurals[0] = "Переносить по горизонтали:";
    strings["No##wrapH0"].plurals[0] = "Нет##wrapH0";
    strings["Yes##wrapH1"].plurals[0] = "Да##wrapH1";
    strings["Yes, and move to next/prev row##wrapH2"].plurals[0] = "Да, и переходить на следующую/предыдущую строку##wrapH2";
    strings["Wrap vertically:##sgse"].plurals[0] = "Переносить по вертикали:";
    strings["No##wrapV0"].plurals[0] = "Нет##wrapV0";
    strings["Yes##wrapV1"].plurals[0] = "Да##wrapV1";
    strings["Yes, and move to next/prev pattern##wrapV2"].plurals[0] = "Да, и переходить на следующий/предыдущий паттерн##wrapV2";
    strings["Yes, and move to next/prev pattern (wrap around)##wrapV2"].plurals[0] = "Да, и переходить на следующий/предыдущий паттерн (с переносом в начало/конец)##wrapV2";
    strings["Cursor movement keys behavior:##sgse"].plurals[0] = "Передвижение курсора при помощи клавиш:";
    strings["Move by one##cmk0"].plurals[0] = "Сдвигать на одну позицию##cmk0";
    strings["Move by Edit Step##cmk1"].plurals[0] = "Сдвигать на шаг редактирования##cmk1";
    strings["Move cursor by edit step on delete##sgse"].plurals[0] = "Сдвигать курсор на шаг редактирования при удалении";
    strings["Move cursor by edit step on insert (push)##sgse"].plurals[0] = "Сдвигать курсор на шаг редактирования при вставке";
    strings["Move cursor up on backspace-delete##sgse"].plurals[0] = "Сдвигать курсор вверх при удалении по нажатию Backspace";
    strings["Move cursor to end of clipboard content when pasting##sgse"].plurals[0] = "Сдвигать курсор в конец вставленного фрагмента при вставке";
    strings["Scrolling##sgse"].plurals[0] = "Прокрутка";
    strings["Change order when scrolling outside of pattern bounds:##sgse"].plurals[0] = "Менять положение в матрице паттернов при прокрутке за пределы паттернов:";
    strings["No##pscroll0"].plurals[0] = "Нет##pscroll0";
    strings["Yes##pscroll1"].plurals[0] = "Да##pscroll1";
    strings["Yes, and wrap around song##pscroll2"].plurals[0] = "Да, и переносить в начало/конец трека##pscroll2";
    strings["Cursor follows current order when moving it##sgse"].plurals[0] = "Курсор остаётся на строке матрицы паттернов при перемещении этой строки";
    strings["applies when playback is stopped.##sgse"].plurals[0] = "действительно только при остановленном воспроизведении.";
    strings["Don't scroll when moving cursor##sgse"].plurals[0] = "Не прокручивать при перемещении курсора";
    strings["Move cursor with scroll wheel:##sgse"].plurals[0] = "Перемещать курсор при помощи колёсика мыши:";
    strings["No##csw0"].plurals[0] = "Нет##csw0";
    strings["Yes##csw1"].plurals[0] = "Да##csw1";
    strings["Inverted##csw2"].plurals[0] = "Да, но в обратном направлении##csw2";
    strings["How many steps to move with each scroll wheel step?##sgse"].plurals[0] = "На сколько позиций свдигаться при каждом шаге колёсика мыши?";
    strings["One##cws0"].plurals[0] = "На одну##cws0";
    strings["Edit Step##cws1"].plurals[0] = "На шаг редактирования##cws1";
    strings["Assets##sgse0"].plurals[0] = "Ресурсы";
    strings["Display instrument type menu when adding instrument##sgse"].plurals[0] = "Отображать тип инструмента при добавлении инструмента";
    strings["Select asset after opening one##sgse"].plurals[0] = "Выделять объект после открытия";
    strings["Appearance##sgse"].plurals[0] = "Внешний вид";
    strings["Scaling##sgse"].plurals[0] = "Масштаб";
    strings["Automatic UI scaling factor##sgse"].plurals[0] = "Автоматическое масштабирование интерфейса";
    strings["UI scaling factor##sgse"].plurals[0] = "Масштаб интерфейса";
    strings["Icon size##sgse"].plurals[0] = "Размер иконок";
    strings["Text##sgse"].plurals[0] = "Текст";
    strings["Font renderer##sgse"].plurals[0] = "Отрисовщик шрифта";
    strings["Main font##sgse"].plurals[0] = "Основной шрифт";
    strings["Size##MainFontSize"].plurals[0] = "Размер##MainFontSize";
    strings["Header font##sgse"].plurals[0] = "Шрифт заголовков";
    strings["Size##HeadFontSize"].plurals[0] = "Размер##HeadFontSize";
    strings["Pattern font##sgse"].plurals[0] = "Шрифт паттернов";
    strings["Size##PatFontSize"].plurals[0] = "Размер##PatFontSize";
    strings["Anti-aliased fonts##sgse"].plurals[0] = "Сглаживание шрифтов";
    strings["Support bitmap fonts##sgse"].plurals[0] = "Поддерживать растровые шрифты";
    strings["Hinting:##sgse"].plurals[0] = "Хинтование";
    strings["Off (soft)##fh0"].plurals[0] = "Нет (слабое)##fh0";
    strings["Slight##fh1"].plurals[0] = "Небольшое##fh1";
    strings["Normal##fh2"].plurals[0] = "Нормальное##fh2";
    strings["Full (hard)##fh3"].plurals[0] = "Полное (жёсткое)##fh3";
    strings["Auto-hinter:##sgse"].plurals[0] = "Автоматическое хинтирование";
    strings["Disable##fah0"].plurals[0] = "Отключить##fah0";
    strings["Enable##fah1"].plurals[0] = "Включить##fah1";
    strings["Force##fah2"].plurals[0] = "Принудительное##fah2";
    strings["Oversample##sgse"].plurals[0] = "Супердискретизация";
    strings["saves video memory. reduces font rendering quality.\nuse for pixel/bitmap fonts.##sgse"].plurals[0] = "уменьшенное использование видеопамяти, ухудшенная отрисовка шрифтов.\nиспользуйте для пиксельных/растровых шрифтов.";
    strings["default.##sgse"].plurals[0] = "настройка по умолчанию.";
    strings["slightly better font rendering quality.\nuses more video memory.##sgse"].plurals[0] = "незначительно улучшенная отрисовка шрифтов.\nиспользует больше видеопамяти.";
    strings["Load fallback font##sgse"].plurals[0] = "Загружать резервный шрифт";
    strings["disable to save video memory.##sgse"].plurals[0] = "отключите, если хотите уменьшить использование видеопамяти.";
    strings["Display Japanese characters##sgse"].plurals[0] = "Отображать японские символы (вкл. иероглифы)";
    strings["Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "このオプションは、十分なグラフィックメモリがある場合にのみ切り替えてください。\n"
            "これは、Dear ImGuiにダイナミックフォントアトラスが実装されるまでの一時的な解決策です。##sgse"].plurals[0] = 
            
            "Включайте эту настройку только в случае наличия достаточного количества графической памяти.\n"
            "Это временное решение, поскольку пока Dear ImGui не поддерживает динамический атлас шрифтов.\n\n"
            "このオプションは、十分なグラフィックメモリがある場合にのみ切り替えてください。\n"
            "これは、Dear ImGuiにダイナミックフォントアトラスが実装されるまでの一時的な解決策です。";
    strings["Display Chinese (Simplified) characters##sgse"].plurals[0] = "Отображать китайские иероглифы (упрощённые)";
    strings["Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "请在确保你有足够的显存后再启动此设定\n"
            "这是一个在ImGui实现动态字体加载之前的临时解决方案##sgse"].plurals[0] = 
            
            "Включайте эту настройку только в случае наличия достаточного количества графической памяти.\n"
            "Это временное решение, поскольку пока Dear ImGui не поддерживает динамический атлас шрифтов.\n\n"
            "请在确保你有足够的显存后再启动此设定\n"
            "这是一个在ImGui实现动态字体加载之前的临时解决方案";
    strings["Display Chinese (Traditional) characters##sgse"].plurals[0] = "Отображать китайские иероглифы (традиционные)";
    strings["Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "請在確保你有足夠的顯存后再啟動此設定\n"
            "這是一個在ImGui實現動態字體加載之前的臨時解決方案##sgse"].plurals[0] = 
            
            "Включайте эту настройку только в случае наличия достаточного количества графической памяти.\n"
            "Это временное решение, поскольку пока Dear ImGui не поддерживает динамический атлас шрифтов.\n\n"
            "請在確保你有足夠的顯存后再啟動此設定\n"
            "這是一個在ImGui實現動態字體加載之前的臨時解決方案";
    strings["Display Korean characters##sgse"].plurals[0] = "Отображать корейские иероглифы";
    strings["Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "그래픽 메모리가 충분한 경우에만 이 옵션을 선택하십시오.\n"
            "이 옵션은 Dear ImGui에 동적 글꼴 아틀라스가 구현될 때까지 임시 솔루션입니다.##sgse"].plurals[0] = 
            
            "Включайте эту настройку только в случае наличия достаточного количества графической памяти.\n"
            "Это временное решение, поскольку пока Dear ImGui не поддерживает динамический атлас шрифтов.\n\n"
            "그래픽 메모리가 충분한 경우에만 이 옵션을 선택하십시오.\n"
            "이 옵션은 Dear ImGui에 동적 글꼴 아틀라스가 구현될 때까지 임시 솔루션입니다.";

    strings["Program##sgse2"].plurals[0] = "Программа";
    strings["Title bar:##sgse"].plurals[0] = "Полоса заголовка окна";
    strings["Furnace-B##tbar0"].plurals[0] = "Furnace-B##tbar0";
    strings["Song Name - Furnace-B##tbar1"].plurals[0] = "Название композиции - Furnace-B##tbar1";
    strings["file_name.fur - Furnace-B##tbar2"].plurals[0] = "название_файла.fur - Furnace-B##tbar2";
    strings["/path/to/file.fur - Furnace-B##tbar3"].plurals[0] = "/путь/к/файлу.fur - Furnace-B##tbar3";
    strings["Display system name on title bar##sgse"].plurals[0] = "Отображать название чипа/системы в полосе заголовка окна";
    strings["Display chip names instead of \"multi-system\" in title bar##sgse"].plurals[0] = "Отображать названия чипов/систем вместо \"мульти-система\" в полосе заголовка окна";
    strings["Status bar:##sgse"].plurals[0] = "Строка состояния:";
    strings["Cursor details##sbar0"].plurals[0] = "Информация о выделенном элементе##sbar0";
    strings["File path##sbar1"].plurals[0] = "Путь к файлу##sbar1";
    strings["Cursor details or file path##sbar2"].plurals[0] = "Информация о выделенном элементе или путь к файлу##sbar2";
    strings["Nothing##sbar3"].plurals[0] = "Ничего##sbar3";
    strings["Display playback status when playing##sgse"].plurals[0] = "Отображать статус проигрывания во время воспроизведения";
    strings["Export options layout:##sgse"].plurals[0] = "Вид настроек экспорта:";
    strings["Sub-menus in File menu##eol0"].plurals[0] = "Подпункты в меню \"Файл\"##eol0";
    strings["Modal window with tabs##eol1"].plurals[0] = "Модальное окно с вкладками##eol1";
    strings["Modal windows with options in File menu##eol2"].plurals[0] = "Модальное окно с настройками в меню \"Файл\"";
    strings["Capitalize menu bar##sgse"].plurals[0] = "Названия пунктов в горизонтальном меню с большой буквы";
    strings["Display add/configure/change/remove chip menus in File menu##sgse"].plurals[0] = "Отображать в меню \"Файл\" пункты: добавить/настроить/изменить/убрать чип";
    strings["Orders##sgse1"].plurals[0] = "Матрица паттернов";
    strings["Highlight channel at cursor in Orders##sgse"].plurals[0] = "Выделить в матрице паттернов канал, на котором находится курсор";
    strings["Orders row number format:##sgse"].plurals[0] = "Формат отображения номера строки матрицы паттернов:";
    strings["Decimal##orbD"].plurals[0] = "Десятеричный##orbD";
    strings["Hexadecimal##orbH"].plurals[0] = "Шестнадцатеричный##orbH";
    strings["Pattern##sgse1"].plurals[0] = "Паттерн";
    strings["Center pattern view##sgse"].plurals[0] = "Центрировать отображаемые паттерны внутри окна";
    strings["Overflow pattern highlights##sgse"].plurals[0] = "Продолжать полосы подсветки строк паттернов за пределы самих паттернов";
    strings["Display previous/next pattern##sgse"].plurals[0] = "Отображать предыдущий/следующий паттерн";
    strings["Pattern row number format:##sgse"].plurals[0] = "Формат отображения номера строки паттерна:";
    strings["Decimal##prbD"].plurals[0] = "Десятеричный##prbD";
    strings["Hexadecimal##prbH"].plurals[0] = "Шестнадцатеричный##prbH";
    strings["Pattern view labels:##sgse"].plurals[0] = "Маркировка ячеек в паттерне";
    strings["Note off (3-char)##sgse"].plurals[0] = "\"Отпускание клавиши\" (резкое) (3 символа)";
    strings["Note release (3-char)##sgse"].plurals[0] = "\"Отпускание клавиши\" (с включением фазы затухания огибающей) (3 символа)";
    strings["Macro release (3-char)##sgse"].plurals[0] = "\"Отпускание клавиши\" (только для макросов (3 символа))";
    strings["Empty field (3-char)##sgse"].plurals[0] = "Пустая ячейка (3 символа)";
    strings["Empty field (2-char)##sgse"].plurals[0] = "Пустая ячейка (2 символа)";
    strings["Pattern view spacing after:##sgse"].plurals[0] = "Разбивка в отображении паттерна:";
    strings["Note##sgse"].plurals[0] = "Нота";
    strings["Instrument##sgse1"].plurals[0] = "Инструмент";
    strings["Volume##sgse4"].plurals[0] = "Громкость";
    strings["Effect##sgse"].plurals[0] = "Индекс эффекта";
    strings["Effect value##sgse"].plurals[0] = "Параметр эффекта";
    strings["Single-digit effects for 00-0F##sgse"].plurals[0] = "Отображать одной цифрой индекс эффекта для индексов 00-0F";
    strings["Use flats instead of sharps##sgse"].plurals[0] = "Отображать бемоли вместо диезов";
    strings["Use German notation##sgse"].plurals[0] = "Использовать немецкие имена нот";
    strings["Channel##sgse1"].plurals[0] = "Канал";
    strings["Channel style:##sgse"].plurals[0] = "Стиль заголовка:";
    strings["Classic##CHS0"].plurals[0] = "Классический##CHS0";
    strings["Line##CHS1"].plurals[0] = "Линия##CHS1";
    strings["Round##CHS2"].plurals[0] = "Со скруглениями##CHS2";
    strings["Split button##CHS3"].plurals[0] = "С отдельной кнопкой отключения звука##CHS3";
    strings["Square border##CHS4"].plurals[0] = "С прямоугольной границей вокруг названия##CHS4";
    strings["Round border##CHS5"].plurals[0] = "Со скруглённой границей вокруг названия##CHS5";
    strings["Channel volume bar:##sgse"].plurals[0] = "Полоска громкости в заголовке канала:";
    strings["Non##CHV0"].plurals[0] = "Нет##CHV0";
    strings["Simple##CHV1"].plurals[0] = "Простая##CHV1";
    strings["Stereo##CHV2"].plurals[0] = "Стерео##CHV2";
    strings["Real##CHV3"].plurals[0] = "Настоящая громкость##CHV3";
    strings["Real (stereo)##CHV4"].plurals[0] = "Настоящая громкость (стерео)##CHV4";
    strings["Channel feedback style:##sgse"].plurals[0] = "Подсветка заголовка канала:";
    strings["Off##CHF0"].plurals[0] = "Выкл.##CHF0";
    strings["Note##CHF1"].plurals[0] = "Начало ноты##CHF1";
    strings["Volume##CHF2"].plurals[0] = "Пропорционально громкости##CHF2";
    strings["Active##CHF3"].plurals[0] = "При активности канала##CHF3";
    strings["Channel font:##sgse"].plurals[0] = "Шрифт заголовка канала:";
    strings["Regular##CHFont0"].plurals[0] = "Обычный##CHFont0";
    strings["Monospace##CHFont1"].plurals[0] = "Моноширинный##CHFont1";
    strings["Center channel name##sgse"].plurals[0] = "Центрировать название канала";
    strings["Channel colors:##sgse"].plurals[0] = "Цвета заголовка канала";
    strings["Single##CHC0"].plurals[0] = "Единый цвет##CHC0";
    strings["Channel type##CHC1"].plurals[0] = "Согласно типу канала##CHC1";
    strings["Instrument type##CHC2"].plurals[0] = "Согласно типу инструмента##CHC2";
    strings["Channel name colors:##sgse"].plurals[0] = "Цвета названия канала:";
    strings["Single##CTC0"].plurals[0] = "Единый цвет##CTC0";
    strings["Channel type##CTC1"].plurals[0] = "Согласно типу канала##CTC1";
    strings["Instrument type##CTC2"].plurals[0] = "Согласно типу инструмента##CTC2";
    strings["Assets##sgse1"].plurals[0] = "Представление ресурсов модуля";
    strings["Unified instrument/wavetable/sample list##sgse"].plurals[0] = "Единый список инструментов, волновых таблиц и сэмплов";
    strings["Horizontal instrument list##sgse"].plurals[0] = "Горизонтальный список инструментов";
    strings["Instrument list icon style:##sgse"].plurals[0] = "Стиль иконок в списке инструментов:";
    strings["None##iis0"].plurals[0] = "Не показывать##iis0";
    strings["Graphical icons##iis1"].plurals[0] = "Графические иконки##iis1";
    strings["Letter icons##iis2"].plurals[0] = "Иконки с буквами##iis2";
    strings["Colorize instrument editor using instrument type##sgse"].plurals[0] = "Изменять оттенки цветов редактора инструмента согласно типу инструмента";
    strings["Macro Editor##sgse0"].plurals[0] = "Редактор макросов";
    strings["Macro editor layout:##sgse"].plurals[0] = "Компоновка редактора макросов:";
    strings["Unified##mel0"].plurals[0] = "Общий список##mel0";
    strings["Grid##mel2"].plurals[0] = "Прямоугольная сетка##mel2";
    strings["Single (with list)##mel3"].plurals[0] = "Окно редактирования одного макроса + список##mel3";
    strings["Use classic macro editor vertical slider##sgse"].plurals[0] = "Использовать классическую вертикальную полосу прокрутки";
    strings["Wave Editor##sgse"].plurals[0] = "Редактор волновых таблиц";
    strings["Use compact wave editor##sgse"].plurals[0] = "Использовать компактный редактор волновых таблиц";
    strings["FM Editor##sgse0"].plurals[0] = "Редактор FM-инструментов";
    strings["FM parameter names:##sgse"].plurals[0] = "Названия параметров:";
    strings["Friendly##fmn0"].plurals[0] = "Понятные##fmn0";
    strings["Technical##fmn1"].plurals[0] = "Исходные формальные##fmn1";
    strings["Technical (alternate)##fmn2"].plurals[0] = "Исходные формальные (альтернативные)##fmn2";
    strings["Use standard OPL waveform names##sgse"].plurals[0] = "Использовать стандартные названия волн для чипов серии OPL";
    strings["FM parameter editor layout:##sgse"].plurals[0] = "Компоновка редактора FM-инструментов";
    strings["Modern##fml0"].plurals[0] = "Современная##fml0";
    strings["Compact (2x2, classic)##fml1"].plurals[0] = "Компактная (2x2, классическая)##fml1";
    strings["Compact (1x4)##fml2"].plurals[0] = "Компактная (1x4)##fml2";
    strings["Compact (4x1)##fml3"].plurals[0] = "Компактная (4x1)##fml3";
    strings["Alternate (2x2)##fml4"].plurals[0] = "Альтернативная (2x2)##fml4";
    strings["Alternate (1x4)##fml5"].plurals[0] = "Альтернативная (1x4)##fml5";
    strings["Alternate (4x1)##fml6"].plurals[0] = "Альтернативная (4x1)##fml6";
    strings["Position of Sustain in FM editor:##sgse"].plurals[0] = "Позиция параметра \"Сустейн\" в редакторе:";
    strings["Between Decay and Sustain Rate##susp0"].plurals[0] = "Между спадом и уклоном сустейна##susp0";
    strings["After Release Rate##susp1"].plurals[0] = "После уклона релиза##susp1";
    strings["Use separate colors for carriers/modulators in FM editor##sgse"].plurals[0] = "Использовать различные цвета для модулирующих и несущих операторов";
    strings["Unsigned FM detune values##sgse"].plurals[0] = "Беззнаковое значение параметра расстройки";
    strings["Memory Composition##sgse"].plurals[0] = "Содержание памяти";
    strings["Chip memory usage unit:##sgse"].plurals[0] = "Единицы измерения использования памяти чипа:";
    strings["Bytes##MUU0"].plurals[0] = "Байты##MUU0";
    strings["Kilobytes##MUU1"].plurals[0] = "Килобайты##MUU1";
    strings["Oscilloscope##set"].plurals[0] = "Осциллограф##set";
    strings["Rounded corners##sgse"].plurals[0] = "Закруглённые края";
    strings["Border##sgse"].plurals[0] = "Граница";
    strings["Mono##sgse1"].plurals[0] = "Моно";
    strings["Anti-aliased##sgse"].plurals[0] = "Со сглаживанием";
    strings["Fill entire window##sgse"].plurals[0] = "Полностью заполняет окно";
    strings["Waveform goes out of bounds##sgse"].plurals[0] = "Волна может выходить за пределы окна";
    strings["Line size##sgse"].plurals[0] = "Толщина линии";
    strings["Windows##sgse"].plurals[0] = "Окна";
    strings["Rounded window corners##sgse"].plurals[0] = "Закруглённые края окон";
    strings["Rounded buttons##sgse"].plurals[0] = "Закруглённые края кнопок";
    strings["Rounded tabs##sgse"].plurals[0] = "Закруглённые края заголовков вкладок";
    strings["Rounded scrollbars##sgse"].plurals[0] = "Закруглённые края ползунка полосы прокрутки";
    strings["Rounded menu corners##sgse"].plurals[0] = "Закруглённые края выпадающих меню";
    strings["Borders around widgets##sgse"].plurals[0] = "Границы вокруг кнопок, списков и т.д.";
    strings["Misc##sgse"].plurals[0] = "Разное";
    strings["Wrap text##sgse"].plurals[0] = "Переносить текст на новую строку";
    strings["Wrap text in song/subsong comments window.##sgse"].plurals[0] = "Переносить текст на новую строку в окне информации/комментариев к композиции.";
    strings["Frame shading in text windows##sgse"].plurals[0] = "Градиент в текстовых окнах";
    strings["Apply frame shading to the multiline text fields\nsuch as song/subsong info/comments.##sgse"].plurals[0] = "Применять градиент в окне информации/комментариев к композиции.";
    strings["Show chip info in chip manager##sgse"].plurals[0] = "Отображать информацию о чипе в менеджере чипов";
    strings["Display tooltip in chip manager when hovering over the chip. Tooltip shows chip name and description.##sgse"].plurals[0] = "Отображать подсказку при наведении курсора на чип. Подсказка отображает название и описание чипа.";
    strings["Color##sgse"].plurals[0] = "Цвета";
    strings["Color scheme##sgse"].plurals[0] = "Цветовая схема";
    strings["Import##sgse2"].plurals[0] = "Импорт";
    strings["Export##sgse2"].plurals[0] = "Экспорт";
    strings["Reset defaults##sgse1"].plurals[0] = "Сбросить";
    strings["Are you sure you want to reset the color scheme?##sgse"].plurals[0] = "Вы действительно хотите сбросить цветовую схему?";
    strings["Interface##sgse1"].plurals[0] = "Интерфейс";
    strings["Frame shading##sgse"].plurals[0] = "Затенение рамок";
    strings["Interface (other)##sgse"].plurals[0] = "Интерфейс (другое)";
    strings["Miscellaneous##sgse"].plurals[0] = "Разное";
    strings["File Picker (built-in)##sgse"].plurals[0] = "Меню выбора файлов (встроенное)";
    strings["Oscilloscope##sgse"].plurals[0] = "Осциллограф";
    strings["Wave (non-mono)##sgse"].plurals[0] = "Волна (не моно)";
    strings["Volume Meter##sgse"].plurals[0] = "Измеритель громкости";
    strings["Orders##sgse2"].plurals[0] = "Матрица паттернов";
    strings["Envelope View##sgse"].plurals[0] = "Визуализатор огибающей";
    strings["FM Editor##sgse1"].plurals[0] = "Редактор FM-инструментов";
    strings["Macro Editor##sgse1"].plurals[0] = "Редактор макросов";
    strings["Instrument Types##sgse"].plurals[0] = "Типы инструментов";
    strings["Channel##sgse2"].plurals[0] = "Канал";
    strings["Pattern##sgse2"].plurals[0] = "Паттерн";
    strings["Sample Editor##sgse"].plurals[0] = "Редактор сэмплов";
    strings["Pattern Manager##sgse"].plurals[0] = "Менеджер паттернов";
    strings["Piano##sgse"].plurals[0] = "Клавиатура пианино";
    strings["Clock##sgse"].plurals[0] = "Часы";
    strings["Patchbay##sgse"].plurals[0] = "Соединение каналов";
    strings["Memory Composition##sgse"].plurals[0] = "Содержание памяти##sgse";
    strings["Log Viewer##sgse"].plurals[0] = "Просмотр логов";

    // these are messy, but the ##CC_GUI... is required.
    strings["Button##CC_GUI_COLOR_BUTTON"].plurals[0] = "Кнопка##CC_GUI_COLOR_BUTTON";
    strings["Button (hovered)##CC_GUI_COLOR_BUTTON_HOVER"].plurals[0] = "Кнопка (курсор на кнопке)##CC_GUI_COLOR_BUTTON_HOVER";
    strings["Button (active)##CC_GUI_COLOR_BUTTON_ACTIVE"].plurals[0] = "Кнопка (нажатая)##CC_GUI_COLOR_BUTTON_ACTIVE";
    strings["Tab##CC_GUI_COLOR_TAB"].plurals[0] = "Вкладка##CC_GUI_COLOR_TAB";
    strings["Tab (hovered)##CC_GUI_COLOR_TAB_HOVER"].plurals[0] = "Вкладка (курсор на вкладке)##CC_GUI_COLOR_TAB_HOVER";
    strings["Tab (active)##CC_GUI_COLOR_TAB_ACTIVE"].plurals[0] = "Вкладка (активная)##CC_GUI_COLOR_TAB_ACTIVE";
    strings["Tab (unfocused)##CC_GUI_COLOR_TAB_UNFOCUSED"].plurals[0] = "Вкладка (не текущая)##CC_GUI_COLOR_TAB_UNFOCUSED";
    strings["Tab (unfocused and active)##CC_GUI_COLOR_TAB_UNFOCUSED_ACTIVE"].plurals[0] = "Вкладка (курсор на вкладке, текущая)##CC_GUI_COLOR_TAB_UNFOCUSED_ACTIVE";
    strings["ImGui header##CC_GUI_COLOR_IMGUI_HEADER"].plurals[0] = "Заголовок ImGui##CC_GUI_COLOR_IMGUI_HEADER";
    strings["ImGui header (hovered)##CC_GUI_COLOR_IMGUI_HEADER_HOVER"].plurals[0] = "Заголовок ImGui (курсор на заголовке)##CC_GUI_COLOR_IMGUI_HEADER_HOVER";
    strings["ImGui header (active)##CC_GUI_COLOR_IMGUI_HEADER_ACTIVE"].plurals[0] = "Заголовок ImGui (активный)##CC_GUI_COLOR_IMGUI_HEADER_ACTIVE";
    strings["Resize grip##CC_GUI_COLOR_RESIZE_GRIP"].plurals[0] = "Захват для изменения размера окна##CC_GUI_COLOR_RESIZE_GRIP";
    strings["Resize grip (hovered)##CC_GUI_COLOR_RESIZE_GRIP_HOVER"].plurals[0] = "Захват для изменения размера окна (курсор на захвате)##CC_GUI_COLOR_RESIZE_GRIP_HOVER";
    strings["Resize grip (active)##CC_GUI_COLOR_RESIZE_GRIP_ACTIVE"].plurals[0] = "Захват для изменения размера окна (активный)##CC_GUI_COLOR_RESIZE_GRIP_ACTIVE";
    strings["Widget background##CC_GUI_COLOR_WIDGET_BACKGROUND"].plurals[0] = "Задний фон виджета##CC_GUI_COLOR_WIDGET_BACKGROUND";
    strings["Widget background (hovered)##CC_GUI_COLOR_WIDGET_BACKGROUND_HOVER"].plurals[0] = "Задний фон виджета (курсор на виджете)##CC_GUI_COLOR_WIDGET_BACKGROUND_HOVER";
    strings["Widget background (active)##CC_GUI_COLOR_WIDGET_BACKGROUND_ACTIVE"].plurals[0] = "Задний фон виджета (активный)##CC_GUI_COLOR_WIDGET_BACKGROUND_ACTIVE";
    strings["Slider grab##CC_GUI_COLOR_SLIDER_GRAB"].plurals[0] = "Ползунок выставления значения##CC_GUI_COLOR_SLIDER_GRAB";
    strings["Slider grab (active)##CC_GUI_COLOR_SLIDER_GRAB_ACTIVE"].plurals[0] = "Ползунок выставления значения (активный)##CC_GUI_COLOR_SLIDER_GRAB_ACTIVE";
    strings["Title background (active)##CC_GUI_COLOR_TITLE_BACKGROUND_ACTIVE"].plurals[0] = "Задний фон заголовка (активный)##CC_GUI_COLOR_TITLE_BACKGROUND_ACTIVE";
    strings["Checkbox/radio button mark##CC_GUI_COLOR_CHECK_MARK"].plurals[0] = "Маркировка в поле для галочки и радиокнопке##CC_GUI_COLOR_CHECK_MARK";
    strings["Text selection##CC_GUI_COLOR_TEXT_SELECTION"].plurals[0] = "Выделение текста##CC_GUI_COLOR_TEXT_SELECTION";
    strings["Line plot##CC_GUI_COLOR_PLOT_LINES"].plurals[0] = "Цвет линий##CC_GUI_COLOR_PLOT_LINES";
    strings["Line plot (hovered)##CC_GUI_COLOR_PLOT_LINES_HOVER"].plurals[0] = "Цвет линий (курсор на поле)##CC_GUI_COLOR_PLOT_LINES_HOVER";
    strings["Histogram plot##CC_GUI_COLOR_PLOT_HISTOGRAM"].plurals[0] = "Столбчатая диаграмма##CC_GUI_COLOR_PLOT_HISTOGRAM";
    strings["Histogram plot (hovered)##CC_GUI_COLOR_PLOT_HISTOGRAM_HOVER"].plurals[0] = "Столбчатая диаграмма (курсор на диаграмме)##CC_GUI_COLOR_PLOT_HISTOGRAM_HOVER";
    strings["Table row (even)##CC_GUI_COLOR_TABLE_ROW_EVEN"].plurals[0] = "Строка таблицы (чётная)##CC_GUI_COLOR_TABLE_ROW_EVEN";
    strings["Table row (odd)##CC_GUI_COLOR_TABLE_ROW_ODD"].plurals[0] = "Строка таблицы (нечётная)##CC_GUI_COLOR_TABLE_ROW_ODD";

    strings["Background##CC_GUI_COLOR_BACKGROUND"].plurals[0] = "Задний фон##CC_GUI_COLOR_BACKGROUND";
    strings["Window background##CC_GUI_COLOR_FRAME_BACKGROUND"].plurals[0] = "Фон окон##CC_GUI_COLOR_FRAME_BACKGROUND";
    strings["Sub-window background##CC_GUI_COLOR_FRAME_BACKGROUND_CHILD"].plurals[0] = "Фон подокн##CC_GUI_COLOR_FRAME_BACKGROUND_CHILD";
    strings["Pop-up background##CC_GUI_COLOR_FRAME_BACKGROUND_POPUP"].plurals[0] = "Фон всплывающих окон##CC_GUI_COLOR_FRAME_BACKGROUND_POPUP";
    strings["Modal backdrop##CC_GUI_COLOR_MODAL_BACKDROP"].plurals[0] = "Затенение при возникновении модального окна##CC_GUI_COLOR_MODAL_BACKDROP";
    strings["Header##CC_GUI_COLOR_HEADER"].plurals[0] = "Заголовок##CC_GUI_COLOR_HEADER";
    strings["Text##CC_GUI_COLOR_TEXT"].plurals[0] = "Текст##CC_GUI_COLOR_TEXT";
    strings["Text (disabled)##CC_GUI_COLOR_TEXT_DISABLED"].plurals[0] = "Текст (выключенный)##CC_GUI_COLOR_TEXT_DISABLED";
    strings["Title bar (inactive)##CC_GUI_COLOR_TITLE_INACTIVE"].plurals[0] = "Полоса заголовка (неактивная)##CC_GUI_COLOR_TITLE_INACTIVE";
    strings["Title bar (collapsed)##CC_GUI_COLOR_TITLE_COLLAPSED"].plurals[0] = "Полоса заголовка (свёрнутое окно)##CC_GUI_COLOR_TITLE_COLLAPSED";
    strings["Menu bar##CC_GUI_COLOR_MENU_BAR"].plurals[0] = "Полоса меню##CC_GUI_COLOR_MENU_BAR";
    strings["Border##CC_GUI_COLOR_BORDER"].plurals[0] = "Граница##CC_GUI_COLOR_BORDER";
    strings["Border shadow##CC_GUI_COLOR_BORDER_SHADOW"].plurals[0] = "Тень границы##CC_GUI_COLOR_BORDER_SHADOW";
    strings["Scroll bar##CC_GUI_COLOR_SCROLL"].plurals[0] = "Полоса прокрутки##CC_GUI_COLOR_SCROLL";
    strings["Scroll bar (hovered)##CC_GUI_COLOR_SCROLL_HOVER"].plurals[0] = "Полоса прокрутки (курсор на полосе)##CC_GUI_COLOR_SCROLL_HOVER";
    strings["Scroll bar (clicked)##CC_GUI_COLOR_SCROLL_ACTIVE"].plurals[0] = "Полоса прокрутки (нажата)##CC_GUI_COLOR_SCROLL_ACTIVE";
    strings["Scroll bar background##CC_GUI_COLOR_SCROLL_BACKGROUND"].plurals[0] = "Фон полосы прокрутки##CC_GUI_COLOR_SCROLL_BACKGROUND";
    strings["Separator##CC_GUI_COLOR_SEPARATOR"].plurals[0] = "Разделитель строк##CC_GUI_COLOR_SEPARATOR";
    strings["Separator (hover)##CC_GUI_COLOR_SEPARATOR_HOVER"].plurals[0] = "Разделитель строк (курсор на разделителе)##CC_GUI_COLOR_SEPARATOR_HOVER";
    strings["Separator (active)##CC_GUI_COLOR_SEPARATOR_ACTIVE"].plurals[0] = "Разделитель строк (активный)##CC_GUI_COLOR_SEPARATOR_ACTIVE";
    strings["Docking preview##CC_GUI_COLOR_DOCKING_PREVIEW"].plurals[0] = "Превью стыковки окон##CC_GUI_COLOR_DOCKING_PREVIEW";
    strings["Docking empty##CC_GUI_COLOR_DOCKING_EMPTY"].plurals[0] = "Пустое поле стыковки окон##CC_GUI_COLOR_DOCKING_EMPTY";
    strings["Table header##CC_GUI_COLOR_TABLE_HEADER"].plurals[0] = "Заголовок таблицы##CC_GUI_COLOR_TABLE_HEADER";
    strings["Table border (hard)##CC_GUI_COLOR_TABLE_BORDER_HARD"].plurals[0] = "Заголовок таблицы (жёсткий)##CC_GUI_COLOR_TABLE_BORDER_HARD";
    strings["Table border (soft)##CC_GUI_COLOR_TABLE_BORDER_SOFT"].plurals[0] = "Заголовок таблицы (мягкий)##CC_GUI_COLOR_TABLE_BORDER_SOFT";
    strings["Drag and drop target##CC_GUI_COLOR_DRAG_DROP_TARGET"].plurals[0] = "Цель перетаскивания##CC_GUI_COLOR_DRAG_DROP_TARGET";
    strings["Window switcher (highlight)##CC_GUI_COLOR_NAV_WIN_HIGHLIGHT"].plurals[0] = "Переключатель окон (подсвечен)##CC_GUI_COLOR_NAV_WIN_HIGHLIGHT";
    strings["Window switcher backdrop##CC_GUI_COLOR_NAV_WIN_BACKDROP"].plurals[0] = "Переключатель окон (цвет затенения остального интерфейса)##CC_GUI_COLOR_NAV_WIN_BACKDROP";

    strings["Toggle on##CC_GUI_COLOR_TOGGLE_ON"].plurals[0] = "Переключатель вкл.##CC_GUI_COLOR_TOGGLE_ON";
    strings["Toggle off##CC_GUI_COLOR_TOGGLE_OFF"].plurals[0] = "Переключатель выкл.##CC_GUI_COLOR_TOGGLE_OFF";
    strings["Playback status##CC_GUI_COLOR_PLAYBACK_STAT"].plurals[0] = "Статус воспроизведения##CC_GUI_COLOR_PLAYBACK_STAT";
    strings["Destructive hint##CC_GUI_COLOR_DESTRUCTIVE"].plurals[0] = "Подсказка об удалении##CC_GUI_COLOR_DESTRUCTIVE";
    strings["Warning hint##CC_GUI_COLOR_WARNING"].plurals[0] = "Подсказка о сообщении##CC_GUI_COLOR_WARNING";
    strings["Error hint##CC_GUI_COLOR_ERROR"].plurals[0] = "Подсказка об ошибке##CC_GUI_COLOR_ERROR";

    strings["Directory##CC_GUI_COLOR_FILE_DIR"].plurals[0] = "Папка##CC_GUI_COLOR_FILE_DIR";
    strings["Song (native)##CC_GUI_COLOR_FILE_SONG_NATIVE"].plurals[0] = "Файл трека (нативно поддерживается)##CC_GUI_COLOR_FILE_SONG_NATIVE";
    strings["Song (import)##CC_GUI_COLOR_FILE_SONG_IMPORT"].plurals[0] = "Файл трека (импорт)##CC_GUI_COLOR_FILE_SONG_IMPORT";
    strings["Instrument##CC_GUI_COLOR_FILE_INSTR"].plurals[0] = "Инструмент##CC_GUI_COLOR_FILE_INSTR";
    strings["Audio##CC_GUI_COLOR_FILE_AUDIO"].plurals[0] = "Аудиофайл##CC_GUI_COLOR_FILE_AUDIO";
    strings["Wavetable##CC_GUI_COLOR_FILE_WAVE"].plurals[0] = "Волновая таблица##CC_GUI_COLOR_FILE_WAVE";
    strings["VGM##CC_GUI_COLOR_FILE_VGM"].plurals[0] = "VGM##CC_GUI_COLOR_FILE_VGM";
    strings["ZSM##CC_GUI_COLOR_FILE_ZSM"].plurals[0] = "ZSM##CC_GUI_COLOR_FILE_ZSM";
    strings["Font##CC_GUI_COLOR_FILE_FONT"].plurals[0] = "Шрифт##CC_GUI_COLOR_FILE_FONT";
    strings["Other##CC_GUI_COLOR_FILE_OTHER"].plurals[0] = "Другое##CC_GUI_COLOR_FILE_OTHER";

    strings["Border##CC_GUI_COLOR_OSC_BORDER"].plurals[0] = "Граница осциллографа##CC_GUI_COLOR_OSC_BORDER";
    strings["Background (top-left)##CC_GUI_COLOR_OSC_BG1"].plurals[0] = "Фон (верхний левый угол)##CC_GUI_COLOR_OSC_BG1";
    strings["Background (top-right)##CC_GUI_COLOR_OSC_BG2"].plurals[0] = "Фон (верхний правый угол)##CC_GUI_COLOR_OSC_BG2";
    strings["Background (bottom-left)##CC_GUI_COLOR_OSC_BG3"].plurals[0] = "Фон (нижний левый угол)##CC_GUI_COLOR_OSC_BG3";
    strings["Background (bottom-right)##CC_GUI_COLOR_OSC_BG4"].plurals[0] = "Фон (нижний правый угол)##CC_GUI_COLOR_OSC_BG4";
    strings["Waveform##CC_GUI_COLOR_OSC_WAVE"].plurals[0] = "Волна##CC_GUI_COLOR_OSC_WAVE";
    strings["Waveform (clip)##CC_GUI_COLOR_OSC_WAVE_PEAK"].plurals[0] = "Волна (зашкал амплитуды)##CC_GUI_COLOR_OSC_WAVE_PEAK";
    strings["Reference##CC_GUI_COLOR_OSC_REF"].plurals[0] = "Образец##CC_GUI_COLOR_OSC_REF";
    strings["Guide##CC_GUI_COLOR_OSC_GUIDE"].plurals[0] = "Справка##CC_GUI_COLOR_OSC_GUIDE";

    strings["Waveform (1)##CC_GUI_COLOR_OSC_WAVE_CH0"].plurals[0] = "Волна (1)##CC_GUI_COLOR_OSC_WAVE_CH0";
    strings["Waveform (2)##CC_GUI_COLOR_OSC_WAVE_CH1"].plurals[0] = "Волна (2)##CC_GUI_COLOR_OSC_WAVE_CH1";
    strings["Waveform (3)##CC_GUI_COLOR_OSC_WAVE_CH2"].plurals[0] = "Волна (3)##CC_GUI_COLOR_OSC_WAVE_CH2";
    strings["Waveform (4)##CC_GUI_COLOR_OSC_WAVE_CH3"].plurals[0] = "Волна (4)##CC_GUI_COLOR_OSC_WAVE_CH3";
    strings["Waveform (5)##CC_GUI_COLOR_OSC_WAVE_CH4"].plurals[0] = "Волна (5)##CC_GUI_COLOR_OSC_WAVE_CH4";
    strings["Waveform (6)##CC_GUI_COLOR_OSC_WAVE_CH5"].plurals[0] = "Волна (6)##CC_GUI_COLOR_OSC_WAVE_CH5";
    strings["Waveform (7)##CC_GUI_COLOR_OSC_WAVE_CH6"].plurals[0] = "Волна (7)##CC_GUI_COLOR_OSC_WAVE_CH6";
    strings["Waveform (8)##CC_GUI_COLOR_OSC_WAVE_CH7"].plurals[0] = "Волна (8)##CC_GUI_COLOR_OSC_WAVE_CH7";
    strings["Waveform (9)##CC_GUI_COLOR_OSC_WAVE_CH8"].plurals[0] = "Волна (9)##CC_GUI_COLOR_OSC_WAVE_CH8";
    strings["Waveform (10)##CC_GUI_COLOR_OSC_WAVE_CH9"].plurals[0] = "Волна (10)##CC_GUI_COLOR_OSC_WAVE_CH9";
    strings["Waveform (11)##CC_GUI_COLOR_OSC_WAVE_CH10"].plurals[0] = "Волна (11)##CC_GUI_COLOR_OSC_WAVE_CH10";
    strings["Waveform (12)##CC_GUI_COLOR_OSC_WAVE_CH11"].plurals[0] = "Волна (12)##CC_GUI_COLOR_OSC_WAVE_CH11";
    strings["Waveform (13)##CC_GUI_COLOR_OSC_WAVE_CH12"].plurals[0] = "Волна (13)##CC_GUI_COLOR_OSC_WAVE_CH12";
    strings["Waveform (14)##CC_GUI_COLOR_OSC_WAVE_CH13"].plurals[0] = "Волна (14)##CC_GUI_COLOR_OSC_WAVE_CH13";
    strings["Waveform (15)##CC_GUI_COLOR_OSC_WAVE_CH14"].plurals[0] = "Волна (15)##CC_GUI_COLOR_OSC_WAVE_CH14";
    strings["Waveform (16)##CC_GUI_COLOR_OSC_WAVE_CH15"].plurals[0] = "Волна (16)##CC_GUI_COLOR_OSC_WAVE_CH15";

    strings["Low##CC_GUI_COLOR_VOLMETER_LOW"].plurals[0] = "Низкий уровень##CC_GUI_COLOR_VOLMETER_LOW";
    strings["High##CC_GUI_COLOR_VOLMETER_HIGH"].plurals[0] = "Высокий уровень##CC_GUI_COLOR_VOLMETER_HIGH";
    strings["Clip##CC_GUI_COLOR_VOLMETER_PEAK"].plurals[0] = "Зашкал##CC_GUI_COLOR_VOLMETER_PEAK";

    strings["Order number##CC_GUI_COLOR_ORDER_ROW_INDEX"].plurals[0] = "Номер строки##CC_GUI_COLOR_ORDER_ROW_INDEX";
    strings["Playing order background##CC_GUI_COLOR_ORDER_ACTIVE"].plurals[0] = "Фон текущей строки##CC_GUI_COLOR_ORDER_ACTIVE";
    strings["Song loop##CC_GUI_COLOR_SONG_LOOP"].plurals[0] = "Место зацикливания трека##CC_GUI_COLOR_SONG_LOOP";
    strings["Selected order##CC_GUI_COLOR_ORDER_SELECTED"].plurals[0] = "Выделенный столбец##CC_GUI_COLOR_ORDER_SELECTED";
    strings["Similar patterns##CC_GUI_COLOR_ORDER_SIMILAR"].plurals[0] = "Похожие паттерны##CC_GUI_COLOR_ORDER_SIMILAR";
    strings["Inactive patterns##CC_GUI_COLOR_ORDER_INACTIVE"].plurals[0] = "Неактивные паттерны##CC_GUI_COLOR_ORDER_INACTIVE";

    strings["Envelope##CC_GUI_COLOR_FM_ENVELOPE"].plurals[0] = "Огибающая##CC_GUI_COLOR_FM_ENVELOPE";
    strings["Sustain guide##CC_GUI_COLOR_FM_ENVELOPE_SUS_GUIDE"].plurals[0] = "Визуализация сустейна##CC_GUI_COLOR_FM_ENVELOPE_SUS_GUIDE";
    strings["Release##CC_GUI_COLOR_FM_ENVELOPE_RELEASE"].plurals[0] = "Релиз##CC_GUI_COLOR_FM_ENVELOPE_RELEASE";

    strings["Algorithm background##CC_GUI_COLOR_FM_ALG_BG"].plurals[0] = "Задний фон схемы алгоритма##CC_GUI_COLOR_FM_ALG_BG";
    strings["Algorithm lines##CC_GUI_COLOR_FM_ALG_LINE"].plurals[0] = "Линии схемы алгоритма##CC_GUI_COLOR_FM_ALG_LINE";
    strings["Modulator##CC_GUI_COLOR_FM_MOD"].plurals[0] = "Модулирующий оператор##CC_GUI_COLOR_FM_MOD";
    strings["Carrier##CC_GUI_COLOR_FM_CAR"].plurals[0] = "Несущий оператор##CC_GUI_COLOR_FM_CAR";

    strings["SSG-EG##CC_GUI_COLOR_FM_SSG"].plurals[0] = "SSG-EG##CC_GUI_COLOR_FM_SSG";
    strings["Waveform##CC_GUI_COLOR_FM_WAVE"].plurals[0] = "Волна##CC_GUI_COLOR_FM_WAVE";

    strings["Mod. accent (primary)##CC_GUI_COLOR_FM_PRIMARY_MOD"].plurals[0] = "Оттенок модулирующего оператора (основной)##CC_GUI_COLOR_FM_PRIMARY_MOD";
    strings["Mod. accent (secondary)##CC_GUI_COLOR_FM_SECONDARY_MOD"].plurals[0] = "Оттенок модулирующего оператора (неосновной)##CC_GUI_COLOR_FM_SECONDARY_MOD";
    strings["Mod. border##CC_GUI_COLOR_FM_BORDER_MOD"].plurals[0] = "Граница модулирующего оператора##CC_GUI_COLOR_FM_BORDER_MOD";
    strings["Mod. border shadow##CC_GUI_COLOR_FM_BORDER_SHADOW_MOD"].plurals[0] = "Тень границы модулирующего оператора##CC_GUI_COLOR_FM_BORDER_SHADOW_MOD";

    strings["Car. accent (primary)##CC_GUI_COLOR_FM_PRIMARY_CAR"].plurals[0] = "Оттенок несущего оператора (основной)##CC_GUI_COLOR_FM_PRIMARY_CAR";
    strings["Car. accent (secondary)##CC_GUI_COLOR_FM_SECONDARY_CAR"].plurals[0] = "Оттенок несущего оператора (неосновной)##CC_GUI_COLOR_FM_SECONDARY_CAR";
    strings["Car. border##CC_GUI_COLOR_FM_BORDER_CAR"].plurals[0] = "Граница несущего оператора##CC_GUI_COLOR_FM_BORDER_CAR";
    strings["Car. border shadow##CC_GUI_COLOR_FM_BORDER_SHADOW_CAR"].plurals[0] = "Тень границы несущего оператора##CC_GUI_COLOR_FM_BORDER_SHADOW_CAR";

    strings["Volume##CC_GUI_COLOR_MACRO_VOLUME"].plurals[0] = "Громкость##CC_GUI_COLOR_MACRO_VOLUME";
    strings["Pitch##CC_GUI_COLOR_MACRO_PITCH"].plurals[0] = "Частота##CC_GUI_COLOR_MACRO_PITCH";
    strings["Wave##CC_GUI_COLOR_MACRO_WAVE"].plurals[0] = "Волна##CC_GUI_COLOR_MACRO_WAVE";
    strings["Other##CC_GUI_COLOR_MACRO_OTHER"].plurals[0] = "Другое##CC_GUI_COLOR_MACRO_OTHER";

    strings["FM (OPN)##CC_GUI_COLOR_INSTR_FM"].plurals[0] = "FM (OPN)##CC_GUI_COLOR_INSTR_FM";
    strings["SN76489/Sega PSG##CC_GUI_COLOR_INSTR_STD"].plurals[0] = "SN76489/Sega PSG##CC_GUI_COLOR_INSTR_STD";
    strings["T6W28##CC_GUI_COLOR_INSTR_T6W28"].plurals[0] = "T6W28##CC_GUI_COLOR_INSTR_T6W28";
    strings["Game Boy##CC_GUI_COLOR_INSTR_GB"].plurals[0] = "Game Boy##CC_GUI_COLOR_INSTR_GB";
    strings["C64##CC_GUI_COLOR_INSTR_C64"].plurals[0] = "C64##CC_GUI_COLOR_INSTR_C64";
    strings["Amiga/Generic Sample##CC_GUI_COLOR_INSTR_AMIGA"].plurals[0] = "Amiga/Типичный сэмпл##CC_GUI_COLOR_INSTR_AMIGA";
    strings["PC Engine##CC_GUI_COLOR_INSTR_PCE"].plurals[0] = "PC Engine##CC_GUI_COLOR_INSTR_PCE";
    strings["AY-3-8910/SSG##CC_GUI_COLOR_INSTR_AY"].plurals[0] = "AY-3-8910/SSG##CC_GUI_COLOR_INSTR_AY";
    strings["AY8930##CC_GUI_COLOR_INSTR_AY8930"].plurals[0] = "AY8930##CC_GUI_COLOR_INSTR_AY8930";
    strings["TIA##CC_GUI_COLOR_INSTR_TIA"].plurals[0] = "TIA##CC_GUI_COLOR_INSTR_TIA";
    strings["SAA1099##CC_GUI_COLOR_INSTR_SAA1099"].plurals[0] = "SAA1099##CC_GUI_COLOR_INSTR_SAA1099";
    strings["VIC##CC_GUI_COLOR_INSTR_VIC"].plurals[0] = "VIC##CC_GUI_COLOR_INSTR_VIC";
    strings["PET##CC_GUI_COLOR_INSTR_PET"].plurals[0] = "PET##CC_GUI_COLOR_INSTR_PET";
    strings["VRC6##CC_GUI_COLOR_INSTR_VRC6"].plurals[0] = "VRC6##CC_GUI_COLOR_INSTR_VRC6";
    strings["VRC6 (saw)##CC_GUI_COLOR_INSTR_VRC6_SAW"].plurals[0] = "VRC6 (пила)##CC_GUI_COLOR_INSTR_VRC6_SAW";
    strings["FM (OPLL)##CC_GUI_COLOR_INSTR_OPLL"].plurals[0] = "FM (OPLL)##CC_GUI_COLOR_INSTR_OPLL";
    strings["FM (OPL)##CC_GUI_COLOR_INSTR_OPL"].plurals[0] = "FM (OPL)##CC_GUI_COLOR_INSTR_OPL";
    strings["FDS##CC_GUI_COLOR_INSTR_FDS"].plurals[0] = "FDS##CC_GUI_COLOR_INSTR_FDS";
    strings["Virtual Boy##CC_GUI_COLOR_INSTR_VBOY"].plurals[0] = "Virtual Boy##CC_GUI_COLOR_INSTR_VBOY";
    strings["Namco 163##CC_GUI_COLOR_INSTR_N163"].plurals[0] = "Namco 163##CC_GUI_COLOR_INSTR_N163";
    strings["Konami SCC##CC_GUI_COLOR_INSTR_SCC"].plurals[0] = "Konami SCC##CC_GUI_COLOR_INSTR_SCC";
    strings["FM (OPZ)##CC_GUI_COLOR_INSTR_OPZ"].plurals[0] = "FM (OPZ)##CC_GUI_COLOR_INSTR_OPZ";
    strings["POKEY##CC_GUI_COLOR_INSTR_POKEY"].plurals[0] = "POKEY##CC_GUI_COLOR_INSTR_POKEY";
    strings["PC Beeper##CC_GUI_COLOR_INSTR_BEEPER"].plurals[0] = "PC Beeper##CC_GUI_COLOR_INSTR_BEEPER";
    strings["WonderSwan##CC_GUI_COLOR_INSTR_SWAN"].plurals[0] = "WonderSwan##CC_GUI_COLOR_INSTR_SWAN";
    strings["Lynx##CC_GUI_COLOR_INSTR_MIKEY"].plurals[0] = "Lynx##CC_GUI_COLOR_INSTR_MIKEY";
    strings["VERA##CC_GUI_COLOR_INSTR_VERA"].plurals[0] = "VERA##CC_GUI_COLOR_INSTR_VERA";
    strings["X1-010##CC_GUI_COLOR_INSTR_X1_010"].plurals[0] = "X1-010##CC_GUI_COLOR_INSTR_X1_010";
    strings["ES5506##CC_GUI_COLOR_INSTR_ES5506"].plurals[0] = "ES5506##CC_GUI_COLOR_INSTR_ES5506";
    strings["MultiPCM##CC_GUI_COLOR_INSTR_MULTIPCM"].plurals[0] = "MultiPCM##CC_GUI_COLOR_INSTR_MULTIPCM";
    strings["SNES##CC_GUI_COLOR_INSTR_SNES"].plurals[0] = "SNES##CC_GUI_COLOR_INSTR_SNES";
    strings["Sound Unit##CC_GUI_COLOR_INSTR_SU"].plurals[0] = "Sound Unit##CC_GUI_COLOR_INSTR_SU";
    strings["Namco WSG##CC_GUI_COLOR_INSTR_NAMCO"].plurals[0] = "Namco WSG##CC_GUI_COLOR_INSTR_NAMCO";
    strings["FM (OPL Drums)##CC_GUI_COLOR_INSTR_OPL_DRUMS"].plurals[0] = "FM (OPL, ударные)##CC_GUI_COLOR_INSTR_OPL_DRUMS";
    strings["FM (OPM)##CC_GUI_COLOR_INSTR_OPM"].plurals[0] = "FM (OPM)##CC_GUI_COLOR_INSTR_OPM";
    strings["NES##CC_GUI_COLOR_INSTR_NES"].plurals[0] = "NES##CC_GUI_COLOR_INSTR_NES";
    strings["MSM6258##CC_GUI_COLOR_INSTR_MSM6258"].plurals[0] = "MSM6258##CC_GUI_COLOR_INSTR_MSM6258";
    strings["MSM6295##CC_GUI_COLOR_INSTR_MSM6295"].plurals[0] = "MSM6295##CC_GUI_COLOR_INSTR_MSM6295";
    strings["ADPCM-A##CC_GUI_COLOR_INSTR_ADPCMA"].plurals[0] = "АДИКМ-A##CC_GUI_COLOR_INSTR_ADPCMA";
    strings["ADPCM-B##CC_GUI_COLOR_INSTR_ADPCMB"].plurals[0] = "АДИКМ-B##CC_GUI_COLOR_INSTR_ADPCMB";
    strings["Sega PCM##CC_GUI_COLOR_INSTR_SEGAPCM"].plurals[0] = "Sega PCM##CC_GUI_COLOR_INSTR_SEGAPCM";
    strings["QSound##CC_GUI_COLOR_INSTR_QSOUND"].plurals[0] = "QSound##CC_GUI_COLOR_INSTR_QSOUND";
    strings["YMZ280B##CC_GUI_COLOR_INSTR_YMZ280B"].plurals[0] = "YMZ280B##CC_GUI_COLOR_INSTR_YMZ280B";
    strings["RF5C68##CC_GUI_COLOR_INSTR_RF5C68"].plurals[0] = "RF5C68##CC_GUI_COLOR_INSTR_RF5C68";
    strings["MSM5232##CC_GUI_COLOR_INSTR_MSM5232"].plurals[0] = "MSM5232##CC_GUI_COLOR_INSTR_MSM5232";
    strings["K007232##CC_GUI_COLOR_INSTR_K007232"].plurals[0] = "K007232##CC_GUI_COLOR_INSTR_K007232";
    strings["GA20##CC_GUI_COLOR_INSTR_GA20"].plurals[0] = "GA20##CC_GUI_COLOR_INSTR_GA20";
    strings["Pokémon Mini##CC_GUI_COLOR_INSTR_POKEMINI"].plurals[0] = "Pokémon Mini##CC_GUI_COLOR_INSTR_POKEMINI";
    strings["SM8521##CC_GUI_COLOR_INSTR_SM8521"].plurals[0] = "SM8521##CC_GUI_COLOR_INSTR_SM8521";
    strings["PV-1000##CC_GUI_COLOR_INSTR_PV1000"].plurals[0] = "PV-1000##CC_GUI_COLOR_INSTR_PV1000";
    strings["K053260##CC_GUI_COLOR_INSTR_K053260"].plurals[0] = "K053260##CC_GUI_COLOR_INSTR_K053260";
    strings["C140##CC_GUI_COLOR_INSTR_C140"].plurals[0] = "C140##CC_GUI_COLOR_INSTR_C140";
    strings["C219##CC_GUI_COLOR_INSTR_C219"].plurals[0] = "C219##CC_GUI_COLOR_INSTR_C219";
    strings["ESFM##CC_GUI_COLOR_INSTR_ESFM"].plurals[0] = "ESFM##CC_GUI_COLOR_INSTR_ESFM";
    strings["ES5503##CC_GUI_COLOR_INSTR_ES5503"].plurals[0] = "ES5503##CC_GUI_COLOR_INSTR_ES5503";
    strings["PowerNoise (noise)##CC_GUI_COLOR_INSTR_POWERNOISE"].plurals[0] = "PowerNoise (шум)##CC_GUI_COLOR_INSTR_POWERNOISE";
    strings["PowerNoise (slope)##CC_GUI_COLOR_INSTR_POWERNOISE_SLOPE"].plurals[0] = "PowerNoise (скат)##CC_GUI_COLOR_INSTR_POWERNOISE_SLOPE";
    strings["Other/Unknown##CC_GUI_COLOR_INSTR_UNKNOWN"].plurals[0] = "Другой/неизв.##CC_GUI_COLOR_INSTR_UNKNOWN";

    strings["Single color (background)##CC_GUI_COLOR_CHANNEL_BG"].plurals[0] = "Один цвет (фон)##CC_GUI_COLOR_CHANNEL_BG";
    strings["Single color (text)##CC_GUI_COLOR_CHANNEL_FG"].plurals[0] = "Один цвет (текст)##CC_GUI_COLOR_CHANNEL_FG";
    strings["FM##CC_GUI_COLOR_CHANNEL_FM"].plurals[0] = "FM##CC_GUI_COLOR_CHANNEL_FM";
    strings["Pulse##CC_GUI_COLOR_CHANNEL_PULSE"].plurals[0] = "Меандр##CC_GUI_COLOR_CHANNEL_PULSE";
    strings["Noise##CC_GUI_COLOR_CHANNEL_NOISE"].plurals[0] = "Шум##CC_GUI_COLOR_CHANNEL_NOISE";
    strings["PCM##CC_GUI_COLOR_CHANNEL_PCM"].plurals[0] = "ИКМ##CC_GUI_COLOR_CHANNEL_PCM";
    strings["Wave##CC_GUI_COLOR_CHANNEL_WAVE"].plurals[0] = "Волна##CC_GUI_COLOR_CHANNEL_WAVE";
    strings["FM operator##CC_GUI_COLOR_CHANNEL_OP"].plurals[0] = "FM-оператор##CC_GUI_COLOR_CHANNEL_OP";
    strings["Muted##CC_GUI_COLOR_CHANNEL_MUTED"].plurals[0] = "Заглушен##CC_GUI_COLOR_CHANNEL_MUTED";

    strings["Playhead##CC_GUI_COLOR_PATTERN_PLAY_HEAD"].plurals[0] = "Указатель воспроизведения##CC_GUI_COLOR_PATTERN_PLAY_HEAD";
    strings["Editing##CC_GUI_COLOR_EDITING"].plurals[0] = "Редактирование##CC_GUI_COLOR_EDITING";
    strings["Editing (will clone)##CC_GUI_COLOR_EDITING_CLONE"].plurals[0] = "Редактирование (будет клонирован)##CC_GUI_COLOR_EDITING_CLONE";
    strings["Cursor##CC_GUI_COLOR_PATTERN_CURSOR"].plurals[0] = "Курсор##CC_GUI_COLOR_PATTERN_CURSOR";
    strings["Cursor (hovered)##CC_GUI_COLOR_PATTERN_CURSOR_HOVER"].plurals[0] = "Курсор (мышь на курсоре)##CC_GUI_COLOR_PATTERN_CURSOR_HOVER";
    strings["Cursor (clicked)##CC_GUI_COLOR_PATTERN_CURSOR_ACTIVE"].plurals[0] = "Курсор (нажат)##CC_GUI_COLOR_PATTERN_CURSOR_ACTIVE";
    strings["Selection##CC_GUI_COLOR_PATTERN_SELECTION"].plurals[0] = "Выделение##CC_GUI_COLOR_PATTERN_SELECTION";
    strings["Selection (hovered)##CC_GUI_COLOR_PATTERN_SELECTION_HOVER"].plurals[0] = "Выделение (курсор мыши на выделении)##CC_GUI_COLOR_PATTERN_SELECTION_HOVER";
    strings["Selection (clicked)##CC_GUI_COLOR_PATTERN_SELECTION_ACTIVE"].plurals[0] = "Выделение (нажато)##CC_GUI_COLOR_PATTERN_SELECTION_ACTIVE";
    strings["Highlight 1##CC_GUI_COLOR_PATTERN_HI_1"].plurals[0] = "Подсветка 1##CC_GUI_COLOR_PATTERN_HI_1";
    strings["Highlight 2##CC_GUI_COLOR_PATTERN_HI_2"].plurals[0] = "Подсветка 2##CC_GUI_COLOR_PATTERN_HI_2";
    strings["Row number##CC_GUI_COLOR_PATTERN_ROW_INDEX"].plurals[0] = "Номер строки##CC_GUI_COLOR_PATTERN_ROW_INDEX";
    strings["Row number (highlight 1)##CC_GUI_COLOR_PATTERN_ROW_INDEX_HI1"].plurals[0] = "Номер строки (подсветка 1)##CC_GUI_COLOR_PATTERN_ROW_INDEX_HI1";
    strings["Row number (highlight 2)##CC_GUI_COLOR_PATTERN_ROW_INDEX_HI2"].plurals[0] = "Номер строки (подсветка 2)##CC_GUI_COLOR_PATTERN_ROW_INDEX_HI2";
    strings["Note##CC_GUI_COLOR_PATTERN_ACTIVE"].plurals[0] = "Нота##CC_GUI_COLOR_PATTERN_ACTIVE";
    strings["Note (highlight 1)##CC_GUI_COLOR_PATTERN_ACTIVE_HI1"].plurals[0] = "Нота (подсветка 1)##CC_GUI_COLOR_PATTERN_ACTIVE_HI1";
    strings["Note (highlight 2)##CC_GUI_COLOR_PATTERN_ACTIVE_HI2"].plurals[0] = "Нота (подсветка 2)##CC_GUI_COLOR_PATTERN_ACTIVE_HI2";
    strings["Blank##CC_GUI_COLOR_PATTERN_INACTIVE"].plurals[0] = "Пустая ячейка##CC_GUI_COLOR_PATTERN_INACTIVE";
    strings["Blank (highlight 1)##CC_GUI_COLOR_PATTERN_INACTIVE_HI1"].plurals[0] = "Пустая ячейка (подсветка 1)##CC_GUI_COLOR_PATTERN_INACTIVE_HI1";
    strings["Blank (highlight 2)##CC_GUI_COLOR_PATTERN_INACTIVE_HI2"].plurals[0] = "Пустая ячейка (подсветка 2)##CC_GUI_COLOR_PATTERN_INACTIVE_HI2";
    strings["Instrument##CC_GUI_COLOR_PATTERN_INS"].plurals[0] = "Инструмент##CC_GUI_COLOR_PATTERN_INS";
    strings["Instrument (invalid type)##CC_GUI_COLOR_PATTERN_INS_WARN"].plurals[0] = "Инструмент (неправильный тип)##CC_GUI_COLOR_PATTERN_INS_WARN";
    strings["Instrument (out of range)##CC_GUI_COLOR_PATTERN_INS_ERROR"].plurals[0] = "Инструмент (индекс не соответствует существующему инструменту)##CC_GUI_COLOR_PATTERN_INS_ERROR";
    strings["Volume (0%)##CC_GUI_COLOR_PATTERN_VOLUME_MIN"].plurals[0] = "Громкость (0%)##CC_GUI_COLOR_PATTERN_VOLUME_MIN";
    strings["Volume (50%)##CC_GUI_COLOR_PATTERN_VOLUME_HALF"].plurals[0] = "Громкость (50%)##CC_GUI_COLOR_PATTERN_VOLUME_HALF";
    strings["Volume (100%)##CC_GUI_COLOR_PATTERN_VOLUME_MAX"].plurals[0] = "Громкость (100%)##CC_GUI_COLOR_PATTERN_VOLUME_MAX";
    strings["Invalid effect##CC_GUI_COLOR_PATTERN_EFFECT_INVALID"].plurals[0] = "Неправильный индекс эффекта##CC_GUI_COLOR_PATTERN_EFFECT_INVALID";
    strings["Pitch effect##CC_GUI_COLOR_PATTERN_EFFECT_PITCH"].plurals[0] = "Эффект (частота)##CC_GUI_COLOR_PATTERN_EFFECT_PITCH";
    strings["Volume effect##CC_GUI_COLOR_PATTERN_EFFECT_VOLUME"].plurals[0] = "Эффект (громкость)##CC_GUI_COLOR_PATTERN_EFFECT_VOLUME";
    strings["Panning effect##CC_GUI_COLOR_PATTERN_EFFECT_PANNING"].plurals[0] = "Эффект (панорамирование)##CC_GUI_COLOR_PATTERN_EFFECT_PANNING";
    strings["Song effect##CC_GUI_COLOR_PATTERN_EFFECT_SONG"].plurals[0] = "Эффект (трек)##CC_GUI_COLOR_PATTERN_EFFECT_SONG";
    strings["Time effect##CC_GUI_COLOR_PATTERN_EFFECT_TIME"].plurals[0] = "Эффект (время)##CC_GUI_COLOR_PATTERN_EFFECT_TIME";
    strings["Speed effect##CC_GUI_COLOR_PATTERN_EFFECT_SPEED"].plurals[0] = "Эффект (громкость)##CC_GUI_COLOR_PATTERN_EFFECT_SPEED";
    strings["Primary specific effect##CC_GUI_COLOR_PATTERN_EFFECT_SYS_PRIMARY"].plurals[0] = "Основной эффект чипа##CC_GUI_COLOR_PATTERN_EFFECT_SYS_PRIMARY";
    strings["Secondary specific effect##CC_GUI_COLOR_PATTERN_EFFECT_SYS_SECONDARY"].plurals[0] = "Вспомогательный эффект чипа##CC_GUI_COLOR_PATTERN_EFFECT_SYS_SECONDARY";
    strings["Miscellaneous##CC_GUI_COLOR_PATTERN_EFFECT_MISC"].plurals[0] = "Эффект (рзное)##CC_GUI_COLOR_PATTERN_EFFECT_MISC";
    strings["External command output##CC_GUI_COLOR_EE_VALUE"].plurals[0] = "Вывод внешней команды##CC_GUI_COLOR_EE_VALUE";
    strings["Status: off/disabled##CC_GUI_COLOR_PATTERN_STATUS_OFF"].plurals[0] = "Статус: выкл./отключён##CC_GUI_COLOR_PATTERN_STATUS_OFF";
    strings["Status: off + macro rel##CC_GUI_COLOR_PATTERN_STATUS_REL"].plurals[0] = "Статус: выкл. + релиз макроса##CC_GUI_COLOR_PATTERN_STATUS_REL";
    strings["Status: on + macro rel##CC_GUI_COLOR_PATTERN_STATUS_REL_ON"].plurals[0] = "Статус: вкл. + релиз макроса##CC_GUI_COLOR_PATTERN_STATUS_REL_ON";
    strings["Status: on##CC_GUI_COLOR_PATTERN_STATUS_ON"].plurals[0] = "Статус: вкл.##CC_GUI_COLOR_PATTERN_STATUS_ON";
    strings["Status: volume##CC_GUI_COLOR_PATTERN_STATUS_VOLUME"].plurals[0] = "Статус: громкость##CC_GUI_COLOR_PATTERN_STATUS_VOLUME";
    strings["Status: pitch##CC_GUI_COLOR_PATTERN_STATUS_PITCH"].plurals[0] = "Статус: частота##CC_GUI_COLOR_PATTERN_STATUS_PITCH";
    strings["Status: panning##CC_GUI_COLOR_PATTERN_STATUS_PANNING"].plurals[0] = "Статус: панорамирование##CC_GUI_COLOR_PATTERN_STATUS_PANNING";
    strings["Status: chip (primary)##CC_GUI_COLOR_PATTERN_STATUS_SYS1"].plurals[0] = "Статус: основной эффект чипа##CC_GUI_COLOR_PATTERN_STATUS_SYS1";
    strings["Status: chip (secondary)##CC_GUI_COLOR_PATTERN_STATUS_SYS2"].plurals[0] = "Статус: вспомогательный эффект чипа##CC_GUI_COLOR_PATTERN_STATUS_SYS2";
    strings["Status: mixing##CC_GUI_COLOR_PATTERN_STATUS_MIXING"].plurals[0] = "Статус: микширование##CC_GUI_COLOR_PATTERN_STATUS_MIXING";
    strings["Status: DSP effect##CC_GUI_COLOR_PATTERN_STATUS_DSP"].plurals[0] = "Статус: эффект ЦОС##CC_GUI_COLOR_PATTERN_STATUS_DSP";
    strings["Status: note altering##CC_GUI_COLOR_PATTERN_STATUS_NOTE"].plurals[0] = "Статус: изменение ноты##CC_GUI_COLOR_PATTERN_STATUS_NOTE";
    strings["Status: misc color 1##CC_GUI_COLOR_PATTERN_STATUS_MISC1"].plurals[0] = "Статус: разное (цвет 1)##CC_GUI_COLOR_PATTERN_STATUS_MISC1";
    strings["Status: misc color 2##CC_GUI_COLOR_PATTERN_STATUS_MISC2"].plurals[0] = "Статус: разное (цвет 2)##CC_GUI_COLOR_PATTERN_STATUS_MISC2";
    strings["Status: misc color 3##CC_GUI_COLOR_PATTERN_STATUS_MISC3"].plurals[0] = "Статус: разное (цвет 3)##CC_GUI_COLOR_PATTERN_STATUS_MISC3";
    strings["Status: attack##CC_GUI_COLOR_PATTERN_STATUS_ATTACK"].plurals[0] = "Статус: атака##CC_GUI_COLOR_PATTERN_STATUS_ATTACK";
    strings["Status: decay##CC_GUI_COLOR_PATTERN_STATUS_DECAY"].plurals[0] = "Статус: спад##CC_GUI_COLOR_PATTERN_STATUS_DECAY";
    strings["Status: sustain##CC_GUI_COLOR_PATTERN_STATUS_SUSTAIN"].plurals[0] = "Статус: сустейн##CC_GUI_COLOR_PATTERN_STATUS_SUSTAIN";
    strings["Status: release##CC_GUI_COLOR_PATTERN_STATUS_RELEASE"].plurals[0] = "Статус: релиз##CC_GUI_COLOR_PATTERN_STATUS_RELEASE";
    strings["Status: decrease linear##CC_GUI_COLOR_PATTERN_STATUS_DEC_LINEAR"].plurals[0] = "Статус: линейное уменьшение##CC_GUI_COLOR_PATTERN_STATUS_DEC_LINEAR";
    strings["Status: decrease exp##CC_GUI_COLOR_PATTERN_STATUS_DEC_EXP"].plurals[0] = "Статус: экспоненциальное уменьшение##CC_GUI_COLOR_PATTERN_STATUS_DEC_EXP";
    strings["Status: increase##CC_GUI_COLOR_PATTERN_STATUS_INC"].plurals[0] = "Статус: увеличение##CC_GUI_COLOR_PATTERN_STATUS_INC";
    strings["Status: bent##CC_GUI_COLOR_PATTERN_STATUS_BENT"].plurals[0] = "Статус: изменена частота##CC_GUI_COLOR_PATTERN_STATUS_BENT";
    strings["Status: direct##CC_GUI_COLOR_PATTERN_STATUS_DIRECT"].plurals[0] = "Статус: прямой##CC_GUI_COLOR_PATTERN_STATUS_DIRECT";

    strings["Background##CC_GUI_COLOR_SAMPLE_BG"].plurals[0] = "Фон##CC_GUI_COLOR_SAMPLE_BG";
    strings["Waveform##CC_GUI_COLOR_SAMPLE_FG"].plurals[0] = "Волна##CC_GUI_COLOR_SAMPLE_FG";
    strings["Time background##CC_GUI_COLOR_SAMPLE_TIME_BG"].plurals[0] = "Фон поля времени##CC_GUI_COLOR_SAMPLE_TIME_BG";
    strings["Time text##CC_GUI_COLOR_SAMPLE_TIME_FG"].plurals[0] = "Текст поля времени##CC_GUI_COLOR_SAMPLE_TIME_FG";
    strings["Loop region##CC_GUI_COLOR_SAMPLE_LOOP"].plurals[0] = "Зацикленная часть##CC_GUI_COLOR_SAMPLE_LOOP";
    strings["Center guide##CC_GUI_COLOR_SAMPLE_CENTER"].plurals[0] = "Центральная полоса##CC_GUI_COLOR_SAMPLE_CENTER";
    strings["Grid##CC_GUI_COLOR_SAMPLE_GRID"].plurals[0] = "Сетка##CC_GUI_COLOR_SAMPLE_GRID";
    strings["Selection##CC_GUI_COLOR_SAMPLE_SEL"].plurals[0] = "Выделение##CC_GUI_COLOR_SAMPLE_SEL";
    strings["Selection points##CC_GUI_COLOR_SAMPLE_SEL_POINT"].plurals[0] = "Границы выделения##CC_GUI_COLOR_SAMPLE_SEL_POINT";
    strings["Preview needle##CC_GUI_COLOR_SAMPLE_NEEDLE"].plurals[0] = "Курсор воспроизведения превью##CC_GUI_COLOR_SAMPLE_NEEDLE";
    strings["Playing needles##CC_GUI_COLOR_SAMPLE_NEEDLE_PLAYING"].plurals[0] = "Курсоры воспроизведения##CC_GUI_COLOR_SAMPLE_NEEDLE_PLAYING";
    strings["Loop markers##CC_GUI_COLOR_SAMPLE_LOOP_POINT"].plurals[0] = "Маркеры зацикливания##CC_GUI_COLOR_SAMPLE_LOOP_POINT";
    strings["Chip select: disabled##CC_GUI_COLOR_SAMPLE_CHIP_DISABLED"].plurals[0] = "Выбор чипа: выкл.##CC_GUI_COLOR_SAMPLE_CHIP_DISABLED";
    strings["Chip select: enabled##CC_GUI_COLOR_SAMPLE_CHIP_ENABLED"].plurals[0] = "Выбор чипа: вкл.##CC_GUI_COLOR_SAMPLE_CHIP_ENABLED";
    strings["Chip select: enabled (failure)##CC_GUI_COLOR_SAMPLE_CHIP_WARNING"].plurals[0] = "Выбор чипа: вкл. (ошибка)##CC_GUI_COLOR_SAMPLE_CHIP_WARNING";

    strings["Unallocated##CC_GUI_COLOR_PAT_MANAGER_NULL"].plurals[0] = "Паттерн не используется и память под него не выделена##CC_GUI_COLOR_PAT_MANAGER_NULL";
    strings["Unused##CC_GUI_COLOR_PAT_MANAGER_UNUSED"].plurals[0] = "Паттерн не используется##CC_GUI_COLOR_PAT_MANAGER_UNUSED";
    strings["Used##CC_GUI_COLOR_PAT_MANAGER_USED"].plurals[0] = "Паттерн используется##CC_GUI_COLOR_PAT_MANAGER_USED";
    strings["Overused##CC_GUI_COLOR_PAT_MANAGER_OVERUSED"].plurals[0] = "Паттерн используется очень часто##CC_GUI_COLOR_PAT_MANAGER_OVERUSED";
    strings["Really overused##CC_GUI_COLOR_PAT_MANAGER_EXTREMELY_OVERUSED"].plurals[0] = "Паттерн используется крайне часто##CC_GUI_COLOR_PAT_MANAGER_EXTREMELY_OVERUSED";
    strings["Combo Breaker##CC_GUI_COLOR_PAT_MANAGER_COMBO_BREAKER"].plurals[0] = "Число использований паттерна больше, чем длина трека!##CC_GUI_COLOR_PAT_MANAGER_COMBO_BREAKER";

    strings["Background##CC_GUI_COLOR_PIANO_BACKGROUND"].plurals[0] = "Фон##CC_GUI_COLOR_PIANO_BACKGROUND";
    strings["Upper key##CC_GUI_COLOR_PIANO_KEY_TOP"].plurals[0] = "Верхние клавиши##CC_GUI_COLOR_PIANO_KEY_TOP";
    strings["Upper key (feedback)##CC_GUI_COLOR_PIANO_KEY_TOP_HIT"].plurals[0] = "Верхние клавиши (обратная связь)##CC_GUI_COLOR_PIANO_KEY_TOP_HIT";
    strings["Upper key (pressed)##CC_GUI_COLOR_PIANO_KEY_TOP_ACTIVE"].plurals[0] = "Верхние клавиши (нажатые)##CC_GUI_COLOR_PIANO_KEY_TOP_ACTIVE";
    strings["Lower key##CC_GUI_COLOR_PIANO_KEY_BOTTOM"].plurals[0] = "Нижние клавиши##CC_GUI_COLOR_PIANO_KEY_BOTTOM";
    strings["Lower key (feedback)##CC_GUI_COLOR_PIANO_KEY_BOTTOM_HIT"].plurals[0] = "Нижние клавиши (обратная связь)##CC_GUI_COLOR_PIANO_KEY_BOTTOM_HIT";
    strings["Lower key (pressed)##CC_GUI_COLOR_PIANO_KEY_BOTTOM_ACTIVE"].plurals[0] = "Нижние клавиши (нажатые)##CC_GUI_COLOR_PIANO_KEY_BOTTOM_ACTIVE";

    strings["Clock text##CC_GUI_COLOR_CLOCK_TEXT"].plurals[0] = "Текст часов##CC_GUI_COLOR_CLOCK_TEXT";
    strings["Beat (off)##CC_GUI_COLOR_CLOCK_BEAT_LOW"].plurals[0] = "Слабая доля##CC_GUI_COLOR_CLOCK_BEAT_LOW";
    strings["Beat (on)##CC_GUI_COLOR_CLOCK_BEAT_HIGH"].plurals[0] = "Сильная доля##CC_GUI_COLOR_CLOCK_BEAT_HIGH";

    strings["PortSet##CC_GUI_COLOR_PATCHBAY_PORTSET"].plurals[0] = "Группа портов##CC_GUI_COLOR_PATCHBAY_PORTSET";
    strings["Port##CC_GUI_COLOR_PATCHBAY_PORT"].plurals[0] = "Порт##CC_GUI_COLOR_PATCHBAY_PORT";
    strings["Port (hidden/unavailable)##CC_GUI_COLOR_PATCHBAY_PORT_HIDDEN"].plurals[0] = "Порт (скрыт/недоступен)##CC_GUI_COLOR_PATCHBAY_PORT_HIDDEN";
    strings["Connection (selected)##CC_GUI_COLOR_PATCHBAY_CONNECTION"].plurals[0] = "Соединение (выделенное)##CC_GUI_COLOR_PATCHBAY_CONNECTION";
    strings["Connection (other)##CC_GUI_COLOR_PATCHBAY_CONNECTION_BG"].plurals[0] = "Соединение (обычное)##CC_GUI_COLOR_PATCHBAY_CONNECTION_BG";

    strings["Background##CC_GUI_COLOR_MEMORY_BG"].plurals[0] = "Фон##CC_GUI_COLOR_MEMORY_BG";
    strings["Unknown##CC_GUI_COLOR_MEMORY_FREE"].plurals[0] = "Неизв.##CC_GUI_COLOR_MEMORY_FREE";
    strings["Reserved##CC_GUI_COLOR_MEMORY_RESERVED"].plurals[0] = "Зарезерв.##CC_GUI_COLOR_MEMORY_RESERVED";
    strings["Sample##CC_GUI_COLOR_MEMORY_SAMPLE"].plurals[0] = "Сэмпл##CC_GUI_COLOR_MEMORY_SAMPLE";
    strings["Sample (alternate 1)##CC_GUI_COLOR_MEMORY_SAMPLE_ALT1"].plurals[0] = "Сэмпл (альтерн. 1)##CC_GUI_COLOR_MEMORY_SAMPLE_ALT1";
    strings["Sample (alternate 2)##CC_GUI_COLOR_MEMORY_SAMPLE_ALT2"].plurals[0] = "Сэмпл (альтерн. 2)##CC_GUI_COLOR_MEMORY_SAMPLE_ALT2";
    strings["Sample (alternate 3)##CC_GUI_COLOR_MEMORY_SAMPLE_ALT3"].plurals[0] = "Сэмпл (альтерн. 3)##CC_GUI_COLOR_MEMORY_SAMPLE_ALT3";
    strings["Wave RAM##CC_GUI_COLOR_MEMORY_WAVE_RAM"].plurals[0] = "ОЗУ волн##CC_GUI_COLOR_MEMORY_WAVE_RAM";
    strings["Wavetable (static)##CC_GUI_COLOR_MEMORY_WAVE_STATIC"].plurals[0] = "Волновая таблица (статич.)##CC_GUI_COLOR_MEMORY_WAVE_STATIC";
    strings["Echo buffer##CC_GUI_COLOR_MEMORY_ECHO"].plurals[0] = "Буфер эхо##CC_GUI_COLOR_MEMORY_ECHO";
    strings["Namco 163 load pos##CC_GUI_COLOR_MEMORY_N163_LOAD"].plurals[0] = "Namco 163 положение загрузки##CC_GUI_COLOR_MEMORY_N163_LOAD";
    strings["Namco 163 play pos##CC_GUI_COLOR_MEMORY_N163_PLAY"].plurals[0] = "Namco 163 положение проигрывания##CC_GUI_COLOR_MEMORY_N163_PLAY";
    strings["Sample (bank 0)##CC_GUI_COLOR_MEMORY_BANK0"].plurals[0] = "Сэмпл (банк 0)##CC_GUI_COLOR_MEMORY_BANK0";
    strings["Sample (bank 1)##CC_GUI_COLOR_MEMORY_BANK1"].plurals[0] = "Сэмпл (банк 1)##CC_GUI_COLOR_MEMORY_BANK1";
    strings["Sample (bank 2)##CC_GUI_COLOR_MEMORY_BANK2"].plurals[0] = "Сэмпл (банк 2)##CC_GUI_COLOR_MEMORY_BANK2";
    strings["Sample (bank 3)##CC_GUI_COLOR_MEMORY_BANK3"].plurals[0] = "Сэмпл (банк 3)##CC_GUI_COLOR_MEMORY_BANK3";
    strings["Sample (bank 4)##CC_GUI_COLOR_MEMORY_BANK4"].plurals[0] = "Сэмпл (банк 4)##CC_GUI_COLOR_MEMORY_BANK4";
    strings["Sample (bank 5)##CC_GUI_COLOR_MEMORY_BANK5"].plurals[0] = "Сэмпл (банк 5)##CC_GUI_COLOR_MEMORY_BANK5";
    strings["Sample (bank 6)##CC_GUI_COLOR_MEMORY_BANK6"].plurals[0] = "Сэмпл (банк 6)##CC_GUI_COLOR_MEMORY_BANK6";
    strings["Sample (bank 7)##CC_GUI_COLOR_MEMORY_BANK7"].plurals[0] = "Сэмпл (банк 7)##CC_GUI_COLOR_MEMORY_BANK7";

    strings["Log level: Error##CC_GUI_COLOR_LOGLEVEL_ERROR"].plurals[0] = "Уровень лога: ошибка##CC_GUI_COLOR_LOGLEVEL_ERROR";
    strings["Log level: Warning##CC_GUI_COLOR_LOGLEVEL_WARNING"].plurals[0] = "Уровень лога: предупреждение##CC_GUI_COLOR_LOGLEVEL_WARNING";
    strings["Log level: Info##CC_GUI_COLOR_LOGLEVEL_INFO"].plurals[0] = "Уровень лога: информация##CC_GUI_COLOR_LOGLEVEL_INFO";
    strings["Log level: Debug##CC_GUI_COLOR_LOGLEVEL_DEBUG"].plurals[0] = "Уровень лога: отладка##CC_GUI_COLOR_LOGLEVEL_DEBUG";
    strings["Log level: Trace/Verbose##CC_GUI_COLOR_LOGLEVEL_TRACE"].plurals[0] = "Уровень лога: диагностика/подробный##CC_GUI_COLOR_LOGLEVEL_TRACE";

    strings["Backup##sgse"].plurals[0] = "Резервное копирование";
    strings["Configuration##sgse1"].plurals[0] = "Настройки";
    strings["Enable backup system##sgse"].plurals[0] = "Включить резервное копирование";
    strings["Interval (in seconds)##sgse"].plurals[0] = "Интервал (в секундах)";
    strings["Backups per file##sgse"].plurals[0] = "Резервных копий каждого файла";
    strings["Backup Management##sgse"].plurals[0] = "Управление резервными копиями";
    strings["Purge before:##sgse"].plurals[0] = "Удалить все до даты:";
    strings["Go##PDate"].plurals[0] = "Удалить##PDate";
    strings["PB used##sgse"].plurals[0] = " ПиБ использовано";
    strings["TB used##sgse"].plurals[0] = " ТиБ использовано";
    strings["GB used##sgse"].plurals[0] = " ГиБ использовано";
    strings["MB used##sgse"].plurals[0] = " МиБ использовано";
    strings["KB used##sgse"].plurals[0] = " КиБ использовано";
    strings[" bytes used##sgse"].plurals[0] = " байт использовано";
    strings[" bytes used##sgse"].plurals[1] = " байта использовано";
    strings[" bytes used##sgse"].plurals[2] = " байтов использовано";
    strings["Refresh##sgse"].plurals[0] = "Обновить список";
    strings["Delete all##sgse"].plurals[0] = "Удалить все";
    strings["Name##sgse"].plurals[0] = "Название";
    strings["Size##sgse"].plurals[0] = "Размер посл. рез. копии";
    strings["Latest##sgse"].plurals[0] = "Посл. рез. копия";
    strings["P##sgse"].plurals[0] = " ПиБ";
    strings["T##sgse"].plurals[0] = " ТиБ";
    strings["G##sgse"].plurals[0] = " ГиБ";
    strings["M##sgse"].plurals[0] = " МиБ";
    strings["K##sgse"].plurals[0] = " КиБ";

    strings["OK##SettingsOK"].plurals[0] = "ОК##SettingsOK";
    strings["Cancel##SettingsCancel"].plurals[0] = "Отмена##SettingsCancel";
    strings["Apply##SettingsApply"].plurals[0] = "Применить##SettingsApply";

    strings["could not initialize audio!##sgse"].plurals[0] = "не смог инициализировать аудио!";
    strings["error while loading fonts! please check your settings.##sgse"].plurals[0] = "произошла ошибка при загрузке шрифтов! проверьте свои настройки.";
    strings["error while loading config! (%s)##sgse"].plurals[0] = "ошибка при загрузке файла настроек! (%s)";

    //src/gui/util.cpp

    strings["<nothing>##sgut"].plurals[0] = "<ничего>";
    strings["Unknown##sgut0"].plurals[0] = "Неизв.";
    strings["Unknown##sgut1"].plurals[0] = "Неизв.";
    
    //   sgiPCMA  src/gui/inst/adpcma.cpp

    strings["Macros##sgiPCMA"].plurals[0] = "Макросы";
    strings["Volume##sgiPCMA"].plurals[0] = "Громкость";
    strings["Global Volume##sgiPCMA"].plurals[0] = "Глобальная громкость";
    strings["Panning##sgiPCMA"].plurals[0] = "Панорамирование";
    strings["Phase Reset##sgiPCMA"].plurals[0]  = "Сброс фазы";

    //   sgiPCMB   src/gui/inst/adpcmb.cpp

    strings["Macros##sgiPCMB"].plurals[0] = "Макросы";
    strings["Volume##sgiPCMB"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiPCMB"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiPCMB"].plurals[0] = "Частота";
    strings["Panning##sgiPCMB"].plurals[0] = "Панорамирование";
    strings["Phase Reset##sgiPCMB"].plurals[0]  = "Сброс фазы";

    //   sgiSAMPLE src/gui/inst/amiga.cpp

    strings["Macros##sgiSAMPLE"].plurals[0] = "Макросы";
    strings["Volume##sgiSAMPLE"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiSAMPLE"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiSAMPLE"].plurals[0] = "Частота";
    strings["Panning##sgiSAMPLE"].plurals[0] = "Панорамирование";
    strings["Panning (left)##sgiSAMPLE"].plurals[0] = "Панорамирование (левый)";
    strings["Surround##sgiSAMPLE"].plurals[0] = "Окружающее звучание";
    strings["Panning (right)##sgiSAMPLE"].plurals[0] = "Панорамирование (правый)";
    strings["Waveform##sgiSAMPLE"].plurals[0] = "Волна";
    strings["Phase Reset##sgiSAMPLE"].plurals[0]  = "Сброс фазы";

    //   sgiAY     src/gui/inst/ay.cpp

    strings["Macros##sgiAY"].plurals[0] = "Макросы";
    strings["Volume##sgiAY"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiAY"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiAY"].plurals[0] = "Частота";
    strings["Noise Freq##sgiAY"].plurals[0] = "Частота шума";
    strings["Waveform##sgiAY"].plurals[0] = "Волна";
    strings["Raw Period##sgiAY"].plurals[0] = "Период (регистровое знач.)";
    strings["Raw Envelope Period##sgiAY"].plurals[0] = "Период огиб. (регистровое знач.)";
    strings["Phase Reset##sgiAY"].plurals[0]  = "Сброс фазы";
    strings["Envelope##sgiAY"].plurals[0] = "Огибающая";
    strings["AutoEnv Num##sgiAY"].plurals[0] = "Множ. част. авто-огиб.";
    strings["AutoEnv Den##sgiAY"].plurals[0] = "Дел. част. авто-огиб.";

    //   sgi8930   src/gui/inst/ay8930.cpp

    strings["Macros##sgi8930"].plurals[0] = "Макросы";
    strings["Volume##sgi8930"].plurals[0] = "Громкость";
    strings["Arpeggio##sgi8930"].plurals[0] = "Арпеджио";
    strings["Pitch##sgi8930"].plurals[0] = "Частота";
    strings["Noise Freq##sgi8930"].plurals[0] = "Частота шума";
    strings["Waveform##sgi8930"].plurals[0] = "Волна";
    strings["Raw Period##sgi8930"].plurals[0] = "Период (регистровое знач.)";
    strings["Raw Envelope Period##sgi8930"].plurals[0] = "Период огиб. (регистровое знач.)";
    strings["Phase Reset##sgi8930"].plurals[0]  = "Сброс фазы";
    strings["Duty##sgi8930"].plurals[0] = "Скважность";
    strings["Envelope##sgi8930"].plurals[0] = "Огибающая";
    strings["AutoEnv Num##sgi8930"].plurals[0] = "Множ. част. авто-огиб.";
    strings["AutoEnv Den##sgi8930"].plurals[0] = "Дел. част. авто-огиб.";
    strings["Noise AND Mask##sgi8930"].plurals[0] = "Маска шума (И)";
    strings["Noise OR Mask##sgi8930"].plurals[0] = "Маска шума (ИЛИ)";

    //   sgiB      src/gui/inst/beeper.cpp

    strings["Macros##sgiB"].plurals[0] = "Макросы";
    strings["Volume##sgiB"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiB"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiB"].plurals[0] = "Частота";
    strings["Pulse Width##sgiB"].plurals[0] = "Скважность";

    //   sgiBIFUR  src/gui/inst/bifurcator.cpp

    strings["Macros##sgiBIFUR"].plurals[0] = "Макросы";
    strings["Volume##sgiBIFUR"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiBIFUR"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiBIFUR"].plurals[0] = "Частота";
    strings["Parameter##sgiBIFUR"].plurals[0] = "Параметр";
    strings["Panning (left)##sgiBIFUR"].plurals[0] = "Панорамирование (левый)";
    strings["Panning (right)##sgiBIFUR"].plurals[0] = "Панорамирование (правый)";
    strings["Load Value##sgiBIFUR"].plurals[0] = "Загрузить значение";

    //   sgiC140   src/gui/inst/c140.cpp

    strings["Macros##sgiC140"].plurals[0] = "Макросы";
    strings["Volume##sgiC140"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiC140"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiC140"].plurals[0] = "Частота";
    strings["Panning (left)##sgiC140"].plurals[0] = "Панорамирование (левый)";
    strings["Panning (right)##sgiC140"].plurals[0] = "Панорамирование (правый)";
    strings["Phase Reset##sgiC140"].plurals[0] = "Сброс фазы";

    //   sgiC219   src/gui/inst/c219.cpp

    strings["Macros##sgiC219"].plurals[0] = "Макросы";
    strings["Volume##sgiC219"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiC219"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiC219"].plurals[0] = "Частота";
    strings["Control##sgiC219"].plurals[0] = "Управление";
    strings["Panning (left)##sgiC219"].plurals[0] = "Панорамирование (левый)";
    strings["Panning (right)##sgiC219"].plurals[0] = "Панорамирование (правый)";
    strings["Phase Reset##sgiC219"].plurals[0] = "Сброс фазы";

    //   sgiC64    src/gui/inst/c64.cpp

    strings["Waveform##sgiC640"].plurals[0] = "Волна";
    strings["tri##sgiC64"].plurals[0] = "треуг.";
    strings["saw##sgiC64"].plurals[0] = "пила";
    strings["pulse##sgiC64"].plurals[0] = "прямоуг.";
    strings["noise##sgiC64"].plurals[0] = "шум";
    strings["A##sgiC640"].plurals[0] = "А";
    strings["A##sgiC641"].plurals[0] = "А";
    strings["D##sgiC640"].plurals[0] = "С";
    strings["D##sgiC641"].plurals[0] = "С";
    strings["S##sgiC640"].plurals[0] = "С";
    strings["S##sgiC641"].plurals[0] = "С";
    strings["R##sgiC640"].plurals[0] = "Р";
    strings["R##sgiC641"].plurals[0] = "Р";
    strings["Envelope##sgiC640"].plurals[0] = "Огибающая";
    strings["Envelope##sgiC641"].plurals[0] = "Огибающая";
    strings["Duty##sgiC640"].plurals[0] = "Скважность";
    strings["Ring Modulation##sgiC64"].plurals[0] = "Кольцевая модуляция";
    strings["Oscillator Sync##sgiC64"].plurals[0] = "Синхронизация осцилляторов";
    strings["Enable filter##sgiC64"].plurals[0] = "Включить фильтр";
    strings["Initialize filter##sgiC64"].plurals[0] = "Инициализировать фильтр";
    strings["Cutoff##sgiC640"].plurals[0] = "Частота среза";
    strings["Resonance##sgiC640"].plurals[0] = "Резонанс (добротность)";
    strings["Filter Mode##sgiC640"].plurals[0] = "Тип фильтра";
    strings["low##sgiC64"].plurals[0] = "ФНЧ";
    strings["band##sgiC64"].plurals[0] = "ППФ";
    strings["high##sgiC64"].plurals[0] = "ФВЧ";
    strings["ch3off##sgiC64"].plurals[0] = "выкл. 3 кан.";
    strings["Absolute Cutoff Macro##sgiC64"].plurals[0] = "Абсолютный макрос частоты среза";
    strings["Absolute Duty Macro##sgiC64"].plurals[0] = "Абсолютный макрос скважности";
    strings["Don't test before new note##sgiC64"].plurals[0] = "Не включать тестовый бит перед новой нотой";
    strings["Macros##sgiC64"].plurals[0] = "Макросы";
    strings["Volume##sgiC64"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiC64"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiC64"].plurals[0] = "Частота";
    strings["Duty##sgiC641"].plurals[0] = "Скважность";
    strings["Waveform##sgiC641"].plurals[0] = "Волна";
    strings["Cutoff##sgiC641"].plurals[0] = "Частота среза";
    strings["Filter Mode##sgiC641"].plurals[0] = "Тип фильтра";
    strings["Filter Toggle##sgiC64"].plurals[0] = "Вкл./выкл. фильтр";
    strings["Resonance##sgiC641"].plurals[0] = "Резонанс";
    strings["Special##sgiC64"].plurals[0] = "Разное";
    strings["Attack##sgiC64"].plurals[0] = "Атака";
    strings["Decay##sgiC64"].plurals[0] = "Спад";
    strings["Sustain##sgiC64"].plurals[0] = "Сустейн";
    strings["Release##sgiC64"].plurals[0] = "Релиз";

    //   sgiDAVE   src/gui/inst/dave.cpp

    strings["Macros##sgiDAVE"].plurals[0] = "Макросы";
    strings["Volume##sgiDAVE"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiDAVE"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiDAVE"].plurals[0] = "Частота";
    strings["Noise Freq##sgiDAVE"].plurals[0] = "Частота шума";
    strings["Waveform##sgiDAVE"].plurals[0] = "Волна";
    strings["Panning (left)##sgiDAVE"].plurals[0] = "Панорамирование (левый)";
    strings["Panning (right)##sgiDAVE"].plurals[0] = "Панорамирование (правый)";
    strings["Phase Reset##sgiDAVE"].plurals[0] = "Сброс фазы";
    strings["Control##sgiDAVE"].plurals[0] = "Управление";
    strings["Raw Frequency##sgiDAVE"].plurals[0] = "Частота (регистровое знач.)";

    //   sgi5503   src/gui/inst/es5503.cpp

    strings["Oscillator mode:##sgi5503"].plurals[0] = "Режим осциллятора:";
    strings["Freerun##sgi5503"].plurals[0] = "Свободн. (зацикл.)";
    strings["Oneshot##sgi5503"].plurals[0] = "Однократн.";
    strings["Sync/AM##sgi5503"].plurals[0] = "Синхр./АМ";
    strings["Swap##sgi5503"].plurals[0] = "Смена";
    strings["Virtual softpan channel##sgi5503"].plurals[0] = "Виртуальный стерео-канал";
    strings["Combines odd and next even channel into one virtual channel with 256-step panning.\nInstrument, volume and effects need to be placed on the odd channel (e.g. 1st, 3rd, 5th etc.)##sgi5503"].plurals[0] = "Использует нечётный и следующий за ним чётный канал для создания виртуального канала с возможностью плавного панорамирования (256 шагов).\nНоты, инструменты, команды громкости и эффекты необходимо размещать в нечётном канале (1-ый, 3-ий и т.д.)";
    strings["Phase reset on key-on##sgi5503"].plurals[0] = "Сброс фазы в начале ноты";
    strings["Macros##sgi5503"].plurals[0] = "Макросы";
    strings["Volume##sgi5503"].plurals[0] = "Громкость";
    strings["Arpeggio##sgi5503"].plurals[0] = "Арпеджио";
    strings["Pitch##sgi5503"].plurals[0] = "Частота";
    strings["Osc. mode##sgi5503"].plurals[0] = "Реж. осцилл.";
    strings["Panning (left)##sgi5503"].plurals[0] = "Панорамирование (левый)";
    strings["Panning (right)##sgi5503"].plurals[0] = "Панорамирование (правый)";
    strings["Waveform##sgi5503"].plurals[0] = "Волна";
    strings["Phase Reset##sgi5503"].plurals[0] = "Сброс фазы";
    strings["Wave/sample pos.##sgi5503"].plurals[0] = "Полож. волны/сэмпла в памяти";
    strings["Osc. output##sgi5503"].plurals[0] = "Вывод осцилл.";

    //   sgiOTTO   src/gui/inst/es5506.cpp

    strings["Filter Mode##sgiOTTO0"].plurals[0] = "Режим фильтра";
    strings["Filter K1##sgiOTTO0"].plurals[0] = "K1 фильтра";
    strings["Filter K2##sgiOTTO0"].plurals[0] = "K2 фильтра";
    strings["Envelope length##sgiOTTO"].plurals[0] = "Длина огиб.";
    strings["Envelope count##sgiOTTO"].plurals[0] = "Скорость огиб.";
    strings["Left Volume Ramp##sgiOTTO"].plurals[0] = "Нараст. лев. громк.";
    strings["Right Volume Ramp##sgiOTTO"].plurals[0] = "Нараст. прав. громк.";
    strings["Filter K1 Ramp##sgiOTTO"].plurals[0] = "Нараст. K1 фильтра";
    strings["Filter K2 Ramp##sgiOTTO"].plurals[0] = "Нараст. K2 фильтра";
    strings["K1 Ramp Slowdown##sgiOTTO"].plurals[0] = "Замедл. нараст. K1";
    strings["K2 Ramp Slowdown##sgiOTTO"].plurals[0] = "Замедл. нараст. K2";
    strings["Macros##sgiOTTO"].plurals[0] = "Макросы";
    strings["Volume##sgiOTTO"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiOTTO"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiOTTO"].plurals[0] = "Частота";
    strings["Panning (left)##sgiOTTO"].plurals[0] = "Панорамирование (левый)";
    strings["Panning (right)##sgiOTTO"].plurals[0] = "Панорамирование (правый)";
    strings["Phase Reset##sgiOTTO"].plurals[0] = "Сброс фазы";
    strings["Filter Mode##sgiOTTO1"].plurals[0] = "Режим фильтра";
    strings["Filter K1##sgiOTTO1"].plurals[0] = "K1 фильтра";
    strings["Filter K2##sgiOTTO1"].plurals[0] = "K2 фильтра";
    strings["Outputs##sgiOTTO"].plurals[0] = "Выводы";
    strings["Control##sgiOTTO"].plurals[0] = "Управление";

    //   sgiESFM   src/gui/inst/esfm.cpp

    strings["Other##sgiESFM0"].plurals[0] = "Другое";
    strings["Other##sgiESFM1"].plurals[0] = "Другое";
    strings["Envelope##sgiESFM0"].plurals[0] = "Огибающая";
    strings["Envelope##sgiESFM1"].plurals[0] = "Огибающая";
    strings["op%d##sgiESFM0"].plurals[0] = "оп%d";
    strings["OP%d##sgiESFM1"].plurals[0] = "ОП%d";
    strings["Detune in semitones##sgiESFM0"].plurals[0] = "Расстройка в полутонах";
    strings["Detune in fractions of semitone.\n128 = +1 semitone, -128 = -1 semitone.##sgiESFM0"].plurals[0] = "Расстройка в долях полутона.\n128 = +1 полутон, -128 = -1 полутон.";
    strings["If operator outputs sound, enable left channel output.##sgiESFM0"].plurals[0] = "Включить вывод звука в левый канал.";
    strings["If operator outputs sound, enable right channel output.##sgiESFM0"].plurals[0] = "Включить вывод звука в правый канал.";
    strings["Block##sgiESFM0"].plurals[0] = "Блок";
    strings["FreqNum##sgiESFM0"].plurals[0] = "Частота";
    strings["op%d##sgiESFM2"].plurals[0] = "оп%d";
    strings["Operator %d##sgiESFM"].plurals[0] = "Оператор %d";
    strings["Waveform##sgiESFM"].plurals[0] = "Волна";
    strings["Envelope##sgiESFM"].plurals[0] = "Огибающая";
    strings["Blk##sgiESFM"].plurals[0] = "Блк";
    strings["Block##sgiESFM1"].plurals[0] = "Блок";
    strings["F##sgiESFM"].plurals[0] = "Ч";
    strings["Frequency (F-Num)##sgiESFM"].plurals[0] = "Частота (F-Num)";
    strings["Detune in semitones##sgiESFM1"].plurals[0] = "Расстройка в полутонах";
    strings["Detune in fractions of semitone.\n128 = +1 semitone, -128 = -1 semitone.##sgiESFM1"].plurals[0] = "Расстройка в долях полутона.\n128 = +1 полутон, -128 = -1 полутон.";
    strings["If operator outputs sound, enable left channel output.##sgiESFM1"].plurals[0] = "Включить вывод звука в левый канал.";
    strings["If operator outputs sound, enable right channel output.##sgiESFM1"].plurals[0] = "Включить вывод звука в правый канал.";
    strings["op%d##sgiESFM3"].plurals[0] = "оп%d";
    strings["OP%d##sgiESFM4"].plurals[0] = "ОП%d";
    strings["Block##sgiESFM2"].plurals[0] = "Блок";
    strings["FreqNum##sgiESFM1"].plurals[0] = "Частота";
    strings["Detune in semitones##sgiESFM2"].plurals[0] = "Расстройка в полутонах";
    strings["Detune in fractions of semitone.\n128 = +1 semitone, -128 = -1 semitone.##sgiESFM2"].plurals[0] = "Расстройка в долях полутона.\n128 = +1 полутон, -128 = -1 полутон.";
    strings["If operator outputs sound, enable left channel output.##sgiESFM2"].plurals[0] = "Включить вывод звука в левый канал.";
    strings["If operator outputs sound, enable right channel output.##sgiESFM2"].plurals[0] = "Включить вывод звука в правый канал.";
    strings["OP%d Macros##sgiESFM"].plurals[0] = "Макросы ОП%d";
    strings["Block##sgiESFM3"].plurals[0] = "Блок";
    strings["FreqNum##sgiESFM2"].plurals[0] = "Частота";
    strings["Op. Arpeggio##sgiESFM"].plurals[0] = "Арпеджио оператора";
    strings["Op. Pitch##sgiESFM"].plurals[0] = "Частота оператора";
    strings["Op. Panning##sgiESFM"].plurals[0] = "Панорамирование оператора";
    strings["Macros##sgiESFM"].plurals[0] = "Макросы";
    strings["Volume##sgiESFM"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiESFM"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiESFM"].plurals[0] = "Частота";
    strings["OP4 Noise Mode##sgiESFM"].plurals[0] = "Режим шума ОП4";
    strings["Panning##sgiESFM"].plurals[0] = "Панорамирование";
    strings["Phase Reset##sgiESFM"].plurals[0] = "Сброс фазы";

    //   sgiFDS    src/gui/inst/fds.cpp

    strings["Compatibility mode##sgiFDS"].plurals[0] = "Режим совместимости";
    strings["only use for compatibility with .dmf modules!\n- initializes modulation table with first wavetable\n- does not alter modulation parameters on instrument change##sgiFDS"].plurals[0] = "только для совместимости с модулями .dmf!\n- инициализирует волновую таблицу модулятора первой волновой таблицей\n- не изменяет параметры модуляции при смене инструмента";
    strings["Modulation depth##sgiFDS"].plurals[0] = "Глубина модуляции";
    strings["Modulation speed##sgiFDS"].plurals[0] = "Скорость модуляции";
    strings["Modulation table##sgiFDS"].plurals[0] = "Волновая таблица модуляции";
    strings["Macros##sgiFDS"].plurals[0] = "Макросы";
    strings["Volume##sgiFDS"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiFDS"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiFDS"].plurals[0] = "Частота";
    strings["Waveform##sgiFDS"].plurals[0] = "Волна";
    strings["Mod Depth##sgiFDS"].plurals[0] = "Глуб. мод.";
    strings["Mod Speed##sgiFDS"].plurals[0] = "Скор. мод.";
    strings["Mod Position##sgiFDS"].plurals[0] = "Полож. мод.";

    //   sgifmeu   src/gui/inst/fmEnvUtil.cpp

    strings["left click to restart\nmiddle click to pause\nright click to see algorithm##sgifmeu"].plurals[0] = "ЛКМ для перезапуска\nНажмите колёсико, чтобы поставить на паузу\nПКМ для переключения на визуализацию алгоритма";
    strings["left click to configure TL scaling\nright click to see FM preview##sgifmeu"].plurals[0] = "ЛКМ для конфигурации масштабирования громкости операторов\nПКМ для переключения на превью FM-сигнала";
    strings["right click to see FM preview##sgifmeu"].plurals[0] = "ПКМ для переключения на превью FM-сигнала";
    strings["operator level changes with volume?##sgifmeu"].plurals[0] = "Громкость оператора зависит от общей громкости?";
    strings["AUTO##OPKVS"].plurals[0] = "АВТО##OPKVS";
    strings["NO##OPKVS"].plurals[0] = "НЕТ##OPKVS";
    strings["YES##OPKVS"].plurals[0] = "ДА##OPKVS";

    //   sgifmeh   src/gui/inst/fmEnvUtil.h

    strings["(copying)##sgifmeh"].plurals[0] = "(копирование)";
    strings["(swapping)##sgifmeh"].plurals[0] = "(замена)";
    strings["- drag to swap operator\n- shift-drag to copy operator##sgifmeh"].plurals[0] = "- перетащите, чтобы поменять настройки операторов местами\n- перетащите с зажатой клавишей SHIFT, чтобы скопировать настройки оператора";

    //   sgiFZT    src/gui/inst/fzt.cpp

    strings["Base note##sgiFZT"].plurals[0] = "Базовая нота";
    strings["Finetune##sgiFZT"].plurals[0] = "Расстройка";
    strings["Slide speed##sgiFZT"].plurals[0] = "Скорость слайда";
    strings["Set PW##sgiFZT"].plurals[0] = "Уст. скважн.";
    strings["Set pulse width on keydown##sgiFZT"].plurals[0] = "Установить скважность на новой ноте";
    strings["Initial pulse width##sgiFZT"].plurals[0] = "Начальная скважность";
    strings["Set cutoff##sgiFZT"].plurals[0] = "Устан. част. среза";
    strings["Set filter cutoff on keydown##sgiFZT"].plurals[0] = "Установить частоту среза фильтра на новой ноте";
    strings["Waveform##sgiFZT"].plurals[0] = "Волна";
    strings["noise##sgiFZT"].plurals[0] = "шум";
    strings["pulse##sgiFZT"].plurals[0] = "прямоуг.";
    strings["triangle##sgiFZT"].plurals[0] = "треуг.";
    strings["saw##sgiFZT"].plurals[0] = "пила";
    strings["metal##sgiFZT"].plurals[0] = "\"металл.\"";
    strings["sine##sgiFZT"].plurals[0] = "синус";
    strings["Enable filter##sgiFZT"].plurals[0] = "Включить фильтр";
    strings["Cutoff##sgiFZT"].plurals[0] = "Частота среза";
    strings["Resonance##sgiFZT"].plurals[0] = "Резонанс";
    strings["Type##sgiFZT"].plurals[0] = "Тип";
    strings["Enable ring modulation##sgiFZT"].plurals[0] = "Включить кольцевую модуляцию";
    strings["Ring mod source##sgiFZT"].plurals[0] = "Источник кольцевой модуляции";
    strings["FF = self-modulation##sgiFZT"].plurals[0] = "FF = самомодуляция";
    strings["Enable hard sync##sgiFZT"].plurals[0] = "Включить синхронизацию осцилляторов";
    strings["Hard sync source##sgiFZT"].plurals[0] = "Источник синхронизации";
    strings["FF = self-sync##sgiFZT"].plurals[0] = "FF = самосинхронизация";
    strings["Retrigger on slide##sgiFZT"].plurals[0] = "Перезапуск на слайде";
    strings["Restart instrument and envelope even if slide command (03xx) is placed with the note.##sgiFZT"].plurals[0] = "Перезапустить инструмент и огибающую, даже если у ноты стоит эффект слайда (03xx).";
    strings["Sync osc. on keydown##sgiFZT"].plurals[0] = "Синхр. осц. на новой ноте";
    strings["Reset phase of oscillator each time new note is played.\nDoes not happen when slide (03xx) or legato command is placed.##sgiFZT"].plurals[0] = "Сбросить фазу осциллятора на каждой новой ноте.\nНе выполняется при наличии команды слайда (03xx) или легато.";
    strings["Vibrato##sgiFZT"].plurals[0] = "Вибрато";
    strings["Speed##sgiFZT0"].plurals[0] = "Скорость##sgiFZT0";
    strings["Depth##sgiFZT0"].plurals[0] = "Глубина##sgiFZT0";
    strings["Delay##sgiFZT0"].plurals[0] = "Задержка##sgiFZT0";
    strings["PWM##sgiFZT"].plurals[0] = "ШИМ";
    strings["Speed##sgiFZT1"].plurals[0] = "Скорость##sgiFZT1";
    strings["Depth##sgiFZT1"].plurals[0] = "Глубина##sgiFZT1";
    strings["Delay##sgiFZT1"].plurals[0] = "Задержка##sgiFZT1";
    strings["A##sgiFZT"].plurals[0] = "А";
    strings["D##sgiFZT"].plurals[0] = "С";
    strings["S##sgiFZT"].plurals[0] = "С";
    strings["R##sgiFZT"].plurals[0] = "Р";
    strings["VOL##sgiFZT"].plurals[0] = "ГР";
    strings["Envelope##sgiFZT"].plurals[0] = "Огибающая";
    strings["Instrument program##sgiFZT"].plurals[0] = "Программа инструмента";
    strings["Program period##sgiFZT22"].plurals[0] = "Период программы";
    strings["Do not restart instrument program on keydown##sgiFZT"].plurals[0] = "Не перезапускать программу инструмента на новой ноте";
    strings["Tick##sgiFZT"].plurals[0] = "Шаг";
    strings["Command##sgiFZT"].plurals[0] = "Команда";
    strings["Move/Remove##sgiFZT"].plurals[0] = "Перем./Удал.";
    strings["Unite##sgiFZT"].plurals[0] = "Объед.";
    strings["Value##sgiFZT"].plurals[0] = "Значение";
    strings["Semitones##sgiFZT"].plurals[0] = "Полутонов";
    strings["First external arpeggio note##sgiFZT"].plurals[0] = "Первая внешняя нота арпеджио";
    strings["Second external arpeggio note##sgiFZT1"].plurals[0] = "Вторая внешняя нота арпеджио";
    strings["Speed##sgiFZT2"].plurals[0] = "Скорость";
    strings["Depth##sgiFZT11"].plurals[0] = "Глубина";
    strings["Value##sgiFZT"].plurals[0] = "Значение";
    strings["Up##sgiFZT2"].plurals[0] = "Вверх";
    strings["Down##sgiFZT11"].plurals[0] = "Вниз";
    strings["Source channel##sgiFZT"].plurals[0] = "Канал-источник";
    strings["Source is self##sgiFZT"].plurals[0] = "Текущий канал является источником";
    strings["Loops##sgiFZT"].plurals[0] = "Циклов";
    strings["Step to jump to##sgiFZT"].plurals[0] = "Шаг программы, на который совершается переход";
    strings["No operation##sgiFZT"].plurals[0] = "Ничего не делать";
    strings["Program end##sgiFZT"].plurals[0] = "Конец программы";
    strings["Execute next command at the same tick##sgiFZT"].plurals[0] = "Выполнить следующую команду в этом же шаге";
    strings["Macros##sgiFZT"].plurals[0] = "Макросы";
    strings["Warning! Macros are NOT supported by FZT file format! Do not use them if you want to export .fzt file!##sgiFZT"].plurals[0] = "Внимание! Макросы НЕ ПОДДЕРЖИВАЮТСЯ форматом файла FZT! Не используйте их, если хотите экспортировать модуль в файл .fzt!";
    strings["Volume##sgiFZT"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiFZT"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiFZT"].plurals[0] = "Частота";
    strings["Duty##sgiFZT"].plurals[0] = "Скважность";
    strings["Waveform##sgiFZT"].plurals[0] = "Волна";
    strings["Cutoff##sgiFZT"].plurals[0] = "Частота среза";
    strings["Filter Mode##sgiFZT"].plurals[0] = "Режим фильтра";
    strings["Filter Toggle##sgiFZT"].plurals[0] = "Фильтр вкл./выкл.";
    strings["Resonance##sgiFZT"].plurals[0] = "Резонанс";
    strings["Phase Reset##sgiFZT"].plurals[0] = "Сброс фазы";
    strings["Envelope Reset/Key Control##sgiFZT"].plurals[0] = "Упр. огибающей";
    strings["Ring mod toggle##sgiFZT"].plurals[0] = "Кольц. мод. вкл./выкл.";
    strings["Ring mod source##sgiFZT"].plurals[0] = "Источник кольц. мод.";
    strings["Hard sync toggle##sgiFZT"].plurals[0] = "Синхр. осц. вкл./выкл.";
    strings["Hard sync source##sgiFZT"].plurals[0] = "Источник синхр. осц.";
    strings["Attack##sgiFZT"].plurals[0] = "Атака";
    strings["Decay##sgiFZT"].plurals[0] = "Спад";
    strings["Sustain##sgiFZT"].plurals[0] = "Сустейн";
    strings["Release##sgiFZT"].plurals[0] = "Релиз";

    //   sgiGA20   src/gui/inst/ga20.cpp

    strings["Macros##sgiGA20"].plurals[0] = "Макросы";
    strings["Volume##sgiGA20"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiGA20"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiGA20"].plurals[0] = "Частота";
    strings["Phase Reset##sgiGA20"].plurals[0] = "Сброс фазы";
    
    //   sgiGB     src/gui/inst/gb.cpp

    strings["Game Boy##sgiGB"].plurals[0] = "Game Boy";
    strings["Use software envelope##sgiGB"].plurals[0] = "Использовать программную огибающую громкости";
    strings["Initialize envelope on every note##sgiGB"].plurals[0] = "Инициализировать огибающую на каждой ноте";
    strings["Volume##sgiGB0"].plurals[0] = "Громкость";
    strings["Length##sgiGB"].plurals[0] = "Длина";
    strings["Sound Length##sgiGB0"].plurals[0] = "Длина звука";
    strings["Infinity##sgiGB"].plurals[0] = "Бесконечность";
    strings["Direction##sgiGB"].plurals[0] = "Направление";
    strings["Up##sgiGB0"].plurals[0] = "Вверх";
    strings["Down##sgiGB0"].plurals[0] = "Вниз";
    strings["Hardware Sequence##sgiGB"].plurals[0] = "Аппаратная последовательность";
    strings["Tick##sgiGB"].plurals[0] = "Шаг движка";
    strings["Command##sgiGB"].plurals[0] = "Команда";
    strings["Move/Remove##sgiGB"].plurals[0] = "Перем./Удал.";
    strings["Volume##sgiGB1"].plurals[0] = "Громкость";
    strings["Env Length##sgiGB"].plurals[0] = "Длина огибающей";
    strings["Sound Length##sgiGB1"].plurals[0] = "Длина звука";
    strings["Up##sgiGB1"].plurals[0] = "Вверх";
    strings["Down##sgiGB1"].plurals[0] = "Вниз";
    strings["Shift##sgiGB"].plurals[0] = "На сколько";
    strings["Speed##sgiGB"].plurals[0] = "Скорость";
    strings["Up##sgiGB2"].plurals[0] = "Вверх";
    strings["Down##sgiGB2"].plurals[0] = "Вниз";
    strings["Ticks##sgiGB"].plurals[0] = "Шагов";
    strings["Position##sgiGB"].plurals[0] = "Положение";
    strings["Macros##sgiGB"].plurals[0] = "Макросы";
    strings["Volume##sgiGB2"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiGB"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiGB"].plurals[0] = "Частота";
    strings["Duty/Noise##sgiGB"].plurals[0] = "Скважность/режим шума";
    strings["Waveform##sgiGB"].plurals[0] = "Волна";
    strings["Panning##sgiGB"].plurals[0] = "Панорамирование";
    strings["Phase Reset##sgiGB"].plurals[0] = "Сброс фазы";

    //   sgiGBADMA src/gui/inst/gbadma.cpp

    strings["Macros##sgiGBADMA"].plurals[0] = "Макросы";
    strings["Volume##sgiGBADMA"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiGBADMA"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiGBADMA"].plurals[0] = "Частота";
    strings["Waveform##sgiGBADMA"].plurals[0] = "Волна";
    strings["Panning##sgiGBADMA"].plurals[0] = "Панорамирование";
    strings["Phase Reset##sgiGBADMA"].plurals[0] = "Сброс фазы";

    //sgiGBAMINMOD src/gui/inst/gbaminmod.cpp

    strings["Macros##sgiGBAMINMOD"].plurals[0] = "Макросы";
    strings["Volume##sgiGBAMINMOD"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiGBAMINMOD"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiGBAMINMOD"].plurals[0] = "Частота";
    strings["Waveform##sgiGBAMINMOD"].plurals[0] = "Волна";
    strings["Panning (left)##sgiGBAMINMOD"].plurals[0] = "Панорамирование (левый)";
    strings["Panning (right)##sgiGBAMINMOD"].plurals[0] = "Панорамирование (правый)";
    strings["Special##sgiGBAMINMOD"].plurals[0] = "Разное";
    strings["Phase Reset##sgiGBAMINMOD"].plurals[0] = "Сброс фазы";

    //   sgiK00    src/gui/inst/k007232.cpp

    strings["Macros##sgiK00"].plurals[0] = "Макросы";
    strings["Volume##sgiK00"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiK00"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiK00"].plurals[0] = "Частота";
    strings["Waveform##sgiK00"].plurals[0] = "Волна";
    strings["Panning (left)##sgiK00"].plurals[0] = "Панорамирование (левый)";
    strings["Panning (right)##sgiK00"].plurals[0] = "Панорамирование (правый)";
    strings["Phase Reset##sgiK00"].plurals[0] = "Сброс фазы";

    //   sgiK05    src/gui/inst/k053260.cpp

    strings["Macros##sgiK05"].plurals[0] = "Макросы";
    strings["Volume##sgiK05"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiK05"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiK05"].plurals[0] = "Частота";
    strings["Panning##sgiK05"].plurals[0] = "Панорамирование";
    strings["Phase Reset##sgiK05"].plurals[0] = "Сброс фазы";

    //   sgimcd    src/gui/inst/macroDraw.cpp

    strings["Triangle##sgimcd"].plurals[0] = "Треуг. волна";
    strings["Saw##sgimcd"].plurals[0] = "Пила";
    strings["Square##sgimcd"].plurals[0] = "Меандр";
    strings["How did you even##sgimcd0"].plurals[0] = "Как вы вообще смогли";
    strings["Bottom##sgimcd0"].plurals[0] = "Мин.";
    strings["Top##sgimcd0"].plurals[0] = "Макс.";
    strings["Attack##sgimcd"].plurals[0] = "Атака";
    strings["Sustain##sgimcd"].plurals[0] = "Сустейн";
    strings["Hold##sgimcd"].plurals[0] = "Задержка после атаки";
    strings["SusTime##sgimcd"].plurals[0] = "Время сустейна";
    strings["Decay##sgimcd"].plurals[0] = "Спад";
    strings["SusDecay##sgimcd"].plurals[0] = "Спад сустейна";
    strings["Release##sgimcd"].plurals[0] = "Релиз";
    strings["Bottom##sgimcd1"].plurals[0] = "Мин.";
    strings["Top##sgimcd1"].plurals[0] = "Макс.";
    strings["Speed##sgimcd"].plurals[0] = "Скорость";
    strings["Phase##sgimcd"].plurals[0] = "Фаза";
    strings["Shape##sgimcd"].plurals[0] = "Форма волны";
    strings["Macro type: Sequence##sgimcd"].plurals[0] = "Тип макроса: последовательность";
    strings["Macro type: ADSR##sgimcd"].plurals[0] = "Тип макроса: ADSR-огибающая";
    strings["Macro type: LFO##sgimcd"].plurals[0] = "Тип макроса: ГНЧ";
    strings["Macro type: What's going on here?##sgimcd"].plurals[0] = "Тип макроса: Что здесь происходит?";
    strings["Delay/Step Length##sgimcd"].plurals[0] = "Задержка/длина шага";
    strings["Step Length (ticks)##IMacroSpeed"].plurals[0] = "Длина шага (в шагах движка трекера)##IMacroSpeed";
    strings["Delay##IMacroDelay"].plurals[0] = "Задержка##IMacroDelay";
    strings["Release mode: Active (jump to release pos)##sgimcd"].plurals[0] = "Тип релиза: активный (прыгнуть на позицию релиза)";
    strings["Release mode: Passive (delayed release)##sgimcd"].plurals[0] = "Тип релиза: пассивный (отложенный релиз)";
    strings["Tabs##sgimcd"].plurals[0] = "Вкладки";
    strings["Length##sgimcd"].plurals[0] = "Длина";
    strings["StepLen##sgimcd"].plurals[0] = "Дл. шага";
    strings["Delay##sgimcd"].plurals[0] = "Задержка";
    strings["The heck? No, this isn't even working correctly...##sgimcd"].plurals[0] = "Что за?.. Нет, оно ведь работает неправильно...";
    strings["The only problem with that selectedMacro is that it's a bug...##sgimcd"].plurals[0] = "Единственная проблема с тем selectedMacro в том, что это не сбой...";
    strings["Single (combo box)##sgimcd"].plurals[0] = "Один (выпадающий список)";

    //   sgiLYNX   src/gui/inst/mikey.cpp

    strings["Macros##sgiLYNX"].plurals[0] = "Макросы";
    strings["Volume##sgiLYNX"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiLYNX"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiLYNX"].plurals[0] = "Частота";
    strings["Duty/Int##sgiLYNX"].plurals[0] = "Скважность/инт.";
    strings["Panning (left)##sgiLYNX"].plurals[0] = "Панорамирование (левый)";
    strings["Panning (right)##sgiLYNX"].plurals[0] = "Панорамирование (правый)";
    strings["Phase Reset##sgiLYNX"].plurals[0] = "Сброс фазы";
    strings["Load LFSR##sgiLYNX"].plurals[0] = "Загрузить в РСЛОС";

    //   sgi5232   src/gui/inst/msm5232.cpp

    strings["Macros##sgi5232"].plurals[0] = "Макросы";
    strings["Volume##sgi5232"].plurals[0] = "Громкость";
    strings["Arpeggio##sgi5232"].plurals[0] = "Арпеджио";
    strings["Pitch##sgi5232"].plurals[0] = "Частота";
    strings["Group Ctrl##sgi5232"].plurals[0] = "Контроль группы";
    strings["Group Attack##sgi5232"].plurals[0] = "Атака группы";
    strings["Group Decay##sgi5232"].plurals[0] = "Спад группы";
    strings["Noise##sgi5232"].plurals[0] = "Шум";

    //   sgi6258   src/gui/inst/msm6258.cpp

    strings["Macros##sgi6258"].plurals[0] = "Макросы";
    strings["Frequency Divider##sgi6258"].plurals[0] = "Делитель частоты";
    strings["Panning##sgi6258"].plurals[0] = "Панорамирование";
    strings["Phase Reset##sgi6258"].plurals[0] = "Сброс фазы";
    strings["Clock Divider##sgi6258"].plurals[0] = "Делитель тактовой частоты";

    //   sgi6295   src/gui/inst/msm6295.cpp

    strings["Macros##sgi6295"].plurals[0] = "Макросы";
    strings["Volume##sgi6295"].plurals[0] = "Громкость";
    strings["Frequency Divider##sgi6295"].plurals[0] = "Делитель частоты";
    strings["Phase Reset##sgi6295"].plurals[0] = "Сброс фазы";

    //   sgiMULPCM src/gui/inst/multipcm.cpp

    strings["MultiPCM##sgiMULPCM"].plurals[0] = "MultiPCM";
    strings["AR##sgiMULPCM0"].plurals[0] = "СА";
    strings["AR##sgiMULPCM1"].plurals[0] = "СА";
    strings["Attack Rate##sgiMULPCM"].plurals[0] = "Скорость атаки";
    strings["D1R##sgiMULPCM0"].plurals[0] = "С1С";
    strings["D1R##sgiMULPCM1"].plurals[0] = "С1С";
    strings["Decay 1 Rate##sgiMULPCM"].plurals[0] = "Скорость спада 1";
    strings["DL##sgiMULPCM0"].plurals[0] = "УС";
    strings["DL##sgiMULPCM1"].plurals[0] = "УС";
    strings["Decay Level##sgiMULPCM"].plurals[0] = "Уровень спада";
    strings["D2R##sgiMULPCM0"].plurals[0] = "С2С";
    strings["D2R##sgiMULPCM1"].plurals[0] = "С2С";
    strings["Decay 2 Rate##sgiMULPCM"].plurals[0] = "Скорость спада 2";
    strings["RR##sgiMULPCM0"].plurals[0] = "СР";
    strings["RR##sgiMULPCM1"].plurals[0] = "СР";
    strings["Release Rate##sgiMULPCM"].plurals[0] = "Скорость релиза";
    strings["RC##sgiMULPCM0"].plurals[0] = "КС";
    strings["RC##sgiMULPCM1"].plurals[0] = "КС";
    strings["Rate Correction##sgiMULPCM"].plurals[0] = "Коррекция скорости";
    strings["Envelope##sgiMULPCM0"].plurals[0] = "Огибающая";
    strings["Envelope##sgiMULPCM1"].plurals[0] = "Огибающая";
    strings["LFO Rate##sgiMULPCM"].plurals[0] = "Скорость ГНЧ";
    strings["PM Depth##sgiMULPCM"].plurals[0] = "Глубина ФМ";
    strings["AM Depth##sgiMULPCM"].plurals[0] = "Глубина АМ";
    strings["Macros##sgiMULPCM"].plurals[0] = "Макросы";
    strings["Volume##sgiMULPCM"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiMULPCM"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiMULPCM"].plurals[0] = "Частота";
    strings["Panning##sgiMULPCM"].plurals[0] = "Панорамирование";
    strings["Phase Reset##sgiMULPCM"].plurals[0] = "Сброс фазы";

    //   sgiN163   src/gui/inst/n163.cpp

    strings["Namco 163##sgiN163"].plurals[0] = "Namco 163";
    strings["Load waveform##sgiN163"].plurals[0] = "Загрузить волну";
    strings["when enabled, a waveform will be loaded into RAM.\nwhen disabled, only the offset and length change.##sgiN163"].plurals[0] = "при включении этой опции олна будет загружена в ОЗУ.\nесли опция выключена, меняться будут только смещение и длина волны.";
    strings["Waveform##WAVE"].plurals[0] = "Волна##WAVE";
    strings["Per-channel wave position/length##sgiN163"].plurals[0] = "Позиционирование и длина волны отдельно для каждого канала";
    strings["Ch##sgiN163"].plurals[0] = "Кан.";
    strings["Position##sgiN163"].plurals[0] = "Положение";
    strings["Length##sgiN163"].plurals[0] = "Длина";
    strings["Position##WAVEPOS"].plurals[0] = "Положение##WAVEPOS";
    strings["Length##WAVELEN"].plurals[0] = "Длина##WAVELEN";
    strings["Macros##sgiN163"].plurals[0] = "Макросы";
    strings["Volume##sgiN163"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiN163"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiN163"].plurals[0] = "Частота";
    strings["Wave Pos##sgiN163"].plurals[0] = "Полож. волны";
    strings["Waveform##sgiN163"].plurals[0] = "Волна";
    strings["Wave Length##sgiN163"].plurals[0] = "Длина волны";

    //   sgiWSG    src/gui/inst/namco.cpp

    strings["Macros##sgiWSG"].plurals[0] = "Макросы";
    strings["Volume##sgiWSG"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiWSG"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiWSG"].plurals[0] = "Частота";
    strings["Noise##sgiWSG"].plurals[0] = "Шум";
    strings["Waveform##sgiWSG"].plurals[0] = "Волна";
    strings["Panning (left)##sgiWSG"].plurals[0] = "Панорамирование (левый)";
    strings["Panning (right)##sgiWSG"].plurals[0] = "Панорамирование (правый)";

    //   sgiNDS    src/gui/inst/nds.cpp

    strings["Macros##sgiNDS"].plurals[0] = "Макросы";
    strings["Volume##sgiNDS"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiNDS"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiNDS"].plurals[0] = "Частота";
    strings["Duty##sgiNDS"].plurals[0] = "Скважность";
    strings["Panning##sgiNDS"].plurals[0] = "Панорамирование";
    strings["Phase Reset##sgiNDS"].plurals[0] = "Сброс фазы";

    //   sgiNES    src/gui/inst/nes.cpp

    strings["Macros##sgiNES"].plurals[0] = "Макросы";
    strings["Volume##sgiNES"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiNES"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiNES"].plurals[0] = "Частота";
    strings["Duty/Noise##sgiNES"].plurals[0] = "Скважность/режим шума";
    strings["Phase Reset##sgiNES"].plurals[0] = "Сброс фазы";

    //   sgiOPL    src/gui/inst/opl.cpp

    strings["4-op##sgiOPL"].plurals[0] = "4-оп";
    strings["Drums##sgiOPL"].plurals[0] = "Ударные";
    strings["Fixed frequency mode##sgiOPL"].plurals[0] = "Режим фиксированной частоты";
    strings["when enabled, drums will be set to the specified frequencies, ignoring the note.##sgiOPL"].plurals[0] = "при включении режима ударные будут звучать на указанных частотах. Значения нот будут игнорироваться.";
    strings["Drum##sgiOPL"].plurals[0] = "Ударный";
    strings["Block##sgiOPL"].plurals[0] = "Блок";
    strings["FreqNum##sgiOPL"].plurals[0] = "Частота";
    strings["Kick##sgiOPL0"].plurals[0] = "Бас-барабан";
    strings["Snare/Hi-hat##sgiOPL"].plurals[0] = "Рабочий барабан/хай-хэт";
    strings["Tom/Top##sgiOPL"].plurals[0] = "Там-там/тарелка";
    strings["Other##sgiOPL0"].plurals[0] = "Другое";
    strings["Other##sgiOPL1"].plurals[0] = "Другое";
    strings["Envelope##sgiOPL0"].plurals[0] = "Огибающая";
    strings["Envelope##sgiOPL1"].plurals[0] = "Огибающая";
    strings["op%d##sgiOPL0"].plurals[0] = "оп%d";
    strings["Kick##sgiOPL1"].plurals[0] = "Бочка";
    strings["Env##sgiOPL"].plurals[0] = "Огиб.";
    strings["OP%d##sgiOPL1"].plurals[0] = "ОП%d";
    strings["OPL2/3 only (last 4 waveforms are OPL3 only)##sgiOPL0"].plurals[0] = "Только OPL2/3 (последние 4 волны только для OPL3)";
    strings["op%d##sgiOPL2"].plurals[0] = "оп%d";
    strings["Envelope 2 (kick only)##sgiOPL0"].plurals[0] = "Огибающая 2 (только бочка)";
    strings["Envelope##sgiOPL2"].plurals[0] = "Огибающая";
    strings["Operator %d##sgiOPL"].plurals[0] = "Оператор %d";
    strings["Waveform##sgiOPL"].plurals[0] = "Волна";
    strings["Envelope##sgiOPL3"].plurals[0] = "Огибающая";
    strings["OPL2/3 only (last 4 waveforms are OPL3 only)##sgiOPL1"].plurals[0] = "Только OPL2/3 (последние 4 волны только для OPL3)";
    strings["op%d##sgiOPL3"].plurals[0] = "оп%d";
    strings["Envelope 2 (kick only)##sgiOPL1"].plurals[0] = "Огибающая 2 (только бочка)";
    strings["Envelope##sgiOPL4"].plurals[0] = "Огибающая";
    strings["OP%d##sgiOPL4"].plurals[0] = "ОП%d";
    strings["OPL2/3 only (last 4 waveforms are OPL3 only)##sgiOPL2"].plurals[0] = "Только OPL2/3 (последние 4 волны только для OPL3)";
    strings["FM Macros##sgiOPL"].plurals[0] = "Макросы FM";
    strings["OP%d Macros##sgiOPL"].plurals[0] = "Макросы ОП%d";
    strings["Macros##sgiOPL"].plurals[0] = "Макросы";
    strings["Volume##sgiOPL"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiOPL"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiOPL"].plurals[0] = "Частота";
    strings["Panning##sgiOPL"].plurals[0] = "Панорамирование";
    strings["Phase Reset##sgiOPL"].plurals[0] = "Сброс фазы";

    //   sgiOPLL   src/gui/inst/opll.cpp

    strings["%s name##sgiOPLL"].plurals[0] = "название %s";
    strings["Fixed frequency mode##sgiOPLL"].plurals[0] = "Режим фиксированной частоты";
    strings["when enabled, drums will be set to the specified frequencies, ignoring the note.##sgiOPLL"].plurals[0] = "при включении режима ударные будут звучать на указанных частотах. Значения нот будут игнорироваться.";
    strings["Drum##sgiOPLL"].plurals[0] = "Ударный";
    strings["Block##sgiOPLL"].plurals[0] = "Блок";
    strings["FreqNum##sgiOPLL"].plurals[0] = "Частота";
    strings["Kick##sgiOPLL"].plurals[0] = "Бас-барабан";
    strings["Snare/Hi-hat##sgiOPLL"].plurals[0] = "Рабочий барабан/хай-хэт";
    strings["Tom/Top##sgiOPLL"].plurals[0] = "Там-там/тарелка";
    strings["Volume##TL"].plurals[0] = "Громкость##TL";
    strings["this volume slider only works in compatibility (non-drums) system.##sgiOPLL"].plurals[0] = "эта регулировка громкости работает только в совместимой (без ударных) системе.";
    strings["Other##sgiOPLL0"].plurals[0] = "Другое";
    strings["Other##sgiOPLL1"].plurals[0] = "Другое";
    strings["Envelope##sgiOPLL0"].plurals[0] = "Огибающая";
    strings["Envelope##sgiOPLL1"].plurals[0] = "Огибающая";
    strings["op%d##sgiOPLL0"].plurals[0] = "оп%d";
    strings["OP%d##sgiOPLL1"].plurals[0] = "ОП%d";
    strings["op%d##sgiOPLL2"].plurals[0] = "оп%d";
    strings["Operator %d##sgiOPLL"].plurals[0] = "Оператор %d";
    strings["Waveform##sgiOPLL"].plurals[0] = "Волна";
    strings["Envelope##sgiOPLL2"].plurals[0] = "Огибающая";
    strings["op%d##sgiOPLL3"].plurals[0] = "оп%d";
    strings["OP%d##sgiOPLL4"].plurals[0] = "ОП%d";
    strings["SSG On##sgiOPLL"].plurals[0] = "Вкл. SSG";
    strings["FM Macros##sgiOPLL"].plurals[0] = "Макросы FM";
    strings["OP%d Macros##sgiOPLL"].plurals[0] = "Макросы ОП%d";
    strings["Macros##sgiOPLL"].plurals[0] = "Макросы";
    strings["Volume##sgiOPLL"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiOPLL"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiOPLL"].plurals[0] = "Частота";
    strings["Patch##sgiOPLL"].plurals[0] = "Патч";
    strings["Phase Reset##sgiOPLL"].plurals[0] = "Сброс фазы";

    //   sgiOPM    src/gui/inst/opm.cpp

    strings["Envelope##sgiOPM0"].plurals[0] = "Огибающая";
    strings["op%d##sgiOPM0"].plurals[0] = "оп%d";
    strings["OP%d##sgiOPM3"].plurals[0] = "ОП%d";
    strings["op%d##sgiOPM1"].plurals[0] = "оп%d";
    strings["Operator %d##sgiOPM"].plurals[0] = "Оператор %d";
    strings["Waveform##sgiOPM"].plurals[0] = "Волна";
    strings["Envelope##sgiOPM1"].plurals[0] = "Огибающая";
    strings["op%d##sgiOPM2"].plurals[0] = "оп%d";
    strings["OP%d##sgiOPM4"].plurals[0] = "ОП%d";
    strings["FM Macros##sgiOPM"].plurals[0] = "Макросы FM";
    strings["AM Depth##sgiOPM"].plurals[0] = "Глубина АМ";
    strings["PM Depth##sgiOPM"].plurals[0] = "Глубина ФМ";
    strings["LFO Speed##sgiOPM"].plurals[0] = "Частота ГНЧ";
    strings["LFO Shape##sgiOPM"].plurals[0] = "Форма волны ГНЧ";
    strings["OpMask##sgiOPM"].plurals[0] = "Маска операторов";
    strings["OP%d Macros##sgiOPM"].plurals[0] = "Макросы ОП%d";
    strings["Macros##sgiOPM"].plurals[0] = "Макросы";
    strings["Volume##sgiOPM"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiOPM"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiOPM"].plurals[0] = "Частота";
    strings["Noise Freq##sgiOPM"].plurals[0] = "Частота шума";
    strings["Panning##sgiOPM"].plurals[0] = "Панорамирование";
    strings["Phase Reset##sgiOPM"].plurals[0] = "Сброс фазы";

    //   sgiOPN    src/gui/inst/opn.cpp

    strings["Envelope##sgiOPN0"].plurals[0] = "Огибающая";
    strings["Envelope##sgiOPN1"].plurals[0] = "Огибающая";
    strings["op%d##sgiOPN0"].plurals[0] = "оп%d";
    strings["OP%d##sgiOPN1"].plurals[0] = "ОП%d";
    strings["op%d##sgiOPN2"].plurals[0] = "оп%d";
    strings["Operator %d##sgiOPN"].plurals[0] = "Оператор %d";
    strings["SSG-EG##sgiOPN"].plurals[0] = "SSG-EG";
    strings["Envelope##sgiOPN2"].plurals[0] = "Огибающая";
    strings["op%d##sgiOPN3"].plurals[0] = "оп%d";
    strings["OP%d##sgiOPN4"].plurals[0] = "ОП%d";
    strings["SSG On##sgiOPN"].plurals[0] = "вкл. SSG";
    strings["FM Macros##sgiOPN"].plurals[0] = "Макросы FM";
    strings["LFO Speed##sgiOPN"].plurals[0] = "Частота ГНЧ";
    strings["OpMask##sgiOPN"].plurals[0] = "Маска операторов";
    strings["OP%d Macros##sgiOPN"].plurals[0] = "Макросы ОП%d";
    strings["Macros##sgiOPN"].plurals[0] = "Макросы";
    strings["Volume##sgiOPN"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiOPN"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiOPN"].plurals[0] = "Частота";
    strings["Panning##sgiOPN"].plurals[0] = "Панорамирование";
    strings["Phase Reset##sgiOPN"].plurals[0] = "Сброс фазы";

    //   sgiOPZ    src/gui/inst/opz.cpp

    strings["Request from TX81Z##sgiOPZ"].plurals[0] = "Запросить у TX81Z";
    strings["Other##sgiOPZ0"].plurals[0] = "Другое";
    strings["Other##sgiOPZ1"].plurals[0] = "Другое";
    strings["Envelope##sgiOPZ0"].plurals[0] = "Огибающая";
    strings["Envelope##sgiOPZ1"].plurals[0] = "Огибающая";
    strings["op%d##sgiOPZ0"].plurals[0] = "оп%d";
    strings["OP%d##sgiOPZ1"].plurals[0] = "ОП%d";
    strings["Fixed##sgiOPZ0"].plurals[0] = "Фикс.";
    strings["Block##sgiOPZ0"].plurals[0] = "Блок";
    strings["FreqNum##sgiOPZ0"].plurals[0] = "Частота";
    strings["op%d##sgiOPZ2"].plurals[0] = "оп%d";
    strings["Operator %d##sgiOPZ"].plurals[0] = "Оператор %d";
    strings["Waveform##sgiOPZ"].plurals[0] = "Волна";
    strings["Envelope##sgiOPZ2"].plurals[0] = "Огибающая";
    strings["Block##sgiOPZ1"].plurals[0] = "Блок";
    strings["Freq##sgiOPZ"].plurals[0] = "Част.";
    strings["Fixed##sgiOPZ1"].plurals[0] = "Фикс.";
    strings["op%d##sgiOPZ3"].plurals[0] = "оп%d";
    strings["OP%d##sgiOPZ4"].plurals[0] = "ОП%d";
    strings["Fixed##sgiOPZ2"].plurals[0] = "Фикс.";
    strings["Block##sgiOPZ2"].plurals[0] = "Блок";
    strings["FreqNum##sgiOPZ1"].plurals[0] = "Частота";
    strings["FM Macros##sgiOPZ"].plurals[0] = "Макросы FM";
    strings["AM Depth##sgiOPZ"].plurals[0] = "Глубина АМ";
    strings["PM Depth##sgiOPZ"].plurals[0] = "Глубина ФМ";
    strings["LFO Speed##sgiOPZ"].plurals[0] = "Частота ГНЧ";
    strings["LFO Shape##sgiOPZ"].plurals[0] = "Форма волны ГНЧ";
    strings["AM Depth 2##sgiOPZ"].plurals[0] = "Глубина АМ 2";
    strings["PM Depth 2##sgiOPZ"].plurals[0] = "Глубина ФМ 2";
    strings["LFO2 Speed##sgiOPZ"].plurals[0] = "Скорость ГНЧ 2";
    strings["LFO2 Shape##sgiOPZ"].plurals[0] = "Форма волны ГНЧ 2";
    strings["OP%d Macros##sgiOPZ"].plurals[0] = "Макросы ОП%d";
    strings["Macros##sgiOPZ"].plurals[0] = "Макросы";
    strings["Volume##sgiOPZ"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiOPZ"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiOPZ"].plurals[0] = "Частота";
    strings["Noise Freq##sgiOPZ"].plurals[0] = "Частота шума";
    strings["Panning##sgiOPZ"].plurals[0] = "Панорамирование";
    strings["Phase Reset##sgiOPZ"].plurals[0] = "Сброс фазы";

    //   sgiPCE    src/gui/inst/pce.cpp

    strings["Macros##sgiPCE"].plurals[0] = "Макросы";
    strings["Volume##sgiPCE"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiPCE"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiPCE"].plurals[0] = "Частота";
    strings["Noise##sgiPCE"].plurals[0] = "Шум";
    strings["Waveform##sgiPCE"].plurals[0] = "Волна";
    strings["Panning (left)##sgiPCE"].plurals[0] = "Панорамирование (левый)";
    strings["Panning (right)##sgiPCE"].plurals[0] = "Панорамирование (правый)";
    strings["Phase Reset##sgiPCE"].plurals[0] = "Сброс фазы";

    //   sgiPET    src/gui/inst/pet.cpp

    strings["Macros##sgiPET"].plurals[0] = "Макросы";
    strings["Volume##sgiPET"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiPET"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiPET"].plurals[0] = "Частота";
    strings["Waveform##sgiPET"].plurals[0] = "Волна";

    //   sgiPMQT   src/gui/inst/pokemini.cpp

    strings["Macros##sgiPMQT"].plurals[0] = "Макросы";
    strings["Volume##sgiPMQT"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiPMQT"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiPMQT"].plurals[0] = "Частота";
    strings["Pulse Width##sgiPMQT"].plurals[0] = "Скважность";

    //   sgiPOKEY  src/gui/inst/pokey.cpp

    strings["16-bit raw period macro##sgiPOKEY"].plurals[0] = "16-битный макрос периода (регистровое знач.)";
    strings["Macros##sgiPOKEY"].plurals[0] = "Макросы";
    strings["Volume##sgiPOKEY"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiPOKEY"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiPOKEY"].plurals[0] = "Частота";
    strings["Waveform##sgiPOKEY"].plurals[0] = "Волна";
    strings["Raw Period##sgiPOKEY"].plurals[0] = "Период (регистровое знач.)";

    //   sgiPNN    src/gui/inst/powernoise.cpp

    strings["Octave offset##sgiPNN"].plurals[0] = "Сдвиг октавы";
    strings["go to Macros for other parameters.##sgiPNN"].plurals[0] = "Откройте вкладку макросов для управления другими параметрами.";
    strings["Macros##sgiPNN"].plurals[0] = "Макросы";
    strings["Volume##sgiPNN"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiPNN"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiPNN"].plurals[0] = "Частота";
    strings["Panning (left)##sgiPNN"].plurals[0] = "Панорамирование (левый)";
    strings["Panning (right)##sgiPNN"].plurals[0] = "Панорамирование (правый)";
    strings["Phase Reset##sgiPNN"].plurals[0] = "Сброс фазы";
    strings["Control##sgiPNN"].plurals[0] = "Управление";
    strings["Tap A Location##sgiPNN"].plurals[0] = "Положение отвода A";
    strings["Tap B Location##sgiPNN"].plurals[0] = "Положение отвода B";
    strings["Load LFSR##sgiPNN"].plurals[0] = "Загрузить в РСЛОС";

    //   sgiPNS    src/gui/inst/powernoise_slope.cpp

    strings["Octave offset##sgiPNS"].plurals[0] = "Сдвиг октавы";
    strings["go to Macros for other parameters.##sgiPNS"].plurals[0] = "Откройте вкладку макросов для управления другими параметрами.";
    strings["Macros##sgiPNS"].plurals[0] = "Макросы";
    strings["Volume##sgiPNS"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiPNS"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiPNS"].plurals[0] = "Частота";
    strings["Panning (left)##sgiPNS"].plurals[0] = "Панорамирование (левый)";
    strings["Panning (right)##sgiPNS"].plurals[0] = "Панорамирование (правый)";
    strings["Phase Reset##sgiPNS"].plurals[0] = "Сброс фазы";
    strings["Control##sgiPNS"].plurals[0] = "Управление";
    strings["Portion A Length##sgiPNS"].plurals[0] = "Длина части A";
    strings["Portion B Length##sgiPNS"].plurals[0] = "Длина части B";
    strings["Portion A Offset##sgiPNS"].plurals[0] = "Сдвиг части A";
    strings["Portion B Offset##sgiPNS"].plurals[0] = "Сдвиг части B";

    //   sgiPV     src/gui/inst/pv1000.cpp

    strings["Macros##sgiPV"].plurals[0] = "Макросы";
    strings["Volume##sgiPV"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiPV"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiPV"].plurals[0] = "Частота";
    strings["Raw Frequency##sgiPV"].plurals[0] = "Частота (регистровое знач.)";

    //   sgiQ      src/gui/inst/qsound.cpp

    strings["Macros##sgiQ"].plurals[0] = "Макросы";
    strings["Volume##sgiQ"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiQ"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiQ"].plurals[0] = "Частота";
    strings["Panning##sgiQ"].plurals[0] = "Панорамирование";
    strings["Surround##sgiQ"].plurals[0] = "Окружение";
    strings["Phase Reset##sgiQ"].plurals[0] = "Сброс фазы";
    strings["Echo Level##sgiQ"].plurals[0] = "Уровень эхо";
    strings["Echo Feedback##sgiQ"].plurals[0] = "Обратная связь эхо";
    strings["Echo Length##sgiQ"].plurals[0] = "Длина эхо";

    //   sgiRF5    src/gui/inst/rf5c68.cpp

    strings["Macros##sgiRF5"].plurals[0] = "Макросы";
    strings["Volume##sgiRF5"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiRF5"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiRF5"].plurals[0] = "Частота";
    strings["Panning (left)##sgiRF5"].plurals[0] = "Панорамирование (левый)";
    strings["Panning (right)##sgiRF5"].plurals[0] = "Панорамирование (правый)";
    strings["Phase Reset##sgiRF5"].plurals[0] = "Сброс фазы";

    //   sgiSAA    src/gui/inst/saa1099.cpp

    strings["Macros##sgiSAA"].plurals[0] = "Макросы";
    strings["Volume##sgiSAA"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiSAA"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiSAA"].plurals[0] = "Частота";
    strings["Duty/Noise##sgiSAA"].plurals[0] = "Скважность/режим шума";
    strings["Waveform##sgiSAA"].plurals[0] = "Волна";
    strings["Panning (left)##sgiSAA"].plurals[0] = "Панорамирование (левый)";
    strings["Panning (right)##sgiSAA"].plurals[0] = "Панорамирование (правый)";
    strings["Envelope##sgiSAA"].plurals[0] = "Огибающая";

    //   sgismpd   src/gui/inst/sampleDraw.cpp

    strings["Sample##sgismpd0"].plurals[0] = "Сэмплы";
    strings["DPCM##sgismpd"].plurals[0] = "ДИКМ";
    strings["new DPCM features disabled (compatibility)!##sgismpd"].plurals[0] = "новые функции ДИКМ отключены (совместимость)!";
    strings["click here to enable them.##sgismpd"].plurals[0] = "нажмите сюда, чтобы включить их.";
    strings["none selected##sgismpd"].plurals[0] = "не выбран";
    strings["Use sample##sgismpd"].plurals[0] = "Использовать сэмпл";
    strings["Sample bank slot##BANKSLOT"].plurals[0] = "Номер банка сэмплов##BANKSLOT";
    strings["Sample##sgismpd1"].plurals[0] = "Сэмпл";
    strings["Use wavetable (Amiga/Generic DAC only)##sgismpd"].plurals[0] = "Использовать волновую таблицу (только для Amiga/типичного ЦАП)";
    strings["Use wavetable##sgismpd"].plurals[0] = "Использовать волновую таблицу";
    strings["Width##sgismpd"].plurals[0] = "Длина";
    strings["Use sample map##sgismpd"].plurals[0] = "Использовать карту сэмплов";
    strings["pitch##sgismpd"].plurals[0] = "частота";
    strings["loop##sgismpd"].plurals[0] = "цикл";
    strings["delta##sgismpd"].plurals[0] = "нач. знач.";
    strings["note##sgismpd"].plurals[0] = "нота";
    strings["sample name##sgismpd"].plurals[0] = "название сэмпла";
    strings["L##L%d"].plurals[0] = "Ц##L%d";
    strings["set entire map to this pitch##sgismpd"].plurals[0] = "установить эту частоту для всей карты";
    strings["set entire map to this delta counter value##sgismpd"].plurals[0] = "установить это нач. знач. дельта-счётчика для всей карты";
    strings["set entire map to this note##sgismpd"].plurals[0] = "установить эту ноту для всей карты";
    strings["set entire map to this sample##sgismpd"].plurals[0] = "установить этот сэмпл для всей карты";
    strings["reset pitches##sgismpd"].plurals[0] = "сбросить частоты";
    strings["clear delta counter values##sgismpd"].plurals[0] = "очистить нач. знач. дельта-счётчика";
    strings["reset notes##sgismpd"].plurals[0] = "сбросить ноты";
    strings["clear map samples##sgismpd"].plurals[0] = "сбросить сэмплы карты";

    //   sgiSCC    src/gui/inst/scc.cpp

    strings["Macros##sgiSCC"].plurals[0] = "Макросы";
    strings["Volume##sgiSCC"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiSCC"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiSCC"].plurals[0] = "Частота";
    strings["Waveform##sgiSCC"].plurals[0] = "Волна";

    //   sgiSEGA   src/gui/inst/segapcm.cpp

    strings["Macros##sgiSEGA"].plurals[0] = "Макросы";
    strings["Volume##sgiSEGA"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiSEGA"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiSEGA"].plurals[0] = "Частота";
    strings["Panning (left)##sgiSEGA"].plurals[0] = "Панорамирование (левый)";
    strings["Panning (right)##sgiSEGA"].plurals[0] = "Панорамирование (правый)";
    strings["Phase Reset##sgiSEGA"].plurals[0] = "Сброс фазы";

    //   sgiSID2   src/gui/inst/sid2.cpp

    strings["Waveform##sgiSID2"].plurals[0] = "Волна";
    strings["tri##sgiSID2"].plurals[0] = "треуг.";
    strings["saw##sgiSID2"].plurals[0] = "пила";
    strings["pulse##sgiSID2"].plurals[0] = "прямоуг.";
    strings["noise##sgiSID2"].plurals[0] = "шум";
    strings["A##sgiSID2"].plurals[0] = "А";
    strings["D##sgiSID2"].plurals[0] = "С";
    strings["S##sgiSID2"].plurals[0] = "С";
    strings["R##sgiSID2"].plurals[0] = "Р";
    strings["VOL##sgiSID2"].plurals[0] = "ГР";
    strings["Envelope##sgiSID2"].plurals[0] = "Огибающая";
    strings["Duty##sgiSID2"].plurals[0] = "Скважность";
    strings["Ring Modulation##sgiSID2"].plurals[0] = "Кольцевая модуляция";
    strings["Oscillator Sync##sgiSID2"].plurals[0] = "Синхронизация осцилляторов";
    strings["Enable filter##sgiSID2"].plurals[0] = "Включить фильтр";
    strings["Initialize filter##sgiSID2"].plurals[0] = "Инициализировать фильтр";
    strings["Cutoff##sgiSID2"].plurals[0] = "Частота среза";
    strings["Resonance##sgiSID2"].plurals[0] = "Резонанс (добротность)";
    strings["Filter Mode##sgiSID2"].plurals[0] = "Тип фильтра";
    strings["low##sgiSID2"].plurals[0] = "ФНЧ";
    strings["band##sgiSID2"].plurals[0] = "ППФ";
    strings["high##sgiSID2"].plurals[0] = "ФВЧ";
    strings["Noise Mode##sgiSID2"].plurals[0] = "Режим шума";
    strings["Wave Mix Mode##sgiSID2"].plurals[0] = "Режим смешения волн";
    strings["Absolute Cutoff Macro##sgiSID2"].plurals[0] = "Абсолютный макрос частоты среза";
    strings["Absolute Duty Macro##sgiSID2"].plurals[0] = "Абсолютный макрос скважности";
    strings["Macros##sgiSID2"].plurals[0] = "Макросы";
    strings["Volume##sgiSID2"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiSID2"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiSID2"].plurals[0] = "Частота";
    strings["Duty##sgiSID21"].plurals[0] = "Скважность";
    strings["Waveform##sgiSID21"].plurals[0] = "Волна";
    strings["Noise Mode##sgiSID21"].plurals[0] = "Режим шума";
    strings["Wave Mix Mode##sgiSID21"].plurals[0] = "Режим смеш. волн";
    strings["Cutoff##sgiSID21"].plurals[0] = "Частота среза";
    strings["Filter Mode##sgiSID21"].plurals[0] = "Тип фильтра";
    strings["Filter Toggle##sgiSID2"].plurals[0] = "Вкл./выкл. фильтр";
    strings["Resonance##sgiSID21"].plurals[0] = "Резонанс";
    strings["Phase Reset##sgiSID2"].plurals[0] = "Сброс фазы";
    strings["Envelope Reset/Key Control##sgiSID2"].plurals[0] = "Упр. огибающей";
    strings["Special##sgiSID2"].plurals[0] = "Разное";
    strings["Attack##sgiSID2"].plurals[0] = "Атака";
    strings["Decay##sgiSID2"].plurals[0] = "Спад";
    strings["Sustain##sgiSID2"].plurals[0] = "Сустейн";
    strings["Release##sgiSID2"].plurals[0] = "Релиз";

    //   sgiSM     src/gui/inst/sm8521.cpp

    strings["Macros##sgiSM"].plurals[0] = "Макросы";
    strings["Volume##sgiSM"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiSM"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiSM"].plurals[0] = "Частота";
    strings["Waveform##sgiSM"].plurals[0] = "Волна";

    //   sgiSNES   src/gui/inst/snes.cpp

    strings["Use envelope##sgiSNES"].plurals[0] = "Использовать огибающую";
    strings["Envelope##sgiSNES0"].plurals[0] = "Огибающая";
    strings["Envelope##sgiSNES1"].plurals[0] = "Огибающая";
    strings["Sustain/release mode:##sgiSNES"].plurals[0] = "Режим сустейна/релиза:";
    strings["Direct (cut on release)##sgiSNES"].plurals[0] = "Прямой (заглушить на релизе)";
    strings["Effective (linear decrease)##sgiSNES"].plurals[0] = "Эффективный (линейный спад)";
    strings["Effective (exponential decrease)##sgiSNES"].plurals[0] = "Эффективный (экспоненциальный спад)";
    strings["Delayed (write R on release)##sgiSNES"].plurals[0] = "Отложенный (записать R на релизе)";
    strings["Gain Mode##sgiSNES0"].plurals[0] = "Режим усиления";
    strings["Gain Mode##sgiSNES1"].plurals[0] = "Режим усиления";
    strings["Gain##sgiSNES0"].plurals[0] = "Усиление";
    strings["Gain##sgiSNES1"].plurals[0] = "Усиление";
    strings["Direct##sgiSNES"].plurals[0] = "Прямое";
    strings["Decrease (linear)##sgiSNES"].plurals[0] = "Спад (линейный)";
    strings["Decrease (logarithmic)##sgiSNES"].plurals[0] = "Спад (логарифмический)";
    strings["Increase (linear)##sgiSNES"].plurals[0] = "Нарастание (линейное)";
    strings["Increase (bent line)##sgiSNES"].plurals[0] = "Нарастание (изогн. линия)";
    strings["using decrease modes will not produce any sound at all, unless you know what you are doing.\nit is recommended to use the Gain macro for decrease instead.##sgiSNES"].plurals[0] = "использование режимов спада приведёт к отсутствию звука, если вы не понимаете, как это всё работает.\nрекомендуется использовать макрос усиления для спада.";
    strings["Macros##sgiSNES"].plurals[0] = "Макросы";
    strings["Volume##sgiSNES"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiSNES"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiSNES"].plurals[0] = "Частота";
    strings["Noise Freq##sgiSNES"].plurals[0] = "Частота шума";
    strings["Waveform##sgiSNES"].plurals[0] = "Волна";
    strings["Panning (left)##sgiSNES"].plurals[0] = "Панорамирование (левый)";
    strings["Panning (right)##sgiSNES"].plurals[0] = "Панорамирование (правый)";
    strings["Special##sgiSNES"].plurals[0] = "Разное";
    strings["Gain##sgiSNES2"].plurals[0] = "Усиление";

    //   sgiPSG    src/gui/inst/std.cpp

    strings["Macros##sgiPSG"].plurals[0] = "Макросы";
    strings["Volume##sgiPSG"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiPSG"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiPSG"].plurals[0] = "Частота";
    strings["Duty##sgiPSG"].plurals[0] = "Скважность";
    strings["Panning##sgiPSG"].plurals[0] = "Панорамирование";
    strings["Phase Reset##sgiPSG"].plurals[0] = "Сброс фазы";

    //   sgistru   src/gui/inst/stringsUtil.cpp

    strings["Down Down Down##sgistru"].plurals[0] = "Вниз Вниз Вниз";
    strings["Down.##sgistru"].plurals[0] = "Вниз.";
    strings["Down Up Down Up##sgistru"].plurals[0] = "Вниз Вверх Вниз Вверх";
    strings["Down UP##sgistru"].plurals[0] = "Вниз ВВЕРХ";
    strings["Up Up Up##sgistru"].plurals[0] = "Вверх Вверх Вверх";
    strings["Up.##sgistru"].plurals[0] = "Вверх.";
    strings["Up Down Up Down##sgistru"].plurals[0] = "Вверх Вниз Вверх Вниз";
    strings["Up DOWN##sgistru"].plurals[0] = "Вверх ВНИЗ";
    strings["Algorithm##sgistru"].plurals[0] = "Алгоритм";
    strings["Feedback##sgistru"].plurals[0] = "Обр. связь.";
    strings["LFO > Freq##sgistru"].plurals[0] = "ГНЧ > част.";
    strings["LFO > Amp##sgistru"].plurals[0] = "ГНЧ > ампл.";
    strings["Attack##sgistru"].plurals[0] = "Атака";
    strings["Decay##sgistru"].plurals[0] = "Спад";
    strings["Decay 2##sgistru"].plurals[0] = "Спад 2";
    strings["Release##sgistru"].plurals[0] = "Релиз";
    strings["Sustain##sgistru0"].plurals[0] = "Сустейн";
    strings["Level##sgistru"].plurals[0] = "Уровень";
    strings["EnvScale##sgistru"].plurals[0] = "Масш. огиб.";
    strings["Multiplier##sgistru"].plurals[0] = "Множитель";
    strings["Detune##sgistru0"].plurals[0] = "Расстройка";
    strings["Detune 2##sgistru"].plurals[0] = "Расстройка 2";
    strings["SSG-EG##sgistru"].plurals[0] = "SSG-EG";
    strings["AM Depth##sgistru"].plurals[0] = "Глубина АМ";
    strings["Vibrato Depth##sgistru"].plurals[0] = "Глубина вибрато";
    strings["Sustained##sgistru0"].plurals[0] = "Сустейн";
    strings["Sustained##sgistru1"].plurals[0] = "Сустейн";
    strings["Level Scaling##sgistru"].plurals[0] = "Масштаб. громкости";
    strings["Sustain##sgistru1"].plurals[0] = "Сустейн";
    strings["Vibrato##sgistru"].plurals[0] = "Вибрато";
    strings["Waveform##sgistru"].plurals[0] = "Волна";
    strings["Scale Rate##sgistru"].plurals[0] = "Измен. коэфф. масш. в зав. от част.";
    strings["OP2 Half Sine##sgistru"].plurals[0] = "полу-синус для ОП2";
    strings["OP1 Half Sine##sgistru"].plurals[0] = "полу-синус для ОП1";
    strings["EnvShift##sgistru"].plurals[0] = "Сдвиг огиб.";
    strings["Reverb##sgistru"].plurals[0] = "Реверб";
    strings["Fine##sgistru0"].plurals[0] = "Точн.";
    strings["LFO2 > Freq##sgistru"].plurals[0] = "ОНЧ2 > част.";
    strings["LFO2 > Amp##sgistru"].plurals[0] = "ОНЧ2 > част.";
    strings["Fine##sgistru1"].plurals[0] = "Точн.";
    strings["Fine##sgistru2"].plurals[0] = "Точн.";
    strings["OP4 Noise Mode##sgistru0"].plurals[0] = "Режим шума у ОП4";
    strings["Envelope Delay##sgistru"].plurals[0] = "Задержка огибающей";
    strings["Output Level##sgistru0"].plurals[0] = "Громкость вывода";
    strings["Modulation Input Level##sgistru"].plurals[0] = "Сила модуляции на входе";
    strings["Left Output##sgistru"].plurals[0] = "Вывод звука влево";
    strings["Right Output##sgistru"].plurals[0] = "Вывод звука вправо";
    strings["Coarse Tune (semitones)##sgistru"].plurals[0] = "Грубая расстройка (полутоны)";
    strings["Detune##sgistru1"].plurals[0] = "Расстройка";
    strings["Fixed Frequency Mode##sgistru"].plurals[0] = "Режим фиксированной частоты";
    strings["OP4 Noise Mode##sgistru1"].plurals[0] = "Режим шума у ОП4";
    strings["Env. Delay##sgistru"].plurals[0] = "Задержка огибающей";
    strings["Output Level##sgistru1"].plurals[0] = "Громкость вывода";
    strings["ModInput##sgistru"].plurals[0] = "Мод. на входе";
    strings["Left##sgistru"].plurals[0] = "Лево";
    strings["Right##sgistru"].plurals[0] = "Право";
    strings["Tune##sgistru"].plurals[0] = "Груб. расстр.";
    strings["Detune##sgistru2"].plurals[0] = "Растройка";
    strings["Fixed##sgistru0"].plurals[0] = "Фикс. част.";
    strings["Fine##sgistru3"].plurals[0] = "Точн.";
    strings["Fine##sgistru4"].plurals[0] = "Точн.";
    strings["Fine##sgistru5"].plurals[0] = "Точн.";
    strings["User##sgistru0"].plurals[0] = "Пользовательский";
    strings["1. Violin##sgistru"].plurals[0] = "1. Скрипка";
    strings["2. Guitar##sgistru0"].plurals[0] = "2. Гитара";
    strings["3. Piano##sgistru0"].plurals[0] = "3. Пианино";
    strings["4. Flute##sgistru0"].plurals[0] = "4. Флейта";
    strings["5. Clarinet##sgistru0"].plurals[0] = "5. Кларнет";
    strings["6. Oboe##sgistru"].plurals[0] = "6. Гобой";
    strings["7. Trumpet##sgistru0"].plurals[0] = "7. Тромбон";
    strings["8. Organ##sgistru"].plurals[0] = "8. Орган";
    strings["9. Horn##sgistru"].plurals[0] = "9. Рог";
    strings["10. Synth##sgistru"].plurals[0] = "10. Синтезатор";
    strings["11. Harpsichord##sgistru"].plurals[0] = "11. Клавесин";
    strings["12. Vibraphone##sgistru0"].plurals[0] = "12. Вибрафон";
    strings["13. Synth Bass##sgistru"].plurals[0] = "13. Синтезаторный бас";
    strings["14. Acoustic Bass##sgistru"].plurals[0] = "14. Акустический бас";
    strings["15. Electric Guitar##sgistru"].plurals[0] = "15. Электрогитара";
    strings["Drums##sgistru0"].plurals[0] = "Ударные";
    strings["User##sgistru1"].plurals[0] = "Пользовательский";
    strings["1. Electric String##sgistru"].plurals[0] = "1. Электронный струнный инструмент";
    strings["2. Bow wow##sgistru"].plurals[0] = "2. Гам-гам";
    strings["3. Electric Guitar##sgistru0"].plurals[0] = "3. Электрогитара";
    strings["4. Organ##sgistru"].plurals[0] = "4. Орган";
    strings["5. Clarinet##sgistru1"].plurals[0] = "5. Кларнет";
    strings["6. Saxophone##sgistru"].plurals[0] = "6. Саксофон";
    strings["7. Trumpet##sgistru1"].plurals[0] = "7. Тромбон";
    strings["8. Street Organ##sgistru"].plurals[0] = "8. Шарманка";
    strings["9. Synth Brass##sgistru"].plurals[0] = "9. Синтезаторный медный духовой";
    strings["10. Electric Piano##sgistru"].plurals[0] = "10. Электронное пианино";
    strings["11. Bass##sgistru"].plurals[0] = "11. Бас";
    strings["12. Vibraphone##sgistru1"].plurals[0] = "12. Вибрафон";
    strings["13. Chime##sgistru"].plurals[0] = "13. Колокольчик";
    strings["14. Tom Tom II##sgistru"].plurals[0] = "14. Там-там №2";
    strings["15. Noise##sgistru"].plurals[0] = "15. Шум";
    strings["Drums##sgistru1"].plurals[0] = "Ударные";
    strings["User##sgistru2"].plurals[0] = "Пользовательский";
    strings["1. Strings##sgistru"].plurals[0] = "1. Струнные";
    strings["2. Guitar##sgistru1"].plurals[0] = "2. Гитара";
    strings["3. Electric Guitar##sgistru1"].plurals[0] = "3. Электрогитара";
    strings["4. Electric Piano##sgistru"].plurals[0] = "4. Электронное пианино";
    strings["5. Flute##sgistru"].plurals[0] = "5. Флейта";
    strings["6. Marimba##sgistru"].plurals[0] = "6. Маримба";
    strings["7. Trumpet##sgistru2"].plurals[0] = "7. Тромбон";
    strings["8. Harmonica##sgistru"].plurals[0] = "8. Гармоника";
    strings["9. Tuba##sgistru"].plurals[0] = "9. Туба";
    strings["10. Synth Brass##sgistru"].plurals[0] = "10. Синтезаторный медный духовой";
    strings["11. Short Saw##sgistru"].plurals[0] = "11. Короткая пила";
    strings["12. Vibraphone##sgistru2"].plurals[0] = "12. Вибрафон";
    strings["13. Electric Guitar 2##sgistru"].plurals[0] = "13. Электрогитара 2";
    strings["14. Synth Bass##sgistru"].plurals[0] = "14. Синтезаторный бас";
    strings["15. Sitar##sgistru"].plurals[0] = "15. Ситар";
    strings["Drums##sgistru2"].plurals[0] = "Ударные";
    strings["User##sgistru3"].plurals[0] = "Пользовательский";
    strings["1. Bell##sgistru"].plurals[0] = "1. Колокол";
    strings["2. Guitar##sgistru2"].plurals[0] = "2. Гитара";
    strings["3. Piano##sgistru1"].plurals[0] = "3. Пианино";
    strings["4. Flute##sgistru1"].plurals[0] = "4. Флейта";
    strings["5. Clarinet##sgistru2"].plurals[0] = "5. Кларнет";
    strings["6. Rattling Bell##sgistru"].plurals[0] = "6. Дребезжащий колокол";
    strings["7. Trumpet##sgistru3"].plurals[0] = "7. Тромбон";
    strings["8. Reed Organ##sgistru"].plurals[0] = "8. Свирель";
    strings["9. Soft Bell##sgistru"].plurals[0] = "9. \"Мягкий\" колокол";
    strings["10. Xylophone##sgistru"].plurals[0] = "10. Ксилофон";
    strings["11. Vibraphone##sgistru"].plurals[0] = "11. Вибрафон";
    strings["12. Brass##sgistru"].plurals[0] = "12. Медный духовой инструмент";
    strings["13. Bass Guitar##sgistru"].plurals[0] = "13. Бас-гитара";
    strings["14. Synth##sgistru"].plurals[0] = "14. Синтезатор";
    strings["15. Chorus##sgistru"].plurals[0] = "15. Хорус";
    strings["Drums##sgistru3"].plurals[0] = "Ударные";
    strings["Sine##sgistru0"].plurals[0] = "Синус";
    strings["Half Sine##sgistru0"].plurals[0] = "Половинка синуса";
    strings["Absolute Sine##sgistru0"].plurals[0] = "Модуль синуса";
    strings["Quarter Sine##sgistru"].plurals[0] = "Четверть синуса";
    strings["Squished Sine##sgistru0"].plurals[0] = "Сжатый синус";
    strings["Squished AbsSine##sgistru0"].plurals[0] = "Сж. модуль синуса";
    strings["Square##sgistru0"].plurals[0] = "Меандр";
    strings["Derived Square##sgistru0"].plurals[0] = "Производная от меандра";
    strings["Sine##sgistru1"].plurals[0] = "Синус";
    strings["Half Sine##sgistru1"].plurals[0] = "Половинка синуса";
    strings["Absolute Sine##sgistru1"].plurals[0] = "Модуль синуса";
    strings["Pulse Sine##sgistru"].plurals[0] = "Синус-меандр";
    strings["Sine (Even Periods)##sgistru"].plurals[0] = "Синус (чётные периоды)";
    strings["AbsSine (Even Periods)##sgistru"].plurals[0] = "Мод. синуса (чётн. пер.)";
    strings["Square##sgistru1"].plurals[0] = "Меандр";
    strings["Derived Square##sgistru1"].plurals[0] = "Производная от меандра";
    strings["Sine##sgistru2"].plurals[0] = "Синус";
    strings["Triangle##sgistru"].plurals[0] = "Треуг. волна";
    strings["Cut Sine##sgistru"].plurals[0] = "Обрезанный синус";
    strings["Cut Triangle##sgistru"].plurals[0] = "Обрез. треуг. волна";
    strings["Squished Sine##sgistru1"].plurals[0] = "Сжатый синус";
    strings["Squished Triangle##sgistru"].plurals[0] = "Сжатая треуг. волна";
    strings["Squished AbsSine##sgistru1"].plurals[0] = "Сжатый модуль синуса";
    strings["Squished AbsTriangle##sgistru"].plurals[0] = "Сж. мод. треуг. волны";
    strings["Snare##sgistru0"].plurals[0] = "Рабочий барабан";
    strings["Tom##sgistru"].plurals[0] = "Там-там";
    strings["Top##sgistru0"].plurals[0] = "Тарелка";
    strings["HiHat##sgistru0"].plurals[0] = "Хай-хэт";
    strings["Normal##sgistru"].plurals[0] = "Обычный";
    strings["Snare##sgistru1"].plurals[0] = "Малый барабан";
    strings["HiHat##sgistru1"].plurals[0] = "Хай-хэт";
    strings["Top##sgistru1"].plurals[0] = "Тарелка";
    strings["Noise disabled##sgistru"].plurals[0] = "Шум выключен";
    strings["Square + noise##sgistru"].plurals[0] = "Меандр + шум";
    strings["Ringmod from OP3 + noise##sgistru"].plurals[0] = "Кольцевая модуляция от ОП3 + шум";
    strings["Ringmod from OP3 + double pitch ModInput\nWARNING - has emulation issues, subject to change##sgistru"].plurals[0] = "Кольцевая модуляция от ОП3 + входная модуляция с удвоенной частотой\nВНИМАНИЕ - имеются проблемы с эмуляцией этого режима, она может измениться";
    strings["op1##sgistru"].plurals[0] = "оп1";
    strings["op2##sgistru"].plurals[0] = "оп2";
    strings["op3##sgistru"].plurals[0] = "оп3";
    strings["op4##sgistru"].plurals[0] = "оп4";
    strings["triangle##sgistru"].plurals[0] = "треуг.";
    strings["saw##sgistru"].plurals[0] = "пила";
    strings["pulse##sgistru"].plurals[0] = "прямоуг.";
    strings["noise##sgistru0"].plurals[0] = "шум";
    strings["tone##sgistru"].plurals[0] = "тон";
    strings["noise##sgistru1"].plurals[0] = "шум";
    strings["envelope##sgistru"].plurals[0] = "огибающая";
    strings["hold##sgistru"].plurals[0] = "удержание";
    strings["alternate##sgistru"].plurals[0] = "изм. направл.";
    strings["direction##sgistru0"].plurals[0] = "направление";
    strings["enable##sgistru0"].plurals[0] = "вкл.";
    strings["enabled##sgistru0"].plurals[0] = "вкл.";
    strings["mirror##sgistru"].plurals[0] = "отраж.";
    strings["loop##sgistru0"].plurals[0] = "цикл";
    strings["cut##sgistru"].plurals[0] = "обрез.";
    strings["direction##sgistru1"].plurals[0] = "направление";
    strings["resolution##sgistru"].plurals[0] = "разрешение";
    strings["fixed##sgistru1"].plurals[0] = "фиксир.";
    strings["N/A##sgistru"].plurals[0] = "-";
    strings["enabled##sgistru1"].plurals[0] = "вкл.";
    strings["noise##sgistru2"].plurals[0] = "шум";
    strings["echo##sgistru"].plurals[0] = "эхо";
    strings["pitch mod##sgistru"].plurals[0] = "част. мод.";
    strings["invert right##sgistru"].plurals[0] = "инв. справа";
    strings["invert left##sgistru"].plurals[0] = "инв. слева";
    strings["low##sgistru"].plurals[0] = "ФНЧ";
    strings["band##sgistru"].plurals[0] = "ППФ";
    strings["high##sgistru"].plurals[0] = "ФВЧ";
    strings["ch3off##sgistru"].plurals[0] = "выкл. 3 кан.";
    strings["gate##sgistru"].plurals[0] = "старт/стоп огиб.";
    strings["sync##sgistru"].plurals[0] = "синхр.";
    strings["ring##sgistru"].plurals[0] = "кольц.";
    strings["test##sgistru"].plurals[0] = "тест";
    strings["15kHz##sgistru"].plurals[0] = "15 кГц";
    strings["filter 2+4##sgistru"].plurals[0] = "фильтр 2+4";
    strings["filter 1+3##sgistru"].plurals[0] = "фильтр 1+3";
    strings["16-bit 3+4##sgistru"].plurals[0] = "16-битн. 3+4";
    strings["16-bit 1+2##sgistru"].plurals[0] = "16-битн. 1+2";
    strings["high3##sgistru"].plurals[0] = "ФВЧ 3";
    strings["high1##sgistru"].plurals[0] = "ФВЧ 1";
    strings["poly9##sgistru"].plurals[0] = "полином 9";
    strings["int##sgistru"].plurals[0] = "интегр.";
    strings["sustain##sgistru2"].plurals[0] = "сустейн";
    strings["square##sgistru2"].plurals[0] = "прямоуг.";
    strings["noise##sgistru3"].plurals[0] = "шум";
    strings["noise##sgistru4"].plurals[0] = "шум";
    strings["invert##sgistru"].plurals[0] = "инверт.";
    strings["surround##sgistru"].plurals[0] = "окр. звуч.";
    strings["enable##sgistru1"].plurals[0] = "вкл.";
    strings["oneshot##sgistru"].plurals[0] = "однокр.";
    strings["split L/R##sgistru"].plurals[0] = "разд. лев/прав";
    strings["HinvR##sgistru"].plurals[0] = "HинвR";
    strings["VinvR##sgistru"].plurals[0] = "VинвR";
    strings["HinvL##sgistru"].plurals[0] = "HинвL";
    strings["VinvL##sgistru"].plurals[0] = "VинвL";
    strings["ring mod##sgistru"].plurals[0] = "кольц. мод.";
    strings["low pass##sgistru"].plurals[0] = "ФНЧ";
    strings["high pass##sgistru"].plurals[0] = "ФВЧ";
    strings["band pass##sgistru"].plurals[0] = "ППФ";
    strings["HP/K2, HP/K2##sgistru"].plurals[0] = "ВЧ/K2, ВЧ/K2";
    strings["HP/K2, LP/K1##sgistru"].plurals[0] = "ВЧ/K2, НЧ/K1";
    strings["LP/K2, LP/K2##sgistru"].plurals[0] = "НЧ/K2, НЧ/K2";
    strings["LP/K2, LP/K1##sgistru"].plurals[0] = "НЧ/K2, НЧ/K1";
    strings["right##sgistru"].plurals[0] = "правый";
    strings["left##sgistru"].plurals[0] = "левый";
    strings["rear right##sgistru"].plurals[0] = "задний правый";
    strings["rear left##sgistru"].plurals[0] = "задний левый";
    strings["enable tap B##sgistru"].plurals[0] = "вкл. отвод B";
    strings["AM with slope##sgistru"].plurals[0] = "АМ со скатом";
    strings["invert B##sgistru"].plurals[0] = "инв. B";
    strings["invert A##sgistru"].plurals[0] = "инв. A";
    strings["reset B##sgistru"].plurals[0] = "перезап. B";
    strings["reset A##sgistru"].plurals[0] = "перезап. A";
    strings["clip B##sgistru"].plurals[0] = "огран. B";
    strings["clip A##sgistru"].plurals[0] = "огран. A";
    strings["on##sgistru"].plurals[0] = "вкл.";
    strings["k1 slowdown##sgistru"].plurals[0] = "замедл. k1";
    strings["k2 slowdown##sgistru"].plurals[0] = "замедл. k2";
    strings["pause##sgistru"].plurals[0] = "стоп";
    strings["reverse##sgistru"].plurals[0] = "реверс";
    strings["high pass##sgistru1"].plurals[0] = "ФВЧ";
    strings["ring mod##sgistru1"].plurals[0] = "кольц. мод.";
    strings["swap counters (noise)##sgistru"].plurals[0] = "помен. счётч. (шум)";
    strings["low pass (noise)##sgistru"].plurals[0] = "ФНЧ (шум)";
    strings["sync##sgistru1"].plurals[0] = "синхр.";
    strings["ring##sgistru1"].plurals[0] = "кольц.";
    strings["low##sgistru1"].plurals[0] = "ФНЧ";
    strings["band##sgistru1"].plurals[0] = "ППФ";
    strings["high##sgistru1"].plurals[0] = "ФВЧ";
    strings["8580 SID##sgistru"].plurals[0] = "8580 SID";
    strings["bitwise AND##sgistru"].plurals[0] = "побитовое И";
    strings["bitwise OR##sgistru"].plurals[0] = "побитовое ИЛИ";
    strings["bitwise XOR##sgistru"].plurals[0] = "побитовое искл. ИЛИ";
    strings["None##sgistru"].plurals[0] = "Нет";
    strings["Invert##sgistru"].plurals[0] = "Инвертировать";
    strings["Add##sgistru"].plurals[0] = "Сложить";
    strings["Subtract##sgistru"].plurals[0] = "Вычесть";
    strings["Average##sgistru"].plurals[0] = "Усреднить";
    strings["Phase##sgistru"].plurals[0] = "Временной сдвиг";
    strings["Chorus##sgistru"].plurals[0] = "Хорус";
    strings["None (dual)##sgistru"].plurals[0] = "Нет (две таблицы)";
    strings["Wipe##sgistru"].plurals[0] = "Замена";
    strings["Fade##sgistru"].plurals[0] = "Плавный переход";
    strings["Fade (ping-pong)##sgistru"].plurals[0] = "Плавный переход (туда-обратно)";
    strings["Overlay##sgistru"].plurals[0] = "Наложение";
    strings["Negative Overlay##sgistru"].plurals[0] = "Наложение с обратным знаком";
    strings["Slide##sgistru"].plurals[0] = "Сдвиг";
    strings["Mix Chorus##sgistru"].plurals[0] = "Микширование с хорусом";
    strings["Phase Modulation##sgistru"].plurals[0] = "Фазовая модуляция";
    strings["Envelope##sgistru"].plurals[0] = "Огибающая";
    strings["Sweep##sgistru"].plurals[0] = "Аппаратное портаменто";
    strings["Wait##sgistru0"].plurals[0] = "Ждать";
    strings["Wait for Release##sgistru0"].plurals[0] = "Ждать релиз";
    strings["Loop##sgistru1"].plurals[0] = "Цикл";
    strings["Loop until Release##sgistru0"].plurals[0] = "Цикл до релиза";
    strings["Volume Sweep##sgistru"].plurals[0] = "Изменение громкости";
    strings["Frequency Sweep##sgistru"].plurals[0] = "Изменение частоты";
    strings["Cutoff Sweep##sgistru"].plurals[0] = "Изменение частоты среза";
    strings["Wait##sgistru1"].plurals[0] = "Ждать";
    strings["Wait for Release##sgistru1"].plurals[0] = "Ждать релиз";
    strings["Loop##sgistru2"].plurals[0] = "Цикл";
    strings["Loop until Release##sgistru1"].plurals[0] = "Цикл до релиза";
    strings["Direct##sgistru"].plurals[0] = "Прямое";
    strings["Decrease (linear)##sgistru"].plurals[0] = "Уменьш. (линейное)";
    strings["Decrease (logarithmic)##sgistru"].plurals[0] = "Уменьш. (логарифмическое)";
    strings["Increase (linear)##sgistru"].plurals[0] = "Увелич. (линейное)";
    strings["Increase (bent line)##sgistru"].plurals[0] = "Увелич. (изогн. линия)";

    strings["noise##sgistru5"].plurals[0] = "шум";
    strings["pulse##sgistru5"].plurals[0] = "прямоуг.";
    strings["triangle##sgistru5"].plurals[0] = "треуг.";
    strings["saw##sgistru5"].plurals[0] = "пила";
    strings["metal##sgistru5"].plurals[0] = "\"металл.\"";
    strings["sine##sgistru5"].plurals[0] = "синус";

    strings["Off##sgistru"].plurals[0] = "Выкл.";
    strings["Lowpass##sgistru"].plurals[0] = "ФНЧ";
    strings["Highpass##sgistru"].plurals[0] = "ФВЧ";
    strings["Bandpass##sgistru"].plurals[0] = "ППФ";
    strings["Low + High##sgistru"].plurals[0] = "ФНЧ + ФВЧ";
    strings["High + Band##sgistru"].plurals[0] = "ФВЧ + ППФ";
    strings["Low + Band##sgistru"].plurals[0] = "ФНЧ + ППФ";
    strings["Low + High + Band##sgistru"].plurals[0] = "ФНЧ + ФВЧ + ППФ";

    strings["Arpeggio##sgistru"].plurals[0] = "Арпеджио";
    strings["Portamento up##sgistru"].plurals[0] = "Портаменто вверх";
    strings["Portamento down##sgistru"].plurals[0] = "Портаменто вниз";
    strings["Vibrato##sgistru1"].plurals[0] = "Вибрато";
    strings["PWM##sgistru"].plurals[0] = "ШИМ";
    strings["Set pulse width##sgistru"].plurals[0] = "Скважность";
    strings["Pulse width down##sgistru"].plurals[0] = "Скважность вниз";
    strings["Pulse width up##sgistru"].plurals[0] = "Скважность вверх";
    strings["Set filter cutoff##sgistru"].plurals[0] = "Частота среза";
    strings["Volume fade##sgistru"].plurals[0] = "Изменение громкости";
    strings["Set waveform##sgistru"].plurals[0] = "Волна";
    strings["Set volume##sgistru"].plurals[0] = "Громкость";
    strings["Toggle filter##sgistru"].plurals[0] = "Вкл./выкл. фильтр";
    strings["Fine portamento up##sgistru"].plurals[0] = "Точное портаменто вверх";
    strings["Fine portamento down##sgistru"].plurals[0] = "Точное портаменто вниз";
    strings["Filter mode##sgistru"].plurals[0] = "Режим фильтра";
    strings["Retrigger##sgistru"].plurals[0] = "Циклич. перезапуск";
    strings["Fine volume down##sgistru"].plurals[0] = "Точное изменение громкости вниз";
    strings["Fine volume up##sgistru"].plurals[0] = "Точное изменение громкости вверх";
    strings["Note cut##sgistru"].plurals[0] = "Заглушить ноту";
    strings["Phase reset##sgistru"].plurals[0] = "Сброс фазы";
    strings["Program period##sgistru"].plurals[0] = "Период программы";
    strings["Filter cutoff up##sgistru"].plurals[0] = "Частота среза вверх";
    strings["Filter cutoff down##sgistru"].plurals[0] = "Частота среза вниз";
    strings["Set filter resonance##sgistru"].plurals[0] = "Резонанс";
    strings["Filter resonance up##sgistru"].plurals[0] = "Резонанс вверх";
    strings["Filter resonance down##sgistru"].plurals[0] = "Резонанс вниз";
    strings["Set attack##sgistru"].plurals[0] = "Атака";
    strings["Set decay##sgistru"].plurals[0] = "Спад";
    strings["Set sustain##sgistru"].plurals[0] = "Сустейн";
    strings["Set release##sgistru"].plurals[0] = "Релиз";
    strings["Ring modulation source##sgistru"].plurals[0] = "Источник кольцевой модуляции";
    strings["Hard sync source##sgistru"].plurals[0] = "Источник синхронизации осцилляторов";
    strings["Portamento up (semitones)##sgistru"].plurals[0] = "Портаменто вверх (полутонов)";
    strings["Portamento down (semitones)##sgistru"].plurals[0] = "Портаменто вниз (полутонов)";
    strings["Detune##sgistru"].plurals[0] = "Расстройка";
    strings["Absolute arpeggio note##sgistru"].plurals[0] = "Абсолютная нота арпеджио";
    strings["Trigger release##sgistru"].plurals[0] = "Релиз огибающей";
    strings["Loop begin##sgistru"].plurals[0] = "Начало цикла";
    strings["Loop end##sgistru"].plurals[0] = "Конец цикла";
    strings["Jump##sgistru"].plurals[0] = "Переход на шаг";
    strings["NOP##sgistru"].plurals[0] = "Ничего не делать";
    strings["Program end##sgistru"].plurals[0] = "Конец программы";

    strings["Fixed##sgistru2"].plurals[0] = "Абсолютное";
    strings["Relative##sgistru"].plurals[0] = "Относительное";
    strings["QSound##sgistru"].plurals[0] = "QSound";
    strings["Bug##sgistru"].plurals[0] = "Сбой";

    strings["Fixed"].plurals[0] = "Абсолютное";
    strings["Relative"].plurals[0] = "Относительное";
    strings["Local"].plurals[0] = "Локальная";
    strings["Global"].plurals[0] = "Глобальная";

    //   sgiSU     src/gui/inst/su.cpp

    strings["Sound Unit##sgiSU"].plurals[0] = "Sound Unit";
    strings["Switch roles of frequency and phase reset timer##sgiSU"].plurals[0] = "Поменять роли таймера сброса фазы и таймера изменения частоты";
    strings["Hardware Sequence##sgiSU"].plurals[0] = "Аппаратная последовательность";
    strings["Tick##sgiSU0"].plurals[0] = "Шаг движка";
    strings["Command##sgiSU0"].plurals[0] = "Команда";
    strings["Move/Remove##sgiSU0"].plurals[0] = "Перем./Удал.";
    strings["Period##sgiSU0"].plurals[0] = "Период";
    strings["Amount##sgiSU0"].plurals[0] = "Количество";
    strings["Bound##sgiSU0"].plurals[0] = "Граница";
    strings["Up##sgiSU0"].plurals[0] = "Вверх";
    strings["Down##sgiSU0"].plurals[0] = "Вниз";
    strings["Loop##sgiSU"].plurals[0] = "Цикл";
    strings["Flip##sgiSU"].plurals[0] = "Туда-обратно";
    strings["Period##sgiSU1"].plurals[0] = "Период";
    strings["Amount##sgiSU1"].plurals[0] = "Количество";
    strings["Bound##sgiSU1"].plurals[0] = "Граница";
    strings["Up##sgiSU1"].plurals[0] = "Вверх";
    strings["Down##sgiSU1"].plurals[0] = "Вниз";
    strings["Ticks##sgiSU"].plurals[0] = "Шагов";
    strings["Position##sgiSU"].plurals[0] = "Положение";
    strings["Macros##sgiSU"].plurals[0] = "Макросы";
    strings["Volume##sgiSU"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiSU"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiSU"].plurals[0] = "Частота";
    strings["Duty/Noise##sgiSU"].plurals[0] = "Скважность/режим шума";
    strings["Waveform##sgiSU"].plurals[0] = "Волна";
    strings["Panning##sgiSU"].plurals[0] = "Панорамирование";
    strings["Phase Reset##sgiSU"].plurals[0] = "Сброс фазы";
    strings["Cutoff##sgiSU"].plurals[0] = "Частота среза";
    strings["Resonance##sgiSU"].plurals[0] = "Резонанс";
    strings["Control##sgiSU"].plurals[0] = "Управление";
    strings["Phase Reset Timer##sgiSU"].plurals[0] = "Таймер сброса фазы";

    //   sgiSWAN   src/gui/inst/swan.cpp

    strings["Macros##sgiSWAN"].plurals[0] = "Макросы";
    strings["Volume##sgiSWAN"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiSWAN"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiSWAN"].plurals[0] = "Частота";
    strings["Noise##sgiSWAN"].plurals[0] = "Шум";
    strings["Waveform##sgiSWAN"].plurals[0] = "Волна";
    strings["Phase Reset##sgiSWAN"].plurals[0] = "Сброс фазы";

    //   sgiT6W    src/gui/inst/t6w28.cpp

    strings["Macros##sgiT6W"].plurals[0] = "Макросы";
    strings["Volume##sgiT6W"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiT6W"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiT6W"].plurals[0] = "Частота";
    strings["Panning (left)##sgiT6W"].plurals[0] = "Панорамирование (левый)";
    strings["Panning (right)##sgiT6W"].plurals[0] = "Панорамирование (правый)";
    strings["Noise Type##sgiT6W"].plurals[0] = "Режим шума";
    strings["Phase Reset##sgiT6W"].plurals[0] = "Сброс фазы";

    //   sgiTED    src/gui/inst/ted.cpp

    strings["Macros##sgiTED"].plurals[0] = "Макросы";
    strings["Volume##sgiTED"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiTED"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiTED"].plurals[0] = "Частота";
    strings["Square/Noise##sgiTED"].plurals[0] = "Меандр/шум";
    strings["Phase Reset##sgiTED"].plurals[0] = "Сброс фазы";

    //   sgiTIA    src/gui/inst/tia.cpp

    strings["Macros##sgiTIA"].plurals[0] = "Макросы";
    strings["Volume##sgiTIA"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiTIA"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiTIA"].plurals[0] = "Частота";
    strings["Raw Period##sgiTIA"].plurals[0] = "Период (регистровое знач.)";
    strings["Waveform##sgiTIA"].plurals[0] = "Волна";

    //   sgiVB     src/gui/inst/vboy.cpp

    strings["Set modulation table (channel 5 only)##sgiVB"].plurals[0] = "Выставить таблицу модуляции (только для 5-го канала)";
    strings["Hex##MTHex"].plurals[0] = "Шест.##MTHex";
    strings["Dec##MTHex"].plurals[0] = "Дес.##MTHex";
    strings["Macros##sgiVB"].plurals[0] = "Макросы";
    strings["Volume##sgiVB"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiVB"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiVB"].plurals[0] = "Частота";
    strings["Noise Length##sgiVB"].plurals[0] = "Длина шума";
    strings["Waveform##sgiVB"].plurals[0] = "Волна";
    strings["Panning (left)##sgiVB"].plurals[0] = "Панорамирование (левый)";
    strings["Panning (right)##sgiVB"].plurals[0] = "Панорамирование (правый)";
    strings["Phase Reset##sgiVB"].plurals[0] = "Сброс фазы";

    //   sgiVERA   src/gui/inst/vera.cpp

    strings["Macros##sgiVERA"].plurals[0] = "Макросы";
    strings["Volume##sgiVERA"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiVERA"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiVERA"].plurals[0] = "Частота";
    strings["Duty##sgiVERA"].plurals[0] = "Скважность";
    strings["Waveform##sgiVERA"].plurals[0] = "Волна";
    strings["Panning##sgiVERA"].plurals[0] = "Панорамирование";

    //   sgiVIC    src/gui/inst/vic.cpp

    strings["Macros##sgiVIC"].plurals[0] = "Макросы";
    strings["Volume##sgiVIC"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiVIC"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiVIC"].plurals[0] = "Частота";
    strings["On/Off##sgiVIC"].plurals[0] = "Вкл./выкл.";
    strings["Waveform##sgiVIC"].plurals[0] = "Волна";

    //   sgiVRC6   src/gui/inst/vrc6.cpp

    strings["Macros##sgiVRC6"].plurals[0] = "Макросы";
    strings["Volume##sgiVRC6"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiVRC6"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiVRC6"].plurals[0] = "Частота";
    strings["Duty##sgiVRC6"].plurals[0] = "Скважность";
    strings["Waveform##sgiVRC6"].plurals[0] = "Волна";
    strings["Phase Reset##sgiVRC6"].plurals[0] = "Сброс фазы";

    //   sgiVRC6S  src/gui/inst/vrc6saw.cpp

    strings["Macros##sgiVRC6S"].plurals[0] = "Макросы";
    strings["Volume##sgiVRC6S"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiVRC6S"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiVRC6S"].plurals[0] = "Частота";

    //   sgiwave   src/gui/inst/wavetable.cpp

    strings["Wavetable##sgiwave"].plurals[0] = "Волновые таблицы";
    strings["Enable synthesizer##sgiwave"].plurals[0] = "Включить синтезатор";
    strings["Single-waveform##sgiwave"].plurals[0] = "Одна волна";
    strings["Dual-waveform##sgiwave"].plurals[0] = "Две волны";
    strings["Wave 1##sgiwave0"].plurals[0] = "Волна 1";
    strings["Wave 2##sgiwave0"].plurals[0] = "Волна 2";
    strings["Result##sgiwave"].plurals[0] = "Результат";
    strings["Wave 1 ##sgiwave"].plurals[0] = "Волна 1 ";
    strings["waveform macro is controlling wave 1!\nthis value will be ineffective.##sgiwave"].plurals[0] = "макрос волны контролирует волну 1!\nэто значение не даст результата.";
    strings["Wave 1##sgiwave1"].plurals[0] = "Волна 1";
    strings["Wave 2##sgiwave1"].plurals[0] = "Волна 2";
    strings["Resume preview##sgiwave"].plurals[0] = "Возобовить превью";
    strings["Pause preview##sgiwave"].plurals[0] = "Приостановить превью";
    strings["Restart preview##sgiwave"].plurals[0] = "Перезапустить превью";
    strings["too many wavetables!##sgiwave"].plurals[0] = "слишком много волновых таблиц!";
    strings["Copy to new wavetable##sgiwave"].plurals[0] = "Копировать в новую волновую таблицу";
    strings["Update Rate##sgiwave"].plurals[0] = "Частота обновления";
    strings["Speed##sgiwave"].plurals[0] = "Скорость";
    strings["Amount##sgiwave"].plurals[0] = "Количество";
    strings["Power##sgiwave"].plurals[0] = "Степень";
    strings["Global##sgiwave"].plurals[0] = "Глобально";
    strings["Global##sgiwave1"].plurals[0] = "Глобальн.";
    strings["Global##sgiwave2"].plurals[0] = "Глобальн.##jesus";
    strings["wavetable synthesizer disabled.\nuse the Waveform macro to set the wave for this instrument.##sgiwave"].plurals[0] = "синтезатор волновых таблиц выключен.\nиспользуйте макрос волны для задания волновой таблицы для этого инструмента.";
    strings["Local Waves##sgiwave"].plurals[0] = "Локальные волн. табл.";

    //   sgiX1     src/gui/inst/x1_010.cpp

    strings["Macros##sgiX1"].plurals[0] = "Макросы";
    strings["Volume##sgiX1"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiX1"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiX1"].plurals[0] = "Частота";
    strings["Waveform##sgiX1"].plurals[0] = "Волна";
    strings["Panning (left)##sgiX1"].plurals[0] = "Панорамирование (левый)";
    strings["Panning (right)##sgiX1"].plurals[0] = "Панорамирование (правый)";
    strings["Phase Reset##sgiX1"].plurals[0] = "Сброс фазы";
    strings["Envelope##sgiX1"].plurals[0] = "Огибающая";
    strings["Envelope Mode##sgiX1"].plurals[0] = "Режим огибающей";
    strings["AutoEnv Num##sgiX1"].plurals[0] = "Множ. част. авто-огиб.";
    strings["AutoEnv Den##sgiX1"].plurals[0] = "Знам. част. авто-огиб.";

    //   sgiYMZ    src/gui/inst/ymz280b.cpp

    strings["Macros##sgiYMZ"].plurals[0] = "Макросы";
    strings["Volume##sgiYMZ"].plurals[0] = "Громкость";
    strings["Arpeggio##sgiYMZ"].plurals[0] = "Арпеджио";
    strings["Pitch##sgiYMZ"].plurals[0] = "Частота";
    strings["Panning##sgiYMZ"].plurals[0] = "Панорамирование";
    strings["Phase Reset##sgiYMZ"].plurals[0] = "Сброс фазы";

    // no more instruments

    //           src/engine/cmdStream.cpp

    strings["not a command stream"].plurals[0] = "не является потоком команд";

    //   seen    src/engine/engine.cpp

    strings["00xy: Arpeggio##seen"].plurals[0] = "00xy: Арпеджио";
    strings["01xx: Pitch slide up##seen"].plurals[0] = "01xx: Портаменто вверх";
    strings["02xx: Pitch slide down##seen"].plurals[0] = "02xx: Портаменто вниз";
    strings["03xx: Portamento##seen"].plurals[0] = "03xx: Авто-портаменто (до указ. ноты)";
    strings["04xy: Vibrato (x: speed; y: depth)##seen"].plurals[0] = "04xy: Вибрато (x: скорость; y: глубина)";
    strings["05xy: Volume slide + vibrato (compatibility only!)##seen"].plurals[0] = "05xy: Изменение громкости + вибрато (исключительно в целях совместимости!)";
    strings["06xy: Volume slide + portamento (compatibility only!)##seen"].plurals[0] = "06xy: Изменение громкости + портаменто (исключительно в целях совместимости!)";
    strings["07xy: Tremolo (x: speed; y: depth)##seen"].plurals[0] = "07xy: Тремоло (x: скорость; y: глубина)";
    strings["08xy: Set panning (x: left; y: right)##seen"].plurals[0] = "08xy: Панорамирование (x: лево; y: право)";
    strings["09xx: Set groove pattern (speed 1 if no grooves exist)##seen"].plurals[0] = "09xx: Установить ритм-паттерн (скорость 1 при их отсутствии)";
    strings["0Axy: Volume slide (0y: down; x0: up)##seen"].plurals[0] = "0Axy: Изменение громкости (0y: вниз; x0: вверх)";
    strings["0Bxx: Jump to pattern##seen"].plurals[0] = "0Bxx: Прыжок на паттерн";
    strings["0Cxx: Retrigger##seen"].plurals[0] = "0Cxx: Циклич. перезапуск";
    strings["0Dxx: Jump to next pattern##seen"].plurals[0] = "0Dxx: Прыжок на след. паттерн";
    strings["0Fxx: Set speed (speed 2 if no grooves exist)##seen"].plurals[0] = "0Fxx: Установить скорость (скорость 2 при отсутствии ритм-паттернов)";
    strings["80xx: Set panning (00: left; 80: center; FF: right)##seen"].plurals[0] = "80xx: Панорамирование (00: лево; 80: центр; FF: право)";
    strings["81xx: Set panning (left channel)##seen"].plurals[0] = "81xx: Панорамирование (левый канал)";
    strings["82xx: Set panning (right channel)##seen"].plurals[0] = "82xx: Панорамирование (правый канал)";
    strings["88xy: Set panning (rear channels; x: left; y: right)##seen"].plurals[0] = "88xy: Панорамирование (задние каналы; x: левый; y: правый)";
    strings["89xx: Set panning (rear left channel)##seen"].plurals[0] = "89xx: Панорамирование (задний левый канал)";
    strings["8Axx: Set panning (rear right channel)##seen"].plurals[0] = "8Axx: Панорамирование (задний правый канал)";
    strings["Cxxx: Set tick rate (hz)##seen"].plurals[0] = "Cxxx: Установить частоту движка трекера (Гц)";
    strings["E0xx: Set arp speed##seen"].plurals[0] = "E0xx: Установить скорость арпеджио";
    strings["E1xy: Note slide up (x: speed; y: semitones)##seen"].plurals[0] = "E1xy: Портаменто вверх (x: скорость; y: полутонов)";
    strings["E2xy: Note slide down (x: speed; y: semitones)##seen"].plurals[0] = "E2xy: Портаменто вниз (x: скорость; y: полутонов)";
    strings["E3xx: Set vibrato shape (0: up/down; 1: up only; 2: down only)##seen"].plurals[0] = "E3xx: Тип вибрато (0: вверх/вниз; 1: только вверх; 2: только вниз)";
    strings["E4xx: Set vibrato range##seen"].plurals[0] = "E4xx: Установить глубину вибрато";
    strings["E5xx: Set pitch (80: center)##seen"].plurals[0] = "E5xx: Расстройка (80: без расстройки)";
    strings["E6xy: Delayed note transpose (x: 0-7 = up, 8-F = down (after (x % 7) ticks); y: semitones)##seen"].plurals[0] = "E6xy: Отложенное транспонирование ноты (x: 0-7 = вверх, 8-F = вниз (после (x % 7) шагов движка); y: полутонов)";
    strings["E7xx: Macro release##seen"].plurals[0] = "E7xx: Релиз макросов";
    strings["E8xy: Delayed note transpose up (x: ticks; y: semitones)##seen"].plurals[0] = "E8xy: Отложенное транспонирование ноты вверх (x: шагов движка; y: полутонов)";
    strings["E9xy: Delayed note transpose down (x: ticks; y: semitones)##seen"].plurals[0] = "E9xy: Отложенное транспонирование ноты вниз (x: шагов движка; y: полутонов)";
    strings["EAxx: Legato##seen"].plurals[0] = "EAxx: Легато";
    strings["EBxx: Set LEGACY sample mode bank##seen"].plurals[0] = "EBxx: (СОВМЕСТИМОСТЬ) Установить банк сэмплов";
    strings["ECxx: Note cut##seen"].plurals[0] = "ECxx: Заглушить ноту";
    strings["EDxx: Note delay##seen"].plurals[0] = "EDxx: Задержать ноту";
    strings["EExx: Send external command##seen"].plurals[0] = "EExx: Послать внешнюю команду";
    strings["F0xx: Set tick rate (bpm)##seen"].plurals[0] = "F0xx: Установить частоту движка трекера (BPM)";
    strings["F1xx: Single tick note slide up##seen"].plurals[0] = "F1xx: Портаменто вверх (один шаг движка)";
    strings["F2xx: Single tick note slide down##seen"].plurals[0] = "F2xx: Портаменто вниз (один шаг движка)";
    strings["F3xx: Fine volume slide up##seen"].plurals[0] = "F3xx: Точное изменение громкости вверх";
    strings["F4xx: Fine volume slide down##seen"].plurals[0] = "F4xx: Точное изменение громкости вниз";
    strings["F5xx: Disable macro (see manual)##seen"].plurals[0] = "F5xx: Отключить макрос (см. инструкцию)";
    strings["F6xx: Enable macro (see manual)##seen"].plurals[0] = "F6xx: Включить макрос (см. инструкцию)";
    strings["F7xx: Restart macro (see manual)##seen"].plurals[0] = "F7xx: Перезапустить макрос (см. инструкцию)";
    strings["F8xx: Single tick volume slide up##seen"].plurals[0] = "F8xx: Изменение громкости вверх (один шаг движка)";
    strings["F9xx: Single tick volume slide down##seen"].plurals[0] = "F9xx: Изменение громкости вниз (один шаг движка)";
    strings["FAxx: Fast volume slide (0y: down; x0: up)##seen"].plurals[0] = "FAxx: Быстрое изменение громкости (0y: вниз; x0: вверх)";
    strings["FCxx: Note release##seen"].plurals[0] = "FCxx: Релиз ноты";
    strings["FDxx: Set virtual tempo numerator##seen"].plurals[0] = "FDxx: Числитель виртуального темпа";
    strings["FExx: Set virtual tempo denominator##seen"].plurals[0] = "FExx: Знаменатель виртуального темпа";
    strings["FFxx: Stop song##seen"].plurals[0] = "FFxx: Остановить трек";
    strings["9xxx: Set sample offset*256##seen"].plurals[0] = "9xxx: Начальное смещение сэмпла (xxx*256 шагов)";
    strings["90xx: Set sample offset (first byte)##seen"].plurals[0] = "90xx: Начальное смещение сэмпла (младший байт)";
    strings["91xx: Set sample offset (second byte, ×256)##seen"].plurals[0] = "91xx: Начальное смещение сэмпла (средний байт, ×256)";
    strings["92xx: Set sample offset (third byte, ×65536)##seen"].plurals[0] = "92xx: Начальное смещение сэмпла (старший байт, ×65536)";

    strings["on seek: %s"].plurals[0] = "во время перехода по файлу: %s";
    strings["on pre tell: %s"].plurals[0] = "перед запросом положения в файле: %s";
    strings["file is empty"].plurals[0] = "пустой файл";
    strings["on tell: %s"].plurals[0] = "во время запроса положения в файле: %s";
    strings["ROM size mismatch, expected: %d bytes, was: %d"].plurals[0] = "несоответствие размера ROM, ожидалось: %d байт, на самом деле: %d";
    strings["on get size: %s"].plurals[0] = "при запросе размера: %s";
    strings["on read: %s"].plurals[0] = "при чтении: %s";
    strings["invalid index"].plurals[0] = "недействительный индекс";
    strings["max number of total channels is %d"].plurals[0] = "максимальное общее число каналов равно %d";
    strings["max number of systems is %d"].plurals[0] = "максимальное число чипов/систем равно %d";
    strings["cannot remove the last one"].plurals[0] = "не могу удалить последнюю";
    strings["source and destination are equal"].plurals[0] = "системы совпадают";
    strings["invalid source index"].plurals[0] = "недействительный индекс исходной системы";
    strings["invalid destination index"].plurals[0] = "недействительный индекс системы назначения";
    strings["too many wavetables!"].plurals[0] = "слишком много волновых таблиц!";
    strings["could not seek to end: %s"].plurals[0] = "не смог перейти в конец файла: %s";
    strings["could not determine file size: %s"].plurals[0] = "не смог определить размер файла: %s";
    strings["file size is invalid!"].plurals[0] = "недействительный размер файла!";
    strings["could not seek to beginning: %s"].plurals[0] = "не смог перейти в начало файла: %s";
    strings["could not read entire file: %s"].plurals[0] = "не смог прочитать весь файл: %s";
    strings["invalid wavetable header/data!"].plurals[0] = "неправильный заголовок/данные волновой таблицы!";
    strings["premature end of file"].plurals[0] = "преждевременный конец файла";
    strings["too many samples!"].plurals[0] = "слишком много сэмплов!";
    strings["no free patterns in channel %d!"].plurals[0] = "нет свободных паттернов для канала %d!";
    
    //           src/engine/fileOps.cpp
        
    strings["this module was created with a more recent version of Furnace!"].plurals[0] = "этот модуль был создан в более новой версии Furnace!";
    strings["couldn't seek to info header!"].plurals[0] = "не смог перейти к заголовку с информацией!";
    strings["invalid info header!"].plurals[0] = "неправильный заголовок с информацией!";
    strings["pattern length is negative!"].plurals[0] = "отрицательная длина паттерна!";
    strings["pattern length is too large!"].plurals[0] = "слишком большая длина паттерна!";
    strings["song length is negative!"].plurals[0] = "отрицательная длина трека!";
    strings["song is too long!"].plurals[0] = "трек слишком длинный!";
    strings["invalid instrument count!"].plurals[0] = "неправильное число инструментов!";
    strings["invalid wavetable count!"].plurals[0] = "неправильное число волновых таблиц!";
    strings["invalid sample count!"].plurals[0] = "неправильное число сэмплов!";
    strings["invalid pattern count!"].plurals[0] = "неправильное число паттернов!";
    strings["unrecognized system ID %.2x!"].plurals[0] = "неизвестный индекс системы %.2x!";
    strings["zero chips!"].plurals[0] = "нулевое число чипов!";
    strings["channel %d has too many effect columns! (%d)"].plurals[0] = "канал %d содержит слишком много столбцов эффектов! (%d)";
    strings["couldn't seek to chip %d flags!"].plurals[0] = "не смог перейти к флагам чипа %d!";
    strings["invalid flag header!"].plurals[0] = "неправильный заголовок флагов!";
    strings["couldn't read instrument directory"].plurals[0] = "не смог прочитать папку с инструментами";
    strings["invalid instrument directory data!"].plurals[0] = "неправильные данные папки с инструментами!";
    strings["couldn't read wavetable directory"].plurals[0] = "не смог прочитать папку с волновыми таблицами";
    strings["invalid wavetable directory data!"].plurals[0] = "неправильные данные папки с волновыми таблицами!";
    strings["couldn't read sample directory"].plurals[0] = "не смог прочитать папку с сэмплами";
    strings["invalid sample directory data!"].plurals[0] = "неправильные данные папки с сэмплами!";
    strings["couldn't seek to subsong %d!"].plurals[0] = "не смог перейти к подпесне %d!";
    strings["invalid subsong header!"].plurals[0] = "неправильный заголовок подпесни!";
    strings["couldn't seek to instrument %d!"].plurals[0] = "не смог перейти к инструменту %d!";
    strings["invalid instrument header/data!"].plurals[0] = "неправильный заголовок/данные инструмента!";
    strings["couldn't seek to wavetable %d!"].plurals[0] = "не смог перейти к волновой таблице %d!";
    strings["couldn't seek to sample %d!"].plurals[0] = "не смог перейти к сэмплу %d!";
    strings["invalid sample header/data!"].plurals[0] = "неправильный заголовок/данные сэмпла!";
    strings["couldn't seek to pattern in %x!"].plurals[0] = "не смог перейти к паттерну в %x!";
    strings["invalid pattern header!"].plurals[0] = "неправильный заголовок паттерна!";
    strings["pattern channel out of range!"].plurals[0] = "канал паттерна за пределами числа каналов!";
    strings["pattern index out of range!"].plurals[0] = "индекс паттерна за пределами числа паттернов!";
    strings["pattern subsong out of range!"].plurals[0] = "подпесня паттерна за пределами числа подпесен!";
    strings["incomplete file"].plurals[0] = "неполный файл";
    strings["file is too small"].plurals[0] = "файл слишком маленький";
    strings["not a .dmf/.fur/.fub song"].plurals[0] = "не является модулем .dmf/.fur/.fub";
    strings["unknown decompression error"].plurals[0] = "неизвестная ошибка распаковки";
    strings["decompression error: %s"].plurals[0] = "ошибка распаковки: %s";
    strings["unknown decompression finish error"].plurals[0] = "неизвестная ошибка при завершении распаковки";
    strings["decompression finish error: %s"].plurals[0] = "ошибка при завершении распаковки: %s";
    strings["not a compatible song/instrument"].plurals[0] = "не является совместимым модулем/файлом инструмента";
    strings["maximum number of instruments is 256"].plurals[0] = "максимальное число инструментов равно 256";
    strings["maximum number of wavetables is 256"].plurals[0] = "максимальное число волновых таблиц равно 256";
    strings["maximum number of samples is 256"].plurals[0] = "максимальное число сэмплов равно 256";

    //           src/engine/fileOpsIns.cpp

    strings["did not read entire instrument file!"].plurals[0] = "не смог прочитать весь файл инструмента!";
    strings["this instrument is made with a more recent version of Furnace!"].plurals[0] = "этот инструмент был создан в более новой версии Furnace!";
    strings["unknown instrument format"].plurals[0] = "неизвестный формат инструмента";
    strings["there is more data at the end of the file! what happened here!"].plurals[0] = "в конце файла содержатся ещё данные! что происходит!";
    strings["exactly %d bytes, if you are curious"].plurals[0] = "а именно %d байт, если вам интересно";

    //           src/engine/fileOpsSample.cpp

    strings["could not open file! (%s)"].plurals[0] = "не смог открыть файл! (%s)";
    strings["could not get file length! (%s)"].plurals[0] = "не смог определить длину файла! (%s)";
    strings["file is empty!"].plurals[0] = "файл пустой!";
    strings["file is invalid!"].plurals[0] = "файл повреждён/слишком большой!";
    strings["could not seek to beginning of file! (%s)"].plurals[0] = "не смог перейти к началу файла! (%s)";
    strings["wait... is that right? no I don't think so..."].plurals[0] = "подождите... так вообще правильно? нет, я так не думаю...";
    strings["BRR sample is empty!"].plurals[0] = "BRR-сэмпл пуст!";
    strings["possibly corrupt BRR sample!"].plurals[0] = "BRR-сэмпл, возможно, повреждён!";
    strings["could not read file! (%s)"].plurals[0] = "не смог прочитать файл! (%s)";
    strings["Furnace was not compiled with libsndfile!"].plurals[0] = "Furnace не был скомпилирован с libsndfile!";
    strings["could not open file! (%s %s)"].plurals[0] = "не смог открыть файл! (%s %s)";
    strings["could not open file! (%s)\nif this is raw sample data, you may import it by right-clicking the Load Sample icon and selecting \"import raw\"."].plurals[0] = "не смог открыть файл! (%s)\nесли это сырые данные сэмпла, попробуйте импортировать их: ПКМ по иконке \"Открыть\" в списке сэмплов, выберите \"импорт сырых данных\".";
    strings["this sample is too big! max sample size is 16777215."].plurals[0] = "сэмпл слишком большой! максимальный размер сэмпла 16777215.";

    //           src/engine/importExport/bnk.cpp

    strings["GEMS BNK currently not supported."].plurals[0] = "GEMS BNK пока не поддерживается.";

    //           src/engine/importExport/fzt.cpp

    strings["You are using %02Xxx effect with param higher than 0xf (channel %d, pattern %d, row %d).\nThe effect param will be capped at 0xf.\n\n"].plurals[0] = "Вы используете эффект %02Xxx с параметром, превышающим 0xf (канал %d, паттерн %d, строка %d).\nПараметр эффекта будет уменьшен до 0xf.\n\n";
    strings["You are using %02Xxx effect which is not supported by FZT format (channel %d, pattern %d, row %d).\n\n"].plurals[0] = "Вы используете эффект %02Xxx, который не поддерживается форматом FZT (канал %d, паттерн %d, строка %d).\n\n";
    strings["invalid pattern length!"].plurals[0] = "Неправильная длина паттерна!";
    strings["invalid loop start and loop end!"].plurals[0] = "неправильные начало и конец зацикленной части трека!";
    strings["invalid orders length!"].plurals[0] = "неправильное число строк матрицы паттернов!";
    strings["instrument program is too long!"].plurals[0] = "слишком длинная программа инструмента!";
    strings["couldn't place 0Bxx command to make a loop point"].plurals[0] = "не смог разместить команду 0Bxx для создания точки зацикливания";
    strings["song contains more than one system."].plurals[0] = "в треке содержится больше одной системы.";
    strings["system is not FZT sound source."].plurals[0] = "система не является источником звука FZT.";
    strings["you have no subsongs in the module."].plurals[0] = "в модуле нет подпесен.";
    strings["you must have at least one instrument in the song."].plurals[0] = "необходимо наличие хотя бы одного инструмента.";
    strings["song name is too long. Only first %d characters will be written.\n\n"].plurals[0] = "слишком длинное название трека. Будут записаны только первые %d символов.\n\n";
    strings[
        "you have 0Bxx command placed not on the last pattern row (channel %d, pattern %d, row %d, effect column %d).\n"
        "FZT export will try to loop your song as if it was placed on last pattern row.\n\n"].plurals[0] = 
        
        "вы разместили команду 0Bxx не на последней строке паттерна (канал %d, паттерн %d, строка %d, столбец эффектов %d).\n"
        "экспорт FZT попытается зациклить трек, исходя из того, что эффект как бы размещён на последней строке паттерна.\n\n";
    strings[
        "There wasn't any 0Bxx command, so your song won't be looped.\n"
        "To make song loop, place 0Bxx command somewhere in the song on the last pattern row.\n\n"].plurals[0] =

        "Нет ни одного эффекта 0Bxx, поэтому ваш трек не будет зациклен.\n"
        "Для зацикливания трека разместите команду 0Bxx в конце какого-либо паттерна.\n\n";
    strings["there are more than %d patterns in the song. only %d patterns will be saved.\n\n"].plurals[0] = "в треке содержится более %d паттернов. только первые %d паттернов будут сохранены.\n\n";
    strings["you are using two speeds or groove patterns which are not supported in FZT.\nFirst speed will be used.\n\n"].plurals[0] = "вы используете две скорости или ритм-паттерны, которые не поддерживаются в FZT.\nБудет использована первая скорость.\n\n";
    strings["you are using virtual tempo which is not supported in FZT.\nTo stop using virtual tempo, simply make virtual tempo numerator and denominator equal.\nFZT does not support virtual tempo, so the settings will be ignored.\n\n"].plurals[0] = "вы используете виртуальный темп, который не поддерживается в FZT.\nДля того, чтобы прекратить его использование, просто выставьте равные значения в числителе и знаменателе виртуального темпа.\nFZT не поддерживает виртуальный темп, поэтому его настройки будут проигнорированы.\n\n";
    strings["Your song rate is higher than 255 Hz. It will be capped at 255 Hz in FZT file.\n\n"].plurals[0] = "Частота двидка трекера превышает 255 Гц. Она будет ограничена до 255 Гц в файле FZT.\n\n";
    strings["You are setting engine rate that is higher than 255 Hz (channel %d, pattern %d, row %d, effect column %d).\nThe command(s) will be capped at 255 Hz.\n\n"].plurals[0] = "Вы устанавливаете частоту движка трекера, превышающую 255 Гц (канал %d, паттерн %d, строка %d, столбец эффектов %d).\nКоманда(-ы) будет(-ут) ограничены до 255 Гц.\n\n";
    strings["You are using instrument index that is higher than %02X (channel %d, pattern %d, row %d).\nThe index will be capped at %02X.\n\n"].plurals[0] = "Вы используете индекс инструмента, который превышает %02X (канал %d, паттерн %d, строка %d).\nИндекс будет ограничен до %02X.\n\n";
    strings["You are using virtual tempo control effect (channel %d, pattern %d, row %d).\nThe effect is not supported in FZT and will be ignored.\n\n"].plurals[0] = "Вы используете эффект управления виртуальным темпом (канал %d, паттерн %d, строка %d).\nЭффект не поддерживается в FZT и будет проигнорирован.\n\n";
    strings["You are using macros in instrument %d.\nFZT does not support macros, so they will be ignored.\n\n"].plurals[0] = "Вы используете макросы в инструменте %d.\nFZT не поддерживает макросы, поэтому они будут проигнорированы.\n\n";

    //           src/engine/importExport/dmf.cpp

    strings["this version is not supported by Furnace yet"].plurals[0] = "эта версия пока не поддерживается Furnace";
    strings["system not supported. running old version?"].plurals[0] = "система не поддерживается. вы на старой версии?";
    strings["Yamaha YMU759 emulation is incomplete! please migrate your song to the OPL3 system."].plurals[0] = "Эмуляция Yamaha YMU759 неполноценна! переделайте свой трек под OPL3.";
    strings["order at %d, %d out of range! (%d)"].plurals[0] = "значение в матрице паттернов %d, %d недействительно! (%d)";
    strings["file is corrupt or unreadable at operators"].plurals[0] = "файл повреждён/нечитаем в секции операторов";
    strings["file is corrupt or unreadable at wavetables"].plurals[0] = "файл повреждён/нечитаем в секции волновых таблиц";
    strings["file is corrupt or unreadable at effect columns"].plurals[0] = "файл повреждён/нечитаем в секции столбцов эффектов";
    strings["file is corrupt or unreadable at samples"].plurals[0] = "файл повреждён/нечитаем в секции сэмплов";
    strings["invalid version to save in! this is a bug!"].plurals[0] = "енправильная версия для сохранения! это баг!";
    strings["multiple systems not possible on .dmf"].plurals[0] = "несколько чипов/систем невозможно сохранить в .dmf";
    strings["YMU759 song saving is not supported"].plurals[0] = "Сохранение треков с YMU759 не поддерживается";
    strings["Master System FM expansion not supported in 1.0/legacy .dmf!"].plurals[0] = "Master System FM расширение не поддерживается в 1.0/legacy .dmf!";
    strings["NES + VRC7 not supported in 1.0/legacy .dmf!"].plurals[0] = "NES + VRC7 не поддерживаются в 1.0/legacy .dmf!";
    strings["FDS not supported in 1.0/legacy .dmf!"].plurals[0] = "FDS не поддерживается в 1.0/legacy .dmf!";
    strings["this system is not possible on .dmf"].plurals[0] = "эта система не поддерживается в .dmf";
    strings["maximum .dmf song length is 127"].plurals[0] = "маскимальная длина трека .dmf составляет 127";
    strings["maximum number of instruments in .dmf is 128"].plurals[0] = "максимальное число инструментов в .dmf равно 128";
    strings["maximum number of wavetables in .dmf is 64"].plurals[0] = "максимальное число волновых таблиц в .dmf равно 64";
    strings["order %d, %d is out of range (0-127)"].plurals[0] = "значение в матрице паттернов %d, %d недействительно (0-127)";
    strings["only the currently selected subsong will be saved"].plurals[0] = "будет сохранена только текущая подпесня";
    strings["grooves will not be saved"].plurals[0] = "ритм-паттерны не будут сохранены";
    strings["only the first two speeds will be effective"].plurals[0] = "только первые две скорости будут применены";
    strings[".dmf format does not support virtual tempo"].plurals[0] = ".dmf не поддерживает виртуальный темп";
    strings[".dmf format does not support tuning"].plurals[0] = ".dmf не поддерживает настройку строя (частоты ноты A-4)";
    strings["absolute duty/cutoff macro not available in .dmf!"].plurals[0] = "абсолютные макросы частоты среза/скважности не поддерживаются в .dmf!";
    strings["duty precision will be lost"].plurals[0] = "точность настройки скважности не будет сохранена";
    strings[".dmf format does not support arbitrary-pitch sample mode"].plurals[0] = ".dmf не поддерживает проигрывание сэмплов с произвольной частотой";
    strings["no FM macros in .dmf format"].plurals[0] = "формат .dmf не поддерживает FM-макросы";
    strings[".dmf only supports volume or cutoff macro in C64, but not both. volume macro will be lost."].plurals[0] = ".dmf позволяет применять для C64 либо макрос громкости, либо макрос частоты среза, но не оба одновременно. макрос громкости не будет сохранён.";
    strings["note/macro release will be converted to note off!"].plurals[0] = "ноты релиза макросов или огибающей будут преобразованы в заглушение ноты!";
    strings["samples' rates will be rounded to nearest compatible value"].plurals[0] = "частоты дискретизации сэмплов будут преобразованы в ближайшие совместимые.";

    //           src/engine/importExport/dmp.cpp

    strings["unknown instrument type %d!"].plurals[0] = "неизвестный тип инструмента %d!";

    //           src/engine/importExport/fc.cpp

    strings["invalid header!"].plurals[0] = "неправильный заголовок";

    //           src/engine/importExport/ftm.cpp

    strings["incompatible version"].plurals[0] = "несовместимая версия";
    strings["channel counts do not match"].plurals[0] = "количества каналов не совпадают";
    strings["too many instruments/out of range"].plurals[0] = "слишком много инструментов/недействительное значение";
    strings["invalid instrument type"].plurals[0] = "неизвестный тип инструмента";
    strings["too many sequences"].plurals[0] = "слишком много последовательностей (макросов)";
    strings["sequences block version is too old"].plurals[0] = "слишком старая версия блока последовательностей";
    strings["unknown block "].plurals[0] = "неизвестный блок ";
    strings["incomplete block "].plurals[0] = "неполный блок ";
    strings[" [VRC6 copy]"].plurals[0] = " [копия для VRC6]";
    strings[" [VRC6 saw copy]"].plurals[0] = " [копия для пилы VRC6]";
    strings[" [NES copy]"].plurals[0] = " [копия для NES]";

    //           src/engine/importExport/gyb.cpp

    strings["GYBv3 file appears to have invalid data offsets."].plurals[0] = "Похоже, что в файле GYBv3 неверные смещения (указатели на данные).";
    strings["Invalid value found in patch file. %s"].plurals[0] = "Найдено недействительное значение в файле патча. %s";

    //           src/engine/importExport/s3i.cpp

    strings["S3I PCM samples currently not supported."].plurals[0] = "S3I: ИКМ-сэмплы пока не поддерживаются.";

    //           src/engine/importExport/tfm.cpp

    strings["interleave factor is bigger than 8, speed information may be inaccurate"].plurals[0] = "параметр чередования больше 8, скорость проигрывания трека может быть неправильной";

    //           src/engine/vgmOps.cpp

    strings["VGM version is too low"].plurals[0] = "Версия VGM слишком старая";

    //names of memory composition memories

    strings["DPCM"].plurals[0] = "ДИКМ";
    strings["Chip Memory"].plurals[0] = "Память чипа";
    strings["Sample ROM"].plurals[0] = "ПЗУ сэмплов";
    strings["Sample Memory"].plurals[0] = "Память сэмплов";
    strings["SPC/DSP Memory"].plurals[0] = "Память SPC/DSP";
    strings["Sample RAM"].plurals[0] = "ОЗУ сэмплов";
    strings["ADPCM"].plurals[0] = "АДИКМ";

    //names of memory entries

    strings["Sample"].plurals[0] = "Сэмпл";
    strings["Wave RAM"].plurals[0] = "ОЗУ волн";
    strings["End of Sample"].plurals[0] = "Конец сэмпла";
    strings["Wavetable RAM"].plurals[0] = "ОЗУ под волновую таблицу";
    strings["Reserved wavetable RAM"].plurals[0] = "ОЗУ, зарезервированная под волновую таблицу";
    strings["Phrase Book"].plurals[0] = "Книга фраз";
    strings["Channel %d (load)"].plurals[0] = "Канал %d (загрузка)";
    strings["Channel %d (play)"].plurals[0] = "Канал %d (проигрывание)";
    strings["Channel %d"].plurals[0] = "Канал %d";
    strings["Buffer %d Left"].plurals[0] = "Буфер %d лево";
    strings["Buffer %d Right"].plurals[0] = "Буфер %d право";
    strings["Registers"].plurals[0] = "Регистры";
    strings["PCM"].plurals[0] = "ИКМ";
    strings["ADPCM"].plurals[0] = "АДИКМ";
    strings["State"].plurals[0] = "Состояние";
    strings["Sample Directory"].plurals[0] = "Секция сэмплов";
    strings["Echo Buffer"].plurals[0] = "Буфер эхо";
    strings["Mix/Echo Buffer"].plurals[0] = "Буфер микширования/эхо";
    strings["Main Memory"].plurals[0] = "Основная память";

    //   sesd    src/engine/sysDef.cpp

    strings["20xx: Set channel mode (bit 0: square; bit 1: noise; bit 2: envelope)##sesd"].plurals[0] = "20xx: Режим канала (бит 0: меандр; бит 1: шум; бит 2: огибающая)";
    strings["21xx: Set noise frequency (0 to 1F)##sesd"].plurals[0] = "21xx: Частота шума (0-1F)";
    strings["21xx: Set noise frequency (0 to FF)##sesd"].plurals[0] = "21xx: Частота шума (0-FF)";
    strings["22xy: Set envelope mode (x: shape, y: enable for this channel)##sesd"].plurals[0] = "22xy: Режим огибающей (x: форма, y: включить для этого канала)";
    strings["23xx: Set envelope period low byte##sesd"].plurals[0] = "23xx: Младший байт периода огибающей";
    strings["24xx: Set envelope period high byte##sesd"].plurals[0] = "24xx: Старший байт периода огибающей";
    strings["25xx: Envelope slide up##sesd0"].plurals[0] = "25xx: Портаменто огибающей вверх";
    strings["26xx: Envelope slide down##sesd0"].plurals[0] = "26xx: Портаменто огибающей вниз";
    strings["29xy: Set auto-envelope (x: numerator; y: denominator)##sesd0"].plurals[0] = "29xy: Авто-огибающая (x: числитель; y: знаменатель)";
    strings["2Exx: Write to I/O port A##sesd"].plurals[0] = "2Exx: Запись в порт ввода-вывода A";
    strings["2Fxx: Write to I/O port B##sesd"].plurals[0] = "2Fxx: Запись в порт ввода-вывода B";
    strings["12xx: Set duty cycle (0 to 8)##sesd"].plurals[0] = "12xx: Скважность (0-8)";
    strings["16xx: Set raw period higher byte##sesd"].plurals[0] = "16xx: \"Сырое\" значение периода (старший байт)";
    strings["27xx: Set noise AND mask##sesd"].plurals[0] = "27xx: Маска шума И";
    strings["28xx: Set noise OR mask##sesd"].plurals[0] = "28xx: Маска шума ИЛИ";
    strings["2Cxy: Automatic noise frequency (auto-PWM) (x: mode (0: disable, 1: freq, 2: freq + OR mask); y: offset##sesd"].plurals[0] = "2Cxy: Автоматическая частота шума (авто-ШИМ) (x: режим (0: выкл., 1: частота, 2: частота + маска ИЛИ); y: сдвиг";
    strings["2Dxx: NOT TO BE EMPLOYED BY THE COMPOSER##sesd"].plurals[0] = "2Dxx: НЕ ДЛЯ ИСПОЛЬЗОВАНИЯ КОМПОЗИТОРОМ";
    strings["30xx: Toggle hard envelope reset on new notes##sesd"].plurals[0] = "30xx: Переключить жёсткий перезапуск огибающей на новой ноте";
    strings["18xx: Toggle extended channel 3 mode##sesd"].plurals[0] = "18xx: Переключить расширенный режим 3-го канала";
    strings["17xx: Toggle PCM mode (LEGACY)##sesd0"].plurals[0] = "17xx: Переключить режим ИКМ (СОВМЕСТИМОСТЬ)";
    strings["DFxx: Set sample playback direction (0: normal; 1: reverse)##sesd0"].plurals[0] = "DFxx: Направление проигрывания сэмпла (0: обычное; 1: обратное)";
    strings["18xx: Toggle drums mode (1: enabled; 0: disabled)##sesd"].plurals[0] = "18xx: Переключить режим ударных (1: включить; 0: выключить)";
    strings["11xx: Set feedback (0 to 7)##sesd0"].plurals[0] = "11xx: Обратная связь (0-7)";
    strings["12xx: Set level of operator 1 (0 highest, 7F lowest)##sesd"].plurals[0] = "12xx: Уровень оператора 1 (0 макс., 7F мин.)";
    strings["13xx: Set level of operator 2 (0 highest, 7F lowest)##sesd"].plurals[0] = "13xx: Уровень оператора 2 (0 макс., 7F мин.)";
    strings["14xx: Set level of operator 3 (0 highest, 7F lowest)##sesd"].plurals[0] = "14xx: Уровень оператора 3 (0 макс., 7F мин.)";
    strings["15xx: Set level of operator 4 (0 highest, 7F lowest)##sesd"].plurals[0] = "15xx: Уровень оператора 4 (0 макс., 7F мин.)";
    strings["16xy: Set operator multiplier (x: operator from 1 to 4; y: multiplier)##sesd0"].plurals[0] = "16xy: Множитель частоты (x: оператор (1-4); y: множитель)";
    strings["19xx: Set attack of all operators (0 to 1F)##sesd"].plurals[0] = "19xx: Атака всех операторов (0-1F)";
    strings["1Axx: Set attack of operator 1 (0 to 1F)##sesd"].plurals[0] = "1Axx: Атака оператора 1 (0-1F)";
    strings["1Bxx: Set attack of operator 2 (0 to 1F)##sesd"].plurals[0] = "1Bxx: Атака оператора 2 (0-1F)";
    strings["1Cxx: Set attack of operator 3 (0 to 1F)##sesd"].plurals[0] = "1Cxx: Атака оператора 3 (0-1F)";
    strings["1Dxx: Set attack of operator 4 (0 to 1F)##sesd"].plurals[0] = "1Dxx: Атака оператора 4 (0-1F)";
    strings["50xy: Set AM (x: operator from 1 to 4 (0 for all ops); y: AM)##sesd0"].plurals[0] = "50xy: Включить АМ (x: оператор 1-4 (0 для всех операторов); y: АМ)";
    strings["51xy: Set sustain level (x: operator from 1 to 4 (0 for all ops); y: sustain)##sesd0"].plurals[0] = "51xy: Уровень сустейна (x: оператор 1-4 (0 для всех операторов); y: уровень)";
    strings["52xy: Set release (x: operator from 1 to 4 (0 for all ops); y: release)##sesd0"].plurals[0] = "52xy: Релиз (x: оператор 1-4 (0 для всех операторов); y: релиз)";
    strings["53xy: Set detune (x: operator from 1 to 4 (0 for all ops); y: detune where 3 is center)##sesd"].plurals[0] = "53xy: Расстройка (x: оператор 1-4 (0 для всех операторов); y: расстройка (3 - без расстройки))";
    strings["54xy: Set envelope scale (x: operator from 1 to 4 (0 for all ops); y: scale from 0 to 3)##sesd0"].plurals[0] = "54xy: Масш. огиб. (x: оператор 1-4 (0 для всех операторов); y: масш. 0-3)";
    strings["56xx: Set decay of all operators (0 to 1F)##sesd"].plurals[0] = "56xx: Спад всех операторов (0-1F)";
    strings["57xx: Set decay of operator 1 (0 to 1F)##sesd"].plurals[0] = "57xx: Спад оператора 1 (0-1F)";
    strings["58xx: Set decay of operator 2 (0 to 1F)##sesd"].plurals[0] = "58xx: Спад оператора 2 (0-1F)";
    strings["59xx: Set decay of operator 3 (0 to 1F)##sesd"].plurals[0] = "59xx: Спад оператора 3 (0-1F)";
    strings["5Axx: Set decay of operator 4 (0 to 1F)##sesd"].plurals[0] = "5Axx: Спад оператора 4 (0-1F)";
    strings["5Bxx: Set decay 2 of all operators (0 to 1F)##sesd"].plurals[0] = "5Bxx: Спад 2 всех операторов (0-1F)";
    strings["5Cxx: Set decay 2 of operator 1 (0 to 1F)##sesd"].plurals[0] = "5Cxx: Спад 2 оператора 1 (0-1F)";
    strings["5Dxx: Set decay 2 of operator 2 (0 to 1F)##sesd"].plurals[0] = "5Dxx: Спад 2 оператора 2 (0-1F)";
    strings["5Exx: Set decay 2 of operator 3 (0 to 1F)##sesd"].plurals[0] = "5Exx: Спад 2 оператора 3 (0-1F)";
    strings["5Fxx: Set decay 2 of operator 4 (0 to 1F)##sesd"].plurals[0] = "5Fxx: Спад 2 оператора 4 (0-1F)";
    strings["10xx: Set noise frequency (xx: value; 0 disables noise)##sesd"].plurals[0] = "10xx: Частота шума (xx: частота; 0 отключает шум)";
    strings["17xx: Set LFO speed##sesd"].plurals[0] = "17xx: Установить частоту ГНЧ";
    strings["18xx: Set LFO waveform (0 saw, 1 square, 2 triangle, 3 noise)##sesd"].plurals[0] = "18xx: Форма волны ГНЧ (0 пила, 1 меандр, 2 треуг., 3 шум)";
    strings["1Exx: Set AM depth (0 to 7F)##sesd"].plurals[0] = "1Exx: Глубина АМ (0-7F)";
    strings["1Fxx: Set PM depth (0 to 7F)##sesd"].plurals[0] = "1Fxx: Глубина ФМ (0-7F)";
    strings["55xy: Set detune 2 (x: operator from 1 to 4 (0 for all ops); y: detune from 0 to 3)##sesd"].plurals[0] = "55xy: Расстройка 2 (x: оператор 1-4 (0 для всех операторов); y: расстройка 0-3)";
    strings["24xx: Set LFO 2 speed##sesd"].plurals[0] = "24xx: Установить частоту ГНЧ 2";
    strings["25xx: Set LFO 2 waveform (0 saw, 1 square, 2 triangle, 3 noise)##sesd"].plurals[0] = "25xx: Форма волны ГНЧ 2 (0 пила, 1 меандр, 2 треуг., 3 шум)";
    strings["26xx: Set AM 2 depth (0 to 7F)##sesd"].plurals[0] = "26xx: Глубина АМ 2 (0-7F)";
    strings["27xx: Set PM 2 depth (0 to 7F)##sesd"].plurals[0] = "27xx: Глубина ФМ 2 (0-7F)";
    strings["28xy: Set reverb (x: operator from 1 to 4 (0 for all ops); y: reverb from 0 to 7)##sesd"].plurals[0] = "28xy: Реверб (x: оператор 1-4 (0 для всех операторов); y: реверб 0-7)";
    strings["2Axy: Set waveform (x: operator from 1 to 4 (0 for all ops); y: waveform from 0 to 7)##sesd0"].plurals[0] = "2Axy: Форма волны (x: оператор 1-4 (0 для всех операторов); y: форма волны 0-7)";
    strings["2Bxy: Set envelope generator shift (x: operator from 1 to 4 (0 for all ops); y: shift from 0 to 3)##sesd"].plurals[0] = "2Bxy: Сдвиг генератора огибающей (x: оператор 1-4 (0 для всех операторов); y: сдвиг 0-3)";
    strings["2Cxy: Set fine multiplier (x: operator from 1 to 4 (0 for all ops); y: fine)##sesd"].plurals[0] = "2Cxy: Точный множитель (x: оператор 1-4 (0 для всех операторов); y: множитель)";
    strings["3xyy: Set fixed frequency of operator 1 (x: octave from 0 to 7; y: frequency)##sesd"].plurals[0] = "3xyy: Установить фиксированную частоту оператора 1 (x: октава 0-7; y: частота)";
    strings["3xyy: Set fixed frequency of operator 2 (x: octave from 8 to F; y: frequency)##sesd"].plurals[0] = "3xyy: Установить фиксированную частоту оператора 2 (x: октава 8-F; y: частота)";
    strings["4xyy: Set fixed frequency of operator 3 (x: octave from 0 to 7; y: frequency)##sesd"].plurals[0] = "4xyy: Установить фиксированную частоту оператора 3 (x: октава 0-7; y: частота)";
    strings["4xyy: Set fixed frequency of operator 4 (x: octave from 8 to F; y: frequency)##sesd"].plurals[0] = "4xyy: Установить фиксированную частоту оператора 4 (x: октава 8-F; y: частота)";
    strings["10xy: Setup LFO (x: enable; y: speed)##sesd"].plurals[0] = "10xy: Настройка ГНЧ (x: включить; y: скорость)";
    strings["55xy: Set SSG envelope (x: operator from 1 to 4 (0 for all ops); y: 0-7 on, 8 off)##sesd"].plurals[0] = "55xy: Огибающая SSG (x: оператор 1-4 (0 для всех операторов); y: 0-7 вкл., 8 выкл.)";
    strings["1Fxx: Set ADPCM-A global volume (0 to 3F)##sesd"].plurals[0] = "1Fxx: Глобальная громкость ADPCM-A (0-3F)";
    strings["11xx: Set feedback (0 to 7)##sesd1"].plurals[0] = "11xx: Обратная связь (0-7)";
    strings["12xx: Set level of operator 1 (0 highest, 3F lowest)##sesd0"].plurals[0] = "12xx: Уровень оператора 1 (0 макс., 3F мин.)";
    strings["13xx: Set level of operator 2 (0 highest, 3F lowest)##sesd0"].plurals[0] = "13xx: Уровень оператора 2 (0 макс., 3F мин.)";
    strings["16xy: Set operator multiplier (x: operator from 1 to 2; y: multiplier)##sesd"].plurals[0] = "16xy: Множитель частоты (x: оператор 1-2; y: множитель)";
    strings["19xx: Set attack of all operators (0 to F)##sesd0"].plurals[0] = "19xx: Атака всех операторов (0-F)";
    strings["1Axx: Set attack of operator 1 (0 to F)##sesd0"].plurals[0] = "1Axx: Атака оператора 1 (0-F)";
    strings["1Bxx: Set attack of operator 2 (0 to F)##sesd0"].plurals[0] = "1Bxx: Атака оператора 2 (0-F)";
    strings["10xx: Set patch (0 to F)##sesd"].plurals[0] = "10xx: Патч (0-F)";
    strings["50xy: Set AM (x: operator from 1 to 2 (0 for all ops); y: AM)##sesd"].plurals[0] = "50xy: Включить АМ (x: оператор 1-2 (0 для всех операторов); y: АМ)";
    strings["51xy: Set sustain level (x: operator from 1 to 2 (0 for all ops); y: sustain)##sesd"].plurals[0] = "51xy: Уровень сустейна (x: оператор 1-2 (0 для всех операторов); y: сустейн)";
    strings["52xy: Set release (x: operator from 1 to 2 (0 for all ops); y: release)##sesd"].plurals[0] = "52xy: Релиз (x: оператор 1-2 (0 для всех операторов); y: релиз)";
    strings["53xy: Set vibrato (x: operator from 1 to 2 (0 for all ops); y: enabled)##sesd"].plurals[0] = "53xy: Вибрато (x: оператор 1-2 (0 для всех операторов); y: вкл.)";
    strings["54xy: Set envelope scale (x: operator from 1 to 2 (0 for all ops); y: scale from 0 to 3)##sesd"].plurals[0] = "54xy: Масш. огиб. (x: оператор 1-2 (0 для всех операторов); y: масш. 0-3)";
    strings["55xy: Set envelope sustain (x: operator from 1 to 2 (0 for all ops); y: enabled)##sesd"].plurals[0] = "55xy: Сустейн огибающей (x: оператор 1-2 (0 для всех операторов); y: вкл.)";
    strings["56xx: Set decay of all operators (0 to F)##sesd0"].plurals[0] = "56xx: Спад всех операторов (0-F)";
    strings["57xx: Set decay of operator 1 (0 to F)##sesd0"].plurals[0] = "57xx: Спад оператора 1 (0-F)";
    strings["58xx: Set decay of operator 2 (0 to F)##sesd0"].plurals[0] = "58xx: Спад оператора 2 (0-F)";
    strings["5Bxy: Set whether key will scale envelope (x: operator from 1 to 2 (0 for all ops); y: enabled)##sesd"].plurals[0] = "5Bxy: Масштаб. огиб. в зав. от ноты (x: оператор 1-2 (0 для всех операторов); y: вкл.)";
    strings["10xx: Set global AM depth (0: 1dB, 1: 4.8dB)##sesd"].plurals[0] = "10xx: Глобальная глубина АМ (0: 1 дБ, 1: 4.8 дБ)";
    strings["11xx: Set feedback (0 to 7)##sesd2"].plurals[0] = "11xx: Обратная связь (0-7)";
    strings["12xx: Set level of operator 1 (0 highest, 3F lowest)##sesd1"].plurals[0] = "12xx: Уровень оператора 1 (0 макс., 3F мин.)";
    strings["13xx: Set level of operator 2 (0 highest, 3F lowest)##sesd1"].plurals[0] = "13xx: Уровень оператора 2 (0 макс., 3F мин.)";
    strings["14xx: Set level of operator 3 (0 highest, 3F lowest)##sesd0"].plurals[0] = "14xx: Уровень оператора 3 (0 макс., 3F мин.)";
    strings["15xx: Set level of operator 4 (0 highest, 3F lowest)##sesd0"].plurals[0] = "15xx: Уровень оператора 4 (0 макс., 3F мин.)";
    strings["16xy: Set operator multiplier (x: operator from 1 to 4; y: multiplier)##sesd1"].plurals[0] = "16xy: Множитель частоты (x: оператор 1-4; y: множитель)";
    strings["17xx: Set global vibrato depth (0: normal, 1: double)##sesd"].plurals[0] = "17xx: Глобальная глубина вибрато (0: нормальная, 1: двойная)";
    strings["19xx: Set attack of all operators (0 to F)##sesd1"].plurals[0] = "19xx: Атака всех операторов (0-F)";
    strings["1Axx: Set attack of operator 1 (0 to F)##sesd1"].plurals[0] = "1Axx: Атака оператора 1 (0-F)";
    strings["1Bxx: Set attack of operator 2 (0 to F)##sesd1"].plurals[0] = "1Bxx: Атака оператора 2 (0-F)";
    strings["1Cxx: Set attack of operator 3 (0 to F)##sesd0"].plurals[0] = "1Cxx: Атака оператора 3 (0-F)";
    strings["1Dxx: Set attack of operator 4 (0 to F)##sesd0"].plurals[0] = "1Dxx: Атака оператора 4 (0-F)";
    strings["2Axy: Set waveform (x: operator from 1 to 4 (0 for all ops); y: waveform from 0 to 3 in OPL2 and 0 to 7 in OPL3)##sesd"].plurals[0] = "2Axy: Форма волны (x: оператор 1-4 (0 для всех операторов); y: форма волны 0-3 для OPL2 и 0-7 для OPL3)";
    strings["50xy: Set AM (x: operator from 1 to 4 (0 for all ops); y: AM)##sesd1"].plurals[0] = "50xy: Включить АМ (x: оператор 1-4 (0 для всех операторов); y: АМ)";
    strings["51xy: Set sustain level (x: operator from 1 to 4 (0 for all ops); y: sustain)##sesd1"].plurals[0] = "51xy: Уровень сустейна (x: оператор 1-4 (0 для всех операторов); y: сустейн)";
    strings["52xy: Set release (x: operator from 1 to 4 (0 for all ops); y: release)##sesd1"].plurals[0] = "52xy: Релиз (x: оператор 1-4 (0 для всех операторов); y: релиз)";
    strings["53xy: Set vibrato (x: operator from 1 to 4 (0 for all ops); y: enabled)##sesd0"].plurals[0] = "53xy: Вибрато (x: оператор 1-4 (0 для всех операторов); y: вкл.)";
    strings["54xy: Set envelope scale (x: operator from 1 to 4 (0 for all ops); y: scale from 0 to 3)##sesd1"].plurals[0] = "54xy: Масш. огиб. (x: оператор 1-4 (0 для всех операторов); y: масш. 0-3)";
    strings["55xy: Set envelope sustain (x: operator from 1 to 4 (0 for all ops); y: enabled)##sesd0"].plurals[0] = "55xy: Сустейн огибающей (x: оператор 1-4 (0 для всех операторов); y: вкл.)";
    strings["56xx: Set decay of all operators (0 to F)##sesd1"].plurals[0] = "56xx: Спад всех операторов (0-F)";
    strings["57xx: Set decay of operator 1 (0 to F)##sesd1"].plurals[0] = "57xx: Спад оператора 1 (0-F)";
    strings["58xx: Set decay of operator 2 (0 to F)##sesd1"].plurals[0] = "58xx: Спад оператора 2 (0-F)";
    strings["59xx: Set decay of operator 3 (0 to F)##sesd0"].plurals[0] = "59xx: Спад оператора 3 (0-F)";
    strings["5Axx: Set decay of operator 4 (0 to F)##sesd0"].plurals[0] = "5Axx: Спад оператора 4 (0-F)";
    strings["5Bxy: Set whether key will scale envelope (x: operator from 1 to 4 (0 for all ops); y: enabled)##sesd0"].plurals[0] = "5Bxy: Масштаб. огиб. в зав. от ноты (x: оператор 1-4 (0 для всех операторов); y: вкл.)";
    strings["10xx: Set waveform (bit 0: triangle; bit 1: saw; bit 2: pulse; bit 3: noise)##sesd"].plurals[0] = "10xx: Форма волны (бит 0: треуг.; бит 1: пила; бит 2: прямоуг.; бит 3: шум)";
    strings["11xx: Set coarse cutoff (not recommended; use 4xxx instead)##sesd"].plurals[0] = "11xx: Грубая частота среза (не рекомендуется; используйте 4xxx)";
    strings["12xx: Set coarse pulse width (not recommended; use 3xxx instead)##sesd"].plurals[0] = "12xx: Грубая скважность (не рекомендуется; используйте 3xxx)";
    strings["13xx: Set resonance (0 to F)##sesd"].plurals[0] = "13xx: Резонанс (0-F)";
    strings["14xx: Set filter mode (bit 0: low pass; bit 1: band pass; bit 2: high pass)##sesd"].plurals[0] = "14xx: Режим фильтра (бит 0: ФНЧ; бит 1: ППФ; бит 2: ФВЧ)";
    strings["15xx: Set envelope reset time##sesd"].plurals[0] = "15xx: Установить время ресета огибающей";
    strings["1Axx: Disable envelope reset for this channel (1 disables; 0 enables)##sesd"].plurals[0] = "1Axx: Отключить ресет огибающей для этого канала (1 выкл.; 0 вкл.)";
    strings["1Bxy: Reset cutoff (x: on new note; y: now)##sesd"].plurals[0] = "1Bxy: Сбросить частоту среза (x: на след. ноте; y: сейчас)";
    strings["1Cxy: Reset pulse width (x: on new note; y: now)##sesd"].plurals[0] = "1Cxy: Сбросить скважность (x: на след. ноте; y: сейчас)";
    strings["1Exy: Change other parameters (LEGACY)##sesd"].plurals[0] = "1Exy: Изменить другие параметры (СОВМЕСТИМОСТЬ)";
    strings["20xy: Set attack/decay (x: attack; y: decay)##sesd"].plurals[0] = "20xy: Установить атаку/спад (x: атака; y: спад)";
    strings["21xy: Set sustain/release (x: sustain; y: release)##sesd"].plurals[0] = "21xy: Установить сустейн/релиз (x: сустейн; y: релиз)";
    strings["22xx: Pulse width slide up##sesd"].plurals[0] = "22xx: Изменение скважности вверх";
    strings["23xx: Pulse width slide down##sesd"].plurals[0] = "23xx: Изменение скважности вниз";
    strings["24xx: Cutoff slide up##sesd"].plurals[0] = "24xx: Изменение частоты среза вверх";
    strings["25xx: Cutoff slide down##sesd"].plurals[0] = "25xx: Изменение частоты среза вниз";
    strings["3xxx: Set pulse width (0 to FFF)##sesd"].plurals[0] = "3xxx: Скважность (0-FFF)";
    strings["4xxx: Set cutoff (0 to 7FF)##sesd"].plurals[0] = "4xxx: Частота среза (0-7FF)";
    strings["10xx: Set waveform##sesd0"].plurals[0] = "10xx: Волна";
    strings["13xx: Set waveform (local)##sesd"].plurals[0] = "13xx: Волна (локальная)";
    strings["11xx: Set raw period (0-1F)##sesd"].plurals[0] = "11xx: \"Сырое\" значение периода (0-1F)";
    strings["11xx: Set waveform (local)##sesd"].plurals[0] = "11xx: Волна (локальная)";
    strings["20xx: Set PCM frequency##sesd"].plurals[0] = "20xx: Частота ИКМ";
    strings["10xy: Set AM depth (x: operator from 1 to 4 (0 for all ops); y: depth (0: 1dB, 1: 4.8dB))##sesd"].plurals[0] = "10xy: Глубина АМ (x: оператор 1-4 (0 для всех операторов); y: глубина (0: 1 дБ, 1: 4.8 дБ))";
    strings["12xx: Set level of operator 1 (0 highest, 3F lowest)##sesd2"].plurals[0] = "12xx: Уровень оператора 1 (0 макс., 3F мин.)";
    strings["13xx: Set level of operator 2 (0 highest, 3F lowest)##sesd2"].plurals[0] = "13xx: Уровень оператора 2 (0 макс., 3F мин.)";
    strings["14xx: Set level of operator 3 (0 highest, 3F lowest)##sesd1"].plurals[0] = "14xx: Уровень оператора 3 (0 макс., 3F мин.)";
    strings["15xx: Set level of operator 4 (0 highest, 3F lowest)##sesd1"].plurals[0] = "15xx: Уровень оператора 4 (0 макс., 3F мин.)";
    strings["16xy: Set operator multiplier (x: operator from 1 to 4; y: multiplier)##sesd2"].plurals[0] = "16xy: Множитель частоты (x: оператор 1-4; y: мнжитель)";
    strings["17xy: Set vibrato depth (x: operator from 1 to 4 (0 for all ops); y: depth (0: normal, 1: double))##sesd"].plurals[0] = "17xy: Глубина вибрато (x: оператор 1-4 (0 для всех операторов); y: глубина (0: нормальная, 1: удвоенная))";
    strings["19xx: Set attack of all operators (0 to F)##sesd"].plurals[0] = "19xx: Атака всех операторов (0-F)";
    strings["1Axx: Set attack of operator 1 (0 to F)##sesd2"].plurals[0] = "1Axx: Атака оператора 1 (0-F)";
    strings["1Bxx: Set attack of operator 2 (0 to F)##sesd2"].plurals[0] = "1Bxx: Атака оператора 1 (0-F)";
    strings["1Cxx: Set attack of operator 3 (0 to F)##sesd1"].plurals[0] = "1Cxx: Атака оператора 1 (0-F)";
    strings["1Dxx: Set attack of operator 4 (0 to F)##sesd1"].plurals[0] = "1Dxx: Атака оператора 1 (0-F)";
    strings["20xy: Set panning of operator 1 (x: left; y: right)##sesd"].plurals[0] = "20xy: Панорамирование оператора 1 (x: лево; y: право)";
    strings["21xy: Set panning of operator 2 (x: left; y: right)##sesd"].plurals[0] = "21xy: Панорамирование оператора 2 (x: лево; y: право)";
    strings["22xy: Set panning of operator 3 (x: left; y: right)##sesd"].plurals[0] = "22xy: Панорамирование оператора 3 (x: лево; y: право)";
    strings["23xy: Set panning of operator 4 (x: left; y: right)##sesd"].plurals[0] = "23xy: Панорамирование оператора 4 (x: лево; y: право)";
    strings["24xy: Set output level register (x: operator from 1 to 4 (0 for all ops); y: level from 0 to 7)##sesd"].plurals[0] = "24xy: Регистр выходного уровня (громкости) (x: оператор 1-4 (0 для всех операторов); y: уровень 0-7)";
    strings["25xy: Set modulation input level (x: operator from 1 to 4 (0 for all ops); y: level from 0 to 7)##sesd"].plurals[0] = "25xy: Уровень входной модуляции (x: оператор 1-4 (0 для всех операторов); y: уровень 0-7)";
    strings["26xy: Set envelope delay (x: operator from 1 to 4 (0 for all ops); y: delay from 0 to 7)##sesd"].plurals[0] = "26xy: Задержка огибающей (x: оператор 1-4 (0 для всех операторов); y: задержка 0-7)";
    strings["27xx: Set noise mode for operator 4 (x: mode from 0 to 3)##sesd"].plurals[0] = "27xx: Режим шума для оператора 4 (x: режим 0-3)";
    strings["2Axy: Set waveform (x: operator from 1 to 4 (0 for all ops); y: waveform from 0 to 7)##sesd1"].plurals[0] = "2Axy: Форма волны (x: оператор 1-4 (0 для всех операторов); y: форма волны 0-7)";
    strings["2Fxy: Set fixed frequency block (x: operator from 1 to 4; y: octave from 0 to 7)##sesd"].plurals[0] = "2Fxy: Блок в режиме фиксированной частоты (x: оператор 1-4; y: октава 0-7)";
    strings["40xx: Set detune of operator 1 (80: center)##sesd"].plurals[0] = "40xx: Расстройка оператора 1 (80: без расстройки)";
    strings["41xx: Set detune of operator 2 (80: center)##sesd"].plurals[0] = "41xx: Расстройка оператора 2 (80: без расстройки)";
    strings["42xx: Set detune of operator 3 (80: center)##sesd"].plurals[0] = "42xx: Расстройка оператора 3 (80: без расстройки)";
    strings["43xx: Set detune of operator 4 (80: center)##sesd"].plurals[0] = "43xx: Расстройка оператора 4 (80: без расстройки)";
    strings["50xy: Set AM (x: operator from 1 to 4 (0 for all ops); y: AM)##sesd2"].plurals[0] = "50xy: Включить АМ (x: оператор 1-4 (0 для всех операторов); y: АМ)";
    strings["51xy: Set sustain level (x: operator from 1 to 4 (0 for all ops); y: sustain)##sesd2"].plurals[0] = "51xy: Уровень сустейна (x: оператор 1-4 (0 для всех операторов); y: сустейн)";
    strings["52xy: Set release (x: operator from 1 to 4 (0 for all ops); y: release)##sesd2"].plurals[0] = "52xy: Релиз (x: оператор 1-4 (0 для всех операторов); y: релиз)";
    strings["53xy: Set vibrato (x: operator from 1 to 4 (0 for all ops); y: enabled)##sesd1"].plurals[0] = "53xy: Вибрато (x: оператор 1-4 (0 для всех операторов); y: вкл.)";
    strings["54xy: Set envelope scale (x: operator from 1 to 4 (0 for all ops); y: scale from 0 to 3)##sesd2"].plurals[0] = "54xy: Масш. огиб. (x: оператор 1-4 (0 для всех операторов); y: масш. 0-3)";
    strings["55xy: Set envelope sustain (x: operator from 1 to 4 (0 for all ops); y: enabled)##sesd1"].plurals[0] = "55xy: Сустейн огибающей (x: оператор 1-4 (0 для всех операторов); y: вкл.)";
    strings["56xx: Set decay of all operators (0 to F)##sesd2"].plurals[0] = "56xx: Спад всех операторов (0-F)";
    strings["57xx: Set decay of operator 1 (0 to F)##sesd2"].plurals[0] = "57xx: Спад оператора 1 (0-F)";
    strings["58xx: Set decay of operator 2 (0 to F)##sesd2"].plurals[0] = "58xx: Спад оператора 2 (0-F)";
    strings["59xx: Set decay of operator 3 (0 to F)##sesd1"].plurals[0] = "59xx: Спад оператора 3 (0-F)";
    strings["5Axx: Set decay of operator 4 (0 to F)##sesd1"].plurals[0] = "5Axx: Спад оператора 4 (0-F)";
    strings["5Bxy: Set whether key will scale envelope (x: operator from 1 to 4 (0 for all ops); y: enabled)##sesd1"].plurals[0] = "5Bxy: Масштаб. огиб. в зав. от ноты (x: оператор 1-4 (0 для всех операторов); y: вкл.)";
    strings["3xyy: Set fixed frequency F-num of operator 1 (x: high 2 bits from 0 to 3; y: low 8 bits of F-num)##sesd"].plurals[0] = "3xyy: Частота в режиме фиксированной частоты для оператора 1 (x: два старших бита 0-3; y: 8 младших битов частоты)";
    strings["3xyy: Set fixed frequency F-num of operator 2 (x: high 2 bits from 4 to 7; y: low 8 bits of F-num)##sesd"].plurals[0] = "3xyy: Частота в режиме фиксированной частоты для оператора 2 (x: два старших бита 4-7; y: 8 младших битов частоты)";
    strings["3xyy: Set fixed frequency F-num of operator 3 (x: high 2 bits from 8 to B; y: low 8 bits of F-num)##sesd"].plurals[0] = "3xyy: Частота в режиме фиксированной частоты для оператора 3 (x: два старших бита 8-B; y: 8 младших битов частоты)";
    strings["3xyy: Set fixed frequency F-num of operator 4 (x: high 2 bits from C to F; y: low 8 bits of F-num)##sesd"].plurals[0] = "3xyy: Частота в режиме фиксированной частоты для оператора 4 (x: два старших бита C-F; y: 8 младших битов частоты)";
    strings["10xx: Set waveform (bit 0: triangle; bit 1: saw; bit 2: pulse; bit 3: noise)##sesd1"].plurals[0] = "10xx: Форма волны (бит 0: треуг.; бит 1: пила; бит 2: прямоуг.; бит 3: шум)";
    strings["11xx: Set resonance (0 to FF)##sesd"].plurals[0] = "11xx: Резонанс (0-FF)";
    strings["12xx: Set filter mode (bit 0: low pass; bit 1: band pass; bit 2: high pass)##sesd"].plurals[0] = "12xx: Режим фильтра (бит 0: ФНЧ; бит 1: ППФ; бит 2: ФВЧ)";
    strings["13xx: Disable envelope reset for this channel (1 disables; 0 enables)##sesd"].plurals[0] = "13xx: Отключить ресет огибающей для этого канала (1 выкл.; 0 вкл.)";
    strings["14xy: Reset cutoff (x: on new note; y: now)##sesd"].plurals[0] = "14xy: Сбросить частоту среза (x: на след. ноте; y: сейчас)";
    strings["15xy: Reset pulse width (x: on new note; y: now)##sesd"].plurals[0] = "15xy: Сбросить скважность (x: на след. ноте; y: сейчас)";
    strings["16xy: Change other parameters##sesd"].plurals[0] = "16xy: Изменить другие параметры";
    strings["17xx: Pulse width slide up##sesd"].plurals[0] = "17xx: Изменение скважности вверх";
    strings["18xx: Pulse width slide down##sesd"].plurals[0] = "18xx: Изменение скважности вниз";
    strings["19xx: Cutoff slide up##sesd"].plurals[0] = "19xx: Изменение частоты среза вверх";
    strings["1Axx: Cutoff slide down##sesd"].plurals[0] = "1Axx: Изменение частоты среза вниз";
    strings["3xxx: Set pulse width (0 to FFF)##sesd1"].plurals[0] = "3xxx: Скважность (0-FFF)";
    strings["4xxx: Set cutoff (0 to FFF)##sesd1"].plurals[0] = "4xxx: Частота среза (0-FFF)";
    strings["a chip which found its way inside mobile phones in the 2000's.\nas proprietary as it is, it passed away after losing to MP3 in the mobile hardware battle.##sesd"].plurals[0] = "чип, начавший появляться в мобильных телефонах в 2000-ые.\nнесмотря на проприетарность, он проиграл формату MP3 во время соперничества разных видов мобильного железа.";
    strings["<COMPOUND SYSTEM!>##sesd0"].plurals[0] = "<СОСТАВНАЯ СИСТЕМА!>";
    strings["<COMPOUND SYSTEM!>##sesd1"].plurals[0] = "<СОСТАВНАЯ СИСТЕМА!>";
    strings["a square/noise sound chip found on the Sega Master System, ColecoVision, Tandy, TI's own 99/4A and a few other places.##sesd"].plurals[0] = "чип с квадратными волнами и шумом, который был установлен в Sega Master System, ColecoVision, Tandy, собственном устройстве TI 99/4A и некоторых других местах.";
    strings["20xy: Set noise mode (x: preset freq/ch3 freq; y: thin pulse/noise)##sesd"].plurals[0] = "20xy: Режим шума (x: фикс. част./част. 3-го кан.; y: \"тонкая\" прямоуг. аолна/шум)";
    strings["<COMPOUND SYSTEM!>##sesd2"].plurals[0] = "<СОСТАВНАЯ СИСТЕМА!>";
    strings["the most popular portable game console of the era.##sesd"].plurals[0] = "самая популярная портативная игровая консоль той эпохи.";
    strings["10xx: Set waveform##sesd1"].plurals[0] = "10xx: Волна";
    strings["11xx: Set noise length (0: long; 1: short)##sesd"].plurals[0] = "11xx: Длина шума (0: длинный; 1: короткий)";
    strings["12xx: Set duty cycle (0 to 3)##sesd"].plurals[0] = "12xx: Скважность (0-3)";
    strings["13xy: Setup sweep (x: time; y: shift)##sesd"].plurals[0] = "13xy: Сконфигурировать аппаратное портаменто (x: длительность; y: расстояние)";
    strings["14xx: Set sweep direction (0: up; 1: down)##sesd"].plurals[0] = "14xx: Направление аппаратного портаменто (0: вверх; 1: вниз)";
    strings["15xx: Set waveform (local)##sesd"].plurals[0] = "15xx: Волна (локальная)";
    strings["an '80s game console with a wavetable sound chip, popular in Japan.##sesd"].plurals[0] = "игровая консоль из 80-ых с чипом на волновых таблицах. Была популярна в Японии.";
    strings["10xx: Set waveform##sesd2"].plurals[0] = "10xx: Волна";
    strings["11xx: Toggle noise mode##sesd0"].plurals[0] = "11xx: Переключить режим шума";
    strings["12xx: Setup LFO (0: disabled; 1: 1x depth; 2: 16x depth; 3: 256x depth)##sesd"].plurals[0] = "12xx: Настроить ГНЧ (0: выкл.; 1: глубина 1x; 2: глубина 16x; 3: глубина 256x)";
    strings["13xx: Set LFO speed##sesd"].plurals[0] = "13xx: Частота ГНЧ";
    strings["17xx: Toggle PCM mode (LEGACY)##sesd1"].plurals[0] = "17xx: Переключить режим ИКМ (СОВМЕСТИМОСТЬ)";
    strings["18xx: Set waveform (local)##sesd"].plurals[0] = "18xx: Волна (локальная)";
    strings["also known as Famicom in Japan, it's the most well-known game console of the '80s.##sesd"].plurals[0] = "также известная в Японии как Famicom. Самая известная игровая консоль 80-ых.";
    strings["11xx: Write to delta modulation counter (0 to 7F)##sesd"].plurals[0] = "11xx: Запись в регистр дельта-счётчика (0-7F)";
    strings["12xx: Set duty cycle/noise mode (pulse: 0 to 3; noise: 0 or 1)##sesd0"].plurals[0] = "12xx: Скважность/режим шума (меандр: 0-3; шум: 0 или 1)";
    strings["13xy: Sweep up (x: time; y: shift)##sesd"].plurals[0] = "13xy: Аппаратное портаменто вверх (x: время; y: сдвиг)";
    strings["14xy: Sweep down (x: time; y: shift)##sesd"].plurals[0] = "14xy: Аппаратное портаменто вниз (x: время; y: сдвиг)";
    strings["15xx: Set envelope mode (0: envelope, 1: length, 2: looping, 3: constant)##sesd"].plurals[0] = "15xx: Режим огибающей (0: огибающая, 1: длина, 2: цикл, 3: постоянная)";
    strings["16xx: Set length counter (refer to manual for a list of values)##sesd"].plurals[0] = "16xx: Счётчик длины (список значений см. в инструкции)";
    strings["17xx: Set frame counter mode (0: 4-step, 1: 5-step)##sesd"].plurals[0] = "17xx: Режим счётчика кадров (0: 4 шага, 1: 5 шагов)";
    strings["18xx: Select PCM/DPCM mode (0: PCM; 1: DPCM)##sesd"].plurals[0] = "18xx: Выбрать режим ИКМ/ДИКМ (0: ИКМ; 1: ДИКМ)";
    strings["19xx: Set triangle linear counter (0 to 7F; 80 and higher halt)##sesd"].plurals[0] = "19xx: Линейный счётчик треуг. волны (0-7F; 80 и выше мгновенно останавливают волну)";
    strings["20xx: Set DPCM frequency (0 to F)##sesd"].plurals[0] = "20xx: Частота ДИКМ (0-F)";
    strings["<COMPOUND SYSTEM!>##sesd3"].plurals[0] = "<СОСТАВНАЯ СИСТЕМА!>";
    strings["<COMPOUND SYSTEM!>##sesd4"].plurals[0] = "<СОСТАВНАЯ СИСТЕМА!>";
    strings["this computer is powered by the SID chip, which had synthesizer features like a filter and ADSR.##sesd"].plurals[0] = "в этом компьютере стоит чип SID, который имеет продвинутый функционал, присущий синтезаторам, например, фильтр и ADSR-огибающую.";
    strings["this computer is powered by the SID chip, which had synthesizer features like a filter and ADSR.\nthis is the newer revision of the chip.##sesd"].plurals[0] = "в этом компьютере стоит чип SID, который имеет продвинутый функционал, присущий синтезаторам, например, фильтр и ADSR-огибающую.\nЭто более новая ревизия чипа.";
    strings["<COMPOUND SYSTEM!>##sesd5"].plurals[0] = "<СОСТАВНАЯ СИСТЕМА!>";
    strings["like Neo Geo, but lacking the ADPCM-B channel since they couldn't connect the pins.##sesd"].plurals[0] = "как Neo Geo, но без канала АДИКМ-B, потому что они не смогли подключить пины.";
    strings["like Neo Geo, but lacking the ADPCM-B channel since they couldn't connect the pins.\nthis one is in Extended Channel mode, which turns the second FM channel into four operators with independent notes/frequencies.##sesd"].plurals[0] = "как Neo Geo, но без канала АДИКМ-B, потому что они не смогли подключить пины.\nЭто версия в режиме расширенного канала, который превращает второй FM-канал в четыре оператора с независимыми нотами/частотами.";
    strings["this chip is everywhere! ZX Spectrum, MSX, Amstrad CPC, Intellivision, Vectrex...\nthe discovery of envelope bass helped it beat the SN76489 with ease.##sesd"].plurals[0] = "этот чип везде! ZX Spectrum, MSX, Amstrad CPC, Intellivision, Vectrex...\nОткрытие метода использования огибающей для баса легко позволило этому чипу победить SN76489.";
    strings["a computer from the '80s with full sampling capabilities, giving it a sound ahead of its time.##sesd"].plurals[0] = "компьютер из 80-ых с полноценными возможностями сэмплирования, что давало ему звучание, опережавшее его эпоху.";
    strings["10xx: Toggle filter (0 disables; 1 enables)##sesd"].plurals[0] = "10xx: Переключить фильтр (0 выкл.; 1 вкл.)";
    strings["11xx: Toggle AM with next channel##sesd"].plurals[0] = "11xx: Переключить АМ со следующим каналом";
    strings["12xx: Toggle period modulation with next channel##sesd"].plurals[0] = "12xx: Переключить модуляцию периода со следующим каналом";
    strings["13xx: Set waveform##sesd"].plurals[0] = "13xx: Волна";
    strings["14xx: Set waveform (local)##sesd"].plurals[0] = "14xx: Волна (локлаьная)";
    strings["this was Yamaha's first integrated FM chip.\nit was used in several synthesizers, computers and arcade boards.##sesd"].plurals[0] = "этот чип был первой интегральной микросхемой Yamaha.\nОн использовался в нескольких синтезаторах, компьютерах и аркадных автоматах.";
    strings["this chip is mostly known for being in the Sega Genesis (but it also was on the FM Towns computer).##sesd"].plurals[0] = "этот чип в основном известен по причине того, что он находился в Sega Genesis (но он также использовался в компьютере FM Towns).";
    strings["it's a challenge to make music on this chip which barely has musical capabilities...##sesd"].plurals[0] = "на этом чипе очень сложно писать музыку, ведь он едва ли обладает музыкальными возможностями...";
    strings["supposedly an upgrade from the AY-3-8910, this was present on the Creative Music System (Game Blaster) and SAM Coupé.##sesd"].plurals[0] = "этот чип, судя по всему, улучшенная версия AY-3-8910, использовался в Creative Music System (Game Blaster) и SAM Coupé.";
    strings["10xy: Set channel mode (x: noise; y: tone)##sesd"].plurals[0] = "10xy: Режим канала (x: шум; y: тон)";
    strings["11xx: Set noise frequency##sesd"].plurals[0] = "11xx: Частота шума";
    strings["12xx: Setup envelope (refer to docs for more information)##sesd"].plurals[0] = "12xx: Настройка огибающей (см. инструкцию)";
    strings["an improved version of the AY-3-8910 with a bigger frequency range, duty cycles, configurable noise and per-channel envelopes!##sesd"].plurals[0] = "улучшенная версия AY-3-8910 с большим диапазоном частот, настройкой скважности, настраиваемым шумом и огибающей на каждый канал!";
    strings["Commodore's successor to the PET.\nits square wave channels are more than just square...##sesd"].plurals[0] = "Компьютер Commodore, вышедший после PET.\nего каналы квадратных волн могут играть не только лишь квадратные волны...";
    strings["one channel of 1-bit wavetable which is better (and worse) than the PC Speaker.##sesd"].plurals[0] = "один канал 1-битной волновой таблицы, что лучше (или хуже), чем PC Speaker (пищалка).";
    strings["FM? nah... samples! Nintendo's answer to Sega.##sesd"].plurals[0] = "FM? не... сэмплы! Ответ Nintendo в сторону Sega.";
    strings["18xx: Enable echo buffer##sesd"].plurals[0] = "18xx: Включить эхо-буфер";
    strings["19xx: Set echo delay (0 to F)##sesd"].plurals[0] = "19xx: Задержка эхо (0-F)";
    strings["1Axx: Set left echo volume##sesd"].plurals[0] = "1Axx: Громкость эхо на левом канале";
    strings["1Bxx: Set right echo volume##sesd"].plurals[0] = "1Bxx: Громкость эхо на правом канале";
    strings["1Cxx: Set echo feedback##sesd"].plurals[0] = "1Cxx: Обратная связь эхо";
    strings["1Exx: Set dry output volume (left)##sesd"].plurals[0] = "1Exx: Громкость канала (лево)";
    strings["1Fxx: Set dry output volume (right)##sesd"].plurals[0] = "1Fxx: Громкость канала (право)";
    strings["30xx: Set echo filter coefficient 0##sesd"].plurals[0] = "30xx: Коэффициент 0 фильтра эхо";
    strings["31xx: Set echo filter coefficient 1##sesd"].plurals[0] = "31xx: Коэффициент 1 фильтра эхо";
    strings["32xx: Set echo filter coefficient 2##sesd"].plurals[0] = "32xx: Коэффициент 2 фильтра эхо";
    strings["33xx: Set echo filter coefficient 3##sesd"].plurals[0] = "33xx: Коэффициент 3 фильтра эхо";
    strings["34xx: Set echo filter coefficient 4##sesd"].plurals[0] = "34xx: Коэффициент 4 фильтра эхо";
    strings["35xx: Set echo filter coefficient 5##sesd"].plurals[0] = "35xx: Коэффициент 5 фильтра эхо";
    strings["36xx: Set echo filter coefficient 6##sesd"].plurals[0] = "36xx: Коэффициент 6 фильтра эхо";
    strings["37xx: Set echo filter coefficient 7##sesd"].plurals[0] = "37xx: Коэффициент 7 фильтра эхо";
    strings["10xx: Set waveform##sesd3"].plurals[0] = "10xx: Волна";
    strings["11xx: Toggle noise mode##sesd1"].plurals[0] = "11xx: Переключить режим шума";
    strings["12xx: Toggle echo on this channel##sesd"].plurals[0] = "12xx: Переключить эхо наэтом канале";
    strings["13xx: Toggle pitch modulation##sesd"].plurals[0] = "13xx: Переключить частотную модуляцию";
    strings["14xy: Toggle invert (x: left; y: right)##sesd"].plurals[0] = "14xy: Переключить инвертирование сигнала (x: левый; y: правый)";
    strings["15xx: Set envelope mode (0: ADSR, 1: gain/direct, 2: dec, 3: exp, 4: inc, 5: bent)##sesd"].plurals[0] = "15xx: Режим огибающей (0: ADSR, 1: усиление/прямой, 2: спад, 3: экспоненциальная, 4: нарастание, 5: изогн.)";
    strings["16xx: Set gain (00 to 7F if direct; 00 to 1F otherwise)##sesd"].plurals[0] = "16xx: Усиление (00-7F в прямом режиме; иначе 00-1F)";
    strings["17xx: Set waveform (local)##sesd"].plurals[0] = "17xx: Волна (локальная)";
    strings["1Dxx: Set noise frequency (00 to 1F)##sesd"].plurals[0] = "1Dxx: Частота шума (00-1F)";
    strings["20xx: Set attack (0 to F)##sesd"].plurals[0] = "20xx: Атака (0-F)";
    strings["21xx: Set decay (0 to 7)##sesd"].plurals[0] = "21xx: Спад (0-7)";
    strings["22xx: Set sustain (0 to 7)##sesd"].plurals[0] = "22xx: Сустейн (0-7)";
    strings["23xx: Set release (00 to 1F)##sesd"].plurals[0] = "23xx: Релиз (00-1F)";
    strings["an expansion chip for the Famicom, featuring a quirky sawtooth channel.##sesd"].plurals[0] = "чип расширения для Famicom, содержащий своеобразный канал пилообразной волны.";
    strings["12xx: Set duty cycle (pulse: 0 to 7)##sesd"].plurals[0] = "12xx: Скважность (меандр: 0-7)";
    strings["17xx: Toggle PCM mode (LEGACY)##sesd2"].plurals[0] = "17xx: Переключить режим ИКМ (СОВМЕСТИМОСТЬ)";
    strings["cost-reduced version of the OPL with 16 patches and only one of them is user-configurable.##sesd"].plurals[0] = "удешевлённая версия OPL с 16-ю патчами, причём только один из них доступен для настройки пользователем.";
    strings["a disk drive for the Famicom which also contains one wavetable channel.##sesd"].plurals[0] = "дисковый привод для Famicom, также содержащий один канал волновых таблиц.";
    strings["10xx: Set waveform##sesd4"].plurals[0] = "10xx: Волна";
    strings["11xx: Set modulation depth##sesd"].plurals[0] = "11xx: Глубина модуляции";
    strings["12xy: Set modulation speed high byte (x: enable; y: value)##sesd"].plurals[0] = "12xy: Старший байт скорости модуляции (x: вкл.; y: значение)";
    strings["13xx: Set modulation speed low byte##sesd"].plurals[0] = "13xx: Младший байт скорости модуляции";
    strings["14xx: Set modulator position##sesd"].plurals[0] = "14xx: Положение модулятора";
    strings["15xx: Set modulator table to waveform##sesd"].plurals[0] = "15xx: Назначить таблицу модуляции волны";
    strings["16xx: Set waveform (local)##sesd"].plurals[0] = "16xx: Волна (локальная)";
    strings["an expansion chip for the Famicom, featuring a little-known PCM channel.##sesd"].plurals[0] = "чип расширения для Famicom, имеющий малоизвестный канал ИКМ-сэмплов.";
    strings["12xx: Set duty cycle/noise mode (pulse: 0 to 3; noise: 0 or 1)##sesd1"].plurals[0] = "12xx: Скважность/режим шума (меандр: 0-3; шум: 0 или 1)";
    strings["an expansion chip for the Famicom, with full wavetable.##sesd"].plurals[0] = "чип расширения для Famicom, полностью на волновых таблицах.";
    strings["18xx: Change channel limits (0 to 7, x + 1)##sesd"].plurals[0] = "18xx: Изменить лимит каналов (0-7, x + 1)";
    strings["20xx: Load a waveform into memory##sesd"].plurals[0] = "20xx: Загрузить волну в память";
    strings["21xx: Set position for wave load##sesd"].plurals[0] = "21xx: Задать начальное смещение для загрузки волны";
    strings["10xx: Select waveform##sesd"].plurals[0] = "10xx: Выбрать волну";
    strings["11xx: Set waveform position in RAM##sesd"].plurals[0] = "11xx: Задать положение волны в ОЗУ";
    strings["12xx: Set waveform length in RAM (04 to FC in steps of 4)##sesd"].plurals[0] = "12xx: Задать длину волны в ОЗУ (04-FC с шагом 4)";
    strings["15xx: Set waveform load position##sesd"].plurals[0] = "15xx: Задать положение для загрузки волны";
    strings["16xx: Set waveform load length (04 to FC in steps of 4)##sesd"].plurals[0] = "16xx: Задать длину загружаемой волны (04-FC с шагом 4)";
    strings["17xx: Select waveform (local)##sesd1"].plurals[0] = "17xx: Волна (локальная)";
    strings["cost-reduced version of the OPM with a different register layout and no stereo...\n...but it has a built-in AY-3-8910! (actually an YM2149)##sesd"].plurals[0] = "удешевлённая версия OPM с другим расположением регистров и отсутствием стерео...\n...но у неё внутри AY-3-8910! (на самом деле YM2149)";
    strings["cost-reduced version of the OPM with a different register layout and no stereo...\n...but it has a built-in AY-3-8910! (actually an YM2149)\nthis one is in Extended Channel mode, which turns the third FM channel into four operators with independent notes/frequencies##sesd"].plurals[0] = "удешевлённая версия OPM с другим расположением регистров и отсутствием стерео...\n...но у неё внутри AY-3-8910! (на самом деле YM2149)\nЭто версия в режиме расширенного канала, который превращает третий FM-канал в четыре оператора с независимыми нотами/частотами.";
    strings["cost-reduced version of the OPM with a different register layout and no stereo...\n...but it has a built-in AY-3-8910! (actually an YM2149)\nthis one is in Extended Channel mode, which turns the third FM channel into four operators with independent notes/frequencies.\nthis one includes CSM mode control for special effects on Channel 3.##sesd"].plurals[0] = "удешевлённая версия OPM с другим расположением регистров и отсутствием стерео...\n...но у неё внутри AY-3-8910! (на самом деле YM2149)\nЭто версия в режиме расширенного канала, который превращает третий FM-канал в четыре оператора с независимыми нотами/частотами.\nУ этой версии есть контроль режима CSM для специальных эффектов на третьем канале.";
    strings["OPN but twice the FM channels, stereo makes a come-back and has rhythm and ADPCM channels.##sesd"].plurals[0] = "OPN, но вдвое больше FM-каналов, возвращённое стерео, ритм- и АДИКМ каналы.";
    strings["OPN but twice the FM channels, stereo makes a come-back and has rhythm and ADPCM channels.\nthis one is in Extended Channel mode, which turns the third FM channel into four operators with independent notes/frequencies##sesd"].plurals[0] = "OPN, но вдвое больше FM-каналов, возвращённое стерео, ритм- и АДИКМ каналы.\nЭто версия в режиме расширенного канала, который превращает третий FM-канал в четыре оператора с независимыми нотами/частотами.";
    strings["OPN but twice the FM channels, stereo makes a come-back and has rhythm and ADPCM channels.\nthis one is in Extended Channel mode, which turns the third FM channel into four operators with independent notes/frequencies.\nthis one includes CSM mode control for special effects on Channel 3.##sesd"].plurals[0] = "OPN, но вдвое больше FM-каналов, возвращённое стерео, ритм- и АДИКМ каналы.\nЭто версия в режиме расширенного канала, который превращает третий FM-канал в четыре оператора с независимыми нотами/частотами.\nУ этой версии есть контроль режима CSM для специальных эффектов на третьем канале.";
    strings["OPN, but what if you only had two operators, no stereo, no detune and a lower ADSR parameter range?##sesd"].plurals[0] = "OPN, но что если у него всего два оператора, нет стерео, нет расстройки и меньший диапазон регулировки параметров ADSR?";
    strings["OPL, but what if you had more waveforms to choose than the normal sine?##sesd"].plurals[0] = "OPL, но что если у него есть ещё волны, помимо синусоиды?";
    strings["OPL2, but what if you had twice the channels, 4-op mode, stereo and even more waveforms?##sesd"].plurals[0] = "OPL2, но что если у него вдвое больше каналов, 4-оп режим, стерео и ещё больше волн?";
    strings["how many channels of PCM do you want?\nMultiPCM: yes##sesd"].plurals[0] = "Сколько каналов ИКМ-сэмплов вам нужно?\nMultiPCM: да";
    strings["good luck! you get one square and no volume control.##sesd"].plurals[0] = "удачи! у вас один канал меандра и нет регулировки громкости.";
    strings["please don't use this chip. it was added as a joke.##sesd"].plurals[0] = "пожалуйста, не используйте этот чип. он был добавлен в качестве шутки.";
    strings["TIA, but better and more flexible.\nused in the Atari 8-bit family of computers (400/800/XL/XE).##sesd"].plurals[0] = "TIA, но лучше и более гибкий.\nиспользовался в семействе 8-битных компьютеров Atari (400/800/XL/XE).";
    strings["10xx: Set waveform (0 to 7)##sesd0"].plurals[0] = "10xx: Волна (0-7)";
    strings["11xx: Set AUDCTL##sesd"].plurals[0] = "11xx: Установить AUDCTL";
    strings["12xx: Toggle two-tone mode##sesd"].plurals[0] = "12xx: Переключить двухголосый режим";
    strings["13xx: Set raw period##sesd"].plurals[0] = "13xx: \"Сырое\" значение периода";
    strings["14xx: Set raw period (higher byte; only for 16-bit mode)##sesd"].plurals[0] = "14xx: \"Сырое\" значение периода (старший байт; только для 16-битного режима)";
    strings["this is like SNES' sound chip but without interpolation and the rest of nice bits.##sesd"].plurals[0] = "похоже на звуковой чип SNES, но без интерполяции и других приятных вещей.";
    strings["developed by the makers of the Game Boy and the Virtual Boy...##sesd"].plurals[0] = "от создателей Game Boy и Virtual Boy...";
    strings["10xx: Set waveform##sesd5"].plurals[0] = "10xx: Волна";
    strings["11xx: Setup noise mode (0: disabled; 1-8: enabled/tap)##sesd"].plurals[0] = "11xx: Настроить режим шума (0: выкл.; 1-8: вкл./отвод)";
    strings["12xx: Setup sweep period (0: disabled; 1-20: enabled/period)##sesd"].plurals[0] = "12xx: Настроить период аппаратного портаменто (0: выкл.; 1-20: вкл./период)";
    strings["13xx: Set sweep amount##sesd"].plurals[0] = "13xx: Скорость аппаратного портаменто";
    strings["17xx: Toggle PCM mode (LEGACY)##sesd3"].plurals[0] = "17xx: Переключить режим ИКМ (СОВМЕСТИМОСТЬ)";
    strings["18xx: Set waveform (local)##sesd1"].plurals[0] = "18xx: Волна (локальная)";
    strings["like OPM, but with more waveforms, fixed frequency mode and totally... undocumented.\nused in the Yamaha TX81Z and some other synthesizers.##sesd"].plurals[0] = "как OPM, но с большим количеством волн, режимом фиксированной частоты, и полным... отсутствием сведений о его устройстве.\nиспользовался в Yamaha TX81Z и некоторых других синтезаторах.";
    strings["2Fxx: Toggle hard envelope reset on new notes##sesd"].plurals[0] = "2Fxx: Переключить жёсткий перезапуск огибающей на новой ноте";
    strings["this one is like PC Speaker but has duty cycles.##sesd"].plurals[0] = "Похож на PC Speaker, но имеет настройку скважности.";
    strings["used in some Sega arcade boards (like OutRun), and usually paired with a YM2151.##sesd"].plurals[0] = "использовался в некоторых аркадных автоматах Sega (например, OutRun), и обычно использовался в связке с YM2151.";
    strings["a console which failed to sell well due to its headache-inducing features.##sesd"].plurals[0] = "консоль, продажи которой не удались из-за того, что её особенности вызывали головную боль у игроков.";
    strings["10xx: Set waveform##sesd6"].plurals[0] = "10xx: Волна";
    strings["11xx: Set noise length (0 to 7)##sesd"].plurals[0] = "11xx: Длина шума (0-7)";
    strings["12xy: Setup envelope (x: enabled/loop (1: enable, 3: enable+loop); y: speed/direction (0-7: down, 8-F: up))##sesd"].plurals[0] = "12xy: Настройка огибающей (x: вкл./цикл (1: вкл., 3: вкл.+цикл); y: скорость/направление (0-7: вниз, 8-F: вверх))";
    strings["13xy: Setup sweep (x: speed; y: shift; channel 5 only)##sesd"].plurals[0] = "13xy: Настройка аппаратного портаменто (x: скорость; y: количество; только для 5-го канала)";
    strings["14xy: Setup modulation (x: enabled/loop (1: enable, 3: enable+loop); y: speed; channel 5 only)##sesd"].plurals[0] = "14xy: Настроить модуляцию (x: вкл./цикл (1: вкл., 3: вкл.+цикл); y: скорость; только для 5-го канала)";
    strings["15xx: Set modulation waveform (x: wavetable; channel 5 only)##sesd"].plurals[0] = "15xx: Настройка волны модулятора (x: волновая таблица; только для 5-го канала)";
    strings["16xx: Set waveform (local)##sesd1"].plurals[0] = "16xx: Волна (локальная)";
    strings["like OPLL, but even more cost reductions applied. three less FM channels, and no drums mode...##sesd"].plurals[0] = "как OPLL, но ещё более удешевлённый. на три канала меньше, нет режима ударных...";
    strings["so Taito asked Yamaha if they could get the two missing FM channels back, and Yamaha gladly provided them with this chip.##sesd"].plurals[0] = "Taito попросили Yamaha вернуть два отсутствующих FM-канала, и Yamaha с радостью предоставила им этот чип.";
    strings["the ZX Spectrum only had a basic beeper capable of...\n...a bunch of thin pulses and tons of other interesting stuff!\nFurnace provides a thin pulse system.##sesd"].plurals[0] = "ZX Spectrum имел только пищалку, способную на...\n...несколько \"тонких\" прямоугольных волн и множество других интересных вещей!\nFurnace предоставляет систему с \"тонкими\" прямоугольными волнами.";
    strings["12xx: Set pulse width##sesd0"].plurals[0] = "12xx: Скважность";
    strings["17xx: Trigger overlay drum##sesd"].plurals[0] = "17xx: Запуск перекрывающего ударного инструмента";
    strings["this chip is mostly known for being in the Sega Genesis (but it also was on the FM Towns computer).\nthis one is in Extended Channel mode, which turns the third FM channel into four operators with independent notes/frequencies.##sesd"].plurals[0] = "этот чип в основном известен по причине того, что он находился в Sega Genesis (но он также использовался в компьютере FM Towns).\nЭто версия в режиме расширенного канала, который превращает третий FM-канал в четыре оператора с независимыми нотами/частотами.";
    strings["this chip is mostly known for being in the Sega Genesis (but it also was on the FM Towns computer).\nthis one includes CSM mode control for special effects on Channel 3.##sesd"].plurals[0] = "этот чип в основном известен по причине того, что он находился в Sega Genesis (но он также использовался в компьютере FM Towns).\nУ этой версии есть контроль режима CSM для специальных эффектов на третьем канале.";
    strings["a wavetable chip made by Konami for use with the MSX.\nthe last channel shares its wavetable with the previous one though.##sesd"].plurals[0] = "чип с волновыми таблицами, сделанный Konami для использования с MSX.\nправда, последний и предпоследний каналы используют одну и ту же таблицу.";
    strings["the OPL chip but with drums mode enabled.##sesd"].plurals[0] = "чип OPL, но с включённым режимом ударных.";
    strings["the OPL2 chip but with drums mode enabled.##sesd"].plurals[0] = "чип OPL2, но с включённым режимом ударных.";
    strings["the OPL3 chip but with drums mode enabled.##sesd"].plurals[0] = "чип OPL3, но с включённым режимом ударных.";
    strings["this chip was used in SNK's Neo Geo arcade board and video game console.\nit's like OPNA but the rhythm channels are ADPCM channels and two FM channels went missing.##sesd"].plurals[0] = "этот чип использовался в аркадном автомате и игровой консоли Neo Geo от SNK.\nпохож на OPNA, но ритм-каналы теперь стали АДИКМ-каналами, и пропали два FM-канала.";
    strings["this chip was used in SNK's Neo Geo arcade board and video game console.\nit's like OPNA but the rhythm channels are ADPCM channels and two FM channels went missing.\nthis one is in Extended Channel mode, which turns the second FM channel into four operators with independent notes/frequencies.##sesd"].plurals[0] = "этот чип использовался в аркадном автомате и игровой консоли Neo Geo от SNK.\nпохож на OPNA, но ритм-каналы теперь стали АДИКМ-каналами, и пропали два FM-канала.\nЭто версия в режиме расширенного канала, который превращает второй FM-канал в четыре оператора с независимыми нотами/частотами.";
    strings["this chip was used in SNK's Neo Geo arcade board and video game console.\nit's like OPNA but the rhythm channels are ADPCM channels and two FM channels went missing.\nthis one is in Extended Channel mode, which turns the second FM channel into four operators with independent notes/frequencies.\nthis one includes CSM mode control for special effects on Channel 2.##sesd"].plurals[0] = "этот чип использовался в аркадном автомате и игровой консоли Neo Geo от SNK.\nпохож на OPNA, но ритм-каналы теперь стали АДИКМ-каналами, и пропали два FM-канала.\nЭто версия в режиме расширенного канала, который превращает второй FM-канал в четыре оператора с независимыми нотами/частотами.\nУ этой версии есть контроль режима CSM для специальных эффектов на втором канале.";
    strings["the OPLL chip but with drums mode turned on.##sesd"].plurals[0] = "чип OPLL с включённым режимом ударных.";
    strings["3xxx: Load LFSR (0 to FFF)##sesd"].plurals[0] = "3xxx: Загрузить в РСЛОС (0-FFF)";
    strings["a portable console made by Atari. it has all of Atari's trademark waveforms.##sesd"].plurals[0] = "портативная игровая консоль от Atari. у неё есть все фирменные волны Atari.";
    strings["10xx: Set echo feedback level (00 to FF)##sesd"].plurals[0] = "10xx: Уровень обратной связи эхо (00-FF)";
    strings["11xx: Set channel echo level (00 to FF)##sesd"].plurals[0] = "11xx: Уровень эхо на канале (00-FF)";
    strings["12xx: Toggle QSound algorithm (0: disabled; 1: enabled)##sesd"].plurals[0] = "12xx: Переключить алгоритм QSound (0: выкл.; 1: вкл.)";
    strings["3xxx: Set echo delay buffer length (000 to AA5)##sesd"].plurals[0] = "3xxx: Длина буфера задержки эхо (000-AA5)";
    strings["used in some of Capcom's arcade boards. surround-like sampled sound with echo.##sesd"].plurals[0] = "использовался в некоторых аркадных автоматах Capcom. сэмпилрованный звук с эхо и эффектом окружающего звучания.";
    strings["the chip used in a computer design created by The 8-Bit Guy.##sesd"].plurals[0] = "чип, используемый в модели компьютера, разработанного 8-Bit Guy.";
    strings["20xx: Set waveform##sesd"].plurals[0] = "20xx: Волна";
    strings["22xx: Set duty cycle (0 to 3F)##sesd"].plurals[0] = "22xx: Скважность (0-3F)";
    strings["so Taito asked Yamaha if they could get the two missing FM channels back, and Yamaha gladly provided them with this chip.\nthis one is in Extended Channel mode, which turns the third FM channel into four operators with independent notes/frequencies.##sesd"].plurals[0] = "Taito попросили Yamaha вернуть два отсутствующих FM-канала, и Yamaha с радостью предоставила им этот чип.\nЭто версия в режиме расширенного канала, который превращает третий FM-канал в четыре оператора с независимыми нотами/частотами.";
    strings["so Taito asked Yamaha if they could get the two missing FM channels back, and Yamaha gladly provided them with this chip.\nthis one is in Extended Channel mode, which turns the third FM channel into four operators with independent notes/frequencies.\nthis one includes CSM mode control for special effects on Channel 3.##sesd"].plurals[0] = "Taito попросили Yamaha вернуть два отсутствующих FM-канала, и Yamaha с радостью предоставила им этот чип.\nЭто версия в режиме расширенного канала, который превращает третий FM-канал в четыре оператора с независимыми нотами/частотами.\nУ этой версии есть контроль режима CSM для специальных эффектов на третьем канале.";
    strings["this is the same thing as SegaPCM, but only exposes 5 of the channels for compatibility with DefleMask.##sesd"].plurals[0] = "это то же самое, что и SegaPCM, но доступно всего пять каналов для совместимости с DefleMask.";
    strings["a sound chip used in several Seta/Allumer-manufactured arcade boards with too many channels of wavetable sound, which also are capable of sampled sound.##sesd"].plurals[0] = "звуковой чип, использовавшийся в нескольких аркадных автоматах Seta/Allumer. имеет слишком много каналов волновых таблиц, способных так же воспроизводить сэмплы.";
    strings["10xx: Set waveform##sesd7"].plurals[0] = "10xx: Волна";
    strings["11xx: Set envelope shape##sesd"].plurals[0] = "11xx: Форма огибающей";
    strings["12xx: Set sample bank slot (0 to 7)##sesd"].plurals[0] = "12xx: Слот банка сэмплов (0-7)";
    strings["17xx: Toggle PCM mode (LEGACY)##sesd4"].plurals[0] = "17xx: Переключить режим ИКМ (СОВМЕСТИМОСТЬ)";
    strings["18xx: Set waveform (local)##sesd2"].plurals[0] = "18xx: Волна (локальная)";
    strings["20xx: Set PCM frequency (1 to FF)##sesd"].plurals[0] = "20xx: Частота ИКМ (1-FF)";
    strings["22xx: Set envelope mode (bit 0: enable; bit 1: one-shot; bit 2: split shape to L/R; bit 3/5: H.invert right/left; bit 4/6: V.invert right/left)##sesd"].plurals[0] = "22xx: Режим огибающей (бит 0: вкл.; бит 1: однокр.; бит 2: разделить форму на лево/право; биты 3/5: Г.инвертир. право/лево; бит 4/6: В.инвертир. право/лево)";
    strings["23xx: Set envelope period##sesd"].plurals[0] = "23xx: Период огибающей";
    strings["25xx: Envelope slide up##sesd1"].plurals[0] = "25xx: Портаменто огибающей вверх";
    strings["26xx: Envelope slide down##sesd1"].plurals[0] = "26xx: Портаменто огибающей вниз";
    strings["29xy: Set auto-envelope (x: numerator; y: denominator)##sesd1"].plurals[0] = "29xy: Авто-огибающая (x: числитель; y: знаменатель)";
    strings["this is the wavetable part of the Bubble System, which also had two AY-3-8910s.##sesd"].plurals[0] = "это таблично-волновая часть Bubble System, которая так же имела два AY-3-8910.";
    strings["like OPL3, but this time it also has a 24-channel version of MultiPCM.##sesd"].plurals[0] = "как OPL3, но на этот раз также имеется 24-канальная версия MultiPCM.";
    strings["the OPL4 but with drums mode turned on.##sesd"].plurals[0] = "OPL4 с включёным режимом ударных.";
    strings["11xx: Set filter mode (00 to 03)##sesd"].plurals[0] = "11xx: Режим фильтра (00-03)";
    strings["14xx: Set filter coefficient K1 low byte (00 to FF)##sesd"].plurals[0] = "14xx: Младший байт коэффициента фильтра K1 (00-FF)";
    strings["15xx: Set filter coefficient K1 high byte (00 to FF)##sesd"].plurals[0] = "15xx: Старыший байт коэффициента фильтра K1 (00-FF)";
    strings["16xx: Set filter coefficient K2 low byte (00 to FF)##sesd"].plurals[0] = "16xx: Младший байт коэффициента фильтра K2 (00-FF)";
    strings["17xx: Set filter coefficient K2 high byte (00 to FF)##sesd"].plurals[0] = "17xx: Старыший байт коэффициента фильтра K2 (00-FF)";
    strings["18xx: Set filter coefficient K1 slide up (00 to FF)##sesd"].plurals[0] = "18xx: Нарастание коэффициента фильтра K1 (00-FF)";
    strings["19xx: Set filter coefficient K1 slide down (00 to FF)##sesd"].plurals[0] = "19xx: Уменьшение коэффициента фильтра K1 (00-FF)";
    strings["1Axx: Set filter coefficient K2 slide up (00 to FF)##sesd"].plurals[0] = "1Axx: Нарастание коэффициента фильтра K2 (00-FF)";
    strings["1Bxx: Set filter coefficient K2 slide down (00 to FF)##sesd"].plurals[0] = "1Bxx: Уменьшение коэффициента фильтра K2 (00 to FF)";
    strings["22xx: Set envelope left volume ramp (signed) (00 to FF)##sesd"].plurals[0] = "22xx: Нарастание огибающей громкости левого канала (знаковое) (00-FF)";
    strings["23xx: Set envelope right volume ramp (signed) (00 to FF)##sesd"].plurals[0] = "23xx: Нарастание огибающей громкости правого канала (знаковое) (00-FF)";
    strings["24xx: Set envelope filter coefficient k1 ramp (signed) (00 to FF)##sesd"].plurals[0] = "24xx: Нарастание огибающей коэффициента фильтра K1 (знаковое) (00-FF)";
    strings["25xx: Set envelope filter coefficient k1 ramp (signed, slower) (00 to FF)##sesd"].plurals[0] = "25xx: Нарастание огибающей коэффициента фильтра K1 (знаковое, медленнее) (00-FF)";
    strings["26xx: Set envelope filter coefficient k2 ramp (signed) (00 to FF)##sesd"].plurals[0] = "26xx: Нарастание огибающей коэффициента фильтра K2 (знаковое) (00-FF)";
    strings["27xx: Set envelope filter coefficient k2 ramp (signed, slower) (00 to FF)##sesd"].plurals[0] = "27xx: 25xx: Нарастание огибающей коэффициента фильтра K2 (знаковое, медленнее) (00-FF)";
    strings["DFxx: Set sample playback direction (0: normal; 1: reverse)##sesd1"].plurals[0] = "DFxx: Направление проигрывания сэмпла (0: обычное; 1: обратное)";
    strings["120x: Set pause (bit 0)##sesd"].plurals[0] = "120x: Пауза (бит 0)";
    strings["2xxx: Set envelope count (000 to 1FF)##sesd"].plurals[0] = "2xxx: Отсчёт огибающей (000-1FF)";
    strings["3xxx: Set filter coefficient K1 (000 to FFF)##sesd"].plurals[0] = "3xxx: Коэффициент фильтра K1 (000-FFF)";
    strings["4xxx: Set filter coefficient K2 (000 to FFF)##sesd"].plurals[0] = "4xxx: Коэффициент фильтра K2 (000-FFF)";
    strings["a sample chip made by Ensoniq, which is the basis for the GF1 chip found in Gravis' Ultrasound cards.##sesd"].plurals[0] = "сэмплерный чип от Ensoniq, на основе которго была сделан чип GF1, применявшийся в звуковых картах Gravis Ultrasound.";
    strings["like OPL but with an ADPCM channel.##sesd"].plurals[0] = "как OPL, но с каналом АДИКМ-сэмплов.";
    strings["the Y8950 chip, in drums mode.##sesd"].plurals[0] = "Чип Y8950 в режиме ударных.";
    strings["this is a variant of Konami's SCC chip with the last channel's wavetable being independent.##sesd"].plurals[0] = "вариант чипа Konami SCC с независимой волновой таблицей последнего канала.";
    strings["10xx: Set waveform (0 to 7)##sesd1"].plurals[0] = "10xx: Волна (0-7)";
    strings["12xx: Set pulse width (0 to 7F)##sesd"].plurals[0] = "12xx: Скважность (0-7F)";
    strings["13xx: Set resonance (0 to FF)##sesd"].plurals[0] = "13xx: Резонанс (0-FF)";
    strings["14xx: Set filter mode (bit 0: ring mod; bit 1: low pass; bit 2: high pass; bit 3: band pass)##sesd"].plurals[0] = "14xx: Режим фильтра (бит 0: кольцевая модуляция; бит 1: ФНЧ; бит 2: ФВЧ; бит 3: ППФ)";
    strings["15xx: Set frequency sweep period low byte##sesd"].plurals[0] = "15xx: Младший байт аппаратного портаменто";
    strings["16xx: Set frequency sweep period high byte##sesd"].plurals[0] = "16xx: Старший байт аппаратного портаменто";
    strings["17xx: Set volume sweep period low byte##sesd"].plurals[0] = "17xx: Младший байт аппаратного изменения громкости";
    strings["18xx: Set volume sweep period high byte##sesd"].plurals[0] = "18xx: Старший байт аппаратного изменения громкости";
    strings["19xx: Set cutoff sweep period low byte##sesd"].plurals[0] = "19xx: Младший байт аппаратного изменения частоты среза";
    strings["1Axx: Set cutoff sweep period high byte##sesd"].plurals[0] = "1Axx: Старший байт аппаратного изменения частоты среза";
    strings["1Bxx: Set frequency sweep boundary##sesd"].plurals[0] = "1Bxx: Граница аппаратного портаменто";
    strings["1Cxx: Set volume sweep boundary##sesd"].plurals[0] = "1Cxx: Граница аппаратного изменения громкости";
    strings["1Dxx: Set cutoff sweep boundary##sesd"].plurals[0] = "1Dxx: Граница аппаратного изменения частоты среза";
    strings["1Exx: Set phase reset period low byte##sesd"].plurals[0] = "1Exx: Младший байт периода сброса фазы";
    strings["1Fxx: Set phase reset period high byte##sesd"].plurals[0] = "1Fxx: Старший байт периода сброса фазы";
    strings["20xx: Toggle frequency sweep (bit 0-6: speed; bit 7: direction is up)##sesd"].plurals[0] = "20xx: Переключить аппаратное портаменто (биты 0-6: скорость; бит 7: направление вверх)";
    strings["21xx: Toggle volume sweep (bit 0-4: speed; bit 5: direction is up; bit 6: loop; bit 7: alternate)##sesd"].plurals[0] = "21xx: Переключить аппаратное изменение громкости (биты 0-4: скорость; бит 5: направление вверх; бит 6: цикл; бит 7: вверх-вниз)";
    strings["22xx: Toggle cutoff sweep (bit 0-6: speed; bit 7: direction is up)##sesd"].plurals[0] = "22xx: Переключить аппаратное изменение частоты среза фильтра (бит 0-6: скорость; бит 7: направление вверх)";
    strings["4xxx: Set cutoff (0 to FFF)##sesd"].plurals[0] = "4xxx: Частота среза (0-FFF)";
    strings["tildearrow's fantasy sound chip. put SID, AY and VERA in a blender, and you get this!##sesd"].plurals[0] = "вымышленный звуковой чип tildearrow. загрузите в блендер SID, AY и VERA, и вы получите этот чип!";
    strings["an ADPCM sound chip manufactured by OKI and used in many arcade boards.##sesd"].plurals[0] = "АДИКМ звуковой чип фирмы OKI. Использовался во многих аркадных автоматах.";
    strings["20xx: Set chip output rate (0: clock/132; 1: clock/165)##sesd"].plurals[0] = "20xx: Частота дискретизации выходного сигнала (0: тактовая частота/132; 1: тактовая частота/165)";
    strings["an ADPCM sound chip manufactured by OKI and used in the Sharp X68000.##sesd"].plurals[0] = "АДИКМ звуковой чип фирмы OKI, использовавшийся в Sharp X68000.";
    strings["20xx: Set frequency divider (0-2)##sesd"].plurals[0] = "20xx: Делитель частоты (0-2)";
    strings["21xx: Select clock rate (0: full; 1: half)##sesd"].plurals[0] = "21xx: Тактовая частота (0: полная; 1: половинная)";
    strings["used in some arcade boards. Can play back either 4-bit ADPCM, 8-bit PCM or 16-bit PCM.##sesd"].plurals[0] = "использовался в некоторых аркадных автоматах. Может воспроизводить 4-битные АДИКМ, 8- и 16-битные ИКМ сэмплы.";
    strings["10xx: Set waveform##sesd8"].plurals[0] = "10xx: Волна";
    strings["11xx: Select waveform (local)##sesd1"].plurals[0] = "11xx: Волна (локальная)";
    strings["10xx: Set waveform##sesd9"].plurals[0] = "10xx: Волна";
    strings["11xx: Toggle noise mode##sesd2"].plurals[0] = "11xx: Переключить режим шума";
    strings["a wavetable sound chip used in Pac-Man, among other early Namco arcade games.##sesd"].plurals[0] = "таблично-волновой чип, использовавшийся в игровом автомате Pac-Man и других ранних автоматах Namco.";
    strings["successor of the original Namco WSG chip, used in later Namco arcade games.##sesd"].plurals[0] = "следующая модель после Namco WSG, использовался в поздних игровых автоматах Namco.";
    strings["like Namco C15 but with stereo sound.##sesd"].plurals[0] = "похож на Namco C15, но с поддержкой стерео.";
    strings["a square wave additive synthesis chip made by OKI. used in some arcade machines and instruments.##sesd"].plurals[0] = "чип с меандрами и аддитивным синтезом от фирмы OKI. использовался в некоторых игровых автоматах и инструментах.";
    strings["10xy: Set group control (x: sustain; y: part toggle bitmask)##sesd"].plurals[0] = "10xy: Контроль группы (x: сустейн; y: битовая маска частей)";
    strings["11xx: Set noise mode##sesd0"].plurals[0] = "11xx: Режим шума";
    strings["12xx: Set group attack (0 to 5)##sesd"].plurals[0] = "12xx: Атака группы (0-5)";
    strings["13xx: Set group decay (0 to 11)##sesd"].plurals[0] = "13xx: Спад группы (0-11)";
    strings["this chip is mostly known for being in the Sega Genesis (but it also was on the FM Towns computer).\nthis system uses software mixing to provide two sample channels.##sesd"].plurals[0] = "этот чип в основном известен по причине того, что он находился в Sega Genesis (но он также использовался в компьютере FM Towns).\nэта версия использует программное смешение сигналов, за счёт чего возможно использование двух каналов сэмплов.";
    strings["this chip is mostly known for being in the Sega Genesis (but it also was on the FM Towns computer).\nthis system uses software mixing to provide two sample channels.\nthis one is in Extended Channel mode, which turns the third FM channel into four operators with independent notes/frequencies.\nthis one includes CSM mode control for special effects on Channel 3.##sesd"].plurals[0] = "этот чип в основном известен по причине того, что он находился в Sega Genesis (но он также использовался в компьютере FM Towns).\nэта версия использует программное смешение сигналов, за счёт чего возможно использование двух каналов сэмплов.\nЭто версия в режиме расширенного канала, который превращает третий FM-канал в четыре оператора с независимыми нотами/частотами.\nУ этой версии есть контроль режима CSM для специальных эффектов на третьем канале.";
    strings["an SN76489 derivative used in Neo Geo Pocket, has independent stereo volume and noise channel frequency.##sesd"].plurals[0] = "вариант SN76489, использовавшийся в Neo Geo Pocket. Имеет независимую регулировку громкости стереоканалов и частоты канала шума.";
    strings["20xx: Set noise length (0: short, 1: long)##sesd"].plurals[0] = "20xx: Длина шума (0: короткий, 1: длинный)";
    strings["as generic sample playback as it gets.##sesd"].plurals[0] = "настолько типичное воспроизведение сэмплов, насколько это возможно.";
    strings["this PCM chip was widely used at Konami arcade boards in 1986-1990.##sesd"].plurals[0] = "этот ИКМ чип широко использовался в игровых автоматах Konami с 1986 по 1990 год.";
    strings["yet another PCM chip from Irem. like Amiga, but less pitch resolution and no sample loop.##sesd"].plurals[0] = "ещё один ИКМ чип от Irem. как Amiga, но с меньшим разрешением регулировки частоты и без зацикливания сэмплов.";
    strings["a SoC with wavetable sound hardware.##sesd"].plurals[0] = "СнК с таблично-волновым синтезатором звука.";
    strings["a game console with 3 channels of square wave. it's what happens after fusing TIA and VIC together.##sesd"].plurals[0] = "игровая консоль с тремя каналами квадратных волн. вот что получается, если скрестить TIA и VIC.";
    strings["10xx: Set ring modulation (0: disable, 1: enable)##sesd"].plurals[0] = "10xx: Кольцевая модуляция (0: выкл., 1: вкл.)";
    strings["11xx: Raw frequency (0-3E)##sesd"].plurals[0] = "11xx: Сырая частота (0-3E)";
    strings["another ZX Spectrum beeper system with full PWM pulses and 3-level volume per channel. it also has a pitchable overlay sample channel.##sesd"].plurals[0] = "ещё одна система с пищалкой ZX Spectrum, на этот раз содержащая полноценные прямоугольные волны с регулировкой скважности и тремя уровнями громкости для каждого канала. Также имеется перекрывающий канал сэмплов с регулировкой частоты.";
    strings["12xx: Set pulse width##sesd1"].plurals[0] = "12xx: Скважность";
    strings["this PCM chip was widely used at Konami arcade boards in 1990-1992.##sesd"].plurals[0] = "этот ИКМ чип широко использовался в игровых автоматах Konami с 1990 по 1992.";
    strings["DFxx: Set sample playback direction (0: normal; 1: reverse)##sesd2"].plurals[0] = "DFxx: Направление проигрывания сэмпла (0: обычное; 1: обратное)";
    strings["two square waves (one may be turned into noise). used in the Commodore Plus/4, 16 and 116.##sesd"].plurals[0] = "два канала квадратных волн (один из них может играть шум). использовался в Commodore Plus/4, 16 и 116.";
    strings["Namco's first PCM chip from 1987. it's pretty good for being so.##sesd"].plurals[0] = "первый ИКМ чип Namco из 1987 года. для своего времени он весьма неплох.";
    strings["Namco's PCM chip used in their NA-1/2 hardware.\nvery similar to C140, but has noise generator.##sesd"].plurals[0] = "ИКМ чип Namco, использовавшийся в их устройствах NA-1/2.\nочень похож на C140, но имеет генератор шума.";
    strings["11xx: Set noise mode##sesd1"].plurals[0] = "11xx: Режим шума";
    strings["12xy: Set invert mode (x: surround; y: invert)##sesd"].plurals[0] = "12xy: Режим инвертирования (x: окруж. звук; y: инверт.)";
    strings["a unique FM synth featured in PC sound cards.\nbased on the OPL3 design, but with lots of its features extended.##sesd"].plurals[0] = "уникальный чип с FM-синтезом, применявшийся в звуковых картах для PC.\nоснован на OPL3, но с сильно расширенным функционалом.";
    strings["2Exx: Toggle hard envelope reset on new notes##sesd"].plurals[0] = "2Exx: Переключить жёсткий перезапуск огибающей на новой ноте";
    strings["first Ensoniq chip used in their synths and Apple IIGS computer. Has 32 hard-panned 8-bit wavetable/sample channels, can do oscillator sync (like SID) and amplitude modulation. Can have up to 128 KiB (2 banks of 64 KiB) of memory for wavetables/samples.\nAs Robert Yannes (SID chip creator) said, it's more or less what SID chip could be if he was given enough time for its development.##sesd"].plurals[0] = "первый чип Ensoniq, использовавшийся в их синтезаторах и в компьютере Apple IIGS. Имеет 32 канала волновых таблиц/сэмплов с жёстким панорамированием, имеется синхронизация осцилляторов (как SID) и амплитудная модуляция. Может иметь до 128 КиБ (2 банка по 64 КиБ) памяти для волновых таблиц/сэмплов.\nКак сказал Robert Yannes (создатель чипа SID), это более-менее то, чем мог быть чип SID, если бы ему дали достаточно времени для его разработки.";
    strings["11xx: Set number of enabled oscillators (2-1F)##sesd"].plurals[0] = "11xx: Количество включённых осцилляторов (2-1F)";
    strings["12xx: Set oscillator output (0-7, 0=left, 1=right)##sesd"].plurals[0] = "12xx: Вывод сигнала осциллятора (0-7, 0=лево, 1=право)";
    strings["13xx: Set wave/sample length (0-7, 0=256, 1=512, 2=1024, etc.)##sesd"].plurals[0] = "13xx: Длина волны/сэмпла (0-7, 0=256, 1=512, 2=1024 и т.д.)";
    strings["14xx: Set wave/sample position in memory (xx*256 offset)##sesd"].plurals[0] = "14xx: Начальное смещение волны/сэмпла в памяти (смещение xx*256)";
    strings["15xx: Set oscillator mode (0-3)##sesd"].plurals[0] = "15xx: Режим осциллятора (0-3)";
    strings["a fantasy sound chip designed by jvsTSX and The Beesh-Spweesh!\nused in the Hexheld fantasy console.##sesd"].plurals[0] = "вымышленный звуковой чип, разработанный jvsTSX и The Beesh-Spweesh!\nиспользуется в вымышленной игровой консоли Hexheld.";
    strings["20xx: Load low byte of noise channel LFSR (00 to FF) or slope channel accumulator (00 to 7F)##sesd"].plurals[0] = "20xx: Загрузить младший байт в РСЛОС канала шума (00-FF) или в аккумулятор канала ската (00-7F)";
    strings["21xx: Load high byte of noise channel LFSR (00 to FF)##sesd"].plurals[0] = "21xx: Загрузить старший байт в РСЛОС канала шума (00-FF)";
    strings["22xx: Write to I/O port A##sesd"].plurals[0] = "22xx: Записать в порт ввода-вывода A";
    strings["23xx: Write to I/O port B##sesd"].plurals[0] = "23xx: Записать в порт ввода-вывода B";
    strings["this chip was featured in the Enterprise 128 computer. it is similar to POKEY, but with stereo output, more features and frequency precision and the ability to turn left or right (or both) channel into a 6-bit DAC for sample playback.##sesd"].plurals[0] = "этот чип стоял в компьютере Enterprise 128. он похож на POKEY, но имеет стерео звук, больше функций, более точную настройку частоты и возможность превращать левый или правый (или оба) канал в 6-битный ЦАП для проигрывания сэмплов.";
    strings["10xx: Set waveform (0 to 4; 0 to 3 on noise)##sesd"].plurals[0] = "10xx: Волна (0-4; 0-3 на канале шума)";
    strings["11xx: Set noise frequency source (0: fixed; 1-3: channels 1 to 3)##sesd"].plurals[0] = "11xx: Источник частоты шума (0: фикс.; 1-3: каналы 1-3)";
    strings["12xx: Toggle high-pass with next channel##sesd"].plurals[0] = "12xx: Переключить ФВЧ от следующего канала";
    strings["13xx: Toggle ring modulation with channel+2##sesd"].plurals[0] = "13xx: Переключить кольцевую модуляцию от канал+2";
    strings["14xx: Toggle swap counters (noise only)##sesd"].plurals[0] = "14xx: Переключить смену счётчиков (только шум)";
    strings["15xx: Toggle low pass (noise only)##sesd"].plurals[0] = "15xx: Переключить ФНЧ (только шум)";
    strings["16xx: Set clock divider (0: /2; 1: /3)##sesd"].plurals[0] = "16xx: Делитель частоты (0: /2; 1: /3)";
    strings["a fictional sound chip by LTVA. like SID, but with many of its problems fixed. also features extended functionality like more wave mixing modes, tonal noise, filter and volume per channel.##sesd"].plurals[0] = "вымышленный звуковой чип за авторством LTVA. похож на SID, но в нём исправлены многие его проблемы. также в этом чипе имеется дополнительный функционал, например, новые способы смешения волн, тональный зацикленный шум, отдельные регулировка громкости и фильтр на каждом канале.";
    strings["a fictional sound chip by Euly. similar to Ricoh 2A03, but all the duty cycles are different, noise has 32 pitches instead of 16 and you have four hard-coded waveforms on triangle channel.##sesd"].plurals[0] = "вымышленный звуковой чип за авторством Euly. почти как Ricoh 2A03, но все четыре скважности разные, у шума 32 частоты вместо 16, а на канале треугольной волны можно играть четыре предопределённые волны.";
    strings["12xx: Set duty cycle/noise mode (pulse: 0 to 3; noise: 0 or 1, wave: 0 to 3)##sesd"].plurals[0] = "12xx: Скважность/режим шума (меандр: 0-3; шум: 0 или 1; волна: 0-3)";
    strings["19xx: Set wave linear counter (0 to 7F; 80 and higher halt)##sesd"].plurals[0] = "19xx: Линейный счётчик канала волны (0-7F; 80 и выше мгновенно останавливают волну)";
    strings["additional PCM FIFO channels in Game Boy Advance driven directly by its DMA hardware.##sesd"].plurals[0] = "дополнительные ИКМ FIFO-каналы в Game Boy Advance с сигналом напрямую от аппаратного контроллера DMA.";
    strings["additional PCM FIFO channels in Game Boy Advance driven by software mixing to provide up to sixteen sample channels.##sesd"].plurals[0] = "дополнительные ИКМ FIFO-каналы в Game Boy Advance, управляемые при помощи программного микширования, что даёт до 16 каналов сэмплов.";
    strings["11xy: Set echo channel (x: left/right source; y: delay (0 disables))##sesd"].plurals[0] = "11xy: Канал эхо (x: левый/правый источник; y: задержка (0 отключает))";
    strings["12xy: Toggle invert (x: left; y: right)##sesd"].plurals[0] = "12xy: Инверсия сигнала (x: лево; y: право)";
    strings["a handheld video game console with two screens. it uses a stylus.##sesd"].plurals[0] = "портативная игровая консоль с двумя экранами. необходим стилус.";
    strings["12xx: Set duty cycle (pulse: 0 to 7)##sesd"].plurals[0] = "12xx: Скважность (прямоуг.: 0-7)";
    strings["1Fxx: Set global volume (0 to 7F)##sesd"].plurals[0] = "1Fxx: Глобальная громкость (0-7F)";
    strings["FZT sound source##sesd"].plurals[0] = "Источник звука FZT";
    strings["a software synth core by LTVA used in Flizzer Tracker (Flipper Zero chiptune tracker).##sesd"].plurals[0] = "ядро программного синтеза за авторством LTVA, используемое в программе Flizzer Tracker (чиптюн-трекер для Flipper Zero).";
    strings["10xx: Set wave (bits: 0: noise, 1: pulse, 2: triangle, 3: sawtooth, 4: metallic noise, 5: sine)##sesd"].plurals[0] = "10xx: Волна (биты: 0: шум, 1: прямоуг., 2: треуг., 3: пила, 4: \"металлический\" шум, 5: синус)";
    strings["11xy: PWM (pulsolo) with speed x and depth y##sesd"].plurals[0] = "11xy: ШИМ (пульсоло) со скоростью x и глубиной y";
    strings["12xx: Set pulse width##sesd"].plurals[0] = "12xx: Скважность";
    strings["13xx: Pulse width up##sesd"].plurals[0] = "13xx: Изменение скважности вверх";
    strings["14xx: Pulse width down##sesd"].plurals[0] = "14xx: Изменение скважности вниз";
    strings["15xx: Set filter cutoff##sesd"].plurals[0] = "15xx: Частота среза фильтра";
    strings["16xx: Set volume##sesd"].plurals[0] = "16xx: Громкость";
    strings["17xx: Toggle filter##sesd"].plurals[0] = "17xx: Вкл./выкл. фильтр";
    strings["18xx: Set filter mode##sesd"].plurals[0] = "18xx: Режим фильтра";
    strings["19xx: Phase reset##sesd"].plurals[0] = "19xx: Сброс фазы";
    strings["1Axx: Filter cutoff up##sesd"].plurals[0] = "1Axx: Частота среза вверх";
    strings["1Bxx: Filter cutoff down##sesd"].plurals[0] = "1Bxx: Частота среза вниз";
    strings["1Cxx: Set filter resonance##sesd"].plurals[0] = "1Cxx: Резонанс фильтра";
    strings["1Dxx: Filter resonance up##sesd"].plurals[0] = "1Dxx: Резонанс вверх";
    strings["1Exx: Filter resonance down##sesd"].plurals[0] = "1Exx: Резонанс вниз";
    strings["1Fxx: Ring mod source (FF = self)##sesd"].plurals[0] = "1Fxx: Источник кольцевой модуляции (FF = самомодуляция)";
    strings["20xx: Hard sync source (FF = self)##sesd"].plurals[0] = "20xx: Источник синхронизации осцилляторов (FF = самосинхронизация)";
    strings["21xx: Set attack speed##sesd"].plurals[0] = "21xx: Скорость атаки";
    strings["22xx: Set decay speed##sesd"].plurals[0] = "22xx: Скорость спада";
    strings["23xx: Set sustain level##sesd"].plurals[0] = "23xx: Уровень сустейна";
    strings["24xx: Set release rate##sesd"].plurals[0] = "24xx: Скорость релиза";
    strings["25xx: Restart instrument program##sesd"].plurals[0] = "25xx: Перезапустить программу инструмента";
    strings["26xx: Portamento up (semitones)##sesd"].plurals[0] = "26xx: Портаменто вверх (полутонов)";
    strings["27xx: Portamento down (semitones)##sesd"].plurals[0] = "27xx: Портаменто вниз (полутонов)";
    strings["28xx: Absolute arpeggio note##sesd"].plurals[0] = "28xx: Абсолютная нота арпеджио";
    strings["29xx: Trigger envelope release##sesd"].plurals[0] = "29xx: Релиз огибающей";
    strings["a fantasy sound chip using logistic map iterations to generate sound.##sesd"].plurals[0] = "вымышленный звуковой чип, использующий итерации логистического отображения для генерации звука.";
    strings["10xx: Load low byte of channel sample state##sesd"].plurals[0] = "10xx: Загрузить младший байт сэмплового состояния канала";
    strings["11xx: Load high byte of channel sample state##sesd"].plurals[0] = "11xx: Загрузить старший байт сэмплового состояния канала";
    strings["12xx: Set low byte of channel parameter##sesd"].plurals[0] = "12xx: Младший байт параметра канала";
    strings["13xx: Set high byte of channel parameter##sesd"].plurals[0] = "13xx: Старший байт параметра канала";
    strings["this is a system designed for testing purposes.##sesd"].plurals[0] = "это система, разработанная для тестирования.";

    strings["help! what's going on!"].plurals[0] = "помогите! что происходит!";
    strings["Sunsoft 5B standalone##sesd"].plurals[0] = "Sunsoft 5B отдельно";
    strings["Sunsoft 5B standalone (PAL)##sesd"].plurals[0] = "Sunsoft 5B отдельно (PAL)";
    strings["Overclocked Sunsoft 5B##sesd"].plurals[0] = "Разогнанный Sunsoft 5B";
    strings["Sega Genesis Extended Channel 3##sesd0"].plurals[0] = "Sega Genesis расширенный 3-ий канал";
    strings["NTSC-J Sega Master System + drums##sesd"].plurals[0] = "NTSC-J Sega Master System + ударные";
    strings["MSX-MUSIC + drums##sesd"].plurals[0] = "MSX-MUSIC + ударные";
    strings["Commodore 64 with dual 6581##sesd"].plurals[0] = "Commodore 64 с двумя 6581";
    strings["Commodore 64 with dual 8580##sesd"].plurals[0] = "Commodore 64 с двумя 8580";
    strings["YM2151 + SegaPCM Arcade (compatibility)##sesd"].plurals[0] = "YM2151 + SegaPCM Arcade (совместимость)";
    strings["YM2151 + SegaPCM Arcade##sesd"].plurals[0] = "YM2151 + SegaPCM Arcade";
    strings["Game Boy with AY expansion##sesd"].plurals[0] = "Game Boy с расширением AY";
    strings["multi-system##sesd"].plurals[0] = "мульти-система";
    strings["Unknown##sesd"].plurals[0] = "Неизвестно";
    strings["15xx: Set raw period##sesd"].plurals[0] = "15xx: \"Сырое\" значение периода";
    strings["16xx: Set raw period higher nybble (0-F)##sesd"].plurals[0] = "16xx: \"Сырое\" значение периода, старшая тетрада (0-F)";
    strings["Sega Genesis Extended Channel 3##sesd1"].plurals[0] = "Sega Genesis расширенный 3-ий канал";
    strings["Neo Geo CD Extended Channel 2##sesd"].plurals[0] = "Neo Geo CD расширенный 2-ой канал";
    strings["Famicom Disk System (chip)##sesd"].plurals[0] = "Famicom Disk System (чип)";
    strings["Yamaha YM2203 (OPN) Extended Channel 3##sesd"].plurals[0] = "Yamaha YM2203 (OPN) расширенный 3-ий канал";
    strings["Yamaha YM2608 (OPNA) Extended Channel 3##sesd"].plurals[0] = "Yamaha YM2608 (OPNA) расширенный 3-ий канал";
    strings["Yamaha YM2608 (OPNA) Extended Channel 3 and CSM##sesd"].plurals[0] = "Yamaha YM2608 (OPNA) расширенный 3-ий канал и CSM";
    strings["PC Speaker##sesd"].plurals[0] = "PC Speaker (пищалка)";
    strings["ZX Spectrum Beeper##sesd"].plurals[0] = "ZX Spectrum пищалка";
    strings["Yamaha YM2612 (OPN2) Extended Channel 3##sesd"].plurals[0] = "Yamaha YM2612 (OPN2) расширенный 3-ий канал";
    strings["Yamaha YM3526 (OPL) with drums##sesd"].plurals[0] = "Yamaha YM3526 (OPL) с ударными";
    strings["Yamaha YM3812 (OPL2) with drums##sesd"].plurals[0] = "Yamaha YM3812 (OPL2) с ударными";
    strings["Yamaha YMF262 (OPL3) with drums##sesd"].plurals[0] = "Yamaha YMF262 (OPL3) с ударными";
    strings["Yamaha YM2610 (OPNB) Extended Channel 2##sesd"].plurals[0] = "Yamaha YM2610 (OPNB) расширенный 2-ой канал";
    strings["Yamaha YM2413 (OPLL) with drums##sesd"].plurals[0] = "Yamaha YM2413 (OPLL) с ударными";
    strings["Yamaha YM2610B (OPNB2) Extended Channel 3##sesd"].plurals[0] = "Yamaha YM2610B (OPNB2) расширенный 3-ий канал";
    strings["SegaPCM (compatible 5-channel mode)##sesd"].plurals[0] = "SegaPCM (совместимый 5-канальный режим)";
    strings["Yamaha YMF278B (OPL4) with drums##sesd"].plurals[0] = "Yamaha YMF278B (OPL4) с ударными";
    strings["Yamaha Y8950 with drums##sesd"].plurals[0] = "Yamaha Y8950 с ударными";
    strings["Yamaha YM2612 (OPN2) with DualPCM##sesd"].plurals[0] = "Yamaha YM2612 (OPN2) с DualPCM";
    strings["Yamaha YM2612 (OPN2) Extended Channel 3 with DualPCM and CSM##sesd"].plurals[0] = "Yamaha YM2612 (OPN2) расширенный 3-ий канал с DualPCM и CSM";
    strings["Generic PCM DAC##sesd"].plurals[0] = "Типичный ИКМ ЦАП";
    strings["ZX Spectrum Beeper (QuadTone Engine)##sesd"].plurals[0] = "ZX Spectrum пищалка (движок QuadTone)";
    strings["ESS ES1xxx series (ESFM)##sesd"].plurals[0] = "ESS серия ES1xxx (ESFM)";
    strings["Dummy System##sesd"].plurals[0] = "Система-болванка";
    
    //channel names

    strings["Channel 1##sesd"].plurals[0] = "Канал 1";
    strings["Channel 2##sesd"].plurals[0] = "Канал 2";
    strings["Channel 3##sesd"].plurals[0] = "Канал 3";
    strings["Channel 4##sesd"].plurals[0] = "Канал 4";
    strings["Channel 5##sesd"].plurals[0] = "Канал 5";
    strings["Channel 6##sesd"].plurals[0] = "Канал 6";
    strings["Channel 7##sesd"].plurals[0] = "Канал 7";
    strings["Channel 8##sesd"].plurals[0] = "Канал 8";
    strings["Channel 9##sesd"].plurals[0] = "Канал 9";
    strings["Channel 10##sesd"].plurals[0] = "Канал 10";
    strings["Channel 11##sesd"].plurals[0] = "Канал 11";
    strings["Channel 12##sesd"].plurals[0] = "Канал 12";
    strings["Channel 13##sesd"].plurals[0] = "Канал 13";
    strings["Channel 14##sesd"].plurals[0] = "Канал 14";
    strings["Channel 15##sesd"].plurals[0] = "Канал 15";
    strings["Channel 16##sesd"].plurals[0] = "Канал 16";
    strings["Channel 17##sesd"].plurals[0] = "Канал 17";
    strings["Channel 18##sesd"].plurals[0] = "Канал 18";
    strings["Channel 19##sesd"].plurals[0] = "Канал 19";
    strings["Channel 20##sesd"].plurals[0] = "Канал 20";
    strings["Channel 21##sesd"].plurals[0] = "Канал 21";
    strings["Channel 22##sesd"].plurals[0] = "Канал 22";
    strings["Channel 23##sesd"].plurals[0] = "Канал 23";
    strings["Channel 24##sesd"].plurals[0] = "Канал 24";
    strings["Channel 25##sesd"].plurals[0] = "Канал 25";
    strings["Channel 26##sesd"].plurals[0] = "Канал 26";
    strings["Channel 27##sesd"].plurals[0] = "Канал 27";
    strings["Channel 28##sesd"].plurals[0] = "Канал 28";
    strings["Channel 29##sesd"].plurals[0] = "Канал 29";
    strings["Channel 30##sesd"].plurals[0] = "Канал 30";
    strings["Channel 31##sesd"].plurals[0] = "Канал 31";
    strings["Channel 32##sesd"].plurals[0] = "Канал 32";

    strings["Square##sesd"].plurals[0] = "Меандр";

    strings["Square 1##sesd"].plurals[0] = "Меандр 1";
    strings["Square 2##sesd"].plurals[0] = "Меандр 2";
    strings["Square 3##sesd"].plurals[0] = "Меандр 3";

    strings["Pulse##sesd"].plurals[0] = "Прямоуг.";

    strings["Pulse 1##sesd"].plurals[0] = "Прямоуг. 1";
    strings["Pulse 2##sesd"].plurals[0] = "Прямоуг. 2";

    strings["Wave##sesd"].plurals[0] = "Волна";
    strings["Wavetable##sesd"].plurals[0] = "Волн. т.";

    strings["Triangle##sesd"].plurals[0] = "Треуг.";

    strings["PCM##sesd"].plurals[0] = "ИКМ";

    strings["PCM 1##sesd"].plurals[0] = "ИКМ 1";
    strings["PCM 2##sesd"].plurals[0] = "ИКМ 2";
    strings["PCM 3##sesd"].plurals[0] = "ИКМ 3";
    strings["PCM 4##sesd"].plurals[0] = "ИКМ 4";
    strings["PCM 5##sesd"].plurals[0] = "ИКМ 5";
    strings["PCM 6##sesd"].plurals[0] = "ИКМ 6";
    strings["PCM 7##sesd"].plurals[0] = "ИКМ 7";
    strings["PCM 8##sesd"].plurals[0] = "ИКМ 8";
    strings["PCM 9##sesd"].plurals[0] = "ИКМ 9";
    strings["PCM 10##sesd"].plurals[0] = "ИКМ 10";
    strings["PCM 11##sesd"].plurals[0] = "ИКМ 11";
    strings["PCM 12##sesd"].plurals[0] = "ИКМ 12";
    strings["PCM 13##sesd"].plurals[0] = "ИКМ 13";
    strings["PCM 14##sesd"].plurals[0] = "ИКМ 14";
    strings["PCM 15##sesd"].plurals[0] = "ИКМ 15";
    strings["PCM 16##sesd"].plurals[0] = "ИКМ 16";
    strings["PCM 17##sesd"].plurals[0] = "ИКМ 17";
    strings["PCM 18##sesd"].plurals[0] = "ИКМ 18";
    strings["PCM 19##sesd"].plurals[0] = "ИКМ 19";
    strings["PCM 20##sesd"].plurals[0] = "ИКМ 20";
    strings["PCM 21##sesd"].plurals[0] = "ИКМ 21";
    strings["PCM 22##sesd"].plurals[0] = "ИКМ 22";
    strings["PCM 23##sesd"].plurals[0] = "ИКМ 23";
    strings["PCM 24##sesd"].plurals[0] = "ИКМ 24";

    strings["DPCM##sesd"].plurals[0] = "ДИКМ";

    strings["ADPCM##sesd"].plurals[0] = "АДИКМ";

    strings["ADPCM 1##sesd"].plurals[0] = "АДИКМ 1";
    strings["ADPCM 2##sesd"].plurals[0] = "АДИКМ 2";
    strings["ADPCM 3##sesd"].plurals[0] = "АДИКМ 3";

    strings["ADPCM-A 1##sesd"].plurals[0] = "АДИКМ-A 1";
    strings["ADPCM-A 2##sesd"].plurals[0] = "АДИКМ-A 2";
    strings["ADPCM-A 3##sesd"].plurals[0] = "АДИКМ-A 3";
    strings["ADPCM-A 4##sesd"].plurals[0] = "АДИКМ-A 4";
    strings["ADPCM-A 5##sesd"].plurals[0] = "АДИКМ-A 5";
    strings["ADPCM-A 6##sesd"].plurals[0] = "АДИКМ-A 6";

    strings["ADPCM-B##sesd"].plurals[0] = "АДИКМ-B";

    strings["Sample##sesd"].plurals[0] = "Сэмплы";

    strings["DAC Left##sesd"].plurals[0] = "ЦАП левый";
    strings["DAC Right##sesd"].plurals[0] = "ЦАП правый";

    strings["Noise##sesd"].plurals[0] = "Шум";

    strings["Noise 1##sesd"].plurals[0] = "Шум 1";
    strings["Noise 2##sesd"].plurals[0] = "Шум 2";
    strings["Noise 3##sesd"].plurals[0] = "Шум 3";

    strings["Slope##sesd"].plurals[0] = "Скат";

    strings["FM 6/PCM 1##sesd"].plurals[0] = "FM 6/ИКМ 1";
    strings["CSM Timer##sesd"].plurals[0] = "CSM таймер";

    strings["VRC6 Saw##sesd"].plurals[0] = "VRC6 пила";
    
    strings["4OP 1##sesd"].plurals[0] = "4ОП 1";
    strings["4OP 3##sesd"].plurals[0] = "4ОП 3";
    strings["4OP 5##sesd"].plurals[0] = "4ОП 5";
    strings["4OP 7##sesd"].plurals[0] = "4ОП 7";
    strings["4OP 9##sesd"].plurals[0] = "4ОП 9";
    strings["4OP 11##sesd"].plurals[0] = "4ОП 11";

    strings["Kick/FM 7##sesd"].plurals[0] = "Бочка/FM7";
    strings["Kick/FM 16##sesd"].plurals[0] = "Бочка/FM16";
    strings["Kick##sesd"].plurals[0] = "Бочка";
    strings["Snare##sesd"].plurals[0] = "М. бараб.";
    strings["Top##sesd"].plurals[0] = "Тарелка";
    strings["HiHat##sesd"].plurals[0] = "Хай-хэт";
    strings["Tom##sesd"].plurals[0] = "Том-том";
    strings["Rim##sesd"].plurals[0] = "Римшот";

    // channel short names

    strings["CH1##sesd"].plurals[0] = "КН1";
    strings["CH2##sesd"].plurals[0] = "КН2";
    strings["CH3##sesd"].plurals[0] = "КН3";
    strings["CH4##sesd"].plurals[0] = "КН4";
    strings["CH5##sesd"].plurals[0] = "КН5";
    strings["CH6##sesd"].plurals[0] = "КН6";
    strings["CH7##sesd"].plurals[0] = "КН7";
    strings["CH8##sesd"].plurals[0] = "КН8";
    strings["CH9##sesd"].plurals[0] = "КН9";

    strings["NO##sesd"].plurals[0] = "ШУ";
    strings["N1##sesd"].plurals[0] = "Ш1";
    strings["N2##sesd"].plurals[0] = "Ш2";
    strings["N3##sesd"].plurals[0] = "Ш3";

    strings["SL##sesd"].plurals[0] = "СК";

    strings["S1##sesd"].plurals[0] = "М1";
    strings["S2##sesd"].plurals[0] = "М2";
    strings["S3##sesd"].plurals[0] = "М3";
    strings["S4##sesd"].plurals[0] = "М4";
    strings["S5##sesd"].plurals[0] = "М5";
    strings["S6##sesd"].plurals[0] = "М6";

    strings["P1##sesd"].plurals[0] = "И1";
    strings["P2##sesd"].plurals[0] = "И2";
    strings["P3##sesd"].plurals[0] = "И3";
    strings["P4##sesd"].plurals[0] = "И4";
    strings["P5##sesd"].plurals[0] = "И5";
    strings["P6##sesd"].plurals[0] = "И6";
    strings["P7##sesd"].plurals[0] = "И7";
    strings["P8##sesd"].plurals[0] = "И8";
    strings["P9##sesd"].plurals[0] = "И9";
    strings["P10##sesd"].plurals[0] = "И10";
    strings["P11##sesd"].plurals[0] = "И11";
    strings["P12##sesd"].plurals[0] = "И12";
    strings["P13##sesd"].plurals[0] = "И13";
    strings["P14##sesd"].plurals[0] = "И14";
    strings["P15##sesd"].plurals[0] = "И15";
    strings["P16##sesd"].plurals[0] = "И16";
    strings["P17##sesd"].plurals[0] = "И17";
    strings["P18##sesd"].plurals[0] = "И18";
    strings["P19##sesd"].plurals[0] = "И19";
    strings["P20##sesd"].plurals[0] = "И20";
    strings["P21##sesd"].plurals[0] = "И21";
    strings["P22##sesd"].plurals[0] = "И22";
    strings["P23##sesd"].plurals[0] = "И23";
    strings["P24##sesd"].plurals[0] = "И24";

    strings["BD##sesd"].plurals[0] = "ББ";
    strings["SD##sesd"].plurals[0] = "РБ";
    strings["TP##sesd"].plurals[0] = "ТР";
    strings["HH##sesd"].plurals[0] = "ХХ";
    strings["TM##sesd"].plurals[0] = "ТМ";
    strings["RM##sesd"].plurals[0] = "РМ";

    strings["P##sesd"].plurals[0] = "И";

    strings["VS##sesd"].plurals[0] = "VП";

    strings["TR##sesd"].plurals[0] = "ТР";
    strings["DMC##sesd"].plurals[0] = "ДМК";

    strings["WA##sesd"].plurals[0] = "ВТ";
}
