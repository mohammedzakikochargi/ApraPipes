#include "Module.h"
#include "Command.h"
#include "ControlModule.h"

class IFrameControlModuleProps : public ControlModuleProps
{
public:
	IFrameControlModuleProps() 
	{
		
	}
};

class IFrameControlModule : public ControlModule
{
public:
	IFrameControlModule(IFrameControlModuleProps _props);
	virtual ~IFrameControlModule();
	bool init();
	bool term();
protected:
	bool process(frame_container& frames);
	bool handleCommand(Command::CommandType type, frame_sp& fame);
private:
	IFrameControlModuleProps mProps;
	class Detail;
	boost::shared_ptr<Detail> mDetail;
};