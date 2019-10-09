
//-------------------------------------------------------------------------------
//	Includes 
//-------------------------------------------------------------------------------
#include "AutoArranger.h"
#include "MyUI.h"
#include "PITerminology.h"
#include "PIStringTerminology.h"

std::vector<std::string> FAutoArranger::UnitsList = { "像素","英寸","厘米","毫米" };

//-------------------------------------------------------------------------------
//	Globals
//-------------------------------------------------------------------------------

// everyone needs access to the sPSBasic pointer 
SPBasicSuite * sSPBasic = NULL;
SPPluginRef	gPlugInRef = NULL;
HWND hPSwnd = NULL;
HMODULE h_photoshop;

// there is a problem with the SuspendHistory() so we
// need a global error value to solve the problem
SPErr gError = kSPNoError;


struct FPSInstanceQuery
{
	HINSTANCE ReqInstance;
	HWND ResultHWND;
	const char* psName = "Photoshop";
	FPSInstanceQuery(const HINSTANCE& iInst) :
		ReqInstance(iInst),
		ResultHWND(NULL)
	{}

};

BOOL CALLBACK EnumProcessWindowsProc(HWND hwnd, LPARAM lParam)
{
	char namebuf[255];
	FPSInstanceQuery* Query = reinterpret_cast<FPSInstanceQuery*>(lParam);
	HINSTANCE windowInst = GetWindowInstance(hwnd);

	GetClassName(hwnd, namebuf, 255);
	if (strcmp(namebuf, Query->psName) == 0)
	{
		if (windowInst == Query->ReqInstance)
		{
			Query->ResultHWND = hwnd;
			return false;
		}
	}
	return true;
}

void GetHWNDFromInstance(const HINSTANCE& iInstance, HWND& OutHWND)
{
	std::shared_ptr<FPSInstanceQuery> Query = std::make_shared<FPSInstanceQuery>(iInstance);
	EnumWindows(EnumProcessWindowsProc, (LPARAM)Query.get());
	OutHWND = Query->ResultHWND;
}

std::shared_ptr<FAutoArranger> AutoArranger = 0;

//-------------------------------------------------------------------------------
//
//	AutoPluginMain
//
//	All calls to the plug-in module come through this routine.
//	It must be placed first in the resource.  To achieve this,
//	most development systems require this be the first routine
//	in the source.
//
//	The entrypoint will be "pascal void" for Macintosh,
//	"void" for Windows.
//
//-------------------------------------------------------------------------------
DLLExport SPAPI SPErr AutoPluginMain(
	const char* caller,	// who is calling
	const char* selector, // what do they want
	void* message	// what is the message
)
{
	SPErr error = kSPNoError;

	try
	{
		//all messages contain a SPMessageData*
		SPMessageData* basicMessage;
		basicMessage = (SPMessageData*)message;
		sSPBasic = basicMessage->basic;
		gPlugInRef = basicMessage->self;

		if (!AutoArranger)
		{
			char psfilename[1024];
			GetModuleFileName(NULL, psfilename, 1024);
			boost::filesystem::path psPath(psfilename);

			h_photoshop = GetModuleHandle(psPath.filename().string().c_str());
			GetHWNDFromInstance(h_photoshop, hPSwnd);

			AutoArranger = std::make_shared<FAutoArranger>();
		}

		// check for SP interface callers
		if (sSPBasic->IsEqual(caller, kSPInterfaceCaller))
		{

		}
		// Photoshop is calling us
		if (sSPBasic->IsEqual(caller, kPSPhotoshopCaller))
		{
			// the one and only message 
			if (sSPBasic->IsEqual(selector, kPSDoIt))
			{
				// now that we know more we can cast the message to a PSActionsPlugInMessage*
				PSActionsPlugInMessage* actionsMessage = (PSActionsPlugInMessage*)message;
				error = AutoArranger->DoIt(actionsMessage);
			}
		}
	}

	catch (...)
	{
		if (error == 0)
			error = kSPBadParameterError;
	}


	return error;
}


void FAutoArranger::StartNewTask()
{
	if (!ConfigDialog)
	{
		return;
	}
	int32 item = ConfigDialog->Modal(gPlugInRef, nullptr, IDD_DIALOG1, &hPSwnd);
	if (item == IDC_StartTask)
	{
		AppConfigs.SaveToFile();
		ProgressIndex.WriteProgress(0);		//	进度重新开始
		CreateTaskControl();
		ArrangePics();
	}
	else if (item == IDC_Close)
	{
		AppConfigs.SaveToFile();
	}
}

