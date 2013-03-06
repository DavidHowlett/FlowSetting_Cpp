// Standard Header Files ---------------------------------------------------
#include <cassert>
// My Header Files ---------------------------------------------------------
#include "FlowSetterSettings.h"
//---------------------------------------------------------------------------
FlowSetterSettings::ReadFile(){
	FilePointer = fopen ("settings.txt","r");
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
	int i=0;
	fscanf (FilePointer,"%d",&SafePoint);			fgets (SettingsDescriptors[i++],MaxCharsInSettingDescriptor,FilePointer);
	fscanf (FilePointer,"%d",&NearPoint);			fgets (SettingsDescriptors[i++],MaxCharsInSettingDescriptor,FilePointer);
	fscanf (FilePointer,"%d",&TooFar);				fgets (SettingsDescriptors[i++],MaxCharsInSettingDescriptor,FilePointer);
	fscanf (FilePointer,"%d",&MaxSpeed);			fgets (SettingsDescriptors[i++],MaxCharsInSettingDescriptor,FilePointer);
	fscanf (FilePointer,"%d",&MaxAcc);				fgets (SettingsDescriptors[i++],MaxCharsInSettingDescriptor,FilePointer);
	fscanf (FilePointer,"%d",&MaxIterations);		fgets (SettingsDescriptors[i++],MaxCharsInSettingDescriptor,FilePointer);
	fscanf (FilePointer,"%f",&GasFactor);			fgets (SettingsDescriptors[i++],MaxCharsInSettingDescriptor,FilePointer);
	fscanf (FilePointer,"%f",&tPressure);			fgets (SettingsDescriptors[i++],MaxCharsInSettingDescriptor,FilePointer);
	fscanf (FilePointer,"%f",&MinPressure);			fgets (SettingsDescriptors[i++],MaxCharsInSettingDescriptor,FilePointer);
	fscanf (FilePointer,"%f",&MaxPressure);			fgets (SettingsDescriptors[i++],MaxCharsInSettingDescriptor,FilePointer);
	fscanf (FilePointer,"%f",&tpafr);				fgets (SettingsDescriptors[i++],MaxCharsInSettingDescriptor,FilePointer);
	fscanf (FilePointer,"%f",&tfError);				fgets (SettingsDescriptors[i++],MaxCharsInSettingDescriptor,FilePointer);
	fscanf (FilePointer,"%d",&CalibrationDataNum);	fgets (SettingsDescriptors[i++],MaxCharsInSettingDescriptor,FilePointer);
	fscanf (FilePointer,"%d",&MostRecentlyWrittenCaliberationDataNum);	fgets (SettingsDescriptors[i++],MaxCharsInSettingDescriptor,FilePointer);
	fscanf (FilePointer,"%f",&TimeForStabilization);fgets (SettingsDescriptors[i++],MaxCharsInSettingDescriptor,FilePointer);
	fscanf (FilePointer,"%f",&FractToMove);			fgets (SettingsDescriptors[i++],MaxCharsInSettingDescriptor,FilePointer);
	fscanf (FilePointer,"%f",&MaxTorque);			fgets (SettingsDescriptors[i++],MaxCharsInSettingDescriptor,FilePointer);
	fscanf (FilePointer,"%d",&AlgoNum);				fgets (SettingsDescriptors[i++],MaxCharsInSettingDescriptor,FilePointer);
	fscanf (FilePointer,"%d",&AmountToAdd);			fgets (SettingsDescriptors[i++],MaxCharsInSettingDescriptor,FilePointer);
	fscanf (FilePointer,"%f",&MaxBounceExpected);	fgets (SettingsDescriptors[i++],MaxCharsInSettingDescriptor,FilePointer);
	fscanf (FilePointer,"%d",&DistToBackOff);		fgets (SettingsDescriptors[i++],MaxCharsInSettingDescriptor,FilePointer);
	fscanf (FilePointer,"%c",&ReleaseValveChannel);	fgets (SettingsDescriptors[i++],MaxCharsInSettingDescriptor,FilePointer);
	fscanf (FilePointer,"%d",&MotorPort);			fgets (SettingsDescriptors[i++],MaxCharsInSettingDescriptor,FilePointer);
	fscanf (FilePointer,"%d",&WeederioPort);		fgets (SettingsDescriptors[i++],MaxCharsInSettingDescriptor,FilePointer);
	fscanf (FilePointer,"%d",&PressureTransducerPort);fgets (SettingsDescriptors[i++],MaxCharsInSettingDescriptor,FilePointer);

	for(int j=0;j<MAXFLOWWMETERS;j++){
		fscanf (FilePointer,"%d",&FlowMeterPort[j]);			fgets (SettingsDescriptors[i++],MaxCharsInSettingDescriptor,FilePointer);
		fscanf (FilePointer,"%c",&FlowMeterChannel[j]);			fgets (SettingsDescriptors[i++],MaxCharsInSettingDescriptor,FilePointer);
		fscanf (FilePointer,"%f",&FlowMeterMaxFlow[j]);			fgets (SettingsDescriptors[i++],MaxCharsInSettingDescriptor,FilePointer);
		fscanf (FilePointer,"%d",&FlowMeterUnitsCorrection[j]);	fgets (SettingsDescriptors[i++],MaxCharsInSettingDescriptor,FilePointer);
	}
	fscanf (FilePointer,"%d",&BrokenSettingsTest);	fgets (SettingsDescriptors[i++],MaxCharsInSettingDescriptor,FilePointer);


	fclose (FilePointer);
	assert(1234567890==BrokenSettingsTest); // remove me later
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
	FilePointer = fopen ("settings.txt","w");
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
	int i=0;
	fprintf (FilePointer,"%d%s",SafePoint			,SettingsDescriptors[i++]);
	fprintf (FilePointer,"%d%s",NearPoint			,SettingsDescriptors[i++]);
	fprintf (FilePointer,"%d%s",TooFar				,SettingsDescriptors[i++]);
	fprintf (FilePointer,"%d%s",MaxSpeed			,SettingsDescriptors[i++]);
	fprintf (FilePointer,"%d%s",MaxAcc				,SettingsDescriptors[i++]);
	fprintf (FilePointer,"%d%s",MaxIterations		,SettingsDescriptors[i++]);
	fprintf (FilePointer,"%f%s",GasFactor			,SettingsDescriptors[i++]);
	fprintf (FilePointer,"%f%s",tPressure			,SettingsDescriptors[i++]);
	fprintf (FilePointer,"%f%s",MinPressure			,SettingsDescriptors[i++]);
	fprintf (FilePointer,"%f%s",MaxPressure			,SettingsDescriptors[i++]);
	fprintf (FilePointer,"%f%s",tpafr				,SettingsDescriptors[i++]);
	fprintf (FilePointer,"%f%s",tfError				,SettingsDescriptors[i++]);
	fprintf (FilePointer,"%d%s",CalibrationDataNum	,SettingsDescriptors[i++]);
	fprintf (FilePointer,"%d%s",MostRecentlyWrittenCaliberationDataNum,SettingsDescriptors[i++]);
	fprintf (FilePointer,"%f%s",TimeForStabilization,SettingsDescriptors[i++]);
	fprintf (FilePointer,"%f%s",FractToMove			,SettingsDescriptors[i++]);
	fprintf (FilePointer,"%f%s",MaxTorque			,SettingsDescriptors[i++]);
	fprintf (FilePointer,"%d%s",AlgoNum				,SettingsDescriptors[i++]);
	fprintf (FilePointer,"%d%s",AmountToAdd			,SettingsDescriptors[i++]);
	fprintf (FilePointer,"%f%s",MaxBounceExpected	,SettingsDescriptors[i++]);
	fprintf (FilePointer,"%d%s",DistToBackOff		,SettingsDescriptors[i++]);
	fprintf (FilePointer,"%c%s",ReleaseValveChannel	,SettingsDescriptors[i++]);
	fprintf (FilePointer,"%d%s",MotorPort			,SettingsDescriptors[i++]);
	fprintf (FilePointer,"%d%s",WeederioPort		,SettingsDescriptors[i++]);
	fprintf (FilePointer,"%d%s\n",PressureTransducerPort,SettingsDescriptors[i++]);

	for(int j=0;j<MAXFLOWWMETERS;j++){
		fprintf (FilePointer,"%d%s",FlowMeterPort[j]			,SettingsDescriptors[i++]);
		fprintf (FilePointer,"%c%s",FlowMeterChannel[j]			,SettingsDescriptors[i++]);
		fprintf (FilePointer,"%f%s",FlowMeterMaxFlow[j]			,SettingsDescriptors[i++]);
		fprintf (FilePointer,"%d%s\n",FlowMeterUnitsCorrection[j]	,SettingsDescriptors[i++]);
	}
	fprintf (FilePointer,"%d%s",BrokenSettingsTest,SettingsDescriptors[i++]);

	return 0;
}


