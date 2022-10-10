#include "stdafx.h"
#include <boost/test/unit_test.hpp>
#include "FileReaderModule.h"
#include "ExternalSinkModule.h"
#include "FrameMetadata.h"
#include "FrameMetadataFactory.h"
#include "Frame.h"
#include "Logger.h"
#include "AIPExceptions.h"
#include "test_utils.h"
#include "PipeLine.h"
#include "StatSink.h"
#include "FileWriterModule.h"
#include "ColorConversion.h"

#ifdef ARM64
BOOST_AUTO_TEST_SUITE(color_conversion_tests, *boost::unit_test::disabled())
#else
BOOST_AUTO_TEST_SUITE(color_conversion_tests)
#endif

BOOST_AUTO_TEST_CASE(rgb_mono)
{

	auto fileReader = boost::shared_ptr<FileReaderModule>(new FileReaderModule(FileReaderModuleProps("./data/frame_1280x720_rgb.raw")));
	auto metadata = framemetadata_sp(new RawImageMetadata(1280, 720, ImageMetadata::ImageType::RGB, CV_8UC3, 0, CV_8U, FrameMetadata::HOST, true));
	fileReader->addOutputPin(metadata);

	auto colorchange = boost::shared_ptr<ColorConversion>(new ColorConversion(ColorConversionProps(ColorConversionProps::colorconversion::RGBTOMONO)));
	fileReader->setNext(colorchange);

	auto sink = boost::shared_ptr<ExternalSinkModule>(new ExternalSinkModule());
	colorchange->setNext(sink);

	BOOST_TEST(fileReader->init());
	BOOST_TEST(colorchange->init());
	BOOST_TEST(sink->init());

	fileReader->step();
	colorchange->step();
	auto frames = sink->pop();
	BOOST_TEST(frames.size() == 1);
	auto outputFrame = frames.cbegin()->second;
	BOOST_TEST(outputFrame->getMetadata()->getFrameType() == FrameMetadata::RAW_IMAGE);

	Test_Utils::saveOrCompare("./data/testOutput/frame_1280x720_rgb_cc_mono.raw", const_cast<const uint8_t*>(static_cast<uint8_t*>(outputFrame->data())), outputFrame->size(), 0);

}

BOOST_AUTO_TEST_CASE(bgr_mono)
{

	auto fileReader = boost::shared_ptr<FileReaderModule>(new FileReaderModule(FileReaderModuleProps("./data/frame_1280x720_bgr.raw")));
	auto metadata = framemetadata_sp(new RawImageMetadata(1280, 720, ImageMetadata::ImageType::BGR, CV_8UC3, 0, CV_8U, FrameMetadata::HOST, true));
	fileReader->addOutputPin(metadata);

	auto colorchange = boost::shared_ptr<ColorConversion>(new ColorConversion(ColorConversionProps(ColorConversionProps::colorconversion::BGRTOMONO)));
	fileReader->setNext(colorchange);

	auto sink = boost::shared_ptr<ExternalSinkModule>(new ExternalSinkModule());
	colorchange->setNext(sink);

	BOOST_TEST(fileReader->init());
	BOOST_TEST(colorchange->init());
	BOOST_TEST(sink->init());

	fileReader->step();
	colorchange->step();
	auto frames = sink->pop();
	BOOST_TEST(frames.size() == 1);
	auto outputFrame = frames.cbegin()->second;
	BOOST_TEST(outputFrame->getMetadata()->getFrameType() == FrameMetadata::RAW_IMAGE);

	Test_Utils::saveOrCompare("./data/testOutput/frame_1280x720_bgr_cc_mono.raw", const_cast<const uint8_t*>(static_cast<uint8_t*>(outputFrame->data())), outputFrame->size(), 0);

}

BOOST_AUTO_TEST_CASE(bgr_rgb)
{

	auto fileReader = boost::shared_ptr<FileReaderModule>(new FileReaderModule(FileReaderModuleProps("./data/jpegdecodercv_tests_mono_1920x960.raw")));
	auto metadata = framemetadata_sp(new RawImageMetadata(1280, 720, ImageMetadata::ImageType::BGR, CV_8UC3, 0, CV_8U, FrameMetadata::HOST, true));
	fileReader->addOutputPin(metadata);

	auto colorchange = boost::shared_ptr<ColorConversion>(new ColorConversion(ColorConversionProps(ColorConversionProps::colorconversion::BGRTORGB)));
	fileReader->setNext(colorchange);

	auto sink = boost::shared_ptr<ExternalSinkModule>(new ExternalSinkModule());
	colorchange->setNext(sink);

	BOOST_TEST(fileReader->init());
	BOOST_TEST(colorchange->init());
	BOOST_TEST(sink->init());

	fileReader->step();
	colorchange->step();
	auto frames = sink->pop();
	BOOST_TEST(frames.size() == 1);
	auto outputFrame = frames.cbegin()->second;
	BOOST_TEST(outputFrame->getMetadata()->getFrameType() == FrameMetadata::RAW_IMAGE);

	Test_Utils::saveOrCompare("./data/testOutput/frame_1280x720_bgr_cc_rgb.raw", const_cast<const uint8_t*>(static_cast<uint8_t*>(outputFrame->data())), outputFrame->size(), 0);

}

