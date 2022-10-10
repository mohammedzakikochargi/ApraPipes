
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
#include <opencv2\core\types_c.h>
#include <opencv2/core/core_c.h>
#include "opencv2/core/types_c.h"
#include "opencv2/core/utility.hpp"


using namespace cv;
using namespace std;
class ColorConversion::Detail
{
public:
	Detail(ColorConversionProps& _props) :mProps(_props)
	{

	}
	~Detail() {}
	void initMatImages(framemetadata_sp& input)
	{
		//auto metadata = FrameMetadataFactory::downcast<RawImagePlanarMetadata>(input);
		iImg = Utils::getMatHeader(FrameMetadataFactory::downcast<RawImagePlanarMetadata>(input));
		oImg = Utils::getMatHeader(FrameMetadataFactory::downcast<RawImageMetadata>(mOutputMetadata));
	}
public:
	framemetadata_sp mOutputMetadata;
	std::string mOutputPinId;
	cv::Mat iImg;
	cv::Mat oImg;
	boost::shared_ptr<ColorConversion> Props;
	ColorConversionProps mProps;
};

ColorConversion::ColorConversion(ColorConversionProps _props) : Module(TRANSFORM, "ColorConversion", _props), mProps(_props), mFrameType(FrameMetadata::GENERAL)
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
	if (frameType != FrameMetadata::RAW_IMAGE && frameType != FrameMetadata::RAW_IMAGE_PLANAR)
	{
		LOG_ERROR << "<" << getId() << ">::validateInputPins input frameType is expected to be Raw_Image. Actual<" << frameType << ">";
		return false;
	}
	if (frameType == FrameMetadata::RAW_IMAGE)
	{
		auto rawMetadata = FrameMetadataFactory::downcast<RawImageMetadata>(metadata);
		auto imageType = rawMetadata->getImageType();
		if (imageType != ImageMetadata::RGB && imageType != ImageMetadata::BGR && imageType != ImageMetadata::BG10)
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
		auto imageType = rawMetadata->getFrameType();
		if (imageType != ImageMetadata::MONO && imageType != ImageMetadata::RGB && imageType != ImageMetadata::BGR)
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

void ColorConversion::addInputPin(framemetadata_sp& metadata, string& pinId)
{
	Module::addInputPin(metadata, pinId);
	inputFrameType = metadata->getFrameType();

	if (inputFrameType == FrameMetadata::RAW_IMAGE)
	{
		rawMetadata = FrameMetadataFactory::downcast<RawImageMetadata>(metadata);
		mWidth = rawMetadata->getWidth();
		mHeight = rawMetadata->getHeight();
		mStep = rawMetadata->getStep();
		auto imageType = rawMetadata->getImageType();
	}
	else if (inputFrameType == FrameMetadata::RAW_IMAGE_PLANAR)
	{
		rawPlanarMetadata = FrameMetadataFactory::downcast<RawImagePlanarMetadata>(metadata);
		mWidth = rawPlanarMetadata->getWidth(0);
		mHeight = rawPlanarMetadata->getHeight(0);
		mStep = rawPlanarMetadata->getStep(0);
		auto imageType = rawPlanarMetadata->getImageType();
	}

	switch (mProps.colorchange)
	{
	case ColorConversionProps::colorconversion::RGBTOMONO:
		mDetail->mOutputMetadata = boost::shared_ptr<FrameMetadata>(new RawImageMetadata(rawMetadata->getWidth(), rawMetadata->getHeight(), ImageMetadata::ImageType::MONO, CV_8UC1, 0, CV_8U, FrameMetadata::HOST, true));
		mDetail->initMatImages(metadata);
		break;
	case ColorConversionProps::colorconversion::BGRTOMONO:
		mDetail->mOutputMetadata = boost::shared_ptr<FrameMetadata>(new RawImageMetadata(rawMetadata->getWidth(), rawMetadata->getHeight(), ImageMetadata::ImageType::MONO, CV_8UC1, 0, CV_8U, FrameMetadata::HOST, true));
		mDetail->initMatImages(metadata);
		break;
	case ColorConversionProps::colorconversion::BGRTORGB:
		mDetail->mOutputMetadata = boost::shared_ptr<FrameMetadata>(new RawImageMetadata(rawMetadata->getWidth(), rawMetadata->getHeight(), ImageMetadata::ImageType::RGB, CV_8UC3, 0, CV_8U, FrameMetadata::HOST, true));
		mDetail->initMatImages(metadata);
		break;
	case ColorConversionProps::colorconversion::RGBTOBGR:
		mDetail->mOutputMetadata = boost::shared_ptr<FrameMetadata>(new RawImageMetadata(rawMetadata->getWidth(), rawMetadata->getHeight(), ImageMetadata::ImageType::BGR, CV_8UC3, 0, CV_8U, FrameMetadata::HOST, true));
		mDetail->initMatImages(metadata);
		break;
	case ColorConversionProps::colorconversion::BAYERTOMONO:
		mDetail->mOutputMetadata = boost::shared_ptr<FrameMetadata>(new RawImageMetadata(rawMetadata->getWidth(), rawMetadata->getHeight(), ImageMetadata::ImageType::MONO, CV_8UC1, 0, CV_8U, FrameMetadata::HOST, true));
		break;
	case ColorConversionProps::colorconversion::RGBTOYUV420:
		mDetail->mOutputMetadata = boost::shared_ptr<FrameMetadata>(new RawImagePlanarMetadata(rawMetadata->getWidth(), rawMetadata->getHeight(), ImageMetadata::ImageType::YUV420, size_t(0), CV_8U, FrameMetadata::HOST));
		mDetail->initMatImages(metadata);
		break;
	case ColorConversionProps::colorconversion::YUV420TORGB:
		mDetail->mOutputMetadata = boost::shared_ptr<FrameMetadata>(new RawImageMetadata(mWidth, mHeight, ImageMetadata::ImageType::RGB, CV_8UC3, 0, CV_8U, FrameMetadata::HOST, true));
		mDetail->initMatImages(metadata);
		break;
	default:
		break;
	}
	mDetail->mOutputMetadata->copyHint(*metadata.get());
	mDetail->mOutputPinId = addOutputPin(mDetail->mOutputMetadata);
}

std::string ColorConversion::addOutputPin(framemetadata_sp& metadata)
{
	return Module::addOutputPin(metadata);
}

bool ColorConversion::bayerToMono(frame_sp& frame)
{
	auto outFrame = makeFrame(mWidth * mHeight);
	auto inpPtr = static_cast<uint16_t*>(frame->data());
	auto outPtr = static_cast<uint8_t*>(outFrame->data());

	memset(outPtr, 0, mWidth * mHeight);

	for (auto i = 0; i < mHeight; i++)
	{
		auto inPtr1 = inpPtr + i * mWidth;
		auto outPtr1 = outPtr + i * mWidth;
		for (auto j = 0; j < mWidth; j++)
		{
			*outPtr1++ = (uint8_t)(*inPtr1++) >> 2;
		}
	}
	frame_container frames;
	frames.insert(make_pair(mDetail->mOutputPinId, outFrame));
	send(frames);
	return true;
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
	frame_sp frame;
	if (inputFrameType == FrameMetadata::RAW_IMAGE)
		frame = getFrameByType(frames, FrameMetadata::RAW_IMAGE);
	else
		frame = getFrameByType(frames, FrameMetadata::RAW_IMAGE_PLANAR);
	if (isFrameEmpty(frame))
	{
		return true;
	}

	auto outFrame = makeFrame();

	//	Cv based on color conversion type

	mDetail->iImg.data = static_cast<uint8_t*>(frame->data());
	mDetail->oImg.data = static_cast<uint8_t*>(outFrame->data());

	auto size = mDetail->oImg.size;
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
	case ColorConversionProps::colorconversion::BAYERTOMONO:
		bayerToMono(frame);
		return true;
	case ColorConversionProps::colorconversion::RGBTOYUV420:
		cv::cvtColor(mDetail->iImg, mDetail->oImg, cv::COLOR_RGB2YUV_I420);
		break;
	case ColorConversionProps::colorconversion::YUV420TORGB:
		cv::cvtColor(mDetail->iImg, mDetail->oImg, cv::COLOR_YUV420p2RGB);
		break;
	default:
		break;
	}
	auto rgbData = mDetail->oImg.data;
	 size = mDetail->oImg.size;
	memcpy(outFrame->data(), rgbData, mWidth * mHeight * 3);
	frames.insert(make_pair(mDetail->mOutputPinId, outFrame));
	send(frames);
	return true;
}

void ColorConversion::setMetadata(framemetadata_sp& metadata)
{

}

bool ColorConversion::processSOS(frame_sp& frame)
{
	auto metadata = frame->getMetadata();
	setMetadata(metadata);
	return true;
}