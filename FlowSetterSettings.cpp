// Standard Header Files ---------------------------------------------------
#include <cassert>
// My Header Files ---------------------------------------------------------
#include "FlowSetterSettings.h"
//---------------------------------------------------------------------------
FlowSetterSettings::ReadFile(){
	FilePointer = fopen ("settings file.txt","r");
	if(NULL==FilePointer){
		//int UserResponse = MessageBox(
		//	NULL,
		//	(LPCWSTR)L"Settings file not found\nCheck that the settings file is in the same folder as the program\nDo you want to try again?",
		//	(LPCWSTR)L"Error message",
		//	MB_ICONWARNING | MB_RETRYCANCEL | MB_DEFBUTTON1);
		//if(IDRETRY == UserResponse)
		//	return this->ReadFile(); // I thought that recursion would be nicer then a loop this time
		return 1;
	}else
		return UnguardedReadFile();
}
FlowSetterSettings::UnguardedReadFile(){
	int BrokenSettingsTest = 0;

	fscanf (FilePointer,"%d",&LatestGoodFlowmeterPort);			fgets (SettingsDescriptors[0],MaxCharsInSettingDescriptor,FilePointer);
	fscanf (FilePointer,"%d",&DataGatheringCycleTime);      fgets (SettingsDescriptors[1],MaxCharsInSettingDescriptor,FilePointer);
	fscanf (FilePointer,"%d",&LastRawDataFileNumber);       fgets (SettingsDescriptors[2],MaxCharsInSettingDescriptor,FilePointer);
	fscanf (FilePointer,"%d",&LastProcessedDataFileNumber);	fgets (SettingsDescriptors[3],MaxCharsInSettingDescriptor,FilePointer);
	fscanf (FilePointer,"%f",&TriggerFlow);									fgets (SettingsDescriptors[4],MaxCharsInSettingDescriptor,FilePointer);
	fscanf (FilePointer,"%d",&BrokenSettingsTest);					fgets (SettingsDescriptors[7],MaxCharsInSettingDescriptor,FilePointer);

	fclose (FilePointer);
	assert(1234567890==BrokenSettingsTest);
	if (1234567890==BrokenSettingsTest)
		return 0;
	else{
		//int TheUserResponse = MessageBox(
		//	NULL,
		//	(LPCWSTR)L"The settings file was found but not read successfully\nDo you want to try again?",
		//	(LPCWSTR)L"Error message",
		//	MB_ICONWARNING | MB_RETRYCANCEL | MB_DEFBUTTON1);
		//if(IDRETRY == TheUserResponse)
		//	return this->ReadFile(); // I thought that recursion would be nicer then a loop this time
		return 2;
	}
}
FlowSetterSettings::WriteFile(){
	FilePointer = fopen ("settings file.txt","w");
	if(NULL==FilePointer){
		//int UserResponse = MessageBox(
		//	NULL,
		//	(LPCWSTR)L"Settings file not found\nCheck that the settings file is in the same folder as the program and not write protected\nDo you want to try again?",
		//	(LPCWSTR)L"Error message",
		//	MB_ICONWARNING | MB_RETRYCANCEL | MB_DEFBUTTON1);
		//if(IDRETRY == UserResponse)
		//	return this->WriteFile();
		return 1;
	}else
		UnguardedWriteFile();
	fclose (FilePointer);
	return(0);
}
FlowSetterSettings::UnguardedWriteFile(){
	int BrokenSettingsTest = 1234567890;
	fprintf (FilePointer,"%d%s",LatestGoodFlowmeterPort,SettingsDescriptors[0]);
	fprintf (FilePointer,"%d%s",DataGatheringCycleTime,SettingsDescriptors[1]);
	fprintf (FilePointer,"%d%s",LastRawDataFileNumber,SettingsDescriptors[2]);
	fprintf (FilePointer,"%d%s",LastProcessedDataFileNumber,SettingsDescriptors[3]);
	fprintf (FilePointer,"%f%s",TriggerFlow,SettingsDescriptors[4]);
	fprintf (FilePointer,"%d%s",BrokenSettingsTest,SettingsDescriptors[7]);
	return 0;
}


