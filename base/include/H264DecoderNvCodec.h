#pragma once

#include "Module.h"
#include <cuda_runtime_api.h>
#include "CudaCommon.h"

class H264DecoderNvCodecProps : public ModuleProps
{
public:
	H264DecoderNvCodecProps() {}
};

class H264DecoderNvCodec : public Module
{

public:
	H264DecoderNvCodec(H264DecoderNvCodecProps _props);
	virtual ~H264DecoderNvCodec();
	bool init();
	bool term();

protected:
	bool process(frame_container& frames);
	bool processSOS(frame_sp& frame);
	bool validateInputPins();
	bool validateOutputPins();
	bool shouldTriggerSOS();
	bool processEOS(string& pinId);

private:
	class Detail;
	boost::shared_ptr<Detail> mDetail;

	bool mShouldTriggerSOS;
	framemetadata_sp mOutputMetadata;
	std::string mInputPinId;
	std::string mOutputPinId;

	H264DecoderNvCodecProps props;
};
