// Environment Resources ----------------------------------------------------
#include <iostream>
#include <Classes.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <string>
// My Header Files ----------------------------------------------------------
#include "Motor.h"
#include "flowmaster.h"
#include "Hidden.h"       // this contains documentation
// My Classes ---------------------------------------------------------------
Motor motor_instance;
flowmaster flowmaster_instance;
// Settings -----------------------------------------------------------------
// if settings are working correctly all the below values will be overwritten in setup()
int spoint=0; // this is the safe point near the jig
int npoint=0; // this is an alowed near point to the origin where it does not touch the stopper
int too_far=0; // this is to test if the motor has gone too far and there is an error
int maxspeed=0; // maximum speed
int maxacc=0; // maximum acceleration
int maxiterations=0;  // checks for failure due to infinite iterations
float gas_factor=1; //the flows will be set for air but used for othe gasses
float tpressure=0;   // this is the target pressure, not used yet
float minpressure=0; //not used yet
float maxpressure=0; //not used yet
float tpafr=0;   // this is the target pressure adjusted flow rate
float tferror=0; // this is the target fractional error eg 0.05 means +- 5% is ok
int calibration_data_num=0; //this enables multiple caliberation ccurves to be stored
int most_recently_written_caliberation_data_num=0;
float time_for_stabilization=0;//measured in miliseconds, change for good results
float fract2move=0; // the fraction of the distance to the estimated destination that the motor should move each time
float maxtorque=0; // I can't measure torque only related quantitys so beware
int algonum=0; // this describes which algorithm is used for setting flows
int amount_to_add=0; // this describes how far the motor moves during each iteration of caliberation data generation
int debugging=0; //this should be 1 for debugging mode but 0 for normal use
float max_bounce_expected=0; // the units are SLPM per Bar
int dist_to_back_off=0;// this is used when the motor backs off to get a measurement includeing the bounce effect
char release_valve_channel='A';


int   motor1port=-1; // the port number being -1 signals that the device does not exist
int   weederio1port=-1; // the port number being -1 signals that the device does not exist
int   pressure_transducer1port=-1; // the port number being -1 signals that the device does not exist
int   flowmeter1port=-1;// the port number being -1 signals that the device does not exist
char  flowmeter1channel='A';
float flowmeter1maxflow= 0; //this is in standard litres per min
int   flowmeter1units_correction=1; // the flow data is divided by this value, should be 1 for litres per min and 1000 for ml
int   flowmeter2port   = 0;// the port number being -1 signals that the device does not exist
char  flowmeter2channel='A';
float flowmeter2maxflow= 0; //this is in standard litres per min
int   flowmeter2units_correction=1; // the flow data is divided by this value, should be 1 for litres per min and 1000 for ml
int   flowmeter3port   =-1;// the port number being -1 signals that the device does not exist
char  flowmeter3channel='A';
float flowmeter3maxflow= 0; //this is in standard litres per min
int   flowmeter3units_correction=1; // the flow data is divided by this value, should be 1 for litres per min and 1000 for ml
int   flowmeter4port=-1;// the port number being -1 signals that the device does not exist
char  flowmeter4channel='A';
float flowmeter4maxflow=0; //this is in standard litres per min
int   flowmeter4units_correction=1; // the flow data is divided by this value, should be 1 for litres per min and 1000 for ml
int   flowmeter5port=-1;// the port number being -1 signals that the device does not exist
char  flowmeter5channel='A';
float flowmeter5maxflow=0; //this is in standard litres per min
int   flowmeter5units_correction=1; // the flow data is divided by this value, should be 1 for litres per min and 1000 for ml
int   broken_settings_test=0; // this is overwriten with a known value to check that the read funtion got to the end of the file ok

//settings not yet added to settings file***************************************

//other global variables********************************************************
bool run_again=true;
const int max_table_size=10000;        // I can't use a varable as this value is needed at compile time
int motor_position_table[max_table_size];
float pafr_table[max_table_size]; // pressure is measured in bar, pafr = presure ajusted flow rate measured in ml/bar/min

//function declarations*********************************************************
int setup(); // this should only be called at startup and should contain everything needed to initialise the system
int user_interation_and_action(); // this asks the user what to do next and calls the apropriate code
int read_settings(); // this reads all of the settings from the settings file
int test();  // this was used as a place for code used in debugging, should not be called by end user
int read_calibration_data(); // this is a decleration for a function that reads calibration data.txt and stores it in an array
int create_caliberation_data(); // this slowly moves the motor forwards while monitoring the flow and then records to file a curve that relates position to flow for the prupose of caliberateing future movements
int create_back_off_caliberation_data(); // this works in a similar way to the above but also records flow rate readings before and after backing off
int position_assosiated_with_pafr(float pafr); //note this kind of thing does not initialise pafr
float perform(float tpafr); // tpafr =(target pressure ajusted flow rate), function returns achived pafr
float algorithm_without_bounce_protection(float local_tpafr);// this is the primary flow setting algorithum. it iteratively estimates the final position based on flow rate and moves 50% of the distance to the estimated final position. tpafr =(target pressure ajusted flow rate), function returns achived pafr
float algorithm_without_bounce_protection_for_internal_use(float local_tpafr); // this is almost the same as the above but it does not back off from the jig. Also it does not lock down after finishing
float algorithm_with_bounce_protection(float local_tpafr);// first this calls the main algorithum to get the flow close to it's final value, then it enters a second phase where it acts like the main algoritum with the one change that it backs off before each flow measurement.
float extra_accurate_algorithm(float local_tpafr);// just a place holder, currently calls algorithm_with_bounce_protection
int handle_result(float rpafr, float tpafr); // this looks at the flow rate achived by the setting algorithum, decides if it is within alowed limits and tells the user
int list(); // this lists all alowed commands
int view_settings(); // this displays the more important settings
int write_settings(); // this writes the current settings to the settings file
//int manual();

