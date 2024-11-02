#pragma once

typedef long HRESULT;

class HResultException
{
public:
	HResultException(HRESULT result) : m_result(result) {}

	HRESULT getResult() const { return m_result; }

private:
	HRESULT m_result;
};
