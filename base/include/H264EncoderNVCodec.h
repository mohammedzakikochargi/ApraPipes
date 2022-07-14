#pragma once

#include "Module.h"
#include <cuda_runtime_api.h>
#include "CudaCommon.h"

class H264EncoderNVCodecProps : public ModuleProps
{
public:
	H264EncoderNVCodecProps(apracucontext_sp& _cuContext): targetKbps(0), cuContext(_cuContext)
	{
		
	}

	uint32_t targetKbps;
	apracucontext_sp cuContext;
};

class H264EncoderNVCodec : public Module
{

public:
	H264EncoderNVCodec(H264EncoderNVCodecProps _props);
	virtual ~H264EncoderNVCodec();
	bool init();
	bool term();

	bool getSPSPPS(void*& buffer, size_t& size, int& width, int& height);

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

	H264EncoderNVCodecProps props;
};
