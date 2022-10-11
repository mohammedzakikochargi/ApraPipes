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
		RGBTOBGR = 3,
		BAYERTOMONO = 4,
		RGBTOYUV420 = 5,
		YUV420TORGB
	};
	ColorConversionProps(colorconversion coc) : ModuleProps()
	{
		colorchange = coc;
	}
	ColorConversionProps() : ModuleProps()
	{}
	colorconversion colorchange;
};

class DetailAbs;

class ColorConversion : public Module
{

public:

	ColorConversion(ColorConversionProps _props);
	virtual ~ColorConversion();
	bool bayerToMono(frame_sp& frame);
	bool init();
	bool term();

protected:
	bool process(frame_container& frames);
	bool processSOS(frame_sp& frame);
	
	bool validateInputPins();
	bool validateOutputPins();
	void setConversionStrategy(framemetadata_sp metadata);
	void addInputPin(framemetadata_sp& metadata, string& pinId);
	std::string addOutputPin(framemetadata_sp &metadata);

private:
	void setMetadata(framemetadata_sp& metadata);
	int mFrameType;
	ColorConversionProps mProps;
	boost::shared_ptr<DetailAbs> mDetail;
	framemetadata_sp mOutputMetadata;
	std::string mOutputPinId;
	uint16_t mWidth;
	uint16_t mHeight;
	uint16_t mStep;
	RawImageMetadata* rawMetadata;
	RawImagePlanarMetadata* rawPlanarMetadata;
};

