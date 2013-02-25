#pragma hdrstop
#include "motor.h"
#include <iostream.h>
//---------------------------------------------------------------------------
#pragma hdrstop
#include "motor.h"
//#include <windows.h>
#include <System.hpp>
using namespace std;
//---------------------------------------------------------------------------
#pragma package(smart_init)
/*
the motor will keep track internaly of it's position,
when motor posistion is needed by a higher program it should be requested
the "set origin" and "go to" commands will pause the process until the motor is
in position or is demostrably not going to reach it
*/
int camp=0;  // current actual motor position
HANDLE hComFile;
motor::motor()
{

}
motor::setup(motor1port)
{
	char orig[100];
	sprintf(orig, "\\\\.\\COM%d",motor1port);
	wchar_t wcstring[100];
	mbstowcs(wcstring,orig,100);
	hComFile = CreateFile(wcstring,	            //LPCTSTR lpFileName,
												GENERIC_READ | GENERIC_WRITE, //DWORD dwDesiredAccess ,
												0,	                  //DWORD dwShareMode (0 for COM port access),
												SECURITY_ANONYMOUS,	  //LPSECURITY_ATTRIBUTES lpSecurityAttributes,
												OPEN_EXISTING,	      //DWORD dwCreationDisposition (necessary to be OPEN_EXISTING for COM ports),
//												FILE_FLAG_OVERLAPPED,  //DWORD dwFlagsAndAttributes,
												FILE_ATTRIBUTE_NORMAL,//DWORD dwFlagsAndAttributes,
												0);	                  //HANDLE hTemplateFile

	if (hComFile==INVALID_HANDLE_VALUE)
		printf("could not open motor port\n");
	else
		printf("motor port opened\n");

	//Set the communication parameters
	DCB dcbCommPort;   //This structure is used to hold the COM port parameters
	dcbCommPort.DCBlength = sizeof(DCB); //This never needs to be changed, so set it on startup
	GetCommState(hComFile, &dcbCommPort);  //Read whatever state is to avoid hComPort's random contents corrupting defaults

	dcbCommPort.BaudRate=38400;
	dcbCommPort.fParity =1;
	dcbCommPort.Parity  =0;
	dcbCommPort.ByteSize=8;
	dcbCommPort.StopBits=0; // this means 1 stop bit

	SetCommState(hComFile, &dcbCommPort); //Write the modified result back.

	COMMTIMEOUTS CommTimeouts;
	CommTimeouts.ReadIntervalTimeout          =1;
	CommTimeouts.ReadTotalTimeoutConstant     =1;
	CommTimeouts.ReadTotalTimeoutMultiplier   =1;
	CommTimeouts.WriteTotalTimeoutConstant    =1;
	CommTimeouts.WriteTotalTimeoutMultiplier  =1;

	SetCommTimeouts(hComFile,&CommTimeouts);
	return(0);
}
motor::set_K_values()
{
	//write(".1,K20=0,K23=9,K37=100,K42=1500,^,");
	//wait_for_response();
	//write("K43=5000,K45=2,K46=0,K47=10,K48=0,K55=10,^,"); // this command is 43 char long the hardware limit is 50
	//wait_for_response();
	//write("K57=1000,K58=0,K59=0,^,");
	//wait_for_response();
	//printf("motor settings set\n");
	return(0);
}
motor::set_origin()
{

	//write("S=1000000,A=10000,P=200000,^,"); // goes near to the origin (this is faster then origin setting)
	//wait_for_response();
	clear_buffer();
	write("|,"); //sets the origin
	wait_for_response();
	Sleep(100);
	clear_buffer(); // this is because the motor sends two responces when it reaches the origin with a small gap between them
	camp=0;

	return(0);
}
motor::go_to(	int speed_int,
				int acceleration_int,
				int position_int)
{
	char speed_str[50];// the maximum length of string that the hardware can take is 50 so this char array (with the other stuff) can overload it
	char acceleration_str[50];
	char position_str[50];

	sprintf(speed_str,"S=%d",speed_int);
	sprintf(acceleration_str,",A=%d",acceleration_int);
	sprintf(position_str,",P=%d",position_int);

	write(speed_str);
	write(acceleration_str);
	write(position_str);
	write(",^,"); // string should now look like: S=1000000,A=5000,P=3000000,^,
	wait_for_response();
	camp=position_int;
	return(0);
}
motor::fix_motor_free_state() // note this resumes the previous command so if it was impossible then it may still be impossible now
{
	write("(");
	Sleep(500); // this is to ensure that if the signal is sent back it is cleared so that the buffer is kept clear
	clear_buffer();
	return(0);
}
motor::position()
{
	return (camp);
}
motor::pseudotorque()// note this does not return torque but a related quantity that is roughly linear with torque and current
{
	write("?98,");
	char buffer[100];
	unsigned long BytesRead;
	const int BufferSize=200;
	char Buffer[BufferSize];
	for(int i=0;i<=1000;i++)
	{
	ReadFile(hComFile,                        //HANDLE        hFile,
					 Buffer,                          //LPVOID        lpBuffer,
					 BufferSize,                    //DWORD         nNumberOfBytesToRead,
					&BytesRead,                       //LPDWORD       lpNumberOfBytesRead,
					 FALSE);
		Buffer[BytesRead]=0;									//Null terminate buffer so only valid data appears!
		if (BytesRead>0)
		{
			//printf("%s\n",Buffer);
			for (i = 0; i < 20; i++)
			{
					Buffer[i]=Buffer[i+5]; // this gets rid of the first 5 charictors of the response leaving only the nul termianted number
			}
			//istringstream iss( Buffer );
			int torque=atoi(Buffer);
			//iss >> torque;
			return(torque);
		}
		Sleep(5);
	}
	printf("error: motor torque level not recived after 5 seconds/n");
	return(1000000);
}
motor::wait_for_response()
{
	unsigned long BytesRead;
	const int BufferSize=200;
	char Buffer[BufferSize];
	for(int i=0;i<=5000;i++)
	{
	ReadFile(hComFile,                        //HANDLE        hFile,
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
motor::~motor()
{
	//
}
motor::write(char* to_send)
{
	DWORD BytesWritten;                        //Number of bytes written
	AnsiString AStringSendMe;
	AStringSendMe=to_send;
	WriteFile(		hComFile,                  //HANDLE hComFile
					AStringSendMe.c_str(),                  //LPCVOID lpBuffer,
					AStringSendMe.Length(),                  //DWORD nNumberOfBytesToWrite,  strlen
					&BytesWritten ,                  //LPDWORD lpNumberOfBytesWritten,
					FALSE);                  //LPOVERLAPPED lpOverlapped
	return(0);

}
motor::clear_buffer()
{
	unsigned long BytesRead;
	const int BufferSize=200;
	char Buffer[BufferSize];

	ReadFile(hComFile,                        //HANDLE        hFile,
					 Buffer,                          //LPVOID        lpBuffer,
					 BufferSize,                    //DWORD         nNumberOfBytesToRead,
					&BytesRead,                       //LPDWORD       lpNumberOfBytesRead,
					 FALSE);
	// read file clears windows's internal buffer
	return(0);
}