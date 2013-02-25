//---------------------------------------------------------------------------


#pragma hdrstop
#include <stdio.h>
#include <Classes.hpp>
#include <Dialogs.hpp>
#include "FlowMeter.h"
#include "Functions.h"
//---------------------------------------------------------------------------



#pragma package(smart_init)
//******************************************************************************
FlowMeter::FlowMeter(bool CreateSuspended):RS232Primitive(CreateSuspended)  //Constructor
//******************************************************************************
{
	LogCounter     = 0;                                         //Start putting the data into MeanSomething[0]
	FPressure      =-2;
	FTemperature   =-2;
	FVolumetricFlow=-2;
	FMassFlow      =-2;
	FGasType       ="None";
}

//******************************************************************************
boolean FlowMeter::TooOld()               //Is the data too old to show? eg over 0.05 seconds old
//******************************************************************************
{
  return False;
	Cardinal Age=GetTickCount()-TicksLastRead;
	if (Age<0) {
		ShowMessage("TicksLastRead>GetTickCount()... This should not happen!");
		return True;
	}
	if (Age>1000){ //If the Last Read time is too old
		ShowMessage("GetTickCount()-TicksLastRead)>100... This should not happen!");
		return True;
	}
	return False;
}

//******************************************************************************
void FlowMeter::ReadAndProcess()                         //Read the flowmeter data and update it
//******************************************************************************
{
	AnsiString s1;
	AnsiString s2;
	char sGasType[40];
	Read();                                                    //Read the flowmeter data
	s2=ReadUpTo(CRString);                                          //Get the string, if possible
	if (s2!="") {
		// Find last data line
		while (s2!="") {                                       //If this string was valid, try to get one more recent
			s1=s2;                                                 //Preserve the latest string we know to be goo
			s2=ReadUpTo(CRString);                                      //Try to get a new string from the port
		}
		//We can only get here when S2 is NULL, ie when we have flushed the buffer to the point of the last <CR>
		//We can only get here when S1 is not NULL.
		sscanf(s1.c_str(),"%f %f %f %f %s",&FPressure      ,
											&FTemperature   ,
											&FVolumetricFlow,
											&FMassFlow      ,
											&sGasType       );
		FGasType=sGasType;                                     //Wide to narrow string conversion
		if (FGasType!="Air") {
			FGasType="XXXXXX";
		}
		/*
		if (CompleteRecords>=1000) {
			float RPS=CompleteRecords/(86400*(Now()-StartTime));
			if (RPS>0) {
				RPS=RPS+0;
			}
		}
		*/
		PressureLog      [LogCounter]=FPressure      ;         //Record of the pressures over time
		TemperatureLog   [LogCounter]=FTemperature   ;         //Record of the temperatures over time
		VolumetricFlowLog[LogCounter]=FVolumetricFlow;         //Record of the volumetric flows over time
		MassFlowLog      [LogCounter]=FMassFlow      ;         //Record of the mass flows over time
		if (++LogCounter>=MeanReadingsCount){                  //Update the LogCounter and if too high,
			LogCounter=0;                                        //reset to 0
		}
		TicksLastRead=GetTickCount(); //Capture the date/time of the data
	}
}

//******************************************************************************
Cardinal FlowMeter::DataAge()                              //Age of data in clock ticks
//******************************************************************************
{
	return (GetTickCount()-TicksLastRead);
}

//******************************************************************************
void FlowMeter::SetPressureMonitor      (TEdit* Monitor)   //Optionally use an Edit Box to monitor the output
//******************************************************************************
{
	FPressureMonitor=Monitor;
}

//******************************************************************************
void FlowMeter::SetTemperatureMonitor   (TEdit* Monitor)   //Optionally use an Edit Box to monitor the output
//******************************************************************************
{
	FTemperatureMonitor=Monitor;
}
//******************************************************************************
void FlowMeter::SetVolumetricFlowMonitor(TEdit* Monitor)   //Optionally use an Edit Box to monitor the output
//******************************************************************************
{
	FVolumetricFlowMonitor=Monitor;
}
//******************************************************************************
void FlowMeter::SetMassFlowMonitor      (TEdit* Monitor)   //Optionally use an Edit Box to monitor the output
//******************************************************************************
{
	FMassFlowMonitor=Monitor;
}
//******************************************************************************
void FlowMeter::SetGasTypeMonitor       (TEdit* Monitor)   //Optionally use an Edit Box to monitor the output
//******************************************************************************
{
	FGasTypeMonitor=Monitor;
}
//******************************************************************************
float FlowMeter::Pressure      () //Return the pressure reading
//******************************************************************************
{
	if (TooOld()){
		return -1;
	};
	return FPressure;
}

