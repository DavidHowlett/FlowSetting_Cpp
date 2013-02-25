//---------------------------------------------------------------------------
//The purpose of this class is to provide RS232 data to other processes so that
//they can process the data without having to do lots of hard work.
#ifndef RS232PrimitiveH
#define RS232PrimitiveH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <StdCtrls.hpp>
#include <ComCtrls.hpp>
#include <SyncObjs.hpp>
#include "DumbStringList.h"
//---------------------------------------------------------------------------
class RS232Primitive : public TThread
{
	typedef struct tagTHREADNAME_INFO
	{
		DWORD dwType;     // must be 0x1000
		LPCSTR szName;    // pointer to name (in user addr space)
		DWORD dwThreadID; // thread ID (-1=caller thread)
		DWORD dwFlags;    // reserved for future use, must be zero
	} THREADNAME_INFO;
private:
	void SetName();
	long counter;
	void __fastcall UpdateTextBox();
	void __fastcall UpdateMemoMonitorSend    ();
//	void __fastcall UpdateMemoMonitorSend    (char* s);
	void __fastcall UpdateRichEditMonitorSend();
	void __fastcall UpdateMemoMonitorRead    ();
	void __fastcall UpdateRichEditMonitorRead();
	String FMessageToUser;              //Pointer to the message the program wants the user to see
	bool    FPortOpen;                   //Is this port open?
	HANDLE  hComFile;                    //Handle to the serial stream
	//Communication Parameters:
	String FComPort;                //Pointer to a string stating the ComPort
	DWORD   FBaudRate;                   // Baudrate at which running
	char    FParity   ;                  // (N)one, (O)dd, (E)ven, (M)ark, (S)pace, first char, any case, or 0-4=None,Odd,Even,Mark,Space
	BYTE    FByteSize ;                  // Number of bits/byte, 4-8
	BYTE    FStopBits ;                  // 0,1,2 = 1, 1.5, 2, store as 0,1,2 and NOT 1, 1.5, 2
	//Buffer
	TDumbStringList*  LineBuffer;                  //TList to hold the data as I could not get TStringList to work without exceptions. <sigh>
	//TStringList* LineBufferStrings;              // Replacement for the above Line Buffer
	TCriticalSection* LineBufferStringsCS;         // Critical section for Line buffer. Acquire before accessing it
	String ReadText;                           //Text that was read, to send to monitors
	String SentText;                           //Text that was sent, to send to monitors
protected:
	TMemo*   FMemo      ;                          //Memo to copy communications to. Optional, for monitoring purposes
	TRichEdit* FRichEdit;                          //RichEdit to copy communications to. Optionsl, for monitoring purposes
public:
	void __fastcall Execute();
		RS232Primitive(bool CreateSuspended);        //Constructor
	 __fastcall ~RS232Primitive(void);             //Destructor
	int   Open();
	void  Close();
	void SetMemoMonitor    (TMemo*     Memo    );  //Use a Memo to monitor the output
	void SetRichEditMonitor(TRichEdit* RichEdit);  //Use a RichEdit to monitor the output

	//TTime      StartTime;                                    //When did the run start
	//Integer    CompleteRecords;                              //How many records read so far?

	int   Setup( //All DCB parameters shown so they could be used later if needed
		String ComPort ,        /* Which communications port?      */
		DWORD BaudRate,        /* Baudrate at which running       */
		char Parity   ,        /* (N)one, (O)dd, (E)ven, (M)ark, (S)pace, first char, any case, or 0-4=None,Odd,Even,Mark,Space    */
		BYTE ByteSize ,        /* Number of bits/byte, 4-8        */
		float StopBits );      /* 1, 1.5, 2                       */
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
//		DWORD fAbortOnError:1; /* Abort all reads and writes on Error */
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
	int    Read();                   //Read the data in
	String ReadNBytes(int n);        //Return n bytes, or as near as.
	String ReadUpTo(String Str);      //Read all bytes up to "str", usually a <CR> or similar
	void   Cleanup(int LinesToSave); //Clear every line except last LinesToSave.
	DWORD  Send(String SendMe);       //Send this text out through the RS232 port
};
//---------------------------------------------------------------------------
#endif
