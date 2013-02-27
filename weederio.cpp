#include <iostream.h>
#include <System.hpp>
#include "weederio.h"
HANDLE aComFile;
int status[20]; // this will keep track internaly of the values shown by the io device. There will be no feedback. It should be 0 for closed and 1 for open
weederio::weederio()
{
}
weederio::setup(int weederio1port)
{
	char orig[100];
	sprintf(orig, "\\\\.\\COM%d",weederio1port);
	const size_t newsize = 100;
	wchar_t wcstring[newsize];
	mbstowcs(wcstring,orig,100);
	aComFile = CreateFile(		wcstring,	            //LPCTSTR lpFileName,
												GENERIC_READ | GENERIC_WRITE, //DWORD dwDesiredAccess ,
												0,	                  //DWORD dwShareMode (0 for COM port access),
												SECURITY_ANONYMOUS,	  //LPSECURITY_ATTRIBUTES lpSecurityAttributes,
												OPEN_EXISTING,	      //DWORD dwCreationDisposition (necessary to be OPEN_EXISTING for COM ports),
//												FILE_FLAG_OVERLAPPED,  //DWORD dwFlagsAndAttributes,
												FILE_ATTRIBUTE_NORMAL,//DWORD dwFlagsAndAttributes,
												0);	                  //HANDLE hTemplateFile

	if ((aComFile==INVALID_HANDLE_VALUE)||(aComFile==NULL))
	{
		printf("could not open weeder board port, please fix and restart program\n");
		system("PAUSE");
	}
	else
		printf("weeder board port opened\n");

	//Set the communication parameters
	DCB dcbCommPort;   //This structure is used to hold the COM port parameters
	dcbCommPort.DCBlength = sizeof(DCB); //This never needs to be changed, so set it on startup
	GetCommState(aComFile, &dcbCommPort);  //Read whatever state is to avoid hComPort's random contents corrupting defaults

	dcbCommPort.BaudRate=9600;
	dcbCommPort.fParity =1;
	dcbCommPort.Parity  =0;
	dcbCommPort.ByteSize=8;
	dcbCommPort.StopBits=0; // this means 1 stop bit

	SetCommState(aComFile, &dcbCommPort); //Write the modified result back.

	COMMTIMEOUTS CommTimeouts;
	CommTimeouts.ReadIntervalTimeout          =1;
	CommTimeouts.ReadTotalTimeoutConstant     =1;
	CommTimeouts.ReadTotalTimeoutMultiplier   =1;
	CommTimeouts.WriteTotalTimeoutConstant    =1;
	CommTimeouts.WriteTotalTimeoutMultiplier  =1;

	SetCommTimeouts(aComFile,&CommTimeouts);
	for(int i=0;i<20;i++)
	 status[i]=0;
	return(0);
}
weederio::open(char chanel)// note that chanel must be a capital letter
{
	char to_send[10];
	sprintf(to_send,"AH%c",chanel); // 'A' refers to the name of the io board, H says set the voltage high and the letter gives the chanel
	to_send[3]=13; //number 13 is carriage return used because all messages sent must end with carriage return return to be processed correctly
	status[(int)chanel-65]=1;   // the  purpose of this is to record that the chanel is high
	write(to_send);
	return(0);
}
weederio::close(char chanel)// note that chanel must be a capital letter
{
	char to_send[10];
	sprintf(to_send,"AL%c",chanel);
	to_send[3]=13; //number 13 is carriage return used because all messages sent must end with carriage return to be processed correctly
	status[(int)chanel-65]=0; // the  purpose of this is to record that the chanel is low
	write(to_send);
	return(0);
}
weederio::request_status(char chanel)// note that chanel must be a capital letter
{
	return(status[(int)chanel-65]);
}
weederio::~weederio()
{

}
weederio::write(char* to_send)
{
	DWORD BytesWritten;                        //Number of bytes written
	AnsiString AStringSendMe;
	AStringSendMe=to_send;
	//printf(to_send); // remove me
	//printf("\n");                  //remove me
	clear_buffer();// this clears window's internal buffer
	WriteFile(		aComFile,                  //HANDLE hComFile
								AStringSendMe.c_str(),                  //LPCVOID lpBuffer,
								AStringSendMe.Length(),                  //DWORD nNumberOfBytesToWrite,
								&BytesWritten ,                  //LPDWORD lpNumberOfBytesWritten,
								FALSE);                  //LPOVERLAPPED lpOverlapped
	wait_for_response();
	return(0);
}
weederio::wait_for_response()
{
	unsigned long BytesRead;
	const int BufferSize=20;
	char Buffer[BufferSize];
	for(int i=0;i<=5000;i++)
	{
	ReadFile(aComFile,                        //HANDLE        hFile,
					 Buffer,                          //LPVOID        lpBuffer,
					 BufferSize,                    //DWORD         nNumberOfBytesToRead,
					&BytesRead,                       //LPDWORD       lpNumberOfBytesRead,
					 FALSE);
		if (BytesRead>0)
		{
			//printf("%s\n",Buffer);
			return(0);
		}
		Sleep(1);
	}
	printf("error: weederio response not recived after 5 seconds\n");
	return (1);
}
weederio::clear_buffer()
{
	unsigned long BytesRead;
	const int BufferSize=200;
	char Buffer[BufferSize];
	ReadFile(aComFile,                        //HANDLE        hFile,
					 Buffer,                          //LPVOID        lpBuffer,
					 BufferSize,                    //DWORD         nNumberOfBytesToRead,
					&BytesRead,                       //LPDWORD       lpNumberOfBytesRead,
					 FALSE);
	// read file clears windows's internal buffer
	return(0);
}

