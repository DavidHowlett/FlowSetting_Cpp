//---------------------------------------------------------------------------


#pragma hdrstop

#include "Functions.h"
#include <Math.h>
#include <stdlib.h>
#include <string.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)

char Ux39UjK[]={13,0};
AnsiString CRString=Ux39UjK;

//******************************************************************************
int FindCharInBuffer(char *buffer, char SoughtCharacter)
//Seek "SoughtCharacter" in buffer. Return -1 for not found, or position.
//******************************************************************************
{
	for (unsigned int i=0; i < strlen(buffer); i++) {
		if (buffer[i]==SoughtCharacter) {
			return i;
		}
	}
	return -1; //Not found, despite reaching the end of the buffer
}

//Identify where to burn a new hole with the laser.
//******************************************************************************
void GetHolePosition(double HoleNo,   //How many holes precede this one in the sequence?
					double HoleDiameter,      //How bid does the laser make the holes?
					double *x,                //What is the x coordinate of the proposed hole?
					double *y)                //What is the y coordinate of the proposed hole?
//Notice that HoleNo is a double not an integer- sometimes we don't want to
//burn complete holes, but want to "nibble" round the edges of other holes by
//burning "hole" 3.11, for example. As this is a semi-continuous spiral this is easier.
//******************************************************************************
{
	double theta ;  //How far round?
	double radius;  //How far from the centre?
	theta = (double) (4.0 * sqrt(HoleNo));
	radius= (double) (0.1 * HoleDiameter * theta);
	*x    = (double) (radius * cos (theta));
	*y    = (double) (radius * sin (theta));
}

//******************************************************************************
char* uCase(char *buffer)
//return uppercase(buffer): Silly default command set doesn't seem to support this.
//******************************************************************************
{
	char* str;
	unsigned int lenstr;
	lenstr=strlen(buffer);
	str=(char*)malloc(lenstr+1);
	strcpy(str,buffer);
	for (unsigned int i=0; i < lenstr; i++) {
		if ((str[i]>='a') &&(str[i]<='z')) {
			str[i]=str[i]+('A'-'a');
		}
	}
	return str;
}

