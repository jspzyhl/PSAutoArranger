
//-------------------------------------------------------------------------------
//	Includes 
//-------------------------------------------------------------------------------

#include <iostream>
#include <sstream>
#include "MyUI.h"

#if !__LP64__


//-------------------------------------------------------------------------------
//
//	MyTestDialog::Init
//	
//
//-------------------------------------------------------------------------------
void FConfigDialog::Init(void)
{

	PIDialogPtr dialog = GetDialog();
	PIItem item;
	std::string str;
	item = PIGetDialogItem(dialog, IDC_EDIT1);
	EB_FolderPath.SetItem(item);
	EB_FolderPath.SetText("");

	item = PIGetDialogItem(dialog, IDC_EDIT2);
	EB_ManualWidth.SetItem(item);
	str = boost::lexical_cast<std::string>(AppConfigs->ManualWidth);
	EB_ManualWidth.SetText(str);

	item = PIGetDialogItem(dialog, IDC_EDIT3);
	EB_ManualHeight.SetItem(item);
	str = boost::lexical_cast<std::string>(AppConfigs->ManualWidth);
	EB_ManualHeight.SetText(str);

	item = PIGetDialogItem(dialog, IDC_EDIT4);
	EB_SpaceHorizon.SetItem(item);
	str = boost::lexical_cast<std::string>(AppConfigs->SpaceHorizon);
	EB_SpaceHorizon.SetText(str);

	item = PIGetDialogItem(dialog, IDC_EDIT5);
	EB_SpaceVertical.SetItem(item);
	str = boost::lexical_cast<std::string>(AppConfigs->SpaceVertical);
	EB_SpaceVertical.SetText(str);

	//	单位选择下拉框
	item = PIGetDialogItem(dialog, IDC_COMBO1);
	CMB_PicUnit.SetItem(item);
	CMB_PicUnit.SetReadOnly(true);
	item = PIGetDialogItem(dialog, IDC_COMBO2);
	CMB_SpcUnit.SetItem(item);
	CMB_SpcUnit.SetReadOnly(true);
	const vector<string>& UnitList = _instance->GetUnitList();
	for (auto it = UnitList.begin(), itn = UnitList.end(); it != itn; ++it)
	{
		CMB_PicUnit.AppendItem(*it);
		CMB_SpcUnit.AppendItem(*it);
	}
	CMB_PicUnit.SetCurrentSelection(AppConfigs->PicSizeUnit);
	CMB_SpcUnit.SetCurrentSelection(AppConfigs->SpcSizeUnit);

	item = PIGetDialogItem(dialog, IDC_CHECK1);
	CB_AutoDet.SetItem(item);
	CB_AutoDet.SetChecked(AppConfigs->AutoDetect);
	UpdateCheckBox1Related();

	item = PIGetDialogItem(dialog, IDC_COMBO3);
	CMB_OutputDoc.SetItem(item);


	const vector<string>& TitleList = _instance->GetDocumentTitles();
	for (auto it = TitleList.begin(), itn = TitleList.end(); it != itn; ++it)
	{
		CMB_OutputDoc.AppendItem(*it);
	}
	CMB_OutputDoc.SetCurrentSelection(0);


}



//-------------------------------------------------------------------------------
//
//	MyTestDialog::Notify
//	
//  Anytime something happens we get notified. It would be nice to have each
//	button with its own notify routine but you need to make statics so the 
//	compiler is happy and it's just a pain in the
//
//  We only care about changes to the buttons.
//
//-------------------------------------------------------------------------------
void FConfigDialog::Notify(int32 item)
{

	switch (item)
	{
	case IDC_BUTTON1:
	{
		string folderPath;
		callOpenFolderDialog(GetDialog(), folderPath);
		EB_FolderPath.SetText(folderPath);
		break;
	}
	case IDC_CHECK1:
	{
		UpdateCheckBox1Related();
		break;
	}
	case IDC_StartTask:
	{
		if (BeginTask())
		{
			EndDialog(GetDialog(), item);
		}
		break;
	}
	case IDC_Close:
	{
		if (CollectSimpleParams())
		{
			EndDialog(GetDialog(), item);
		}	
		break;
	}

	default:
		break;
	}

}


void wstring2string(const std::wstring& ws, std::string& s)
{
	static std::string codepage;
	if (codepage.empty())
	{
		CPINFOEX  cpinfo;
		GetCPInfoEx(CP_ACP, 0, &cpinfo);	// 获得系统当前的代码页。对于中文Windows, 
		cpinfo.CodePageName;
		codepage = "CP" + boost::lexical_cast<std::string>(cpinfo.CodePage);
	}
	s = boost::locale::conv::from_utf(ws, codepage);
}

// end 

HRESULT callOpenFolderDialog(HWND hwnd, std::string& FolderPath)
{
	HRESULT hr;
	IFileOpenDialog *pOpenFolderDialog;

	// CoCreate the dialog object.
	hr = CoCreateInstance(CLSID_FileOpenDialog,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&pOpenFolderDialog));

	if (SUCCEEDED(hr))
	{
		pOpenFolderDialog->SetOptions(FOS_PICKFOLDERS);

		hr = pOpenFolderDialog->Show(hwnd);

		if (SUCCEEDED(hr))
		{
			// Obtain the result of the user's interaction with the dialog.
			IShellItem *psiResult;
			hr = pOpenFolderDialog->GetResult(&psiResult);

			if (SUCCEEDED(hr))
			{
				// Do something with the result.
				LPWSTR pwsz = NULL;

				hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pwsz);

				if (SUCCEEDED(hr))
				{
					wstring2string(pwsz, FolderPath);
					CoTaskMemFree(pwsz);
				}
				psiResult->Release();
			}
		}
		pOpenFolderDialog->Release();
	}
	return hr;
}


#endif // #if !__LP64__
