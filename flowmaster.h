#ifndef FlowmasterH
#define FlowmasterH
#include "AlicatFlowmeter.h"
#include "Weederio.h"
class FlowMaster
{
	public:
		FlowMaster(void);
		void Setup		   (int flowmeter1port, char flowmeter1channel,float flowmeter1maxflow,int flowmeter1units_correction,
							int flowmeter2port, char flowmeter2channel,float flowmeter2maxflow,int flowmeter2units_correction,
							int flowmeter3port, char flowmeter3channel,float flowmeter3maxflow,int flowmeter3units_correction,
							int flowmeter4port, char flowmeter4channel,float flowmeter4maxflow,int flowmeter4units_correction,
							int flowmeter5port, char flowmeter5channel,float flowmeter5maxflow,int flowmeter5units_correction,
							int weederio1port,  char pressure_transducer1port,float time_for_stabilization, char release_valve_channel);
		// setup should only be called on startup, it sets up this class
		int CreateFlowMeter(int PortNum,int FlowmeterNum);  // creates an instance of the flowmeter class
		int FindSizeOrder(void);                            // creates an ordering of the flowmeters sorted by maximum flow rate
		int Reset(void);    																	// gets system ready for flow setting (closes all valves exept what is needed  for the largest flowmeter)
		int ReconsiderValves(void); 													// this checks if it is apropriate to move to a smaller valve set and if aproprate makes the change
		int LockDown(void); 																	// this closes all valves and renders the system useless but safe
		float pafr(void);                                     // this gets the current pressure adjusted flow rate in SLPM per bar
		float Pressure(void);                                 // this gets the current pressure
		float MassFlow(void);                                 // this gets the current massflow measured in SLPM
		float QuickMassFlow(void);														// this is like the above but it does not check if the valves are apropriate
		~FlowMaster(void);                                    // this does nothing, my flowmeter classes should probably be deleted here
	private:
		Weederio WeederioInstance;
		float LocalTimeForStabilization;
		char LocalReleaseValveChannel; // this stores the channel for the releace valve
		AlicatFlowmeter* FlowMeter[10]; // an array of pointers to flowmeter class instances
		int NumFlowMeters; // the number of avalible flowmeters
		int SizeOrder[10]; // this should contain the avalible flowmeters in size order.
		int fmInUse; // flow meter in use, 0 is a special value to say "none"
		bool fmPresent[10]; // to say which flowmeters are present, starts with position 1 for flowmeter 1
		bool io1Present;    // to say if the io board is present
		bool pt1Present;    // to say if the pressure transducer is present
		char FlowMeterChannel[10]; // a list of what valve chanel is assosiated with each flowmeter, note the first element should never be used, I start from number 1
		float FlowMeterMaxFlow[10]; // a list of the flowmeter's maximum mass flows, note the first element should never be used, I start from number 1
		float FlowMeterUnitsCorrection[10]; // a list of the units corrections needed to convert the measurements to SLPM, note the first element should never be used, I start from number 1
};
#endif
