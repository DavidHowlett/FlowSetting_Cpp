//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <windows.h>
#include <stdio.h>
#include "Functions.h"
#include "RS232Primitive.h"
//#include "MainForm.h"
#include "StrUtils.hpp"
#include <conio.h>
#pragma package(smart_init)
//---------------------------------------------------------------------------

//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(&UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void  RS232Primitive::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------


//******************************************************************************
int  RS232Primitive::Setup( //All DCB parameters shown so they could be used later if needed
		String ComPort ,        /* Which communications port?      */
		DWORD BaudRate,        /* Baudrate at which running       */
		char Parity   ,        /* (N)one, (O)dd, (E)ven, (M)ark, (S)pace, first char, any case, or 0-4=None,Odd,Even,Mark,Space    */
		BYTE ByteSize ,        /* Number of bits/byte, 4-8        */
		float StopBits )       /* 1, 1.5, 2                       */
//		DWORD fBinary: 1;     /* Binary Mode (skip EOF check)    */
//		DWORD fParity: 1;     /* Enable parity checking          */
//		DWORD fOutxCtsFlow:1; /* CTS handshaking on output       */
//		DWORD fOutxDsrFlow:1; /* DSR handshaking on output       */
//		DWORD fDtrControl:2;  /* DTR Flow control                */
//		DWORD fDsrSensitivity:1; /* DSR Sensitivity              */
//		DWORD fTXContinueOnXoff: 1; /* Continue TX when Xoff sent */
//		DWORD fOutX: 1;       /* Enable output X-ON/X-OFF        */
//		DWORD fInX: 1;        /* Enable input X-ON/X-OFF         */
//		DWORD fErrorChar: 1;  /* Enable Err Replacement          */
//		DWORD fNull: 1;       /* Enable Null stripping           */
//		DWORD fRtsControl:2;  /* Rts Flow control                */
//		DWORD fAbortOnError:1;/* Abort all reads and writes on Error */
//		DWORD fDummy2:17;     /* Reserved                        */
//		WORD wReserved;       /* Not currently used              */
//		WORD XonLim;          /* Transmit X-ON threshold         */
//		WORD XoffLim;         /* Transmit X-OFF threshold        */
//		char XonChar;         /* Tx and Rx X-ON character        */
//		char XoffChar;        /* Tx and Rx X-OFF character       */
//		char ErrorChar;       /* Error replacement char          */
//		char EofChar;         /* End of Input character          */
//		char EvtChar;         /* Received Event character        */
//		WORD wReserved1;      /* Fill for now.                   */
//******************************************************************************
{
	FComPort=ComPort; //Capture the ComPort details
	FBaudRate  =BaudRate;        /* Baudrate at which running       */
	switch (Parity) {   /* (N)one, (O)dd, (E)ven, (M)ark, (S)pace, first char, any case, or 0-4=None,Odd,Even,Mark,Space    */
	case 'N':	case 'n':	case '0': FParity  =0; break; //None
	case 'O':	case 'o':	case '1': FParity  =1; break; //Odd
	case 'E': case 'e':	case '2': FParity  =2; break; //Even
	case 'M':	case 'm':	case '3': FParity  =3; break; //Mark
	case 'S':	case 's':	case '4': FParity  =4; break; //Space
	}
	FByteSize =ByteSize;            /* Number of bits/byte, 4-8        */
	if (StopBits<1.25) {            //1 stop bit. Don't ask me why "0" means 1 stop bit!
		FStopBits=0;
	}else if (StopBits<1.75) {      //1.5 stop bits. Don't ask me why "1" means 1.5 stop bits!
		FStopBits=1;
	}else{                          //2 stop bits. Finally 2 means 2. Sigh....
		FStopBits=2;
	};
	return 0;
};


