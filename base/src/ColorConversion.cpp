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
#include "AbsColorConversionFactory.h"
#include "ColorConversionStrategy.h"

using namespace cv;
using namespace std;


ColorConversion::ColorConversion(ColorConversionProps _props) : Module(TRANSFORM, "ColorConversion", _props), mProps(_props), mFrameType(FrameMetadata::GENERAL)
{
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
	if (frameType != FrameMetadata::RAW_IMAGE && frameType != FrameMetadata::RAW_IMAGE_PLANAR)
	{
		LOG_ERROR << "<" << getId() << ">::validateInputPins input frameType is expected to be Raw_Image. Actual<" << frameType << ">";
		return false;
	}
	if (frameType == FrameMetadata::RAW_IMAGE)
	{
		auto rawMetadata = FrameMetadataFactory::downcast<RawImageMetadata>(metadata);
		auto imageType = rawMetadata->getImageType();
		if (imageType != ImageMetadata::RGB && imageType != ImageMetadata::BGR && imageType != ImageMetadata::BAYERBG10 && imageType != ImageMetadata::BAYERBG8 && imageType != ImageMetadata::BAYERGB8 && imageType != ImageMetadata::BAYERGR8 && imageType != ImageMetadata::BAYERRG8)
		{
			LOG_ERROR << "<" << getId() << ">It is expected to be a RGB or BGR image. Actual<" << imageType << ">";
			return false;
		}
	}
	else if (frameType == FrameMetadata::RAW_IMAGE_PLANAR)
	{
		auto rawMetadata = FrameMetadataFactory::downcast<RawImagePlanarMetadata>(metadata);
		auto imageType = rawMetadata->getImageType();
		if (imageType != ImageMetadata::YUV420)
		{
			LOG_ERROR << "<" << getId() << ">Output Image is of incorrect type . Actual<" << imageType << ">";
			return false;
		}
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
	if ((frameType != FrameMetadata::RAW_IMAGE) && (frameType != FrameMetadata::RAW_IMAGE_PLANAR))
	{
		LOG_ERROR << "<" << getId() << ">::validateOutputPins input frameType is expected to be RAW_IMAGE. Actual<" << frameType << ">";
		return false;
	}
	if (frameType == FrameMetadata::RAW_IMAGE)
	{
		auto rawMetadata = FrameMetadataFactory::downcast<RawImageMetadata>(metadata);
		auto imageType = rawMetadata->getImageType();
		if (imageType != ImageMetadata::MONO && imageType != ImageMetadata::RGB && imageType != ImageMetadata::BGR && imageType != ImageMetadata::BAYERBG8)
		{
			LOG_ERROR << "<" << getId() << ">Output Image is of incorrect type . Actual<" << imageType << ">";
			return false;
		}
	}
	else if (frameType == FrameMetadata::RAW_IMAGE_PLANAR)
	{
		auto rawMetadata = FrameMetadataFactory::downcast<RawImagePlanarMetadata>(metadata);
		auto imageType = rawMetadata->getImageType();
		if (imageType != ImageMetadata::YUV420)
		{
			LOG_ERROR << "<" << getId() << ">Output Image is of incorrect type . Actual<" << imageType << ">";
			return false;
		}
	}
	return true;
}

void ColorConversion::setConversionStrategy(framemetadata_sp inputMetadata,framemetadata_sp outputMetadata)
{
	mDetail = AbsColorConversionFactory::create(inputMetadata, outputMetadata);
}

void ColorConversion::addInputPin(framemetadata_sp& metadata, string& pinId)
{
	Module::addInputPin(metadata, pinId);
	auto inputFrameType = metadata->getFrameType();

	if (inputFrameType == FrameMetadata::RAW_IMAGE)
	{
		auto rawMetadata = FrameMetadataFactory::downcast<RawImageMetadata>(metadata);
		mWidth = rawMetadata->getWidth();
		mHeight = rawMetadata->getHeight();
		auto imageType = rawMetadata->getImageType();
	}
	else if (inputFrameType == FrameMetadata::RAW_IMAGE_PLANAR)
	{
		auto rawPlanarMetadata = FrameMetadataFactory::downcast<RawImagePlanarMetadata>(metadata);
		mWidth = rawPlanarMetadata->getWidth(0);
		mHeight = rawPlanarMetadata->getHeight(0);
		auto imageType = rawPlanarMetadata->getImageType();
	}

	switch (mProps.colorchange)
	{
	case ColorConversionProps::colorconversion::RGB_2_MONO:
	case ColorConversionProps::colorconversion::BGR_2_MONO:
	case ColorConversionProps::colorconversion::BAYERBG8_2_MONO:
		mOutputMetadata = boost::shared_ptr<FrameMetadata>(new RawImageMetadata(mWidth, mHeight, ImageMetadata::MONO, CV_8UC1, 0, CV_8U, FrameMetadata::HOST, true));
		break;
	case ColorConversionProps::colorconversion::BGR_2_RGB:
	case ColorConversionProps::colorconversion::BAYERBG8_2_RGB:
	case ColorConversionProps::colorconversion::YUV420PLANAR_2_RGB:
	case ColorConversionProps::colorconversion::BAYERGB8_2_RGB:
	case ColorConversionProps::colorconversion::BAYERGR8_2_RGB:
	case ColorConversionProps::colorconversion::BAYERRG8_2_RGB:
		mOutputMetadata = boost::shared_ptr<FrameMetadata>(new RawImageMetadata(mWidth, mHeight, ImageMetadata::RGB, CV_8UC3, 0, CV_8U, FrameMetadata::HOST, true));
		break;
	case ColorConversionProps::colorconversion::RGB_2_BGR:
		mOutputMetadata = boost::shared_ptr<FrameMetadata>(new RawImageMetadata(mWidth, mHeight, ImageMetadata::BGR, CV_8UC3, 0, CV_8U, FrameMetadata::HOST, true));
		break;
	case ColorConversionProps::colorconversion::RGB_2_YUV420PLANAR:
		mOutputMetadata = boost::shared_ptr<FrameMetadata>(new RawImagePlanarMetadata(mWidth, mHeight, ImageMetadata::YUV420, size_t(0), CV_8U, FrameMetadata::HOST));
		break;
	default:
		break;
	}
	mOutputMetadata->copyHint(*metadata.get());
	mOutputPinId = addOutputPin(mOutputMetadata);

	setConversionStrategy(metadata,mOutputMetadata);

}

std::string ColorConversion::addOutputPin(framemetadata_sp& metadata)
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

bool ColorConversion::process(frame_container& frames)
{
	auto outFrame = makeFrame();
	mDetail->doColorConversion(frames, outFrame, mOutputMetadata);
	frames.insert(make_pair(mOutputPinId, outFrame));
	send(frames);
	return true;
}

bool ColorConversion::processSOS(frame_sp& frame)
{
	return true;
}