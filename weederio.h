#ifndef WeederioH
#define WeederioH
//---------------------------------------------------------------------------
class Weederio
{
	public:
		Weederio(void);                // does nothing
		int Setup(int weederio1port);  // opens the port and sets up
		int Open(char chanel);				 // opens the valve specified in channel, note that chanel must be a capital letter
		int Close(char chanel); 			 // closes the valve specified in channel, note that chanel must be a capital letter
		int RequestStatus(char chanel);// tells the caller whether the valve is open, note that chanel must be a capital letter
		~Weederio(void);               // does nothing
	private:
		int Write(char*);              // sends a c string to the weederio board
		int WaitForResponse(void);   // waits for responce from weederio board
	private:
		HANDLE WeederioPortHandle;
		int Status[20]; // this will keep track internaly of the values shown by the io device. There will be no feedback. It should be 0 for closed and 1 for open
		static const int TmpBufferSize=200;
		char TmpBuffer[TmpBufferSize];
		DWORD JunkData; // this is to satisfy API calls that want a DWORD
		COMMTIMEOUTS StdTimeouts;
		COMMTIMEOUTS WaitForData;
};
#endif
