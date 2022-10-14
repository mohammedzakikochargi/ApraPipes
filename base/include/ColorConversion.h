#pragma once
#include "FrameMetadata.h"
#include "Module.h"
#include "AbsColorConversionFactory.h"

class ColorConversionProps : public ModuleProps
{
public:
	enum colorconversion
	{
		RGB_2_MONO = 0,
		BGR_2_MONO = 1,
		BGR_2_RGB = 2,
		RGB_2_BGR = 3,
		BAYERBG8_2_MONO = 4,
		RGB_2_YUV420PLANAR = 5,
		YUV420PLANAR_2_RGB = 6,
		BAYERBG8_2_RGB = 7,
		BAYERGB8_2_RGB = 8,
		BAYERRG8_2_RGB = 9,
		BAYERGR8_2_RGB 
	};
	ColorConversionProps(colorconversion coc) : ModuleProps()
	{
		colorchange = coc;
	}
	ColorConversionProps() : ModuleProps()
	{}
	colorconversion colorchange;
};

//class DetailAbstract;

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
	void setConversionStrategy(framemetadata_sp inputMetadata, framemetadata_sp outputMetadata);
	void addInputPin(framemetadata_sp& metadata, string& pinId);
	std::string addOutputPin(framemetadata_sp &metadata);

private:
	void setMetadata(framemetadata_sp& metadata);
	int mFrameType;
	ColorConversionProps mProps;
	boost::shared_ptr<DetailAbstract> mDetail;
	std::string mOutputPinId;
	uint16_t mWidth;
	uint16_t mHeight;
	framemetadata_sp mOutputMetadata;
};

