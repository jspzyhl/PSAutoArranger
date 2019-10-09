#pragma once

#include "PIDefines.h"
#include "PIUSelect.h"
#include "SPHost.h"
#include "PIUGet.h"
#include "PIActionsPlugIn.h"
#include "PIUIHooksSuite.h"
#include "PIHandleSuite.h"
#include "ASZStringSuite.h"
#include "PIUSuites.h"
#include "PIStringTerminology.h"

#include <string>
#include <sstream>
#include <windows.h>
#include <vector>
#include <memory>
#include <thread>
#include <condition_variable>
#include <mutex>

using namespace std;

class FApplication;
class FLayer;
class FDocument;

#define UNT_Pixel 0
#define UNT_Inch 1
#define UNT_CM 2
#define UNT_MM 3


namespace EUnit
{
	enum Type
	{
		Pixel = 0,
		Inch = 1,
		CM = 2,
		MM = 3,
	};
}


namespace PEApi
{
	PIDescriptorHandle DescriptorToHandle(const PIActionDescriptor& iDesc);
	void HandleToDescriptor(const PIDescriptorHandle& idesc_h, PIActionDescriptor* oDesc);

	int32 OpenDocument(const string& DocFilePath);

	void GetBooleanByKeyName(const PIDescriptorHandle& iDesc_h, const string& iKeyName, Boolean& OutValue);
	void GetBooleanByKeyName(const PIActionDescriptor& iDesc, const string& iKeyName, Boolean& OutValue);

	void GetIntegerValueByKeyName(const PIDescriptorHandle& iDesc_h, const string& iKeyName, int32& OutValue);
	void GetIntegerValueByKeyName(const PIActionDescriptor& iDesc, const string& iKeyName, int32& OutValue);

	void GetUnitFloatByKeyName(const PIDescriptorHandle& iDesc_h, const string& iKeyName, uint32& OutUnitID, double& OutValue);
	void GetUnitFloatByKeyName(const PIActionDescriptor& iDesc, const string& iKeyName, uint32& OutUnitID, double& OutValue);

	void GetStringValueByKeyName(const PIDescriptorHandle& iDesc_h, const string& iKeyName, string& OutValue);
	void GetStringValueByKeyName(const PIActionDescriptor& iDesc, const string& iKeyName, string& OutValue);

	void GetObjectByKeyName(const PIDescriptorHandle& iDesc_h, const string& iKeynameStr, PIActionDescriptor* Out_ObjDesc);
	void GetObjectByKeyName(const PIActionDescriptor& iDesc, const string& iKeynameStr, PIActionDescriptor* Out_ObjDesc);

	void GetListByKeyName(const PIDescriptorHandle& iDesc_h, const string& iKeyName, PIActionList* OutList);
	void GetListByKeyName(const PIActionDescriptor& iDesc, const string& iKeyName, PIActionList* OutList);

	void GetApplicationDesc(PIActionDescriptor* OutDesc);

	void GetCurrentDocument(PIActionDescriptor* OutDesc);

	void GetClassRefByIndex(DescriptorClassID desiredClass, uint32 Index, PIActionReference* OutRef);

	void GetClassDescByIndex(DescriptorClassID desiredClass, uint32 Index, PIActionDescriptor* OutDesc);

	void ShowMessage(const stringstream& ss);

	void ShowMessage(const std::string& s);


	bool KeyValueToString(const PIActionDescriptor& idesc, const DescriptorKeyID& ikeyID, string& OutString);

	void GetAllKeysFromDescriptor(const PIActionDescriptor& Desc, string& OutString);

	void GetAllItemsFromList(const PIActionList& iList, string& OutString);

	void GetAllKeysFromReference(const PIActionReference& Ref, string& OutString);

	void WriteAllKeysFromReferenceTo(const PIActionDescriptor& Desc, const string& OutFilePath);

