#pragma once

namespace Utils
{
	void Shell(const char*);
	void DisableHighDPIScaling();
	void SetAppCompat();
	void HookExceptionHandler();
	void UnHookExceptionHandler();
	void WriteMemory();
	void StopWriteMemoryThread();
}
