#ifndef FlowSetterSettingsH
#define FlowSetterSettingsH
#include <stdio.h>
class SettingsFileManager
{
	// the purpose of this class is to store and manage all settings that must be stored in the settings file
	// I do this to aid encapsulation
	// rule for use of this settings file manager: whenever a setting is changed by the user of this class the write_file() method must be called as well.
	// I could have enforced this by forceing the modification of variables to happen through method calls but I decided against this due to the extra code required.
	public: // functions
		SettingsFileManager();
		~SettingsFileManager();
		int 	ReadFile();
		int 	WriteFile();
	public: // variables
		int		LatestGoodFlowmeterPort;
		int 	DataGatheringCycleTime; // this is in milliseconds
		int 	LastRawDataFileNumber;
		int 	LastProcessedDataFileNumber;
		float TriggerFlow; // this is the flow rate in standard litres per minute at which the software recognises that a pulse of air has started
	private:
		FILE * FilePointer;
		static const int MaxSettings = 10;
		static const int MaxCharsInSettingDescriptor = 500;
		char 	SettingsDescriptors[MaxSettings][MaxCharsInSettingDescriptor];
	private:
		int 	UnguardedReadFile();
		int 	UnguardedWriteFile();
};
#endif
