# DxWrapper
## Introduction
DxWrapper is a dll file designed to wrap or hook DirectX (or other) files to fix compatibility issues in older games.  This project is primarily targeted to fixing compatibility issues with running games on Windows 10, ideally by simply dropping a dll file and ini file into the game folder.  A secondary purpose for this is to offer a single tool (one stop shop, so to speak) by combining several other projects into one.  So far this dll can wrap the following files: d3d8.dll, d3d9.dll, ddraw.dll, dplayx.dll, dsound.dll, dxgi.dll or winmm.dll

## Features
DxWrapper has a number of features and projects that it includes.  Below is a list of features:

 - Integration of DdrawCompat project
 - Proxy for DxWnd project
 - Convert D3D8 to D3D9
 - Hot patch memory support (update game memory at run time, can be used to remove CD/DVD checks or resolve graphics glitches)
 - Set single CPU affinity (some games have issue with dual-core CPUs)
 - Disable display scaling on high DPI setting
 - Set game window to full screen (some games have full screen issues)
 - Handle exceptions or crashes in games
 - Load custom dll files into the game
 - Run processes when the game starts up

## Windows 10 Games Compatibility
So far I have tested this with over 70 games (that don't otherwise work correctly) to get them running on Windows 10.  Below is a list of some of the games I have tested so far.  Most work flawlessly, some still have issues I need to resolve.  I am planning to create a page for each game explaining how I got them working and include a simple download for anyone who wants to run these games on Windows 10.

 - Alien Nations
 - Atlantis The Lost Tales
 - Blitzkrieg
 - Blitzkrieg BHRT
 - Close Combat IV
 - Codename Outbreak
 - Combat Mission Beyond Overlord
 - Conquest Frontier Wars
 - Dark Fall 2 Lights Out
 - Dark Reign
 - Deadlock II
 - Desperados Wanted Dead or Alive
 - Diablo
 - Diablo Hellfire
 - Double Dragon Trilogy
 - Dungeons and Dragons Dragonshard
 - Freespace 2
 - Gangsters Organized Crime
 - Gothic
 - Guilty Gear Isuka
 - Guilty Gear X2 Reload
 - Heretic Kingdoms
 - InkBall
 - Interstate 82
 - Jagged Alliance 2
 - Majesty Gold HD
 - MechWarrior 4
 - Might and Magic IX
 - Moto Racer 3
 - Myst III Exile
 - Oddworld Abe's Oddysee
 - Outpost 2
 - Pandemonium
 - Patrician 3
 - Perimeter
 - Port Royale
 - Prince of Persia
 - Raiden Legacy
 - RavenShield
 - Return of TIM
 - RollerCoaster Tycoon 3 Platinum
 - SimCity 4 Deluxe Edition
 - SimCity Classic
 - Splinter Cell
 - Star Trek Armada
 - Star Trek Starfleet Command
 - Star Trek Voyager Elite Force
 - Star Wars Battlefront
 - Star Wars Starfighter
 - SWAT 3
 - Temple of Elemental Evil
 - The Great Battles of Caesar
 - The Great Battles of Hannibal
 - The Longest Journey
 - The Nations Gold
 - The Settlers IV Gold
 - The Summoner
 - TIM Even More Contraptions
 - Tomb Raider The Angel of Darkness
 - War Wind
 - War Wind II Human Onslaught
 - WarGames
 - Warhammer 40,000 Chaos Gate

## Installation

1. Download all the files from the repository [Build](https://github.com/elishacloud/dxwrapper/tree/master/Release/Build) page and save them to a folder.
2. Determine which dll file is needed for the game.  This depends on many factors and I will add a page explaining this later.  Most commonly it is 'ddraw.dll' or 'dsound.dll'.
3. Copy this dll file and the dgame.ini file into the game's installation directory, next to the main executable file.  For some games the main executable is in a subdirectory (like 'Bin', 'Exe' or 'App') you will need to copy it into that directory.  It is not recommended to overwrite any existing files from the game.
4. (Optional, but recommended) Rename the 'dgame.ini' to match the name of the dll file.  Example if you are using the 'ddraw.dll' then you want to rename 'dgame.ini' to 'ddraw.ini'.
5. Open up the ini file in a text editor (like Notepad) and enable settings needed for the game.

Do not attempt to overwrite any dll in a Windows system directory, it is currently not supported and will not work.

## Uninstallation

Delete DxWrapper dll files and the ini files from the game's directory. You can also delete the log file, if there is one.

## Configuration

I am planning to add a configuration page later.  Please check back soon!

## License
This project uses code from several other projects. So to give credit where credit is due, below is a list of locations that source code was taken from:

 - DdrawCompat: https://github.com/narzoul/DDrawCompat/
 - DxWnd: https://sourceforge.net/projects/dxwnd/
 - OllyDbg: http://www.ollydbg.de/
 - Ultimate-ASI-Loader: https://github.com/ThirteenAG/Ultimate-ASI-Loader
 - Aqrit's ddwrapper: http://bitpatch.com/ddwrapper.html

## Development
DxWrapper is written in C++ using Microsoft Visual Studio Community 2015.

Compilation depends on [Detours Express 3.0](http://research.microsoft.com/en-us/projects/detours/). It needs to be installed first before DxWrapper can be built. You will need to change the include and library paths as needed if you didn't install/build Detours in the default directory.

Thanks for stopping by!