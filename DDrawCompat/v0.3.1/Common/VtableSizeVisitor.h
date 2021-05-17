#pragma once

template <typename Vtable>
class VtableSizeVisitor
{
public:
	VtableSizeVisitor() : m_size(0)
	{
	}

	unsigned getSize() const { return m_size; };

	template <auto memberPtr>
	void visit(const char* /*funcName*/)
	{
		Vtable* vtable = nullptr;
		m_size = reinterpret_cast<unsigned>(&(vtable->*memberPtr)) + sizeof(vtable->*memberPtr);
	}

private:
	unsigned m_size;
};