	double ConvertToPixels(const int32& iDocIndex, const double& iVal, const EUnit::Type& iUnitType);

	void OpenDocument2(const string& DocFilePath);


}

using namespace PEApi;

class FApplication
{
public:
	FApplication()
	{

	}
	~FApplication()
	{

	}

public:

	static int32 GetNumberOfDocuments()
	{
		Auto_Desc appDesc;
		GetApplicationDesc(&appDesc);
		int32 docNum = 0;
		GetIntegerValueByKeyName(appDesc.get(), "numberOfDocuments", docNum);
		return std::move(docNum);
	}




private:


};


class FDocument
{
public:

	FDocument(const PIActionDescriptor& idesc)
	{
		sPSActionDescriptor->AsHandle(idesc, &docDesc_h);
	}
	FDocument(const Auto_Desc& idesc)
	{
		sPSActionDescriptor->AsHandle(idesc.get(), &docDesc_h);
	}
	FDocument(const FDocument& idoc) :
		docDesc_h(idoc.docDesc_h)
	{

	}
	FDocument(const uint32 iDocIndex)
	{
		Auto_Ref docRef;
		Auto_Desc docDesc;
		sPSActionReference->PutIndex(docRef.get(), classDocument, iDocIndex);
		sPSActionControl->Get(&docDesc, docRef.get());
		sPSActionDescriptor->AsHandle(docDesc.get(), &docDesc_h);
	}

	~FDocument()
	{

	}

	FDocument operator=(const FDocument& iDoc)
	{
		return std::move(FDocument(iDoc));
	}

	void GetSize(double& oWidth, double& oHeight) const
	{
		uint32 unitID_w = 0;
		double doc_w = 0;
		GetUnitFloatByKeyName(docDesc_h, string("width"), unitID_w, doc_w);
		uint32 unitID_h = 0;
		double doc_h = 0;
		GetUnitFloatByKeyName(docDesc_h, string("height"), unitID_h, doc_h);
		uint32 unitID_res = 0;
		double doc_res = 0;
		GetUnitFloatByKeyName(docDesc_h, string("resolution"), unitID_res, doc_res);
		oWidth = UnitsToPixels(doc_w, (int)unitID_w, doc_res);
		oHeight = UnitsToPixels(doc_h, (int)unitID_h, doc_res);
	}
	double GetResolution();
	std::string GetTitle() const
	{
		std::string title;
		GetStringValueByKeyName(docDesc_h, "title", title);
		return std::move(title);
	}
	int32 GetIndex() const
	{
		int32 Index;
		GetIntegerValueByKeyName(docDesc_h, "itemIndex", Index);
		return  std::move(Index);
	}
	int32 GetDocumentID() const
	{
		int32 id;
		GetIntegerValueByKeyName(docDesc_h, "documentID", id);
		return std::move(id);
	}
	void GetSelectedLayers(std::vector<FLayer>& oLayerList)
	{
		PIActionList selected_layerList;
		sPSActionList->Make(&selected_layerList);
		GetListByKeyName(docDesc_h, "targetLayers", &selected_layerList);
		uint32 sel_count;
		sPSActionList->GetCount(selected_layerList, &sel_count);

		oLayerList.clear();
		for (uint32 i = 0, n = sel_count; i < n; ++i)
		{
			Auto_Ref layerRef;
			sPSActionList->GetReference(selected_layerList, i, &layerRef);
			Auto_Desc l_desc;
			sPSActionControl->Get(&l_desc, layerRef.get());
			oLayerList.emplace_back(*this, DescriptorToHandle(l_desc.get()));
		}
		sPSActionList->Free(selected_layerList);
	}

	void GetDocumentReference(PIActionReference* ioRef) const
	{
		sPSActionReference->PutIndex(*ioRef, classDocument, (uint32)GetIndex());
	}

