# po

these are GNU gettext Portable Object language translation files for Furnace.

if you want to help translating Furnace to your language, read below.

# getting started

Furnace uses GNU gettext for language support.
it was chosen over other solutions due to notable advantages such as ease of use.

text strings in the source code are marked for translation with special delimiters.
afterwards, xgettext is used to extract these and generate a "template" file (furnace.pot) which contains all translatable strings.
some other tools are then used to generate new translation files (e.g. es.po for Spanish) or update them.
during build time, these translation files are converted into binary translation data which is then distributed in releases.

Furnace ships a script to help with the string extraction and translation file updating process.

# preparing

read the following instructions before you start working with translation files.

## preparing the environment

while you may edit translation files regardless of the employed method, setting them up (adding new languages, updating translation files or compiling these) is best done on a Linux environment with the GNU gettext tools installed.

if you use Windows, you may be able to perform setup through the Unix-like MSYS2 environment. see https://www.msys2.org/ for more information.

if you cannot/aren't willing to go through the process of preparing the environment, you may contact me so I can perform setup on my side and provide you with ready-to-edit translation files.

for the testing stage, Linux or Windows with MinGW (installed through the MSYS2 environment) is necessary. Visual Studio/MSVC is not supported yet.

## adding a new language

in order to add a new language, edit the setup script located in `scripts/update-po.sh`.
add a language code to the `EXPORT_LANGS` list (see `https://en.wikipedia.org/wiki/IETF_language_tag` for a list of common language codes).
then run the script in your environment:

```
cd path/to/Furnace/repo
./scripts/update-po.sh
```

if successful, a new file (.po) will be created in the `po` directory that you can edit using a text editor.

## editing an existing translation

just open the translation file in your text editor.

# translating

once you've opened the translation file, you'll see a bunch of strings that may be translated.

"msgid" is the original string in English, whereas "msgstr" is the translation. fill this in with a translated version of the English string.

the `#:` line at the top points to the source code file(s) where the string appears. you may use this if you need to understand which context is the string being used in.

if a string is marked with "c-format", it means the string contains C format codes. these codes consist of a percent sign (`%`) and a type (but don't worry too much about it).
make sure to place these format codes in the translated string appropriately, as if they were objects or subjects in a sentence.

sometimes you will see a string marked as "fuzzy" after running the setup script. if this appears, it means that it is a new string that looks like another. the translation is filled in automatically, but don't trust it as it's almost guaranteed to be wrong.

a `\n` means a line break.

# compiling

to compile your .po file(s) into mo (Machine Object) files which contains translation data that can be consumed by Furnace, run:

```
cd path/to/Furnace/repo
./scripts/build-po.sh
```


if you've encounter a error during compiling, fix it.

if you did this correctly, you will see a directory called `locale` in the po/ directory, containing compiled translation files (with `.mo` extension).

# testing

you may skip this step if you don't feel like building Furnace, or are unable to.

compile Furnace (see the project's README.md file (the developer info section) for information), but in the CMake stage, be sure to pass the CMake flag `-DWITH_LOCALE=ON` before the two dots (`..`) to enable language support in Furnace and therefore compile translation files.


run Furnace from the build directory like so:

```
LANG=language-code ./furnace
```

replace `language-code` with the language code to use.

on Linux, you'll need to add .UTF-8 to language-code.

you should see Furnace start up in the language you specified. if it doesn't, check out the logs.

# submitting

after doing some work, be sure to submit it so you see it in the next version.

you can contact me via email or Discord (be sure to send the translation file), or alternatively send a pull request on GitHub. the latter is preferred if you're going to work frequently.

if sending a pull request, be sure to target the `master` branch.

that's all for now. thank you for helping translate Furnace, and good luck!

- tildearrow

# footnotes

## note 1

the "LocaleSettings: ccjk" string is special. it allows you to define which character sets to load.

this is needed by a couple languages, such as Chinese, Japanese and Korean.

the letters after LocaleSettings mean, in the following order: Chinese (simplified), Chinese (traditional), Japanese and Korean. if any of these is in caps, the corresponding character set will be loaded.

some values include:

```
zh_CN: Ccjk
zh_HK: cCjk
ja_JP: ccJk
ko_KR: ccjK
```

you can load other character ranges by defining them next to the "ccjk" part. for example:

```
LocaleSettings: ccjk 1000-107F 1100-107F
```

each range is in the format FROM-TO, where FROM and TO are hexadecimal values.
if you just want to load one particular character, you still have to specify both FROM and TO. for example 1000-1000.
