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
	auto width = 704;
	auto height = 576;
	auto props = FileReaderModuleProps("./data/h264_data/FVDO_Freeway_4cif_???.H264", 0, -1, 100000);
	props.readLoop = true;
	auto fileReader = boost::shared_ptr<FileReaderModule>(new FileReaderModule(props));
	
	auto h264ImageMetadata = framemetadata_sp(new H264Metadata(width, height));

	auto rawImagePin = fileReader->addOutputPin(h264ImageMetadata);

	auto Decoder = boost::shared_ptr<Module>(new H264DecoderNvCodec(H264DecoderNvCodecProps()));
	fileReader->setNext(Decoder);

	auto fileWriter = boost::shared_ptr<Module>(new FileWriterModule(FileWriterModuleProps("./data/testOutput/h264images/Raw_YUV420_640x360????.raw")));
	Decoder->setNext(fileWriter);

	BOOST_TEST(fileReader->init());
	BOOST_TEST(Decoder->init());
	BOOST_TEST(fileWriter->init());

	fileReader->play(true);

	for (auto i = 0; i < 231; i++)
	{
		fileReader->step();
	//	copy->step();
		Decoder->step();
		if (i > 18)
		{
			fileWriter->step();
		}
	}
	for (auto j = 0; j < 1; j++)
	{
		Decoder->step();
	}
	
}
}