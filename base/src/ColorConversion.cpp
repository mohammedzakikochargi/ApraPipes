
#include "ColorConversion.h"
#include "FrameMetadata.h"
#include "FrameMetadataFactory.h"
#include "Frame.h"
#include "Logger.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include "opencv2/highgui.hpp"
#include "Utils.h"
#include "stdafx.h"


using namespace cv;
using namespace std;
class ColorConversion::Detail
{
public:
	Detail(ColorConversionProps &_props) :mProps(_props)
	{

	}
	~Detail() {}
	void initMatImages(framemetadata_sp& input)
	{
		iImg = Utils::getMatHeader(FrameMetadataFactory::downcast<RawImageMetadata>(input));
		oImg = Utils::getMatHeader(FrameMetadataFactory::downcast<RawImageMetadata>(mOutputMetadata));
	}

public:
	size_t mFrameLength;
	framemetadata_sp mOutputMetadata;
	std::string mOutputPinId;
	cv::Mat iImg;
	cv::Mat oImg;
	ColorConversionProps mProps;
};

ColorConversion::ColorConversion(ColorConversionProps _props) : Module(TRANSFORM, "RGB2MONO", _props), mProps(_props), mFrameType(FrameMetadata::GENERAL)
{
	mDetail.reset(new Detail(_props));
}

ColorConversion::~ColorConversion()
{}
bool ColorConversion::validateInputPins()
{
	if (getNumberOfInputPins() != 1)
	{
		LOG_ERROR << "<" << getId() << ">::validateInputPins size is expected to be 1. Actual<" << getNumberOfInputPins() << ">";
		return false;
	}
	framemetadata_sp metadata = getFirstInputMetadata();
	FrameMetadata::FrameType frameType = metadata->getFrameType();
	if (frameType != FrameMetadata::RAW_IMAGE)
	{
		LOG_ERROR << "<" << getId() << ">::validateInputPins input frameType is expected to be Raw_Image. Actual<" << frameType << ">";
		return false;
	}

	auto rawMetadata = FrameMetadataFactory::downcast<RawImageMetadata>(metadata);
	auto imageType = rawMetadata->getImageType();
	if (imageType != ImageMetadata::RGB && imageType != ImageMetadata::BGR)
	{
		LOG_ERROR << "<" << getId() << ">It is expected to be a RGB or BGR image. Actual<" << imageType << ">";
		return false;
	}
	return true;
}

bool ColorConversion::validateOutputPins()
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
		LOG_ERROR << "<" << getId() << ">::validateOutputPins input frameType is expected to be RAW_IMAGE. Actual<" << frameType << ">";
		return false;
	}
	auto rawMetadata = FrameMetadataFactory::downcast<RawImageMetadata>(metadata);
	auto imageType = rawMetadata->getImageType();
	if (imageType != ImageMetadata::MONO && imageType != ImageMetadata::RGB && imageType != ImageMetadata::BGR)
	{
		LOG_ERROR << "<" << getId() << ">Output Image is of incorrect type . Actual<" << imageType << ">";
		return false;
	}
	return true;
}

void ColorConversion::addInputPin(framemetadata_sp &metadata, string &pinId)
{
	Module::addInputPin(metadata, pinId);
	auto rawMetadata = FrameMetadataFactory::downcast<RawImageMetadata>(metadata);
	auto imageType = rawMetadata->getImageType();

	switch (mProps.colorchange)
	{
	case ColorConversionProps::colorconversion::RGBTOMONO:
		mDetail->mOutputMetadata = boost::shared_ptr<FrameMetadata>(new RawImageMetadata(rawMetadata->getWidth(), rawMetadata->getHeight(), ImageMetadata::ImageType::MONO, CV_8UC1, 0, CV_8U, FrameMetadata::HOST, true));
		break;
	case ColorConversionProps::colorconversion::BGRTOMONO:
		mDetail->mOutputMetadata = boost::shared_ptr<FrameMetadata>(new RawImageMetadata(rawMetadata->getWidth(), rawMetadata->getHeight(), ImageMetadata::ImageType::MONO, CV_8UC1, 0, CV_8U, FrameMetadata::HOST, true));
		break;
	case ColorConversionProps::colorconversion::BGRTORGB:
		mDetail->mOutputMetadata = boost::shared_ptr<FrameMetadata>(new RawImageMetadata(rawMetadata->getWidth(), rawMetadata->getHeight(), ImageMetadata::ImageType::RGB, CV_8UC3, 0, CV_8U, FrameMetadata::HOST, true));
		break;
	case ColorConversionProps::colorconversion::RGBTOBGR:
		mDetail->mOutputMetadata = boost::shared_ptr<FrameMetadata>(new RawImageMetadata(rawMetadata->getWidth(), rawMetadata->getHeight(), ImageMetadata::ImageType::BGR, CV_8UC3, 0, CV_8U, FrameMetadata::HOST, true));
		break;
	default:
		break;
	}
	mDetail->initMatImages(metadata);
	mDetail->mOutputMetadata->copyHint(*metadata.get());
	mDetail->mOutputPinId = addOutputPin(mDetail->mOutputMetadata);
}

std::string ColorConversion::addOutputPin(framemetadata_sp &metadata)
{
	return Module::addOutputPin(metadata);
}

bool ColorConversion::init()
{
	return Module::init();
}

bool ColorConversion::term()
{
	return Module::term();
}

bool ColorConversion::process(frame_container &frames)
{
	auto frame = getFrameByType(frames, FrameMetadata::RAW_IMAGE);
	if (isFrameEmpty(frame))
	{
		return true;
	}

	auto outFrame = makeFrame();

	mDetail->iImg.data = static_cast<uint8_t *>(frame->data());
	mDetail->oImg.data = static_cast<uint8_t *>(outFrame->data());

	framemetadata_sp metadata = getFirstInputMetadata();
	auto rawMetadata = FrameMetadataFactory::downcast<RawImageMetadata>(metadata);
	auto imageType = rawMetadata->getImageType();


	switch (mProps.colorchange)
	{
	case ColorConversionProps::colorconversion::RGBTOMONO:
		cv::cvtColor(mDetail->iImg, mDetail->oImg, cv::COLOR_RGB2GRAY);
		break;
	case ColorConversionProps::colorconversion::BGRTOMONO:
		cv::cvtColor(mDetail->iImg, mDetail->oImg, cv::COLOR_BGR2GRAY);
		break;
	case ColorConversionProps::colorconversion::BGRTORGB:
		cv::cvtColor(mDetail->iImg, mDetail->oImg, cv::COLOR_BGR2RGB);
		break;
	case ColorConversionProps::colorconversion::RGBTOBGR:
		cv::cvtColor(mDetail->iImg, mDetail->oImg, cv::COLOR_RGB2BGR);
		break;
	default:
		break;
	}
	frames.insert(make_pair(mDetail->mOutputPinId, outFrame));
	send(frames);
	return true;
}

void ColorConversion::setMetadata(framemetadata_sp &metadata)
{

}

bool ColorConversion::processSOS(frame_sp &frame)
{
	auto metadata = frame->getMetadata();
	setMetadata(metadata);
	return true;
}