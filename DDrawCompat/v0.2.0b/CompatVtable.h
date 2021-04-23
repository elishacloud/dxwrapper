#pragma once

#define WIN32_LEAN_AND_MEAN

#include <map>
#include <string>
#include <vector>

#include <Windows.h>
#include <External\detours\src\detours.h>

#include "DDrawCompat\DDrawLog.h"
#include "DDrawVtableVisitor.h"

namespace Compat20
{
	template <typename Interface>
	using Vtable = typename std::remove_pointer<decltype(Interface::lpVtbl)>::type;

	namespace Compat
	{
		struct DetouredMethodInfo
		{
			DetouredMethodInfo(void*& updatedOrigMethodPtr, std::map<void*, void*>& vtablePtrToCompatVtable)
				: updatedOrigMethodPtr(updatedOrigMethodPtr), vtablePtrToCompatVtable(vtablePtrToCompatVtable)
			{
			}

			void*& updatedOrigMethodPtr;
			std::map<void*, void*>& vtablePtrToCompatVtable;
		};

		extern std::map<void*, DetouredMethodInfo> detouredMethods;
	}

	template <typename CompatInterface, typename Interface>
	class CompatVtable
	{
	public:
		typedef Interface Interface;

		static void hookVtable(Interface& intf)
		{
			static bool isInitialized = false;
			if (!isInitialized)
			{
				isInitialized = true;

				s_vtablePtr = intf.lpVtbl;
				s_origVtable = *intf.lpVtbl;

				InitVisitor visitor;
				forEach<Vtable<Interface>>(visitor);
			}
		}

		static Vtable<Interface> s_origVtable;

	private:
		class InitVisitor
		{
		public:
			template <typename MemberDataPtr, MemberDataPtr ptr>
			void visit()
			{
				if (!(s_compatVtable.*ptr))
				{
					s_threadSafeVtable.*ptr = s_origVtable.*ptr;
					s_compatVtable.*ptr = s_origVtable.*ptr;
				}
				else
				{
					s_threadSafeVtable.*ptr = getThreadSafeFuncPtr<MemberDataPtr, ptr>(s_compatVtable.*ptr);
					hookMethod(reinterpret_cast<void*&>(s_origVtable.*ptr), s_threadSafeVtable.*ptr);
				}
			}

			template <typename MemberDataPtr, MemberDataPtr ptr>
			void visitDebug(const std::string& vtableTypeName, const std::string& funcName)
			{
				s_funcNames[getKey<MemberDataPtr, ptr>()] = vtableTypeName + "::" + funcName;

				s_threadSafeVtable.*ptr = getThreadSafeFuncPtr<MemberDataPtr, ptr>(s_compatVtable.*ptr);
				hookMethod(reinterpret_cast<void*&>(s_origVtable.*ptr), s_threadSafeVtable.*ptr);

				if (!(s_compatVtable.*ptr))
				{
					s_compatVtable.*ptr = s_origVtable.*ptr;
				}
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

			void hookMethod(void*& origMethodPtr, void* newMethodPtr)
			{
				auto it = Compat::detouredMethods.find(origMethodPtr);
				if (it != Compat::detouredMethods.end())
				{
					origMethodPtr = it->second.updatedOrigMethodPtr;
					it->second.vtablePtrToCompatVtable[s_vtablePtr] = &s_compatVtable;
				}
				else
				{
					s_vtablePtrToCompatVtable[s_vtablePtr] = &s_compatVtable;
					Compat::detouredMethods.emplace(origMethodPtr,
						Compat::DetouredMethodInfo(origMethodPtr, s_vtablePtrToCompatVtable));
					DetourTransactionBegin();
					DetourAttach(&origMethodPtr, newMethodPtr);
					DetourTransactionCommit();
				}
			}

			template <typename MemberDataPtr, MemberDataPtr ptr, typename Result, typename IntfPtr, typename... Params>
			static Result STDMETHODCALLTYPE threadSafeFunc(IntfPtr This, Params... params)
			{
				Compat::origProcs.AcquireDDThreadLock();
#ifdef _DEBUG
				Compat::LogEnter(s_funcNames[getKey<MemberDataPtr, ptr>()].c_str(), This, params...);
#endif

				Result result;
				auto it = s_vtablePtrToCompatVtable.find(This->lpVtbl);
				if (it != s_vtablePtrToCompatVtable.end())
				{
					Vtable<Interface>& compatVtable = *static_cast<Vtable<Interface>*>(it->second);
					result = (compatVtable.*ptr)(This, params...);
				}
				else
				{
					result = (s_origVtable.*ptr)(This, params...);
				}

#ifdef _DEBUG
				Compat::LogLeave(s_funcNames[getKey<MemberDataPtr, ptr>()].c_str(), This, params...) << result;
#endif
				Compat::origProcs.ReleaseDDThreadLock();
				return result;
				}
		};

		static Vtable<Interface> createCompatVtable()
		{
			Vtable<Interface> vtable = {};
			CompatInterface::setCompatVtable(vtable);
			return vtable;
		}

		static Vtable<Interface>& getCompatVtable()
		{
			static Vtable<Interface> vtable(createCompatVtable());
			return vtable;
		}

		static Vtable<Interface>* s_vtablePtr;
		static Vtable<Interface> s_compatVtable;
		static Vtable<Interface> s_threadSafeVtable;
		static std::map<void*, void*> s_vtablePtrToCompatVtable;
		static std::map<std::vector<unsigned char>, std::string> s_funcNames;
	};

	template <typename CompatInterface, typename Interface>
	Vtable<Interface>* CompatVtable<CompatInterface, Interface>::s_vtablePtr = nullptr;

	template <typename CompatInterface, typename Interface>
	Vtable<Interface> CompatVtable<CompatInterface, Interface>::s_origVtable = {};

	template <typename CompatInterface, typename Interface>
	Vtable<Interface> CompatVtable<CompatInterface, Interface>::s_compatVtable(CompatInterface::getCompatVtable());

	template <typename CompatInterface, typename Interface>
	Vtable<Interface> CompatVtable<CompatInterface, Interface>::s_threadSafeVtable = {};

	template <typename CompatInterface, typename Interface>
	std::map<void*, void*> CompatVtable<CompatInterface, Interface>::s_vtablePtrToCompatVtable;

	template <typename CompatInterface, typename Interface>
	std::map<std::vector<unsigned char>, std::string> CompatVtable<CompatInterface, Interface>::s_funcNames;
}
