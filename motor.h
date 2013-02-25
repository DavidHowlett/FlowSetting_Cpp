//---------------------------------------------------------------------------

#ifndef motorH
#define motorH
//---------------------------------------------------------------------------
class motor
{
	public:
		motor(void);                  	// does nothing
		int setup(int);                 // opens the motor port, should only be called on startup
		int set_K_values(void);       	// sets the motor's internal settings, has the side effect of making the motor rush forwards
		int go_to(int,int,int);         // inputs are speed, accseleration and position. This tells the motor to go to the position specified, also updates the object's internal position monitor
		int set_origin(void);     			// this reverses the motor until it hits a mechanical stopper, then it sets the 0 position to be the stopper
		int fix_motor_free_state(void); // this sends the "(" command causeing the motor to reattempt the prevous command
		int position(void);             // this gets the motor's current position
		int pseudotorque(void); 				// this does not return torque but a related quantity that is correlated with torque
		~motor(void);                   // does nothing
	private:
		int write(char*);               // sends a c string to the motor
		int wait_for_response(void);    // pauses the program's operation until the motor sends data back
		int clear_buffer(void);         // emptys window's internal buffer of data sent from the motor
};
#endif
