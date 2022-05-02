MODIFIED

this is a modified version of the NES audio emulation core.
it converts the files to UTF-8 and Unix line endings.

XGM SOURCE ARCHIVE

This source archive is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY. You can reuse these source code freely. However,
we possibly change the structure and interface of the program code without 
any advance notice.

HOW TO COMPILE

Open the workspace file top.sln with Visual C++ 7.0 or later
version. We can compile both KbMediaPlayer and Winamp version of
NSFplug on this workspace.

To make a KbMediaPlayer version of NSFplug, that is, in_nsf.kpi,
Please choose 'kbnsf' project as an active project.  Then, you can
build in_nsf.kpi by build menu.

On the other hand, to make a Winamp version of NSFplug, activate 
'wa2nsf' project and build it. 

Note that after the build process, VC++ copies the plugin files to:

C:\Program Files\KbMediaPlayer\Plugins\OK\in_nsf\in_nsf.kpi
C:\Program Files\Windamp\Plugins\in_nsf.dll

If you don't need to have these copies, please remove or modify the
 custom build settings.

ACKNOWLEDGEMENT

I thank Mamiya and Kobarin and Nullsoft for their great source code.
I thank Norix and Izumi for the fruitful discussions and the NSFplug
users for their comments and bug reports.

COPYRIGHTS

NSFplug is built on KM6502, KbMediaPlayer plugin SDK and Winamp2
plugin SDK.

NSFplug uses KM6502 in emulating a 6502 cpu.  KM6502 code is stored in
devices\CPU\km6502 folder of this source archive. KM6502 is a public
domain software.  See the document of KM6502 stored in the folder.

KbMediaPlayer Plugin SDK is provided by Kobarin.  The SDK code is also
packed in the kbmedia\sdk folder of this archive.  The copyright of
the source code remains with Kobarin.

The files in winamp/sdk folder of this archive are the header files
from Winamp2 Plugin SDK provided by Nullsoft. The copyright of these
header files remains with Nullsoft.

CONTACT

Digital Sound Antiques
http://dsa.sakura.ne.jp/
