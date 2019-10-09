
// TaskControlDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "resource.h"
#include "afxcmn.h"

#include <boost/filesystem.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>

class FDlgCfg
{
public:

	FDlgCfg() :
		Left(0),
		Top(0)
	{
		char exePathChr[1024];
		GetModuleFileName(NULL, exePathChr, 1024);
		boost::filesystem::path exePath(exePathChr);
		DlgCfgPath = exePath.parent_path().append("TCcfg.bin");
	}

	void SaveToFile()
	{
		std::ofstream outDataStream(DlgCfgPath.string().c_str(), std::ios::binary);
		if (outDataStream.is_open())
		{
			boost::archive::binary_oarchive oa(outDataStream, boost::archive::no_header);
			oa << (*this);
			outDataStream.close();
		}
	}
	bool LoadFromFile()
	{
		if (boost::filesystem::exists(DlgCfgPath) != true)
		{
			return false;
		}

		std::ifstream inDataStream(DlgCfgPath.string().c_str(), std::ios::binary);
		if (inDataStream.is_open())
		{
			boost::archive::binary_iarchive ia(inDataStream, boost::archive::no_header);
			ia >> (*this);
			inDataStream.close();
			return true;
		}
		else
		{
			return false;
		}
	}

	void GetPos(int32_t& oLeft, int32_t& oTop) const
	{
		oLeft = Left;
		oTop = Top;
	}

	void SetPos(const int32_t& iLeft, const int32_t& iTop)
	{
		Left = iLeft;
		Top = iTop;
	}

private:
	friend class boost::serialization::access;
	// When the class Archive corresponds to an output archive, the
	// & operator is defined similar to <<.  Likewise, when the class Archive
	// is a type of input archive the & operator is defined similar to >>.
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & Left;
		ar & Top;
	}

	boost::filesystem::path DlgCfgPath;
	int32_t Left;
	int32_t Top;

};


// CTaskControlDlg 对话框
class CTaskControlDlg : public CDialogEx
{
	// 构造
public:
	CTaskControlDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TASKCONTROL_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	virtual BOOL PreTranslateMessage(MSG* pMsg) override
	{
		if (pMsg->message == WM_KEYDOWN)
		{
			if (pMsg->wParam == VK_ESCAPE)
			{
				return TRUE;
			}
		}
		return CDialog::PreTranslateMessage(pMsg);
	}

private:

	void InitParams()
	{
		::SetDlgItemText(this->m_hWnd, IDC_Stat, "...");
		SetCounter(0, 0, 0);
		PPG1.SetRange32(0, pProcInfo->FileCount);
		PPG1.SetStep(1);
		PPG1.SetPos(0);
		EProcLog.SetDlgItemTextA(IDC_EDIT1, "");
		EProcLog.SetReadOnly(1);
		SetBtn1Mode1();
		::SetDlgItemText(this->m_hWnd, IDC_BUTTON2, "立即停止");
		BBtn1.EnableWindow(1);
		BBtn2.EnableWindow(1);
		ShowWindow(SW_RESTORE);
	}