SPErr FAutoArranger::DoIt(PSActionsPlugInMessage * message)
{
	if (!ConfigDialog)
	{
		ConfigDialog = std::make_shared<FConfigDialog>(this);
	}
	if (!RecDialog)
	{
		RecDialog = std::make_shared<FRecDlg>(this);
	}

	UpdateDocumentTitles();

	int32 pgIndex = ProgressIndex.ReadProgress();
	if (pgIndex > 0)
	{
		size_t rem = AppConfigs.TaskFiles.size() - (size_t)pgIndex;
		stringstream ss;
		ss << "上一次任务还有：" << rem << "个 未处理的文件，是否继续上次进度？";
		if (MessageBox(hPSwnd, ss.str().c_str(), TEXT("信息"), MB_YESNO | MB_ICONQUESTION) == IDYES)
		{
			int32 item = RecDialog->Modal(gPlugInRef, nullptr, IDD_DIALOG2, &hPSwnd);
			if (item == IDC_ApplyRecParams)
			{
				CreateTaskControl();
				ArrangePics();
			}
		}
		else
		{
			StartNewTask();
		}
	}	
	else
	{
		StartNewTask();
	}
	
	return (error);
}

void recuFiles(const boost::filesystem::path& src_path, vector<string>& FilePathList)
{
	boost::filesystem::directory_iterator end;				//迭代器的终点
	boost::filesystem::directory_iterator dir(src_path);	//迭代器的起点

	for (dir; dir != end; dir++)
	{
		if (is_directory(*dir))
		{
			recuFiles(*dir, FilePathList); //检查路径是否与目录对应（是文件夹则递归）
		}
		else
		{
			boost::filesystem::path p(*dir);
			FilePathList.emplace_back(p.string());
		}
	}
}

