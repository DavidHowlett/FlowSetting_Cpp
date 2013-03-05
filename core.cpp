// Environment Resources ----------------------------------------------------
#include <iostream>
#include <Classes.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <string>
// My Header Files ----------------------------------------------------------
#include "Motor.h"
#include "FlowMaster.h"
#include "FlowSetterSettings.h"
#include "Hidden.h"       // this contains documentation
// My Classes ---------------------------------------------------------------
Motor MotorInstance;
FlowMaster FlowMasterInstance;
FlowSetterSettings Settings;
//other global variables********************************************************
bool RunAgain=true;
const int MaxTableSize=10000;        // I can't use a varable as this value is needed at compile time
int MotorPositionTable[MaxTableSize];
float PafrTable[MaxTableSize]; // pressure is measured in bar, pafr = presure ajusted flow rate measured in ml/bar/min

//function declarations*********************************************************
int Setup(); // this should only be called at startup and should contain everything needed to initialise the system
int UserInterationAndAction(); // this asks the user what to do next and calls the apropriate code
int ReadCalibrationData(); // this is a decleration for a function that reads calibration data.txt and stores it in an array
int CreateCaliberationData(); // this slowly moves the motor forwards while monitoring the flow and then records to file a curve that relates position to flow for the prupose of caliberateing future movements
//int CreateBackOffCaliberationData(); // this works in a similar way to the above but also records flow rate readings before and after backing off
int PositionAssosiatedWithPafr(float pafr); //note this kind of thing does not initialise pafr
float Perform(float tpafr); // tpafr =(target pressure ajusted flow rate), function returns achived pafr
float AlgorithmWithoutBounceProtection(float Local_tpafr);// this is the primary flow setting algorithum. it iteratively estimates the final position based on flow rate and moves 50% of the distance to the estimated final position. tpafr =(target pressure ajusted flow rate), function returns achived pafr
float AlgorithmWithoutBounceProtectionForInternalUse(float Local_tpafr); // this is almost the same as the above but it does not back off from the jig. Also it does not lock down after finishing
float AlgorithmWithBounceProtection(float Local_tpafr);// first this calls the main algorithum to get the flow close to it's final value, then it enters a second phase where it acts like the main algoritum with the one change that it backs off before each flow measurement.
int HandleResult(float rpafr, float tpafr); // this looks at the flow rate achived by the setting algorithum, decides if it is within alowed limits and tells the user
int List(); // this lists all alowed commands
int ViewSettings(); // this displays the more important settings
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
	if((Answer[0]<=57)&&(Answer[0]>=48)){ // this piece of code tests if the input was a number
		ToBeSet=atof(Answer);
		IsNumeric=true;
	}
	if(! strcmp (Answer, "s")){// note strcmp outputs the opposite of what you would expect
		rpafr=Perform(Settings.tpafr); // this tells the perform function to try to set the pafr to target pafr
		HandleResult(rpafr,Settings.tpafr);
	}
	else if(! strcmp (Answer, "e"))
		RunAgain=false;
	else if(! strcmp (Answer, "l"))
		List();
	//else if(! strcmp (answer, "m"))
	//	manual();
	else if(! strcmp (Answer, "v"))
		ViewSettings();
	//else if(! strcmp (Answer, "cbo"))
	//	CreateBackOffCaliberationData();
	else if(! strcmp (Answer, "c"))
		CreateCaliberationData();
	else if(IsNumeric){
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
	Settings.ReadFile(); // it matters that read settings is before I get the caliberation data
	//write_settings();
	ReadCalibrationData();
	FlowMasterInstance.Setup(&Settings);
	//flowmaster_instance.lockdown();// this closes all known valves
	MotorInstance.Setup(Settings.MotorPort);
	MotorInstance.SetOrigin();
	printf("pressure: %f bar  ",FlowMasterInstance.Pressure());
	printf("massflow: %f SLPM  ",FlowMasterInstance.MassFlow());
	printf("pafr: %f SPLM/bar\n",FlowMasterInstance.pafr());
	return 0;
}
int ReadCalibrationData()
{
	char Name[30];
	sprintf(Name,"calibration data%d.txt",Settings.CalibrationDataNum);  // these lines of code assemble the name of the calibaration data file
	int PositionInArray=0;
	for(PositionInArray=0; PositionInArray<=MaxTableSize-1; PositionInArray++){
		MotorPositionTable[PositionInArray]=1000000000;
		PafrTable[PositionInArray]=0;  // these two lines initialise my caliberation array to harmless values
	}
	FILE * pFile;
	pFile = fopen (Name,"r");// opens the caliberation data file
	for(PositionInArray=0;PositionInArray<=MaxTableSize-1; PositionInArray++){   // the data terminates with 987654321 0, the motor will never go that far
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
	int CurrentAmountToAdd=Settings.AmountToAdd;
	FlowMasterInstance.Reset();
	Sleep(2*Settings.TimeForStabilization);
	CalMotorPositionTable[0]=-1000000000;
	CalPafrTable[0]=700000;           // this and the above line just ensure that if flows are requested that are above the caliberration curve's highest point then the system simply moves forwards slowly
	for(PositionInArray=1; PositionInArray<=MaxTableSize-1;PositionInArray++){
		CalMotorPositionTable[PositionInArray]=1000000; // this is harmless and it is the end of data marker
		CalPafrTable[PositionInArray]=0;  // these two lines initialise my caliberation array to harmless values
	}
	MotorInstance.GoTo(Settings.MaxSpeed,Settings.MaxAcc,Settings.SafePoint);
	float pafr=FlowMasterInstance.pafr();
	for(PositionInArray=1;(	pafr>0.001)
							&&(Torque<Settings.MaxTorque)
							&&(PositionInArray<=(MaxTableSize-1))
							&&(MotorPosition<Settings.TooFar);PositionInArray++){   // this while sets the point of cut off after which caliberation ends
		MotorInstance.GoTo(Settings.MaxSpeed,Settings.MaxAcc,(MotorInstance.Position()+CurrentAmountToAdd));
		Sleep(Settings.TimeForStabilization);
		CalMotorPositionTable[PositionInArray]=MotorPosition=MotorInstance.Position();
		CalPafrTable[PositionInArray]=pafr=FlowMasterInstance.pafr();
		Torque=MotorInstance.PseudoTorque();
		printf(	"position: %d  pseudotorque: %d  massflow: %f  pafr: %f\n"
				,CalMotorPositionTable[PositionInArray],Torque,FlowMasterInstance.MassFlow(),CalPafrTable[PositionInArray]);
		if (pafr<0.5)
			CurrentAmountToAdd=Settings.AmountToAdd/5;
		if (pafr<0.1)
			CurrentAmountToAdd=Settings.AmountToAdd/50;
	}
	if(!(pafr>0.001))
		printf("caliberation stopped because low flow was reached\n");
	if(!(MotorInstance.PseudoTorque()<Settings.MaxTorque))
		printf("caliberation stopped due to excess torque\n");
	if(!(PositionInArray<=(MaxTableSize-1)))
		printf("caliberation stopped due to running out of space in this program's array\n");
	if(!(MotorInstance.Position()<Settings.TooFar))
		printf("caliberation stopped due to motor going too far\n");
	Settings.MostRecentlyWrittenCaliberationDataNum++;
	Settings.CalibrationDataNum=Settings.MostRecentlyWrittenCaliberationDataNum;
	Settings.WriteFile();
	char Name[30];
	sprintf(Name,"calibration data%d.txt",Settings.CalibrationDataNum);  // these lines of code assemble the name of the calibaration data file
	FILE * pFile;
	pFile = fopen (Name,"w+");
	if( pFile==NULL)
		printf("error: could not access calibration file");
	else{
		for(PositionInArray=0; PositionInArray<=(MaxTableSize-1); PositionInArray++)   // the data terminates with 987654321 0, the motor will never go that far
			fprintf (pFile, "%d\t%f\n", CalMotorPositionTable[PositionInArray], CalPafrTable[PositionInArray]);
	}
	FlowMasterInstance.LockDown();
	MotorInstance.GoTo(Settings.MaxSpeed,Settings.MaxAcc,Settings.NearPoint); // NearPoint= near point to origin
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
	if(Settings.AlgoNum==1)
		return(AlgorithmWithoutBounceProtection(LocalTpafr));
	//if(AlgonNm==2)
	//	return(AlgorithmWithBounceProtection(LocalTpafr));
	else{
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
	MotorInstance.GoTo(Settings.MaxSpeed,Settings.MaxAcc,Settings.SafePoint);
	FlowMasterInstance.Reset();
	Sleep(2*Settings.TimeForStabilization);
	float cpafr=FlowMasterInstance.pafr(); // current pressure ajusted flow rate
	int tep=PositionAssosiatedWithPafr(LocalTpafr);// target equivelent position, current equivelent position
	int cep=PositionAssosiatedWithPafr(cpafr);
	if (LocalTpafr<0.0001)
	{
		printf("flow too small to set\n");
		FlowMasterInstance.LockDown();
		return (-2);
	}
	if (cpafr<=(LocalTpafr*(1+Settings.tfError)))
	{
		FlowMasterInstance.LockDown();
		return (cpafr);
	}
	while((Iterations<Settings.MaxIterations)
				&&(Torque<Settings.MaxTorque)
				&&(MotorPosition<Settings.TooFar))
		{
			cpafr=FlowMasterInstance.pafr();
			printf("iterations: %d position: %d pafr: %f\n",Iterations,MotorPosition,cpafr);
			if((Flag>=1)||(cpafr<=LocalTpafr))// this breaks out of the loop if the flag is high or if it has gone below it's target flow
			{
				MotorInstance.GoTo(Settings.MaxSpeed,Settings.MaxAcc,Settings.NearPoint);
				Sleep(Settings.TimeForStabilization); // I sleep for a second time because the flow may change as the motor backs off
				cpafr=FlowMasterInstance.pafr();
				FlowMasterInstance.LockDown();
				return (cpafr); // this returns the achieved flowrate, the calling function must decide if this flow is acceptable
			}
			if(cpafr<=(LocalTpafr*(1+Settings.tfError))) // this breaks out of the loop if the flow is low enough to warrant it
				Flag++;
			//torque=motor_instance.pseudotorque();     // this line is not useful
			cep=PositionAssosiatedWithPafr(cpafr);
			MotorPosition=MotorInstance.Position();
			MotorInstance.GoTo(Settings.MaxSpeed,Settings.MaxAcc,(MotorPosition+Settings.FractToMove*(tep-cep)));
			Sleep(Settings.TimeForStabilization);
			Iterations++;
		}
	if(!(Iterations<Settings.MaxIterations))
		printf("error: too many iterations,\n this program can handle an infinite number of iterations but the fact that there have been %d iterations without the desired flow rate being reached indicates something else is wrong\n",Settings.MaxIterations);
	if(!(MotorInstance.Position()<Settings.TooFar))
		printf("error: program stopped motor because it went too far\n");
	if(!(MotorInstance.PseudoTorque()<Settings.MaxTorque))
		printf("caliberation stopped due to excess torque\n");
	FlowMasterInstance.LockDown();
	MotorInstance.GoTo(Settings.MaxSpeed,Settings.MaxAcc,Settings.NearPoint); // NearPoint= near point to origin
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
	MotorInstance.GoTo(MaxSpeed,MaxAcc,NearPoint); // NearPoint= near point to origin
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
	if((((1-Settings.tfError)*LocalTpafr)<=rpafr)&&(rpafr<=((1+Settings.tfError)*LocalTpafr)))
	{
		printf("success: pressure adjusted flow rate=%f\n",rpafr);
		return 0;
	}
	else if(!(((1-Settings.tfError)*LocalTpafr)<=rpafr))
	{
		printf("failure: set flow too small, pressure adjusted flow rate=%f SLPM per bar\n",rpafr);
		return 0;
	}
	else if (!(rpafr<=((1+Settings.tfError)*LocalTpafr)))
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
int ViewSettings()
{
	printf ("most important current settings are:\n");
	printf ("safe point: %d\n",Settings.SafePoint);
	printf ("calibration data file number: %d\n",Settings.CalibrationDataNum);
	printf ("maxiterations: %d\n",Settings.MaxIterations);
	printf ("target pressure adjusted flow rate: %f\n",Settings.tpafr);
	printf ("target fractional error: %f\n",Settings.tfError);
	printf ("time for stabilization of flow rate: %f\n",Settings.TimeForStabilization);
	printf ("fraction of distance to target to move: %f\n",Settings.FractToMove);
	printf ("to see all the settings look in settings file\n\n");
	return 0;
}
