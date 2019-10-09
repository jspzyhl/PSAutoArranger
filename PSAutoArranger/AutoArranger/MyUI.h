
//-------------------------------------------------------------------------------
//	Includes 
//-------------------------------------------------------------------------------
#ifndef __MYUI_H__
#define __MYUI_H__

#include "AutoArranger.h"
#include "PIUI.h"
#include "resource.h"

#include "Encap.h"

#include <boost/lexical_cast.hpp>

using namespace boost;


extern SPBasicSuite* sSPBasic;

// forward declare
class FAutoArranger;


void wstring2string(const std::wstring& ws, std::string& s);
HRESULT callOpenFolderDialog(HWND hwnd, std::string& FolderPath);



// class to control our dialog
// we override the Init, Notify, and Destroy routines
class FConfigDialog : public PIDialog
{

public:

	FConfigDialog(FAutoArranger *instance) : PIDialog(),
		_instance(instance),
		AppConfigs(&(instance->AppConfigs))
	{

	}

	~FConfigDialog()
	{

	}

	virtual void Init(void) override;
	virtual void Notify(int32 item) override;


private:

	static void recuAllFiles(const boost::filesystem::path& src_path, vector<string>& FilePathList)
	{
		boost::filesystem::directory_iterator end;				//���������յ�
		boost::filesystem::directory_iterator dir(src_path);	//�����������

		for (dir; dir != end; dir++)
		{
			if (is_directory(*dir))
			{
				recuAllFiles(*dir, FilePathList); //���·���Ƿ���Ŀ¼��Ӧ�����ļ�����ݹ飩
			}
			else
			{
				boost::filesystem::path p(*dir);
				FilePathList.emplace_back(p.string());
			}
		}
	}

	void UpdateCheckBox1Related()
	{
		if (CB_AutoDet.GetChecked())
		{
			EB_ManualWidth.SetEnabled(false);
			EB_ManualHeight.SetEnabled(false);
			CMB_PicUnit.SetEnabled(false);
		}
		else
		{
			EB_ManualWidth.SetEnabled(true);
			EB_ManualHeight.SetEnabled(true);
			CMB_PicUnit.SetEnabled(true);
		}
	}

	bool CollectSimpleParams()
	{
		PIDialogPtr dialog = GetDialog();

		AppConfigs->AutoDetect = CB_AutoDet.GetChecked();

		if (!CB_AutoDet.GetChecked())
		{
			string mw;
			EB_ManualWidth.GetText(mw);
			string mh;
			EB_ManualHeight.GetText(mh);
			double dmw = 0;
			double dmh = 0;
			try
			{
				dmw = boost::lexical_cast<double>(mw.c_str());
				dmh = boost::lexical_cast<double>(mh.c_str());
			}
			catch (const boost::bad_lexical_cast &e)
			{
				stringstream ss;
				ss << "��ЧͼƬ�ߴ�: " << e.what();
				MessageBox(dialog, ss.str().c_str(), TEXT("��������"), MB_OK);
				return false;
			}

			if ((dmw <= 0) || (dmh <= 0))
			{
				MessageBox(dialog, TEXT("��ЧͼƬ�ߴ磺ͼƬ�ߴ粻��С�ڵ���0"), TEXT("��������"), MB_OK);
				return false;
			}
			else
			{
				AppConfigs->ManualWidth = dmw;
				AppConfigs->ManualHeight = dmh;
			}
		}

		string sh;
		EB_SpaceHorizon.GetText(sh);
		string sv;
		EB_SpaceVertical.GetText(sv);
		double dsh = 0;
		double dsv = 0;

		try
		{
			dsh = boost::lexical_cast<double>(sh.c_str());
			dsv = boost::lexical_cast<double>(sv.c_str());
		}
		catch (const boost::bad_lexical_cast &e)
		{
			stringstream ss;
			ss << "��Ч����ߴ�: " << e.what();
			MessageBox(dialog, ss.str().c_str(), TEXT("��������"), MB_OK);
			return false;
		}

		if ((dsh < 0) || (dsv < 0))
		{
			MessageBox(dialog, TEXT("��Ч����ߴ�: �������С��0"), TEXT("��������"), MB_OK);
			return false;
		}
		else
		{
			AppConfigs->SpaceHorizon = dsh;
			AppConfigs->SpaceVertical = dsv;
		}

		int32 PicUnitIndex;
		CMB_PicUnit.GetCurrentIndex(PicUnitIndex);
		int32 SpcUnitIndex;
		CMB_SpcUnit.GetCurrentIndex(SpcUnitIndex);
		AppConfigs->PicSizeUnit = PicUnitIndex;
		AppConfigs->SpcSizeUnit = SpcUnitIndex;

		int32 OutputDocIndex;
		CMB_OutputDoc.GetCurrentIndex(OutputDocIndex);
		AppConfigs->TargetDocIndex = OutputDocIndex + 1;

		return true;
	}


