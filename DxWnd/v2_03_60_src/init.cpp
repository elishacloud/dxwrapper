/**
* Created from source code found in DxWnd v 2.03.60
* https://sourceforge.net/projects/dxwnd/
*
* Updated 2017 by Elisha Riedlinger
*/

#include <windows.h>
#include <stdio.h>
//********** Begin Edit *************
#include "Logging\Logging.h"
//********** End Edit ***************
#include "dxwnd.h"

typedef int (*StartHook_)();
typedef int (*EndHook_)();
typedef int (*SetTarget_)(TARGETMAP *);

BOOL DxWndEnabled = FALSE;
char DxWndIniPath[] = ".\\dxwnd.ini";

static TARGETMAP target;
static HMODULE hModule;

static int GetIntEntry(char *tag, int default)
{
	return GetPrivateProfileInt("target", tag, default, DxWndIniPath); 
}

//********** Begin Edit *************
void InitDxWnd(HMODULE Module)
{
	hModule = Module;
//********** End Edit ***************

	char DxWndPath[MAX_PATH];
	GetPrivateProfileString("system", "dxwndpath", ".", DxWndPath, MAX_PATH, DxWndIniPath);

    if (hModule)
    {
        StartHook_ startHook = (StartHook_)GetProcAddress(hModule, "StartHook");
        SetTarget_ setTarget = (SetTarget_)GetProcAddress(hModule, "SetTarget");
        if (startHook && setTarget)
        {
			// target.path must be set to current task pathname.
            GetModuleFileNameA(NULL, target.path, MAX_PATH);

			target.FakeVersionId = (short)GetIntEntry("winver0", 0);
			target.flags   = GetIntEntry("flag0", EMULATESURFACE|MODIFYMOUSE|USERGB565);
			target.flags2  = GetIntEntry("flagg0", WINDOWIZE|SETCOMPATIBILITY);
			target.flags3  = GetIntEntry("flagh0", HOOKDLLS|HOOKENABLED);
			target.flags4  = GetIntEntry("flagi0", SUPPORTSVGA);
			target.flags5  = GetIntEntry("flagj0", AEROBOOST|CENTERTOWIN);
			target.flags6  = GetIntEntry("flagk0", 0);
			target.tflags  = GetIntEntry("tflag0", 0);
			//target.initx   = GetIntEntry("initx0", 0);
			//target.inity   = GetIntEntry("inity0", 0);
			target.posx    = (short)GetIntEntry("posx0", 0);
			target.posy    = (short)GetIntEntry("posy0", 0);
            target.sizx    = (short)GetIntEntry("sizx0", 0);
            target.sizy    = (short)GetIntEntry("sizy0", 0);
			//target.maxx    = GetIntEntry("maxx0", 0);
			//target.maxy    = GetIntEntry("maxy0", 0);
			//target.minx    = GetIntEntry("minx0", 0);
			//target.miny    = GetIntEntry("miny0", 0);
            target.coordinates = GetIntEntry("coord0", 1); // centered to desktop
            target.MaxFPS  = (short)GetIntEntry("maxfps0", 0);
			target.dxversion = GetIntEntry("ver0", 0);
			if(target.dxversion > 1) target.dxversion += 5;
			target.FakeVersionId = (short)GetIntEntry("winver0", 0);
			target.MaxScreenRes = (short)GetIntEntry("maxres0", 0);
			target.SwapEffect = (short)GetIntEntry("swapeffect0", 0);
			target.InitTS = (short)GetIntEntry("initts0", 0);
                   
            setTarget((TARGETMAP *)&target);
            startHook();
        }
    }
	//********** Begin Edit *************
	else
	{
		Logging::Log() << "Cannot load dxwnd.dll library";
		//MessageBox(0, "Cannot load dxwnd.dll library", "dxwnd", MB_ICONERROR);
	}
	//********** End Edit ***************
}

void DxWndEndHook(void)
{
    if (hModule)
    {
        EndHook_ endHook = (EndHook_)GetProcAddress(hModule, "EndHook");
        if (endHook) endHook();
    }
}
