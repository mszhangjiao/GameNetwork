#pragma once

#pragma once
#include <Windows.h>
#include <iostream>

using namespace std;

class CriticalSection
{
public:
	CriticalSection()
	{
		Init();
	}

	~CriticalSection()
	{
		DeleteCriticalSection(&m_CriticalSection);
	}

	bool Lock()
	{
		bool result = false;
		__try
		{
			EnterCriticalSection(&m_CriticalSection);
			result = true;
		}
		__except (GetExceptionCode() == STATUS_NO_MEMORY)
		{
			cout << "Failed to Lock critical section: out of memory" << endl;
		}

		return result;
	}

	bool Unlock()
	{
		bool result = false;
		__try
		{
			LeaveCriticalSection(&m_CriticalSection);
			result = true;
		}
		__except (GetExceptionCode() == STATUS_NO_MEMORY)
		{
			cout << "Failed to Unlock critical section: out of memory" << endl;
		}

		return result;
	}

private:
	bool Init() throw()
	{
		bool result = false;
		__try
		{
			InitializeCriticalSection(&m_CriticalSection);
			result = true;
		}
		__except (GetExceptionCode() == STATUS_NO_MEMORY)
		{
			cout << "Failed to initialize critical section: out of memory" << endl;
		}

		return result;
	}

	CriticalSection(const CriticalSection& section);
	CriticalSection& operator=(const CriticalSection& section);
	CRITICAL_SECTION m_CriticalSection;
};

class AutoCriticalSection
{
public:
	AutoCriticalSection(CriticalSection* pSection)
	{
		m_pSection = pSection;
		m_pSection->Lock();
	}

	~AutoCriticalSection()
	{
		m_pSection->Unlock();
	}

private:
	AutoCriticalSection(const AutoCriticalSection& aSection);
	AutoCriticalSection& operator=(const AutoCriticalSection& aSection);
	CriticalSection *m_pSection;
};