
//-------------------------------------------------------------------------------
//	Includes 
//-------------------------------------------------------------------------------
#ifndef __AUTOARR_H__
#define __AUTOARR_H__

#include "PIDefines.h"
#if __PIMac__
#include <string.h>
#endif

#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <windows.h>
#include <process.h>

#include "PIUSelect.h"
#include "SPHost.h"
#include "PIUGet.h"
#include "PIActionsPlugIn.h"
#include "PIUIHooksSuite.h"
#include "PIHandleSuite.h"
#include "ASZStringSuite.h"
#include "PIUSuites.h"
#include "PIStringTerminology.h"

#include "resource.h"

#include <boost/filesystem.hpp>
#include <boost/locale.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>

#include "Encap.h"
#include "commTypes.h"

using namespace std;



class FConfigDialog;
class FProcessDialog;
class FRecDlg;

extern SPBasicSuite* sSPBasic;
extern SPPluginRef	gPlugInRef;
extern HWND hPSwnd;
extern HANDLE hWaitEvent;

DLLExport SPAPI SPErr AutoPluginMain(
	const char* caller,	// who is calling
	const char* selector, // what do they want
	void* message	// what is the message
);


static void recuFiles(const boost::filesystem::path& src_path, vector<string>& FilePathList);


inline void GetAppDataPath(std::string& OutPath)
{
	char* appdata;
	size_t sz = 0;
	_dupenv_s(&appdata, &sz, "APPDATA");
	OutPath.clear();
	OutPath.append(appdata);
	delete(appdata);
}

inline boost::filesystem::path GetAppPath()
{
	std::string appdataPath;
	GetAppDataPath(appdataPath);
	boost::filesystem::path AppPath(appdataPath);
	AppPath.append("AutoArranger");
	return std::move(AppPath);
}

inline boost::filesystem::path GetConfigFilePath()
{
	return GetAppPath().append("Config.bin");
}

inline boost::filesystem::path GetProgressFilePath()
{
	return GetAppPath().append("Progress.bin");
}

inline boost::filesystem::path GetTaskControlPath()
{
//#if _DEBUG
//	return boost::filesystem::path("E:\\Users\\YHL\\Desktop\\ps_ext\\adobe_photoshop_sdk_cc_2015_5_win\\pluginsdk\\samplecode\\automation\\AutoArranger\\win\\x64\\Debug\\TaskControl.exe");
//#endif
	return GetAppPath().append("TaskControl.exe");
}

class FProgressIndex
{
public:

	FProgressIndex(const boost::filesystem::path& iFilePath)
	{
		if (boost::filesystem::exists(iFilePath))
		{
			ProgressFile = fopen(iFilePath.string().c_str(), "r+b");
		}
		else
		{
			boost::filesystem::create_directories(iFilePath.parent_path());
			FILE* newFile = fopen(iFilePath.string().c_str(), "w");
			const int32 InvalidProgress = -1;
			fwrite(&InvalidProgress, sizeof(int32), 1, newFile);
			fclose(newFile);
			ProgressFile = fopen(iFilePath.string().c_str(), "r+b");
		}
		if (!ProgressFile)
		{
			throw std::string("无法读写文件");
		}
	}
	~FProgressIndex()
	{
		fclose(ProgressFile);
	}

	int32 ReadProgress()
	{
		int32 data = 0;
		rewind(ProgressFile);
		size_t sz = fread(&data, sizeof(int32), 1, ProgressFile);
		int e = ferror(ProgressFile);
		return std::move(data);
	}

	void WriteProgress(const int32 iProgress)
	{
		rewind(ProgressFile);
		size_t sz = fwrite(&iProgress, sizeof(int32), 1, ProgressFile);
		int e = ferror(ProgressFile);
		int a;
		a = 1;
	}

	void CloseFile()
	{
		fclose(ProgressFile);
	}

private:
	FILE* ProgressFile;
	std::string FilePath;
};




class FAppConfigs
{
private:
	SPPluginRef PlgRef;

public:
	FAppConfigs(const boost::filesystem::path& iFilePath) :
		AutoDetect(true),
		ManualWidth(100),
		ManualHeight(100),
		PicSizeUnit(UNT_Pixel),
		SpaceHorizon(10),
		SpaceVertical(10),
		SpcSizeUnit(UNT_Pixel),
		TargetDocIndex(0)
	{
		ConfigFilePath = iFilePath;
		TaskFiles.clear();
	}
	~FAppConfigs()
	{

	}

