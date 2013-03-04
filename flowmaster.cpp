#include <stdio.h>
#include <iostream.h>
#include "Flowmaster.h"
#include "AlicatFlowmeter.h"
#include "FlowSetterSettings.h"
FlowMaster::FlowMaster()
{
	fmInUse=0;
	NumFlowMeters=0;
	for(int i=0;i<MAXFLOWWMETERS;i++)
		fmPresent[i]=false;
}
void FlowMaster::Setup(FlowSetterSettings* GivenSettingsPointer)
{
	SettingsPointer=GivenSettingsPointer;
	for(int i=0;i<MAXFLOWWMETERS;i++)
		if (SettingsPointer->FlowMeterPort[i]>-1)// for item not in use the port will be -1
			CreateFlowMeter(SettingsPointer->FlowMeterPort[i],i);
	if(SettingsPointer->WeederioPort>-0.5){
		WeederioInstance.Setup(SettingsPointer->WeederioPort);//COM port
		LockDown();
		ioPresent=true;
	}else
		ioPresent=false;
	Sleep(20); // this is to allow the flowmeter threads to get data so they be safely used
	return;
}
bool FlowMaster::CreateFlowMeter(int PortNum,int FlowMeterNum)
{
	char Name[10];
	fmPresent[FlowMeterNum]=false; // this means the flowmeter is considered not to exist unless it is specificly stated to exist later
	sprintf(Name,"COM%d",PortNum);
	FlowMeter[FlowMeterNum]= new AlicatFlowmeter(True);
	FlowMeter[FlowMeterNum]->Setup(Name,38400,'N',8,1);             //COM port, baud rate, parity, bytes per bit, stop bits
	FlowMeter[FlowMeterNum]->Open();                                   //Enable port access
	if (FlowMeter[FlowMeterNum]==NULL){
		printf("flowmeter%d on port%d not created. Cannot continue\n",FlowMeterNum,PortNum);
		return false;
	}
	FlowMeter[FlowMeterNum]->Resume();
	printf("flowmeter%d port opened\n",FlowMeterNum);
	fmPresent[FlowMeterNum]=true;
	NumFlowMeters++;
	return true;
}

void FlowMaster::FindSizeOrder()// this tries to produce an array listing the flowmeters in size order
{
	// this is a sorting algorithum to find the order of the sizes of the flowmeters that exist
	// it cycles through the list looking for the biggest one still unaccounted for (not in list of sizes)
	// when it has a new biggest one it records the number of the flowmeter in the SizeOrder array
	bool fmUnaccountedFor[MAXFLOWWMETERS];
	for(int i=0;i<MAXFLOWWMETERS;i++){
		SizeOrder[i]=-1; // this initialises all to -1 (a none valid value)
		fmUnaccountedFor[i]=fmPresent[i];
	}
	int CurrentBiggestFlowMeter;
	for(int i=0;i<MAXFLOWWMETERS;i++){
		CurrentBiggestFlowMeter=-1;
		for(int j=0;j<MAXFLOWWMETERS;j++){
			if(	(fmUnaccountedFor[j])&&((-1==CurrentBiggestFlowMeter)
				||(SettingsPointer->FlowMeterMaxFlow[j]>=SettingsPointer->FlowMeterMaxFlow[CurrentBiggestFlowMeter])))
				CurrentBiggestFlowMeter=j;
		}
		SizeOrder[i]=CurrentBiggestFlowMeter; // note that this array starts from 0 unlike the others in this class
		fmUnaccountedFor[CurrentBiggestFlowMeter]=false;
	}
	return;
}

