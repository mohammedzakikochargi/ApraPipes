#include "ColorConversion.h"

class DetailAbstract
{
public:
	DetailAbstract() {}
	DetailAbstract(ColorConversionProps& _props) :mProps(_props)
	{

	};

	~DetailAbstract() {}
	virtual void doColorConversion(frame_container& inputFrame, frame_sp& outFrame, framemetadata_sp outputMetadata) {};
public:
	cv::Mat iImg;
	cv::Mat oImg;
	ColorConversionProps mProps;
};

class CpuInterleaved2Planar : public DetailAbstract
{
public:
	CpuInterleaved2Planar() {}
	~CpuInterleaved2Planar() {}

protected:
	void initMatImages(frame_container& inputFrame, frame_sp& outputFrame, framemetadata_sp outputMetadata)
	{
		auto frame = Module::getFrameByType(inputFrame, FrameMetadata::RAW_IMAGE);
		auto inputMetadata = frame->getMetadata();

		iImg = Utils::getMatHeader(FrameMetadataFactory::downcast<RawImageMetadata>(inputMetadata));
		oImg = Utils::getMatHeader(FrameMetadataFactory::downcast<RawImagePlanarMetadata>(outputMetadata));
		auto planarMetadata = FrameMetadataFactory::downcast<RawImageMetadata>(inputMetadata);
		mWidth = planarMetadata->getWidth();
		mHeight = planarMetadata->getHeight();

		iImg.data = static_cast<uint8_t*>(frame->data());
		oImg.data = static_cast<uint8_t*>(outputFrame->data());
	}

	int mWidth = 0;
	int mHeight = 0;
};

class CpuRGB2YUV420Planar : public CpuInterleaved2Planar
{
public:
	CpuRGB2YUV420Planar() {}
	~CpuRGB2YUV420Planar() {}

	void CpuRGB2YUV420Planar::doColorConversion(frame_container& inputFrame, frame_sp& outputFrame, framemetadata_sp outputMetadata)
	{
		CpuInterleaved2Planar::initMatImages(inputFrame, outputFrame, outputMetadata);
		cv::cvtColor(iImg, oImg, cv::COLOR_RGB2YUV_I420);
		auto yuvData = oImg.data;
		memcpy(outputFrame->data(), yuvData, mWidth * mHeight * 1.5);
	}
};

class CpuInterleaved2Interleaved : public DetailAbstract
{
public:
	CpuInterleaved2Interleaved() {}
	~CpuInterleaved2Interleaved() {}
protected:
	void initMatImages(frame_container& inputFrame, frame_sp& outputFrame, framemetadata_sp outputMetadata)
	{
		auto frame = Module::getFrameByType(inputFrame, FrameMetadata::RAW_IMAGE);
		auto inputMetadata = frame->getMetadata();

		iImg = Utils::getMatHeader(FrameMetadataFactory::downcast<RawImageMetadata>(inputMetadata));
		oImg = Utils::getMatHeader(FrameMetadataFactory::downcast<RawImageMetadata>(outputMetadata));

		iImg.data = static_cast<uint8_t*>(frame->data());
		oImg.data = static_cast<uint8_t*>(outputFrame->data());
	}
};

class CpuRGB2BGR : public CpuInterleaved2Interleaved
{
public:
	CpuRGB2BGR() {}
	~CpuRGB2BGR() {}
	void CpuRGB2BGR::doColorConversion(frame_container& inputFrame, frame_sp& outputFrame, framemetadata_sp outputMetadata)
	{
		CpuInterleaved2Interleaved::initMatImages(inputFrame, outputFrame, outputMetadata);
		cv::cvtColor(iImg, oImg, cv::COLOR_RGB2BGR);
	}
};

class CpuBGR2RGB : public CpuInterleaved2Interleaved
{
public:
	CpuBGR2RGB() {}
	~CpuBGR2RGB() {}

	void CpuBGR2RGB::doColorConversion(frame_container& inputFrame, frame_sp& outputFrame, framemetadata_sp outputMetadata)
	{
		CpuInterleaved2Interleaved::initMatImages(inputFrame, outputFrame, outputMetadata);
		cv::cvtColor(iImg, oImg, cv::COLOR_BGR2RGB);
	}
};

class CpuRGB2MONO : public CpuInterleaved2Interleaved
{
public:
	CpuRGB2MONO() {}
	~CpuRGB2MONO() {}

	void CpuRGB2MONO::doColorConversion(frame_container& inputFrame, frame_sp& outputFrame, framemetadata_sp outputMetadata)
	{
		CpuInterleaved2Interleaved::initMatImages(inputFrame, outputFrame, outputMetadata);
		cv::cvtColor(iImg, oImg, cv::COLOR_RGB2GRAY);
	}
};

class CpuBGR2MONO : public CpuInterleaved2Interleaved
{
public:
	CpuBGR2MONO() {}
	~CpuBGR2MONO() {}

	void CpuBGR2MONO::doColorConversion(frame_container& inputFrame, frame_sp& outputFrame, framemetadata_sp outputMetadata)
	{
		CpuInterleaved2Interleaved::initMatImages(inputFrame, outputFrame, outputMetadata);
		cv::cvtColor(iImg, oImg, cv::COLOR_BGR2GRAY);
	}
};

