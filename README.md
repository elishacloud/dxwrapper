### 🚀 Support the Project! 🌟

Your support fuels development of the `dd7to9` functionality to enable older games to run smoothly on current platforms. Whether it's a one-time boost via PayPal or ongoing monthly support on Patreon, your contribution ensures continued improvements and commitment to the growth of the project. Please note, your support is invaluable, but it won't influence specific game prioritization. Join us on this journey!

Support via [PayPal](https://paypal.me/elishacloud) | [Patreon](https://patreon.com/dxwrapper)

# DxWrapper
### Introduction
DxWrapper is a .dll file designed to wrap DirectX files to fix compatibility issues in older games.  This project is primarily targeted at fixing issues with running games on Windows 10/11, by simply dropping .dll and .ini files into the game folder.  Its secondary purpose is to offer a single tool combining several projects into one.

### Features

DxWrapper includes a wide range of features:

#### 🎮 Major Graphics Conversion and Enhancement
- **DirectDraw/Direct3D 1–7 to Direct3D 9** conversion via [Dd7to9](https://github.com/elishacloud/dxwrapper/wiki/DirectDraw-to-Direct3D9-Conversion)
- **Direct3D 8 to Direct3D 9** conversion via [d3d8to9](https://github.com/crosire/d3d8to9)
- **Direct3D 9 hooking** to intercept and enhance graphics calls
- **DirectInput 1–7 to DirectInput 8** conversion via [dinputto8](https://github.com/elishacloud/dinputto8)
- **DirectSound hooking** (`dsound.dll`), similar to [DSoundCtrl](http://www.bockholdt.com/dsc/), to enhance or fix audio
- **[DDrawCompat](https://github.com/narzoul/DDrawCompat) integration** (v2.0, 2.1, 3.2) for improved compatibility with configurable options

#### 🖥️ Resolution and Renderer Enhancements
- **Resolution unlock** for Direct3D 1–7 using [LegacyD3DResolutionHack](https://github.com/UCyborg/LegacyD3DResolutionHack) — enables 4K and beyond
- **Direct3D 9 to Direct3D9Ex** conversion for enhanced rendering performance
- **Direct3D 9 to Direct3D 12** conversion via [D3D9On12](https://github.com/microsoft/D3D9On12)

#### 🛠️ Rendering and Compatibility Fixes
- **Clip plane caching** to fix issues in D3D9 games ([Far Cry example](https://houssemnasri.github.io/2018/07/07/farcry-d3d9-bug/))
- **Environment cube map fixes** for broken textures ([issue example](https://github.com/elishacloud/dxwrapper/issues/136))
- **Vertex processing mode override** to fix performance issues
- **Scanline removal** from games that display them
- **Interlacing removal** to improve visual quality

#### 🧩 Rendering Enhancements (Forced Features)
- **Force anti-aliasing** support in games that don't support it natively
- **Force anisotropic filtering** in unsupported games
- **Force vertical sync (VSync)** on or off
- **Force windowed mode** in fullscreen-only games
- **Force use of discrete GPU** (dGPU) via `GraphicsHybridAdapter` call
- **Force single Begin/EndScene pair** per Present call (per Microsoft documentation)

#### 🎛️ Frame and Timing Control
- **FPS limiter** to prevent games from running too fast
- **Performance counter patching** to fake uptime < 1 day (fixes long-uptime issues)
- **Single CPU affinity** setting for multi-core compatibility issues
- **Application Compatibility Toolkit settings override** for `DXPrimaryEmulation`:
  - Includes: `LockEmulation`, `BltEmulation`, `ForceLockNoWindow`, `ForceBltNoWindow`, `LockColorkey`, `FullscreenWithDWM`, `DisableLockEmulation`, `EnableOverlays`, `DisableSurfaceLocks`, `RedirectPrimarySurfBlts`, `StripBorderStyle`, `DisableMaxWindowedMode`

#### 📐 Compatibility and GDI Fixes
- **GDI and DirectDraw mixing support** to improve 2D compatibility
- **Pitch lock fix** for games with misaligned surfaces
- **Disable Fullscreen Optimizations (MaximizedWindowedMode)** to resolve performance/stability issues
- **Disable High DPI scaling** to fix UI scaling issues
- **Disable Microsoft Game Explorer (GameUX)** to stop `rundll32.exe` CPU spikes
- **Disable audio clipping** to eliminate pops and clicks during playback

#### 🔧 Advanced Customization and Modding
- **Hot-patch memory** in real time (e.g., remove CD checks or fix bugs)
- **Set Windows version** seen by game (helps with OS compatibility)
- **Handle in-game crashes** by patching problematic instructions (nop'ing offending code)
- **Launch custom processes** when the game starts
- **Load custom `.dll` files** into game processes
- **ASI loader** to inject `.asi` plug-ins ([Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader) compatible)
- **DxWrapper as an ASI plug-in** (can be loaded by other ASI loaders)

#### 🧩 Miscellaneous Fixes and Tweaks
- **Set game window to fullscreen** (if native fullscreen fails)
- **Show FPS counter** in-game
- **Filter input** when the game window loses focus (prevents input when other windows are active)
- **Various compatibility flags and tweaks**, including:
  - `DdrawEmulateSurface`, `DdrawEmulateLock`, `DdrawKeepAllInterfaceCache`, `DdrawLimitTextureFormats`, `DdrawLimitDisplayModeCount`, `LimitStateBlocks`, `SetPOW2Caps`

### Compatibility List for Games on Windows 10/11
So far I have tested this with hundreds of games (many of which  don't otherwise work correctly) to get them running on Windows 10/11.  Most games will work, but some still have issues.  Check out the [Compatible Games wiki](https://github.com/elishacloud/dxwrapper/wiki/Compatible-Games) and the [Dd7to9 Supported Games wiki](https://github.com/elishacloud/dxwrapper/wiki/DirectDraw-to-Direct3D9-Conversion#supported-2d-games-directdraw) for a list.

### Installation

1. Download the latest binary release from the repository's [Release](https://github.com/elishacloud/dxwrapper/releases) page and unzip it to a folder.
2. Determine which stub .dll file is needed for the game.  This depends on many factors which will be explained on page created later.  Common stub dll's to use are `ddraw.dll`, `d3d8.dll`, `d3d9.dll`, `dsound.dll` or `winmm.dll`.  You only need to choose one stub file to load dxwrapper into the game.
3. Copy this .dll file from the 'Stub' folder plus the `dxwrapper.dll` and `dxwrapper.ini` files into the game's installation directory, next to the main executable file.  For some games the main executable is in a subdirectory (like 'Bin', 'Exe' or 'App') so the files will need to be copied it into that directory.  Overwriting of any existing game files is not recommended.
4. Open up the `dxwrapper.ini` file in a text editor and enable the settings needed for the game.

Do not attempt to overwrite any .dll in a Windows system directory as it is currently not supported and will not work.

### Uninstallation

Delete the DxWrapper .dll and .ini files from the game's directory. You can also delete the log file, if there is one.

### Configuration

To configure DxWrapper, edit the .ini file and enable the settings wanted.  See the [Configuration wiki](https://github.com/elishacloud/dxwrapper/wiki/Configuration) for more details.

Sample configuration file: [dxwrapper.ini](https://github.com/elishacloud/dxwrapper/blob/master/Settings/Settings.ini).

List of all configuration options: [allsettings.ini](https://github.com/elishacloud/dxwrapper/blob/master/Settings/AllSettings.ini)

### Logging
The log file will be created in the same folder where the game executable is located.  It will be named 'dxwrapper' with the name of the game executable appended to it.  So if you are running the file `game.exe` then the log file will be called `dxwrapper-game.log`.

### Supported DLLs

DxWrapper can wrap the following dlls:
 - bcrypt.dll
 - cryptbase.dll
 - cryptsp.dll
 - d2d1.dll
 - d3d8.dll
 - d3d9.dll
 - dciman32.dll
 - ddraw.dll
 - dinput.dll
 - dinput8.dll
 - dplayx.dll
 - dsound.dll
 - dwmapi.dll
 - msacm32.dll
 - msvfw32.dll
 - version.dll
 - wininet.dll
 - winmm.dll
 - winmmbase.dll
 - winspool.drv
 - wsock32.dll

### License

Copyright (C) 2025 Elisha Riedlinger

This software is provided 'as-is', without any express or implied warranty. In no event will the author(s) be held liable for any damages arising from the use of this software. Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.  
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.  
3. This notice may not be removed or altered from any source distribution.  

### Third-Party Licenses

#### d3d8to9 by Patrick Mours

Portions of this project make use of code from the [d3d8to9](https://github.com/crosire/d3d8to9) project by Patrick Mours, which is licensed as follows:

> Copyright (C) 2015 Patrick Mours.  
> All rights reserved.  
>
> Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:  
>
> - Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.  
> - Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.  
>
> THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#### Detours & DirectXMath by Microsoft

Portions of this project make use of code from the [detours](https://github.com/microsoft/Detours) and [DirectXMath](https://github.com/microsoft/DirectXMath) projects by Microsoft, which is licensed as follows:

> Copyright (c) Microsoft Corporation.
>
> MIT License
>
> Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
>
> The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
>
> THE SOFTWARE IS PROVIDED *AS IS*, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#### Hooking.Patterns by ThirteenAG

Portions of this project make use of code from the [Hooking.Patterns](https://github.com/ThirteenAG/Hooking.Patterns) project by ThirteenAG, which is licensed as follows:

> Copyright (c) 2014 Bas Timmer/NTAuthority et al.
>
> Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
>
> The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
>
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#### imgui by ocornut

Portions of this project make use of code from the [imgui](https://github.com/ocornut/imgui) project by ocornut, which is licensed as follows:

> The MIT License (MIT)
>
> Copyright (c) 2014-2025 Omar Cornut
>
> Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
>
> The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
>
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#### MemoryModule by fancycode

Portions of this project make use of code from the [MemoryModule](https://github.com/fancycode/MemoryModule) project by fancycode, which is licensed as follows:

> Mozilla Public License Version 2.0
>
> For details see here: [LICENSE.txt](https://github.com/fancycode/MemoryModule/blob/master/LICENSE.txt)

### Credits
DxWrapper uses code from several other projects. So to give credit where credit is due, below is a list of locations that source code was taken from:

 - [AppCompatData](http://www.mojolabs.nz/posts.php?topic=99477): Used comments from blitzbasic.com to build the feature to configure the DirectDraw AppCompatData settings.
 - [d3d8to9](https://github.com/crosire/d3d8to9): Includes the full Direct3D 8 to Direct3D 9 code.
 - [DDrawCompat](https://github.com/narzoul/DDrawCompat/): Includes the full DDrawCompat v0.2.0b, v0.2.1 and v0.3.2 and for API hooking.
 - [detours](https://github.com/microsoft/Detours): Includes the Microsoft's detours.
 - [DirectXMath](https://github.com/microsoft/DirectXMath): Includes the Microsoft's DirectXMath.
 - [Hooking.Patterns](https://github.com/ThirteenAG/Hooking.Patterns): Includes code from ThirteenAG's Hooking.Patterns.
 - [imgui](https://github.com/ocornut/imgui): Includes the imgui code from ocornut.
 - [LegacyD3DResolutionHack](https://github.com/UCyborg/LegacyD3DResolutionHack): Includes code from LegacyD3DResolutionHack to removes the artificial resolution limit from Direct3D 7 and below.
 - [MemoryModule](https://github.com/fancycode/MemoryModule): Includes code for loading libraries from memory.

### Development
DxWrapper is written mostly in C++ using Microsoft Visual Studio 2022.

The project requires both the Windows 10 SDK and WDK (needs to have matching SDK and WDK versions installed). The exact version required can be seen in the project properties in Visual Studio.

GitHub Link: https://github.com/elishacloud/dxwrapper

Thanks for stopping by!