//******************************************************************************
int  RS232Primitive::Open()
//******************************************************************************
{
	AnsiString AComPort;
	AComPort="\\\\.\\"+FComPort;
	const size_t newsize = 100;
	wchar_t wcstring[newsize];
	mbstowcs(wcstring,(AComPort.c_str()),100);
	hComFile = CreateFile(wcstring,	            //LPCTSTR lpFileName,
												GENERIC_READ | GENERIC_WRITE, //DWORD dwDesiredAccess ,
												0,	                  //DWORD dwShareMode (0 for COM port access),
												SECURITY_ANONYMOUS,	  //LPSECURITY_ATTRIBUTES lpSecurityAttributes,
												OPEN_EXISTING,	      //DWORD dwCreationDisposition (necessary to be OPEN_EXISTING for COM ports),
//												FILE_FLAG_OVERLAPPED,  //DWORD dwFlagsAndAttributes,
												FILE_ATTRIBUTE_NORMAL,//DWORD dwFlagsAndAttributes,
												0);	                  //HANDLE hTemplateFile
	DWORD ErrorCode=GetLastError();
	/*
	if (ErrorCode!=0) {
		char buff[1024];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
			0, ErrorCode, 0, buff, sizeof(buff), 0);
		ShowMessage(buff);
//		MessageBox(Handle, buff,
//			"System Error", MB_OK | MB_ICONWARNING);
	}
	*/
	if (ErrorCode!=0) {
		AnsiString msg="Cannot open port "+AComPort;
		puts(msg.c_str());
	}
	assert(ErrorCode==0);
	if (hComFile==INVALID_HANDLE_VALUE) {
		FPortOpen=False;  //Port is closed
//		Exception->Raise("Invalid Handle opening "+ComPort);
	}else{
		FPortOpen=True;  //Port is open
	}

	//Set the communication parameters
	DCB dcbCommPort;   //This structure is used to hold the COM port parameters
	dcbCommPort.DCBlength = sizeof(DCB); //This never needs to be changed, so set it on startup
	GetCommState(hComFile, &dcbCommPort);  //Read whatever state is to avoid hComPort's random contents corrupting defaults
//	ErrorCode=GetLastError();

	dcbCommPort.BaudRate=FBaudRate;
	dcbCommPort.fParity =False;
	dcbCommPort.Parity  =FParity;
	dcbCommPort.ByteSize=FByteSize;
	dcbCommPort.StopBits=FStopBits;
	SetCommState(hComFile, &dcbCommPort); //Write the modified result back.
	ErrorCode=GetLastError();

	//Set the communication time-outs.
	COMMTIMEOUTS CommTimeouts;
	CommTimeouts.ReadIntervalTimeout          =1;
	CommTimeouts.ReadTotalTimeoutConstant     =1;
	CommTimeouts.ReadTotalTimeoutMultiplier   =1;
	CommTimeouts.WriteTotalTimeoutConstant    =1;
	CommTimeouts.WriteTotalTimeoutMultiplier  =1;

	SetCommTimeouts(hComFile,&CommTimeouts);


	//StartTime=Now();                                    //When did the run start
	//CompleteRecords=0;                              //How many records read so far?

	return 0;
}

//******************************************************************************
void  RS232Primitive::Close()
//******************************************************************************
{
	CloseHandle(hComFile);
}

//******************************************************************************
 RS232Primitive::RS232Primitive(bool CreateSuspended): TThread(CreateSuspended)
//Constructor without alot of constructing to do!
//******************************************************************************
{
	hComFile  =NULL;                          //Just set to NULL to be sure
	FMemo     =NULL;                          //Set the monitoring Memo     to NULL to be sure
	FRichEdit =NULL;                          //Set the monitoring RichEdit to NULL to be sure
	LineBuffer= new TDumbStringList;        //TSList to hold the data in boring C strings
	LineBufferStringsCS=new TCriticalSection();
}

//******************************************************************************
 __fastcall RS232Primitive::~RS232Primitive(void)       //Destructor
//Destructor without alot of denstructing to do!
//******************************************************************************
{
	Close();
	LineBufferStringsCS->Free();
}

//******************************************************************************
void RS232Primitive::SetMemoMonitor(TMemo* Memo)
//This sets up FMemo for monitoring i/o
//******************************************************************************
{
	if ((FMemo==NULL) && (Memo!=NULL)) {
		Memo->Lines->Add("Monitoring connected");
	}
	if ((FMemo!=NULL) && (Memo==NULL)) {
		FMemo->Lines->Add("Monitoring disconnecting");
	}
	FMemo=Memo;
}

//******************************************************************************
void RS232Primitive::SetRichEditMonitor(TRichEdit* RichEdit)
//This sets up FRichEdit for monitoring i/o
//******************************************************************************
{
	if ((FRichEdit==NULL) && (RichEdit!=NULL)) {
		RichEdit->Lines->Add("Monitoring connected");
	}
	if ((FRichEdit!=NULL) && (RichEdit==NULL)) {
		FRichEdit->Lines->Add("Monitoring disconnecting");
	}
	FRichEdit=RichEdit;
}

//******************************************************************************
void RS232Primitive::SetName()
//******************************************************************************
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = "RS232Primitive";
	info.dwThreadID = -1;
	info.dwFlags = 0;

	__try
	{
		RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD),(DWORD*)&info );
	}
	__except (EXCEPTION_CONTINUE_EXECUTION)
	{
	}
}

//******************************************************************************
void __fastcall RS232Primitive::UpdateTextBox()
//******************************************************************************
{
	/*
	String s=ReadUpTo(CRString);         //This allocates memory to *s
	if (s!="") {
		Form1->Memo1->Lines->Add(s); //This uses it
//		free(s);                     //This releases memory again. <sigh>
	}
	*/
}
//******************************************************************************
void __fastcall RS232Primitive::UpdateMemoMonitorSend()
//******************************************************************************
{
	if (FMemo!=NULL) {
		FMemo->Text=FMemo->Text+"Sent:"+SentText;
		FMemo->Text=FMemo->Text+SentText;
	}
}