class CpuBayerBG82RGB : public CpuInterleaved2Interleaved
{
public:
	CpuBayerBG82RGB() {}
	~CpuBayerBG82RGB() {}

	void CpuBayerBG82RGB::doColorConversion(frame_container& inputFrame, frame_sp& outputFrame, framemetadata_sp outputMetadata)
	{
		CpuInterleaved2Interleaved::initMatImages(inputFrame, outputFrame, outputMetadata);
		cv::cvtColor(iImg, oImg, cv::COLOR_BayerBG2RGB);
	}
};

class CpuBayerGB82RGB : public CpuInterleaved2Interleaved
{
public:
	CpuBayerGB82RGB() {}
	~CpuBayerGB82RGB() {}

	void CpuBayerGB82RGB::doColorConversion(frame_container& inputFrame, frame_sp& outputFrame, framemetadata_sp outputMetadata)
	{
		CpuInterleaved2Interleaved::initMatImages(inputFrame, outputFrame, outputMetadata);
		cv::cvtColor(iImg, oImg, cv::COLOR_BayerGB2RGB);
	}
};

class CpuBayerGR82RGB : public CpuInterleaved2Interleaved
{
public:
	CpuBayerGR82RGB() {}
	~CpuBayerGR82RGB() {}

	void CpuBayerGR82RGB::doColorConversion(frame_container& inputFrame, frame_sp& outputFrame, framemetadata_sp outputMetadata)
	{
		CpuInterleaved2Interleaved::initMatImages(inputFrame, outputFrame, outputMetadata);
		cv::cvtColor(iImg, oImg, cv::COLOR_BayerGR2RGB);
	}
};

class CpuBayerRG82RGB : public CpuInterleaved2Interleaved
{
public:
	CpuBayerRG82RGB() {}
	~CpuBayerRG82RGB() {}

	void CpuBayerRG82RGB::doColorConversion(frame_container& inputFrame, frame_sp& outputFrame, framemetadata_sp outputMetadata)
	{
		CpuInterleaved2Interleaved::initMatImages(inputFrame, outputFrame, outputMetadata);
		cv::cvtColor(iImg, oImg, cv::COLOR_BayerRG2RGB);
	}
};

class CpuBayerBG82Mono : public CpuInterleaved2Interleaved
{
public:
	CpuBayerBG82Mono() {}
	~CpuBayerBG82Mono() {}

	void CpuBayerBG82Mono::doColorConversion(frame_container& inputFrame, frame_sp& outputFrame, framemetadata_sp outputMetadata)
	{
		CpuInterleaved2Interleaved::initMatImages(inputFrame, outputFrame, outputMetadata);
		cv::cvtColor(iImg, oImg, cv::COLOR_BayerBG2GRAY);
	}
};

class CpuPlanar2Interleaved : public DetailAbstract
{
public:
	CpuPlanar2Interleaved() {}
	~CpuPlanar2Interleaved() {}
	void initMatImages(frame_container& inputFrame, frame_sp& outputFrame, framemetadata_sp outputMetadata)
	{
		auto frame = Module::getFrameByType(inputFrame, FrameMetadata::RAW_IMAGE_PLANAR);
		auto inputMetadata = frame->getMetadata();

		iImg = Utils::getMatHeader(FrameMetadataFactory::downcast<RawImagePlanarMetadata>(inputMetadata));
		oImg = Utils::getMatHeader(FrameMetadataFactory::downcast<RawImageMetadata>(outputMetadata));
		auto planarMetadata = FrameMetadataFactory::downcast<RawImageMetadata>(outputMetadata);
		mWidth = planarMetadata->getWidth();
		mHeight = planarMetadata->getHeight();

		iImg.data = static_cast<uint8_t*>(frame->data());
		oImg.data = static_cast<uint8_t*>(outputFrame->data());
	}

	int mWidth = 0;
	int mHeight = 0;
};

class CpuYUV420Planar2RGB : public CpuPlanar2Interleaved
{
public:
	CpuYUV420Planar2RGB() {}
	~CpuYUV420Planar2RGB() {}

	void CpuYUV420Planar2RGB::doColorConversion(frame_container& inputFrame, frame_sp& outputFrame, framemetadata_sp outputMetadata)
	{
		CpuPlanar2Interleaved::initMatImages(inputFrame, outputFrame, outputMetadata);
		cv::cvtColor(iImg, oImg, cv::COLOR_RGB2YUV_I420);
		auto yuvData = oImg.data;
		memcpy(outputFrame->data(), yuvData, mWidth * mHeight * 3);
	}
};

class GpuInterleaved2Planar : public DetailAbstract
{
public:
	GpuInterleaved2Planar() {}
	~GpuInterleaved2Planar() {}
};

class GpuInterleaved2Interleaved : public DetailAbstract
{
public:
	GpuInterleaved2Interleaved() {}
	~GpuInterleaved2Interleaved() {}
};

class GpuPlanar2Interleaved : public DetailAbstract
{
public:
	GpuPlanar2Interleaved() {}
	~GpuPlanar2Interleaved() {}
};