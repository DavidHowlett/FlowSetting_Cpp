#include <System.hpp>
#ifndef MotorH
#define MotorH
/*
the motor will keep track internaly of it's position,
when motor posistion is needed by a higher program it should be requested.
The "set origin" and "go to" commands will pause the process until the motor is
in position or is demostrably not going to reach it
*/
class Motor
{
	public:
		Motor(void);
		int Setup(int);                 // opens the motor port, should only be called on startup
		int GoTo(int,int,int);         // inputs are speed, accseleration and position. This tells the motor to go to the position specified, also updates the object's internal position monitor
		int SetOrigin(void);     		// this reverses the motor until it hits a mechanical stopper, then it sets the 0 position to be the stopper
		int FixMotorFreeState(void); // this sends the "(" command causeing the motor to reattempt the prevous command
		int Position(void);             // this gets the motor's current position
		int PseudoTorque(void); 		// this does not return torque but a related quantity that is correlated with torque
		~Motor(void);
	private:
		int SetMotorSettings(void);       	// sets the motor's internal settings, has the side effect of making the motor rush forwards
		int WriteToMotor(char*);               // sends a c string to the motor
		int WaitForResponse(void);    // pauses the program's operation until the motor sends data back
		int ClearBuffer(void);         // emptys window's internal buffer of data sent from the motor
	private:
		int MotorPosition;  // current actual motor position
		HANDLE MotorPortHandle;
		static const int TmpBufferSize=200;
		char TmpBuffer[TmpBufferSize];
		DWORD JunkData; // this is to satisfy API calls that want a DWORD
		COMMTIMEOUTS StdTimeouts;
		COMMTIMEOUTS WaitForData;
};
#endif
