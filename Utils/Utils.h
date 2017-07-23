#pragma once

namespace Utils
{
	void Shell(char*);
	void DisableHighDPIScaling();
	void SetAppCompat();
	void HookExceptionHandler();
	void WriteMemory();
	void StopWriteMemoryThread();
}