BOOST_AUTO_TEST_CASE(rgb_bgr)
{

	auto fileReader = boost::shared_ptr<FileReaderModule>(new FileReaderModule(FileReaderModuleProps("./data/frame_1280x720_rgb.raw")));
	auto metadata = framemetadata_sp(new RawImageMetadata(1280, 720, ImageMetadata::ImageType::RGB, CV_8UC3, 0, CV_8U, FrameMetadata::HOST, true));
	fileReader->addOutputPin(metadata);

	auto colorchange = boost::shared_ptr<ColorConversion>(new ColorConversion(ColorConversionProps(ColorConversionProps::colorconversion::RGBTOBGR)));
	fileReader->setNext(colorchange);

	auto sink = boost::shared_ptr<ExternalSinkModule>(new ExternalSinkModule());
	colorchange->setNext(sink);

	BOOST_TEST(fileReader->init());
	BOOST_TEST(colorchange->init());
	BOOST_TEST(sink->init());

	fileReader->step();
	colorchange->step();
	auto frames = sink->pop();
	BOOST_TEST(frames.size() == 1);
	auto outputFrame = frames.cbegin()->second;
	BOOST_TEST(outputFrame->getMetadata()->getFrameType() == FrameMetadata::RAW_IMAGE);

	Test_Utils::saveOrCompare("./data/testOutput/frame_1280x720_rgb_cc_bgr.raw", const_cast<const uint8_t*>(static_cast<uint8_t*>(outputFrame->data())), outputFrame->size(), 0);

}

BOOST_AUTO_TEST_CASE(bayer_mono)
{
	auto fileReader = boost::shared_ptr<FileReaderModule>(new FileReaderModule(FileReaderModuleProps("./data/frame-4.raw")));
	auto metadata = framemetadata_sp(new RawImageMetadata(800, 800, ImageMetadata::ImageType::BG10, CV_16UC1, 0 , CV_16U, FrameMetadata::HOST, true));
	fileReader->addOutputPin(metadata);

	auto colorchange = boost::shared_ptr<ColorConversion>(new ColorConversion(ColorConversionProps(ColorConversionProps::colorconversion::BAYERTOMONO)));
	fileReader->setNext(colorchange);

	auto sink = boost::shared_ptr<ExternalSinkModule>(new ExternalSinkModule());
	colorchange->setNext(sink);

	BOOST_TEST(fileReader->init());
	BOOST_TEST(colorchange->init());
	BOOST_TEST(sink->init());

	fileReader->step();
	colorchange->step();
	auto frames = sink->pop();
	BOOST_TEST(frames.size() == 1);
	auto outputFrame = frames.cbegin()->second;
	BOOST_TEST(outputFrame->getMetadata()->getFrameType() == FrameMetadata::RAW_IMAGE);

	Test_Utils::saveOrCompare("./data/testOutput/frame_1280x720_bayer_cc_mono.raw", const_cast<const uint8_t*>(static_cast<uint8_t*>(outputFrame->data())), outputFrame->size(), 0);

}

BOOST_AUTO_TEST_CASE(RGB_YUV420)
{
	auto fileReader = boost::shared_ptr<FileReaderModule>(new FileReaderModule(FileReaderModuleProps("./data/frame_1280x720_rgb.raw")));
	auto metadata = framemetadata_sp(new RawImageMetadata(1280,720, ImageMetadata::ImageType::RGB, CV_8UC3, 0, CV_8U, FrameMetadata::HOST, true));
	fileReader->addOutputPin(metadata);

	auto colorchange = boost::shared_ptr<ColorConversion>(new ColorConversion(ColorConversionProps(ColorConversionProps::colorconversion::RGBTOYUV420)));
	fileReader->setNext(colorchange);

	auto sink = boost::shared_ptr<ExternalSinkModule>(new ExternalSinkModule());
	colorchange->setNext(sink);

	BOOST_TEST(fileReader->init());
	BOOST_TEST(colorchange->init());//
	BOOST_TEST(sink->init());

	fileReader->step();
	colorchange->step();
	auto frames = sink->pop();
	BOOST_TEST(frames.size() == 1);
	auto outputFrame = frames.cbegin()->second;
	BOOST_TEST(outputFrame->getMetadata()->getFrameType() == FrameMetadata::RAW_IMAGE_PLANAR);

	Test_Utils::saveOrCompare("./data/testOutput/_1280X720_RGB_cc_YUV420.raw", const_cast<const uint8_t*>(static_cast<uint8_t*>(outputFrame->data())), outputFrame->size(), 0);

}

BOOST_AUTO_TEST_CASE(YUV420_RGB)
{
	auto fileReader = boost::shared_ptr<FileReaderModule>(new FileReaderModule(FileReaderModuleProps("./data/YUV_420_planar.raw")));
	auto metadata = framemetadata_sp(new RawImagePlanarMetadata(1280, 720, ImageMetadata::ImageType::YUV420, size_t(0), CV_8U));
	fileReader->addOutputPin(metadata);

	auto colorchange = boost::shared_ptr<ColorConversion>(new ColorConversion(ColorConversionProps(ColorConversionProps::colorconversion::YUV420TORGB)));
	fileReader->setNext(colorchange);

	auto sink = boost::shared_ptr<ExternalSinkModule>(new ExternalSinkModule());
	colorchange->setNext(sink);

	BOOST_TEST(fileReader->init());
	BOOST_TEST(colorchange->init());
	BOOST_TEST(sink->init());

	fileReader->step();
	colorchange->step();
	auto frames = sink->pop();
	BOOST_TEST(frames.size() == 1);
	auto outputFrame = frames.cbegin()->second;
	BOOST_TEST(outputFrame->getMetadata()->getFrameType() == FrameMetadata::RAW_IMAGE);

	Test_Utils::saveOrCompare("./data/testOutput/frame__1280x720_YUV420p_cc_RGB.raw", const_cast<const uint8_t*>(static_cast<uint8_t*>(outputFrame->data())), outputFrame->size(), 0);

}


BOOST_AUTO_TEST_SUITE_END()