#include "Encap.h"

PIDescriptorHandle PEApi::DescriptorToHandle(const PIActionDescriptor& iDesc)
{
	PIDescriptorHandle desc_h;
	sPSActionDescriptor->AsHandle(iDesc, &desc_h);
	return std::move(desc_h);
}

void PEApi::HandleToDescriptor(const PIDescriptorHandle& idesc_h, PIActionDescriptor* oDesc)
{
	sPSActionDescriptor->HandleToDescriptor(idesc_h, oDesc);
}

int32 PEApi::OpenDocument(const string& DocFilePath)
{
	char* fpath = _strdup(DocFilePath.c_str());
	fpath[0] = toupper(fpath[0]);

	Auto_Desc result(false);
	Auto_Desc desc;
	Handle aliasValue = NULL;
	FullPathToAlias(fpath, aliasValue);
	sPSActionDescriptor->PutAlias(desc.get(), keyNull, aliasValue);
	sPSActionControl->Play(&result, eventOpen, desc.get(), plugInDialogSilent);
	sPSHandle->DisposeRegularHandle(aliasValue);
	int32 docID = 0;
	GetIntegerValueByKeyName(result.get(), "documentID", docID);

	delete(fpath);
	return std::move(docID);
}

void PEApi::GetBooleanByKeyName(const PIDescriptorHandle& iDesc_h, const string& iKeyName, Boolean& OutValue)
{
	DescriptorTypeID TypeID;
	sPSActionControl->StringIDToTypeID(iKeyName.c_str(), &TypeID);
	Auto_Desc desc;
	HandleToDescriptor(iDesc_h, &desc);
	sPSActionDescriptor->GetBoolean(desc.get(), TypeID, &OutValue);
}

void PEApi::GetIntegerValueByKeyName(const PIDescriptorHandle& iDesc_h, const string& iKeyName, int32& OutValue)
{
	DescriptorTypeID TypeID;
	sPSActionControl->StringIDToTypeID(iKeyName.c_str(), &TypeID);
	Auto_Desc desc;
	sPSActionDescriptor->HandleToDescriptor(iDesc_h, &desc);
	sPSActionDescriptor->GetInteger(desc.get(), TypeID, &OutValue);
}

void PEApi::GetUnitFloatByKeyName(const PIDescriptorHandle& iDesc_h, const string& iKeyName, uint32& OutUnitID, double& OutValue)
{
	DescriptorTypeID TypeID;
	sPSActionControl->StringIDToTypeID(iKeyName.c_str(), &TypeID);
	Auto_Desc desc;
	sPSActionDescriptor->HandleToDescriptor(iDesc_h, &desc);
	sPSActionDescriptor->GetUnitFloat(desc.get(), TypeID, &OutUnitID, &OutValue);
}

void PEApi::GetStringValueByKeyName(const PIDescriptorHandle& iDesc_h, const string& iKeyName, string& OutValue)
{
	DescriptorTypeID TypeID;
	sPSActionControl->StringIDToTypeID(iKeyName.c_str(), &TypeID);
	uint32 len = 255;
	char cstr[255];
	Auto_Desc desc;
	sPSActionDescriptor->HandleToDescriptor(iDesc_h, &desc);
	sPSActionDescriptor->GetString(desc.get(), TypeID, cstr, len);
	OutValue.clear();
	OutValue.append(cstr);
}

void PEApi::GetObjectByKeyName(const PIDescriptorHandle& iDesc_h, const string& iKeynameStr, PIActionDescriptor* Out_ObjDesc)
{
	DescriptorTypeID TypeID;
	sPSActionControl->StringIDToTypeID(iKeynameStr.c_str(), &TypeID);
	DescriptorClassID ClassID;
	Auto_Desc desc;
	HandleToDescriptor(iDesc_h, &desc);
	sPSActionDescriptor->GetObject(desc.get(), TypeID, &ClassID, Out_ObjDesc);
}