	void Active()
	{
		Auto_Desc currDocDesc;
		GetCurrentDocument(&currDocDesc);
		int32 currdocIndex;
		GetIntegerValueByKeyName(currDocDesc.get(), "itemIndex", currdocIndex);
		int32 thisIndex = GetIndex();
		if (currdocIndex != thisIndex)
		{
			Auto_Desc resultDesc;
			Auto_Desc selectDesc;
			Auto_Ref offsetRef;
			sPSActionReference->PutOffset(offsetRef.get(), classDocument, currdocIndex - thisIndex);
			sPSActionDescriptor->PutReference(selectDesc.get(), keyNull, offsetRef.get());
			sPSActionControl->Play(&resultDesc, eventSelect, selectDesc.get(), plugInDialogSilent);
		}
	}

	std::shared_ptr<FLayer> GetSelectedLayer()
	{
		std::shared_ptr<FLayer> result;
		PIActionList selected_layerList;
		sPSActionList->Make(&selected_layerList);
		GetListByKeyName(docDesc_h, "targetLayers", &selected_layerList);
		uint32 sel_count;
		sPSActionList->GetCount(selected_layerList, &sel_count);

		if (sel_count > 0)
		{
			Auto_Ref layerRef;
			sPSActionList->GetReference(selected_layerList, 0, &layerRef);
			Auto_Desc l_desc(false);
			sPSActionControl->Get(&l_desc, layerRef.get());
			PIDescriptorHandle layer_h = DescriptorToHandle(l_desc.get());
			result = std::make_shared<FLayer>(*this, layer_h);
		}
		else
		{
			result.reset();
		}
		sPSActionList->Free(selected_layerList);

		return std::move(result);
	}

	//
	std::shared_ptr<FLayer> GetLayerByIndex(const int32& iIndex)
	{
		Auto_Desc desc;
		GetClassDescByIndex(classLayer, iIndex, &desc);
		return std::move(std::make_shared<FLayer>(*this, desc.get()));
	}
	std::shared_ptr<FLayer> GetLayerByName(const std::string& iLayerName)
	{
		Auto_Ref Queryref;
		Auto_Desc LayerDesc(false);
		sPSActionReference->PutName(Queryref.get(), classLayer, iLayerName.c_str());

		OSErr e = sPSActionControl->Get(&LayerDesc, Queryref.get());
		if (e != 0)
		{
			return 0;
		}
		return std::move(std::make_shared<FLayer>(*this, LayerDesc.get()));
	}

	Boolean hasBackgroundLayer();
	std::shared_ptr<FLayer> GetBackgroundLayer()
	{
		Boolean hasBG = hasBackgroundLayer();
		if (hasBG)
		{
			Auto_Desc desc;
			GetClassDescByIndex(classLayer, 0, &desc);
			return std::move(std::make_shared<FLayer>(*this, desc.get()));
		}
		return std::move(std::shared_ptr<FLayer>());
	}
	int32 GetNumberOfLayers()
	{
		Auto_Desc desc;
		HandleToDescriptor(docDesc_h, &desc);
		int32 lyrnum;
		GetIntegerValueByKeyName(desc.get(), string("numberOfLayers"), lyrnum);
		return std::move(lyrnum);
	}

	//	拼合可见图层（所有图层合成一个普通图层）
	void MergeVisible()
	{
		Active();
		Auto_Desc resultDesc;
		sPSActionControl->Play(&resultDesc, eventMergeVisible, NULL, plugInDialogSilent);
	}
	//	拼合图像（所有图层合成一个背景图层）
	void FlattenImage()
	{
		Active();
		Auto_Desc result;
		sPSActionControl->Play(&result, eventFlattenImage, NULL, plugInDialogSilent);
	}