//******************************************************************************
void __fastcall RS232Primitive::UpdateRichEditMonitorSend()
//******************************************************************************
{
	if (FRichEdit!=NULL) {
//		FRichEdit->Font->Color = clBlue;
		FRichEdit->Text        =FRichEdit->Text+SentText;
//		FRichEdit->SelStart    =FRichEdit->GetTextLen();
//		FRichEdit->SelLength   =0-strlen(SentText);
//		FRichEdit->Attributes->Color = clBlue;
	}
}

//******************************************************************************
void __fastcall RS232Primitive::UpdateMemoMonitorRead()
//******************************************************************************
{
	if (FMemo!=NULL) {
		FMemo->Text=FMemo->Text+ReadText;
//		FMemo->Lines->Add("Read:"+ReadText);
	}
}

//******************************************************************************
void __fastcall RS232Primitive::UpdateRichEditMonitorRead()
//******************************************************************************
{
	if (FRichEdit!=NULL) {
//		FRichEdit->Font->Color = clGreen;
		FRichEdit->Text=FRichEdit->Text+ReadText;
//		FRichEdit->SelStart    =FRichEdit->GetTextLen();
//		FRichEdit->SelLength   =0-strlen(SentText);
//		FRichEdit->SelAttributes->Color = clGreen;
	}
}


//******************************************************************************
String  RS232Primitive::ReadUpTo(String Str)
//Read all bytes up to "str", usually a <CR> or similar
//If str not found, return everything so far.
//******************************************************************************
{
	//char* posn;                                            //Where is "str" in the buffer
	String ReturnStr="";                                     //The string to be returned to caller
	String Residue  ="";                                     //Bytes left over to return to the buffer
	String Temp;
	int LineCount;
	LineBufferStringsCS->Acquire();                          //Critical Section: Enter
	__try{
		if (Str.Length()>0) {                                  //if str is empty then we don't return anything
			int TerminatorPos=-1;
			while (LineBuffer->Count()>0) {//While there are data lines remaining
				Residue=Residue+LineBuffer->Get(0);
				LineBuffer->Delete(0);                             //We've consumed this data- now forget about it.
				TerminatorPos=Residue.Pos(Str);                    //Find the position of str in ReturnStr
				if (TerminatorPos>0) {
					//CompleteRecords++;
					ReturnStr=Residue.SubString(1,TerminatorPos+Str.Length()-1); //Got something!
					Residue  =Residue.SubString(TerminatorPos+Str.Length(),Residue.Length());  //Find the bytes not used this time
					break;
				}
			}
		}
		if (LineBuffer->Count()>0) {
			LineBuffer->Set(0,Residue);
		}else{
			LineBuffer->Add(Residue);
		}
	}__finally{
		LineBufferStringsCS->Release();                        //Dispose of Critical Section
	}
	return ReturnStr;
}


//******************************************************************************
String RS232Primitive::ReadNBytes(int n)        //Return LineBuffer[0] so no data is lost
//******************************************************************************
{
	AnsiString ReturnStr;                                 //We shall return this string... maybe!
	String s;                                         //Pointer to current buffer line
	LineBufferStringsCS->Acquire();
	__try{
		while ((ReturnStr.Length()<n) &&                                   //While not enough bytes yet and
					 (LineBuffer->Count()>0)){                         //there are lines still in the buffer and
			ReturnStr=ReturnStr+LineBuffer->Get(0);             //We plan to return this... or most of it. :-)
			if (ReturnStr.Length()>=n) {                          //Woohoo, we have enough text!
				LineBuffer->Set(0,MidStr(ReturnStr,n+1,ReturnStr.Length()));         //Return the unwanted bytes
				return ReturnStr.SubString(1,n);                                 //We have done the task, return this string
			}else{
				LineBuffer->Delete(0);                            //Delete this line, we still don't have enough bytes
			}
		}
		//We can only get here if we don't have enough bytes. Return the string to the buffer and return nothing
		LineBuffer->Add(ReturnStr);
	}__finally{
		LineBufferStringsCS->Release();
	}
	return "";
}

//******************************************************************************
void  RS232Primitive::Cleanup(int LinesToSave) //Clear every line except last LinesToSave.
//******************************************************************************
{
	LineBufferStringsCS->Acquire();
	__try{
		// Nuke all lines until we only have "LinesToSave" left.  Original code wiped everything
		while (LineBuffer->Count()>LinesToSave){
			LineBuffer->Delete(0);
		}
	}__finally{
		LineBufferStringsCS->Release();
	}
}