//******************************************************************************
float FlowMeter::Temperature   () //Return the temperature reading
//******************************************************************************
{
	if (TooOld()){
		return -1;
	};
	return FTemperature;
}
//******************************************************************************
float FlowMeter::VolumetricFlow() //Return the volumetric flow reading
//******************************************************************************
{
	if (TooOld()){
		return -1;
	};
	return FVolumetricFlow;
}
//******************************************************************************
float FlowMeter::MassFlow      () //Return the mass flow reading
//******************************************************************************
{
	if (TooOld()){
		return -1;
	};
	return FMassFlow;
}
//******************************************************************************
String FlowMeter::GasType       () //Return the gas type
//******************************************************************************
{
	if (TooOld()){
		return "Too old";
	};
	return FGasType;
}

//******************************************************************************
float FlowMeter::MeanPressure      () //Return the mean pressure reading
//******************************************************************************
{
	if (TooOld()){
		return -1;
	};
	float Total=0;
	for (int i=0; i < MeanReadingsCount; i++) {
		Total=Total+PressureLog[i];
	}
	return Total/MeanReadingsCount;
}

//******************************************************************************
float FlowMeter::MeanTemperature   () //Return the mean temperature reading
//******************************************************************************
{
	if (TooOld()){
		return -1;
	};
	float Total=0;
	for (int i=0; i < MeanReadingsCount; i++) {
		Total=Total+TemperatureLog[i];
	}
	return Total/MeanReadingsCount;
}
//******************************************************************************
float FlowMeter::MeanVolumetricFlow() //Return the mean volumetric flow reading
//******************************************************************************
{
	if (TooOld()){
		return -1;
	};
	float Total=0;
	for (int i=0; i < MeanReadingsCount; i++) {
		Total=Total+VolumetricFlowLog[i];
	}
	return Total/MeanReadingsCount;
}
//******************************************************************************
float FlowMeter::MeanMassFlow      () //Return the mean mass flow reading
//******************************************************************************
{
	if (TooOld()){
		return -1;
	};
	float Total=0;
	for (int i=0; i < MeanReadingsCount; i++) {
		Total=Total+MassFlowLog[i];
	}
	return Total/MeanReadingsCount;
}
//******************************************************************************
void FlowMeter::SetName()
//******************************************************************************
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = "FlowMeter";
	info.dwThreadID = -1;
	info.dwFlags = 0;

	__try
	{
		RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD),(DWORD*)&info );
	}
	__except (EXCEPTION_CONTINUE_EXECUTION)
	{
	}
}

//******************************************************************************
void __fastcall FlowMeter::UpdateMonitors()  //Update the monitor fields, if any
//Monitor fields, if set, are automatically updated 
//******************************************************************************
{
	char tmp[20];
	if (FPressureMonitor      !=NULL) {sprintf(tmp,"%f",Pressure()      );FPressureMonitor      ->Text=tmp      ;};
	if (FTemperatureMonitor   !=NULL) {sprintf(tmp,"%f",Temperature()   );FTemperatureMonitor   ->Text=tmp      ;};
	if (FVolumetricFlowMonitor!=NULL) {sprintf(tmp,"%f",VolumetricFlow());FVolumetricFlowMonitor->Text=tmp      ;};
	if (FMassFlowMonitor      !=NULL) {sprintf(tmp,"%f",MassFlow()      );FMassFlowMonitor      ->Text=tmp      ;};
	if (FGasTypeMonitor       !=NULL) {                                  ;FGasTypeMonitor       ->Text=GasType();};
}

//******************************************************************************
void __fastcall FlowMeter::Execute()
//******************************************************************************
{
//	char TestMessage[500];
	AnsiString s;
	SetName();
	//---- Place thread code here ----
	FreeOnTerminate = true;  //Free memory on termination
//	FMessageToUser=TestMessage;

	while (!Terminated){
		Sleep(1);              //Don't cycle too fast- there's no point as there is not enough data.
//		sprintf(FMessageToUser,"Cycle %d\n",counter);
//		Synchronize(&UpdateTextBox);
		ReadAndProcess  (); //Obtain the data;
//		UpdateMonitors(); //Only to prove that this exists and that the fault is in Synchronize.
//		Synchronize(&UpdateMonitors);
//		if (LineBuffer->Count>=2) {
//			FMessageToUser=NextLine();
//			if (strlen(FMessageToUser)>0) {
//				Synchronize(&UpdateTextBox);
//			}
//			delete FMessageToUser; //Release this memory- we don't need it any more.
//		}

//		if (Terminated) {                 //If nicely told to stop running
//			strcpy (TestMessage,"Goodbye");
//			FMessageToUser=TestMessage;
//			Synchronize(&UpdateTextBox);
//			break;
//		}
	}
//	FMessageToUser=NULL;
}