void FAutoArranger::ArrangePics()
{
	bStopAtonce = false;
	bStopThisRow = false;
	int32 docNum = FApplication::GetNumberOfDocuments();
	int32& TargetDocIndex = AppConfigs.TargetDocIndex;
	if (TargetDocIndex == 0 || TargetDocIndex > docNum)
	{
		//throw string("输出目标文档指定错误");
		SendLog("输出目标文档指定错误\r\n");
		return;
	}
	FDocument TargetDocument(TargetDocIndex);
	int32 TargetDocID = TargetDocument.GetDocumentID();
	double TargetDocResolution = TargetDocument.GetResolution();
	double doc_pixels_w = 0;
	double doc_pixels_h = 0;
	TargetDocument.GetSize(doc_pixels_w, doc_pixels_h);

	double doc_origin_x = doc_pixels_w * 0.5l;
	double doc_origin_y = doc_pixels_h * 0.5l;

	uint32 i;		//	文件列表处理进度
	int32 pg = ProgressIndex.ReadProgress();
	if (pg < 0)
	{
		i = 0;
	}
	else
	{
		i = (uint32)pg;
	}

	bool cellsizeValid = false;
	double cell_w = 0;
	double cell_h = 0;
	uint32 cols = 0;
	uint32 rows = 0;
	double space_x = ConvertToPixels(TargetDocIndex, AppConfigs.SpaceHorizon, (EUnit::Type)AppConfigs.SpcSizeUnit);
	double space_y = ConvertToPixels(TargetDocIndex, AppConfigs.SpaceVertical, (EUnit::Type)AppConfigs.SpcSizeUnit);


	uint32 c_succ = 0;	//	已成功处理的文档数量
	uint32 c_fail = 0;
	std::vector<std::string>& file_list = AppConfigs.TaskFiles;
	uint32 FileCount = (uint32)file_list.size() - i;
	const uint32 FileListCount = (uint32)file_list.size();
	FExitReasons ExitReasons;
	std::stringstream sProcessLog;

	SendInitData(c_succ, c_fail, FileCount);
	SendProcState(EProcState::Processing);

	sProcessLog << "~~~~~";
	sProcessLog << ((i == 0) ? " 已启动排布任务" : " 已恢复上次进度");
	sProcessLog << ((i == 0) ? ": 文件总数: " : ": 剩余数量: ") << FileCount;
	sProcessLog << " ~~~~~\r\n";
	SendLog(sProcessLog.str().c_str());

	for (; i < FileListCount; ++i)
	{
		sProcessLog.clear();
		sProcessLog.str("");
		if (bStopAtonce)
		{
			SendProcState(EProcState::Stoped);
			bStopAtonce = false;
			break;
		}
		if (bStopThisRow)
		{
			if (cols != 0)
			{
				if (c_succ % cols == 0)
				{
					SendProcState(EProcState::Stoped);
					bStopThisRow = false;
					break;
				}
			}
		}
		sProcessLog << ">\"" << file_list[i] << "\"";			//	输出文件名
		uint32 newDocID = (uint32)OpenDocument(file_list[i]);
		if (newDocID == 0)										//	文档打开失败
		{
			c_fail += 1;
			sProcessLog << ": 文档打开失败\r\n";
			SendProgress(c_succ, c_fail, sProcessLog.str().c_str());
			continue;
		}

		FDocument openedDoc(FDocument::MakeByID(newDocID));
		openedDoc.FlattenImage();								//	拼合图层
		if (openedDoc.GetResolution() != TargetDocResolution)	//	和输出文档统一分辨率
		{
			openedDoc.SetResolution(TargetDocResolution);
		}

		if (cellsizeValid != true)								//	自动/手动设置输入文档尺寸
		{
			if (AppConfigs.AutoDetect)
			{
				double opDoc_pixels_w = 0;
				double opDoc_pixels_h = 0;
				openedDoc.GetSize(opDoc_pixels_w, opDoc_pixels_h);
				cell_w = opDoc_pixels_w;
				cell_h = opDoc_pixels_h;
			}
			else
			{
				cell_w = ConvertToPixels(TargetDocIndex, AppConfigs.ManualWidth, (EUnit::Type)AppConfigs.PicSizeUnit);
				cell_h = ConvertToPixels(TargetDocIndex, AppConfigs.ManualHeight, (EUnit::Type)AppConfigs.PicSizeUnit);
			}

			cols = (uint32)(doc_pixels_w / (cell_w + space_x));
			rows = (uint32)(doc_pixels_h / (cell_h + space_y));

			cellsizeValid = true;
		}

		uint32 cell_x = c_succ % cols;				//	当前图像排布位置（网格）
		uint32 cell_y = c_succ / cols;
		if (cell_y >= rows)							//	已经超出文档边界
		{
			ExitReasons.SetFlag(EExitReason::PageFull, true);
			openedDoc.CloseDocument();
			break;
		}

		double pos_x = cell_w * 0.5l + (double)cell_x * (cell_w + space_x);
		double pos_y = cell_h * 0.5l + (double)cell_y * (cell_h + space_y);

		auto Layer_bg = openedDoc.GetBackgroundLayer();
		if (!Layer_bg)
		{
			c_fail += 1;
			sProcessLog << ": 该文档无背景图层\r\n";
			SendProgress(c_succ, c_fail, sProcessLog.str().c_str());
			continue;
		}
		/*string s = Layer_bg->GetLayerName();
		int32 ind = Layer_bg->GetLayerIndex();*/

		bool dupSucc = Layer_bg->DuplicateLayerTo(TargetDocument, &lexical_cast<std::string>(c_succ));
		openedDoc.CloseDocument();
		if (!dupSucc)
		{
			c_fail += 1;
			sProcessLog << ": 图层复制失败\r\n";
			SendProgress(c_succ, c_fail, sProcessLog.str().c_str());
			continue;
		}
		auto duped_layer = TargetDocument.GetSelectedLayer();
		duped_layer->MoveLayer(pos_x - doc_origin_x, pos_y - doc_origin_y);


		sProcessLog << "\r\n";
		c_succ++;
		SendProgress(c_succ, c_fail, sProcessLog.str().c_str());
	}

	if (i < FileListCount)					//	检查是否处理完文件列表，保存进度
	{
		ProgressIndex.WriteProgress(i);
	}
	else
	{
		ProgressIndex.WriteProgress(0);
	}

	if (c_succ == FileCount)
	{
		ExitReasons.SetFlag(EExitReason::AllFinished, true);
		ExitReasons.SetFlag(EExitReason::Partly, false);
	}
	else
	{
		ExitReasons.SetFlag(EExitReason::AllFinished, false);
		ExitReasons.SetFlag(EExitReason::Partly, true);
	}
	sProcessLog.clear();
	sProcessLog.str("");
	sProcessLog << "已停止：" << ExitReasons.GetAllReasons() << "\r\n";
	SendLog(sProcessLog.str().c_str());

	sProcessLog.clear();
	sProcessLog.str("");
	sProcessLog << "======";
	sProcessLog << " 处理: 成功: " << c_succ << ", 失败: " << c_fail << ", 未处理: " << FileCount - c_succ - c_fail;
	sProcessLog << " ======\r\n\r\n";
	SendLog(sProcessLog.str().c_str());

	SendProcState(EProcState::Finished);

}


