
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



using namespace cv;
using namespace std;

class DetailAbs
{
public:
	DetailAbs() {}
	DetailAbs(ColorConversionProps& _props) :mProps(_props)
	{

	};

	~DetailAbs() {}
	virtual void initMatImages(framemetadata_sp& input) {};
	virtual void doColorConversion(frame_container& inputFrame, frame_sp& outFrame) {};
public:
	framemetadata_sp mOutputMetadata;
	std::string mOutputPinId;
	cv::Mat iImg;
	cv::Mat oImg;
	ColorConversionProps mProps;
	FrameMetadata::FrameType inputFrameType;
};

class CpuInterleaved2Planar : public DetailAbs
{
public:
	CpuInterleaved2Planar() {}
	~CpuInterleaved2Planar() {}

	void CpuInterleaved2Planar::doColorConversion(frame_container& inputFrame, frame_sp& outputFrame)
	{
		auto frame = Module::getFrameByType(inputFrame, FrameMetadata::RAW_IMAGE);
	}

	void CpuInterleaved2Planar::initMatImages(framemetadata_sp& input)
	{
		iImg = Utils::getMatHeader(FrameMetadataFactory::downcast<RawImageMetadata>(input));
		oImg = Utils::getMatHeader(FrameMetadataFactory::downcast<RawImagePlanarMetadata>(mOutputMetadata));
	}
private:
	int mWidth =0;
	int mHeight =0;
};

class CpuInterleaved2Interleaved : public DetailAbs
{
public:
	CpuInterleaved2Interleaved() {}
	~CpuInterleaved2Interleaved() {}

	void CpuInterleaved2Interleaved::doColorConversion(frame_container& inputFrame, frame_sp& outputFrame)
	{
		auto frame = Module::getFrameByType(inputFrame, FrameMetadata::RAW_IMAGE);
		auto metadata = frame->getMetadata();
		initMatImages(metadata);
		iImg.data = static_cast<uint8_t*>(frame->data());
		oImg.data = static_cast<uint8_t*>(outputFrame->data());

		switch (mProps.colorchange)
		{
		case ColorConversionProps::colorconversion::RGBTOMONO:
			cv::cvtColor(iImg, oImg, cv::COLOR_RGB2GRAY);
			break;
		case ColorConversionProps::colorconversion::BGRTOMONO:
			cv::cvtColor(iImg, oImg, cv::COLOR_BGR2GRAY);
			break;
		case ColorConversionProps::colorconversion::BGRTORGB:
			cv::cvtColor(iImg, oImg, cv::COLOR_BGR2RGB);
			break;
		case ColorConversionProps::colorconversion::RGBTOBGR:
			cv::cvtColor(iImg, oImg, cv::COLOR_RGB2BGR);
			break;
		default:
			break;
		}
	}

	void CpuInterleaved2Interleaved::initMatImages(framemetadata_sp& input)
	{
		iImg = Utils::getMatHeader(FrameMetadataFactory::downcast<RawImageMetadata>(input));
		oImg = Utils::getMatHeader(FrameMetadataFactory::downcast<RawImageMetadata>(mOutputMetadata));
	}
};

class CpuPlanar2Interleaved : public DetailAbs
{
public:
	CpuPlanar2Interleaved() {}
	~CpuPlanar2Interleaved() {}

	void CpuPlanar2Interleaved::doColorConversion(frame_container& inputFrame, frame_sp& outputFrame)
	{
		auto frame = Module::getFrameByType(inputFrame, FrameMetadata::RAW_IMAGE_PLANAR);
		auto metadata = frame->getMetadata();
		initMatImages(metadata);
		iImg.data = static_cast<uint8_t*>(frame->data());
		oImg.data = static_cast<uint8_t*>(outputFrame->data());

		switch (mProps.colorchange)
		{
		case ColorConversionProps::colorconversion::YUV420TORGB:
			cv::cvtColor(iImg, oImg, cv::COLOR_RGB2YUV_I420);
			auto rgbData = oImg.data;
		//	memcpy(outFrame->data(), rgbData,   * mHeight * 1.5);
			break;
		}

	}