	void SetResolution(const double& newResolution)
	{
		Active();
		Auto_Desc resizeDesc;
		Auto_Desc result;
		sPSActionDescriptor->PutUnitFloat(resizeDesc.get(), keyResolution, unitDensity, newResolution);
		DescriptorTypeID TypeID;
		sPSActionControl->StringIDToTypeID("automaticInterpolation", &TypeID);
		sPSActionDescriptor->PutEnumerated(resizeDesc.get(), keyInterfaceIconFrameDimmed, typeInterpolation, TypeID);
		sPSActionControl->Play(&result, eventImageSize, resizeDesc.get(), plugInDialogSilent);
		UpdateHandle();
	}

	void CloseDocument()
	{
		Active();
		Auto_Desc resultDesc;
		Auto_Desc closeDesc;
		sPSActionDescriptor->PutEnumerated(closeDesc.get(), keySaving, typeYesNo, enumNo);
		sPSActionControl->Play(&resultDesc, eventClose, closeDesc.get(), plugInDialogSilent);
	}


public:

	static FDocument GetCurrent()
	{
		Auto_Desc currdocDesc;
		GetCurrentDocument(&currdocDesc);
		return std::move(FDocument(currdocDesc));
	}

	static FDocument MakeByID(const uint32& iDocID)
	{
		Auto_Ref docRef;
		Auto_Desc docDesc;
		sPSActionReference->PutIdentifier(docRef.get(), classDocument, iDocID);
		sPSActionControl->Get(&docDesc, docRef.get());
		return std::move(FDocument(docDesc));
	}

private:

	void UpdateHandle()
	{
		Auto_Ref docRef;
		Auto_Desc docDesc;
		sPSActionReference->PutIndex(docRef.get(), classDocument, GetIndex());
		sPSActionControl->Get(&docDesc, docRef.get());
		sPSActionDescriptor->AsHandle(docDesc.get(), &docDesc_h);
	}



private:

	PIDescriptorHandle docDesc_h;


};

class FLayer
{
public:
	FLayer(FDocument& iparent, const PIActionDescriptor& iLayerDesc) :
		ParentDocument(iparent)
	{
		sPSActionDescriptor->AsHandle(iLayerDesc, &layerDesc_h);
	}
	FLayer(FDocument& iparent, const PIDescriptorHandle& ih_desc) :
		ParentDocument(iparent)
	{
		layerDesc_h = ih_desc;
	}
	~FLayer()
	{

	}

	std::string GetLayerName()
	{
		std::string name;
		Auto_Desc t_desc;
		sPSActionDescriptor->HandleToDescriptor(layerDesc_h, &t_desc);
		GetStringValueByKeyName(t_desc.get(), "name", name);
		return std::move(name);
	}
	//	LayerID是图层的固定唯一标识
	int32 GetLayerID()
	{
		int32 layerID;
		GetIntegerValueByKeyName(layerDesc_h, "layerID", layerID);
		return std::move(layerID);
	}
	//	 LayerIndex是图层栈的堆放序号，大序号图层遮盖小序号图层
	int32 GetLayerIndex()
	{
		Boolean hasBG = ParentDocument.hasBackgroundLayer();
		int32 layerIndex;
		GetIntegerValueByKeyName(layerDesc_h, "itemIndex", layerIndex);
		if (hasBG)
		{
			layerIndex -= 1;	//	此处将图层序号处理成与查找序号相同的值（与有无背景图层无关）
		}
		return std::move(layerIndex);
	}
	void GetLayerReference(PIActionReference* ioRef)
	{
		sPSActionReference->PutIdentifier(*ioRef, classLayer, (uint32)GetLayerID());
	}

