#ifndef FlowmasterH
#define FlowmasterH
#include "Weederio.h"
class AlicatFlowmeter;
#include "FlowSetterSettings.h"

class FlowMaster
{
	public:
		FlowMaster(void);
		void Setup(FlowSetterSettings* SettingsPointer);// setup should only be called on startup, it sets up this class

		bool CreateFlowMeter(int PortNum,int FlowmeterNum);  // creates an instance of the flowmeter class
		void FindSizeOrder(void);  // creates an ordering of the flowmeters sorted by maximum flow rate
		int Reset(void);    		// gets system ready for flow setting (closes all valves exept what is needed  for the largest flowmeter)
		int LockDown(void); 		// this closes all valves and renders the system useless but safe
		float pafr(void);           // this gets the current pressure adjusted flow rate in SLPM per bar
		float Pressure(void);       // this gets the current pressure
		float MassFlow(void);       // this gets the current massflow measured in SLPM
		~FlowMaster(void);          // this does nothing, my flowmeter classes should probably be deleted here
	private:
		int ReconsiderValves(void); // this checks if it is apropriate to move to a smaller valve set and if aproprate makes the change
		float QuickMassFlow(void);	// this is like the above but it does not check if the valves are apropriate
		int BiggestFlowMeter();		// this function returns the position in the array that corresponds to the biggest flowmeter
	private:
		Weederio WeederioInstance;
		FlowSetterSettings* SettingsPointer;
		AlicatFlowmeter* FlowMeter[MAXFLOWWMETERS];// an array of pointers to flowmeter class instances
		int NumFlowMeters; 				// the number of avalible flowmeters
		int SizeOrder[MAXFLOWWMETERS]; 	// this should contain the avalible flowmeters in size order.
		int fmInUse; 					// flow meter in use, 0 is a special value to say "none"
		bool fmPresent[MAXFLOWWMETERS]; // to say which flowmeters are present
		bool ioPresent;    				// to say if the io board is present
};
#endif
