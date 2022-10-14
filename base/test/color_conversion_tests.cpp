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

frame_sp colorConversion(std::string inputPathName, framemetadata_sp metadata, ColorConversionProps::colorconversion conversionType)
{
	auto fileReader = boost::shared_ptr<FileReaderModule>(new FileReaderModule(FileReaderModuleProps(inputPathName)));
	fileReader->addOutputPin(metadata);

	auto colorchange = boost::shared_ptr<ColorConversion>(new ColorConversion(ColorConversionProps(conversionType)));
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

	return outputFrame;
}

BOOST_AUTO_TEST_CASE(rgb_mono)
{
	std::string inputPathName = "./data/frame_1280x720_rgb.raw";
	auto metadata = framemetadata_sp(new RawImageMetadata(1280, 720, ImageMetadata::ImageType::RGB, CV_8UC3, 0, CV_8U, FrameMetadata::HOST, true));
	auto conversionType = ColorConversionProps::colorconversion::RGB_2_MONO;

	auto outputFrame = colorConversion(inputPathName, metadata, conversionType);

	Test_Utils::saveOrCompare("./data/testOutput/frame_1280x720_rgb_cc_mono.raw", const_cast<const uint8_t*>(static_cast<uint8_t*>(outputFrame->data())), outputFrame->size(), 0);
}

BOOST_AUTO_TEST_CASE(bgr_mono)
{
	std::string inputPathName = "./data/frame_1280x720_bgr.raw";
	auto conversionType = ColorConversionProps::colorconversion::BGR_2_MONO;
	auto metadata = framemetadata_sp(new RawImageMetadata(1280, 720, ImageMetadata::ImageType::BGR, CV_8UC3, 0, CV_8U, FrameMetadata::HOST, true));
	
	auto outputFrame = colorConversion(inputPathName, metadata, conversionType);

	Test_Utils::saveOrCompare("./data/testOutput/frame_1280x720_bgr_cc_mono.raw", const_cast<const uint8_t*>(static_cast<uint8_t*>(outputFrame->data())), outputFrame->size(), 0);

}

BOOST_AUTO_TEST_CASE(bgr_rgb)
{
	std::string inputPathName = "./data/BGR_1080x720.raw";
	auto conversionType = ColorConversionProps::colorconversion::BGR_2_RGB;
	auto metadata = framemetadata_sp(new RawImageMetadata(1280, 720, ImageMetadata::ImageType::BGR, CV_8UC3, 0, CV_8U, FrameMetadata::HOST, true));
	
	auto outputFrame = colorConversion(inputPathName, metadata, conversionType);

	Test_Utils::saveOrCompare("./data/testOutput/frame_1280x720_bgr_cc_rgb.raw", const_cast<const uint8_t*>(static_cast<uint8_t*>(outputFrame->data())), outputFrame->size(), 0);

}

BOOST_AUTO_TEST_CASE(rgb_bgr)
{

	std::string inputPathName = "./data/frame_1280x720_rgb.raw";
	auto conversionType = ColorConversionProps::colorconversion::RGB_2_BGR;
	auto metadata = framemetadata_sp(new RawImageMetadata(1280, 720, ImageMetadata::ImageType::RGB, CV_8UC3, 0, CV_8U, FrameMetadata::HOST, true));
	
	auto outputFrame = colorConversion(inputPathName, metadata, conversionType);

	Test_Utils::saveOrCompare("./data/testOutput/frame_1280x720_rgb_cc_bgr.raw", const_cast<const uint8_t*>(static_cast<uint8_t*>(outputFrame->data())), outputFrame->size(), 0);

}

BOOST_AUTO_TEST_CASE(RGB_YUV420)
{
	std::string inputPathName = "./data/frame_1280x720_rgb.raw";
	auto conversionType = ColorConversionProps::colorconversion::RGB_2_YUV420PLANAR;
	auto metadata = framemetadata_sp(new RawImageMetadata(1280,720, ImageMetadata::ImageType::RGB, CV_8UC3, 0, CV_8U, FrameMetadata::HOST, true));
	
	auto outputFrame = colorConversion(inputPathName, metadata, conversionType);

	Test_Utils::saveOrCompare("./data/testOutput/frame_1280X720_RGB_cc_YUV420.raw", const_cast<const uint8_t*>(static_cast<uint8_t*>(outputFrame->data())), outputFrame->size(), 0);

}

