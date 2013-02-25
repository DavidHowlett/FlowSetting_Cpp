//---------------------------------------------------------------------------

#ifndef DumbStringListH
#define DumbStringListH
#include <classes.hpp>
//---------------------------------------------------------------------------
// Because TStringList is not Dumb enough
class TDumbStringList{
	private:
		TList *List;
	public:
		TDumbStringList();
		~TDumbStringList();
		void Add(String Text);
		void Delete(int Index);
		void Clear();
		String Get(int Index);
		void Set(int Index,String Text);
		int Count();
};
#endif
