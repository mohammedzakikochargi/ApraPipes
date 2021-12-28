#include <boost/test/unit_test.hpp>
#include <sw/redis++/redis++.h>
#include "FileReaderModule.h"
#include "FrameMetadata.h"
#include "Frame.h"
#include "Logger.h"
#include "AIPExceptions.h"
#include "GstWebRTCSink.h"
#include "PipeLine.h"
#include "H264Metadata.h"
#include "WebCamSource.h"
#include "StatSink.h"
#include "EventSource.h"
#include "RedisRepositoryController.h"
#include "RedisDBReader.h"
#include "FileReaderModule.h"
#include "FrameMetadataFactory.h"


BOOST_AUTO_TEST_SUITE(gstwebrtcsink_tests)

BOOST_AUTO_TEST_CASE(gstwebrtctest, *boost::unit_test::disabled())
{

	LoggerProps logprops;
	logprops.logLevel = boost::log::trivial::severity_level::info;
	Logger::initLogger(logprops);

	// metadata is known
	auto width = 640;
	auto height = 360;

	FileReaderModuleProps fileReaderProps("./data/h264_frames/Raw_YUV420_640x360_????.h264");
	fileReaderProps.fps = 1000;
	fileReaderProps.qlen = 1;
	fileReaderProps.quePushStrategyType = QuePushStrategy::NON_BLOCKING_ANY;
	fileReaderProps.readLoop = true;
	auto fileReader = boost::shared_ptr<FileReaderModule>(new FileReaderModule(fileReaderProps));
	//create a class of h264Metadata
	auto metadata = framemetadata_sp(new H264Metadata(width, height));
	fileReader->addOutputPin(metadata);

	GStreamerWebRTCSinkProps _props;
	_props.frameFetchStrategy = ModuleProps::FrameFetchStrategy::PULL;
	_props.fps = 30;
    _props.width = width;
    _props.height = height;
    _props.peerId = "2605";
	auto sink = boost::shared_ptr<GStreamerWebRTCSink>(new GStreamerWebRTCSink(_props));
	fileReader->setNext(sink);

	PipeLine p("test");
	p.appendModule(fileReader);
	BOOST_TEST(p.init());

	p.run_all_threaded();
	boost::this_thread::sleep_for(boost::chrono::seconds(100000));
	LOG_INFO << "profiling done - stopping the pipeline";
	p.stop();
	p.term();
	boost::this_thread::sleep_for(boost::chrono::seconds(1));
	LOG_INFO << "profiling done - wait_for_all the pipeline";

	p.wait_for_all();
}

BOOST_AUTO_TEST_CASE(gstwebrtctestrawrgbwcs, *boost::unit_test::disabled())
{
	LoggerProps logprops;
	logprops.logLevel = boost::log::trivial::severity_level::info;
	Logger::initLogger(logprops);

	// metadata is known
	auto width = 640;
	auto height = 480;

	WebCamSourceProps webCamSourceprops(-1, width, height);
	webCamSourceprops.qlen = 1;
    webCamSourceprops.fps = 30;
	webCamSourceprops.quePushStrategyType = QuePushStrategy::NON_BLOCKING_ANY;
	auto source = boost::shared_ptr<WebCamSource>(new WebCamSource(webCamSourceprops));

	GStreamerWebRTCSinkProps _props;
	_props.frameFetchStrategy = ModuleProps::FrameFetchStrategy::PULL;
	_props.fps = 30;
	_props.goplength = 1;
	_props.width = width;
	_props.height = height;
	_props.qlen = 1;
    _props.peerId = "7696";
	_props.quePushStrategyType = QuePushStrategy::NON_BLOCKING_ANY;
	auto sink = boost::shared_ptr<GStreamerWebRTCSink>(new GStreamerWebRTCSink(_props));
	source->setNext(sink);

	PipeLine p("test");
	p.appendModule(source);
	BOOST_TEST(p.init());

	p.run_all_threaded();
	boost::this_thread::sleep_for(boost::chrono::seconds(100000));
	LOG_INFO << "profiling done - stopping the pipeline";
	p.stop();
	p.term();
	p.wait_for_all();
	boost::this_thread::sleep_for(boost::chrono::seconds(100000));
}

BOOST_AUTO_TEST_SUITE_END()