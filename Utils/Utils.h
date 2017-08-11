#pragma once

namespace Utils
{
	void Shell(const char*);
	void DisableHighDPIScaling();
	void SetProcessAffinity();
	void SetAppCompat();
	void HookExceptionHandler();
	void UnHookExceptionHandler();

	namespace WriteMemory
	{
		void WriteMemory();
		void StopThread();
	}

	namespace Fullscreen
	{
		void StartThread();
		bool IsThreadRunning();
		void StopThread();
		void CheckCurrentScreenRes();
		void ResetScreen();
	}
}
