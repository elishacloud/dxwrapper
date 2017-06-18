#pragma once

struct screen_res
{
	LONG Width = 0;
	LONG Height = 0;

	screen_res& operator=(const screen_res& a)
	{
		Width = a.Width;
		Height = a.Height;
		return *this;
	}

	bool operator==(const screen_res& a) const
	{
		return (Width == a.Width && Height == a.Height);
	}

	bool operator!=(const screen_res& a) const
	{
		return (Width != a.Width || Height != a.Height);
	}
};

void CheckCurrentScreenRes(screen_res&);
void SetScreen(screen_res);
void ResetScreen();
void StartFullscreenThread();
bool IsFullscreenThreadRunning();
DWORD GetMyThreadId(HANDLE);
void StopFullscreenThread();
void CreateTimer(HWND);