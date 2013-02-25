#ifndef weederioH
#define weederioH
//---------------------------------------------------------------------------
class weederio
{
	public:
		weederio(void);                // does nothing
		int setup(int weederio1port);  // opens the port and sets up
		int open(char chanel);				 // opens the valve specified in channel, note that chanel must be a capital letter
		int close(char chanel); 			 // closes the valve specified in channel, note that chanel must be a capital letter
		int request_status(char chanel);// tells the caller whether the valve is open, note that chanel must be a capital letter
		~weederio(void);               // does nothing
	private:
		int write(char*);              // sends a c string to the weederio board
		int wait_for_response(void);   // waits for responce from weederio board
		int clear_buffer(void);        // clears window's internal buffer of data sent from the board
};
#endif
