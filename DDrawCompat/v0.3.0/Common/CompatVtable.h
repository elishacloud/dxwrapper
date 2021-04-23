#pragma once

#include <Windows.h>

#include <DDrawCompat/v0.3.0/Common/VtableHookVisitor.h>
#include <DDrawCompat/v0.3.0/Common/VtableSizeVisitor.h>
#include <DDrawCompat/v0.3.0/Common/VtableVisitor.h>

template <typename Interface>
using Vtable = typename std::remove_pointer<decltype(Interface::lpVtbl)>::type;

template <typename Interface>
const Vtable<Interface>& getOrigVtable(Interface* /*This*/)
{
	return CompatVtable<Vtable<Interface>>::s_origVtable;
}

inline const IUnknownVtbl& getOrigVtable(IUnknown* This)
{
	return *This->lpVtbl;
}

template <typename Vtable>
class CompatVtable
{
public:
	static void hookCallbackVtable(const Vtable& vtable, UINT version)
	{
		static unsigned origVtableSize = 0;
		auto vtableSize = getVtableSize(version);
		memcpy(const_cast<Vtable*>(&vtable), &s_origVtable, min(vtableSize, origVtableSize));

		class NullLock {};
		hookVtable<NullLock>(vtable, version);

		if (vtableSize > origVtableSize)
		{
			origVtableSize = vtableSize;
		}
	}

	template <typename Lock>
	static void hookVtable(const Vtable& vtable, UINT version = 0)
	{
		auto vtableSize = getVtableSize(version);
		memcpy(&s_origVtable, &vtable, vtableSize);

		DWORD oldProtect = 0;
		VirtualProtect(const_cast<Vtable*>(&vtable), vtableSize, PAGE_READWRITE, &oldProtect);

		VtableHookVisitor<Vtable, Lock> vtableHookVisitor(vtable);
		forEach<Vtable>(vtableHookVisitor, version);

		VirtualProtect(const_cast<Vtable*>(&vtable), vtableSize, oldProtect, &oldProtect);
	}

	static Vtable s_origVtable;

private:
	static unsigned getVtableSize(UINT version)
	{
		VtableSizeVisitor<Vtable> vtableSizeVisitor;
		forEach<Vtable>(vtableSizeVisitor, version);
		return vtableSizeVisitor.getSize();
	}
};

template <typename Vtable>
Vtable CompatVtable<Vtable>::s_origVtable = {};
