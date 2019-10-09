#pragma once

#include "membuf.h"
#include <windows.h>
#include <process.h>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <functional>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>

#define PROCINFONAME "fmpProcInfo"
#define APPDATASNAME "pStCShaRdAt"
#define WAITFORTCINIT "EVenT_WaitforTCInit"

class EProcState
{
public:

	enum Type
	{
		Processing = 0,
		Pending,
		Stopping,
		Stoped,
		Finished,
		MAX = 99,
	};

	static char* GetName(const Type& iType)
	{
		switch (iType)
		{
		case EProcState::Processing:
			return "处理中";
			break;
		case EProcState::Pending:
			return "即将停止";
			break;
		case EProcState::Stopping:
			return "即将停止";
			break;
		case EProcState::Stoped:
			return "已停止";
			break;
		case EProcState::Finished:
			return "任务结束";
			break;
		default:
			return "";
			break;
		}
	}

};

struct FProcInfo
{
	uint32_t FileSuccess = 0;
	uint32_t FileFailed = 0;
	uint32_t FileCount = 0;
	EProcState::Type ProcState = EProcState::MAX;
	char ProcMessage[1024] = { 0, };
};

struct FAppDataTs
{
	DWORD dPSpid = NULL;
	DWORD dTCpid = NULL;
};

template<typename T>
class FFileMappingPtr
{
public:

	FFileMappingPtr(LPCSTR _MappingObjName)
	{
		DWORD e = 0;
		hMapFile = OpenFileMapping(
			FILE_MAP_ALL_ACCESS,			// read/write access
			FALSE,							// do not inherit the name
			_MappingObjName);		// name of mapping object
		e = GetLastError();

		if (e == 0)
		{
			pBuf = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(T));
		}
		else
		{
			hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(T), _MappingObjName);
			e = GetLastError();
			if (e == 0)
			{
				pBuf = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(T));
				new(pBuf) T();
			}
			else
			{
				throw e;
			}
		}
	}
	FFileMappingPtr(LPCSTR _MappingObjName, const T& _Ref)
	{
		DWORD e = 0;
		hMapFile = OpenFileMapping(
			FILE_MAP_ALL_ACCESS,			// read/write access
			FALSE,							// do not inherit the name
			_MappingObjName);		// name of mapping object
		e = GetLastError();

		if (e == 0)
		{
			pBuf = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(T));
		}
		else
		{
			hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(T), _MappingObjName);
			e = GetLastError();
			if (e == 0)
			{
				pBuf = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(T));
				CopyMemory(pBuf, &_Ref, sizeof(T));
			}
			else
			{
				throw e;
			}
		}
	}

	~FFileMappingPtr()
	{
		if (pBuf)
		{
			UnmapViewOfFile(pBuf);
		}
		if (hMapFile)
		{
			CloseHandle(hMapFile);
		}
	}

	T* Get()
	{
		return (T*)pBuf;
	}
	T* operator->()
	{
		return (T*)pBuf;
	}
	T& operator*()
	{
		return *(T*)pBuf;
	}

private:

	HANDLE hMapFile = NULL;
	void* pBuf = nullptr;
};



class FProcessingInfo
{
public:

	std::string msgStr;

	template<typename T>
	void SaveToMemory(T* begin, T* end)
	{
		MemoryBuffer memStream((uint8_t*)begin, (uint8_t*)end);
		boost::archive::binary_oarchive oa(memStream, boost::archive::no_header);
		oa << (*this);
	}
	template<typename T>
	void LoadFromMemory(T* begin, T* end)
	{
		MemoryBuffer memStream((uint8_t*)begin, (uint8_t*)end);
		boost::archive::binary_iarchive ia(memStream, boost::archive::no_header);
		ia >> (*this);
	}


private:

	friend class boost::serialization::access;
	// When the class Archive corresponds to an output archive, the
	// & operator is defined similar to <<.  Likewise, when the class Archive
	// is a type of input archive the & operator is defined similar to >>.
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & msgStr;
	}

};


class FHostNotifier
{
private:

#define EVENTPREFIX "eVEnt_"

	struct FThreadParam
	{
		FThreadParam(const HANDLE& iEventHandle, const std::function<void()>& iFunc) :
			EventHandle(iEventHandle),
			Func(iFunc)
		{}
		HANDLE EventHandle;
		std::function<void()> Func;
	};

	struct FNotifyData
	{
		FNotifyData() :
			hEvent(0),
			hThread(0),
			pThreadParam(nullptr)
		{}
		FNotifyData(const HANDLE& iEvent, const HANDLE& iThread, FThreadParam* ipThreadParam) :
			hEvent(iEvent),
			hThread(iThread),
			pThreadParam(ipThreadParam)
		{}
		HANDLE hEvent;
		HANDLE hThread;
		FThreadParam* pThreadParam;
	};

	static void __cdecl HostThFunc(void* iParam)
	{
		FThreadParam* pthParam = (FThreadParam*)iParam;
		while (true)
		{
			WaitForSingleObject(pthParam->EventHandle, INFINITE);
			pthParam->Func();
		}
	}

public:

