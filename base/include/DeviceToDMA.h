#pragma once

#include "Module.h"
#include "CudaCommon.h"

#include <boost/pool/object_pool.hpp>

class DeviceToDMAProps : public ModuleProps
{
public:
	DeviceToDMAProps(int _maxConcurrentFrame=10)
	{
		maxConcurrentFrames = _maxConcurrentFrame;
	}
};

class DeviceToDMA : public Module
{

public:
	DeviceToDMA(DeviceToDMAProps _props);
	virtual ~DeviceToDMA();
	bool init();
	bool term();

protected:
	bool process(frame_container& frames);
	bool processSOS(frame_sp& frame);
	bool validateInputPins();
	bool validateOutputPins();
	void addInputPin(framemetadata_sp& metadata, string& pinId); // throws exception if validation fails		
	bool shouldTriggerSOS();
	bool processEOS(string& pinId);

private:
    void setMetadata(framemetadata_sp& metadata);
	int mInputFrameType;
	int mOutputFrameType;
	size_t mFrameLength;
	framemetadata_sp mOutputMetadata;
	std::string mOutputPinId;
	DeviceToDMAProps props;		
};