//******************************************************************************
int _tmain(int argc, _TCHAR* argv[])
{
	setup();
	while(run_again==true)
		user_interation_and_action();
	flowmaster_instance.lockdown();
	printf("program finished \n");
	system("PAUSE");
	return 0;
}
//function definitions**********************************************************
int user_interation_and_action()
{
	char answer[100];
	double to_be_set; // only used when the user enters a number
	bool isNumeric;
	float rpafr=0; // the resulting pressure ajusted flow rate from "perform"
	//motor_instance.go_to(maxspeed,maxacc,npoint); //speed, acceleration, distance
	printf("enter 's' to set a flow, 'e' for exit or 'l' for a list of possible commands\n");
	scanf("%s",answer);
//**************// the below piece of code tests if the input was a number
	if((answer[0]<=57)&&(answer[0]>=48))
	{
		to_be_set=atof( answer );
		isNumeric=true;
	}
//**************
	if(! strcmp (answer, "s")){// note strcmp outputs the opposite of what you would expect
		rpafr=perform(tpafr); // this tells the perform function to try to set the pafr to target pafr
		handle_result(rpafr,tpafr);
	}
	else if(! strcmp (answer, "e"))
		run_again=false;
	else if(! strcmp (answer, "l"))
		list();
	//else if(! strcmp (answer, "m"))
	//	manual();
	else if(! strcmp (answer, "v"))
		view_settings();
	else if(! strcmp (answer, "cbo"))
		create_back_off_caliberation_data();
	else if(! strcmp (answer, "c"))
		create_caliberation_data();
	else if(isNumeric)
	{
		// commented out code interprets input as SLPM, used code interprets input as pafr
		//float temp_target_pafr=to_be_set/flowmaster_instance.pressure();
		//printf("target pressure adjusted flow rate: %f\n",temp_target_pafr);
		//rpafr=perform(temp_target_pafr); // tells the perform function to set the target pafr and send the achived flow to rpafr
		//handle_result(rpafr,temp_target_pafr);
		rpafr=perform(to_be_set); // tells the perform function to set the target pafr and send the achived flow to rpafr
		handle_result(rpafr,to_be_set);
	}
	else
		printf("command not recognised\n");
	return(0);
}
int setup()
{
	read_settings(); // it matters that read settings is before I get the caliberation data
	//write_settings();
	read_calibration_data();
	flowmaster_instance.setup(flowmeter1port, flowmeter1channel,flowmeter1maxflow,flowmeter1units_correction,
														flowmeter2port, flowmeter2channel,flowmeter2maxflow,flowmeter2units_correction,
														flowmeter3port, flowmeter3channel,flowmeter3maxflow,flowmeter3units_correction,
														flowmeter4port, flowmeter4channel,flowmeter4maxflow,flowmeter4units_correction,
														flowmeter5port, flowmeter5channel,flowmeter5maxflow,flowmeter5units_correction,
														weederio1port,  pressure_transducer1port, time_for_stabilization, release_valve_channel);
	//flowmaster_instance.lockdown();// this closes all known valves
	motor_instance.Setup(motor1port);
	motor_instance.SetOrigin();
	printf("pressure: %f bar  ",flowmaster_instance.pressure());
	printf("massflow: %f SLPM  ",flowmaster_instance.massflow());
	printf("pafr: %f SPLM/bar\n",flowmaster_instance.pafr());
	if(debugging)//this should be 1 for debugging mode but 0 for normal use
		test();
	return (0);
}
int read_calibration_data()
{
	char num[10];
	char name[30];
	sprintf(num,"%d",calibration_data_num);  // these lines of code assemble the name of the calibaration data file
	strcpy(name,"calibration data");
	strcat(name,num);
	strcat(name,".txt");
	int position_in_array=0;
	for(position_in_array=0; position_in_array<=max_table_size-1; position_in_array++)
	{
		motor_position_table[position_in_array]=1000000000;
		pafr_table[position_in_array]=0;  // these two lines initialise my caliberation array to harmless values
	}
	FILE * pFile;
	pFile = fopen (name,"r");// opens the caliberation data file
	for(position_in_array=0;position_in_array<=max_table_size-1; position_in_array++)   // the data terminates with 987654321 0, the motor will never go that far
	{
		fscanf (pFile, "%d %f", &motor_position_table[position_in_array], &pafr_table[position_in_array]);
		// printf ("I have read: %d and %f \n",motor_position_table[position_in_array],pafr_table[position_in_array]);
	}
	fclose (pFile);
	return 0;
}
int create_caliberation_data()
{
	int cal_motor_position_table[max_table_size];
	float cal_pafr_table[max_table_size];
	int position_in_array=0;
	int torque=0;
	int motorposition=0;
	int current_amount_to_add=amount_to_add;
	flowmaster_instance.reset();
	Sleep(2*time_for_stabilization);
	cal_motor_position_table[0]=-1000000000;
	cal_pafr_table[0]=700000;           // this and the above line just ensure that if flows are requested that are above the caliberration curve's highest point then the system simply moves forwards slowly
	for(position_in_array=1; position_in_array<=max_table_size-1;position_in_array++)
	{
		cal_motor_position_table[position_in_array]=1000000; // this is harmless and it is the end of data marker
		cal_pafr_table[position_in_array]=0;  // these two lines initialise my caliberation array to harmless values
	}
	motor_instance.GoTo(maxspeed,maxacc,spoint);
	float pafr=flowmaster_instance.pafr();
	for(position_in_array=1;(pafr>0.001)
												&&(torque<maxtorque)
												&&(position_in_array<=(max_table_size-1))
												&&(motorposition<too_far);position_in_array++)   // this while sets the point of cut off after which caliberation ends
	{
		motor_instance.GoTo(maxspeed,maxacc,(motor_instance.Position()+current_amount_to_add));
		Sleep(time_for_stabilization);
		cal_motor_position_table[position_in_array]=motorposition=motor_instance.Position();
		cal_pafr_table[position_in_array]=pafr=flowmaster_instance.pafr();
		torque=motor_instance.PseudoTorque();
		printf("position: %d  pseudotorque: %d  massflow: %f  pafr: %f\n",cal_motor_position_table[position_in_array],torque,flowmaster_instance.massflow(),cal_pafr_table[position_in_array]);
		if (pafr<0.5)
			current_amount_to_add=amount_to_add/5;
		if (pafr<0.1)
			current_amount_to_add=amount_to_add/50;
	}
	if(!(pafr>0.001))
		printf("caliberation stopped because low flow was reached\n");
	if(!(motor_instance.PseudoTorque()<maxtorque))
		printf("caliberation stopped due to excess torque\n");
	if(!(position_in_array<=(max_table_size-1)))
		printf("caliberation stopped due to running out of space in this program's array\n");
	if(!(motor_instance.Position()<too_far))
		printf("caliberation stopped due to motor going too far\n");
	most_recently_written_caliberation_data_num++;
	calibration_data_num=most_recently_written_caliberation_data_num;
	write_settings();

	char num[10];
	char name[30];
	sprintf(num,"%d",most_recently_written_caliberation_data_num);
	strcpy(name,"calibration data");
	strcat(name,num);
	strcat(name,".txt");
	FILE * pFile;
	pFile = fopen (name,"w+");
	if( pFile==NULL)
		printf("error: could not access calibration file");
	else
	{
		for(position_in_array=0; position_in_array<=(max_table_size-1); position_in_array++)   // the data terminates with 987654321 0, the motor will never go that far
		{
			fprintf (pFile, "%d\t%f\n", cal_motor_position_table[position_in_array], cal_pafr_table[position_in_array]);
			//printf ("I have recorded: %d and %f \n",cal_motor_position_table[position_in_array],cal_pafr_table[position_in_array]);
		}
	}
	flowmaster_instance.lockdown();
	motor_instance.GoTo(maxspeed,maxacc,npoint); // npoint= near point to origin
	fclose (pFile);
	read_calibration_data();

	return 0;
}
int create_back_off_caliberation_data()
{
	int cal_motor_position_table[max_table_size];
	float cal_pafr_table[max_table_size];
	float cal_back_off_pafr_table[max_table_size];
	int position_in_array=0;
	int torque=0;
	int motorposition=0;
	int current_amount_to_add=amount_to_add;
	flowmaster_instance.reset();
	Sleep(2*time_for_stabilization);
	cal_motor_position_table[0]=-1000000000;
	cal_pafr_table[0]=700000;					 // this and the above line just ensure that if flows are requested that are above the caliberration curve's highest point then the system simply moves forwards slowly
	cal_back_off_pafr_table[0]=700000;
	for(position_in_array=1; position_in_array<=max_table_size-1;position_in_array++)
	{
		cal_motor_position_table[position_in_array]=1000000; // this is harmless and it is the end of data marker
		cal_pafr_table[position_in_array]=0;  // these two lines initialise my caliberation array to harmless values
		cal_back_off_pafr_table[position_in_array]=0;
	}
	motor_instance.GoTo(maxspeed,maxacc,spoint);
	float pafr=flowmaster_instance.pafr();
	for(position_in_array=1;(pafr>0.001)
												&&(torque<maxtorque)
												&&(position_in_array<=(max_table_size-1))
												&&(motorposition<too_far);position_in_array++)   // this while sets the point of cut off after which caliberation ends
	{
		motor_instance.GoTo(maxspeed,maxacc,(motor_instance.Position()+current_amount_to_add));

		cal_motor_position_table[position_in_array]=motorposition=motor_instance.Position();
		cal_pafr_table[position_in_array]=pafr=flowmaster_instance.pafr();

		motor_instance.GoTo(maxspeed,maxacc,(motor_instance.Position()- dist_to_back_off)); // this backs the motor off so I can get a reading without bounce
		Sleep(time_for_stabilization);
		cal_back_off_pafr_table[position_in_array]=flowmaster_instance.pafr();
		motor_instance.GoTo(maxspeed,maxacc,(motor_instance.Position()+dist_to_back_off)); // this backs the motor off so I can get a reading without bounce

		printf("position: %d  massflow: %f  pafr: %f backed_off_pafr: %f\n",cal_motor_position_table[position_in_array],flowmaster_instance.massflow(),cal_pafr_table[position_in_array],cal_back_off_pafr_table[position_in_array]);
		if (pafr<0.5)
			current_amount_to_add=amount_to_add/5;
		if (pafr<0.1)
			current_amount_to_add=amount_to_add/50;
	}
	if(!(pafr>0.001))
		printf("caliberation stopped because low flow was reached\n");
	if(!(motor_instance.PseudoTorque()<maxtorque))
		printf("caliberation stopped due to excess torque\n");
	if(!(position_in_array<=(max_table_size-1)))
		printf("caliberation stopped due to running out of space in this program's array\n");
	if(!(motor_instance.Position()<too_far))
		printf("caliberation stopped due to motor going too far\n");

	char name[30];
	strcpy(name,"data with backing off (not used for calibration)");
	strcat(name,".txt");
	FILE * pFile;
	pFile = fopen (name,"w+");
	if( pFile==NULL)
		printf("error: could not access calibration file");
	else
	{
		for(position_in_array=0; position_in_array<=(max_table_size-1); position_in_array++)   // the data terminates with 987654321 0, the motor will never go that far
		{
			fprintf (pFile, "%d\t%f\t%f\n", cal_motor_position_table[position_in_array], cal_pafr_table[position_in_array],cal_back_off_pafr_table[position_in_array]);
			//printf ("I have recorded: %d and %f \n",cal_motor_position_table[position_in_array],cal_pafr_table[position_in_array]);
		}
	}
	flowmaster_instance.lockdown();
	motor_instance.GoTo(maxspeed,maxacc,npoint); // npoint= near point to origin
	fclose (pFile);
	read_calibration_data();
	return 0;
}
int position_assosiated_with_pafr(float pafr)// this gets the position on the caliberation curve that is assosiated with a pressure adjusted flow rate
{
	int position_in_array=0;
	while(pafr<=pafr_table[position_in_array])//
		position_in_array++;
	//now pafr_table[position_in_array-1]>pafr>pafr_table[position_in_array]
	//now I interpolate
	float fraction=(pafr_table[position_in_array-1]-pafr)/(pafr_table[position_in_array-1]-pafr_table[position_in_array]);
	int rvalue=(fraction*motor_position_table[position_in_array]+(1-fraction)*motor_position_table[position_in_array-1]);
	return (rvalue);
}
float perform(float local_tpafr) // target pressure ajusted flow rate
{
	if(algonum==1)
		return(algorithm_without_bounce_protection(local_tpafr));
	if(algonum==2)
		return (algorithm_with_bounce_protection(local_tpafr));
	if(algonum==3)
		return(extra_accurate_algorithm(local_tpafr));
	else
	{
		printf("algorithm specified not recognised");
		return(-2);
	}
}
float algorithm_without_bounce_protection(float local_tpafr)
{
	int iterations=0; // this just checks for the thing looping forever
	int torque=0;
	int flag=0;
	int motorposition=motor_instance.Position();
	motor_instance.SetOrigin();
	motor_instance.GoTo(maxspeed,maxacc,spoint);
	flowmaster_instance.reset();
	Sleep(2*time_for_stabilization);
	float cpafr=flowmaster_instance.pafr(); // current pressure ajusted flow rate
	int tep=position_assosiated_with_pafr(local_tpafr);// target equivelent position, current equivelent position
	int cep=position_assosiated_with_pafr(cpafr);
	if (local_tpafr<0.0001)
	{
		printf("flow too small to set\n");
		flowmaster_instance.lockdown();
		return (-2);
	}
	if (cpafr<=(local_tpafr*(1+tferror)))
	{
		flowmaster_instance.lockdown();
		return (cpafr);
	}
	while((iterations<maxiterations)
				&&(torque<maxtorque)
				&&(motorposition<too_far))
		{
			cpafr=flowmaster_instance.pafr();
			printf("iterations: %d position: %d pafr: %f\n",iterations,motorposition,cpafr);
			if((flag>=1)||(cpafr<=local_tpafr))// this breaks out of the loop if the flag is high or if it has gone below it's target flow
			{
				motor_instance.GoTo(maxspeed,maxacc,npoint);
				Sleep(time_for_stabilization); // I sleep for a second time because the flow may change as the motor backs off
				cpafr=flowmaster_instance.pafr();
				flowmaster_instance.lockdown();
				return (cpafr); // this returns the achieved flowrate, the calling function must decide if this flow is acceptable
			}
			if(cpafr<=(local_tpafr*(1+tferror))) // this breaks out of the loop if the flow is low enough to warrant it
				flag++;
			//torque=motor_instance.pseudotorque();     // this line is not useful
			cep=position_assosiated_with_pafr(cpafr);
			motorposition=motor_instance.Position();
			motor_instance.GoTo(maxspeed,maxacc,(motorposition+fract2move*(tep-cep)));
			Sleep(time_for_stabilization);
			iterations++;
		}
	if(!(iterations<maxiterations))
		printf("error: too many iterations,\n this program can handle an infinite number of iterations but the fact that there have been %d iterations without the desired flow rate being reached indicates something else is wrong\n",maxiterations);
	if(!(motor_instance.Position()<too_far))
		printf("error: program stopped motor because it went too far\n");
	if(!(motor_instance.PseudoTorque()<maxtorque))
		printf("caliberation stopped due to excess torque\n");
	flowmaster_instance.lockdown();
	return(-2.0);
}
float algorithm_without_bounce_protection_for_internal_use(float local_tpafr)
{
	int iterations=0; // this just checks for the thing looping forever
	int torque=0;
	int flag=0;
	int motorposition=motor_instance.Position();
	motor_instance.SetOrigin();
	motor_instance.GoTo(maxspeed,maxacc,spoint);
	flowmaster_instance.reset();
	Sleep(2*time_for_stabilization);
	float cpafr=flowmaster_instance.pafr(); // current pressure ajusted flow rate
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
			cpafr=flowmaster_instance.pafr();
			printf("iterations: %d position: %d pafr: %f\n",iterations,motorposition,cpafr);
			if((flag>=1)||(cpafr<=local_tpafr))// this breaks out of the loop if the flag is high or if it has gone below it's target flow
			{
				//motor_instance.go_to(maxspeed,maxacc,npoint); // not desired for internal use
				Sleep(time_for_stabilization); // I sleep for a second time because the flow may change as the motor backs off
				cpafr=flowmaster_instance.pafr();
				//flowmaster_instance.lockdown(); // not desired for internal use
				return (cpafr); // this returns the achieved flowrate, the calling function must decide if this flow is acceptable
			}
			if(cpafr<=(local_tpafr*(1+tferror))) // this breaks out of the loop if the flow is low enough to warrant it
				flag++;
			//torque=motor_instance.pseudotorque();     // this line is not useful
			cep=position_assosiated_with_pafr(cpafr);
			motorposition=motor_instance.Position();
			motor_instance.GoTo(maxspeed,maxacc,(motorposition+fract2move*(tep-cep)));
			Sleep(time_for_stabilization);
			iterations++;
		}
	if(!(iterations<maxiterations))
		printf("error: too many iterations,\n this program can handle an infinite number of iterations but the fact that there have been %d iterations without the desired flow rate being reached indicates something else is wrong\n",maxiterations);
	if(!(motor_instance.Position()<too_far))
		printf("error: program stopped motor because it went too far\n");
	if(!(motor_instance.PseudoTorque()<maxtorque))
		printf("caliberation stopped due to excess torque\n");
	return(-2.0);
}
float algorithm_with_bounce_protection(float local_tpafr)
{
	float rpafr; // the returned pressure adjusted flow rate
	rpafr=algorithm_without_bounce_protection_for_internal_use(local_tpafr-max_bounce_expected);// this tells the simpler algorithum to get close to the desired answer
	if(rpafr==-2) // if "algorithm_without_bounce_protection" ends badly then this algorithum should do nothing but pass the bad result to the caller
	{
		flowmaster_instance.lockdown(); // this closes the valves to make it safe for the user to open the jig
		return(rpafr);
	}
	if (rpafr<=(local_tpafr*(1+tferror))) // if the flow is lower then the upper bound allowed then nothing more should be done
	{
		flowmaster_instance.lockdown(); // this closes the valves to make it safe for the user to open the jig
		return (rpafr);
	}
	printf("second stage started\n");
	float cpafr=0; // current pressure ajusted flow rate
	int tep=0, cep=0; // target equivelent position, current equivelent position
	int iterations=0; // this just checks for the thing looping forever
	int torque=0;
	motor_instance.GoTo(maxspeed,maxacc,spoint);
	tep=position_assosiated_with_pafr(local_tpafr);
	while((iterations<maxiterations)
				&&(torque<maxtorque)
				&&(motor_instance.Position()<too_far))
		{
			motor_instance.GoTo(maxspeed,maxacc,(motor_instance.Position()-dist_to_back_off)); // this backs the motor off so I can get a reading without bounce
			cpafr=flowmaster_instance.pafr();
			printf("iterations: %d position: %d pafr: %f\n",iterations,motor_instance.Position(),cpafr);
			if(cpafr<=(local_tpafr*(1+tferror))) // this breaks out of the loop if the flow is low enough to warrant it
			{
				motor_instance.GoTo(maxspeed,maxacc,npoint);
				flowmaster_instance.lockdown(); // this closes the valves to make it safe for the user to open the jig
				return (cpafr); // this returns the achieved flowrate, the calling function must decide if this flow is acceptable
			}
			else // this does another iteration of the setting process
			{
				cep=position_assosiated_with_pafr(cpafr);
				//torque=motor_instance.pseudotorque();     // this line is not useful as the torque measurement is broken
				motor_instance.GoTo(maxspeed,maxacc,(motor_instance.Position()+dist_to_back_off));// this moves the motor back to where it was
				motor_instance.GoTo(maxspeed/3,maxacc,(motor_instance.Position()+fract2move*(tep-cep))); // I do this as two seperate moves so that the motor will be going slower for the fine movement at the end
				Sleep(time_for_stabilization);
				iterations++;
			}
		}
	if(!(iterations<maxiterations))
		printf("error: too many iterations,\n this program can handle an infinite number of iterations but the fact that there have been %d iterations without the desired flow rate being reached indicates something else is wrong\n",maxiterations);
	if(!(motor_instance.Position()<too_far))
		printf("error: program stopped motor because it went too far\n");
	if(!(motor_instance.PseudoTorque()<maxtorque))
		printf("caliberation stopped due to excess torque\n");
	flowmaster_instance.lockdown(); // this closes the valves to make it safe for the user to open the jig
	return(-2.0);
}
float extra_accurate_algorithm(float local_tpafr)
{
	return(algorithm_with_bounce_protection(local_tpafr));// this is just a place holder until i come up with something better.
}
int handle_result(float rpafr, float local_tpafr)
{
	if (rpafr==-2.0)
	{
		// this the code for do nothing, things have been handled in the "perform" function and I decided not to put the code here
    // this should also be unreachable through experemental readings unless the flowmeters are backwards
		return 0;
	}
	if((((1-tferror)*local_tpafr)<=rpafr)&&(rpafr<=((1+tferror)*local_tpafr)))
	{
		printf("success: pressure adjusted flow rate=%f\n",rpafr);
		return 0;
	}
	else if(!(((1-tferror)*local_tpafr)<=rpafr))
	{
		printf("failure: set flow too small, pressure adjusted flow rate=%f SLPM per bar\n",rpafr);
		return 0;
	}
	else if (!(rpafr<=((1+tferror)*local_tpafr)))
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
int list()
{
	printf("command  purpose\n\n");
	printf("s        sets a flow\n");
	printf("e        exits program\n");
	printf("l        lists possible commands\n");
//	printf("m        shows built in manual\n");
	printf("v        view settings\n");
	printf("c        creates and uses a new calibration data file\n");
	printf("cbo      creates back off calibration data file\n");
	//printf("f        fixes motor free state (motor being unresponsive)"); // found to not work properly
	printf("number   sets a flow of 'number' SLPM per bar\n\n");

	return 0;
}
int read_settings()
{
	FILE * pFile;
	pFile = fopen ("settings.txt","r");
	if(pFile==NULL)
		printf("could not open settings file");
	fscanf (pFile, "%d safe point \n",&spoint);
	fscanf (pFile, "%d near point \n",&npoint);
	fscanf (pFile, "%d motor gone too far point \n",&too_far);
	fscanf (pFile, "%d motor speed \n",&maxspeed);
	fscanf (pFile, "%d motor acceleration \n",&maxacc);
	fscanf (pFile, "%d maximum iterations in setting (should be large)\n",&maxiterations);
	fscanf (pFile, "%f gas factor (not currently implemented)\n",&gas_factor);
	fscanf (pFile, "%f target pressure (not currently implemented)\n",&tpressure);
	fscanf (pFile, "%f minimum pressure (not currently implemented)\n",&minpressure);
	fscanf (pFile, "%f maximum pressure (not currently implemented)\n",&maxpressure);
	fscanf (pFile, "%f target pressure adjusted flow rate \n",&tpafr);
	fscanf (pFile, "%f target fractional error \n",&tferror);
	fscanf (pFile, "%d number of the calibration file to be used \n",&calibration_data_num);
	fscanf (pFile, "%d number of the most recently written calibration file \n",&most_recently_written_caliberation_data_num);
	fscanf (pFile, "%f time for stabilization\n",&time_for_stabilization);
	fscanf (pFile, "%f fraction to final position to move on each iteration \n",&fract2move);
	fscanf (pFile, "%f maximum torque\n",&maxtorque);
	fscanf (pFile, "%d number of the algorithm used for setting flows\n",&algonum);
	fscanf (pFile, "%d amount to move on each iteration of calibration data generation\n",&amount_to_add);
	fscanf (pFile, "%d debugging (set to 1 for debugging and 0 for normal use)\n",&debugging);
	fscanf (pFile, "%f maximum bounce expected measured in SLPM per Bar\n",&max_bounce_expected);
	fscanf (pFile, "%d distance to back off, this is used when the motor backs off to get a measurement includeing the bounce effect\n",&dist_to_back_off);
	fscanf (pFile, "%c release valve channel\n\n",&release_valve_channel);

	fscanf (pFile, "%d motor port (the port number should be set to -1 to signal when the device does not exist)\n",&motor1port);
	fscanf (pFile, "%d weeder io port (the port number should be set to -1 to signal when the device does not exist) \n",&weederio1port);
	fscanf (pFile, "%d pressure transducer port (the port number should be set to -1 to signal when the device does not exist)\n",&pressure_transducer1port);
	fscanf (pFile, "%d flowmeter 1 port (the port number should be set to -1 to signal when the device does not exist)\n",&flowmeter1port);
	fscanf (pFile, "%c weeder io channel for flowmeter 1(should be a capital letter)", &flowmeter1channel);
	fscanf (pFile, "%f flowmeter 1 maximum flow (slpm)\n",&flowmeter1maxflow);
	fscanf (pFile, "%d flowmeter 1 units correction (1 for litres per min 1000 for ml)\n",&flowmeter1units_correction);
	fscanf (pFile, "%d flowmeter 2 port (the port number should be set to -1 to signal when the device does not exist)\n",&flowmeter2port);
	fscanf (pFile, "%c weeder io channel for flowmeter 2(should be a capital letter)", &flowmeter2channel);
	fscanf (pFile, "%f flowmeter 2 maximum flow (slpm)\n",&flowmeter2maxflow);
	fscanf (pFile, "%d flowmeter 2 units correction (1 for litres per min 1000 for ml)\n",&flowmeter2units_correction);
	fscanf (pFile, "%d flowmeter 3 port (the port number should be set to -1 to signal when the device does not exist)\n",&flowmeter3port);
	fscanf (pFile, "%c weeder io channel for flowmeter 3(should be a capital letter)", &flowmeter3channel);
	fscanf (pFile, "%f flowmeter 3 maximum flow (slpm) \n",&flowmeter3maxflow);
	fscanf (pFile, "%d flowmeter 3 units correction (1 for litres per min 1000 for ml)\n",&flowmeter3units_correction);
	fscanf (pFile, "%d flowmeter 4 port (the port number should be set to -1 to signal when the device does not exist)\n",&flowmeter4port);
	fscanf (pFile, "%c weeder io channel for flowmeter 4(should be a capital letter)", &flowmeter4channel);
	fscanf (pFile, "%f flowmeter 4 maximum flow (slpm)\n",&flowmeter4maxflow);
	fscanf (pFile, "%d flowmeter 4 units correction (1 for litres per min 1000 for ml)\n",&flowmeter4units_correction);
	fscanf (pFile, "%d flowmeter 5 port (the port number should be set to -1 to signal when the device does not exist)\n",&flowmeter5port);
	fscanf (pFile, "%c weeder io channel for flowmeter 5(should be a capital letter)", &flowmeter5channel);
	fscanf (pFile, "%f flowmeter 5 maximum flow (slpm)\n",&flowmeter5maxflow);
	fscanf (pFile, "%d flowmeter 5 units correction (1 for litres per min 1000 for ml) \n\n",&flowmeter5units_correction);

	fscanf (pFile, "%d don't change me, I test if settings are read correctly\n\n", &broken_settings_test);
	if (broken_settings_test!=1234567890)
	{
		printf("did not read settings file correctly\n");
		system("PAUSE");
	}
	fclose (pFile);
	return 0;
}
int test()
{
	int i=0;
	while (i<3)// this can be tweaked for debugging
	{
		//printf("pressure: %f bar  ",flowmaster_instance.pressure());
		//printf("massflow: %f SLPM  ",flowmaster_instance.massflow());
		//printf("pafr: %f SPLM/bar\n",flowmaster_instance.pafr());
		i++;
	}
	//below code is not needed for testing any more
	i=0;
	while (i<3)
	{
	//	motor_instance.go_to(maxspeed,maxacc,spoint/1.5);
	//	motor_instance.go_to(maxspeed,maxacc,npoint);
		i++;
	}
	//printf("motor torque: %d\n",motor_instance.pseudotorque());
	flowmaster_instance.test();
	printf("tests completed\n");
	return(0);
}
int view_settings()
{
	printf ("most important current settings are:\n");
	printf ("safe point: %d\n",spoint);
	printf ("calibration data file number: %d\n",calibration_data_num);
	printf ("maxiterations: %d\n",maxiterations);
	printf ("target pressure adjusted flow rate: %f\n",tpafr);
	printf ("target fractional error: %f\n",tferror);
	printf ("time for stabilization of flow rate: %f\n",time_for_stabilization);
	printf ("fraction of distance to target to move: %f\n",fract2move);
	printf ("to see all the settings look in settings file\n\n");
	return 0;
}
int write_settings()
{
	FILE * pFile;
	pFile = fopen ("settings.txt","w+");
	fprintf (pFile, "%d \t\tsafe point \n",spoint);
	fprintf (pFile, "%d \t\tnear point \n",npoint);
	fprintf (pFile, "%d \tmotor gone too far point \n",too_far);
	fprintf (pFile, "%d \tmotor speed \n",maxspeed);
	fprintf (pFile, "%d \tmotor acceleration \n",maxacc);
	fprintf (pFile, "%d \t\tmaximum iterations in setting (should be large)\n",maxiterations);
	fprintf (pFile, "%f \tgas factor (not currently implemented)\n",gas_factor);
	fprintf (pFile, "%f \ttarget pressure (not currently implemented)\n",tpressure);
	fprintf (pFile, "%f \tminimum pressure (not currently implemented)\n",minpressure);
	fprintf (pFile, "%f \tmaximum pressure (not currently implemented)\n",maxpressure);
	fprintf (pFile, "%f \ttarget pressure adjusted flow rate \n",tpafr);
	fprintf (pFile, "%f \ttarget fractional error \n",tferror);
	fprintf (pFile, "%d \t\tnumber of the calibration file to be used \n",calibration_data_num);
	fprintf (pFile, "%d \t\tnumber of the most recently written calibration file \n",most_recently_written_caliberation_data_num);
	fprintf (pFile, "%f \ttime for stabilization\n",time_for_stabilization);
	fprintf (pFile, "%f \tfraction to final position to move on each iteration \n",fract2move);
	fprintf (pFile, "%f \tmaximum torque\n",maxtorque);
	fprintf (pFile, "%d \t\tnumber of the algorithm used for setting flows\n",algonum);
	fprintf (pFile, "%d \t\tamount to move on each iteration of calibration data generation\n",amount_to_add);
	fprintf (pFile, "%d \t\tdebugging (set to 1 for debugging and 0 for normal use)\n",debugging);
	fprintf (pFile, "%f \t\tmaximum bounce expected measured in SLPM per Bar\n",max_bounce_expected);
	fprintf (pFile, "%d \t\tdistance to back off, this is used when the motor backs off to get a measurement includeing the bounce effect\n",dist_to_back_off);
	fprintf (pFile, "%c \t\trelease valve channel\n\n",release_valve_channel);

	fprintf (pFile, "%d \t\tmotor port (the port number should be set to -1 to signal when the device does not exist)\n",motor1port);
	fprintf (pFile, "%d \t\tweeder io port (the port number should be set to -1 to signal when the device does not exist) \n",weederio1port);
	fprintf (pFile, "%d \t\tpressure transducer port (the port number should be set to -1 to signal when the device does not exist)\n\n",pressure_transducer1port);
	fprintf (pFile, "%d \t\tflowmeter 1 port (the port number should be set to -1 to signal when the device does not exist)\n",flowmeter1port);
	fprintf	(pFile, "%c \t\tweeder io channel for flowmeter 1(should be a capital letter)\n", flowmeter1channel);
	fprintf (pFile, "%f \t\tflowmeter 1 maximum flow (slpm)\n",flowmeter1maxflow);
	fprintf (pFile, "%d \t\tflowmeter 1 units correction (1 for litres per min 1000 for ml)\n\n",flowmeter1units_correction);
	fprintf (pFile, "%d \t\tflowmeter 2 port (the port number should be set to -1 to signal when the device does not exist)\n",flowmeter2port);
	fprintf	(pFile, "%c \t\tweeder io channel for flowmeter 2(should be a capital letter)\n", flowmeter2channel);
	fprintf (pFile, "%f \t\tflowmeter 2 maximum flow (slpm)\n",flowmeter2maxflow);
	fprintf (pFile, "%d \t\tflowmeter 2 units correction (1 for litres per min 1000 for ml)\n\n",flowmeter2units_correction);
	fprintf (pFile, "%d \t\tflowmeter 3 port (the port number should be set to -1 to signal when the device does not exist)\n",flowmeter3port);
	fprintf	(pFile, "%c \t\tweeder io channel for flowmeter 3(should be a capital letter)\n", flowmeter3channel);
	fprintf (pFile, "%f \t\tflowmeter 3 maximum flow (slpm) \n",flowmeter3maxflow);
	fprintf (pFile, "%d \t\tflowmeter 3 units correction (1 for litres per min 1000 for ml)\n\n",flowmeter3units_correction);
	fprintf (pFile, "%d \t\tflowmeter 4 port (the port number should be set to -1 to signal when the device does not exist)\n",flowmeter4port);
	fprintf	(pFile, "%c \t\tweeder io channel for flowmeter 4(should be a capital letter)\n", flowmeter4channel);
	fprintf (pFile, "%f \t\tflowmeter 4 maximum flow (slpm)\n",flowmeter4maxflow);
	fprintf (pFile, "%d \t\tflowmeter 4 units correction (1 for litres per min 1000 for ml)\n\n",flowmeter4units_correction);
	fprintf (pFile, "%d \t\tflowmeter 5 port (the port number should be set to -1 to signal when the device does not exist)\n",flowmeter5port);
	fprintf	(pFile, "%c \t\tweeder io channel for flowmeter 5(should be a capital letter)\n", flowmeter5channel);
	fprintf (pFile, "%f \t\tflowmeter 5 maximum flow (slpm)\n",flowmeter5maxflow);
	fprintf (pFile, "%d \t\tflowmeter 5 units correction (1 for litres per min 1000 for ml) \n\n",flowmeter5units_correction);
	fprintf (pFile, "%d \tdon't change me, I test if settings are read correctly\n\n", 1234567890);

	fprintf (pFile,"******* end of data ******* \n\n the numbers in this file can be modified by the end user, you probably wish to back it up first though. \nAlso you can change the text after 'end of data' how ever you wish and it will be ignored by the program but\n if you change any of the text before the end of data marker the program will break");
	fclose (pFile);
	return 0;
}
/*
int manual()
{
	printf("put user manual here\n");
	return 0;
}
*/
