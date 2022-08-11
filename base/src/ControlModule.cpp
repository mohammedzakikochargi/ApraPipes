#include "ControlModule.h"
#include "FrameMetadata.h"
#include "Frame.h"
#include "Logger.h"
#include "Utils.h"
#include "AIPExceptions.h"
#include "Command.h"


ControlModule::ControlModule(ControlModuleProps _props) : Module(CONTROL,"IFrame",_props),mProps(_props)
{

}

ControlModule::~ControlModule(){}

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