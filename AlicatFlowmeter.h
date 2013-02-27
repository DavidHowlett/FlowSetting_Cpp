//---------------------------------------------------------------------------

#ifndef AlicatFlowmeterH
#define AlicatFlowmeterH
#include <Classes.hpp>
#include "RS232Primitive.h"

const MeanReadingsCount=10;     //Number of readings to get a mean;

//******************************************************************************
class alicat_flowmeter : public RS232Primitive
//******************************************************************************
{
	typedef struct tagTHREADNAME_INFO
	{
		DWORD dwType;     // must be 0x1000
		LPCSTR szName;    // pointer to name (in user addr space)
		DWORD dwThreadID; // thread ID (-1=caller thread)
		DWORD dwFlags;    // reserved for future use, must be zero
	} THREADNAME_INFO;
private:
	void ReadAndProcess();                                   //Read the flowmeter data and update it
public:
	alicat_flowmeter(bool CreateSuspended);                         //Constructor
//	 __fastcall ~FlowMeter(void);                          //Destructor
	void __fastcall Execute();                               //Thread execution loop. Critical!!
	Cardinal DataAge();                                      //Age of data in clock ticks
	//Setting the monitor controls to view the data on-screen:
	void       SetPressureMonitor      (TEdit* Monitor    ); //Optionally use an Edit Box to monitor the output
	void       SetTemperatureMonitor   (TEdit* Monitor    ); //Optionally use an Edit Box to monitor the output
	void       SetVolumetricFlowMonitor(TEdit* Monitor    ); //Optionally use an Edit Box to monitor the output
	void       SetMassFlowMonitor      (TEdit* Monitor    ); //Optionally use an Edit Box to monitor the output
	void       SetGasTypeMonitor       (TEdit* Monitor    ); //Optionally use an Edit Box to monitor the output
private:
	//Administration
	boolean    TooOld();                                     //Is the data too old to show? eg over 0.05 seconds old
	Cardinal   TicksLastRead;                                //When was this data last read?
	void SetName();                                          //Set a name for this FlowMeter
	//Variables to hold the monitor controls, used to show the flow data:
	TEdit* FPressureMonitor;
	TEdit* FTemperatureMonitor;
	TEdit* FVolumetricFlowMonitor;
	TEdit* FMassFlowMonitor;
	TEdit* FGasTypeMonitor;
	//Variables to hold the flow properties
	float      FPressure       ;                             //The current pressure
	float      FTemperature    ;                             //The current temperature
	float      FVolumetricFlow ;                             //The current volumetric flow
	float      FMassFlow       ;                             //The current mass flow
	String     FGasType        ;                             //The current gas type (eg "Air"), String
	void __fastcall UpdateMonitors();                        //Update the monitor fields, if any
	int        LogCounter;                                   //This is used to update the log[LogCounter] data
	float      PressureLog      [MeanReadingsCount];      //Record of the pressures over time
	float      TemperatureLog   [MeanReadingsCount];      //Record of the temperatures over time
	float      VolumetricFlowLog[MeanReadingsCount];      //Record of the volumetric flows over time
	float      MassFlowLog      [MeanReadingsCount];      //Record of the mass flows over time
	//MeanGasType is not implemented in this release... ;-)
public:
	//Functions to access the read data:
	float      Pressure          ();                         //Return the current pressure
	float      Temperature       ();                         //Return the current temperature
	float      VolumetricFlow    ();                         //Return the current volumetric flow
	float      MassFlow          ();                         //Return the current mass flow
	String     GasType           ();                         //Return the current gas type (eg "Air")
	float      MeanPressure      ();                         //Return the mean Pressure
	float      MeanTemperature   ();                         //Return the mean Temperature
	float      MeanVolumetricFlow();                         //Return the mean volumetric flow
	float      MeanMassFlow      ();                         //Return the mean mass flow
};
#endif