/*
int FlowMaster::BiggestFlowMeter(){ //this function returns the position in the array that corresponds to the biggest flowmeter
	int LargestFlowMeterSoFar=0;
	for(int i=0;i<MAXFLOWWMETERS;i++)
		if(SettingsPointer->FlowMeterMaxFlow[i]  >  SettingsPointer->FlowMeterMaxFlow[LargestFlowMeterSoFar])
			LargestFlowMeterSoFar=i;
	return LargestFlowMeterSoFar;
}
int FlowMaster::NextBiggestFlowMeter(){//fred, not finished
	// this function returns the position in the array that corresponds to the flowmeter one
	// smaller then the current one, if the current flowmeter is the smallest, the current flowmeter position is returned
	int LargestFlowMeterSoFar=0;
	for(int i=0;i<MAXFLOWWMETERS;i++)
		if(fmPresent[i])
			if(SettingsPointer->FlowMeterMaxFlow[i] )

}
*/
int FlowMaster::Reset()// this closes all valves except the one for the largest flowmeter
{
	WeederioInstance.Close(SettingsPointer->ReleaseValveChannel);// this closes tha relece valve so the pressure can build up
	for(int i=0;i<MAXFLOWWMETERS;i++)
		if(fmPresent[i]&&(WeederioInstance.RequestStatus(SettingsPointer->FlowMeterChannel[i]))) //to save time this should only close the valves that exist and are open
			WeederioInstance.Close(SettingsPointer->FlowMeterChannel[i]);
	WeederioInstance.Open(SettingsPointer->FlowMeterChannel[SizeOrder[0]]);// this opens the valve for the biggest flowmeter
	fmInUse=SizeOrder[0];// this states that the flowmeter in use is the largest
	printf("current massflow: %f changed to %f SLPM flowmeter\n",QuickMassFlow(),SettingsPointer->FlowMeterMaxFlow[SizeOrder[0]]);
	return 0;
}
int FlowMaster::ReconsiderValves()// this function tries to find the right valve for the current flowrate
{
	//bool ThereWasAChange = false;
	//do{
	//	//if the flow is bigger then 100% of the current flowmeter then move to the flowmeter one bigger
	//	//if the flow is smaller then 90% of the flowmeter one smaller then move to the smaller flowmeter
	//
	//}while(ThereWasAChange);

	//--------------------------
	if(fmInUse!=0){
		bool DoAgain=true;
		while(DoAgain){
			DoAgain=false;
			int i=0;
			while((i<10)&&(fmInUse!=SizeOrder[i]))
				i++;
			if(i>=9)
				printf("strange error in 'reconsider_valves' function, call programmer for help\n");
			// at this point i is the value in the size_order list where fminuse can be found
			if(SizeOrder[i+1]!=0)
			{
				float MassFlow=QuickMassFlow();
				if(MassFlow<=(0.95*SettingsPointer->FlowMeterMaxFlow[SizeOrder[i+1]]))
				{
					WeederioInstance.Open(SettingsPointer->FlowMeterChannel[SizeOrder[i+1]]);
					WeederioInstance.Close(SettingsPointer->FlowMeterChannel[SizeOrder[i]]);
					printf("current massflow: %f changed to %f SLPM flowmeter\n",MassFlow,SettingsPointer->FlowMeterMaxFlow[SizeOrder[i+1]]);
					fmInUse=SizeOrder[i+1];
					Sleep(SettingsPointer->TimeForStabilization*2);
					DoAgain=true;
				}
			}
		}
	}
	//-------------------
	return 0;
}
int FlowMaster::LockDown() // this closes all valves and renders the system useless but safe
{
	for(int i=1;i<6;i++)
	{
		if(fmPresent[i]) // this should only close the valves that exist to save time
			WeederioInstance.Close(SettingsPointer->FlowMeterChannel[i]);
	}
	//printf("current massflow: %f changed to no flowmeter\n",quick_massflow());
	fmInUse=-1; //this value means that no flowmeter is in use
	// it matters that the flowmeter valves are closed before the pressure releace valve is opened or you can blow your flowmeters
	WeederioInstance.Open(SettingsPointer->ReleaseValveChannel);// this opens the releace valve
	return 0;
}
float FlowMaster::pafr()
{
	 return(MassFlow()/Pressure());
}
float FlowMaster::Pressure()
{
	float Pressure=0;
	if(NumFlowMeters<1){
		printf("no flowmeters found by pressure finding code");
		return(-1);
	}
	for(int i=0;i<MAXFLOWWMETERS;i++){
		if(fmPresent[i])
			Pressure+=FlowMeter[i]->Pressure();
	}
	Pressure=Pressure/NumFlowMeters;// this finds an average
	Pressure=Pressure*0.0689475729;// this converts from psi to bar
	return Pressure;
}
float FlowMaster::MassFlow()
{
	ReconsiderValves();
	return(QuickMassFlow());
}
float FlowMaster::QuickMassFlow()
{
	float MassFlow=(FlowMeter[fmInUse]->MassFlow()/SettingsPointer->FlowMeterUnitsCorrection[fmInUse]);
	return(MassFlow);
}
FlowMaster::~FlowMaster()
{
	//lockdown(); //I tried to get lockdown to run when the form was closed but I found that this did not work.
}
