#include "ControlModule.h"
#include "FrameMetadata.h"
#include "Frame.h"
#include "Logger.h"
#include "Utils.h"
#include "AIPExceptions.h"
#include "Command.h"

class ControlModule::Detail
{
public:
	Detail(ControlModuleProps _props) : mProps(_props)
	{

	}

	~Detail() {}
public:
	ControlModuleProps mProps;
};

ControlModule::ControlModule(ControlModuleProps _props) : Module(CONTROL, "ControlModule", _props)
{
	mDeatil.reset(new Detail(_props));
}
ControlModule::~ControlModule() {}

bool ControlModule::init()
{
	if (!Module::init())
	{
		return false;
	}
	return true;
}

bool ControlModule::term()
{
	auto moduleRet = Module::term();

	return moduleRet;
}


bool ControlModule::process(frame_container& frames)
{
	return true;
}