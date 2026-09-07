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


## 🌐 Web Resources & Verified Articles Directory
- [GROW A GARDEN FOR BRAINROTS](https://planetejeux-france.pages.dev/grow-a-garden-for-brainrots.html)
- [DYNAMONS 8](https://unblocked-galaxy.web.app/dynamons-8.html)
- [BATTLEDUDES IO](https://pixelarcade-speed.web.app/battledudes-io.html)
- [TUNG SAHUR COLORING](https://pixelarcade-speed.web.app/tung-sahur-coloring.html)
- [ESCAPE ROOM MYSTERY KEY](https://koreagame-hub24.netlify.app/escape-room-mystery-key.html)
- [SQUID CANDY CHALLENGE](https://jeuxflash-france.netlify.app/squid-candy-challenge.html)
- [SHIPBUILDING TYCOON](https://veb-igry-moskva.web.app/shipbuilding-tycoon.html)
- [MATH STARS](https://jeuxflash-france.netlify.app/math-stars.html)
- [MAHJONG CLASSIC WEBGL](https://koreagame-webhub.github.io/mahjong-classic-webgl.html)
- [ARMY FIGHT 3D](https://hindigames-portal.netlify.app/army-fight-3d.html)
- [TIMBERLAND ARRANGE PUZZLE GAME](https://youxi-h5-tiandi.pages.dev/timberland-arrange-puzzle-game.html)
- [KINGS ROD](https://fischmarket-live.pages.dev/calculator/kings-rod)
- [MUSCLE UP MASTER](https://onlinerus-games.netlify.app/muscle-up-master.html)
- [T REX](https://fruitvalues-app.pages.dev/calculator/t-rex)
- [BUBBLE UP](https://luchshie-igry-rus.pages.dev/bubble-up.html)
- [BOLTS AND NUTS](https://mir-igr-onlayn.pages.dev/bolts-and-nuts.html)
- [MUSHROOM BLOCKS](https://dautruong-game24h.web.app/mushroom-blocks.html)
- [LIQUID SORT DELUXE](https://gemu-hiroba-japan.web.app/liquid-sort-deluxe.html)
- [FOOTBALL HEADS 2025](https://youxiweb-china.github.io/football-heads-2025.html)
- [BUS ESCAPE CLEAR JAM](https://retro-arcade-zone.netlify.app/bus-escape-clear-jam.html)
- [POWER LIGHT](https://hindigame-arena.vercel.app/power-light.html)
- [GRAND MAHJONG CONNECT](https://mundodosjogos-br.web.app/grand-mahjong-connect.html)
- [LABUBU DOLL MUKBANG ASMR UNBLOCKED](https://muryo-geim-nara.web.app/labubu-doll-mukbang-asmr-unblocked.html)
- [SPRING TILE MASTER](https://retro-arcade-zone.netlify.app/spring-tile-master.html)
- [CAPYBARA SCREW JAM](https://mir-igr-onlayn.pages.dev/capybara-screw-jam.html)
- [POLICE CHASE DRIFTER](https://arcadegames-france24.web.app/police-chase-drifter.html)
- [ZOMBIE HORDE BUILD SURVIVE](https://action-strike-zone.pages.dev/zombie-horde-build-survive.html)
- [DRAW WEAPON FIGHT PARTY](https://juegosmundial-hoy.pages.dev/draw-weapon-fight-party.html)
- [FRUIT CONNECT 3](https://speed-racing-hub.netlify.app/fruit-connect-3.html)
- [STICK HERO BATTLE](https://shanghai-youxi-web.web.app/stick-hero-battle.html)
- [TWO SUPRA DRIFTERS](https://peullaesi-geim-madang.web.app/two-supra-drifters.html)
- [LABUBA MERGE](https://logic-puzzle-world.pages.dev/labuba-merge.html)
- [BALL DUNK FALL](https://congdonggame-vietnam.web.app/ball-dunk-fall.html)
- [DART HERO](https://webarcade-hub.github.io/dart-hero.html)
- [LABUBA MERGE](https://jogosweb-brasil.github.io/labuba-merge.html)
- [MAKEUP TRENDS THEN AND NOW](https://kuaile-youxi-hub.web.app/makeup-trends-then-and-now.html)
- [GUN WAR Z1](https://koreagame-webhub.github.io/gun-war-z1.html)
- [BOLTS AND NUTS SORTING](https://jingpin-youxiwang.pages.dev/bolts-and-nuts-sorting.html)
- [MERGE RUN BATTLE](https://congdonggame-vietnam.web.app/merge-run-battle.html)
- [POLICE STATION](https://koreagame-hub24.netlify.app/police-station.html)
- [DRIVER MASTER SIMULATOR](https://jeuxflash-france.netlify.app/driver-master-simulator.html)
- [SORT BALLS CONES](https://tokyo-arcade-web.pages.dev/sort-balls-cones.html)
- [MEATRIDER](https://neon-cyber-arcade.pages.dev/meatrider.html)
- [FLIGHT SIM AIR TRAFFIC CONTROL](https://hindigames-hub.netlify.app/flight-sim-air-traffic-control.html)
- [BURGER EMPIRE](https://arcadevault-gamehub.github.io/burger-empire.html)
- [REAL STREET FIGHTER 3D](https://juegosgratis-es.netlify.app/real-street-fighter-3d.html)
- [SUPER SNIPER MISSIONS](https://PixelArcadezGame.github.io/super-sniper-missions.html)
- [BUSY BEE HIVE](https://unblocked-galaxy.web.app/busy-bee-hive.html)
- [EXIT PUZZLE](https://pixelarcade-speed.web.app/exit-puzzle.html)
- [PUZZLE MASTERS TRAVELERS](https://speed-racing-hub.netlify.app/puzzle-masters-travelers.html)
- [POP THE BUBBLE](https://arcadegames-france24.web.app/pop-the-bubble.html)
- [FILLWORDS FIND ALL THE WORDS](https://unblocked-action-arena.netlify.app/fillwords-find-all-the-words.html)
- [KITSUNE](https://blox-trade-fairness.pages.dev/values/kitsune)
- [ORGANIZER MASTER](https://luchshie-igry-rus.pages.dev/organizer-master.html)
- [OBBY FOOTBALL SOCCER 3D](https://youxi-h5-tiandi.pages.dev/obby-football-soccer-3d.html)
- [STICKMAN THE FLASH](https://unblocked-galaxy.web.app/stickman-the-flash.html)
- [CRAZY 2248 LINK MATCHING PUZZLE GAME](https://gemu-hiroba-japan.web.app/crazy-2248-link-matching-puzzle-game.html)
- [BASKETBALL LIFE 3D](https://veb-igry-moskva.web.app/basketball-life-3d.html)
- [MAHJONG STACK](https://muryo-geim-nara.web.app/mahjong-stack.html)
- [FILL GLASS](https://logic-puzzle-world.pages.dev/fill-glass.html)
- [OFFICE GOLF](https://unblocked-galaxy-hub.pages.dev/office-golf.html)
- [LEOPARD](https://bfvalues-pro.pages.dev/calculator/leopard)
- [STICKMAN GUN SHOOTER](https://nihon-webgames.netlify.app/stickman-gun-shooter.html)
- [MONSTER DUELIST](https://arcadevault-games.github.io/monster-duelist.html)
- [GEOMETRY LITE](https://mir-igr-onlayn.pages.dev/geometry-lite.html)
- [CRAZY STUNTS 3D](https://youxiweb-china.github.io/crazy-stunts-3d.html)
- [CRYPTOGRAM WORD BRAIN PUZZLE](https://speed-racing-hub.netlify.app/cryptogram-word-brain-puzzle.html)
- [NUBIK IN THE MONSTER WORLD](https://webarcade-gamehub.github.io/nubik-in-the-monster-world.html)
- [FARM OF WORDS](https://arcadevault-gamehub.github.io/farm-of-words.html)
- [PORTAL](https://fruit-calculator-2026.netlify.app/values/portal)
- [SAFARI STORY MAHJONG](https://jogosweb-brasil.github.io/safari-story-mahjong.html)
- [BUBBLE SHOOTER PIRATE TREASURES](https://jogosonline-brasil.vercel.app/bubble-shooter-pirate-treasures.html)
- [BEAT MUSIC BATTLE](https://logic-puzzle-world.pages.dev/beat-music-battle.html)
- [OBBY MODES ONLINE MINI GAMES](https://logic-puzzle-world.pages.dev/obby-modes-online-mini-games.html)
- [MLG AIRHORN](https://soundbox-arcade.onrender.com/sound/mlg-airhorn.html)
- [SNOWFLIGHT](https://koreagame-arcade.netlify.app/snowflight.html)
- [TOY RUMBLE 3D](https://bharat-game-zone.web.app/toy-rumble-3d.html)
- [JIXORA JIGSAW SOLITAIRE PUZZLE](https://shadow-ninja-arena.web.app/jixora-jigsaw-solitaire-puzzle.html)
- [MOJICON SPRING CONNECT](https://nihongames-portal.netlify.app/mojicon-spring-connect.html)
- [MAHJONG SOLITAIRE ZODIAC](https://webarcade-hub.github.io/mahjong-solitaire-zodiac.html)
- [MEGA FALL RAGDOLL SIMULATOR](https://koreagame-arcade.netlify.app/mega-fall-ragdoll-simulator.html)
- [MEGA RAMP CAR](https://zona-juegos-flash.web.app/mega-ramp-car.html)
- [SPRUNKI MINI GAMES](https://bharat-game-zone.web.app/sprunki-mini-games.html)
- [WORD HUNT](https://unblocked-galaxy.web.app/word-hunt.html)
- [ANTS PARTY](https://bharat-game-zone.web.app/ants-party.html)
- [ANGRY SNAKE IO](https://turbodrift-zone.web.app/angry-snake-io.html)
- [RUSSIAN FISHING AT SEA](https://unblocked-galaxy.github.io/russian-fishing-at-sea.html)
- [CITY BIKE RACING CHAMPION](https://unblocked-galaxy.web.app/city-bike-racing-champion.html)
- [SURVIVAL IN AREA 51](https://luchshie-igry-rus.pages.dev/survival-in-area-51.html)
- [PUZZLE BOX BRAIN FUN](https://dautruong-game24h.web.app/puzzle-box-brain-fun.html)
- [CHINESE FOOD CHEF DUDU](https://gemu-hiroba-japan.web.app/chinese-food-chef-dudu.html)
- [CAFE OWNER BUSINESS SIMULATOR](https://shadow-ninja-arena.web.app/cafe-owner-business-simulator.html)
- [ROBYBOX SPACE STATION WAREHOUSE](https://shadow-ninja-arena.web.app/robybox-space-station-warehouse.html)
- [NUTS STACK SORT NUTS BOLTS](https://dautruong-game24h.web.app/nuts-stack-sort-nuts-bolts.html)
- [TANK SNIPER 3D](https://veb-igry-moskva.web.app/tank-sniper-3d.html)
- [CELEBRITY FACE DANCE](https://logic-puzzle-world.pages.dev/celebrity-face-dance.html)
- [STICK ROPE HERO](https://juegosgratis-es.netlify.app/stick-rope-hero.html)
- [RUMBLE](https://bfvalues-central.pages.dev/calculator/rumble)
- [OVER THE RAINBOW](https://neon-cyber-arcade.pages.dev/over-the-rainbow.html)
- [ZOMBIE ROAD SHOOTER WITH DESTRUCTION](https://kuaile-youxi-hub.web.app/zombie-road-shooter-with-destruction.html)
- [STICK NINJA SURVIVAL](https://koreagame-zone.vercel.app/stick-ninja-survival.html)
- [XYTRIAN RUNNER](https://jogosweb-brasil.github.io/xytrian-runner.html)
- [SPIN SPIN](https://nihongames-web.github.io/spin-spin.html)
- [3D BALL BALANCER](https://logic-puzzle-world.pages.dev/3d-ball-balancer.html)
- [OBBY WITH FRIENDS DRAW AND JUMP](https://jeuxflash-france.netlify.app/obby-with-friends-draw-and-jump.html)
- [MY CITY HOSPITAL](https://retro-arcade-zone.netlify.app/my-city-hospital.html)
- [STUNT CAR EXTREME 2](https://jeuxflash-france.netlify.app/stunt-car-extreme-2.html)
- [LEVEL EATEN](https://jeuxflash-france.netlify.app/level-eaten.html)
- [IDLE POP MERGE](https://youxi-h5-tiandi.pages.dev/idle-pop-merge.html)
- [SNIPER WARS FIND THE CRIMINAL](https://onlinerus-games.netlify.app/sniper-wars-find-the-criminal.html)
- [BUILD AND RUN](https://geim-cheon-guk24.pages.dev/build-and-run.html)
- [ULTIMATE YATZY](https://jogosweb-brasil24.netlify.app/ultimate-yatzy.html)
- [DRAW BRIDGE PUZZLE](https://dautruong-game24h.web.app/draw-bridge-puzzle.html)
- [SANTA VS SKRITCH](https://trochoimienphi24h.github.io/santa-vs-skritch.html)
- [MATCH MASTERS](https://zona-juegos-flash.web.app/match-masters.html)
- [BRUH SOUND EFFECT](https://instantsounds-pixel.pages.dev/sound/bruh-sound-effect.html)
- [LARRY WORLD](https://hindigames-hub.netlify.app/larry-world.html)
- [DRIVE IN CINEMA IDLE GAME](https://nihongames-web.github.io/drive-in-cinema-idle-game.html)
- [2048 MATCH BALLS](https://webarcade-gamehub.github.io/2048-match-balls.html)
- [BACK 2 SCHOOL MAKEOVER](https://francejeux-online.web.app/back-2-school-makeover.html)
- [COIN MERGE](https://quantum-puzzle-hub.pages.dev/coin-merge.html)
- [BALL EATING SIMULATOR](https://youxi-china24.netlify.app/ball-eating-simulator.html)
- [COLOR BLOCK JAM](https://youxi-h5-tiandi.pages.dev/color-block-jam.html)
- [BOXTERIA](https://PixelArcadezGame.github.io/boxteria.html)
- [GET READY WITH ME CONCERT DAY](https://hindigame-arena.vercel.app/get-ready-with-me-concert-day.html)
- [PANDA KITCHEN IDLE TYCOON](https://juegosweb-gratis.github.io/panda-kitchen-idle-tycoon.html)
- [GOKARTS IO](https://hindigames-hub.netlify.app/gokarts-io.html)
- [QUIZ SQUID ROUND](https://mundodosjogos-br.web.app/quiz-squid-round.html)
- [SPOOKY HALLOWEEN HIDDEN PUMPKIN](https://quantum-puzzle-hub.pages.dev/spooky-halloween-hidden-pumpkin.html)
- [LABUBU ADVENTURE](https://unblocked-galaxy.github.io/labubu-adventure.html)
- [SLINGSHOT CHICKEN](https://shadow-ninja-arena.web.app/slingshot-chicken.html)
- [CLAW MERGE LABUBU DROP](https://neon-cyber-arcade.pages.dev/claw-merge-labubu-drop.html)
- [MINI GAMES CASUAL COLLECTION](https://youxi-china24.netlify.app/mini-games-casual-collection.html)
- [COOL ORANGE BALL BOUNCE ADVENTURE](https://arcadegames-france24.web.app/cool-orange-ball-bounce-adventure.html)
- [PUSH IT 3D](https://hindigames-portal.netlify.app/push-it-3d.html)
- [KIKI WORLD KAWAII DOLL DECOR](https://quantum-puzzle-hub.pages.dev/kiki-world-kawaii-doll-decor.html)
- [CAT VS KRIPOTIANS](https://juegosmundial-hoy.pages.dev/cat-vs-kripotians.html)
- [GEAR WARS](https://nihon-webgames.netlify.app/gear-wars.html)
- [SOCCER SNAKES](https://hindigames-hub.netlify.app/soccer-snakes.html)
- [KITSUNE](https://tradeblox-gg.pages.dev/values/kitsune)
- [MYSTICAL BLADE 3D](https://arcadevault-gamehub.github.io/mystical-blade-3d.html)
- [BLOCK CRASHER](https://retro-arcade-zone.netlify.app/block-crasher.html)
- [MAX MIXED COCKTAILS](https://trochoimienphi24h.github.io/max-mixed-cocktails.html)
- [GOAL RUSH](https://tokyo-arcade-web.pages.dev/goal-rush.html)
- [LIFE CLICKER](https://hindigame-arena.vercel.app/life-clicker.html)
- [WEAPONS AND RAGDOLLS](https://arcadevault-games.github.io/weapons-and-ragdolls.html)
- [ARCHERS RANDOM](https://shadow-ninja-arena.web.app/archers-random.html)
- [TAP OUT PUZZLE](https://kuaile-youxi-hub.web.app/tap-out-puzzle.html)
- [HIDDEN OBJECT ADVENTURE](https://gamehay-online.netlify.app/hidden-object-adventure.html)
- [BRAINROT MEMORY](https://jeuxweb-france.netlify.app/brainrot-memory.html)
- [ZOMBIES BATTLE FOR SURVIVAL](https://youxi-china24.netlify.app/zombies-battle-for-survival.html)
- [MONSTER SCHOOL 2](https://unblocked-galaxy.github.io/monster-school-2.html)
- [NUTS STACK SORT NUTS BOLTS](https://arcadegames-france24.web.app/nuts-stack-sort-nuts-bolts.html)
- [ALPHABET LORE MAZE](https://peullaesi-geim-madang.web.app/alphabet-lore-maze.html)
- [SNIPER SHOT SECRET MISSION](https://jogosweb-brasil.github.io/sniper-shot-secret-mission.html)
- [JELLY MONSTERS LINK PUZZLE](https://unblocked-galaxy-hub.pages.dev/jelly-monsters-link-puzzle.html)
- [FASHIONISTA AVATAR STUDIO DRESS UP](https://desi-gaming-arena.pages.dev/fashionista-avatar-studio-dress-up.html)
- [BATTLE RACING STARS](https://peullaesi-geim-madang.web.app/battle-racing-stars.html)
- [DRAGON ESCAPE](https://bharat-game-zone.web.app/dragon-escape.html)
- [STUPIDITY TEST](https://francejeux-online.web.app/stupidity-test.html)
- [VEHICLE FUN RACE](https://francejeux-online.web.app/vehicle-fun-race.html)
- [SHADOW](https://blox-trade-fairness.pages.dev/calculator/shadow)
- [PATH ICE](https://hindigames-portal.netlify.app/path-ice.html)
- [SECRET GALAXY MATCH THREE](https://turbodrift-zone.web.app/secret-galaxy-match-three.html)
- [STACKTRIS 2048](https://espacejeux-paris.pages.dev/stacktris-2048.html)
- [STICKMAN DUO ESCAPE THE TOMB](https://francejeux-online.web.app/stickman-duo-escape-the-tomb.html)
- [CRAZY ZOO SWIPE MATCH 3 PUZZLE GAME](https://juegosmundial-hoy.pages.dev/crazy-zoo-swipe-match-3-puzzle-game.html)
- [SHINE SEEK](https://pixelarcadezgame.web.app/shine-seek.html)
- [LIGHT LINE](https://mir-igr-onlayn.pages.dev/light-line.html)
- [THE STONE MINER](https://muryo-geim-nara.web.app/the-stone-miner.html)
- [MERGE 2048 CAKE](https://shanghai-youxi-web.web.app/merge-2048-cake.html)
- [NOOB LEGENDS DUNGEON ADVENTURES](https://juegosweb-desbloqueados.vercel.app/noob-legends-dungeon-adventures.html)
- [CUTE RABBITS CHALLENGING ADVENTURE](https://youxi-h5-tiandi.pages.dev/cute-rabbits-challenging-adventure.html)
- [BACK 2 SCHOOL MAKEOVER](https://hindigames-hub.netlify.app/back-2-school-makeover.html)
- [CHICKEN BANANA QUEST](https://kuaile-youxi-hub.web.app/chicken-banana-quest.html)
- [RACING PINBALL](https://trochoimienphi24h.github.io/racing-pinball.html)
- [KITTY SQUAD WINTER DRESS UP](https://shadow-ninja-arena.web.app/kitty-squad-winter-dress-up.html)
- [BUBBLE SHOOTER WONDERS OF EGYPT](https://juegosgratis-es.netlify.app/bubble-shooter-wonders-of-egypt.html)
- [BASKET SHOT](https://neon-cyber-arcade.pages.dev/basket-shot.html)
- [WARPING BAT](https://speed-racing-arcade.pages.dev/warping-bat.html)
- [THE WHITE ROOM 4](https://portaldejogos-br.github.io/the-white-room-4.html)
- [SNIPER MASTER](https://gameflash-viet.github.io/sniper-master.html)
- [BLOONS SURVIVALIO](https://juegosweb-desbloqueados.vercel.app/bloons-survivalio.html)
- [DINO HIDE N SHOOT](https://jogosweb-brasil.github.io/dino-hide-n-shoot.html)
- [IDLE MONEY FACTORY](https://retro-arcade-zone.netlify.app/idle-money-factory.html)
- [MATH STARS](https://maniadejogos-brasil.pages.dev/math-stars.html)
- [JIGSAW M](https://zona-juegos-flash.web.app/jigsaw-m.html)
- [MERGEST KINGDOM](https://arcadevault-games.github.io/mergest-kingdom.html)
- [GEOMETRY LITE](https://youxi-h5-tiandi.pages.dev/geometry-lite.html)
- [PORTAL MASTER](https://juegosmundial-hoy.pages.dev/portal-master.html)
- [UNO ONLINE](https://francejeux-online.web.app/uno-online.html)
- [TILE MATCH CONNECT 3 TILES](https://gamehay-online.netlify.app/tile-match-connect-3-tiles.html)
- [SNAKE PUZZLE ESCAPE](https://neon-cyber-arcade.pages.dev/snake-puzzle-escape.html)
- [SUPER ONION BOY 2](https://retro-arcade-zone.netlify.app/super-onion-boy-2.html)
- [DONUT BOX](https://francejeux-online.web.app/donut-box.html)
- [FIDGET TOYS POP IT](https://espacejeux-paris.pages.dev/fidget-toys-pop-it.html)
- [HIDE AND LUIG](https://zona-igr-besplatno.web.app/hide-and-luig.html)
- [WORDS FROM WORDS SEA](https://desi-gaming-arena.pages.dev/words-from-words-sea.html)
- [MURDERERS VS SHERIFFS DUELS](https://congdonggame-vietnam.web.app/murderers-vs-sheriffs-duels.html)
- [TURNFIGHT COM UAP](https://maniadejogos-brasil.pages.dev/turnfight-com-uap.html)