void PEApi::GetListByKeyName(const PIDescriptorHandle& iDesc_h, const string& iKeyName, PIActionList* OutList)
{
	DescriptorTypeID TypeID;
	sPSActionControl->StringIDToTypeID(iKeyName.c_str(), &TypeID);
	Auto_Desc desc(false);
	HandleToDescriptor(iDesc_h, &desc);
	sPSActionDescriptor->GetList(desc.get(), TypeID, OutList);
}

void PEApi::GetApplicationDesc(PIActionDescriptor* OutDesc)
{
	Auto_Ref appref;
	sPSActionReference->PutEnumerated(appref.get(), classApplication, typeOrdinal, enumTarget);
	sPSActionControl->Get(OutDesc, appref.get());
}

void PEApi::GetCurrentDocument(PIActionDescriptor* OutDesc)
{
	Auto_Ref docref;
	sPSActionReference->PutEnumerated(docref.get(), classDocument, typeOrdinal, enumTarget);
	sPSActionControl->Get(OutDesc, docref.get());
}

void PEApi::GetClassRefByIndex(DescriptorClassID desiredClass, uint32 Index, PIActionReference* OutRef)
{
	sPSActionReference->PutIndex(*OutRef, desiredClass, Index);
}

void PEApi::GetClassDescByIndex(DescriptorClassID desiredClass, uint32 Index, PIActionDescriptor* OutDesc)
{
	Auto_Ref ref;
	sPSActionReference->PutIndex(ref.get(), desiredClass, Index);
	OSErr e = sPSActionControl->Get(OutDesc, ref.get());
	if (e != 0)
	{
		throw e;
	}
}

void PEApi::ShowMessage(const stringstream& ss)
{
	MessageBox(GetActiveWindow(), TEXT(ss.str().c_str()), TEXT("My Caption"), MB_OK);
}

bool PEApi::KeyValueToString(const PIActionDescriptor& idesc, const DescriptorKeyID& ikeyID, string& OutString)
{
	stringstream ss;
	Boolean haskey = false;
	sPSActionDescriptor->HasKey(idesc, ikeyID, &haskey);

	OutString.clear();
	ss.clear();

	if (haskey)
	{
		DescriptorEnumTypeID keytypeID;
		sPSActionDescriptor->GetType(idesc, ikeyID, &keytypeID);

		switch (keytypeID)
		{
		case typeSInt64:
		{
			int64 returnData;
			sPSActionDescriptor->GetInteger64(idesc, ikeyID, &returnData);
			ss << returnData;
			break;
		}
		case typeSInt32: /* was typeInteger: */
		{
			int32 returnData;
			sPSActionDescriptor->GetInteger(idesc, ikeyID, &returnData);
			ss << returnData;
			break;
		}
		case typeIEEE64BitFloatingPoint: /* was typeFloat: */
		{
			double returnData;
			sPSActionDescriptor->GetFloat(idesc, ikeyID, &returnData);
			ss << returnData;
			break;
		}
		case typeUnitFloat:
		{
			DescriptorUnitID unitID;
			double returnData;
			sPSActionDescriptor->GetUnitFloat(idesc, ikeyID, &unitID, &returnData);
			char unitName[255];
			sPSActionControl->TypeIDToStringID(unitID, unitName, 255);
			ss << returnData << " " << unitName;
			break;
		}
		case typeChar:
		{
			char returnData[255];
			sPSActionDescriptor->GetString(idesc, ikeyID, returnData, 255);
			ss << returnData;
			break;
		}
		case typeBoolean:
		{
			Boolean returnData;
			sPSActionDescriptor->GetBoolean(idesc, ikeyID, &returnData);
			ss << (returnData ? "True" : "False");
			break;
		}
		case typeObject:
		{
			DescriptorClassID classID;
			PIActionDescriptor returnData;
			sPSActionDescriptor->GetObject(idesc, ikeyID, &classID, &returnData);
			char classIDStr[255];
			sPSActionControl->TypeIDToStringID(classID, classIDStr, 255);
			ss << "\"" << classIDStr << "\"" << " [Object]";
			break;
		}
		case typeGlobalObject:
		{
			DescriptorClassID classID;
			PIActionDescriptor returnData;
			sPSActionDescriptor->GetGlobalObject(idesc, ikeyID, &classID, &returnData);
			char classIDStr[255];
			sPSActionControl->TypeIDToStringID(classID, classIDStr, 255);
			ss << "\"" << classIDStr << "\"" << " [GlobalObject]";
			break;
		}
		case typeEnumerated:
		{
			DescriptorEnumTypeID EnumTypeID;
			DescriptorEnumID EnumID;
			sPSActionDescriptor->GetEnumerated(idesc, ikeyID, &EnumTypeID, &EnumID);
			char EnumTypeIDStr[255];
			sPSActionControl->TypeIDToStringID(EnumTypeID, EnumTypeIDStr, 255);
			char EnumIDStr[255];
			sPSActionControl->TypeIDToStringID(EnumID, EnumIDStr, 255);
			ss << EnumTypeIDStr << "::" << EnumIDStr << " [Enum]";
			break;
		}
		case typePath:
		{
			ss << "[typePath]";
			break;
		}
		case typeAlias:
		{
			ss << "[typeAlias]";
			break;;
		}
		case typeValueList:
		{
			/*error = sPSActionDescriptor->GetList(result,
			desiredKey,
			(PIActionList *)returnData);*/

			ss << "[typeValueList]";
			break;
		}
		case typeObjectSpecifier:
		{
			/*error = sPSActionDescriptor->GetReference(result,
			desiredKey,
			(PIActionReference *)returnData);*/
			ss << "[typeObjectSpecifier]";
			break;
		}
		case typeType:
		{
			ss << "[typeType]";
			break;
		}
		case typeGlobalClass:
		{
			DescriptorClassID returnData;
			sPSActionDescriptor->GetClass(idesc, ikeyID, &returnData);
			char ClassIDStr[255];
			sPSActionControl->TypeIDToStringID(returnData, ClassIDStr, 255);
			ss << "\"" << ClassIDStr << "\"" << " [GlobalClass]";
			break;
		}
		default:
		{
			break;
		}
		}

		OutString = ss.str();
		return true;
	}
	else
	{
		return false;
	}
}

