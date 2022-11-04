#include "stdafx.h"
#include <boost/test/unit_test.hpp>
#include "FileReaderModule.h"
#include "FileWriterModule.h"
#include "Frame.h"
#include "Logger.h"
#include "AIPExceptions.h"
#include "CudaMemCopy.h"
#include "CCNPPI.h"
#include "CudaStreamSynchronize.h"
#include "H264DecoderNvCodec.h"
#include "ResizeNPPI.h"
#include "test_utils.h"
#include "nv_test_utils.h"
#include "PipeLine.h"
#include "ExternalSinkModule.h"
#include "StatSink.h"
#include "H264Metadata.h"
#include "H264DecoderNvCodecHelper.h"

BOOST_AUTO_TEST_SUITE(h264Decodernvcodec_tests)

BOOST_AUTO_TEST_CASE(H264_704x576)
{
	Logger::setLogLevel("info");

	// metadata is known
	auto props = FileReaderModuleProps("./data/h264_data/FVDO_Freeway_4cif_???.H264", 0, -1, 100000);
	props.readLoop = false;
	auto fileReader = boost::shared_ptr<FileReaderModule>(new FileReaderModule(props));
	
	auto h264ImageMetadata = framemetadata_sp(new H264Metadata(0,0));

	auto rawImagePin = fileReader->addOutputPin(h264ImageMetadata);

	auto Decoder = boost::shared_ptr<Module>(new H264DecoderNvCodec(H264DecoderNvCodecProps()));
	fileReader->setNext(Decoder);

	auto fileWriter = boost::shared_ptr<Module>(new FileWriterModule(FileWriterModuleProps("./data/testOutput/h264images/Raw_YUV420_640x360????.raw")));
	Decoder->setNext(fileWriter);
	fileReader->play(true);

	boost::shared_ptr<PipeLine> p;
	p = boost::shared_ptr<PipeLine>(new PipeLine("test"));
	p->appendModule(fileReader);

	if (!p->init())
	{
		throw AIPException(AIP_FATAL, "Engine Pipeline init failed. Check IPEngine Logs for more details.");
	}

	p->run_all_threaded();

	Test_Utils::sleep_for_seconds(6);

	p->stop();
	p->term();
	p->wait_for_all();
	p.reset();
	
}

BOOST_AUTO_TEST_CASE(Encoder_to_Decoder)
{
	Logger::setLogLevel("info");
	auto cuContext = apracucontext_sp(new ApraCUcontext());

	// metadata is known
	auto width = 640;
	auto height = 360;
	uint32_t gopLength = 25;
	uint32_t bitRateKbps = 1000;
	uint32_t frameRate = 30;
	H264EncoderNVCodecProps::H264CodecProfile profile = H264EncoderNVCodecProps::BASELINE;
	bool enableBFrames = true;

	auto fileReader = boost::shared_ptr<FileReaderModule>(new FileReaderModule(FileReaderModuleProps("./data/Raw_YUV420_640x360/Image???_YUV420.raw",0,-1,500000)));
	auto metadata = framemetadata_sp(new RawImagePlanarMetadata(width, height, ImageMetadata::ImageType::YUV420, size_t(0), CV_8U));

	auto rawImagePin = fileReader->addOutputPin(metadata);


	auto cudaStream_ = boost::shared_ptr<ApraCudaStream>(new ApraCudaStream());

	auto copyProps = CudaMemCopyProps(cudaMemcpyKind::cudaMemcpyHostToDevice, cudaStream_);
	copyProps.sync = true;
	auto copy = boost::shared_ptr<Module>(new CudaMemCopy(copyProps));
	fileReader->setNext(copy);
	auto encoder = boost::shared_ptr<Module>(new H264EncoderNVCodec(H264EncoderNVCodecProps(bitRateKbps, cuContext, gopLength, frameRate, profile, enableBFrames)));
	copy->setNext(encoder);

	auto Decoder = boost::shared_ptr<Module>(new H264DecoderNvCodec(H264DecoderNvCodecProps()));
	encoder->setNext(Decoder);

	auto fileWriter = boost::shared_ptr<Module>(new FileWriterModule(FileWriterModuleProps("./data/testOutput/h264images/Raw_YUV420_640x360????.raw")));
	Decoder->setNext(fileWriter);

	/*auto m2 = boost::shared_ptr<ExternalSinkModule>(new ExternalSinkModule());
	Decoder->setNext(m2);*/

	fileReader->play(true);

	BOOST_TEST(fileReader->init());
	BOOST_TEST(copy->init());
	BOOST_TEST(encoder->init());
	BOOST_TEST(Decoder->init());
	BOOST_TEST(fileWriter->init());
	//BOOST_TEST(m2->init());

	for (auto i = 0; i <= 43; i++)
	{
		fileReader->step();
		copy->step();
		encoder->step();
		Decoder->step();

		if (i >= 3)
		{
			fileWriter->step();
		}
		/*if (i >= 3)
		{
			auto frames = m2->pop();
			BOOST_TEST(frames.size() == 1);
			auto outputFrame = frames.cbegin()->second;
			BOOST_TEST(outputFrame->getMetadata()->getFrameType() == FrameMetadata::RAW_IMAGE_PLANAR);
			Test_Utils::saveOrCompare("/data/Raw_YUV420_640x360/Image???_YUV420.raw", const_cast<const uint8_t*>(static_cast<uint8_t*>(outputFrame->data())), outputFrame->size(), 0);
		}*/
	}
	//boost::shared_ptr<PipeLine> p;
	//p = boost::shared_ptr<PipeLine>(new PipeLine("test"));
	//p->appendModule(fileReader);
	//
	//if (!p->init())
	//{
	//	throw AIPException(AIP_FATAL, "Engine Pipeline init failed. Check IPEngine Logs for more details.");
	//}
	//
	//p->run_all_threaded();
	//
	//Test_Utils::sleep_for_seconds(6);
	//
	//p->stop();
	//p->term();
	//p->wait_for_all();
	//p.reset();
}

BOOST_AUTO_TEST_SUITE_END()