//******************************************************************************
DWORD RS232Primitive::Send(String StringSendMe)       //Send this text out through the RS232 port
//******************************************************************************
{
	DWORD BytesWritten;                        //Number of bytes written
	AnsiString AStringSendMe;
	AStringSendMe=StringSendMe;
	WriteFile(hComFile      ,                  //HANDLE hComFile
						AStringSendMe.c_str(),                  //LPCVOID lpBuffer,
						AStringSendMe.Length(),                  //DWORD nNumberOfBytesToWrite,
					 &BytesWritten  ,                  //LPDWORD lpNumberOfBytesWritten,
						FALSE        );                  //LPOVERLAPPED lpOverlapped

//	SentText=NULL;                             //Buffer for text to send to monitors
	if ((FMemo!=NULL)||(FRichEdit!=NULL)) {
//		SentText=(char*)malloc(BytesWritten+10); //Create a temporary buffer
//		strcpy(SentText,StringSendMe.c_str()); //Copy the transmitted bytes to it, less <CR><LF>
		SentText=StringSendMe;//[BytesWritten]=0;              //Add a null termination

		if (FMemo    !=NULL) {                   //Copy the sent bytes to the Memo Monitor
			UpdateMemoMonitorSend();
//			Synchronize(&UpdateMemoMonitorSend);
		}
		if (FRichEdit!=NULL) {                   //Copy the sent bytes to the RichEdit Monitor
			UpdateRichEditMonitorSend();
//			Synchronize(&UpdateRichEditMonitorSend);
		}

	}
//	free(SentText);                            //Free the memory
//	SentText=NULL;                             //Clear pointer for clarity
	return BytesWritten;                       //Just say how many were send.
}

//******************************************************************************
void __fastcall RS232Primitive::Execute()
//******************************************************************************
{
	char TestMessage[100];
	String s;
	SetName();
	//---- Place thread code here ----
	FreeOnTerminate = true;  //Free memory on termination
	FMessageToUser=TestMessage;

	while (TRUE){
//		Sleep(5);
//		sprintf(FMessageToUser,"Cycle %d\n",counter);
//		Synchronize(&UpdateTextBox);
		Read();  //Obtain the data;
//		Synchronize(&UpdateTextBox);
//		if (LineBuffer->Count>=2) {
//			FMessageToUser=NextLine();
//			if (strlen(FMessageToUser)>0) {
//				Synchronize(&UpdateTextBox);
//			}
//			delete FMessageToUser; //Release this memory- we don't need it any more.
//		}

//		if (Terminated) {                 //If nicely told to stop running
//			strcpy (TestMessage,"Goodbye");
//			FMessageToUser=TestMessage;
////			Synchronize(&UpdateTextBox);
////			Close();
//			break;
//		}
	}
	FMessageToUser=NULL;
}

//******************************************************************************
int  RS232Primitive::Read()
//******************************************************************************
{
	unsigned long BytesRead;
	char* Buffer;                             //Used to retrieve bytes from the string. Must be a C String!
	const BufferSize=200;                      //Size of buffer
//  Sleep(40);
	Buffer=(char*)malloc(BufferSize);         //Number of bytes we need to read. Allow 1 byte for trailing 0
	ReadFile(hComFile,                        //HANDLE        hFile,
					 Buffer,                          //LPVOID        lpBuffer,
					 BufferSize-1,                    //DWORD         nNumberOfBytesToRead,
					&BytesRead,                       //LPDWORD       lpNumberOfBytesRead,
					 FALSE);
	Buffer[BytesRead]=0;                      //Null terminate buffer so only valid data appears!
	if (BytesRead>0) {
		LineBufferStringsCS->Acquire();// Lock critical section
//		for (int i=0; i < BytesRead; i++) {
//			putch(Buffer[i]);
//			if (Buffer[i]==13) {
//				putch(10);
//			}
//		}
		__try{
			LineBuffer->Add(Buffer);                //Add the bytes into the LineBuffer array
		}__finally{
			LineBufferStringsCS->Release();// Unlock critical section
		}
	}else{
		free(Buffer);                          //To avoid memory leak, if buffer not used, destroy it.
	}
	if ((BytesRead>0) && ((FMemo!=NULL) || (FRichEdit!=NULL))){
		ReadText=Buffer;//(char*)malloc(BytesRead+1);       //Grab enough space
		if (FMemo    !=NULL) {                     //Copy the read bytes to the Memo Monitor
			Synchronize(&UpdateMemoMonitorRead);
		}
		if (FRichEdit!=NULL) {                     //Copy the read bytes to the RichEdit Monitor
			Synchronize(&UpdateRichEditMonitorRead);
		}
//		free(ReadText);
//		ReadText=NULL;
	}

	return BytesRead;
}


