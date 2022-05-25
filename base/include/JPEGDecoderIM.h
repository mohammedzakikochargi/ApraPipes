#pragma once

#include "Module.h"

class JPEGDecoderIMProps : public ModuleProps
{
public:
	JPEGDecoderIMProps(bool _sw = false) : ModuleProps()
	{
		sw = _sw;
	}

	bool sw;
};

class JPEGDecoderIM : public Module
{

public:
	JPEGDecoderIM(JPEGDecoderIMProps _props = JPEGDecoderIMProps());
	virtual ~JPEGDecoderIM();
	bool init();
	bool term();

protected:
	bool process(frame_container& frames);
	bool processSOS(frame_sp& frame);
	bool validateInputPins();
	bool validateOutputPins();

private:
	class Detail;
	boost::shared_ptr<Detail> mDetail;
};