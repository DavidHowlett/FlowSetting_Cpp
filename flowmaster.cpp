#include <stdio.h>
#include <iostream.h>
#include "Flowmaster.h"
FlowMaster::FlowMaster()
{
	fmInUse=0;
	NumFlowMeters=0;
	for(int i=0;i<10;i++)
		fmPresent[i]=false;
}
void FlowMaster::Setup(	int FlowMeter1Port, char FlowMeter1Channel,float LocalFlowMeter1MaxFlow,int LocalFlowMeter1UnitsCorrection,
						int FlowMeter2Port, char FlowMeter2Channel,float LocalFlowMeter2MaxFlow,int LocalFlowMeter2UnitsCorrection,
						int FlowMeter3Port, char FlowMeter3Channel,float LocalFlowMeter3MaxFlow,int LocalFlowMeter3UnitsCorrection,
						int FlowMeter4Port, char FlowMeter4Channel,float LocalFlowMeter4MaxFlow,int LocalFlowMeter4UnitsCorrection,
						int FlowMeter5Port, char FlowMeter5Channel,float LocalFlowMeter5MaxFlow,int LocalFlowMeter5UnitsCorrection,
						int Weederio1Port,  char PressureTransducer1Port,float TimeForStabilization, char ReleaseValveChannel)
{
	LocalTimeForStabilization=TimeForStabilization;
	LocalReleaseValveChannel=ReleaseValveChannel;
	// the idea here is to have the below variables freely avalible in this class and only set here
	FlowMeterChannel[1]=FlowMeter1Channel;
	FlowMeterMaxFlow[1]=LocalFlowMeter1MaxFlow;
	FlowMeterUnitsCorrection[1]=LocalFlowMeter1UnitsCorrection;
	FlowMeterChannel[2]=FlowMeter2Channel;
	FlowMeterMaxFlow[2]=LocalFlowMeter2MaxFlow;
	FlowMeterUnitsCorrection[2]=LocalFlowMeter2UnitsCorrection;
	FlowMeterChannel[3]=FlowMeter3Channel;
	FlowMeterMaxFlow[3]=LocalFlowMeter3MaxFlow;
	FlowMeterUnitsCorrection[3]=LocalFlowMeter3UnitsCorrection;
	FlowMeterChannel[4]=FlowMeter4Channel;
	FlowMeterMaxFlow[4]=LocalFlowMeter4MaxFlow;
	FlowMeterUnitsCorrection[4]=LocalFlowMeter4UnitsCorrection;
	FlowMeterChannel[5]=FlowMeter5Channel;
	FlowMeterMaxFlow[5]=LocalFlowMeter5MaxFlow;
	FlowMeterUnitsCorrection[5]=LocalFlowMeter5UnitsCorrection;
	if (FlowMeter1Port>-1)// for item not in use the port will be minus 1
		CreateFlowMeter(FlowMeter1Port,1);
	if (FlowMeter2Port>-1)// for item not in use the port will be minus 1
		CreateFlowMeter(FlowMeter2Port,2);
	if (FlowMeter3Port>-1)// for item not in use the port will be minus 1
		CreateFlowMeter(FlowMeter3Port,3);
	if (FlowMeter4Port>-1)// for item not in use the port will be minus 1
		CreateFlowMeter(FlowMeter4Port,4);
	if (FlowMeter5Port>-1)// for item not in use the port will be minus 1
		CreateFlowMeter(FlowMeter5Port,5);

	if (Weederio1Port>-0.5)
	{
		WeederioInstance.Setup(Weederio1Port);//COM port
	LockDown();
		io1Present=1;
	}else{
		io1Present=0;
	}
	if (PressureTransducer1Port>-1)
	{
		// write code for presure transducer setup here
		pt1Present=1;
	}else
		pt1Present=0;
	FindSizeOrder();
	Sleep(20); // this is to allow the flowmeter threads to get data so they be safely used
	return;
}
int FlowMaster::CreateFlowMeter(int PortNum,int FlowMeterNum)
{
	char Name[10];
	fmPresent[FlowMeterNum]=false; // this means the flowmeter is considered not to exist unless it is specificly stated to exist later
	sprintf(Name,"COM%d",PortNum);
	FlowMeter[FlowMeterNum] = new AlicatFlowmeter(True);
	FlowMeter[FlowMeterNum]->Setup(Name,38400,'N',8,1);             //COM port, baud rate, parity, bytes per bit, stop bits
	FlowMeter[FlowMeterNum]->Open();                                   //Enable port access
	if (FlowMeter[FlowMeterNum]==NULL)
		printf("flowmeter%d on port%d not created. Cannot continue\n",FlowMeterNum,PortNum);
	else{
		FlowMeter[FlowMeterNum]->Resume();
		printf("flowmeter%d port opened\n",FlowMeterNum);
		fmPresent[FlowMeterNum]=true;
		NumFlowMeters++;
	};
	return 0;
}
int FlowMaster::FindSizeOrder()// this tries to produce an array listing the flowmeters in size order
{
	// this is a sorting algorithum to find the order of the sizes of the flowmeters
	// it cycles through the list looking for the biggest one still unaccounted for (not in list of sizes)
	// when it has a new biggest one it records the number of the flowmeter in the size_order array
	bool fmUnaccountedFor[10];
	for(int i=0;i<10;i++){
		SizeOrder[i]=0; // this initialises all to 0 (a none valid value)
		fmUnaccountedFor[i]=fmPresent[i];
	}
	int CurrentBiggestFlowMeter;
	for(int i=0;i<10;i++)
	{
		CurrentBiggestFlowMeter=0;
		for(int j=1;j<6;j++)
		{
			if((fmUnaccountedFor[j])&&((CurrentBiggestFlowMeter==0)||(FlowMeterMaxFlow[j]>=FlowMeterMaxFlow[CurrentBiggestFlowMeter])))
				CurrentBiggestFlowMeter=j;
		}
		SizeOrder[i]=CurrentBiggestFlowMeter; // note that this array starts from 0 unlike the others in this class
		fmUnaccountedFor[CurrentBiggestFlowMeter]=false;
	}
	return 0;
}
int FlowMaster::Reset()// this closes all valves except what is needed  for the largest flowmeter
{
	WeederioInstance.Close(LocalReleaseValveChannel);// this closes tha relece valve so the pressure can build up
	for(int i=1;i<6;i++)
	{
		if(fmPresent[i]&&(WeederioInstance.RequestStatus(FlowMeterChannel[i]))) // this should only close the valves that exist and are open to save time
			WeederioInstance.Close(FlowMeterChannel[i]);
	}
	WeederioInstance.Open(FlowMeterChannel[SizeOrder[0]]);// this opens the valve for the biggest flowmeter
	fmInUse=SizeOrder[0];// this states that the flowmeter in use is the largest
	printf("current massflow: %f changed to %f SLPM flowmeter\n",QuickMassFlow(),FlowMeterMaxFlow[SizeOrder[0]]);
	return 0;
}
int FlowMaster::ReconsiderValves()// this function tries to find the right valve for the current flowrate
{
	if(fmInUse!=0)
	{
		bool DoAgain=true;
		while(DoAgain)
		{
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
				if(MassFlow<=(0.95*FlowMeterMaxFlow[SizeOrder[i+1]]))
				{
					WeederioInstance.Open(FlowMeterChannel[SizeOrder[i+1]]);
					WeederioInstance.Close(FlowMeterChannel[SizeOrder[i]]);
					printf("current massflow: %f changed to %f SLPM flowmeter\n",MassFlow,FlowMeterMaxFlow[SizeOrder[i+1]]);
					fmInUse=SizeOrder[i+1];
					Sleep(LocalTimeForStabilization*2);
					DoAgain=true;
				}
			}
		}
	}
	return 0;
}
int FlowMaster::LockDown() // this closes all valves and renders the system useless but safe
{
	for(int i=1;i<6;i++)
	{
		if(fmPresent[i]) // this should only close the valves that exist to save time
			WeederioInstance.Close(FlowMeterChannel[i]);
	}
	//printf("current massflow: %f changed to no flowmeter\n",quick_massflow());
	fmInUse=0; //this value means that no flowmeter is in use
	// it matters that the flowmeter valves are closed before the pressure releace valve is opened or you can blow your flowmeters
	WeederioInstance.Open(LocalReleaseValveChannel);// this opens the releace valve
	return 0;
}
float FlowMaster::pafr()
{
	 return(MassFlow()/Pressure());
}
float FlowMaster::Pressure()
{
	float Pressure=0;
	if(pt1Present)
	{
		// write code here for getting pressure from transducer here
	}else{
		if(NumFlowMeters<1)
		{
			printf("no flowmeters found by pressure finding code");
			return(-1);
	}
		if(fmPresent[1])
		Pressure+=FlowMeter[1]->Pressure();
		if(fmPresent[2])
		Pressure+=FlowMeter[2]->Pressure();
		if(fmPresent[3])
		Pressure+=FlowMeter[3]->Pressure();
		if(fmPresent[4])
		Pressure+=FlowMeter[4]->Pressure();
		if(fmPresent[5])
		Pressure+=FlowMeter[5]->Pressure();
		Pressure=Pressure/NumFlowMeters;
		// the above finds an average
		Pressure=Pressure*0.0689475729;
		// the above converts from psi to bar
	}
	return Pressure;
}
float FlowMaster::MassFlow()
{
	ReconsiderValves();
	float MassFlow=QuickMassFlow();
	return(MassFlow);
}
float FlowMaster::QuickMassFlow()
{
	float MassFlow=0;
	if(fmInUse>0) //this checks that there is a useable flowmeter so the system does not crash on startup
		MassFlow=(FlowMeter[fmInUse]->MassFlow()/FlowMeterUnitsCorrection[fmInUse]);
	return(MassFlow);
}
FlowMaster::~FlowMaster()
{
	//lockdown(); //I tried to get lockdown to run when the form was closed but I found that this did not work.
}
