
// TaskControl.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "TaskControl.h"
#include "TaskControlDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTaskControlApp

BEGIN_MESSAGE_MAP(CTaskControlApp, CWinApp)
END_MESSAGE_MAP()


// CTaskControlApp 构造

CTaskControlApp::CTaskControlApp() :
	pAppDatas(APPDATASNAME)
{

}

CTaskControlApp theApp;

BOOL CTaskControlApp::InitInstance()
{
	if (SingleInstanceCheck() == false)
	{
		goto ExitApp;
	}

	CWinApp::InitInstance();

	if (CollectArgs() == true)
	{
		if (hPS != NULL)
		{
			HANDLE hTH = (HANDLE)_beginthread(WaitForPSExit, 0, &hPS);
			CloseHandle(hTH);
		}

		CTaskControlDlg dlg;
		m_pMainWnd = &dlg;
		INT_PTR nResponse = dlg.DoModal();
	}

ExitApp:

	InformPSContinue();
	CloseHandle(hMu);
	CloseHandle(hPS);

	return FALSE;
}


