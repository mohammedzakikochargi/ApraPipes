#pragma once
#include "Module.h"
#include "Command.h"
#include <map>

class ControlModuleProps : public ModuleProps
{
public:
	ControlModuleProps() : ModuleProps()
	{
		
	}
	void addCommandtarget(Command::CommandType mType, boost::shared_ptr<Module> mControl)
	{
		std::map<Command::CommandType, std::vector<boost::shared_ptr<Module> >>::iterator itr = mControlMap.find(mType);
		if (itr != mControlMap.end())
		{
			mControlMap[mType].push_back(mControl);
		}
		else
		{
			std::vector<boost::shared_ptr<Module>> vec = { mControl };
			mControlMap.insert(std::make_pair(mType, vec));
		}
	}
	std::map<Command::CommandType, std::vector<boost::shared_ptr<Module> >> mControlMap;
};

class ControlModule : public Module
{
public:
	ControlModule(ControlModuleProps _props);
	virtual ~ControlModule();
	//void addCommandtarget(Command::CommandType mType, boost::shared_ptr<Module> mControl);
	bool init();
	bool term();
protected:
	bool process(frame_container& frames);
	ControlModuleProps mProps;
private:
	class Detail;
	boost::shared_ptr<Detail> mDeatil;
	std::map<Command::CommandType, std::vector<boost::shared_ptr<Module> >> mControlMap; 
};