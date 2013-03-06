#include "Motor.h"
#include <iostream.h>
Motor::Motor()
{
	MotorPosition=0;
	StdTimeouts.ReadIntervalTimeout         =MAXDWORD;
	StdTimeouts.ReadTotalTimeoutConstant    =0;
	StdTimeouts.ReadTotalTimeoutMultiplier  =0;
	StdTimeouts.WriteTotalTimeoutConstant   =100;
	StdTimeouts.WriteTotalTimeoutMultiplier =10;

	WaitForData.ReadIntervalTimeout 		=50;
	WaitForData.ReadTotalTimeoutConstant 	=200;
	WaitForData.ReadTotalTimeoutMultiplier 	=10;
	WaitForData.WriteTotalTimeoutConstant  	=100;
	WaitForData.WriteTotalTimeoutMultiplier =10;
}
int Motor::Setup(int motor1port)
{
	sprintf(TmpBuffer, "\\\\.\\COM%d",motor1port);
	wchar_t wcstring[100];
	mbstowcs(wcstring,TmpBuffer,100);
	MotorPortHandle = CreateFile(	wcstring,	          //LPCTSTR lpFileName,
									GENERIC_READ | GENERIC_WRITE, //DWORD dwDesiredAccess ,
									0,	                  //DWORD dwShareMode (0 for COM port access),
									SECURITY_ANONYMOUS,	  //LPSECURITY_ATTRIBUTES lpSecurityAttributes,
									OPEN_EXISTING,	      //DWORD dwCreationDisposition (necessary to be OPEN_EXISTING for COM ports),
									FILE_ATTRIBUTE_NORMAL,//DWORD dwFlagsAndAttributes,
									0);	                  //HANDLE hTemplateFile

	if (MotorPortHandle==INVALID_HANDLE_VALUE)
		printf("could not open motor port\n");
	else
		printf("motor port opened\n");
	//Set the communication parameters
	DCB dcbCommPort;   //This structure is used to hold the COM port parameters
	dcbCommPort.DCBlength = sizeof(DCB); //This never needs to be changed, so set it on startup
	GetCommState(MotorPortHandle, &dcbCommPort);  //Read whatever state is to avoid hComPort's random contents corrupting defaults
	dcbCommPort.BaudRate=38400;
	dcbCommPort.fParity =1;
	dcbCommPort.Parity  =0;
	dcbCommPort.ByteSize=8;
	dcbCommPort.StopBits=0; // this means 1 stop bit
	SetCommState(MotorPortHandle, &dcbCommPort); //Write the modified result back.
	SetCommTimeouts(MotorPortHandle,&WaitForData);
	return(0);
}
int Motor::SetMotorSettings()
{
	//write(".1,K20=0,K23=9,K37=100,K42=1500,^,");
	//wait_for_response();
	//write("K43=5000,K45=2,K46=0,K47=10,K48=0,K55=10,^,"); // this command is 43 char long, the hardware limit is 50
	//wait_for_response();
	//write("K57=1000,K58=0,K59=0,^,");
	//wait_for_response();
	//printf("motor settings set\n");
	return(0);
}
int Motor::SetOrigin()
{
	// I would like to use the below line to speed things up but this would cause issues when
	// the motor is recovering from a motor free state
	//GoTo(1000000,10000,200000); // goes near to the origin (this is faster then origin setting)
	ClearBuffer();
	WriteToMotor("|,"); //sets the origin
	WaitForResponse();
	Sleep(100);
	ClearBuffer(); // this is because the motor sends two responces when it reaches the origin with a small gap between them
	MotorPosition=0;
	return(0);
}
int Motor::GoTo(int Speed,int Acceleration,int Position)
{
	// the maximum length of string that the hardware can take is 50 chars so the below command (with the other stuff) can overload it
	sprintf(TmpBuffer,"S=%d,A=%d,P=%d,^,",Speed,Acceleration,Position);
	WriteToMotor(TmpBuffer); // string should now look something like: S=500000,A=10000,P=1000000,^,
	WaitForResponse();
	MotorPosition=Position;
	return(0);
}
int Motor::FixMotorFreeState() // note this resumes the previous command so if it was impossible then it may still be impossible now
{
	WriteToMotor("(");
	Sleep(500); // this is to ensure that if the signal is sent back it is cleared so that the buffer is kept clear
	ClearBuffer();
	return(0);
}
int Motor::Position()
{
	return (MotorPosition);
}
int Motor::PseudoTorque()// note this does not return torque but a related quantity that is roughly linear with torque and current
{
	DWORD BytesRead;
	WriteToMotor("?98,");
	for(int i=0;i<=1000;i++)
	{
	ReadFile(	MotorPortHandle,//HANDLE        hFile,
				TmpBuffer,      //LPVOID        lpBuffer,
				TmpBufferSize,  //DWORD         nNumberOfBytesToRead,
				&BytesRead,     //LPDWORD       lpNumberOfBytesRead,
				FALSE);
		TmpBuffer[BytesRead]=0;									//Null terminate buffer so only valid data appears!
		if (BytesRead>0)
		{
			for (i = 0; i < 20; i++)
				TmpBuffer[i]=TmpBuffer[i+5]; // this gets rid of the first 5 charictors of the response leaving only the nul termianted number
			int torque=atoi(TmpBuffer);
			return(torque);
		}
		Sleep(5);
	}
	printf("error: motor torque level not recived after 5 seconds/n");
	return(1000000);
}
int Motor::WaitForResponse()
{
	DWORD BytesRead;
	for(int i=0;i<=5000;i++)
	{
	ReadFile(	MotorPortHandle,	//HANDLE        hFile,
				TmpBuffer,      	//LPVOID        lpBuffer,
				TmpBufferSize,  	//DWORD         nNumberOfBytesToRead,
				&BytesRead,          //LPDWORD       lpNumberOfBytesRead,
				FALSE);
		if (BytesRead>0)
			return 0;
		Sleep(1);
	}
	printf("error: expected motor signal not recived after 5 seconds\n");
	return 1;
}
int Motor::WriteToMotor(char* to_send)
{
	AnsiString AStringSendMe=to_send;
	WriteFile(	MotorPortHandle,       	//HANDLE hComFile
				AStringSendMe.c_str(), 	//LPCVOID lpBuffer,
				AStringSendMe.Length(),	//DWORD nNumberOfBytesToWrite,  strlen
				&JunkData ,        		//LPDWORD lpNumberOfBytesWritten,
				FALSE);                	//LPOVERLAPPED lpOverlapped
	return 0;

}
int Motor::ClearBuffer()
{
	SetCommTimeouts(MotorPortHandle,&WaitForData);
	ReadFile(	MotorPortHandle,	//HANDLE        hFile,
				TmpBuffer,      	//LPVOID        lpBuffer,
				TmpBufferSize,  	//DWORD         nNumberOfBytesToRead,
				&JunkData,     		//LPDWORD       lpNumberOfBytesRead,
				FALSE);
	SetCommTimeouts(MotorPortHandle,&StdTimeouts);
	return(0);
}
Motor::~Motor()
{
	PurgeComm(MotorPortHandle,PURGE_RXCLEAR&PURGE_TXCLEAR);// this ditches the data in the port
	CloseHandle(MotorPortHandle);
}
