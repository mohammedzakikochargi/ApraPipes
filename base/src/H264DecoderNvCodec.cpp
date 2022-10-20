#include "H264DecoderNvCodec.h"
#include "H264DecoderNvCodecHelper.h"
#include "FrameMetadata.h"
#include "H264Metadata.h"
#include "Frame.h"
#include "Logger.h"
#include "Utils.h"
#include "AIPExceptions.h"

#include "nvjpeg.h"


class H264DecoderNvCodec::Detail
{
public:
	Detail(H264DecoderNvCodecProps& _props)
	{
		auto myWidth = 704;
		auto myHeight = 576;
		CUcontext cuContext = 0;
		bool bUseDeviceFrame = false;
		cudaVideoCodec eCodec = cudaVideoCodec_H264;
		std::mutex* pMutex = NULL;
		bool bLowLatency = false;
		bool bDeviceFramePitched = false;
		helper.reset(new H264DecoderNvCodecHelper(cuContext, myWidth, myHeight, bUseDeviceFrame, eCodec, pMutex, bLowLatency, bDeviceFramePitched, myWidth, myHeight));
	}

	~Detail()
	{
		helper.reset();
	}

	bool setMetadata(framemetadata_sp& metadata, std::function<frame_sp(size_t)> makeFrame, std::function<void(frame_sp&, frame_sp&)> send)
	{
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t pitch = 0;
		ImageMetadata::ImageType imageType = ImageMetadata::UNSET;

		if (metadata->getFrameType() == FrameMetadata::FrameType::H264_DATA)
		{
			
			width = 704;
			height = 576;
		}
		
		else
		{
			throw AIPException(AIP_NOTIMPLEMENTED, "Unknown frame type");

		}
		//helper->reset();

		return helper->init(width, height, makeFrame, send);
	}

	bool compute(frame_sp& frame)
	{
		return helper->process(frame);
	}

	void endEncode()
	{
		return helper->endDecode();
	}


private:
	boost::shared_ptr<H264DecoderNvCodecHelper> helper;

};

H264DecoderNvCodec::H264DecoderNvCodec(H264DecoderNvCodecProps _props) : Module(TRANSFORM, "H264DecoderNvCodec", _props), mShouldTriggerSOS(true), props(_props)
{
	mDetail.reset(new Detail(props));
	mOutputMetadata = framemetadata_sp(new FrameMetadata(FrameMetadata::FrameType::RAW_IMAGE));
	mOutputPinId = addOutputPin(mOutputMetadata);
}

H264DecoderNvCodec::~H264DecoderNvCodec() {}

bool H264DecoderNvCodec::validateInputPins()
{
	if (getNumberOfInputPins() != 1)
	{
		LOG_ERROR << "<" << getId() << ">::validateInputPins size is expected to be 1. Actual<" << getNumberOfInputPins() << ">";
		return false;
	}

	framemetadata_sp metadata = getFirstInputMetadata();
	FrameMetadata::FrameType frameType = metadata->getFrameType();
	if (frameType!= FrameMetadata::FrameType::H264_DATA)
	{
		LOG_ERROR << "<" << getId() << ">::validateInputPins input frameType is expected to be RAW_IMAGE or RAW_IMAGE_PLANAR. Actual<" << frameType << ">";
		return false;
	}

	FrameMetadata::MemType memType = metadata->getMemType();
	if (memType != FrameMetadata::MemType::HOST)
	{
		LOG_ERROR << "<" << getId() << ">::validateInputPins input memType is expected to be CUDA_DEVICE. Actual<" << memType << ">";
		return false;
	}

	mInputPinId = getInputPinIdByType(frameType);

	return true;
}

bool H264DecoderNvCodec::validateOutputPins()
{
	if (getNumberOfOutputPins() != 1)
	{
		LOG_ERROR << "<" << getId() << ">::validateOutputPins size is expected to be 1. Actual<" << getNumberOfOutputPins() << ">";
		return false;
	}

	framemetadata_sp metadata = getFirstOutputMetadata();
	FrameMetadata::FrameType frameType = metadata->getFrameType();
	if (frameType != FrameMetadata::RAW_IMAGE)
	{
		LOG_ERROR << "<" << getId() << ">::validateOutputPins input frameType is expected to be H264_DATA. Actual<" << frameType << ">";
		return false;
	}

	return true;
}

bool H264DecoderNvCodec::init()
{
	if (!Module::init())
	{
		return false;
	}

	return true;
}

bool H264DecoderNvCodec::term()
{
	mDetail->endEncode();
	mDetail.reset();

	return Module::term();
}

bool H264DecoderNvCodec::process(frame_container& frames)
{
	auto frame = frames.cbegin()->second;

	mDetail->compute(frame);

	return true;
}

bool H264DecoderNvCodec::processSOS(frame_sp& frame)
{
	mDetail->setMetadata(frame->getMetadata(),
		[&](size_t size) -> frame_sp {return makeFrame(size,mOutputPinId); },
		[&](frame_sp& inputFrame, frame_sp& outputFrame) {

			outputFrame->setMetadata(mOutputMetadata);

			frame_container frames;
			frames.insert(make_pair(mInputPinId, inputFrame));
			frames.insert(make_pair(mOutputPinId, outputFrame));
			send(frames);
		}
	);
	mShouldTriggerSOS = false;

	return true;
}

bool H264DecoderNvCodec::shouldTriggerSOS()
{
	return mShouldTriggerSOS;
}

bool H264DecoderNvCodec::processEOS(string& pinId)
{
	mShouldTriggerSOS = true;
	return true;
}