	void GetAllInfo()
	{
		Auto_Desc desc;
		HandleToDescriptor(layerDesc_h, &desc);
		string s;
		GetAllKeysFromDescriptor(desc.get(), s);
		ShowMessage(s);
	}
	void GetBounds(double* oTop, double* oLeft, double* oBottom, double* oRight, double* oWidth, double* oHeight)
	{
		Auto_Desc boundsDesc;
		GetObjectByKeyName(layerDesc_h, "bounds", &boundsDesc);

		uint32 unitID;
		double val;

		if (oTop)
		{
			GetUnitFloatByKeyName(boundsDesc.get(), "top", unitID, val);
			*oTop = UnitsToPixels(val, (int)unitID, ParentDocument.GetResolution());
		}
		if (oLeft)
		{
			GetUnitFloatByKeyName(boundsDesc.get(), "left", unitID, val);
			*oLeft = UnitsToPixels(val, (int)unitID, ParentDocument.GetResolution());
		}
		if (oBottom)
		{
			GetUnitFloatByKeyName(boundsDesc.get(), "bottom", unitID, val);
			*oBottom = UnitsToPixels(val, (int)unitID, ParentDocument.GetResolution());
		}
		if (oRight)
		{
			GetUnitFloatByKeyName(boundsDesc.get(), "right", unitID, val);
			*oRight = UnitsToPixels(val, (int)unitID, ParentDocument.GetResolution());
		}
		if (oWidth)
		{
			GetUnitFloatByKeyName(boundsDesc.get(), "width", unitID, val);
			*oWidth = UnitsToPixels(val, (int)unitID, ParentDocument.GetResolution());
		}
		if (oHeight)
		{
			GetUnitFloatByKeyName(boundsDesc.get(), "height", unitID, val);
			*oHeight = UnitsToPixels(val, (int)unitID, ParentDocument.GetResolution());
		}
	}
	void GetMidPoint(double& o_x, double& o_y)
	{
		double top, left, bottom, right;
		GetBounds(&top, &left, &bottom, &right, nullptr, nullptr);
		o_x = left + (right - left) * 0.5l;
		o_y = top + (bottom - top) * 0.5l;
	}

	bool DuplicateLayerTo(const FDocument& iTargetDocument, std::string* iNewLayerName = nullptr)
	{
		ParentDocument.Active();

		Auto_Desc opDesc;
		Auto_Ref layerRef;
		GetLayerReference(&layerRef);
		sPSActionDescriptor->PutReference(opDesc.get(), keyNull, layerRef.get());

		Auto_Ref docref;
		iTargetDocument.GetDocumentReference(&docref);
		sPSActionDescriptor->PutReference(opDesc.get(), keyTo, docref.get());
		if (iNewLayerName)
		{
			sPSActionDescriptor->PutString(opDesc.get(), keyName, iNewLayerName->c_str());
		}
		Auto_Desc result;
		OSErr e = sPSActionControl->Play(&result, eventDuplicate, opDesc.get(), plugInDialogSilent);
		return (e == 0);
	}

	void MoveLayer(const double& offset_x, const double& offset_y)
	{
		Auto_Ref LayerRef;
		GetLayerReference(&LayerRef);

		Auto_Desc resultDesc;
		Auto_Desc transformDesc;

		sPSActionDescriptor->PutReference(transformDesc.get(), keyNull, LayerRef.get());
		sPSActionDescriptor->PutEnumerated(transformDesc.get(), keyFreeTransformCenterState, typeQuadCenterState, enumQCSAverage);

		Auto_Desc moveparamsDesc;
		sPSActionDescriptor->PutUnitFloat(moveparamsDesc.get(), keyHorizontal, unitPixels, floorl(offset_x));
		sPSActionDescriptor->PutUnitFloat(moveparamsDesc.get(), keyVertical, unitPixels, floorl(offset_y));
		sPSActionDescriptor->PutObject(transformDesc.get(), keyOffset, classOffset, moveparamsDesc.get());
		sPSActionDescriptor->PutEnumerated(transformDesc.get(), keyInterfaceIconFrameDimmed, typeInterpolation, enumBicubic);
		sPSActionControl->Play(&resultDesc, eventTransform, transformDesc.get(), plugInDialogSilent);
	}


private:
	PIDescriptorHandle layerDesc_h;
	FDocument ParentDocument;

};

