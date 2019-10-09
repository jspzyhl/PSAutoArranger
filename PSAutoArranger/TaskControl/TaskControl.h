
// TaskControl.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������
#include "commTypes.h"
#include <boost/lexical_cast.hpp>
#include <thread>
#include <chrono>

#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#define TCINSTANCEMUTEXNAME "mU_tAskcOnTroL"

// CTaskControlApp: 
// �йش����ʵ�֣������ TaskControl.cpp
//

class CTaskControlApp : public CWinApp
{
public:
	CTaskControlApp();

	// ��д
public:
	virtual BOOL InitInstance();

	// ʵ��

	DECLARE_MESSAGE_MAP()

private:

	bool CollectArgs()
	{
		FHostNotifierSync::CallHostFunc("WritePSpid", 10000);
		hPS = OpenProcess(PROCESS_ALL_ACCESS, 0, pAppDatas->dPSpid);
		pAppDatas->dPSpid = NULL;
#if _DEBUG
		if (__argc > 1)
		{
			if (strcmp(__argv[1], "-DEBUG") == 0)
			{
				return true;
			}
		}
#endif
		if (hPS == NULL)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

private:

	static void __cdecl WaitForPSExit(void* lpParam)
	{
		HANDLE hPS = *(HANDLE*)lpParam;
	
		WaitForSingleObject(hPS, INFINITE);
		FHostNotifier::CallHostFunc("OnPSExit");
	}

	bool SingleInstanceCheck()
	{
		hMu = OpenMutex(MUTEX_ALL_ACCESS, 0, TCINSTANCEMUTEXNAME);
		if (hMu == NULL)
		{
			hMu = CreateMutex(NULL, 1, TCINSTANCEMUTEXNAME);
			return true;
		}
		else
		{
			return false;
		}	
	}

public:
	
	static void InformPSContinue()
	{
		HANDLE hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, WAITFORTCINIT);
		if (hEvent)
		{
			SetEvent(hEvent);
			CloseHandle(hEvent);
		}			
	}

	FFileMappingPtr<FAppDataTs> pAppDatas;

private:

	HANDLE hPS = NULL;
	HANDLE hMu = NULL;

};

extern CTaskControlApp theApp;