	void CpuPlanar2Interleaved::initMatImages(framemetadata_sp& input)
	{
		iImg = Utils::getMatHeader(FrameMetadataFactory::downcast<RawImagePlanarMetadata>(input));
		oImg = Utils::getMatHeader(FrameMetadataFactory::downcast<RawImageMetadata>(mOutputMetadata));
	}
private:
	int mWidth =0;
	int mHeight =0;
};

class GpuInterleaved2Planar : public DetailAbs
{
public:
	GpuInterleaved2Planar() {}
	~GpuInterleaved2Planar() {}

	void GpuInterleaved2Planar::doColorConversion(frame_container& inputFrame, frame_sp& outputFrame)
	{

	}
};

class GpuInterleaved2Interleaved : public DetailAbs
{
public:
	GpuInterleaved2Interleaved() {}
	~GpuInterleaved2Interleaved() {}

	void GpuInterleaved2Interleaved::doColorConversion(frame_container& inputFrame, frame_sp& outputFrame)
	{

	}
};

class GpuPlanar2Interleaved : public DetailAbs
{
public:
	GpuPlanar2Interleaved() {}
	~GpuPlanar2Interleaved() {}

	void GpuPlanar2Interleaved::doColorConversion(frame_container& inputFrame, frame_sp& outputFrame)
	{

	}
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

void ColorConversion::setConversionStrategy(framemetadata_sp metadata)
{
	auto memType = metadata->getMemType();
	auto outputFrameType = metadata->getFrameType();
	if (memType == FrameMetadata::HOST && mDetail->inputFrameType == FrameMetadata::RAW_IMAGE && outputFrameType == FrameMetadata::RAW_IMAGE_PLANAR)
	{
		mDetail.reset(new CpuInterleaved2Planar());
	}
	else if (memType == FrameMetadata::HOST && mDetail->inputFrameType == FrameMetadata::RAW_IMAGE && outputFrameType == FrameMetadata::RAW_IMAGE)
	{
		mDetail.reset(new CpuInterleaved2Interleaved());
	}
	else if (memType == FrameMetadata::HOST && mDetail->inputFrameType == FrameMetadata::RAW_IMAGE_PLANAR && outputFrameType == FrameMetadata::RAW_IMAGE)
	{
		mDetail.reset(new CpuPlanar2Interleaved());
	}
	else if (memType == FrameMetadata::CUDA_DEVICE && mDetail->inputFrameType == FrameMetadata::RAW_IMAGE && outputFrameType == FrameMetadata::RAW_IMAGE_PLANAR)
	{
		mDetail.reset(new GpuInterleaved2Planar());
	}
	else if (memType == FrameMetadata::CUDA_DEVICE && mDetail->inputFrameType == FrameMetadata::RAW_IMAGE && outputFrameType == FrameMetadata::RAW_IMAGE)
	{
		mDetail.reset(new GpuInterleaved2Interleaved());
	}
	else if (memType == FrameMetadata::CUDA_DEVICE && mDetail->inputFrameType == FrameMetadata::RAW_IMAGE_PLANAR && outputFrameType == FrameMetadata::RAW_IMAGE)
	{
		mDetail.reset(new GpuPlanar2Interleaved());
	}
}

void ColorConversion::addInputPin(framemetadata_sp& metadata, string& pinId)
{
	Module::addInputPin(metadata, pinId);
	mDetail->inputFrameType = metadata->getFrameType();

	if (mDetail->inputFrameType == FrameMetadata::RAW_IMAGE)
	{
		rawMetadata = FrameMetadataFactory::downcast<RawImageMetadata>(metadata);
		mWidth = rawMetadata->getWidth();
		mHeight = rawMetadata->getHeight();
		mStep = rawMetadata->getStep();
		auto imageType = rawMetadata->getImageType();
	}
	else if (mDetail->inputFrameType == FrameMetadata::RAW_IMAGE_PLANAR)
	{
		rawPlanarMetadata = FrameMetadataFactory::downcast<RawImagePlanarMetadata>(metadata);
		mWidth = rawPlanarMetadata->getWidth(0);
		mHeight = rawPlanarMetadata->getHeight(0);
		mStep = rawPlanarMetadata->getStep(0);
		auto imageType = rawPlanarMetadata->getImageType();
	}

	//auto out = getFirstOutputMetadata();

	switch (mProps.colorchange)
	{
	case ColorConversionProps::colorconversion::RGBTOMONO:
		mDetail->mOutputMetadata = boost::shared_ptr<FrameMetadata>(new RawImageMetadata(mWidth, mHeight, ImageMetadata::MONO, CV_8UC1, 0, CV_8U, FrameMetadata::HOST, true));
		break;
	case ColorConversionProps::colorconversion::BGRTOMONO:
		mDetail->mOutputMetadata = boost::shared_ptr<FrameMetadata>(new RawImageMetadata(mWidth, mHeight, ImageMetadata::MONO, CV_8UC1, 0, CV_8U, FrameMetadata::HOST, true));
		break;
	case ColorConversionProps::colorconversion::BGRTORGB:
		mDetail->mOutputMetadata = boost::shared_ptr<FrameMetadata>(new RawImageMetadata(mWidth, mHeight, ImageMetadata::RGB, CV_8UC3, 0, CV_8U, FrameMetadata::HOST, true));
		break;
	case ColorConversionProps::colorconversion::RGBTOBGR:
		mDetail->mOutputMetadata = boost::shared_ptr<FrameMetadata>(new RawImageMetadata(mWidth, mHeight, ImageMetadata::BGR, CV_8UC3, 0, CV_8U, FrameMetadata::HOST, true));
		break;
	case ColorConversionProps::colorconversion::BAYERTOMONO:
		mDetail->mOutputMetadata = boost::shared_ptr<FrameMetadata>(new RawImageMetadata(mWidth, mHeight, ImageMetadata::MONO, CV_8UC1, 0, CV_8U, FrameMetadata::HOST, true));
		break;
	case ColorConversionProps::colorconversion::RGBTOYUV420:
		mDetail->mOutputMetadata = boost::shared_ptr<FrameMetadata>(new RawImagePlanarMetadata(mWidth, mHeight, ImageMetadata::YUV420, size_t(0), CV_8U, FrameMetadata::HOST));
		break;
	case ColorConversionProps::colorconversion::YUV420TORGB:
		mDetail->mOutputMetadata = boost::shared_ptr<FrameMetadata>(new RawImageMetadata(mWidth, mHeight, ImageMetadata::RGB, CV_8UC3, 0, CV_8U, FrameMetadata::HOST, true));
		break;
	default:
		break;
	}
	mDetail->mOutputMetadata->copyHint(*metadata.get());
	mDetail->mOutputPinId = addOutputPin(mDetail->mOutputMetadata);

	setConversionStrategy(mDetail->mOutputMetadata);

}

std::string ColorConversion::addOutputPin(framemetadata_sp& metadata)
{
	LOG_ERROR << "cc";
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
	auto outFrame = makeFrame();
	mDetail->doColorConversion(frames, outFrame);
	frame_sp frame;
	if (mDetail->inputFrameType == FrameMetadata::RAW_IMAGE)
		frame = getFrameByType(frames, FrameMetadata::RAW_IMAGE);
	else
		frame = getFrameByType(frames, FrameMetadata::RAW_IMAGE_PLANAR);
	if (isFrameEmpty(frame))
	{
		return true;
	}
	//	Cv based on color conversion type

	mDetail->iImg.data = static_cast<uint8_t*>(frame->data());
	mDetail->oImg.data = static_cast<uint8_t*>(outFrame->data());

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

	//memcpy(outFrame->data(), rgbData, mWidth * mHeight * 1.5);
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