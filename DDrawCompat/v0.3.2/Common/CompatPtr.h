#pragma once

#include <algorithm>

#include <DDrawCompat/v0.3.2/Common/CompatQueryInterface.h>
#include <DDrawCompat/v0.3.2/Common/CompatWeakPtr.h>

template <typename Intf>
class CompatPtr : public CompatWeakPtr<Intf>
{
public:
	template <typename OtherIntf>
	static CompatPtr from(OtherIntf* other)
	{
		return CompatPtr(Compat32::queryInterface<Intf>(other));
	}

	CompatPtr(std::nullptr_t = nullptr)
	{
	}

	explicit CompatPtr(Intf* intf) : CompatWeakPtr(intf)
	{
	}

	CompatPtr(const CompatPtr& other)
	{
		m_intf = Compat32::queryInterface<Intf>(other.get());
	}

	template <typename OtherIntf>
	CompatPtr(const CompatPtr<OtherIntf>& other)
	{
		m_intf = Compat32::queryInterface<Intf>(other.get());
	}

	~CompatPtr()
	{
		release();
	}

	CompatPtr& operator=(CompatPtr rhs)
	{
		swap(rhs);
		return *this;
	}

	Intf* detach()
	{
		Intf* intf = m_intf;
		m_intf = nullptr;
		return intf;
	}

	void reset(Intf* intf = nullptr)
	{
		*this = CompatPtr(intf);
	}

	void swap(CompatPtr& other)
	{
		std::swap(m_intf, other.m_intf);
	}
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const CompatPtr<T>& ptr)
{
	return os << ptr.get();
}
