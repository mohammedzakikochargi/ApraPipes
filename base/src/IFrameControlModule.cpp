#include "IFrameControlModule.h"
#include "FrameMetadata.h"
#include "Frame.h"
#include "Logger.h"
#include "Utils.h"
#include "AIPExceptions.h"
#include "Command.h"

class IFrameControlModule::Detail
{
public:
	Detail(IFrameControlModuleProps _props) : mProps(_props)
	{

	}

	~Detail() {}

public:
	IFrameControlModuleProps mProps;
};
IFrameControlModule::IFrameControlModule(IFrameControlModuleProps _props) : ControlModule(_props)
{
}

IFrameControlModule::~IFrameControlModule() {}

bool IFrameControlModule::init()
{
	if (!Module::init())
	{
		return false;
	}
	return true;
}

bool IFrameControlModule::term()
{
	auto moduleRet = Module::term();

	return moduleRet;
}

bool IFrameControlModule::handleCommand(Command::CommandType type, frame_sp& frame)
{
	if (type == Command::CommandType::iFrame)
	{	
		return sendCommand(true, false);
	}		
	else
	{
		return Module::handleCommand(type, frame);
	}
}

bool IFrameControlModule::process(frame_container& frames)
{
	return true;
}