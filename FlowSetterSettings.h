#ifndef FlowSetterSettingsH
#define FlowSetterSettingsH
#include <stdio.h>
#define MAXFLOWWMETERS 5 // this is the maximum number of flowmeters that this program can have
class FlowSetterSettings
{
	// the purpose of this class is to store and manage all settings that must be stored in the settings file
	// I do this to aid encapsulation
	// rule for use of this settings file manager: whenever a setting is changed by the user of this class the write_file() method must be called as well.
	// I could have enforced this by forceing the modification of variables to happen through method calls but I decided against this due to the extra code required.
	public: // functions
		int 	ReadFile();
		int 	WriteFile();
	public: // the settings themselves
		int 	SafePoint; // this is the safe point near the jig
		int 	NearPoint; // this is an alowed near point to the origin where it does not touch the stopper
		int 	TooFar; // this is to test if the motor has gone too far and there is an error
		int 	MaxSpeed; // maximum speed
		int 	MaxAcc; // maximum acceleration
		int 	MaxIterations;  // checks for failure due to infinite iterations
		float 	GasFactor; //the flows will be set for air but used for othe gasses
		float 	tPressure;   // this is the target pressure, not used yet
		float 	MinPressure; //not used yet
		float 	MaxPressure; //not used yet
		float 	tpafr;   // this is the target pressure adjusted flow rate
		float 	tfError; // this is the target fractional error eg 0.05 means +- 5% is ok
		int 	CalibrationDataNum; //this enables multiple caliberation ccurves to be stored
		int 	MostRecentlyWrittenCaliberationDataNum;
		float 	TimeForStabilization;//measured in miliseconds, change for good results
		float 	FractToMove; // the fraction of the distance to the estimated destination that the motor should move each time
		float 	MaxTorque; // I can't measure torque only related quantitys so beware
		int 	AlgoNum; // this describes which algorithm is used for setting flows
		int 	AmountToAdd; // this describes how far the motor moves during each iteration of caliberation data generation
		float 	MaxBounceExpected; // the units are SLPM per Bar
		int 	DistToBackOff;// this is used when the motor backs off to get a measurement includeing the bounce effect
		char 	ReleaseValveChannel;
		int   	MotorPort; // the port number being -1 signals that the device does not exist
		int   	WeederioPort; // the port number being -1 signals that the device does not exist
		int   	PressureTransducerPort; // the port number being -1 signals that the device does not exist

		int   	FlowMeterPort		[MAXFLOWWMETERS];// the port number being -1 signals that the device does not exist
		char  	FlowMeterChannel	[MAXFLOWWMETERS];
		float 	FlowMeterMaxFlow	[MAXFLOWWMETERS]; //this is in standard litres per min
		int   	FlowMeterUnitsCorrection[MAXFLOWWMETERS]; // the flow data is divided by this value, should be 1 for litres per min and 1000 for ml
		int   	BrokenSettingsTest; // this is overwriten with a known value to check that the read funtion got to the end of the file ok

	private:
		int 	UnguardedReadFile();
		int 	UnguardedWriteFile();
	private:
		FILE*	FilePointer;
		static const int MaxSettings = 10;
		static const int MaxCharsInSettingDescriptor = 500;
		char 	SettingsDescriptors[MaxSettings][MaxCharsInSettingDescriptor];
};
#endif