	FHostNotifier()
	{}
	~FHostNotifier()
	{
		for each (auto var in NotifyDataMap)
		{
			const FNotifyData& n_data = var.second;
			CloseHandle(n_data.hThread);
			CloseHandle(n_data.hEvent);
			delete(n_data.pThreadParam);
		}
	}

	void BindHostFunc(const std::string& iFuncName, const std::function<void()>& iFuncRef)
	{
		std::string EventName(EVENTPREFIX);
		EventName.append(iFuncName);

		HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, EventName.c_str());
		DWORD e = GetLastError();
		if (e == 0)
		{
			FThreadParam* thParam = new FThreadParam(hEvent, iFuncRef);
			HANDLE hThread = (HANDLE)_beginthread(HostThFunc, 0, thParam);
			NotifyDataMap[iFuncName] = FNotifyData(hEvent, hThread, thParam);
		}
		else
		{
			CloseHandle(hEvent);
			assert(0);
		}
	}

	static void CallHostFunc(const std::string& iFuncName)
	{
		std::string EventName(EVENTPREFIX);
		EventName.append(iFuncName);
		HANDLE hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, EventName.c_str());
		SetEvent(hEvent);
		CloseHandle(hEvent);
	}

private:

	std::map<std::string, FNotifyData> NotifyDataMap;

};


class FHostNotifierSync
{
private:

#define EVENTSYNPREFIX "eVEntSyN_"
#define SEMPOSYNPREFIX "seMPOSYn_"


	struct FThreadParamSync
	{
		FThreadParamSync(const HANDLE& iEventHandle, const HANDLE& iSemaphoreHandle, const std::function<void()>& iFunc) :
			EventHandle(iEventHandle),
			SemaphoreHandle(iSemaphoreHandle),
			Func(iFunc)
		{}
		HANDLE EventHandle;
		HANDLE SemaphoreHandle;
		std::function<void()> Func;
	};

	struct FNotifyDataSync
	{
		FNotifyDataSync() :
			hEvent(0),
			hSempo(0),
			hThread(0),
			pThreadParam(nullptr)
		{}
		FNotifyDataSync(const HANDLE& iEvent, const HANDLE& iSempo, const HANDLE& iThread, FThreadParamSync* ipThreadParam) :
			hEvent(iEvent),
			hSempo(iSempo),
			hThread(iThread),
			pThreadParam(ipThreadParam)
		{}
		HANDLE hEvent;
		HANDLE hSempo;
		HANDLE hThread;
		FThreadParamSync* pThreadParam;
	};

	static void __cdecl HostThFuncSync(void* iParam)
	{
		FThreadParamSync* pthParam = (FThreadParamSync*)iParam;
		while (true)
		{
			WaitForSingleObject(pthParam->EventHandle, INFINITE);
			pthParam->Func();
			if (pthParam->SemaphoreHandle != 0)
			{
				ReleaseSemaphore(pthParam->SemaphoreHandle, 1, NULL);
			}
		}
	}

public:

	FHostNotifierSync()
	{}
	~FHostNotifierSync()
	{
		for each (auto var in NotifyDataMap)
		{
			const FNotifyDataSync& n_data = var.second;
			CloseHandle(n_data.hThread);
			CloseHandle(n_data.hSempo);
			CloseHandle(n_data.hEvent);
			delete(n_data.pThreadParam);
		}
	}

	void BindHostFunc(const std::string& iFuncName, const std::function<void()>& iFuncRef)
	{
		std::string EventName(EVENTSYNPREFIX);
		EventName.append(iFuncName);
		std::string SempoName(SEMPOSYNPREFIX);
		SempoName.append(iFuncName);

		HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, EventName.c_str());
		DWORD e = GetLastError();
		HANDLE hSempo = CreateSemaphore(NULL, 0, 1, SempoName.c_str());
		DWORD e2 = GetLastError();
		if ((e == 0) && (e2 == 0))
		{
			FThreadParamSync* pthParam = new FThreadParamSync(hEvent, hSempo, iFuncRef);
			HANDLE hThread = (HANDLE)_beginthread(HostThFuncSync, 0, pthParam);
			NotifyDataMap[iFuncName] = FNotifyDataSync(hEvent, hSempo, hThread, pthParam);
		}
		else
		{
			if (hEvent)
				CloseHandle(hEvent);
			if (hSempo)
				CloseHandle(hSempo);
			assert(0);
		}
	}

	static void CallHostFunc(const std::string& iFuncName, const uint32_t& MaxWaitingTime = INFINITE)
	{
		std::string EventName(EVENTSYNPREFIX);
		EventName.append(iFuncName);
		std::string SempoName(SEMPOSYNPREFIX);
		SempoName.append(iFuncName);

		HANDLE hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, EventName.c_str());
		HANDLE hSempo = OpenSemaphore(SEMAPHORE_ALL_ACCESS, 0, SempoName.c_str());
		SetEvent(hEvent);
		if (hSempo != 0)
		{
			WaitForSingleObject(hSempo, MaxWaitingTime);
			CloseHandle(hSempo);
		}
		CloseHandle(hEvent);
	}

private:

	std::map<std::string, FNotifyDataSync> NotifyDataMap;

};
