// Environment Resources ----------------------------------------------------
#include <iostream>
#include <Classes.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <string>
// My Header Files ----------------------------------------------------------
#include "Motor.h"
#include "FlowMaster.h"
#include "Hidden.h"       // this contains documentation
// My Classes ---------------------------------------------------------------
Motor MotorInstance;
FlowMaster FlowMasterInstance;
// Settings -----------------------------------------------------------------
// if settings are working correctly all the below values will be overwritten in setup()
int SafePoint=0; // this is the safe point near the jig
int NearPoint=0; // this is an alowed near point to the origin where it does not touch the stopper
int TooFar=0; // this is to test if the motor has gone too far and there is an error
int MaxSpeed=0; // maximum speed
int MaxAcc=0; // maximum acceleration
int MaxIterations=0;  // checks for failure due to infinite iterations
float GasFactor=1; //the flows will be set for air but used for othe gasses
float tPressure=0;   // this is the target pressure, not used yet
float MinPressure=0; //not used yet
float MaxPressure=0; //not used yet
float tpafr=0;   // this is the target pressure adjusted flow rate
float tfError=0; // this is the target fractional error eg 0.05 means +- 5% is ok
int CalibrationDataNum=0; //this enables multiple caliberation ccurves to be stored
int MostRecentlyWrittenCaliberationDataNum=0;
float TimeForStabilization=0;//measured in miliseconds, change for good results
float FractToMove=0; // the fraction of the distance to the estimated destination that the motor should move each time
float MaxTorque=0; // I can't measure torque only related quantitys so beware
int AlgoNum=0; // this describes which algorithm is used for setting flows
int AmountToAdd=0; // this describes how far the motor moves during each iteration of caliberation data generation
int Debugging=0; //this should be 1 for debugging mode but 0 for normal use
float MaxBounceExpected=0; // the units are SLPM per Bar
int DistToBackOff=0;// this is used when the motor backs off to get a measurement includeing the bounce effect
char ReleaseValveChannel='A';


int   Motor1Port=-1; // the port number being -1 signals that the device does not exist
int   Weederio1Port=-1; // the port number being -1 signals that the device does not exist
int   PressureTransducer1Port=-1; // the port number being -1 signals that the device does not exist
int   FlowMeter1Port=-1;// the port number being -1 signals that the device does not exist
char  FlowMeter1Channel='A';
float FlowMeter1MaxFlow= 0; //this is in standard litres per min
int   FlowMeter1UnitsCorrection=1; // the flow data is divided by this value, should be 1 for litres per min and 1000 for ml
int   FlowMeter2Port   = 0;// the port number being -1 signals that the device does not exist
char  FlowMeter2Channel='A';
float FlowMeter2MaxFlow= 0; //this is in standard litres per min
int   FlowMeter2UnitsCorrection=1; // the flow data is divided by this value, should be 1 for litres per min and 1000 for ml
int   FlowMeter3Port   =-1;// the port number being -1 signals that the device does not exist
char  FlowMeter3Channel='A';
float FlowMeter3MaxFlow= 0; //this is in standard litres per min
int   FlowMeter3UnitsCorrection=1; // the flow data is divided by this value, should be 1 for litres per min and 1000 for ml
int   FlowMeter4Port=-1;// the port number being -1 signals that the device does not exist
char  FlowMeter4Channel='A';
float FlowMeter4MaxFlow=0; //this is in standard litres per min
int   FlowMeter4UnitsCorrection=1; // the flow data is divided by this value, should be 1 for litres per min and 1000 for ml
int   FlowMeter5Port=-1;// the port number being -1 signals that the device does not exist
char  FlowMeter5Channel='A';
float FlowMeter5MaxFlow=0; //this is in standard litres per min
int   FlowMeter5UnitsCorrection=1; // the flow data is divided by this value, should be 1 for litres per min and 1000 for ml
int   BrokenSettingsTest=0; // this is overwriten with a known value to check that the read funtion got to the end of the file ok

//other global variables********************************************************
bool RunAgain=true;
const int MaxTableSize=10000;        // I can't use a varable as this value is needed at compile time
int MotorPositionTable[MaxTableSize];
float PafrTable[MaxTableSize]; // pressure is measured in bar, pafr = presure ajusted flow rate measured in ml/bar/min

//function declarations*********************************************************
int Setup(); // this should only be called at startup and should contain everything needed to initialise the system
int UserInterationAndAction(); // this asks the user what to do next and calls the apropriate code
int ReadSettings(); // this reads all of the settings from the settings file
int ReadCalibrationData(); // this is a decleration for a function that reads calibration data.txt and stores it in an array
int CreateCaliberationData(); // this slowly moves the motor forwards while monitoring the flow and then records to file a curve that relates position to flow for the prupose of caliberateing future movements
int CreateBackOffCaliberationData(); // this works in a similar way to the above but also records flow rate readings before and after backing off
int PositionAssosiatedWithPafr(float pafr); //note this kind of thing does not initialise pafr
float Perform(float tpafr); // tpafr =(target pressure ajusted flow rate), function returns achived pafr
float AlgorithmWithoutBounceProtection(float Local_tpafr);// this is the primary flow setting algorithum. it iteratively estimates the final position based on flow rate and moves 50% of the distance to the estimated final position. tpafr =(target pressure ajusted flow rate), function returns achived pafr
float AlgorithmWithoutBounceProtectionForInternalUse(float Local_tpafr); // this is almost the same as the above but it does not back off from the jig. Also it does not lock down after finishing
float AlgorithmWithBounceProtection(float Local_tpafr);// first this calls the main algorithum to get the flow close to it's final value, then it enters a second phase where it acts like the main algoritum with the one change that it backs off before each flow measurement.
int HandleResult(float rpafr, float tpafr); // this looks at the flow rate achived by the setting algorithum, decides if it is within alowed limits and tells the user
int List(); // this lists all alowed commands
int ViewSettings(); // this displays the more important settings
int WriteSettings(); // this writes the current settings to the settings file
//int manual();

