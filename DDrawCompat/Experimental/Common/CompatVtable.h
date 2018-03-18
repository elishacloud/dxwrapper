#pragma once

#include <map>
#include <string>
#include <vector>

#include "Common/Hook.h"
#include "DDrawCompat\DDrawLog.h"
#include "Common/VtableVisitor.h"
#include "DDraw/ScopedThreadLock.h"

#define SET_COMPAT_VTABLE(Vtable, CompatInterface) \
	namespace Compat \
	{ \
		inline void setCompatVtable(Vtable& vtable) \
		{ \
			CompatInterface::setCompatVtable(vtable); \
		} \
	}

template <typename Interface>
using Vtable = typename std::remove_pointer<decltype(Interface::lpVtbl)>::type;

template <typename Vtable>
class CompatVtable
{
public:
	static const Vtable& getOrigVtable(const Vtable& vtable)
	{
		return s_origVtable.AddRef ? s_origVtable : vtable;
	}

	static void hookVtable(const Vtable* vtable)
	{
		if (vtable && !s_origVtablePtr)
		{
			s_origVtablePtr = vtable;

			HookVisitor<DDrawHook> visitor(*vtable, s_origVtable);
			forEach<Vtable>(visitor);
		}
	}

	static void hookDriverVtable(HANDLE context, const Vtable* vtable)
	{
		if (vtable && s_origVtables.find(context) == s_origVtables.end())
		{
			HookVisitor<DriverHook> visitor(*vtable, s_origVtables[context]);
			forEach<Vtable>(visitor);
		}
	}

	static Vtable s_origVtable;
	static std::map<HANDLE, Vtable> s_origVtables;
	static const Vtable* s_origVtablePtr;

private:
	class DDrawHook
	{
	public:
		template <typename MemberDataPtr, MemberDataPtr ptr, typename FirstParam>
		static decltype(s_compatVtable.*ptr) getCompatFunc(FirstParam)
		{
			return s_compatVtable.*ptr ? s_compatVtable.*ptr : s_origVtable.*ptr;
		}
	};

	class DriverHook
	{
	public:
		template <typename MemberDataPtr, MemberDataPtr ptr>
		static decltype(s_compatVtable.*ptr) getCompatFunc(HANDLE context)
		{
			return s_compatVtable.*ptr ? s_compatVtable.*ptr : s_origVtables.at(context).*ptr;
		}
	};

	template <typename Hook>
	class HookVisitor
	{
	public:
		HookVisitor(const Vtable& srcVtable, Vtable& origVtable)
			: m_srcVtable(srcVtable)
			, m_origVtable(origVtable)
		{
		}

		template <typename MemberDataPtr, MemberDataPtr ptr>
		void visit()
		{
			m_origVtable.*ptr = m_srcVtable.*ptr;
			if (s_compatVtable.*ptr)
			{
				Compat::hookFunction(reinterpret_cast<void*&>(m_origVtable.*ptr),
					getThreadSafeFuncPtr<MemberDataPtr, ptr>(m_origVtable.*ptr));
			}
		}

		template <typename MemberDataPtr, MemberDataPtr ptr>
		void visitDebug(const std::string& vtableTypeName, const std::string& funcName)
		{
			Compat::Log() << "Hooking function: " << vtableTypeName << "::" << funcName;
			s_funcNames[getKey<MemberDataPtr, ptr>()] = vtableTypeName + "::" + funcName;

			m_origVtable.*ptr = m_srcVtable.*ptr;
			Compat::hookFunction(reinterpret_cast<void*&>(m_origVtable.*ptr),
				getThreadSafeFuncPtr<MemberDataPtr, ptr>(m_origVtable.*ptr));
		}

	private:
		template <typename Result, typename... Params>
		using FuncPtr = Result(STDMETHODCALLTYPE *)(Params...);

		template <typename MemberDataPtr, MemberDataPtr ptr>
		static std::vector<unsigned char> getKey()
		{
			MemberDataPtr mp = ptr;
			unsigned char* p = reinterpret_cast<unsigned char*>(&mp);
			return std::vector<unsigned char>(p, p + sizeof(mp));
		}

		template <typename MemberDataPtr, MemberDataPtr ptr, typename Result, typename... Params>
		static FuncPtr<Result, Params...> getThreadSafeFuncPtr(FuncPtr<Result, Params...>)
		{
			return &threadSafeFunc<MemberDataPtr, ptr, Result, Params...>;
		}

		template <typename MemberDataPtr, MemberDataPtr ptr, typename... Params>
		static FuncPtr<void, Params...> getThreadSafeFuncPtr(FuncPtr<void, Params...>)
		{
			return &threadSafeFunc<MemberDataPtr, ptr, Params...>;
		}

		template <typename MemberDataPtr, MemberDataPtr ptr,
			typename Result, typename FirstParam, typename... Params>
		static Result STDMETHODCALLTYPE threadSafeFunc(FirstParam firstParam, Params... params)
		{
			DDraw::ScopedThreadLock lock;
#ifdef _DEBUG
			const char* funcName = s_funcNames[getKey<MemberDataPtr, ptr>()].c_str();
			Compat::LogEnter(funcName, firstParam, params...);
			Result result = Hook::getCompatFunc<MemberDataPtr, ptr>(firstParam)(firstParam, params...);
			Compat::LogLeave(funcName, firstParam, params...) << result;
			return result;
#else
			return (s_compatVtable.*ptr)(firstParam, params...);
#endif
		}

		template <typename MemberDataPtr, MemberDataPtr ptr, typename FirstParam, typename... Params>
		static void STDMETHODCALLTYPE threadSafeFunc(FirstParam firstParam, Params... params)
		{
			DDraw::ScopedThreadLock lock;
#ifdef _DEBUG
			const char* funcName = s_funcNames[getKey<MemberDataPtr, ptr>()].c_str();
			Compat::LogEnter(funcName, firstParam, params...);
			Hook::getCompatFunc<MemberDataPtr, ptr>(firstParam)(firstParam, params...);
			Compat::LogLeave(funcName, firstParam, params...);
#else
			(s_compatVtable.*ptr)(firstParam, params...);
#endif
		}

		const Vtable& m_srcVtable;
		Vtable& m_origVtable;
	};

	static Vtable createCompatVtable()
	{
		Vtable vtable = {};
		Compat::setCompatVtable(vtable);
		return vtable;
	}

	static Vtable& getCompatVtable()
	{
		static Vtable vtable(createCompatVtable());
		return vtable;
	}

	static Vtable s_compatVtable;
	static std::map<std::vector<unsigned char>, std::string> s_funcNames;
};

template <typename Vtable>
Vtable CompatVtable<Vtable>::s_origVtable = {};

template <typename Vtable>
std::map<HANDLE, Vtable> CompatVtable<Vtable>::s_origVtables;

template <typename Vtable>
const Vtable* CompatVtable<Vtable>::s_origVtablePtr = nullptr;

template <typename Vtable>
Vtable CompatVtable<Vtable>::s_compatVtable(getCompatVtable());

template <typename Vtable>
std::map<std::vector<unsigned char>, std::string> CompatVtable<Vtable>::s_funcNames;
