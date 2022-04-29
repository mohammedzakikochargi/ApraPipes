#pragma once
#include "FrameMetadata.h"
#include "Module.h"

class ColorConversionProps : public ModuleProps
{
public:
	enum colorconversion
	{
		RGBTOMONO = 0,
		BGRTOMONO = 1,
		BGRTORGB = 2,
		RGBTOBGR = 3
	};
	ColorConversionProps(colorconversion coc) : ModuleProps()
	{
		colorchange = coc;
	}
	colorconversion colorchange;
};
class ColorConversion : public Module
{

public:

	ColorConversion(ColorConversionProps _props);
	virtual ~ColorConversion();
	bool init();
	bool term();

protected:
	bool process(frame_container& frames);
	bool processSOS(frame_sp& frame);
	bool validateInputPins();
	bool validateOutputPins();
	void addInputPin(framemetadata_sp& metadata, string& pinId);
	std::string addOutputPin(framemetadata_sp &metadata);

private:
	void setMetadata(framemetadata_sp& metadata);
	int mFrameType;
	ColorConversionProps mProps;
	class Detail;
	boost::shared_ptr<Detail> mDetail;
	framemetadata_sp mOutputMetadata;
	std::string mOutputPinId;
};

