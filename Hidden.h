// I decided to hide a copy of the documentation (up to date 29/09/20011) in the code so that the next guy to work on this has an easier time if the primary copy of the documentation is lost

/*
Technical documentation for automated flow setter

Notes for standard user
•	The program can be exited using the e key at the point that it asks for user input. Exiting by pressing the x in the top right hand corner does not close the valves and hence leaves the system vulnerable to breaking the flow meters. I broke a flowmeter this way. If you wish to exit the program in another way then you can close the valves by turning the off power supply to the weeder io board for a short period of time.
•	Don’t open the jig while the system is setting a flow or producing calibration data. You can break the flowmeter that is currently in use by doing this.
•	When the motor rams the jig it enters a state where it does not respond to any motion command, the solution is to change the setup file so it does not go so far, pull the cable from the back of the motor, plug it back in again and then restart the program. The light on the (redundant) motor controls becoming brighter is an indication of this error. Sometimes when this happens the motor has a broken idea of where the origin is meaning you must connect to the motor port (using termite is a good idea) and send the command   |,   then when the motor has found the origin again you can disconnect and continue.
•	If any flowmeter is removed then the number of the port that it was connected on must be set to -1 to tell the program it is not present anymore
•	The way I have written my program means that if the user attempts to set a flow that is below the minimum value included in the calibration curve  then the motor will probably ram the jig and the flow will be incorrectly set
•	One thing worth checking for if the valves aren’t working is: The channel for the weeder io in the setup file must be a capital letter.
•	My program switches the flow to a lower flowmeter when the mass flow measured by the current flowmeter is equal to or less than 95% of the maximum mass flow of the lower flowmeter. This means that if you want the system to reliably switch then the higher of the two flow meters must be able to reliably measure flows that small.
•	During calibration as the flow rate falls the program will adjust and make smaller increments,  the variable in the settings file defines how big the jumps are initially
•	I have set things up so that throughout the motor’s logic and my code 50,000 = 1 revolution of the motor
•	In the setup files you get a choice of which algorithm you want to use. Number one is the basic algorithm with no bounce protection, two is a similar algorithm with bounce protection. I recommend using number two for small flows. Number one is slightly faster though.

Notes to designer of hardware
•	There is no need for any micro switches/ springs to ensure safety, the motor cuts out if it hits something solid (for operation without cutting out the user must enter correct values for the distances in the setup file).
•	The small round stand on which the jig rests needs to be a bit taller to line up with the motor
•	Double sided sticky tape is required to hold the jig in a constant place
•	The small rod that runs in the groove to stop the motor turning has a habit of vibrating and producing an annoying sound (when the motor is stationary it constantly vibrates at about the same frequency)
•	The motor, the valves, the Alicats and the weeder io board all run happily on 24 volts DC. You can use the same power supply for all of them but note that the weeder io requires the power supply to be regulated

Notes to the next programmer to work on this
•	The idea is that the flowmaster class handles all of the hardware other then the motor and gives as simple interface as possible to the main program.
•	I need various waits in my program for the flow to stabilise. I have chosen to put these waits in my main program where possible. There is one wait while switching valves that needs to be in the flowmaster.

Limitations on speed and things that can be tuned for performance
•	My program works iteratively each time moving say 60% of the estimated distance to the estimated finish position and then re-estimating where the finish position is. The fraction that it moves each time can be set in the settings file by the user.
•	Between every motor movement and flow meter measurement there is a delay of “time_for_stabilization” number of milliseconds. The purpose of this is to allow the flow rate through the flow meter to reach equilibrium again before there is a measure of the new flow. At the moment this is the primary source of delay in setting the flow. There is a trade off because the shorter the user makes  “time_for_stabilization” the less accurate the reading and hence the lower fract2move must be to avoid overshooting.
•	I assume that production of calibration curve will be a rare occurrence and hence a one off time cost will be worth longer term accuracy savings. I suggest that  “time_for_stabilization” should be larger for producing the calibration curve (I used 800 ms) then when you are setting flows
•	When the motor is in position it then sends 8 characters to my program. when my program has received those 8 characters then the communication times out after 1 millisecond and my program continues. It could be redesigned so that the program continued immediately after the program received the first character and then wiped the buffer later in the program, this would give a very small increase in performance but is likely to cause problems
•	The program can handle the safe point being set anywhere before the point at which the desired flow rate is set.
•	If you set the safe point to be closer to the position at which the target flow is reached (but still before the target flow is reached) then you can get the system to do less iterations and hence save time
•	Currently if you set a flow of 0.15 slpm and then tell the system to set the same restrictor to 0.1 slpm then for the whole of the second setting process my program thinks it is close and goes slowly. If you want to do things like the above operation regularly then I could write an exception for cases like this.

Moving the code to the netbook for final use
•	To make the .exe portable to another machine I needed to include the below files

•	I needed to change the ports in the settings file to the appropriate local ones
•	I put termite on the net book for diagnostics
•	It seems that the drivers for the usb to rs232 conversion needed to be installed 8 times for the 8 com ports, this is now done

Motor
•	some useful pdf files should be attached to this document.
•	Cool works lite can be used for motor testing, the software is free from: http://www.rpmechatronics.co.uk/en/Downloads-Motors-And-Cables.
•	Cool works lite lies to the user about what it sends to and receives from the motor.
•	The max force deliverable by the motor is 220N
•	The torque reading the motor gives is not reliable.
•	For this project I am using dynamic mode commands only, no clever CML programs.
•	Motor programming help can be got from Tim Sharples: +44 (0) 1484  601008  who knows much about the motors
•	The command:   .1 sets the motor I am talking to
The K values are the motor’s settings, I set them to be:
•	K20=0 (baud rate= 38.4k)
•	K23=9 (disables echo (4th bit), enable status update on job finish(1st bit))
•	K37=100 ( full resolution of 50000 pulses per revolution and speed measurements in 1 pulse per second)
•	K42=1500 (this sets the return to origin speed, I want a return speed of 3 revolutions per second =3*50000 pulses per second, K42 has units of 100’s of pulses per second so I set K42 to have a value of 3*50000/100=1500)
•	K43=5000(this sets the return to origin acceleration, I set this to max)
•	K45=2 (this reverses my co-ordinates and causes it to search for the origin in the correct direction)
•	K46=0  (set origin with stopper, don’t search for bumper when turned on)
•	K47=10 (origin is detected when current reaches 10% of peak)
•	K48=0 (electrical origin is in the same place as physical origin)
•	K55=10 ( error tolerance for in position signal)
•	K57=1000 (the motor must be overloaded for 1000 milliseconds before giving an error)
•	K58=0 (this means the motor has no limits on its allowed position(no over run protection))
•	K59=0 (this means the motor has no limits on its allowed position(no over run protection))

Alicat
•	Alicats have internal registers that can be modified by rs232, ask the company for more details, as they don’t give out the manual as to how to do this to prevent users screwing things up. One use of this is polling them to find out what flow rates they can withstand.
•	 For their data to be recognised you must set the Alicats to 38400 baud and their unit id to: @
•	I supplied the power to one of my Alicats through the data cable because I had 3 Alicats and 2 power supplies. This worked fine.
•	Remember to tare the flow meters if they start diverging from the correct values

Weeder tech io
•	You need to make sure all of the switches on the io board are off. This means the board will be called ‘A’
•	As well as an rs232 connection a power supply is required. In the range 8 -30 volts.
•	The device runs at 9600 baud 8 data bits, 1 stop bit, no parity and no hardware or software flow control. Connect up with a straight through cable not null modem cable.
•	Outputs A and B seem to be broken on the board I am using for testing. They seem to output a permanently high voltage but when polled always claim to have a low voltage. I will not use them and I have taped over them on my board.
•	The board works ok with the free “ModCom” software and it works fine with termite as long as you remember to append all your lines with carriage returns
•	To a command of A£ the board will reply A? (that is; command not understood), this is useful for diagnostics

Valves
•	Note valve orientation, p should be towards the higher pressure side
•	When the valves experience a voltage they open and when there is no voltage they close.
•	Valves should have their third pin grounded
•	Documentation for the valves should be attached

Termite
•	There is a program called termite that is very useful for debugging anything RS232, I recommend it. Just be sure that you get carriage return/ line feed the right way round or some components will not work.

*/