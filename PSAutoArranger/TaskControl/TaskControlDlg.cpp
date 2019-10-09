
// TaskControlDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TaskControl.h"
#include "TaskControlDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTaskControlDlg 对话框

CTaskControlDlg::CTaskControlDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_TASKCONTROL_DIALOG, pParent),
	pProcInfo(PROCINFONAME)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTaskControlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_Stat, SState);
	DDX_Control(pDX, IDC_Counter, SCounter);
	DDX_Control(pDX, IDC_PROGRESS1, PPG1);
	DDX_Control(pDX, IDC_EDIT1, EProcLog);
	DDX_Control(pDX, IDC_BUTTON1, BBtn1);
	DDX_Control(pDX, IDC_BUTTON2, BBtn2);
}

BEGIN_MESSAGE_MAP(CTaskControlDlg, CDialogEx)	
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CTaskControlDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CTaskControlDlg::OnBnClickedButton2)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CTaskControlDlg 消息处理程序

BOOL CTaskControlDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	NotifierSyn.BindHostFunc("OnStateChanged", std::bind(&CTaskControlDlg::OnStateChanged, this));
	NotifierSyn.BindHostFunc("OnInitParams", std::bind(&CTaskControlDlg::OnInitParams, this));
	NotifierSyn.BindHostFunc("OnProgress", std::bind(&CTaskControlDlg::OnProgress, this));
	NotifierSyn.BindHostFunc("OnReceiveLog", std::bind(&CTaskControlDlg::OnReceiveLog, this));
	NotifierSyn.BindHostFunc("OnRequestPID", std::bind(&CTaskControlDlg::OnRequestPID, this));
	Notifier.BindHostFunc("OnPSExit", std::bind(&CTaskControlDlg::OnPSExit, this));
	
	LoadCfg();
	InitParams();
	CTaskControlApp::InformPSContinue();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CTaskControlDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CTaskControlDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CTaskControlDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	f_Btn1();
}

void CTaskControlDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	FHostNotifier::CallHostFunc("StopAtonce");
}

void CTaskControlDlg::OnClose()
{
	if ((pProcInfo->ProcState != EProcState::Processing) &&
		(pProcInfo->ProcState != EProcState::Pending)
		)
	{
		SaveCfg();
		CDialogEx::OnClose();
	}
	else
	{
		if (MessageBox("是否立即停止处理并关闭对话框？", TEXT("信息"), MB_YESNO | MB_ICONQUESTION) == IDYES)
		{
			FHostNotifier::CallHostFunc("StopAtonce");
			SaveCfg();
			CDialogEx::OnClose();
		}
	}
}

