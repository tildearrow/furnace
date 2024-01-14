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

class DivLocale;

void DivLocale::addTranslationsRussian()
{
    strings["%d apple"].plurals[0] = "%d яблоко";
    strings["%d apple"].plurals[1] = "%d яблока";
    strings["%d apple"].plurals[2] = "%d яблок";

    //MENU BAR ITEMS

    strings["File##menubar"].plurals[0] = "Файл##menubar";
    strings["file##menubar"].plurals[0] = "файл##menubar";
    strings["Edit##menubar"].plurals[0] = "Правка##menubar";
    strings["edit##menubar"].plurals[0] = "правка##menubar";
    strings["Settings##menubar"].plurals[0] = "Настройки##menubar";
    strings["settings##menubar"].plurals[0] = "настройки##menubar";
    strings["Window##menubar"].plurals[0] = "Окно##menubar";
    strings["window##menubar"].plurals[0] = "окно##menubar";
    strings["Help##menubar"].plurals[0] = "Справка##menubar";
    strings["help##menubar"].plurals[0] = "справка##menubar";

    //WINDOW NAMES

    strings["Settings###Settings"].plurals[0] = "Настройки###Settings";
    strings["Pattern###Pattern"].plurals[0] = "Паттерны###Pattern";
    strings["Orders###Orders"].plurals[0] = "Матр. патт.###Orders";
    strings["Statistics###Statistics"].plurals[0] = "Статистика###Statistics";
    strings["Song Info###Song Information"].plurals[0] = "О треке###Song Information";
    strings["Subsongs###Subsongs"].plurals[0] = "Подпесни###Subsongs";
    strings["About Furnace###About Furnace"].plurals[0] = "О Furnace###About Furnace";
    strings["Channels###Channels"].plurals[0] = "Каналы###Channels";
    strings["Oscilloscope (per-channel)###Oscilloscope (per-channel)"].plurals[0] = "Осц-фы (отд. кан.)###Oscilloscope (per-channel)";
    strings["Clock###Clock"].plurals[0] = "Часы###Clock";
    strings["Compatibility Flags###Compatibility Flags"].plurals[0] = "Флаги совм.###Compatibility Flags";
    strings["Instruments###Instruments"].plurals[0] = "Инструменты###Instruments";
    strings["Wavetables###Wavetables"].plurals[0] = "Волн. табл.###Wavetables";
    strings["Debug###Debug"].plurals[0] = "Отладка###Debug";
    strings["Samples###Samples"].plurals[0] = "Сэмплы###Samples";
    strings["MobileEdit###MobileEdit"].plurals[0] = "Моб. меню ред.###MobileEdit";
    strings["Mobile Controls###Mobile Controls"].plurals[0] = "Моб. элем. упр.###Mobile Controls";
    strings["Mobile Menu###Mobile Menu"].plurals[0] = "Моб. меню###Mobile Menu";
    strings["Play/Edit Controls###Play/Edit Controls"].plurals[0] = "Упр. ред./воспр.###Play/Edit Controls";
    strings["Play Controls###Play Controls"].plurals[0] = "Упр. воспр.###Play Controls";
    strings["Edit Controls###Edit Controls"].plurals[0] = "Упр. ред.###Edit Controls";
    strings["Effect List###Effect List"].plurals[0] = "Спис. эффектов###Effect List";
    strings["Find/Replace###Find/Replace"].plurals[0] = "Найти/Заменить###Find/Replace";
    strings["Grooves###Grooves"].plurals[0] = "Ритм-паттерны###Grooves";
    strings["Instrument Editor###Instrument Editor"].plurals[0] = "Ред. инструментов###Instrument Editor";
    strings["Log Viewer###Log Viewer"].plurals[0] = "Просмотр логов###Log Viewer";
    strings["Mixer###Mixer"].plurals[0] = "Микшер###Mixer";
    strings["OrderSel###OrderSel"].plurals[0] = "Выб. матр. пат.###OrderSel";
    strings["Oscilloscope###Oscilloscope"].plurals[0] = "Осциллограф###Oscilloscope";
    strings["Pattern Manager###Pattern Manager"].plurals[0] = "Упр. паттернами###Pattern Manager";
    strings["Input Pad###Input Pad"].plurals[0] = "Панель ввода###Input Pad";
    strings["Piano###Piano"].plurals[0] = "Клав. пианино###Piano";
    strings["Register View###Register View"].plurals[0] = "Просм. регистров###Register View";
    strings["Sample Editor###Sample Editor"].plurals[0] = "Ред. сэмплов###Sample Editor";
    strings["Song Comments###Song Comments"].plurals[0] = "Комм. трека###Song Comments";
    strings["Speed###Speed"].plurals[0] = "Скорость###Speed";
    strings["Spoiler###Spoiler"].plurals[0] = "Спойлер###Spoiler";
    strings["Chip Manager###Chip Manager"].plurals[0] = "Упр. чипами###Chip Manager";
    strings["Volume Meter###Volume Meter"].plurals[0] = "Изм. громкости###Volume Meter";
    strings["Wavetable Editor###Wavetable Editor"].plurals[0] = "Ред. волн. таблиц###Wavetable Editor";
    strings["Oscilloscope (X-Y)###Oscilloscope (X-Y)"].plurals[0] = "Осциллограф (X-Y)###Oscilloscope (X-Y)";

    //EFFECT LIST

    //common (non-chip-specific) effects

    strings["00xy: Arpeggio"].plurals[0] = "00xy: Арпеджио";
    
    //MACRO EDITOR

    //macro names

    strings["Volume"].plurals[0] = "Громкость";
    strings["Gain"].plurals[0] = "Усиление";

    //bitfield macros strings

    strings["hold"].plurals[0] = "удержание";
    strings["alternate"].plurals[0] = "изм. направл.";
    strings["direction"].plurals[0] = "направление";
    strings["enable"].plurals[0] = "вкл.";

    strings["right"].plurals[0] = "правый";
    strings["left"].plurals[0] = "левый";
    strings["rear right"].plurals[0] = "задний правый";
    strings["rear left"].plurals[0] = "задний левый";

    //macro hover notes

    strings["exponential"].plurals[0] = "экспоненциальное";
    strings["linear"].plurals[0] = "линейное";
    strings["direct"].plurals[0] = "прямое";

    strings["Release"].plurals[0] = "Затухание";
    strings["Loop"].plurals[0] = "Цикл";

    strings["Fixed"].plurals[0] = "Абсолютное";
    strings["Relative"].plurals[0] = "Относительное";

    strings["HP/K2, HP/K2"].plurals[0] = "ФВЧ/K2, ФВЧ/K2";
    strings["HP/K2, LP/K1"].plurals[0] = "ФВЧ/K2, ФНЧ/K1";
    strings["LP/K2, LP/K2"].plurals[0] = "ФНЧ/K2, ФНЧ/K2";
    strings["LP/K2, LP/K1"].plurals[0] = "ФНЧ/K2, ФНЧ/K1";

    strings["Saw"].plurals[0] = "Пила";
    strings["Square"].plurals[0] = "Меандр";
    strings["Triangle"].plurals[0] = "Треугольная волна";
    strings["Random"].plurals[0] = "Шум";

    //SETTINGS

    //keybind prompt
    strings["Press key..."].plurals[0] = "Нажмите клавишу...";
    strings["Do you want to save your settings?"].plurals[0] = "Вы хотите сохранить свои настройки?";

    //general section
    strings["General"].plurals[0] = "Основные";

    strings["Program"].plurals[0] = "Программа";
    strings["Render backend"].plurals[0] = "Библиотека отрисовки";
    strings["you may need to restart Furnace for this setting to take effect."].plurals[0] = "возможно, вам потребуется перезапустить Furnace, чтобы эта настрока применилась.";
    strings["Render driver"].plurals[0] = "Драйвер отрисовки";
    strings["Automatic"].plurals[0] = "Выбирать автоматически";
    strings["Late render clear"].plurals[0] = "Запаздывающая очистка буфера отрисовщика";
    strings["calls rend->clear() after rend->present(). might reduce UI latency by one frame in some drivers."].plurals[0] = "вызывает rend->clear() после rend->present(). может устранить запаздывание отрисовки интерфейса на один кадр для некоторых драйверов.";
    strings["Power-saving mode"].plurals[0] = "Режим энергосбережения";
    strings["saves power by lowering the frame rate to 2fps when idle.\nmay cause issues under Mesa drivers!"].plurals[0] = "уменьшает энергопотребление при помощи уменьшения частоты отрисовки до двух кадров в секунду в режиме ожидания.\nможет приводить к проблемам на драйверах Mesa!";
    strings["Disable threaded input (restart after changing!)"].plurals[0] = "Отключить обработку нажатий для превью инструмента в отдельном потоке (перезагрузите программу после изменения!)";
    strings["threaded input processes key presses for note preview on a separate thread (on supported platforms), which reduces latency.\nhowever, crashes have been reported when threaded input is on. enable this option if that is the case."].plurals[0] = "обработка нажатий клавиш для превью инструмента происходит в отдельном потоке (на поддерживаемых платформах), что позволяет уменьшить задержку ввода.\nтем не менее, есть сообщения о вылетах программы при выключённой настройке. включите её, если у вас программа вылетает.";
    strings["Enable event delay"].plurals[0] = "Включить задержку событий";
    strings["may cause issues with high-polling-rate mice when previewing notes."].plurals[0] = "может привести к проблемам во время превью инструмента, если подключена мышь с большой частотой обновления.";
    strings["Per-channel oscilloscope threads"].plurals[0] = "Потоки исполнения осциллографов для отдельных каналов";
    strings["you're being silly, aren't you? that's enough."].plurals[0] = "может, хватит уже хернёй страдать? этого достаточно.";
    strings["what are you doing? stop!"].plurals[0] = "ты чё делаешь? хватит!";
    strings["it is a bad idea to set this number higher than your CPU core count (%d)!"].plurals[0] = "не рекомендуется выставлять здесь значение, большее количества ядер вашего ЦП (%d)!";

    strings["File"].plurals[0] = "Файл";
    strings["Use system file picker"].plurals[0] = "Использовать диалоговое окно выбора файлов ОС";
    strings["Number of recent files"].plurals[0] = "Количество недавних файлов";
    strings["Compress when saving"].plurals[0] = "Сжимать сохраняемые файлы";
    strings["use zlib to compress saved songs."].plurals[0] = "использовать библиотеку zlib для сжатия сохраняемых модулей.";
    strings["Save unused patterns"].plurals[0] = "Сохранять неиспользуемые паттерны";
    strings["Use new pattern format when saving"].plurals[0] = "Использовать новый формат сохранения паттернов";
    strings["use a packed format which saves space when saving songs.\ndisable if you need compatibility with older Furnace and/or tools\nwhich do not support this format."].plurals[0] = "использовать сжатый формат сохранения паттернов, что позволяет уменьшить размер файла с модулем.\nотключите, если нужна совместимость со старыми версиями Furnace и/или другими программами,\nкоторые не поддерживают старый формат.";
    strings["Don't apply compatibility flags when loading .dmf"].plurals[0] = "Не применять флаги совместимости при загрузке .dmf";
    strings["do not report any issues arising from the use of this option!"].plurals[0] = "не жалуйтесь на проблемы, которые возникнут после включения этой настройки!";
    strings["Play after opening song:"].plurals[0] = "Проигрывание модуля после его загрузки:";
    strings["No##pol0"].plurals[0] = "Нет##pol0";
    strings["Only if already playing##pol1"].plurals[0] = "Только если до этого уже играл##pol1";
    strings["Yes##pol0"].plurals[0] = "Да##pol0";
    strings["Audio export loop/fade out time:"].plurals[0] = "Количество циклов проигрывания и время затухания при экспорте аудио:";
    strings["Set to these values on start-up:##fot0"].plurals[0] = "Выставить эти значения при запуске:##fot0";
    strings["Loops"].plurals[0] = "Циклы";
    strings["Fade out (seconds)"].plurals[0] = "Затухание (в секундах)";
    strings["Remember last values##fot1"].plurals[0] = "Запоминать предыдущие значения##fot1";
    strings["Store instrument name in .fui"].plurals[0] = "Сохранять название инструмента в файле .fui";
    strings["when enabled, saving an instrument will store its name.\nthis may increase file size."].plurals[0] = "При включении имя инструмента будет сохраняться в файле.\nэто может увеличить размер файла.";
    strings["Load instrument name from .fui"].plurals[0] = "Загружать имя инструмента из файла .fui";
    strings["when enabled, loading an instrument will use the stored name (if present).\notherwise, it will use the file name."].plurals[0] = "При включении имя инструмента будет загружаться из файла (при наличии имени в файле).\nВ противном случае будет использоваться имя файла.";

    strings["New Song"].plurals[0] = "Новая композиция";
    strings["Initial system:"].plurals[0] = "Система по умолчанию:";
    strings["Current system"].plurals[0] = "Текущая";
    strings["Randomize"].plurals[0] = "Выбрать случайно";
    strings["Reset to defaults"].plurals[0] = "Устан. по умолчанию";
    strings["Name"].plurals[0] = "Название";
    strings["Invert"].plurals[0] = "Обр.";
    strings["Volume"].plurals[0] = "Громкость";
    strings["Panning"].plurals[0] = "Панорамирование";
    strings["Front/Rear"].plurals[0] = "Передн./задн.";
    strings["Configure"].plurals[0] = "Настроить";
    strings["When creating new song:"].plurals[0] = "При создании новой композиции:";
    strings["Display system preset selector##NSB0"].plurals[0] = "Отобразить окно выбора пресета системы##NSB0";
    strings["Start with initial system##NSB1"].plurals[0] = "Начать с системы по умолчанию##NSB1";
    strings["Default author name"].plurals[0] = "Имя автора по умолчанию";

    strings["Start-up"].plurals[0] = "Запуск";
    strings["Disable fade-in during start-up"].plurals[0] = "Отключить плавное появление интерфейса при запуске";
    strings["About screen party time"].plurals[0] = "Вечеринка на экране \"О программе\"";
    strings["Warning: may cause epileptic seizures."].plurals[0] = "Внимание: может вызвать эпилептические приступы.";

    strings["Behavior"].plurals[0] = "Поведение программы";
    strings["New instruments are blank"].plurals[0] = "Пустые новые инструменты";

    strings["Language"].plurals[0] = "Язык";
    strings["GUI language"].plurals[0] = "Язык интерфейса";

    //audio section

    strings["Audio"].plurals[0] = "Аудио";

    strings["Output"].plurals[0] = "Вывод";
    strings["Backend"].plurals[0] = "Интерфейс";
    strings["Driver"].plurals[0] = "Драйвер";
    strings["Automatic"].plurals[0] = "Автоматически";
    strings["you may need to restart Furnace for this setting to take effect."].plurals[0] = "возможно, вам придётся перезапустить Furnace для применения настройки.";
    strings["Device"].plurals[0] = "Устройство вывода";
    strings["<click on OK or Apply first>"].plurals[0] = "=<сначала нажмите на кнопки \"ОК\" или \"Применить\">";
    strings["ALERT - TRESPASSER DETECTED"].plurals[0] = "ВНИМАНИЕ - ОБНАРУЖЕН НАРУШИТЕЛЬ";
    strings["you have been arrested for trying to engage with a disabled combo box."].plurals[0] = "вы были арестованы за попытку взаимодействия с выключенным выпадающим списком.";
    strings["<System default>"].plurals[0] = "=<По умолчанию>";
    strings["Sample rate"].plurals[0] = "Частота дискретизации";
    strings["Outputs"].plurals[0] = "Выводы";
    strings["Channels"].plurals[0] = "Число каналов";
    strings["What?"].plurals[0] = "Что?";
    strings["Buffer size"].plurals[0] = "Размер буфера";
    strings["%d (latency: ~%.1fms)"].plurals[0] = "=%d (задержка: ~%.1f мс)";
    strings["Multi-threaded (EXPERIMENTAL)"].plurals[0] = "Многопоточность (ЭКСПЕРИМЕНТАЛЬНАЯ)";
    strings["runs chip emulation on separate threads.\nmay increase performance when using heavy emulation cores.\n\nwarnings:\n- experimental!\n- only useful on multi-chip songs."].plurals[0] = "исполняет эмуляторы чипов в отдельных потоках.\nможет повысить производительность при использовании тяжёлых эмуляторов.\n\nвнимание:\n- экспериментальная функция!\n- полезна только для композиций, использующих несколько чипов.";
    strings["Number of threads"].plurals[0] = "Количество потоков";
    strings["that's the limit!"].plurals[0] = "это предел!";
    strings["it is a VERY bad idea to set this number higher than your CPU core count (%d)!"].plurals[0] = "это ОЧЕНЬ плохая идея - устанавливать это значение большим, чем колчество ядер ЦП (%d)!";
    strings["Low-latency mode"].plurals[0] = "Режим малой задержки";
    strings["reduces latency by running the engine faster than the tick rate.\nuseful for live playback/jam mode.\n\nwarning: only enable if your buffer size is small (10ms or less)."].plurals[0] = "уменьшает задержку, исполняя код движка трекера быстрее, чем указано в настройках.\nполезно для режима исполнения в реальном времени.\n\nвнимание: включайте только если размер вашего аудиобуфера мал (10 мс или меньше).";
    strings["Force mono audio"].plurals[0] = "Принудительно сводить в моно";
    strings["Exclusive mode"].plurals[0] = "Исключительный режим";
    strings["want: %d samples @ %.0fHz (%d %s)"].plurals[0] = "запрошено: %d сэмплов @ %.0fHz (%d %s)";
    strings["channel"].plurals[0] = "канал";
    strings["channel"].plurals[1] = "канала";
    strings["channel"].plurals[2] = "каналов";
    strings["got: %d samples @ %.0fHz (%d %s)"].plurals[0] = "получено: %d сэмплов @ %.0fHz (%d %s)";

    strings["Mixing"].plurals[0] = "Микширование";
    strings["Quality"].plurals[0] = "Качество";
    strings["Software clipping"].plurals[0] = "Программное ограничение сигнала";
    strings["DC offset correction"].plurals[0] = "Коррекция смещения пост. составляющей";

    strings["Metronome"].plurals[0] = "Метроном";
    strings["Volume##MetroVol"].plurals[0] = "Громкость метронома##MetroVol";

    strings["Sample preview"].plurals[0] = "Превью сэмпла";

    //MIDI section

    //emulation section

    //keyboard section

    //interface selection

    //appearance section

    strings["Appearance"].plurals[0] = "Внешний вид";

    strings["Scaling"].plurals[0] = "Масштаб";
    strings["Automatic UI scaling factor"].plurals[0] = "Автоматическое масштабирование интерфейса";
    strings["UI scaling factor"].plurals[0] = "Масштаб интерфейса";
    strings["Icon size"].plurals[0] = "Размер иконок";

    strings["Text"].plurals[0] = "Текст";
    strings["Font renderer"].plurals[0] = "Отрисовщик шрифта";
    strings["Main font"].plurals[0] = "Основной шрифт";
    strings["Size##MainFontSize"].plurals[0] = "Размер##MainFontSize";
    strings["Header font"].plurals[0] = "Шрифт заголовков";
    strings["Size##HeadFontSize"].plurals[0] = "Размер##HeadFontSize";
    strings["Pattern font"].plurals[0] = "Шрифт паттернов";
    strings["Size##PatFontSize"].plurals[0] = "Размер##PatFontSize";
    strings["Anti-aliased fonts"].plurals[0] = "Сглаживание шрифтов";
    strings["Support bitmap fonts"].plurals[0] = "Поддерживать растровые шрифты";
    strings["Hinting:"].plurals[0] = "Хинтование";
    strings["Off (soft)##fh0"].plurals[0] = "Нет (слабое)##fh0";
    strings["Slight##fh1"].plurals[0] = "Небольшое##fh1";
    strings["Normal##fh2"].plurals[0] = "Нормальное##fh2";
    strings["Full (hard)##fh3"].plurals[0] = "Полное (жёсткое)##fh3";
    strings["Auto-hinter:"].plurals[0] = "Автоматическое хинтирование";
    strings["Disable##fah0"].plurals[0] = "Отключить##fah0";
    strings["Enable##fah1"].plurals[0] = "Включить##fah1";
    strings["Force##fah2"].plurals[0] = "Принудительное##fah2";
    strings["Display Japanese characters"].plurals[0] = "Отображать японские символы (вкл. иероглифы)";
    strings["Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "このオプションは、十分なグラフィックメモリがある場合にのみ切り替えてください。\n"
            "これは、Dear ImGuiにダイナミックフォントアトラスが実装されるまでの一時的な解決策です。"].plurals[0] = 
            
            "Включайте эту настройку только в случае наличия достаточного количества графической памяти.\n"
            "Это временное решение, поскольку пока Dear ImGui не поддерживает динамический атлас шрифтов.\n\n"
            "このオプションは、十分なグラフィックメモリがある場合にのみ切り替えてください。\n"
            "これは、Dear ImGuiにダイナミックフォントアトラスが実装されるまでの一時的な解決策です。";
    strings["Display Chinese (Simplified) characters"].plurals[0] = "Отображать китайские иероглифы (упрощённые)";
    strings["Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "请在确保你有足够的显存后再启动此设定\n"
            "这是一个在ImGui实现动态字体加载之前的临时解决方案"].plurals[0] = 
            
            "Включайте эту настройку только в случае наличия достаточного количества графической памяти.\n"
            "Это временное решение, поскольку пока Dear ImGui не поддерживает динамический атлас шрифтов.\n\n"
            "请在确保你有足够的显存后再启动此设定\n"
            "这是一个在ImGui实现动态字体加载之前的临时解决方案";
    strings["Display Chinese (Traditional) characters"].plurals[0] = "Отображать китайские иероглифы (традиционные)";
    strings["Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "請在確保你有足夠的顯存后再啟動此設定\n"
            "這是一個在ImGui實現動態字體加載之前的臨時解決方案"].plurals[0] = 
            
            "Включайте эту настройку только в случае наличия достаточного количества графической памяти.\n"
            "Это временное решение, поскольку пока Dear ImGui не поддерживает динамический атлас шрифтов.\n\n"
            "請在確保你有足夠的顯存后再啟動此設定\n"
            "這是一個在ImGui實現動態字體加載之前的臨時解決方案";
    strings["Display Korean characters"].plurals[0] = "Отображать корейские иероглифы";
    strings["Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "그래픽 메모리가 충분한 경우에만 이 옵션을 선택하십시오.\n"
            "이 옵션은 Dear ImGui에 동적 글꼴 아틀라스가 구현될 때까지 임시 솔루션입니다."].plurals[0] = 
            
            "Включайте эту настройку только в случае наличия достаточного количества графической памяти.\n"
            "Это временное решение, поскольку пока Dear ImGui не поддерживает динамический атлас шрифтов.\n\n"
            "그래픽 메모리가 충분한 경우에만 이 옵션을 선택하십시오.\n"
            "이 옵션은 Dear ImGui에 동적 글꼴 아틀라스가 구현될 때까지 임시 솔루션입니다.";

    strings["Program"].plurals[0] = "Программа";
    strings["Title bar:"].plurals[0] = "Полоса заголовка окна";
    strings["Song Name - Furnace##tbar1"].plurals[0] = "Название композиции - Furnace##tbar1";
    strings["file_name.fur - Furnace##tbar2"].plurals[0] = "название_файла.fur - Furnace##tbar2";
    strings["/path/to/file.fur - Furnace##tbar3"].plurals[0] = "/путь/к/файлу.fur - Furnace##tbar3";
    strings["Display system name on title bar"].plurals[0] = "Отображать название чипа/системы в полосе заголовка окна";
    strings["Display chip names instead of \"multi-system\" in title bar"].plurals[0] = "Отображать названия чипов/систем вместо \"мульти-система\" в полосе заголовка окна";
    strings["Status bar:"].plurals[0] = "Строка состояния:";
    strings["Cursor details##sbar0"].plurals[0] = "Информация о выделенном элементе##sbar0";
    strings["File path##sbar1"].plurals[0] = "Путь к файлу##sbar1";
    strings["Cursor details or file path##sbar2"].plurals[0] = "Информация о выделенном элементе или путь к файлу##sbar2";
    strings["Nothing##sbar3"].plurals[0] = "Ничего##sbar3";
    strings["Export options layout:"].plurals[0] = "Вид настроек экспорта:";
    strings["Sub-menus in File menu##eol0"].plurals[0] = "Подпункты в меню \"Файл\"##eol0";
    strings["Modal window with tabs##eol1"].plurals[0] = "Модальное окно с вкладками##eol1";
    strings["Modal windows with options in File menu##eol2"].plurals[0] = "Модальное окно с настройками в меню \"Файл\"";
    strings["Capitalize menu bar"].plurals[0] = "Названия пунктов в горизонтальном меню с большой буквы";
    strings["Display add/configure/change/remove chip menus in File menu"].plurals[0] = "Отображать в меню \"Файл\" пункты: добавить/настроить/изменить/убрать чип";

    strings["Orders"].plurals[0] = "Матрица паттернов";
    strings["Highlight channel at cursor in Orders"].plurals[0] = "Выделить в матрице паттернов канал, на котором находится курсор";
    strings["Orders row number format:"].plurals[0] = "Формат отображения номера строки матрицы паттернов:";
    strings["Decimal##orbD"].plurals[0] = "Десятеричный##orbD";
    strings["Hexadecimal##orbH"].plurals[0] = "Шестнадцатеричный##orbH";

    strings["Pattern"].plurals[0] = "Паттерн";
    strings["Center pattern view"].plurals[0] = "Центрировать отображаемые паттерны внутри окна";
    strings["Overflow pattern highlights"].plurals[0] = "Продолжать полосы подсветки строк паттернов за пределы самих паттернов";
    strings["Display previous/next pattern"].plurals[0] = "Отображать предыдущий/следующий паттерн";
    strings["Pattern row number format:"].plurals[0] = "Формат отображения номера строки паттерна:";
    strings["Decimal##prbD"].plurals[0] = "Десятеричный##prbD";
    strings["Hexadecimal##prbH"].plurals[0] = "Шестнадцатеричный##prbH";
    strings["Pattern view labels:"].plurals[0] = "Маркировка ячеек в паттерне";
    strings["Note off (3-char)"].plurals[0] = "\"Отпускание клавиши\" (резкое) (3 символа)";
    strings["Note release (3-char)"].plurals[0] = "\"Отпускание клавиши\" (с включением фазы затухания огибающей) (3 символа)";
    strings["Macro release (3-char)"].plurals[0] = "\"Отпускание клавиши\" для макросов (3 символа)";
    strings["Empty field (3-char)"].plurals[0] = "Пустая ячейка (3 символа)";
    strings["Empty field (2-char)"].plurals[0] = "Пустая ячейка (2 символа)";
    strings["Pattern view spacing after:"].plurals[0] = "Разбивка в отображении паттерна:";
    strings["Note"].plurals[0] = "Нота";
    strings["Instrument"].plurals[0] = "Инструмент";
    strings["Volume"].plurals[0] = "Громкость";
    strings["Effect"].plurals[0] = "Индекс эффекта";
    strings["Effect value"].plurals[0] = "Параметр эффекта";
    strings["Single-digit effects for 00-0F"].plurals[0] = "Отображать одной цифрой индекс эффекта для индексов 00-0F";
    strings["Use flats instead of sharps"].plurals[0] = "Отображать бемоли вместо диезов";
    strings["Use German notation"].plurals[0] = "Использовать немецкие имена нот";

    strings["Channel"].plurals[0] = "Канал";
    strings["Channel style:"].plurals[0] = "Стиль заголовка:";
    strings["Classic##CHS0"].plurals[0] = "Классический##CHS0";
    strings["Line##CHS1"].plurals[0] = "Линия##CHS1";
    strings["Round##CHS2"].plurals[0] = "Со скруглениями##CHS2";
    strings["Split button##CHS3"].plurals[0] = "С отдельной кнопкой отключения звука##CHS3";
    strings["Square border##CHS4"].plurals[0] = "С прямоугольной границей вокруг названия##CHS4";
    strings["Round border##CHS5"].plurals[0] = "Со скруглённой границей вокруг названия##CHS5";
    strings["Channel volume bar:"].plurals[0] = "Полоска громкости в заголовке канала:";
    strings["Non##CHV0"].plurals[0] = "Нет##CHV0";
    strings["Simple##CHV1"].plurals[0] = "Простая##CHV1";
    strings["Stereo##CHV2"].plurals[0] = "Стерео##CHV2";
    strings["Real##CHV3"].plurals[0] = "Настоящая громкость##CHV3";
    strings["Real (stereo)##CHV4"].plurals[0] = "Настоящая громкость (стерео)##CHV4";
    strings["Channel feedback style:"].plurals[0] = "Подсветка заголовка канала:";
    strings["Off##CHF0"].plurals[0] = "Выкл.##CHF0";
    strings["Note##CHF1"].plurals[0] = "Начало ноты##CHF1";
    strings["Volume##CHF2"].plurals[0] = "Пропорционально громкости##CHF2";
    strings["Active##CHF3"].plurals[0] = "При активности канала##CHF3";
    strings["Channel font:"].plurals[0] = "Шрифт заголовка канала:";
    strings["Regular##CHFont0"].plurals[0] = "Обычный##CHFont0";
    strings["Monospace##CHFont1"].plurals[0] = "Моноширинный##CHFont1";
    strings["Center channel name"].plurals[0] = "Центрировать название канала";
    strings["Channel colors:"].plurals[0] = "Цвета заголовка канала";
    strings["Single##CHC0"].plurals[0] = "Единый цвет##CHC0";
    strings["Channel type##CHC1"].plurals[0] = "Согласно типу канала##CHC1";
    strings["Instrument type##CHC2"].plurals[0] = "Согласно типу инструмента##CHC2";
    strings["Channel name colors:"].plurals[0] = "Цвета названия канала:";
    strings["Single##CTC0"].plurals[0] = "Единый цвет##CTC0";
    strings["Channel type##CTC1"].plurals[0] = "Согласно типу канала##CTC1";
    strings["Instrument type##CTC2"].plurals[0] = "Согласно типу инструмента##CTC2";

    strings["Assets"].plurals[0] = "Представление ресурсов модуля";
    strings["Unified instrument/wavetable/sample list"].plurals[0] = "Единый список инструментов, волновых таблиц и сэмплов";
    strings["Horizontal instrument list"].plurals[0] = "Горизонтальный список инструментов";
    strings["Instrument list icon style:"].plurals[0] = "Стиль иконок в списке инструментов:";
    strings["None##iis0"].plurals[0] = "Не показывать##iis0";
    strings["Graphical icons##iis1"].plurals[0] = "Графические иконки##iis1";
    strings["Letter icons##iis2"].plurals[0] = "Иконки с буквами##iis2";
    strings["Colorize instrument editor using instrument type"].plurals[0] = "Изменять оттенки цветов редактора инструмента согласно типу инструмента";

    strings["Macro Editor"].plurals[0] = "Редактор макросов";
    strings["Macro editor layout:"].plurals[0] = "Компоновка редактора макросов:";
    strings["Unified##mel0"].plurals[0] = "Общий список##mel0";
    strings["Grid##mel2"].plurals[0] = "Прямоугольная сетка##mel2";
    strings["Single (with list)##mel3"].plurals[0] = "Окно редактирования одного макроса + список##mel3";
    strings["Use classic macro editor vertical slider"].plurals[0] = "Использовать классическую вертикальную полосу прокрутки";

    strings["Wave Editor"].plurals[0] = "Редактор волновых таблиц";
    strings["Use compact wave editor"].plurals[0] = "Использовать компактный редактор волновых таблиц";

    strings["FM Editor"].plurals[0] = "Редактор FM-инструментов";
    strings["FM parameter names:"].plurals[0] = "Названия параметров:";
    strings["Friendly##fmn0"].plurals[0] = "Понятные##fmn0";
    strings["Technical##fmn1"].plurals[0] = "Исходные формальные##fmn1";
    strings["Technical (alternate)##fmn2"].plurals[0] = "Исходные формальные (альтернативные)##fmn2";
    strings["Use standard OPL waveform names"].plurals[0] = "Использовать стандартные названия волн для чипов серии OPL";
    strings["FM parameter editor layout:"].plurals[0] = "Компоновка редактора FM-инструментов";
    strings["Modern##fml0"].plurals[0] = "Современная##fml0";
    strings["Compact (2x2, classic)##fml1"].plurals[0] = "Компактная (2x2, классическая)##fml1";
    strings["Compact (1x4)##fml2"].plurals[0] = "Компактная (1x4)##fml2";
    strings["Compact (4x1)##fml3"].plurals[0] = "Компактная (4x1)##fml3";
    strings["Alternate (2x2)##fml4"].plurals[0] = "Альтернативная (2x2)##fml4";
    strings["Alternate (1x4)##fml5"].plurals[0] = "Альтернативная (1x4)##fml5";
    strings["Alternate (4x1)##fml6"].plurals[0] = "Альтернативная (4x1)##fml6";
    strings["Position of Sustain in FM editor:"].plurals[0] = "Позиция параметра \"Поддержка\" в редакторе:";
    strings["Between Decay and Sustain Rate##susp0"].plurals[0] = "Между спадом и уклоном поддержки##susp0";
    strings["After Release Rate##susp1"].plurals[0] = "После уклона затухания##susp1";
    strings["Use separate colors for carriers/modulators in FM editor"].plurals[0] = "Использовать различные цвета для модулирующих и несущих операторов";
    strings["Unsigned FM detune values"].plurals[0] = "Беззнаковое значение параметра расстройки";

    strings["Statistics"].plurals[0] = "Окно статистики";
    strings["Chip memory usage unit:"].plurals[0] = "Единицы измерения использования памяти чипа:";
    strings["Bytes##MUU0"].plurals[0] = "Байты##MUU0";
    strings["Kilobytes##MUU1"].plurals[0] = "Килобайты##MUU1";

    strings["Oscilloscope##set"].plurals[0] = "Осциллограф##set";
    strings["Rounded corners"].plurals[0] = "Закруглённые края";
    strings["Border"].plurals[0] = "Граница";
    strings["Mono"].plurals[0] = "Моно";
    strings["Anti-aliased"].plurals[0] = "Со сглаживанием";
    strings["Fill entire window"].plurals[0] = "Полностью заполняет окно";
    strings["Waveform goes out of bounds"].plurals[0] = "Волна может выходить за пределы окна";

    strings["Windows"].plurals[0] = "Окна";
    strings["Rounded window corners"].plurals[0] = "Закруглённые края окон";
    strings["Rounded buttons"].plurals[0] = "Закруглённые края кнопок";
    strings["Rounded tabs"].plurals[0] = "Закруглённые края заголовков вкладок";
    strings["Rounded scrollbars"].plurals[0] = "Закруглённые края ползунка полосы прокрутки";
    strings["Rounded menu corners"].plurals[0] = "Закруглённые края выпадающих меню";
    strings["Borders around widgets"].plurals[0] = "Границы вокруг кнопок, списков и т.д.";

    strings["Misc"].plurals[0] = "Разное";
    strings["Wrap text"].plurals[0] = "Переносить текст на новую строку";
    strings["Wrap text in song/subsong comments window."].plurals[0] = "Переносить текст на новую строку в окне информации/комментариев к композиции.";
    strings["Frame shading in text windows"].plurals[0] = "Градиент в текстовых окнах";
    strings["Apply frame shading to the multiline text fields\nsuch as song/subsong info/comments."].plurals[0] = "Применять градиент в окне информации/комментариев к композиции.";
    //strings[""].plurals[0] = "";
}