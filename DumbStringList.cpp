//---------------------------------------------------------------------------


#pragma hdrstop

#include "DumbStringList.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)
//******************************************************************************
TDumbStringList::TDumbStringList(){
//******************************************************************************
	List=new TList();
}

//******************************************************************************
TDumbStringList::~TDumbStringList(){
//******************************************************************************
	Clear();
	FreeAndNil(List);
}

//******************************************************************************
void TDumbStringList::Add(String Text){
//******************************************************************************
	String *Data=new String(Text);
	List->Add(Data);
}

//******************************************************************************
void TDumbStringList::Delete(int Index){
//******************************************************************************
	String *Data=(String*)List->Items[Index];
	List->Delete(Index);
	delete Data;
}

//******************************************************************************
void TDumbStringList::Clear(){
//******************************************************************************
	while(List->Count>0){
		Delete(0);
	}
}

//******************************************************************************
String TDumbStringList::Get(int Index){
//******************************************************************************
	return *(String*)(List->Items[Index]);
}

//******************************************************************************
void TDumbStringList::Set(int Index,String Text){
//******************************************************************************
	(*(String*)(List->Items[Index]))=Text;
}

//******************************************************************************
int TDumbStringList::Count(){
//******************************************************************************
	return List->Count;
}