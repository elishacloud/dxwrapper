/**
* Created from source code found in DdrawCompat v2.1
* https://github.com/narzoul/DDrawCompat/
*
* Updated 2017 by Elisha Riedlinger
*
*/

#pragma once

#define CINTERFACE

//********** Begin Edit *************
#define DDRAWLOG_H
#define APP_DDRAWNAME			DDrawCompat
#define APP_DDRAWVERSION		v0.2.1
//********** End Edit ***************

#include <ddraw.h>
#include <fstream>
#include <type_traits>

#define LOG_ONCE(msg) \
	static bool isAlreadyLogged##__LINE__ = false; \
	if (!isAlreadyLogged##__LINE__) \
	{ \
		Compat::Log() << msg; \
		isAlreadyLogged##__LINE__ = true; \
	}

std::ostream& operator<<(std::ostream& os, const char* str);
std::ostream& operator<<(std::ostream& os, const unsigned char* data);
std::ostream& operator<<(std::ostream& os, const WCHAR* wstr);
std::ostream& operator<<(std::ostream& os, const RECT& rect);
std::ostream& operator<<(std::ostream& os, HDC__& dc);
std::ostream& operator<<(std::ostream& os, HWND__& hwnd);
std::ostream& operator<<(std::ostream& os, const DDSCAPS& caps);
std::ostream& operator<<(std::ostream& os, const DDSCAPS2& caps);
std::ostream& operator<<(std::ostream& os, const DDPIXELFORMAT& pf);
std::ostream& operator<<(std::ostream& os, const DDSURFACEDESC& sd);
std::ostream& operator<<(std::ostream& os, const DDSURFACEDESC2& sd);
std::ostream& operator<<(std::ostream& os, const CWPSTRUCT& cwrp);
std::ostream& operator<<(std::ostream& os, const CWPRETSTRUCT& cwrp);

template <typename T>
typename std::enable_if<std::is_class<T>::value && !std::is_same<T, std::string>::value, std::ostream&>::type
operator<<(std::ostream& os, const T& t)
{
	return os << static_cast<const void*>(&t);
}

template <typename T>
typename std::enable_if<std::is_class<T>::value, std::ostream&>::type
operator<<(std::ostream& os, T* t)
{
	return t ? (os << *t) : (os << "null");
}

template <typename T>
std::ostream& operator<<(std::ostream& os, T** t)
{
	return t ? (os << reinterpret_cast<void*>(t) << '=' << *t) : (os << "null");
}

namespace Compat
{
	class Log
	{
	public:
		Log();
		~Log();

		template <typename T>
		Log& operator<<(const T& t)
		{
			s_logFile << t;
			return *this;
		}

	protected:
		template <typename... Params>
		Log(const char* prefix, const char* funcName, Params... params) : Log()
		{
			s_logFile << prefix << ' ' << funcName << '(';
			toList(params...);
			s_logFile << ')';
		}

	private:
		void toList()
		{
		}

		template <typename Param>
		void toList(Param param)
		{
			s_logFile << param;
		}

		template <typename Param, typename... Params>
		void toList(Param firstParam, Params... remainingParams)
		{
			s_logFile << firstParam << ", ";
			toList(remainingParams...);
		}

		//********** Begin Edit *************
		// Get wrapper file name
		static char * Init()
		{
			static char wrappername[MAX_PATH];

			// Get module name
			HMODULE hModule = NULL;
			GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCTSTR)Init, &hModule);
			GetModuleFileName(hModule, wrappername, MAX_PATH);

			// Set lower case, remove extension and add dash (-)
			for (int z = 0; z < MAX_PATH && wrappername[z] != '\0'; z++) { wrappername[z] = (char)tolower(wrappername[z]); }
			char* pdest = strrchr(wrappername, '.');
			strcpy_s(pdest, MAX_PATH, "-");

			// Get process name
			char exepath[MAX_PATH];
			GetModuleFileName(NULL, exepath, MAX_PATH);

			// Remove path and add process name
			pdest = strrchr(exepath, '\\');
			strcat_s(wrappername, MAX_PATH, ++pdest);

			// Change extension to .log
			pdest = strrchr(wrappername, '.');
			strcpy_s(pdest, MAX_PATH, ".log");

			return wrappername;
		}
		//********** End Edit ***************

		static std::ofstream s_logFile;
	};

#ifdef _DEBUG
	typedef Log LogDebug;

	class LogEnter : private Log
	{
	public:
		template <typename... Params>
		LogEnter(const char* funcName, Params... params) : Log("-->", funcName, params...)
		{
		}
	};

	class LogLeave : private Log
	{
	public:
		template <typename... Params>
		LogLeave(const char* funcName, Params... params) : Log("<--", funcName, params...)
		{
		}

		template <typename Result>
		void operator<<(const Result& result)
		{
			static_cast<Log&>(*this) << " = " << std::hex << result << std::dec;
		}
	};
#else
	class LogDebug
	{
	public:
		template <typename T> LogDebug& operator<<(const T&) { return *this;  }
	};

	class LogEnter
	{
	public:
		template <typename... Params> LogEnter(const char*, Params...) {}
		template <typename Result> void operator<<(const Result&) {}
	};

	typedef LogEnter LogLeave;
#endif
}
