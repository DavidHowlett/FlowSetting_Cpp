//---------------------------------------------------------------------------

#ifndef FunctionsH
#define FunctionsH
#include <System.hpp>
//---------------------------------------------------------------------------
//int GetHolePosition(int HoleNo, double HoleDiameter);
int  FindCharInBuffer(AnsiString buffer, char SoughtCharacter);                   //Find where SoughtCharacter is in buffer
void GetHolePosition (double HoleNo, double HoleDiameter, double *x, double *y); //Where to burn next hole
char* uCase(char *buffer);
extern char Ux39UjK[];                                     //Carriage Return String because I can't do this in AnsiString;
extern AnsiString CRString;                               //Make "CRString" have the right value
#endif