	void LoadCfg()
	{
		int screen_w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		int screen_h = GetSystemMetrics(SM_CYVIRTUALSCREEN);
		WINDOWINFO winf;
		GetWindowInfo(&winf);
		int32_t width = winf.rcWindow.right - winf.rcWindow.left;
		int32_t height = winf.rcWindow.bottom - winf.rcWindow.top;

		if (DlgCfg.LoadFromFile() == true)
		{
			int32_t l = 0;
			int32_t t = 0;
			DlgCfg.GetPos(l, t);

			int32_t titleheight = (winf.rcWindow.bottom - winf.rcWindow.top) - (winf.rcClient.bottom - winf.rcClient.top);

			if (l < (120 - width))
			{
				l = 120 - width;
			}
			else if ((screen_w - l) < 40)
			{
				l = screen_w - 40;
			}

			if (t < 0)
			{
				t = 0;
			}
			else if ((screen_h - t) < titleheight)
			{
				t = screen_h - titleheight;
			}

			SetWindowPos(nullptr, l, t, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		}
		else
		{
			SetWindowPos(nullptr, (screen_w / 2 - width / 2), (screen_h / 2 - height / 2), 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		}
	}

	void SaveCfg()
	{
		WINDOWINFO winf;
		GetWindowInfo(&winf);
		DlgCfg.SetPos(winf.rcWindow.left, winf.rcWindow.top);
		DlgCfg.SaveToFile();
	}

	inline void SetCounter(const uint32_t& iSuccess, const uint32_t& iFailed, const uint32_t& iTotal)
	{
		std::stringstream s_counter;
		s_counter << "成功： " << iSuccess << " / 失败： " << iFailed << " / 总文件数： " << iTotal;
		::SetDlgItemText(this->m_hWnd, IDC_Counter, s_counter.str().c_str());
	}
	inline void AppendLog(char* iStr)
	{
		if (strlen(iStr) != 0)
		{
			int len = EProcLog.GetWindowTextLengthA();
			EProcLog.SendMessage(EM_SETSEL, (WPARAM)len, (LPARAM)len);
			EProcLog.SendMessage(EM_REPLACESEL, 0, (LPARAM)((LPSTR)iStr));
		}
	}

	void SetBtn1Mode1()
	{
		::SetDlgItemText(this->m_hWnd, IDC_BUTTON1, "完成此行停止");
		f_Btn1 = CallStopThisRow;
	}
	void SetBtn1Mode2()
	{
		::SetDlgItemText(this->m_hWnd, IDC_BUTTON1, "取消停止");
		f_Btn1 = CallResumeProcessing;
	}
	static void CallStopThisRow()
	{
		FHostNotifier::CallHostFunc("StopThisRow");
	}
	static void CallResumeProcessing()
	{
		FHostNotifier::CallHostFunc("ResumeProcessing");
	}
	inline void UpdateStateText()
	{
		::SetDlgItemText(this->m_hWnd, IDC_Stat, EProcState::GetName(pProcInfo->ProcState));
	}
	inline void UpdateBtn1Mode()
	{
		switch (pProcInfo->ProcState)
		{
		case EProcState::Processing:
			SetBtn1Mode1();
			break;
		case EProcState::Pending:
			SetBtn1Mode2();
			break;
		case EProcState::Stopping:
			SetBtn1Mode1();
			break;
		case EProcState::Stoped:
			SetBtn1Mode1();
			break;
		case EProcState::Finished:
			SetBtn1Mode1();
			break;
		default:
			break;
		}
	}
	inline void UpdateBtns()
	{
		if ((pProcInfo->ProcState == EProcState::Stopping) ||
			(pProcInfo->ProcState == EProcState::Stoped) ||
			(pProcInfo->ProcState == EProcState::Finished)
			)
		{
			BBtn1.EnableWindow(0);
			BBtn2.EnableWindow(0);
		}
	}

private:

	void OnStateChanged()
	{
		UpdateStateText();
		UpdateBtn1Mode();
		UpdateBtns();
	}
	void OnInitParams()
	{
		InitParams();
		SetCounter(pProcInfo->FileSuccess, pProcInfo->FileFailed, pProcInfo->FileCount);
		PPG1.SetRange32(0, pProcInfo->FileCount);
		PPG1.SetPos(pProcInfo->FileSuccess + pProcInfo->FileFailed);
		AppendLog(pProcInfo->ProcMessage);
	}
	void OnProgress()
	{
		SetCounter(pProcInfo->FileSuccess, pProcInfo->FileFailed, pProcInfo->FileCount);
		PPG1.SetPos(pProcInfo->FileSuccess + pProcInfo->FileFailed);
		AppendLog(pProcInfo->ProcMessage);
	}
	void OnReceiveLog()
	{
		AppendLog(pProcInfo->ProcMessage);
	}
	void OnRequestPID()
	{
		theApp.pAppDatas->dTCpid = GetCurrentProcessId();
	}
	void OnPSExit()
	{
		EndDialog(0);
	}


private:
	FDlgCfg DlgCfg;
	FHostNotifier Notifier;
	FHostNotifierSync NotifierSyn;

	FFileMappingPtr<FProcInfo> pProcInfo;

	std::function<void()> f_Btn1;

public:
	CStatic SState;
	CStatic SCounter;
	CProgressCtrl PPG1;
	CEdit EProcLog;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	CButton BBtn1;
	CButton BBtn2;
	afx_msg void OnClose();

};
