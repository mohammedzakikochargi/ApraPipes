#include "HostDMA.h"
#include "FrameMetadata.h"
#include "Frame.h"
#include "Logger.h"
#include "Utils.h"
#include "AIPExceptions.h"
#include "DMAFDWrapper.h"

HostDMA::HostDMA(HostDMAProps _props) : Module(TRANSFORM, "HostDMA", _props), props(_props), mFrameLength(0)
{
}

HostDMA::~HostDMA() {}

bool HostDMA::validateInputPins()
{
	if (getNumberOfInputPins() != 1)
	{
		LOG_ERROR << "<" << getId() << ">::validateInputPins size is expected to be 1. Actual<" << getNumberOfInputPins() << ">";
		return false;
	}

	framemetadata_sp metadata = getFirstInputMetadata();

	FrameMetadata::MemType memType = metadata->getMemType();
	if (memType != FrameMetadata::MemType::HOST)
	{
		LOG_ERROR << "<" << getId() << ">::validateInputPins input memType is expected to be HOST. Actual<" << memType << ">";
		return false;
	}

	return true;
}

bool HostDMA::validateOutputPins()
{
	if (getNumberOfOutputPins() != 1)
	{
		LOG_ERROR << "<" << getId() << ">::validateOutputPins size is expected to be 1. Actual<" << getNumberOfOutputPins() << ">";
		return false;
	}

	framemetadata_sp metadata = getFirstOutputMetadata();
	mOutputFrameType = metadata->getFrameType();

	FrameMetadata::MemType memType = metadata->getMemType();
	if (memType != FrameMetadata::MemType::DMABUF)
	{
		LOG_ERROR << "<" << getId() << ">::validateOutputPins input memType is expected to be DMABUF. Actual<" << memType << ">";
		return false;
	}

	return true;
}

void HostDMA::addInputPin(framemetadata_sp &metadata, string &pinId)
{
	Module::addInputPin(metadata, pinId);
	auto inputRawMetadata = FrameMetadataFactory::downcast<RawImagePlanarMetadata>(metadata);
	mOutputMetadata = framemetadata_sp(new RawImagePlanarMetadata(FrameMetadata::MemType::DMABUF));
	mOutputPinId = addOutputPin(mOutputMetadata);
}

bool HostDMA::init()
{
	if (!Module::init())
	{
		return false;
	}

	return true;
}

bool HostDMA::term()
{
	return Module::term();
}

bool HostDMA::process(frame_container &frames)
{
	auto frame = frames.cbegin()->second;
	auto outFrame = makeFrame(mFrameLength);
	auto outBuffer = static_cast<DMAFDWrapper *>(outFrame->data())->getHostPtr();
	memcpy(outBuffer, frame->data(), frame->size());
	frames.insert(make_pair(mOutputPinId, outFrame));
	send(frames);
	return true;
}

bool HostDMA::processSOS(frame_sp &frame)
{
	auto metadata = frame->getMetadata();
	setMetadata(metadata);
	return true;
}

void HostDMA::setMetadata(framemetadata_sp &metadata)
{
	mInputFrameType = metadata->getFrameType();

	auto rawPlanarMetadata = FrameMetadataFactory::downcast<RawImagePlanarMetadata>(metadata);
	auto width = rawPlanarMetadata->getWidth(0);
	auto height = rawPlanarMetadata->getHeight(0);
	// auto type = rawPlanarMetadata->getType();
	auto depth = rawPlanarMetadata->getDepth();
	auto inputImageType = rawPlanarMetadata->getImageType();

	auto rawOutMetadata = FrameMetadataFactory::downcast<RawImagePlanarMetadata>(mOutputMetadata);
	RawImagePlanarMetadata outputMetadata(width, height, inputImageType, size_t(0), depth,  FrameMetadata::MemType::DMABUF);
	rawOutMetadata->setData(outputMetadata);

	mFrameLength = mOutputMetadata->getDataSize();
}

bool HostDMA::shouldTriggerSOS()
{
	return mFrameLength == 0;
}

bool HostDMA::processEOS(string &pinId)
{
	mFrameLength = 0;
	return true;
}