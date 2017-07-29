#pragma once

namespace Utils
{
	void Shell(const char*);
	void DisableHighDPIScaling();
	void SetAppCompat();
	void HookExceptionHandler();
	void WriteMemory();
	void StopWriteMemoryThread();
}