	bool CollectParams()
	{
		PIDialogPtr dialog = GetDialog();
		string folderPathStr;
		EB_FolderPath.GetText(folderPathStr);
		filesystem::path folderPath(folderPathStr.c_str());
		if (filesystem::is_directory(folderPath) && filesystem::exists(folderPath))
		{
			//AppConfigs->CurrentFileIndex = 0;
			AppConfigs->TaskFiles.clear();
			recuAllFiles(folderPath, AppConfigs->TaskFiles);
		}
		else
		{
			MessageBox(dialog, TEXT("��������ȷ·��"), TEXT("��������"), MB_OK);
			return false;
		}

		AppConfigs->AutoDetect = CB_AutoDet.GetChecked();

		if (!CB_AutoDet.GetChecked())
		{
			string mw;
			EB_ManualWidth.GetText(mw);
			string mh;
			EB_ManualHeight.GetText(mh);
			double dmw = 0;
			double dmh = 0;
			try
			{
				dmw = boost::lexical_cast<double>(mw.c_str());
				dmh = boost::lexical_cast<double>(mh.c_str());
			}
			catch (const boost::bad_lexical_cast &e)
			{
				stringstream ss;
				ss << "��ЧͼƬ�ߴ�: " << e.what();
				MessageBox(dialog, ss.str().c_str(), TEXT("��������"), MB_OK);
				return false;
			}

			if ((dmw <= 0) || (dmh <= 0))
			{
				MessageBox(dialog, TEXT("��ЧͼƬ�ߴ磺ͼƬ�ߴ粻��С�ڵ���0"), TEXT("��������"), MB_OK);
				return false;
			}
			else
			{
				AppConfigs->ManualWidth = dmw;
				AppConfigs->ManualHeight = dmh;
			}
		}

		string sh;
		EB_SpaceHorizon.GetText(sh);
		string sv;
		EB_SpaceVertical.GetText(sv);
		double dsh = 0;
		double dsv = 0;

		try
		{
			dsh = boost::lexical_cast<double>(sh.c_str());
			dsv = boost::lexical_cast<double>(sv.c_str());
		}
		catch (const boost::bad_lexical_cast &e)
		{
			stringstream ss;
			ss << "��Ч����ߴ�: " << e.what();
			MessageBox(dialog, ss.str().c_str(), TEXT("��������"), MB_OK);
			return false;
		}

		if ((dsh < 0) || (dsv < 0))
		{
			MessageBox(dialog, TEXT("��Ч����ߴ�: �������С��0"), TEXT("��������"), MB_OK);
			return false;
		}
		else
		{
			AppConfigs->SpaceHorizon = dsh;
			AppConfigs->SpaceVertical = dsv;
		}

		int32 PicUnitIndex;
		CMB_PicUnit.GetCurrentIndex(PicUnitIndex);
		int32 SpcUnitIndex;
		CMB_SpcUnit.GetCurrentIndex(SpcUnitIndex);
		AppConfigs->PicSizeUnit = PicUnitIndex;
		AppConfigs->SpcSizeUnit = SpcUnitIndex;

		int32 OutputDocIndex;
		CMB_OutputDoc.GetCurrentIndex(OutputDocIndex);
		AppConfigs->TargetDocIndex = OutputDocIndex + 1;

		return true;
	}

