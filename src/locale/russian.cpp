#include <map>
#include <string>
#include "locale.h"

class DivLocale;

void DivLocale::addTranslationsRussian()
{
    strings["test"].plurals[0] = "тест";

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

    //appearance section

    strings["Appearance"].plurals[0] = "Внешний вид";

    strings["Scaling"].plurals[0] = "Масштабирование";
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
    strings["Display Chinese (Simplified) characters"].plurals[0] = "Отображать китайские иероглифы (упрощённые)";
    strings["Display Chinese (Traditional) characters"].plurals[0] = "Отображать китайские иероглифы (традиционные)";
    strings["Display Korean characters"].plurals[0] = "Отображать корейские иероглифы";

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

    strings["Oscilloscope"].plurals[0] = "Осциллограф";
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