//******************************************************************************
int _tmain(int argc, _TCHAR* argv[])
{
	Setup();
	while(RunAgain==true)
		UserInterationAndAction();
	FlowMasterInstance.LockDown();
	printf("program finished \n");
	Sleep(1000); // remove me later
	return 0;
}
//function definitions**********************************************************
int UserInterationAndAction()
{
	char Answer[100];
	double ToBeSet; // only used when the user enters a number
	bool IsNumeric;
	float rpafr=0; // the resulting pressure ajusted flow rate from "perform"
	//motor_instance.go_to(maxspeed,maxacc,NearPoint); //speed, acceleration, distance
	printf("enter 's' to set a flow, 'e' for exit or 'l' for a list of possible commands\n");
	scanf("%s",Answer);
//**************// the below piece of code tests if the input was a number
	if((Answer[0]<=57)&&(Answer[0]>=48))
	{
		ToBeSet=atof(Answer);
		IsNumeric=true;
	}
//**************
	if(! strcmp (Answer, "s")){// note strcmp outputs the opposite of what you would expect
		rpafr=Perform(tpafr); // this tells the perform function to try to set the pafr to target pafr
		HandleResult(rpafr,tpafr);
	}
	else if(! strcmp (Answer, "e"))
		RunAgain=false;
	else if(! strcmp (Answer, "l"))
		List();
	//else if(! strcmp (answer, "m"))
	//	manual();
	else if(! strcmp (Answer, "v"))
		ViewSettings();
	else if(! strcmp (Answer, "cbo"))
		CreateBackOffCaliberationData();
	else if(! strcmp (Answer, "c"))
		CreateCaliberationData();
	else if(IsNumeric)
	{
		// commented out code interprets input as SLPM, used code interprets input as pafr
		//float temp_target_pafr=to_be_set/flowmaster_instance.pressure();
		//printf("target pressure adjusted flow rate: %f\n",temp_target_pafr);
		//rpafr=perform(temp_target_pafr); // tells the perform function to set the target pafr and send the achived flow to rpafr
		//handle_result(rpafr,temp_target_pafr);
		rpafr=Perform(ToBeSet); // tells the perform function to set the target pafr and send the achived flow to rpafr
		HandleResult(rpafr,ToBeSet);
	}
	else
		printf("command not recognised\n");
	return 0;
}
int Setup()
{
	ReadSettings(); // it matters that read settings is before I get the caliberation data
	//write_settings();
	ReadCalibrationData();
	FlowMasterInstance.Setup(	FlowMeter1Port, FlowMeter1Channel,FlowMeter1MaxFlow,FlowMeter1UnitsCorrection,
								FlowMeter2Port, FlowMeter2Channel,FlowMeter2MaxFlow,FlowMeter2UnitsCorrection,
								FlowMeter3Port, FlowMeter3Channel,FlowMeter3MaxFlow,FlowMeter3UnitsCorrection,
								FlowMeter4Port, FlowMeter4Channel,FlowMeter4MaxFlow,FlowMeter4UnitsCorrection,
								FlowMeter5Port, FlowMeter5Channel,FlowMeter5MaxFlow,FlowMeter5UnitsCorrection,
								Weederio1Port,  PressureTransducer1Port, TimeForStabilization, ReleaseValveChannel);
	//flowmaster_instance.lockdown();// this closes all known valves
	MotorInstance.Setup(Motor1Port);
	MotorInstance.SetOrigin();
	printf("pressure: %f bar  ",FlowMasterInstance.Pressure());
	printf("massflow: %f SLPM  ",FlowMasterInstance.MassFlow());
	printf("pafr: %f SPLM/bar\n",FlowMasterInstance.pafr());
	return 0;
}
int ReadCalibrationData()
{
	char Num[10];
	char Name[30];
	sprintf(Num,"%d",CalibrationDataNum);  // these lines of code assemble the name of the calibaration data file
	strcpy(Name,"calibration data");
	strcat(Name,Num);
	strcat(Name,".txt");
	int PositionInArray=0;
	for(PositionInArray=0; PositionInArray<=MaxTableSize-1; PositionInArray++)
	{
		MotorPositionTable[PositionInArray]=1000000000;
		PafrTable[PositionInArray]=0;  // these two lines initialise my caliberation array to harmless values
	}
	FILE * pFile;
	pFile = fopen (Name,"r");// opens the caliberation data file
	for(PositionInArray=0;PositionInArray<=MaxTableSize-1; PositionInArray++)   // the data terminates with 987654321 0, the motor will never go that far
	{
		fscanf (pFile, "%d %f", &MotorPositionTable[PositionInArray], &PafrTable[PositionInArray]);
		// printf ("I have read: %d and %f \n",motor_position_table[position_in_array],pafr_table[position_in_array]);
	}
	fclose (pFile);
	return 0;
}
int CreateCaliberationData()
{
	int CalMotorPositionTable[MaxTableSize];
	float CalPafrTable[MaxTableSize];
	int PositionInArray=0;
	int Torque=0;
	int MotorPosition=0;
	int CurrentAmountToAdd=AmountToAdd;
	FlowMasterInstance.Reset();
	Sleep(2*TimeForStabilization);
	CalMotorPositionTable[0]=-1000000000;
	CalPafrTable[0]=700000;           // this and the above line just ensure that if flows are requested that are above the caliberration curve's highest point then the system simply moves forwards slowly
	for(PositionInArray=1; PositionInArray<=MaxTableSize-1;PositionInArray++){
		CalMotorPositionTable[PositionInArray]=1000000; // this is harmless and it is the end of data marker
		CalPafrTable[PositionInArray]=0;  // these two lines initialise my caliberation array to harmless values
	}
	MotorInstance.GoTo(MaxSpeed,MaxAcc,SafePoint);
	float pafr=FlowMasterInstance.pafr();
	for(PositionInArray=1;(	pafr>0.001)
							&&(Torque<MaxTorque)
							&&(PositionInArray<=(MaxTableSize-1))
							&&(MotorPosition<TooFar);PositionInArray++){   // this while sets the point of cut off after which caliberation ends
		MotorInstance.GoTo(MaxSpeed,MaxAcc,(MotorInstance.Position()+CurrentAmountToAdd));
		Sleep(TimeForStabilization);
		CalMotorPositionTable[PositionInArray]=MotorPosition=MotorInstance.Position();
		CalPafrTable[PositionInArray]=pafr=FlowMasterInstance.pafr();
		Torque=MotorInstance.PseudoTorque();
		printf(	"position: %d  pseudotorque: %d  massflow: %f  pafr: %f\n"
				,CalMotorPositionTable[PositionInArray],Torque,FlowMasterInstance.MassFlow(),CalPafrTable[PositionInArray]);
		if (pafr<0.5)
			CurrentAmountToAdd=AmountToAdd/5;
		if (pafr<0.1)
			CurrentAmountToAdd=AmountToAdd/50;
	}
	if(!(pafr>0.001))
		printf("caliberation stopped because low flow was reached\n");
	if(!(MotorInstance.PseudoTorque()<MaxTorque))
		printf("caliberation stopped due to excess torque\n");
	if(!(PositionInArray<=(MaxTableSize-1)))
		printf("caliberation stopped due to running out of space in this program's array\n");
	if(!(MotorInstance.Position()<TooFar))
		printf("caliberation stopped due to motor going too far\n");
	MostRecentlyWrittenCaliberationDataNum++;
	CalibrationDataNum=MostRecentlyWrittenCaliberationDataNum;
	WriteSettings();

	char Num[10]; // this bit of code is poor, should be rewritten
	char Name[30];
	sprintf(Num,"%d",MostRecentlyWrittenCaliberationDataNum);
	strcpy(Name,"calibration data");
	strcat(Name,Num);
	strcat(Name,".txt");
	FILE * pFile;
	pFile = fopen (Name,"w+");
	if( pFile==NULL)
		printf("error: could not access calibration file");
	else{
		for(PositionInArray=0; PositionInArray<=(MaxTableSize-1); PositionInArray++)   // the data terminates with 987654321 0, the motor will never go that far
			fprintf (pFile, "%d\t%f\n", CalMotorPositionTable[PositionInArray], CalPafrTable[PositionInArray]);
	}
	FlowMasterInstance.LockDown();
	MotorInstance.GoTo(MaxSpeed,MaxAcc,NearPoint); // NearPoint= near point to origin
	fclose (pFile);
	ReadCalibrationData();
	return 0;
}
int CreateBackOffCaliberationData()
{
	int CalMotorPositionTable[MaxTableSize];
	float CalPafrTable[MaxTableSize];
	float CalBackOffPafrTable[MaxTableSize];
	int PositionInArray=0;
	int Torque=0;
	int MotorPosition=0;
	int CurrentAmountToAdd=AmountToAdd;
	FlowMasterInstance.Reset();
	Sleep(2*TimeForStabilization);
	CalMotorPositionTable[0]=-1000000000;
	CalPafrTable[0]=700000;					 // this and the above line just ensure that if flows are requested that are above the caliberration curve's highest point then the system simply moves forwards slowly
	CalBackOffPafrTable[0]=700000;
	for(PositionInArray=1; PositionInArray<=MaxTableSize-1;PositionInArray++){
		CalMotorPositionTable[PositionInArray]=1000000; // this is harmless and it is the end of data marker
		CalPafrTable[PositionInArray]=0;  // these two lines initialise my caliberation array to harmless values
		CalBackOffPafrTable[PositionInArray]=0;
	}
	MotorInstance.GoTo(MaxSpeed,MaxAcc,SafePoint);
	float pafr=FlowMasterInstance.pafr();
	for(	PositionInArray=1;(pafr>0.001)
			&&(Torque<MaxTorque)
			&&(PositionInArray<=(MaxTableSize-1))
			&&(MotorPosition<TooFar);PositionInArray++){   // this while sets the point of cut off after which caliberation ends
		MotorInstance.GoTo(MaxSpeed,MaxAcc,(MotorInstance.Position()+CurrentAmountToAdd));
		CalMotorPositionTable[PositionInArray]=MotorPosition=MotorInstance.Position();
		CalPafrTable[PositionInArray]=pafr=FlowMasterInstance.pafr();
		MotorInstance.GoTo(MaxSpeed,MaxAcc,(MotorInstance.Position()- DistToBackOff)); // this backs the motor off so I can get a reading without bounce
		Sleep(TimeForStabilization);
		CalBackOffPafrTable[PositionInArray]=FlowMasterInstance.pafr();
		MotorInstance.GoTo(MaxSpeed,MaxAcc,(MotorInstance.Position()+DistToBackOff)); // this backs the motor off so I can get a reading without bounce

		printf("position: %d  massflow: %f  pafr: %f backed_off_pafr: %f\n",
			CalMotorPositionTable[PositionInArray],FlowMasterInstance.MassFlow(),CalPafrTable[PositionInArray],CalBackOffPafrTable[PositionInArray]);
		if (pafr<0.5)
			CurrentAmountToAdd=AmountToAdd/5;
		if (pafr<0.1)
			CurrentAmountToAdd=AmountToAdd/50;
	}
	if(!(pafr>0.001))
		printf("caliberation stopped because low flow was reached\n");
	if(!(MotorInstance.PseudoTorque()<MaxTorque))
		printf("caliberation stopped due to excess torque\n");
	if(!(PositionInArray<=(MaxTableSize-1)))
		printf("caliberation stopped due to running out of space in this program's array\n");
	if(!(MotorInstance.Position()<TooFar))
		printf("caliberation stopped due to motor going too far\n");

	char Name[30];
	strcpy(Name,"data with backing off (not used for calibration)");
	strcat(Name,".txt");
	FILE * pFile;
	pFile = fopen (Name,"w+");
	if( pFile==NULL)
		printf("error: could not access calibration file");
	else{
		for(PositionInArray=0; PositionInArray<=(MaxTableSize-1); PositionInArray++)   // the data terminates with 987654321 0, the motor will never go that far
			fprintf (pFile, "%d\t%f\t%f\n", CalMotorPositionTable[PositionInArray], CalPafrTable[PositionInArray],CalBackOffPafrTable[PositionInArray]);
	}
	FlowMasterInstance.LockDown();
	MotorInstance.GoTo(MaxSpeed,MaxAcc,NearPoint); // NearPoint= near point to origin
	fclose (pFile);
	ReadCalibrationData();
	return 0;
}
int PositionAssosiatedWithPafr(float pafr)// this gets the position on the caliberation curve that is assosiated with a pressure adjusted flow rate
{
	int PositionInArray=0;
	while(pafr<=PafrTable[PositionInArray])//
		PositionInArray++;
	//now pafr_table[position_in_array-1]>pafr>pafr_table[position_in_array]
	//now I interpolate
	float Fraction=(PafrTable[PositionInArray-1]-pafr)/(PafrTable[PositionInArray-1]-PafrTable[PositionInArray]);
	int rValue=(Fraction*MotorPositionTable[PositionInArray]+(1-Fraction)*MotorPositionTable[PositionInArray-1]);
	return (rValue);
}
float Perform(float LocalTpafr) // target pressure ajusted flow rate
{
	if(AlgoNum==1)
		return(AlgorithmWithoutBounceProtection(LocalTpafr));
	//if(AlgonNm==2)
	//	return(AlgorithmWithBounceProtection(LocalTpafr));
	else
	{
		printf("algorithm specified not recognised");
		return(-2);
	}
}
float AlgorithmWithoutBounceProtection(float LocalTpafr)
{
	int Iterations=0; // this just checks for the thing looping forever
	int Torque=0;
	int Flag=0;
	int MotorPosition=MotorInstance.Position();
	MotorInstance.SetOrigin();
	MotorInstance.GoTo(MaxSpeed,MaxAcc,SafePoint);
	FlowMasterInstance.Reset();
	Sleep(2*TimeForStabilization);
	float cpafr=FlowMasterInstance.pafr(); // current pressure ajusted flow rate
	int tep=PositionAssosiatedWithPafr(LocalTpafr);// target equivelent position, current equivelent position
	int cep=PositionAssosiatedWithPafr(cpafr);
	if (LocalTpafr<0.0001)
	{
		printf("flow too small to set\n");
		FlowMasterInstance.LockDown();
		return (-2);
	}
	if (cpafr<=(LocalTpafr*(1+tfError)))
	{
		FlowMasterInstance.LockDown();
		return (cpafr);
	}
	while((Iterations<MaxIterations)
				&&(Torque<MaxTorque)
				&&(MotorPosition<TooFar))
		{
			cpafr=FlowMasterInstance.pafr();
			printf("iterations: %d position: %d pafr: %f\n",Iterations,MotorPosition,cpafr);
			if((Flag>=1)||(cpafr<=LocalTpafr))// this breaks out of the loop if the flag is high or if it has gone below it's target flow
			{
				MotorInstance.GoTo(MaxSpeed,MaxAcc,NearPoint);
				Sleep(TimeForStabilization); // I sleep for a second time because the flow may change as the motor backs off
				cpafr=FlowMasterInstance.pafr();
				FlowMasterInstance.LockDown();
				return (cpafr); // this returns the achieved flowrate, the calling function must decide if this flow is acceptable
			}
			if(cpafr<=(LocalTpafr*(1+tfError))) // this breaks out of the loop if the flow is low enough to warrant it
				Flag++;
			//torque=motor_instance.pseudotorque();     // this line is not useful
			cep=PositionAssosiatedWithPafr(cpafr);
			MotorPosition=MotorInstance.Position();
			MotorInstance.GoTo(MaxSpeed,MaxAcc,(MotorPosition+FractToMove*(tep-cep)));
			Sleep(TimeForStabilization);
			Iterations++;
		}
	if(!(Iterations<MaxIterations))
		printf("error: too many iterations,\n this program can handle an infinite number of iterations but the fact that there have been %d iterations without the desired flow rate being reached indicates something else is wrong\n",MaxIterations);
	if(!(MotorInstance.Position()<TooFar))
		printf("error: program stopped motor because it went too far\n");
	if(!(MotorInstance.PseudoTorque()<MaxTorque))
		printf("caliberation stopped due to excess torque\n");
	FlowMasterInstance.LockDown();
	return(-2.0);
}
/*
float algorithm_without_bounce_protection_for_internal_use(float local_tpafr)
{
	int iterations=0; // this just checks for the thing looping forever
	int torque=0;
	int flag=0;
	int motorposition=MotorInstance.Position();
	MotorInstance.SetOrigin();
	MotorInstance.GoTo(maxspeed,maxacc,SafePoint);
	FlowMasterInstance.Reset();
	Sleep(2*time_for_stabilization);
	float cpafr=FlowMasterInstance.pafr(); // current pressure ajusted flow rate
	int tep=position_assosiated_with_pafr(local_tpafr);// target equivelent position, current equivelent position
	int cep=position_assosiated_with_pafr(cpafr);
	if (local_tpafr<0.0001)
	{
		printf("flow too small to set\n");
		return (-2);
	}
	if (cpafr<=(local_tpafr*(1+tferror)))
	{
		return (cpafr);
	}
	while((iterations<maxiterations)
				&&(torque<maxtorque)
				&&(motorposition<too_far))
		{
			cpafr=FlowMasterInstance.pafr();
			printf("iterations: %d position: %d pafr: %f\n",iterations,motorposition,cpafr);
			if((flag>=1)||(cpafr<=local_tpafr))// this breaks out of the loop if the flag is high or if it has gone below it's target flow
			{
				//motor_instance.go_to(maxspeed,maxacc,NearPoint); // not desired for internal use
				Sleep(time_for_stabilization); // I sleep for a second time because the flow may change as the motor backs off
				cpafr=FlowMasterInstance.pafr();
				//flowmaster_instance.lockdown(); // not desired for internal use
				return (cpafr); // this returns the achieved flowrate, the calling function must decide if this flow is acceptable
			}
			if(cpafr<=(local_tpafr*(1+tferror))) // this breaks out of the loop if the flow is low enough to warrant it
				flag++;
			//torque=motor_instance.pseudotorque();     // this line is not useful
			cep=position_assosiated_with_pafr(cpafr);
			motorposition=MotorInstance.Position();
			MotorInstance.GoTo(maxspeed,maxacc,(motorposition+fract2move*(tep-cep)));
			Sleep(time_for_stabilization);
			iterations++;
		}
	if(!(iterations<maxiterations))
		printf("error: too many iterations,\n this program can handle an infinite number of iterations but the fact that there have been %d iterations without the desired flow rate being reached indicates something else is wrong\n",maxiterations);
	if(!(MotorInstance.Position()<too_far))
		printf("error: program stopped motor because it went too far\n");
	if(!(MotorInstance.PseudoTorque()<maxtorque))
		printf("caliberation stopped due to excess torque\n");
	return(-2.0);
}
float algorithm_with_bounce_protection(float local_tpafr)
{
	float rpafr; // the returned pressure adjusted flow rate
	rpafr=algorithm_without_bounce_protection_for_internal_use(local_tpafr-max_bounce_expected);// this tells the simpler algorithum to get close to the desired answer
	if(rpafr==-2) // if "algorithm_without_bounce_protection" ends badly then this algorithum should do nothing but pass the bad result to the caller
	{
		FlowMasterInstance.LockDown(); // this closes the valves to make it safe for the user to open the jig
		return(rpafr);
	}
	if (rpafr<=(local_tpafr*(1+tferror))) // if the flow is lower then the upper bound allowed then nothing more should be done
	{
		FlowMasterInstance.LockDown(); // this closes the valves to make it safe for the user to open the jig
		return (rpafr);
	}
	printf("second stage started\n");
	float cpafr=0; // current pressure ajusted flow rate
	int tep=0, cep=0; // target equivelent position, current equivelent position
	int iterations=0; // this just checks for the thing looping forever
	int torque=0;
	MotorInstance.GoTo(maxspeed,maxacc,SafePoint);
	tep=position_assosiated_with_pafr(local_tpafr);
	while((iterations<maxiterations)
				&&(torque<maxtorque)
				&&(MotorInstance.Position()<too_far))
		{
			MotorInstance.GoTo(maxspeed,maxacc,(MotorInstance.Position()-dist_to_back_off)); // this backs the motor off so I can get a reading without bounce
			cpafr=FlowMasterInstance.pafr();
			printf("iterations: %d position: %d pafr: %f\n",iterations,MotorInstance.Position(),cpafr);
			if(cpafr<=(local_tpafr*(1+tferror))) // this breaks out of the loop if the flow is low enough to warrant it
			{
				MotorInstance.GoTo(maxspeed,maxacc,NearPoint);
				FlowMasterInstance.LockDown(); // this closes the valves to make it safe for the user to open the jig
				return (cpafr); // this returns the achieved flowrate, the calling function must decide if this flow is acceptable
			}
			else // this does another iteration of the setting process
			{
				cep=position_assosiated_with_pafr(cpafr);
				//torque=motor_instance.pseudotorque();     // this line is not useful as the torque measurement is broken
				MotorInstance.GoTo(maxspeed,maxacc,(MotorInstance.Position()+dist_to_back_off));// this moves the motor back to where it was
				MotorInstance.GoTo(maxspeed/3,maxacc,(MotorInstance.Position()+fract2move*(tep-cep))); // I do this as two seperate moves so that the motor will be going slower for the fine movement at the end
				Sleep(time_for_stabilization);
				iterations++;
			}
		}
	if(!(iterations<maxiterations))
		printf("error: too many iterations,\n this program can handle an infinite number of iterations but the fact that there have been %d iterations without the desired flow rate being reached indicates something else is wrong\n",maxiterations);
	if(!(MotorInstance.Position()<too_far))
		printf("error: program stopped motor because it went too far\n");
	if(!(MotorInstance.PseudoTorque()<maxtorque))
		printf("caliberation stopped due to excess torque\n");
	FlowMasterInstance.LockDown(); // this closes the valves to make it safe for the user to open the jig
	return(-2.0);
}
*/
int HandleResult(float rpafr, float LocalTpafr)
{
	if (rpafr==-2.0)
	{
		// this the code for do nothing, things have been handled in the "perform" function and I decided not to put the code here
		// this should also be unreachable through experemental readings unless the flowmeters are backwards
		return 0;
	}
	if((((1-tfError)*LocalTpafr)<=rpafr)&&(rpafr<=((1+tfError)*LocalTpafr)))
	{
		printf("success: pressure adjusted flow rate=%f\n",rpafr);
		return 0;
	}
	else if(!(((1-tfError)*LocalTpafr)<=rpafr))
	{
		printf("failure: set flow too small, pressure adjusted flow rate=%f SLPM per bar\n",rpafr);
		return 0;
	}
	else if (!(rpafr<=((1+tfError)*LocalTpafr)))
	{
		printf("failure: set flow too large, pressure adjusted flow rate=%f SLPM per bar\n",rpafr);
		return 0;
	}
	else
	{
		printf("obscure error in 'handle result', call a programmer\n");
		return 1;
	}

}
int List()
{
	printf("command  purpose\n\n");
	printf("s        sets a flow\n");
	printf("e        exits program\n");
	printf("l        lists possible commands\n");
	printf("v        view settings\n");
	printf("c        creates and uses a new calibration data file\n");
	printf("cbo      creates back off calibration data file\n");
	//printf("f        fixes motor free state (motor being unresponsive)"); // found to not work properly
	printf("number   sets a flow of 'number' SLPM per bar\n\n");
	return 0;
}
int ReadSettings()
{
	FILE * pFile;
	pFile = fopen ("settings.txt","r");
	if(pFile==NULL)
		printf("could not open settings file");
	fscanf (pFile, "%d safe point \n",&SafePoint);
	fscanf (pFile, "%d near point \n",&NearPoint);
	fscanf (pFile, "%d motor gone too far point \n",&TooFar);
	fscanf (pFile, "%d motor speed \n",&MaxSpeed);
	fscanf (pFile, "%d motor acceleration \n",&MaxAcc);
	fscanf (pFile, "%d maximum iterations in setting (should be large)\n",&MaxIterations);
	fscanf (pFile, "%f gas factor (not currently implemented)\n",&GasFactor);
	fscanf (pFile, "%f target pressure (not currently implemented)\n",&tPressure);
	fscanf (pFile, "%f minimum pressure (not currently implemented)\n",&MinPressure);
	fscanf (pFile, "%f maximum pressure (not currently implemented)\n",&MaxPressure);
	fscanf (pFile, "%f target pressure adjusted flow rate \n",&tpafr);
	fscanf (pFile, "%f target fractional error \n",&tfError);
	fscanf (pFile, "%d number of the calibration file to be used \n",&CalibrationDataNum);
	fscanf (pFile, "%d number of the most recently written calibration file \n",&MostRecentlyWrittenCaliberationDataNum);
	fscanf (pFile, "%f time for stabilization\n",&TimeForStabilization);
	fscanf (pFile, "%f fraction to final position to move on each iteration \n",&FractToMove);
	fscanf (pFile, "%f maximum torque\n",&MaxTorque);
	fscanf (pFile, "%d number of the algorithm used for setting flows\n",&AlgoNum);
	fscanf (pFile, "%d amount to move on each iteration of calibration data generation\n",&AmountToAdd);
	fscanf (pFile, "%d debugging (set to 1 for debugging and 0 for normal use)\n",&Debugging);
	fscanf (pFile, "%f maximum bounce expected measured in SLPM per Bar\n",&MaxBounceExpected);
	fscanf (pFile, "%d distance to back off, this is used when the motor backs off to get a measurement includeing the bounce effect\n",&DistToBackOff);
	fscanf (pFile, "%c release valve channel\n\n",&ReleaseValveChannel);

	fscanf (pFile, "%d motor port (the port number should be set to -1 to signal when the device does not exist)\n",&Motor1Port);
	fscanf (pFile, "%d weeder io port (the port number should be set to -1 to signal when the device does not exist) \n",&Weederio1Port);
	fscanf (pFile, "%d pressure transducer port (the port number should be set to -1 to signal when the device does not exist)\n",&PressureTransducer1Port);
	fscanf (pFile, "%d flowmeter 1 port (the port number should be set to -1 to signal when the device does not exist)\n",&FlowMeter1Port);
	fscanf (pFile, "%c weeder io channel for flowmeter 1(should be a capital letter)", &FlowMeter1Channel);
	fscanf (pFile, "%f flowmeter 1 maximum flow (slpm)\n",&FlowMeter1MaxFlow);
	fscanf (pFile, "%d flowmeter 1 units correction (1 for litres per min 1000 for ml)\n",&FlowMeter1UnitsCorrection);
	fscanf (pFile, "%d flowmeter 2 port (the port number should be set to -1 to signal when the device does not exist)\n",&FlowMeter2Port);
	fscanf (pFile, "%c weeder io channel for flowmeter 2(should be a capital letter)", &FlowMeter2Channel);
	fscanf (pFile, "%f flowmeter 2 maximum flow (slpm)\n",&FlowMeter2MaxFlow);
	fscanf (pFile, "%d flowmeter 2 units correction (1 for litres per min 1000 for ml)\n",&FlowMeter2UnitsCorrection);
	fscanf (pFile, "%d flowmeter 3 port (the port number should be set to -1 to signal when the device does not exist)\n",&FlowMeter3Port);
	fscanf (pFile, "%c weeder io channel for flowmeter 3(should be a capital letter)", &FlowMeter3Channel);
	fscanf (pFile, "%f flowmeter 3 maximum flow (slpm) \n",&FlowMeter3MaxFlow);
	fscanf (pFile, "%d flowmeter 3 units correction (1 for litres per min 1000 for ml)\n",&FlowMeter3UnitsCorrection);
	fscanf (pFile, "%d flowmeter 4 port (the port number should be set to -1 to signal when the device does not exist)\n",&FlowMeter4Port);
	fscanf (pFile, "%c weeder io channel for flowmeter 4(should be a capital letter)", &FlowMeter4Channel);
	fscanf (pFile, "%f flowmeter 4 maximum flow (slpm)\n",&FlowMeter4MaxFlow);
	fscanf (pFile, "%d flowmeter 4 units correction (1 for litres per min 1000 for ml)\n",&FlowMeter4UnitsCorrection);
	fscanf (pFile, "%d flowmeter 5 port (the port number should be set to -1 to signal when the device does not exist)\n",&FlowMeter5Port);
	fscanf (pFile, "%c weeder io channel for flowmeter 5(should be a capital letter)", &FlowMeter5Channel);
	fscanf (pFile, "%f flowmeter 5 maximum flow (slpm)\n",&FlowMeter5MaxFlow);
	fscanf (pFile, "%d flowmeter 5 units correction (1 for litres per min 1000 for ml) \n\n",&FlowMeter5UnitsCorrection);

	fscanf (pFile, "%d don't change me, I test if settings are read correctly\n\n", &BrokenSettingsTest);
	if (BrokenSettingsTest!=1234567890)
	{
		printf("did not read settings file correctly\n");
		system("PAUSE");
	}
	fclose (pFile);
	return 0;
}
int ViewSettings()
{
	printf ("most important current settings are:\n");
	printf ("safe point: %d\n",SafePoint);
	printf ("calibration data file number: %d\n",CalibrationDataNum);
	printf ("maxiterations: %d\n",MaxIterations);
	printf ("target pressure adjusted flow rate: %f\n",tpafr);
	printf ("target fractional error: %f\n",tfError);
	printf ("time for stabilization of flow rate: %f\n",TimeForStabilization);
	printf ("fraction of distance to target to move: %f\n",FractToMove);
	printf ("to see all the settings look in settings file\n\n");
	return 0;
}
int WriteSettings()
{
	FILE * pFile;
	pFile = fopen ("settings.txt","w+");
	fprintf (pFile, "%d \t\tsafe point \n",SafePoint);
	fprintf (pFile, "%d \t\tnear point \n",NearPoint);
	fprintf (pFile, "%d \tmotor gone too far point \n",TooFar);
	fprintf (pFile, "%d \tmotor speed \n",MaxSpeed);
	fprintf (pFile, "%d \tmotor acceleration \n",MaxAcc);
	fprintf (pFile, "%d \t\tmaximum iterations in setting (should be large)\n",MaxIterations);
	fprintf (pFile, "%f \tgas factor (not currently implemented)\n",GasFactor);
	fprintf (pFile, "%f \ttarget pressure (not currently implemented)\n",tPressure);
	fprintf (pFile, "%f \tminimum pressure (not currently implemented)\n",MinPressure);
	fprintf (pFile, "%f \tmaximum pressure (not currently implemented)\n",MaxPressure);
	fprintf (pFile, "%f \ttarget pressure adjusted flow rate \n",tpafr);
	fprintf (pFile, "%f \ttarget fractional error \n",tfError);
	fprintf (pFile, "%d \t\tnumber of the calibration file to be used \n",CalibrationDataNum);
	fprintf (pFile, "%d \t\tnumber of the most recently written calibration file \n",MostRecentlyWrittenCaliberationDataNum);
	fprintf (pFile, "%f \ttime for stabilization\n",TimeForStabilization);
	fprintf (pFile, "%f \tfraction to final position to move on each iteration \n",FractToMove);
	fprintf (pFile, "%f \tmaximum torque\n",MaxTorque);
	fprintf (pFile, "%d \t\tnumber of the algorithm used for setting flows\n",AlgoNum);
	fprintf (pFile, "%d \t\tamount to move on each iteration of calibration data generation\n",AmountToAdd);
	fprintf (pFile, "%d \t\tdebugging (set to 1 for debugging and 0 for normal use)\n",Debugging);
	fprintf (pFile, "%f \t\tmaximum bounce expected measured in SLPM per Bar\n",MaxBounceExpected);
	fprintf (pFile, "%d \t\tdistance to back off, this is used when the motor backs off to get a measurement includeing the bounce effect\n",DistToBackOff);
	fprintf (pFile, "%c \t\trelease valve channel\n\n",ReleaseValveChannel);

	fprintf (pFile, "%d \t\tmotor port (the port number should be set to -1 to signal when the device does not exist)\n",Motor1Port);
	fprintf (pFile, "%d \t\tweeder io port (the port number should be set to -1 to signal when the device does not exist) \n",Weederio1Port);
	fprintf (pFile, "%d \t\tpressure transducer port (the port number should be set to -1 to signal when the device does not exist)\n\n",PressureTransducer1Port);
	fprintf (pFile, "%d \t\tflowmeter 1 port (the port number should be set to -1 to signal when the device does not exist)\n",FlowMeter1Port);
	fprintf	(pFile, "%c \t\tweeder io channel for flowmeter 1(should be a capital letter)\n", FlowMeter1Channel);
	fprintf (pFile, "%f \t\tflowmeter 1 maximum flow (slpm)\n",FlowMeter1MaxFlow);
	fprintf (pFile, "%d \t\tflowmeter 1 units correction (1 for litres per min 1000 for ml)\n\n",FlowMeter1UnitsCorrection);
	fprintf (pFile, "%d \t\tflowmeter 2 port (the port number should be set to -1 to signal when the device does not exist)\n",FlowMeter2Port);
	fprintf	(pFile, "%c \t\tweeder io channel for flowmeter 2(should be a capital letter)\n", FlowMeter2Channel);
	fprintf (pFile, "%f \t\tflowmeter 2 maximum flow (slpm)\n",FlowMeter2MaxFlow);
	fprintf (pFile, "%d \t\tflowmeter 2 units correction (1 for litres per min 1000 for ml)\n\n",FlowMeter2UnitsCorrection);
	fprintf (pFile, "%d \t\tflowmeter 3 port (the port number should be set to -1 to signal when the device does not exist)\n",FlowMeter3Port);
	fprintf	(pFile, "%c \t\tweeder io channel for flowmeter 3(should be a capital letter)\n", FlowMeter3Channel);
	fprintf (pFile, "%f \t\tflowmeter 3 maximum flow (slpm) \n",FlowMeter3MaxFlow);
	fprintf (pFile, "%d \t\tflowmeter 3 units correction (1 for litres per min 1000 for ml)\n\n",FlowMeter3UnitsCorrection);
	fprintf (pFile, "%d \t\tflowmeter 4 port (the port number should be set to -1 to signal when the device does not exist)\n",FlowMeter4Port);
	fprintf	(pFile, "%c \t\tweeder io channel for flowmeter 4(should be a capital letter)\n", FlowMeter4Channel);
	fprintf (pFile, "%f \t\tflowmeter 4 maximum flow (slpm)\n",FlowMeter4MaxFlow);
	fprintf (pFile, "%d \t\tflowmeter 4 units correction (1 for litres per min 1000 for ml)\n\n",FlowMeter4UnitsCorrection);
	fprintf (pFile, "%d \t\tflowmeter 5 port (the port number should be set to -1 to signal when the device does not exist)\n",FlowMeter5Port);
	fprintf	(pFile, "%c \t\tweeder io channel for flowmeter 5(should be a capital letter)\n", FlowMeter5Channel);
	fprintf (pFile, "%f \t\tflowmeter 5 maximum flow (slpm)\n",FlowMeter5MaxFlow);
	fprintf (pFile, "%d \t\tflowmeter 5 units correction (1 for litres per min 1000 for ml) \n\n",FlowMeter5UnitsCorrection);
	fprintf (pFile, "%d \tdon't change me, I test if settings are read correctly\n\n", 1234567890);

	fprintf (pFile,"******* end of data ******* \n\n the numbers in this file can be modified by the end user, you probably wish to back it up first though. \nAlso you can change the text after 'end of data' how ever you wish and it will be ignored by the program but\n if you change any of the text before the end of data marker the program will break");
	fclose (pFile);
	return 0;
}