	void LoadFromFile()
	{
		std::ifstream inDataStream(ConfigFilePath.string(), std::ios::binary);
		if (inDataStream.is_open())
		{
			boost::archive::binary_iarchive ia(inDataStream, boost::archive::no_header);
			ia >> (*this);
			inDataStream.close();
		}
	}
	void SaveToFile()
	{
		boost::filesystem::create_directories(ConfigFilePath.parent_path());
		std::ofstream outDataStream(ConfigFilePath.string(), std::ios::binary);
		if (outDataStream.is_open())
		{
			boost::archive::binary_oarchive oa(outDataStream, boost::archive::no_header);
			oa << (*this);
			outDataStream.close();
		}
	}

	friend class boost::serialization::access;
	// When the class Archive corresponds to an output archive, the
	// & operator is defined similar to <<.  Likewise, when the class Archive
	// is a type of input archive the & operator is defined similar to >>.
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & AutoDetect;
		ar & ManualWidth;
		ar & ManualHeight;
		ar & PicSizeUnit;
		ar & SpaceHorizon;
		ar & SpaceVertical;
		ar & SpcSizeUnit;
		ar & TaskFiles;
	}

	bool AutoDetect;
	double ManualWidth;
	double ManualHeight;
	int32 PicSizeUnit;

	double SpaceHorizon;
	double SpaceVertical;
	int32 SpcSizeUnit;

	int32 TargetDocIndex;

	std::vector<std::string> TaskFiles;

private:
	boost::filesystem::path ConfigFilePath;
};

class EExitReason
{
public:
	enum Type
	{
		Unknown = 0,
		AllFinished = 1,
		PageFull = 2,
		Partly = 3,
		MAX,
	};

	static char* GetName(const EExitReason::Type& iType)
	{
		switch (iType)
		{
		case EExitReason::Unknown:
			return "未知原因";
			break;
		case  EExitReason::AllFinished:
			return "全部文件已被处理";
			break;
		case  EExitReason::PageFull:
			return "输出页面已满";
			break;
		case EExitReason::Partly:
			return "部分文件未被处理";
			break;
		default:
			return "";
			break;
		}
	}
};

class FExitReasons
{
public:

	void SetFlag(const EExitReason::Type& iReasonEnum, bool iFlg)
	{
		if (iFlg)
		{
			flags |= (1 << (uint32)iReasonEnum);
		}
		else
		{
			flags &= ~(1 << (uint32)iReasonEnum);
		}
	}

	bool GetFlag(const uint32& iBitCount)
	{
		return (1 & (flags >> iBitCount));
	}

	std::string GetAllReasons()
	{
		stringstream ss;
		for (uint32 i = 1, n = (uint32)EExitReason::MAX; i < n; ++i)
		{
			if (GetFlag(i))
			{
				ss << EExitReason::GetName((EExitReason::Type)i) << ",  ";
			}
		}
		return ss.str();
	}

private:

	uint32_t flags = 0;

};

class FAutoArranger
{
private:



	void UpdateDocumentTitles()
	{
		Auto_Desc appDesc;
		GetApplicationDesc(&appDesc);
		int32 docNum = 0;
		GetIntegerValueByKeyName(appDesc.get(), "numberOfDocuments", docNum);
		DocTitles.clear();
		for (int32 i = 1, n = docNum; i <= n; ++i)
		{
			Auto_Desc idocDesc;
			GetClassDescByIndex(classDocument, i, &idocDesc);
			std::string ititle;
			GetStringValueByKeyName(idocDesc.get(), "title", ititle);
			DocTitles.emplace_back(ititle);
		}
	}

	//	~~~~IPC commands~~~~

	void StopThisRow()
	{
		bStopThisRow = true;
		SendProcState(EProcState::Pending);
	}
	void ResumeProcessing()
	{
		bStopThisRow = false;
		SendProcState(EProcState::Processing);
	}
	void StopAtonce()
	{
		bStopAtonce = true;
		SendProcState(EProcState::Stopping);
	}

	void WritePSpid()
	{
		pAppDatas->dPSpid = GetCurrentProcessId();
	}

	//	======= IPC commands end ==========

	void CreateTaskControl()
	{
		boost::filesystem::path TCpath = GetTaskControlPath();
		if (boost::filesystem::exists(TCpath) != true)
		{
			return;
		}

		HANDLE hWaitEvent = CreateEvent(NULL, FALSE, FALSE, WAITFORTCINIT);

		char* szCommandLine = _strdup(TCpath.string().c_str());

		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		si.dwFlags = STARTF_USESHOWWINDOW;  // 指定wShowWindow成员有效
		si.wShowWindow = TRUE;          // 此成员设为TRUE的话则显示新建进程的主窗口，
										// 为FALSE的话则不显示
		BOOL bRet = CreateProcess(
			NULL,           // 不在此指定可执行文件的文件名
			szCommandLine,      // 命令行参数
			NULL,           // 默认进程安全性
			NULL,           // 默认线程安全性
			FALSE,          // 指定当前进程内的句柄不可以被子进程继承
			CREATE_NEW_CONSOLE, // 为新进程创建一个新的控制台窗口
			NULL,           // 使用本进程的环境变量
			NULL,           // 使用本进程的驱动器和目录
			&si,
			&pi);
		delete(szCommandLine);

		if (bRet)
		{
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
		}

		DWORD r = WaitForSingleObject(hWaitEvent, 10000);		//	等待进程启动完成
		CloseHandle(hWaitEvent);

		if (r == WAIT_OBJECT_0)									//	进程启动成功
		{
			if (hSurvThread == NULL)
			{
				FHostNotifierSync::CallHostFunc("OnRequestPID", 8000);
				dCatchedTCpid = pAppDatas->dTCpid;
				hSurvThread = (HANDLE)_beginthread(TCsurvFunc, 0, this);
			}
		}
	}