void PEApi::GetAllKeysFromDescriptor(const PIActionDescriptor& Desc, string& OutString)
{
	stringstream ss;
	uint32 ikeyCount = 0;
	sPSActionDescriptor->GetCount(Desc, &ikeyCount);

	for (uint32 i = 0; i < ikeyCount; i++)
	{
		DescriptorKeyID kid;
		sPSActionDescriptor->GetKey(Desc, i, &kid);
		char keyStr[255];
		sPSActionControl->TypeIDToStringID(kid, keyStr, 255);

		string valueStr;
		KeyValueToString(Desc, kid, valueStr);

		ss << keyStr << " = " << valueStr << endl;
	}
	OutString = ss.str();
}

void PEApi::GetAllItemsFromList(const PIActionList& iList, string& OutString)
{
	uint32 Count = 0;
	sPSActionList->GetCount(iList, &Count);

	stringstream ss;

	for (uint32 i = 0; i < Count; i++)
	{
		DescriptorEnumTypeID itemtypeID;
		sPSActionList->GetType(iList, i, &itemtypeID);

		ss << i << ": ";

		switch (itemtypeID)
		{
		case typeSInt64:
		{
			int64 returnData;
			sPSActionList->GetInteger64(iList, i, &returnData);
			ss << returnData;
			break;
		}
		case typeSInt32: /* was typeInteger: */
		{
			int32 returnData;
			sPSActionList->GetInteger(iList, i, &returnData);
			ss << returnData;
			break;
		}
		case typeIEEE64BitFloatingPoint: /* was typeFloat: */
		{
			double returnData;
			sPSActionList->GetFloat(iList, i, &returnData);
			ss << returnData;
			break;
		}
		case typeUnitFloat:
		{
			DescriptorUnitID unitID;
			double returnData;
			sPSActionList->GetUnitFloat(iList, i, &unitID, &returnData);
			char unitName[255];
			sPSActionControl->TypeIDToStringID(unitID, unitName, 255);
			ss << returnData << " " << unitName;
			break;
		}
		case typeChar:
		{
			char returnData[255];
			sPSActionList->GetString(iList, i, returnData, 255);
			ss << returnData;
			break;
		}
		case typeBoolean:
		{
			Boolean returnData;
			sPSActionList->GetBoolean(iList, i, &returnData);
			ss << (returnData ? "True" : "False");
			break;
		}
		case typeObject:
		{
			DescriptorClassID classID;
			PIActionDescriptor returnData;
			sPSActionList->GetObject(iList, i, &classID, &returnData);
			char classIDStr[255];
			sPSActionControl->TypeIDToStringID(classID, classIDStr, 255);
			ss << "\"" << classIDStr << "\"" << " [Object]";
			break;
		}
		case typeGlobalObject:
		{
			DescriptorClassID classID;
			PIActionDescriptor returnData;
			sPSActionList->GetGlobalObject(iList, i, &classID, &returnData);
			char classIDStr[255];
			sPSActionControl->TypeIDToStringID(classID, classIDStr, 255);
			ss << "\"" << classIDStr << "\"" << " [GlobalObject]";
			break;
		}
		case typeEnumerated:
		{
			DescriptorEnumTypeID EnumTypeID;
			DescriptorEnumID EnumID;
			sPSActionList->GetEnumerated(iList, i, &EnumTypeID, &EnumID);
			char EnumTypeIDStr[255];
			sPSActionControl->TypeIDToStringID(EnumTypeID, EnumTypeIDStr, 255);
			char EnumIDStr[255];
			sPSActionControl->TypeIDToStringID(EnumID, EnumIDStr, 255);
			ss << EnumTypeIDStr << "::" << EnumIDStr << " [Enum]";
			break;
		}
		case typePath:
		{
			ss << "[typePath]";
			break;
		}
		case typeAlias:
		{
			ss << "[typeAlias]";
			break;
		}
		case typeValueList:
		{
			/*error = sPSActionDescriptor->GetList(result,
			desiredKey,
			(PIActionList *)returnData);*/

			ss << "[typeValueList]";
			break;
		}
		case typeObjectSpecifier:
		{
			/*error = sPSActionDescriptor->GetReference(result,
			desiredKey,
			(PIActionReference *)returnData);*/
			ss << "[typeObjectSpecifier]";
			break;
		}
		case typeType:
		{
			ss << "[typeType]";
			break;
		}
		case typeGlobalClass:
		{
			DescriptorClassID returnData;
			sPSActionList->GetClass(iList, i, &returnData);
			char ClassIDStr[255];
			sPSActionControl->TypeIDToStringID(returnData, ClassIDStr, 255);
			ss << "\"" << ClassIDStr << "\"" << " [GlobalClass]";
			break;
		}
		default:
		{
			break;
		}
		}
		ss << endl;
	}

	OutString = ss.str();
}

