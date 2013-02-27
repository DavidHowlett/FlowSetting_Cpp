#include <stdio.h>
#include <iostream.h>
#include "Flowmaster.h"
flowmaster::flowmaster()
{
	fminuse=0;
	num_flowmeters=0;
	for(int i=0;i<10;i++)
		fmpresent[i]=false;
}
int flowmaster::setup(int flowmeter1port, char flowmeter1channel,float local_flowmeter1maxflow,int local_flowmeter1units_correction,
											int flowmeter2port, char flowmeter2channel,float local_flowmeter2maxflow,int local_flowmeter2units_correction,
											int flowmeter3port, char flowmeter3channel,float local_flowmeter3maxflow,int local_flowmeter3units_correction,
											int flowmeter4port, char flowmeter4channel,float local_flowmeter4maxflow,int local_flowmeter4units_correction,
											int flowmeter5port, char flowmeter5channel,float local_flowmeter5maxflow,int local_flowmeter5units_correction,
											int weederio1port,  char pressure_transducer1port,float time_for_stabilization, char release_valve_channel)
{
	local_time_for_stabilization=time_for_stabilization;
	local_release_valve_channel=release_valve_channel;
	// the idea here is to have the below variables freely avalible in this class and only set here
	flowmeterchannel[1]=flowmeter1channel;
	flowmetermaxflow[1]=local_flowmeter1maxflow;
	flowmeterunits_correction[1]=local_flowmeter1units_correction;
	flowmeterchannel[2]=flowmeter2channel;
	flowmetermaxflow[2]=local_flowmeter2maxflow;
	flowmeterunits_correction[2]=local_flowmeter2units_correction;
	flowmeterchannel[3]=flowmeter3channel;
	flowmetermaxflow[3]=local_flowmeter3maxflow;
	flowmeterunits_correction[3]=local_flowmeter3units_correction;
	flowmeterchannel[4]=flowmeter4channel;
	flowmetermaxflow[4]=local_flowmeter4maxflow;
	flowmeterunits_correction[4]=local_flowmeter4units_correction;
	flowmeterchannel[5]=flowmeter5channel;
	flowmetermaxflow[5]=local_flowmeter5maxflow;
	flowmeterunits_correction[5]=local_flowmeter5units_correction;
	if (flowmeter1port>-1)// for item not in use the port will be minus 1
		createflowmeter(flowmeter1port,1);
	if (flowmeter2port>-1)// for item not in use the port will be minus 1
		createflowmeter(flowmeter2port,2);
	if (flowmeter3port>-1)// for item not in use the port will be minus 1
		createflowmeter(flowmeter3port,3);
	if (flowmeter4port>-1)// for item not in use the port will be minus 1
		createflowmeter(flowmeter4port,4);
	if (flowmeter5port>-1)// for item not in use the port will be minus 1
		createflowmeter(flowmeter5port,5);

	if (weederio1port>-0.5)
	{
		weederio_instance.Setup(weederio1port);//COM port
    lockdown();
		io1present=1;
	}else{
		io1present=0;
	}
	if (pressure_transducer1port>-1)
	{
		// write code for presure transducer setup here
		pt1present=1;
	}else{
	pt1present=0;
	}
	find_size_order();
	Sleep(20); // this is to allow the flowmeter threads to get data so they be safely used
	return(0);
}
int flowmaster::createflowmeter(int port_num,int flowmeter_num)
{
	char name[10];
	fmpresent[flowmeter_num]=false; // this means the flowmeter is considered not to exist unless it is specificly stated to exist later
	sprintf(name,"COM%d",port_num);
	flowmeter[flowmeter_num] = new AlicatFlowmeter(True);
	flowmeter[flowmeter_num]->Setup(name,38400,'N',8,1);             //COM port, baud rate, parity, bytes per bit, stop bits
	flowmeter[flowmeter_num]->Open();                                   //Enable port access
	if (flowmeter[flowmeter_num]==NULL)
	{
		printf("flowmeter%d on port%d not created. Cannot continue\n",flowmeter_num,port_num);
	}else
	{
		flowmeter[flowmeter_num]->Resume();
		printf("flowmeter%d port opened\n",flowmeter_num);
		fmpresent[flowmeter_num]=true;
		num_flowmeters++;
	};
	return(0);
}
int flowmaster::find_size_order()// this tries to produce an array listing the flowmeters in size order
{
	// this is a sorting algorithum to find the order of the sizes of the flowmeters
	// it cycles through the list looking for the biggest one still unaccounted for (not in list of sizes) 
	// when it has a new biggest one it records the number of the flowmeter in the size_order array
	bool fm_unaccounted_for[10]; 
	for(int i=0;i<10;i++)
	{
		size_order[i]=0; // this initialises all to 0 (a none valid value)
		fm_unaccounted_for[i]=fmpresent[i];
	}
	int current_biggest_flowmeter;
	for(int i=0;i<10;i++)
	{
		current_biggest_flowmeter=0;
		for(int j=1;j<6;j++)
		{
			if((fm_unaccounted_for[j])&&((current_biggest_flowmeter==0)||(flowmetermaxflow[j]>=flowmetermaxflow[current_biggest_flowmeter])))
				current_biggest_flowmeter=j;	
		}
		size_order[i]=current_biggest_flowmeter; // note that this array starts from 0 unlike the others in this class
		fm_unaccounted_for[current_biggest_flowmeter]=false;
	}
	return 0;
}
int flowmaster::test()
{
	// this should contain code used for debugging, not to be used in final project
//	weederio_instance.open(flowmeterchannel[1]);
//	Sleep(100);
//	weederio_instance.open(flowmeterchannel[2]);
//	Sleep(100);
//	weederio_instance.open(flowmeterchannel[3]);
//	Sleep(100);
//	weederio_instance.open(flowmeterchannel[4]);
//	Sleep(100);
//	weederio_instance.open(flowmeterchannel[5]);
//	Sleep(100);
//	lockdown();
	//printf("%d\n",weederio_instance.request_status('C'));
	//printf("%d\n",weederio_instance.request_status('D'));
	return(0);
}
int flowmaster::reset()// this closes all valves except what is needed  for the largest flowmeter
{
	weederio_instance.Close(local_release_valve_channel);// this closes tha relece valve so the pressure can build up
	for(int i=1;i<6;i++)
	{
		if(fmpresent[i]&&(weederio_instance.RequestStatus(flowmeterchannel[i]))) // this should only close the valves that exist and are open to save time
			weederio_instance.Close(flowmeterchannel[i]);
	}
	weederio_instance.Open(flowmeterchannel[size_order[0]]);// this opens the valve for the biggest flowmeter
	fminuse=size_order[0];// this states that the flowmeter in use is the largest
	printf("current massflow: %f changed to %f SLPM flowmeter\n",quick_massflow(),flowmetermaxflow[size_order[0]]);
	return(0);
}
int flowmaster::reconsider_valves()// this function tries to find the right valve for the current flowrate
{
	if(fminuse!=0)
	{
		bool do_again=true;
		while(do_again)
		{
			do_again=false;
			int i=0;
			while((i<10)&&(fminuse!=size_order[i]))
				i++;
			if(i>=9)
				printf("strange error in 'reconsider_valves' function, call programmer for help\n");
			// at this point i is the value in the size_order list where fminuse can be found
			if(size_order[i+1]!=0)
			{
				float massflow=quick_massflow();
				if(massflow<=(0.95*flowmetermaxflow[size_order[i+1]]))
				{
					weederio_instance.Open(flowmeterchannel[size_order[i+1]]);
					weederio_instance.Close(flowmeterchannel[size_order[i]]);
					printf("current massflow: %f changed to %f SLPM flowmeter\n",massflow,flowmetermaxflow[size_order[i+1]]);
					fminuse=size_order[i+1];
					Sleep(local_time_for_stabilization*2);
					do_again=true;
				}
			}
		}
	}
	return(0);
}
int flowmaster::lockdown() // this closes all valves and renders the system useless but safe
{
	for(int i=1;i<6;i++)
	{
		if(fmpresent[i]) // this should only close the valves that exist to save time
			weederio_instance.Close(flowmeterchannel[i]);
	}
	//printf("current massflow: %f changed to no flowmeter\n",quick_massflow());
	fminuse=0; //this value means that no flowmeter is in use
	// it matters that the flowmeter valves are closed before the pressure releace valve is opened or you can blow your flowmeters
	weederio_instance.Open(local_release_valve_channel);// this opens the releace valve
	return(0);
}
float flowmaster::pafr()
{
	 float answer=massflow()/pressure();
	 return(answer);
}
float flowmaster::pressure()
{
	float pressure=0;
	if(pt1present)
	{
		// write code here for getting pressure from transducer here
	}else{
		if(num_flowmeters<1)
		{
			printf("no flowmeters found by pressure finding code");
			return(-1);
    }
		if(fmpresent[1])
		pressure+=flowmeter[1]->Pressure();
		if(fmpresent[2])
		pressure+=flowmeter[2]->Pressure();
		if(fmpresent[3])
		pressure+=flowmeter[3]->Pressure();
		if(fmpresent[4])
		pressure+=flowmeter[4]->Pressure();
		if(fmpresent[5])
		pressure+=flowmeter[5]->Pressure();
		pressure=pressure/num_flowmeters;
		// the above finds an average
		pressure=pressure*0.0689475729;
		// the above converts from psi to bar
	}
	return(pressure);
}
float flowmaster::massflow()
{
	reconsider_valves();
	float massflow=quick_massflow();
	return(massflow);
}
float flowmaster::quick_massflow()
{
	float massflow=0;
	if(fminuse>0) //this checks that there is a useable flowmeter so the system does not crash on startup
		massflow=(flowmeter[fminuse]->MassFlow());
	return(massflow);
}
flowmaster::~flowmaster()
{
	//lockdown(); //I tried to get lockdown to run when the form was closed but I found that this did not work.
}