	void SendInitData(const uint32_t& iFileSuccess, const uint32_t& iFileFailed, const uint32_t& iFileCount)
	{
		if (bTCAlive != true)
		{
			return;
		}
		pProcInfo->FileSuccess = iFileSuccess;
		pProcInfo->FileFailed = iFileFailed;
		pProcInfo->FileCount = iFileCount;
		strcpy(pProcInfo->ProcMessage, "");
		FHostNotifierSync::CallHostFunc("OnInitParams", 8000);
	}
	void SendProcState(const EProcState::Type& iState)
	{
		if (bTCAlive != true)
		{
			return;
		}
		pProcInfo->ProcState = iState;
		FHostNotifierSync::CallHostFunc("OnStateChanged", 8000);
	}
	void SendProgress(const uint32_t& iFileSuccess, const uint32_t& iFileFailed, const char* iProcLog)
	{
		if (bTCAlive != true)
		{
			return;
		}
		pProcInfo->FileSuccess = iFileSuccess;
		pProcInfo->FileFailed = iFileFailed;
		strcpy(pProcInfo->ProcMessage, iProcLog);
		FHostNotifierSync::CallHostFunc("OnProgress", 8000);
	}
	void SendLog(const char* iProcLog)
	{
		if (bTCAlive != true)
		{
			return;
		}
		strcpy(pProcInfo->ProcMessage, iProcLog);
		FHostNotifierSync::CallHostFunc("OnReceiveLog", 8000);
	}

	void StartNewTask();

	static void __cdecl TCsurvFunc(void* lParam)
	{
		FAutoArranger* aa = (FAutoArranger*)lParam;
		HANDLE hTC = OpenProcess(PROCESS_ALL_ACCESS, 0, aa->dCatchedTCpid);
		if (hTC != NULL)
		{
			aa->bTCAlive = true;
			WaitForSingleObject(hTC, INFINITE);
			aa->bTCAlive = false;
			CloseHandle(hTC);
			aa->hSurvThread = NULL;
		}
	}


public:

	FAutoArranger() :
		pProcInfo(PROCINFONAME),
		pAppDatas(APPDATASNAME),
		AppConfigs(GetConfigFilePath()),
		ProgressIndex(GetProgressFilePath())
	{
		boost::filesystem::path configPath = GetConfigFilePath();
		AppConfigs.LoadFromFile();

		//	注册IPC通知
		Notifier.BindHostFunc("StopThisRow", std::bind(&FAutoArranger::StopThisRow, this));
		Notifier.BindHostFunc("ResumeProcessing", std::bind(&FAutoArranger::ResumeProcessing, this));
		Notifier.BindHostFunc("StopAtonce", std::bind(&FAutoArranger::StopAtonce, this));
		NotifierSync.BindHostFunc("WritePSpid", std::bind(&FAutoArranger::WritePSpid, this));
	}

	~FAutoArranger()
	{

	}



	/// Run the plug-in from this routine
	SPErr DoIt(PSActionsPlugInMessage * message);

	void ArrangePics();

	inline std::vector<std::string>const & GetDocumentTitles() const
	{
		return DocTitles;
	}

	inline std::vector<std::string> const & GetUnitList() const
	{
		return UnitsList;
	}



public:

	FAppConfigs AppConfigs;
	FProgressIndex ProgressIndex;

	std::shared_ptr<FConfigDialog> ConfigDialog;
	std::shared_ptr<FRecDlg> RecDialog;

private:

	PSActionsPlugInMessage * pluginmessage;
	SPErr error = 0;
	static std::vector<std::string> UnitsList;
	std::vector<std::string> DocTitles;
	FHostNotifier Notifier;
	FHostNotifierSync NotifierSync;
	FFileMappingPtr<FProcInfo> pProcInfo;
	FFileMappingPtr<FAppDataTs> pAppDatas;
	HANDLE hMu;
	HANDLE hSurvThread = NULL;
	DWORD dCatchedTCpid = NULL;
	bool bTCAlive = false;
	bool bStopThisRow = false;
	bool bStopAtonce = false;

};
#endif
// end