BOOST_AUTO_TEST_CASE(YUV420_RGB)
{
	std::string inputPathName = "./data/YUV_420_planar.raw";
	auto conversionType = ColorConversionProps::colorconversion::YUV420PLANAR_2_RGB;
	auto metadata = framemetadata_sp(new RawImagePlanarMetadata(1280, 720, ImageMetadata::ImageType::YUV420, size_t(0), CV_8U));

	auto outputFrame = colorConversion(inputPathName, metadata, conversionType);
	
	Test_Utils::saveOrCompare("./data/testOutput/frame_1280x720_YUV420p_cc_RGB.raw", const_cast<const uint8_t*>(static_cast<uint8_t*>(outputFrame->data())), outputFrame->size(), 0);

}

BOOST_AUTO_TEST_CASE(BayerBG8Bit_2_RGB)
{
	std::string inputPathName = "./data/Bayer_images/Rubiks_BayerBG8_800x800.raw";
	auto conversionType = ColorConversionProps::colorconversion::BAYERBG8_2_RGB;
	auto metadata = framemetadata_sp(new RawImageMetadata(800, 800, ImageMetadata::ImageType::BAYERBG8, CV_8UC1, 0, CV_8U, FrameMetadata::HOST, true));

	auto outputFrame = colorConversion(inputPathName, metadata, conversionType);

	Test_Utils::saveOrCompare("./data/testOutput/frame_800x800_bayerBG8bit_cc_rgb.raw", const_cast<const uint8_t*>(static_cast<uint8_t*>(outputFrame->data())), outputFrame->size(), 0);

}

BOOST_AUTO_TEST_CASE(BayerBG8Bit_2_Mono)
{
	std::string inputPathName = "./data/Bayer_images/Rubiks_BayerBG8_800x800.raw";
	auto conversionType = ColorConversionProps::colorconversion::BAYERBG8_2_MONO;
	auto metadata = framemetadata_sp(new RawImageMetadata(800, 800, ImageMetadata::ImageType::BAYERBG8, CV_8UC1, 0, CV_8U, FrameMetadata::HOST, true));
	
	auto outputFrame = colorConversion(inputPathName, metadata, conversionType);

	Test_Utils::saveOrCompare("./data/testOutput/frame_800x800_bayerBG8bit_cc_mono.raw", const_cast<const uint8_t*>(static_cast<uint8_t*>(outputFrame->data())), outputFrame->size(), 0);

}

BOOST_AUTO_TEST_CASE(BayerGB8Bit_2_RGB)
{
	std::string inputPathName = "./data/Bayer_images/Rubiks_bayerGB8_799xx800.raw";
	auto conversionType = ColorConversionProps::colorconversion::BAYERGB8_2_RGB;
	auto metadata = framemetadata_sp(new RawImageMetadata(799, 800, ImageMetadata::ImageType::BAYERGB8, CV_8UC1, 0, CV_8U, FrameMetadata::HOST, true));

	auto outputFrame = colorConversion(inputPathName, metadata, conversionType);

	Test_Utils::saveOrCompare("./data/testOutput/frame_799x800_bayerGB8bit_cc_RGB.raw", const_cast<const uint8_t*>(static_cast<uint8_t*>(outputFrame->data())), outputFrame->size(), 0);

}

BOOST_AUTO_TEST_CASE(BayerGR8Bit_2_RGB)
{
	std::string inputPathName = "./data/Bayer_images/Rubiks_bayerGR8_800x799.raw";
	auto conversionType = ColorConversionProps::colorconversion::BAYERGR8_2_RGB;
	auto metadata = framemetadata_sp(new RawImageMetadata(800, 799, ImageMetadata::ImageType::BAYERGR8, CV_8UC1, 0, CV_8U, FrameMetadata::HOST, true));

	auto outputFrame = colorConversion(inputPathName, metadata, conversionType);

	Test_Utils::saveOrCompare("./data/testOutput/frame_800x799_bayerGR8bit_cc_mono.raw", const_cast<const uint8_t*>(static_cast<uint8_t*>(outputFrame->data())), outputFrame->size(), 0);

}

BOOST_AUTO_TEST_CASE(BayerRG8Bit_2_RGB)
{
	std::string inputPathName = "./data/Bayer_images/Rubiks_bayerRG8_799x799.raw";
	auto conversionType = ColorConversionProps::colorconversion::BAYERRG8_2_RGB;
	auto metadata = framemetadata_sp(new RawImageMetadata(799, 799, ImageMetadata::ImageType::BAYERRG8, CV_8UC1, 0, CV_8U, FrameMetadata::HOST, true));

	auto outputFrame = colorConversion(inputPathName, metadata, conversionType);

	Test_Utils::saveOrCompare("./data/testOutput/frame_799x799_bayerRG8bit_cc_RGB.raw", const_cast<const uint8_t*>(static_cast<uint8_t*>(outputFrame->data())), outputFrame->size(), 0);

}

BOOST_AUTO_TEST_SUITE_END()