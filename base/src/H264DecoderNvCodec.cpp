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
		helper.reset(new H264DecoderNvCodecHelper());
	}

	~Detail()
	{
		helper.reset();
	}

	bool setMetadata(framemetadata_sp& metadata, std::function<frame_sp(size_t)> makeFrame, std::function<void(frame_sp&, frame_sp&)> send)
	{
		if (metadata->getFrameType() == FrameMetadata::FrameType::H264_DATA)
		{
		}
		
		else
		{
			throw AIPException(AIP_NOTIMPLEMENTED, "Unknown frame type");
		}

		return helper->init( makeFrame, send);
	}

	bool compute(frame_sp& frame, frame_sp outFrame)
	{
		return helper->process(frame,outFrame);
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
	mOutputMetadata = boost::shared_ptr<FrameMetadata>(new RawImagePlanarMetadata(704, 576, ImageMetadata::YUV420, size_t(0), CV_8U, FrameMetadata::HOST));
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
	if (frameType != FrameMetadata::RAW_IMAGE_PLANAR)
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
	auto outputFrame = makeFrame();
	mDetail->compute(frame,outputFrame);

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