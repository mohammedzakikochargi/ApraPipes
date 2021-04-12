#pragma once 
#include <string>
#include <unordered_map>
#include "Module.h"
#include "FrameUtils.h"

using namespace std;

class FileSequenceDriver;

class FileWriterModuleProps : public ModuleProps
{
public:
	FileWriterModuleProps(const string& _strFullFileNameWithPattern) : ModuleProps()
	{
		strFullFileNameWithPattern = _strFullFileNameWithPattern;
		append = false;
	}

	FileWriterModuleProps(const string& _strFullFileNameWithPattern, bool _append) : ModuleProps()
	{
		strFullFileNameWithPattern = _strFullFileNameWithPattern;
		append = _append;
	}

	string strFullFileNameWithPattern;
	bool append;
};

class FileWriterModule: public Module {
public:
	FileWriterModule(FileWriterModuleProps _props);
	virtual ~FileWriterModule();
	bool init();
	bool term();
protected:
	bool process(frame_container& frames);
	bool processSOS(frame_sp &frame);
	bool validateInputPins();
	bool shouldTriggerSOS();
private:
	boost::shared_ptr<FileSequenceDriver> mDriver;	
	FrameUtils::GetDataPtr mGetDataPtr;
};


