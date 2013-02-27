#include "motor.h"
#include <iostream.h>
motor::motor()
{
	MotorPosition=0;
}
int motor::setup(int motor1port)
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

	SetCommTimeouts(MotorPortHandle,&StdTimeouts);
	return(0);
}
int motor::SetMotorSettings()
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
int motor::set_origin()
{
	go_to(1000000,10000,200000); // goes near to the origin (this is faster then origin setting)
	clear_buffer();
	write("|,"); //sets the origin
	wait_for_response();
	Sleep(100);
	clear_buffer(); // this is because the motor sends two responces when it reaches the origin with a small gap between them
	MotorPosition=0;
	return(0);
}
int motor::go_to(int speed,int acceleration,int position)
{
	// the maximum length of string that the hardware can take is 50 chars so the below command (with the other stuff) can overload it
	sprintf(TmpBuffer,"S=%d,A=%d,P=%d,^,",speed,acceleration,position);
	write(TmpBuffer); // string should now look like: S=500000,A=5000,P=1000000,^,
	wait_for_response();
	MotorPosition=position;
	return(0);
}
int motor::fix_motor_free_state() // note this resumes the previous command so if it was impossible then it may still be impossible now
{
	write("(");
	Sleep(500); // this is to ensure that if the signal is sent back it is cleared so that the buffer is kept clear
	clear_buffer();
	return(0);
}
int motor::position()
{
	return (MotorPosition);
}
int motor::pseudotorque()// note this does not return torque but a related quantity that is roughly linear with torque and current
{
	write("?98,");
	DWORD BytesRead;
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
int motor::wait_for_response()
{
	unsigned long BytesRead;
	const int BufferSize=200;
	char Buffer[BufferSize];
	for(int i=0;i<=5000;i++)
	{
	ReadFile(MotorPortHandle,                        //HANDLE        hFile,
					 Buffer,                          //LPVOID        lpBuffer,
					 BufferSize,                    //DWORD         nNumberOfBytesToRead,
					&BytesRead,                       //LPDWORD       lpNumberOfBytesRead,
					 FALSE);
		//Buffer[BytesRead]=0;									//Null terminate buffer so only valid data appears!
		if (BytesRead>0)
		{
			//printf("%s\n",Buffer);
			return(0);
		}
		Sleep(1);
	}
	printf("error: expected motor signal not recived after 5 seconds\n");
	return (1);
}
int motor::write(char* to_send)
{
	DWORD BytesWritten;                        //Number of bytes written
	AnsiString AStringSendMe=to_send;
	WriteFile(		MotorPortHandle,                  //HANDLE hComFile
					AStringSendMe.c_str(),                  //LPCVOID lpBuffer,
					AStringSendMe.Length(),                  //DWORD nNumberOfBytesToWrite,  strlen
					&BytesWritten ,                  //LPDWORD lpNumberOfBytesWritten,
					FALSE);                  //LPOVERLAPPED lpOverlapped
	return(0);

}
int motor::clear_buffer()
{
	unsigned long BytesRead;
	const int BufferSize=200;
	char Buffer[BufferSize];

	ReadFile(MotorPortHandle,                        //HANDLE        hFile,
					 Buffer,                          //LPVOID        lpBuffer,
					 BufferSize,                    //DWORD         nNumberOfBytesToRead,
					&BytesRead,                       //LPDWORD       lpNumberOfBytesRead,
					 FALSE);
	// read file clears windows's internal buffer
	return(0);
}
motor::~motor()
{
	PurgeComm(MotorPortHandle,PURGE_RXCLEAR&PURGE_TXCLEAR);// this ditches the data in the port
	CloseHandle(MotorPortHandle);
}
