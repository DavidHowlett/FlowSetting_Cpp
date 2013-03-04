#include "Weederio.h"
#include <System.hpp>
#include <cassert>
#include <iostream.h>

Weederio::Weederio()
{
	for(int i=0;i<20;i++)
		Status[i]=0;
}
Weederio::Setup(int WeederioPort)
{
	sprintf(TmpBuffer, "\\\\.\\COM%d",WeederioPort);
	const size_t newsize = 100;
	wchar_t wcstring[newsize];
	mbstowcs(wcstring,TmpBuffer,100);
	WeederioPortHandle = CreateFile(	wcstring,	          //LPCTSTR lpFileName,
										GENERIC_READ | GENERIC_WRITE, //DWORD dwDesiredAccess ,
										0,	                  //DWORD dwShareMode (0 for COM port access),
										SECURITY_ANONYMOUS,	  //LPSECURITY_ATTRIBUTES lpSecurityAttributes,
										OPEN_EXISTING,	      //DWORD dwCreationDisposition (necessary to be OPEN_EXISTING for COM ports),
										FILE_ATTRIBUTE_NORMAL,//DWORD dwFlagsAndAttributes,
										0);	                  //HANDLE hTemplateFile
	assert(WeederioPortHandle!=INVALID_HANDLE_VALUE); // this pair of checks should probably be replaced with smarter code later
	assert(WeederioPortHandle!=NULL);
	printf("weeder board port opened\n");

	//Set the communication parameters
	DCB dcbCommPort;   //This structure is used to hold the COM port parameters
	dcbCommPort.DCBlength = sizeof(DCB); //This never needs to be changed, so set it on startup
	GetCommState(WeederioPortHandle, &dcbCommPort);  //Read whatever state is to avoid hComPort's random contents corrupting defaults
	dcbCommPort.BaudRate=9600;
	dcbCommPort.fParity =1;
	dcbCommPort.Parity  =0;
	dcbCommPort.ByteSize=8;
	dcbCommPort.StopBits=0; // this means 1 stop bit
	SetCommState(WeederioPortHandle, &dcbCommPort); //Write the modified result back.

	COMMTIMEOUTS CommTimeouts;
	CommTimeouts.ReadIntervalTimeout          =1;
	CommTimeouts.ReadTotalTimeoutConstant     =1;
	CommTimeouts.ReadTotalTimeoutMultiplier   =1;
	CommTimeouts.WriteTotalTimeoutConstant    =1;
	CommTimeouts.WriteTotalTimeoutMultiplier  =1;
	SetCommTimeouts(WeederioPortHandle,&CommTimeouts);
	return 0;
}
Weederio::Open(char Channel)// note that chanel must be a capital letter
{
	char ToSend[10];
	sprintf(ToSend,"AH%c",Channel); // 'A' refers to the name of the io board, H says set the voltage high and the letter gives the chanel
	ToSend[3]=13; //number 13 is carriage return used because all messages sent must end with carriage return return to be processed correctly
	Status[(int)Channel-65]=1;   // the  purpose of this is to record that the chanel is high
	Write(ToSend);
	return(0);
}
Weederio::Close(char Channel)// note that chanel must be a capital letter
{
	char ToSend[10];
	sprintf(ToSend,"AL%c",Channel);
	ToSend[3]=13; //number 13 is carriage return used because all messages sent must end with carriage return to be processed correctly
	Status[(int)Channel-65]=0; // the  purpose of this is to record that the chanel is low
	Write(ToSend);
	return(0);
}
Weederio::RequestStatus(char Chanel)// note that chanel must be a capital letter
{
	return(Status[(int)Chanel-65]);
}

Weederio::Write(char* ToSend)
{
	DWORD BytesWritten;                        //Number of bytes written
	AnsiString AStringSendMe;
	AStringSendMe=ToSend;
	//printf(to_send); // remove me
	//printf("\n");                  //remove me
	PurgeComm(WeederioPortHandle,PURGE_RXCLEAR&PURGE_TXCLEAR);// this clears window's input and output buffers
	WriteFile(	WeederioPortHandle,     //HANDLE hComFile
				AStringSendMe.c_str(),  //LPCVOID lpBuffer,
				AStringSendMe.Length(),	//DWORD nNumberOfBytesToWrite,
				&BytesWritten ,         //LPDWORD lpNumberOfBytesWritten,
				FALSE);                 //LPOVERLAPPED lpOverlapped
	WaitForResponse();
	return(0);
}
Weederio::WaitForResponse()
{
	unsigned long BytesRead;
	const int BufferSize=20;
	char Buffer[BufferSize];
	for(int i=0;i<=5000;i++)
	{
	ReadFile(	WeederioPortHandle,		//HANDLE        hFile,
				Buffer,                 //LPVOID        lpBuffer,
				BufferSize,             //DWORD         nNumberOfBytesToRead,
				&BytesRead,             //LPDWORD       lpNumberOfBytesRead,
				FALSE);
		if (BytesRead>0)
			return 0;
		Sleep(1);
	}
	printf("error: weederio response not recived after 5 seconds\n");
	return (1);
}
Weederio::~Weederio()
{
	PurgeComm(WeederioPortHandle,PURGE_RXCLEAR&PURGE_TXCLEAR);// this clears window's input and output buffers
	CloseHandle(WeederioPortHandle);
}