void PEApi::GetAllKeysFromReference(const PIActionReference& Ref, string& OutString)
{
	Auto_Desc desc(false);
	sPSActionControl->Get(&desc, Ref);
	GetAllKeysFromDescriptor(desc.get(), OutString);
}

void PEApi::WriteAllKeysFromReferenceTo(const PIActionDescriptor& Desc, const string& OutFilePath)
{
	ofstream os(OutFilePath.c_str());
	string keysStr;
	GetAllKeysFromDescriptor(Desc, keysStr);
	os << keysStr;
	os.close();
}

double PEApi::ConvertToPixels(const int32& iDocIndex, const double& iVal, const EUnit::Type& iUnitType)
{
	Auto_Desc TargetDocDesc;
	GetClassDescByIndex(classDocument, iDocIndex, &TargetDocDesc);
	uint32 UnitID;
	double docResolution;
	GetUnitFloatByKeyName(TargetDocDesc.get(), "resolution", UnitID, docResolution);
	if (UnitID != unitDensity)
	{
		throw std::string("Invalid document resolution unit.");
	}

	if (iUnitType == EUnit::Pixel)
	{
		return iVal;
	}
	else if (iUnitType == EUnit::Inch)
	{
		return (iVal * docResolution);
	}
	else if (iUnitType == EUnit::CM)
	{
		return (iVal * 50.0l / 127.0l * docResolution);
	}
	else if (iUnitType == EUnit::MM)
	{
		return (iVal * 5.0l / 127.0l *  docResolution);
	}
	else
	{
		throw std::string("Unsupported unit.");
	}

	return 0;
}

