#include <boost/test/unit_test.hpp>

#include "Logger.h"
#include "AIPExceptions.h"
#include "PipeLine.h"

#include "test_utils.h"
#include "FileReaderModule.h"
#include "CudaCommon.h"
#include "Mp4WriterSink.h"
#include "FramesMuxer.h"
#include "StatSink.h"
#include "EncodedImageMetadata.h"
#include "Mp4VideoMetadata.h"

BOOST_AUTO_TEST_SUITE(mp4WriterSink_tests)

void write(std::string inFolderPath, std::string outFolderPath, int width, int height)
{
	LoggerProps loggerProps;
	loggerProps.logLevel = boost::log::trivial::severity_level::info;
	Logger::setLogLevel(boost::log::trivial::severity_level::info);
	Logger::initLogger(loggerProps);

	auto fileReaderProps = FileReaderModuleProps(inFolderPath, 0, -1, 4 * 1024 * 1024);
	fileReaderProps.fps = 50;
	fileReaderProps.readLoop =true;

	auto fileReader = boost::shared_ptr<Module>(new FileReaderModule(fileReaderProps));
	auto encodedImageMetadata = framemetadata_sp(new EncodedImageMetadata(width, height));
	fileReader->addOutputPin(encodedImageMetadata);

	auto mp4WriterSinkProps = Mp4WriterSinkProps(5, 1, 30, outFolderPath);
	mp4WriterSinkProps.logHealth = true;
	mp4WriterSinkProps.logHealthFrequency = 10;
	auto mp4WriterSink = boost::shared_ptr<Module>(new Mp4WriterSink(mp4WriterSinkProps));
	fileReader->setNext(mp4WriterSink);

	// #Dec_27_Review - do manual init, step and use saveorcompare

	boost::shared_ptr<PipeLine> p;
	p = boost::shared_ptr<PipeLine>(new PipeLine("test"));
	p->appendModule(fileReader);

	if (!p->init())
	{
		throw AIPException(AIP_FATAL, "Engine Pipeline init failed. Check IPEngine Logs for more details.");
	}

	LOG_ERROR << "processing folder <" << inFolderPath << ">";
	p->run_all_threaded();

	boost::this_thread::sleep_for(boost::chrono::seconds(330));

	p->stop();
	p->term();
	p->wait_for_all();
	p.reset();
}

void write_metadata(std::string inFolderPath, std::string outFolderPath, std::string metadataPath, int width, int height, int fps)
{
	LoggerProps loggerProps;
	loggerProps.logLevel = boost::log::trivial::severity_level::info;
	Logger::setLogLevel(boost::log::trivial::severity_level::info);
	Logger::initLogger(loggerProps);

	auto fileReaderProps = FileReaderModuleProps(inFolderPath, 0, -1, 4 * 1024 * 1024);
	fileReaderProps.fps = 100;
	fileReaderProps.readLoop = true;

	auto fileReader = boost::shared_ptr<Module>(new FileReaderModule(fileReaderProps));
	auto encodedImageMetadata = framemetadata_sp(new EncodedImageMetadata(width, height));
	fileReader->addOutputPin(encodedImageMetadata);


	auto fileReaderProps2 = FileReaderModuleProps(metadataPath, 0, -1, 1 * 1024 * 1024);
	fileReaderProps2.fps = 100;
	fileReaderProps2.readLoop = true;
	auto metadataReader = boost::shared_ptr<Module>(new FileReaderModule(fileReaderProps2));
	auto mp4Metadata = framemetadata_sp(new Mp4VideoMetadata("v_1_0"));
	metadataReader->addOutputPin(mp4Metadata);

	auto readerMuxer = boost::shared_ptr<Module>(new FramesMuxer());
	fileReader->setNext(readerMuxer);
	metadataReader->setNext(readerMuxer);

	auto mp4WriterSinkProps = Mp4WriterSinkProps(1, 1, fileReaderProps.fps, outFolderPath);
	mp4WriterSinkProps.logHealth = true;
	mp4WriterSinkProps.logHealthFrequency = 100;
	auto mp4WriterSink = boost::shared_ptr<Module>(new Mp4WriterSink(mp4WriterSinkProps));
	readerMuxer->setNext(mp4WriterSink);

	// #Dec_27_Review - do manual init, step and use saveorcompare

	boost::shared_ptr<PipeLine> p;
	p = boost::shared_ptr<PipeLine>(new PipeLine("test"));
	p->appendModule(fileReader);
	p->appendModule(metadataReader);

	if (!p->init())
	{
		throw AIPException(AIP_FATAL, "Engine Pipeline init failed. Check IPEngine Logs for more details.");
	}

	LOG_ERROR << "processing folder <" << inFolderPath << ">";
	p->run_all_threaded();

	boost::this_thread::sleep_for(boost::chrono::seconds(1800));

	p->stop();
	p->term();
	p->wait_for_all();
	p.reset();
}

BOOST_AUTO_TEST_CASE(jpg_rgb_24_to_mp4v)
{
	int width = 960;
	int height = 480;

	std::string inFolderPath = "./data/streamer_frames";
	std::string outFolderPath = "./data/mp4_videos/";

	write(inFolderPath, outFolderPath, width, height);
}

BOOST_AUTO_TEST_CASE(jpg_mono_8_to_mp4v)
{
	int width = 448;
	int height = 608;

	std::string inFolderPath = "./data/re3_filtered_mono";
	std::string outFolderPath = "./data/mp4_videos/mono_8bpp/";

	write(inFolderPath, outFolderPath, width, height);
}

BOOST_AUTO_TEST_CASE(jpg_mono_8_to_mp4v_metadata)
{
	int width = 448;
	int height = 608;

	std::string inFolderPath = "./data/re3_filtered_mono";
	std::string outFolderPath = "./data/mp4_videos/mono_8bpp/";
	std::string metadataPath = "./data/mp4_videos/metadata/";

	write_metadata(inFolderPath, outFolderPath, metadataPath, width, height, 30);
}

BOOST_AUTO_TEST_CASE(write_metadata_mp4v)
{
	/* metadata, RGB, 24bpp, 960x480 */
	int width = 960;
	int height = 480;
	int fps = 100;

	std::string inFolderPath = "./data/streamer_frames_num";
	std::string outFolderPath = "./data/mp4_videos/metadata_video";
	std::string metadataPath = "./data/mp4_videos/metadata/";

	write_metadata(inFolderPath, outFolderPath, metadataPath, width, height, fps);
}
// add a test for propsChange

BOOST_AUTO_TEST_SUITE_END()