	bool BeginTask()
	{
		if (CollectParams())
		{
			std::vector<std::string>& taskfiles = AppConfigs->TaskFiles;
			stringstream sfiles;
			for (size_t i = 0, n = (taskfiles.size() >= 10) ? 10 : taskfiles.size(); i < n; ++i)
			{
				sfiles << taskfiles[i] << std::endl;
			}
			if (taskfiles.size() > 10)
			{
				sfiles << "......" << std::endl;
			}

			const vector<string>& Titles = _instance->GetDocumentTitles();

			stringstream sMsg;
			sMsg << "���Ŀ���ĵ�����" << Titles[AppConfigs->TargetDocIndex - 1] << std::endl;
			sMsg << "�������ļ�����" << taskfiles.size() << std::endl << std::endl;
			sMsg << "�����ļ�·�����£�" << std::endl;
			sMsg << sfiles.str();
			sMsg << std::endl << std::endl;
			sMsg << "�Ƿ�������ʼ����" << std::endl;

			// ȷ����ʼ�� �رյ�ǰ�Ի��� ���ضԻ�����ô����к�ߵĴ���
			if (MessageBox(GetDialog(), sMsg.str().c_str(), TEXT("��Ϣ"), MB_YESNO | MB_ICONQUESTION) == IDYES)
			{
				return true;
			}
		}
		return false;
	}


private:

	FAutoArranger* _instance;
	FAppConfigs* AppConfigs;

	PIText EB_FolderPath;
	PIText EB_ManualWidth;
	PIText EB_ManualHeight;
	PIText EB_SpaceHorizon;
	PIText EB_SpaceVertical;

	PICheckBox CB_AutoDet;

	PIComboBox CMB_PicUnit;
	PIComboBox CMB_SpcUnit;
	PIComboBox CMB_OutputDoc;

};

class FRecDlg : public PIDialog
{
public:

	FRecDlg(FAutoArranger* _instance) :
		ParentInst(_instance)
	{

	}

	void Init() override
	{
		PIDialogPtr dialog = GetDialog();
		PIItem item;
		item = PIGetDialogItem(dialog, IDC_COMBO1);
		CMB_OutputDoc.SetItem(item);

		const vector<string>& TitleList = ParentInst->GetDocumentTitles();
		for (auto it = TitleList.begin(), itn = TitleList.end(); it != itn; ++it)
		{
			CMB_OutputDoc.AppendItem(*it);
		}
		CMB_OutputDoc.SetCurrentSelection(0);
	}

	void Notify(int32 item) override
	{
		switch (item)
		{
		case IDC_ApplyRecParams:
		{
			int32 OutputDocIndex;
			CMB_OutputDoc.GetCurrentIndex(OutputDocIndex);
			ParentInst->AppConfigs.TargetDocIndex = OutputDocIndex + 1;
			EndDialog(GetDialog(), item);
			break;
		}
		case IDC_Close:
		{
			EndDialog(GetDialog(), item);
			break;
		}

		}
	}

private:
	FAutoArranger* ParentInst;
	PIComboBox CMB_OutputDoc;


};

class FTestDialog : public PIDialog
{
public:
	FTestDialog(SPPluginRef ipluginRef) :
		pluginRef(ipluginRef)
	{}
	~FTestDialog()
	{}

	void Init() override
	{
		PIDialogPtr dialog = GetDialog();
		PIItem item;
		item = PIGetDialogItem(dialog, IDC_EDIT1);
		Edit1.SetItem(item);
		item = PIGetDialogItem(dialog, IDC_EDIT2);
		Edit2.SetItem(item);
		item = PIGetDialogItem(dialog, IDC_EDIT3);
		Edit3.SetItem(item);
	}

	void Notify(int32 item) override
	{
		switch (item)
		{
		case IDC_BUTTON1:
		{
			string pStr;
			Edit1.GetText(pStr);
			int32 docid = OpenDocument(pStr);
			FDocument newdoc = FDocument::MakeByID(docid);
			ShowMessage(newdoc.GetTitle());

		}
		case IDC_BUTTON2:
		{
			FDocument doc1(1);
			FDocument docc(FDocument::GetCurrent());
			string lyname;
			Edit2.GetText(lyname);
			auto l = docc.GetLayerByName(lyname);
			string newlname;
			Edit3.GetText(newlname);


			break;
		}

		default:
			break;
		}

	}


private:

	SPPluginRef pluginRef;
	PIText Edit1;
	PIText Edit2;
	PIText Edit3;
};





#endif
// end 