void PEApi::OpenDocument2(const string& DocFilePath)
{
	char* fpath = _strdup(DocFilePath.c_str());
	fpath[0] = toupper(fpath[0]);

	Auto_Desc result(false);
	Auto_Desc desc;
	Handle aliasValue = NULL;
	FullPathToAlias(fpath, aliasValue);
	sPSActionDescriptor->PutAlias(desc.get(), keyNull, aliasValue);
	sPSActionControl->Play(&result, eventOpen, desc.get(), plugInDialogSilent);

	//string s;
	//GetAllKeysFromDescriptor(result.get(), s);
	//ShowMessage(s);

	int32 docid = 0;
	GetIntegerValueByKeyName(result.get(), "documentID", docid);
	stringstream ss;
	ss << "docid: " << docid;
	ShowMessage(ss);


	sPSHandle->DisposeRegularHandle(aliasValue);

	delete(fpath);
}

void PEApi::ShowMessage(const std::string& s)
{
	MessageBox(GetActiveWindow(), TEXT(s.c_str()), TEXT("My Caption"), MB_OK);
}

void PEApi::GetListByKeyName(const PIActionDescriptor& iDesc, const string& iKeyName, PIActionList* OutList)
{
	DescriptorTypeID TypeID;
	sPSActionControl->StringIDToTypeID(iKeyName.c_str(), &TypeID);
	sPSActionDescriptor->GetList(iDesc, TypeID, OutList);
}

void PEApi::GetObjectByKeyName(const PIActionDescriptor& iDesc, const string& iKeynameStr, PIActionDescriptor* Out_ObjDesc)
{
	DescriptorTypeID TypeID;
	sPSActionControl->StringIDToTypeID(iKeynameStr.c_str(), &TypeID);
	DescriptorClassID ClassID;
	sPSActionDescriptor->GetObject(iDesc, TypeID, &ClassID, Out_ObjDesc);
}

void PEApi::GetStringValueByKeyName(const PIActionDescriptor& iDesc, const string& iKeyName, string& OutValue)
{
	DescriptorTypeID TypeID;
	sPSActionControl->StringIDToTypeID(iKeyName.c_str(), &TypeID);
	uint32 len = 255;
	char cstr[255];
	sPSActionDescriptor->GetString(iDesc, TypeID, cstr, len);
	OutValue.clear();
	OutValue.append(cstr);
}

void PEApi::GetUnitFloatByKeyName(const PIActionDescriptor& iDesc, const string& iKeyName, uint32& OutUnitID, double& OutValue)
{
	DescriptorTypeID TypeID;
	sPSActionControl->StringIDToTypeID(iKeyName.c_str(), &TypeID);
	sPSActionDescriptor->GetUnitFloat(iDesc, TypeID, &OutUnitID, &OutValue);
}

void PEApi::GetIntegerValueByKeyName(const PIActionDescriptor& iDesc, const string& iKeyName, int32& OutValue)
{
	DescriptorTypeID TypeID;
	sPSActionControl->StringIDToTypeID(iKeyName.c_str(), &TypeID);
	sPSActionDescriptor->GetInteger(iDesc, TypeID, &OutValue);
}

void PEApi::GetBooleanByKeyName(const PIActionDescriptor& iDesc, const string& iKeyName, Boolean& OutValue)
{
	DescriptorTypeID TypeID;
	sPSActionControl->StringIDToTypeID(iKeyName.c_str(), &TypeID);
	sPSActionDescriptor->GetBoolean(iDesc, TypeID, &OutValue);
}

double FDocument::GetResolution()
{
	uint32 unitID_res;
	double doc_res;
	GetUnitFloatByKeyName(docDesc_h, string("resolution"), unitID_res, doc_res);
	return std::move(doc_res);
}

Boolean FDocument::hasBackgroundLayer()
{
	Boolean hasBG;
	GetBooleanByKeyName(docDesc_h, "hasBackgroundLayer", hasBG);
	return std::move(hasBG);
}
