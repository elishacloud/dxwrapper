# DxWrapper
### Introduction
DxWrapper is a dll file designed to wrap or hook DirectX (or other) files to fix compatibility issues in older games.  This project is primarily targeted to fixing compatibility issues with running games on Windows 10, ideally by simply dropping a dll file and ini file into the game folder.  A secondary purpose for this is to offer a single tool (one stop shop, so to speak) by combining several other projects into one.  So far this dll can wrap the following files: bcrypt.dll, cryptsp.dll, d3d8.dll, d3d9.dll, dciman32.dll, ddraw.dll, dinput.dll, dplayx.dll, dsound.dll, dxgi.dll, winmm.dll or winspool.drv

### Features
DxWrapper has a number of features that it includes.  Below is a list of features:

 - Integration of [DDrawCompat](https://github.com/narzoul/DDrawCompat/)
 - Can enable and disable DDrawCompat features to improve compatibility
 - Proxy for [DxWnd](https://sourceforge.net/projects/dxwnd/)
 - Convert Direct3D8 (d3d8.dll) to Direct3D9 (d3d9.dll) using [d3d8to9](https://github.com/crosire/d3d8to9)
 - Allows tweaking some Microsoft DirectSound function calls using [DSoundCtrl](http://www.bockholdt.com/dsc/)
 - Hot patch memory support (update game memory at run time, can be used to remove CD/DVD checks or resolve graphics or other glitches)
 - Set single CPU affinity (some games have issue with multi-core CPUs)
 - Disable display scaling on high DPI setting
 - Ability to set all 12 Application Compatibility Toolkit options for DXPrimaryEmulation using SetAppCompatData API
 - Set game window to full screen (some games have full screen issues)
 - Handle exceptions or crashes in games
 - Load custom dll files into the game
 - Run custom processes when the game starts up
 
### Compatibility List for Games on Windows 10
So far I have tested this with over 70 games (that don't otherwise work correctly) to get them running on Windows 10.  Most work flawlessly, some still have issues I need to resolve.  Check out the [Compatible Games wiki](https://github.com/elishacloud/dxwrapper/wiki/Compatible-Games) for a full list.

### Installation

1. Download the latest binary release from the repository's [Release](https://github.com/elishacloud/dxwrapper/releases) page and unzip it to a folder.
2. Determine which dll file is needed for the game.  This depends on many factors, I will add a page explaining this later.  Most commonly it is `ddraw.dll` or `winmm.dll`.
3. Copy this dll file and the `Settings.ini` file into the game's installation directory, next to the main executable file.  For some games the main executable is in a subdirectory (like 'Bin', 'Exe' or 'App') you will need to copy it into that directory.  It is not recommended to overwrite any existing files from the game.
4. Rename the `Settings.ini` to match the name of the dll file.  Example if you are using the `ddraw.dll` then you want to rename `Settings.ini` to `ddraw.ini`.
5. Open up the ini file in a text editor (like Notepad) and enable settings needed for the game.

Do not attempt to overwrite any dll in a Windows system directory, it is currently not supported and will not work.

### Uninstallation

Delete DxWrapper dll files and the ini files from the game's directory. You can also delete the log file, if there is one.

### Configuration

To configure DxWrapper you just need to edit the ini file and enable the settings you want.  See the [Configuration wiki](https://github.com/elishacloud/dxwrapper/wiki/Configuration) for more details.

### License
Copyright (C) 2017 Elisha Riedlinger

This software is provided 'as-is', without any express or implied warranty. In no event will the author(s) be held liable for any damages arising from the use of this software. Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

DxWrapper uses code from several other projects. So to give credit where credit is due, below is a list of locations that source code was taken from:

 - [DDrawCompat](https://github.com/narzoul/DDrawCompat/): Includes the full DDrawCompat version 2.0b and 2.1.
 - [DxWnd](https://sourceforge.net/projects/dxwnd/): Includes code from the proxy dll, single CPU affinity and exception handling. 
 - [OllyDbg](http://www.ollydbg.de/): Includes the full disasm.c (disasm.dll) code used for exception handling.
 - [d3d8to9](https://github.com/crosire/d3d8to9): Includes the full D3D8 to D3D9 code.
 - [DSoundCtrl](https://github.com/nRaecheR/DirectSoundControl): Includes the full DirectSoundControl code.
 - [Aqrit's ddwrapper](http://bitpatch.com/ddwrapper.html): Includes code for single CPU affinity, disable display scaling on high DPI and code to read the ini config file.

### Development
DxWrapper is written in C++ using Microsoft Visual Studio Community 2015.

Compilation depends on [Detours Express 3.0](http://research.microsoft.com/en-us/projects/detours/) and the standalone DirectX SDK, required for the D3DX headers and libraries used for disassembling and assembling the shaders. You will need to change the include and library paths as needed if you didn't install Detours or the DirectX SDK in the default directory.

Thanks for stopping by!