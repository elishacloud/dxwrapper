# DxWrapper
### Introduction
DxWrapper is a .dll file designed to wrap DirectX files to fix compatibility issues in older games.  This project is primarily targeted at fixing issues with running games on Windows 10, by simply dropping .dll and .ini files into the game folder.  Its secondary purpose is to offer a single tool combining several projects into one.

### Features
DxWrapper has many features including:

 - Integration of [DDrawCompat](https://github.com/narzoul/DDrawCompat/)
 - Configuring DDrawCompat options to improve compatibility
 - Proxy for [DxWnd](https://sourceforge.net/projects/dxwnd/)
 - Conversion of DirectDraw 1-6 to DirectDraw 7
 - Conversion of Direct3D 1-6 to Direct3D 7
 - Conversion of DirectDraw 1-7 (ddraw.dll) to Direct3D 9 (d3d9.dll) using [Dd7to9](https://github.com/elishacloud/dxwrapper/wiki/DirectDraw-to-Direct3D9-Conversion)
 - Conversion of Direct3D 8 (d3d8.dll) to Direct3D 9 (d3d9.dll) using [d3d8to9](https://github.com/crosire/d3d8to9)
 - Conversion of DirectInput 1-7 (dinput.dll) to DirectInput 8 (dinput8.dll) using [dinputto8](https://github.com/elishacloud/dinputto8)
 - Includes [LegacyD3DResolutionHack](https://github.com/UCyborg/LegacyD3DResolutionHack) to remove the artificial resolution limit from Direct3D 1-7, which allows DirectDraw games to play at 4K resolution
 - Tweaking of Microsoft DirectSound function calls using [DSoundCtrl](http://www.bockholdt.com/dsc/)
 - Can cause Direct3D 8 and Direct3D 9 games to run in windowed mode
 - Can disable Microsoft Game Explorer (GameUX) to prevent rundll32.exe high CPU
 - Can disable High DPI scaling in Windows
 - ASI loader to load custom libraries with the file extension .asi into game processes using [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader)
 - Supports being loaded as an ASI plug-in
 - Adding AntiAliasing support into DirectX 8 or 9 games even if the game does not support it (Windows 10 only)
 - Hot patch memory support (update game memory at run time, can be used to remove CD/DVD checks or resolve graphics or other glitches)
 - Set single CPU affinity (some games have issues with multi-core CPUs)
 - Disable display scaling on high DPI mode
 - Ability to set all 12 Application Compatibility Toolkit options for DXPrimaryEmulation using SetAppCompatData API
 - Set game window to fullscreen (some games have fullscreen issues)
 - Handling in-game exceptions or crashes
 - Load custom .dll files into games
 - Run custom processes at game start-up

### Compatibility List for Games on Windows 10
So far I have tested this with over 70 games (that don't otherwise work correctly) to get them running on Windows 10.  Most games will work, but some still have issues.  Check out the [Compatible Games wiki](https://github.com/elishacloud/dxwrapper/wiki/Compatible-Games) for a full list.

### Installation

1. Download the latest binary release from the repository's [Release](https://github.com/elishacloud/dxwrapper/releases) page and unzip it to a folder.
2. Determine which .dll file is needed for the game.  This depends on many factors which will be explained on page created later.  Most commonly it is `ddraw.dll`, `dsound.dll` or `winmm.dll`.
3. Copy this .dll file from the 'Stub' folder plus the `dxwrapper.dll` and `dxwrapper.ini` files into the game's installation directory, next to the main executable file.  For some games the main executable is in a subdirectory (like 'Bin', 'Exe' or 'App') so the files will need to be copied it into that directory.  Overwriting of any existing game files is not recommended.
4. Open up the `dxwrapper.ini` file in a text editor and enable the settings needed for the game.

Do not attempt to overwrite any .dll in a Windows system directory as it is currently not supported and will not work.

### Uninstallation

Delete the DxWrapper .dll and .ini files from the game's directory. You can also delete the log file, if there is one.

### Configuration

To configure DxWrapper, edit the .ini file and enable the settings wanted.  See the [Configuration wiki](https://github.com/elishacloud/dxwrapper/wiki/Configuration) for more details.

Sample configuration file: [dxwrapper.ini](https://github.com/elishacloud/dxwrapper/blob/master/Settings/Settings.ini).

List of all configuration options: [allsettings.ini](https://github.com/elishacloud/dxwrapper/blob/master/Settings/AllSettings.ini)

### Supported DLLs

DxWrapper can wrap the following dlls:
 - bcrypt.dll
 - cryptsp.dll
 - d2d1.dll
 - d3d8.dll
 - d3d9.dll
 - d3d10.dll
 - d3d11.dll
 - d3d12.dll
 - dciman32.dll
 - ddraw.dll
 - dinput.dll
 - dinput8.dll
 - dplayx.dll
 - dsound.dll
 - msacm32.dll
 - msvfw32.dll
 - version.dll
 - wininet.dll
 - winmm.dll
 - winmmbase.dll

### Donations

All my work here is free and can be freely used.  For more details on how you can use this module see the [license](#license) section below.  However, if you would like to donate then check out my [donations page](https://PayPal.me/elishacloud).  All donations are for work already completed!  Please don't donate for future work or to try and increase my development speed.  Thanks!

### License
Copyright (C) 2019 Elisha Riedlinger

This software is provided 'as-is', without any express or implied warranty. In no event will the author(s) be held liable for any damages arising from the use of this software. Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

### Credits
DxWrapper uses code from several other projects. So to give credit where credit is due, below is a list of locations that source code was taken from:

 - [AppCompatData](http://www.mojolabs.nz/posts.php?topic=99477): Used comments from blitzbasic.com to build the feature to configure the DirectDraw AppCompatData settings.
 - [Aqrit's ddwrapper](http://bitpatch.com/ddwrapper.html): Includes code to read the ini config file.
 - [d3d8to9](https://github.com/crosire/d3d8to9): Includes the full Direct3D 8 to Direct3D 9 code.
 - [D3DParseUnknownCommand](https://doxygen.reactos.org/d3/d02/dll_2directx_2ddraw_2main_8c.html#af9a1eb1ced046770ad6f79838cc8517d): Includes code for DirectDraw D3DParseUnknownCommand from source code found in ReactOS.
 - [GetPPID](https://gist.github.com/mattn/253013/d47b90159cf8ffa4d92448614b748aa1d235ebe4): Includes Code taken from [mattn](https://gist.github.com/mattn) GitHub project to get the parent process PID.
 - [DDrawCompat](https://github.com/narzoul/DDrawCompat/): Includes the full DDrawCompat v0.2.0b, v0.2.1 and Experimental version.
 - [diablo-ddrawwrapper](https://github.com/strangebytes/diablo-ddrawwrapper): Includes some code for converting ddraw to Direct3D.
 - [DSoundCtrl](https://github.com/nRaecheR/DirectSoundControl): Includes the full DirectSoundControl code.
 - [DxWnd](https://sourceforge.net/projects/dxwnd/): Includes code from DxWnd for API hooking, DxWnd proxy loading (init.cpp) and exception handling.
 - [GetComputerManufacturer](http://www.rohitab.com/discuss/topic/35915-win32-api-to-get-system-information/): Includes code from rohitab.com to get the computer vendor and model.
 - [GetFileVersionInfo ](https://stackoverflow.com/a/940743): Includes code from stackoverflow.com for getting the version of a PE file.
 - [GetFunctionAddress](http://www.rohitab.com/discuss/topic/40594-parsing-pe-export-table/): Includes code from rohitab.com to parse the PE export table.
 - [LegacyD3DResolutionHack](https://github.com/UCyborg/LegacyD3DResolutionHack): Includes code from LegacyD3DResolutionHack to removes the artificial resolution limit from Direct3D 7 and below.
 - [MemoryModule](https://github.com/fancycode/MemoryModule): Includes code for loading libraries from memory.
 - [OllyDbg](http://www.ollydbg.de/): Includes the full cmdlist.c and disasm.c (disasm.dll) code used for exception handling.
 - [PixelShaderPalette](https://github.com/CnCNet/cnc-ddraw): Includes code from ddraw-cnc for a PixelShader that is used to view DirectDraw palette surfaces properly.
 - [SilentPatchFarCry](https://github.com/CookiePLMonster/SilentPatchFarCry): Includes code from SilentPatchFarCry to cache the clip plane in Direct3D 9.
 - [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader): Includes code for loading ASI plugins.
 - [WineD3D d3d8.dll](https://github.com/alexhenrie/wine/tree/master/dlls/d3d8): Includes functions ValidatePixelShader and ValidateVertexShader.

### Development
DxWrapper is written mostly in C++ using Microsoft Visual Studio Community 2017.

The project requires both the Windows 10 SDK and WDK. The exact version required can be seen in the project properties in Visual Studio.

Thanks for stopping by!
