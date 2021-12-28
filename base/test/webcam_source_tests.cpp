#include <boost/test/unit_test.hpp>

#include "FrameMetadata.h"
#include "Frame.h"
#include "Logger.h"
#include "AIPExceptions.h"
#include "PipeLine.h"
#include "WebCamSource.h"
#include "ExternalSinkModule.h"
#include "FileWriterModule.h"
#include "StatSink.h"

BOOST_AUTO_TEST_SUITE(webcam_tests, *boost::unit_test::disabled())

BOOST_AUTO_TEST_CASE(basic)
{

	LoggerProps logprops;
	logprops.logLevel = boost::log::trivial::severity_level::info;
	Logger::initLogger(logprops);
	WebCamSourceProps webCamSourceprops(-1, 640, 480);
	auto source = boost::shared_ptr<WebCamSource>(new WebCamSource(webCamSourceprops));

	auto sink = boost::shared_ptr<ExternalSinkModule>(new ExternalSinkModule());
	source->setNext(sink);

	BOOST_TEST(source->init());
	BOOST_TEST(sink->init());
	source->step();
	auto frames = sink->pop();
	auto outFrame = frames.begin()->second;
	BOOST_TEST(outFrame->size() == 640 * 480 * 3);

	webCamSourceprops.width = 352;
	webCamSourceprops.height = 288;
	source->setProps(webCamSourceprops);
	source->step();
	frames = sink->pop();
	outFrame = frames.begin()->second;
	BOOST_TEST(outFrame->isEOS());
	source->step();
	frames = sink->pop();
	outFrame = frames.begin()->second;
	BOOST_TEST(outFrame->size() == 352 * 288 * 3);
}

BOOST_AUTO_TEST_CASE(basicFileWriter, *boost::unit_test::disabled())
{
	LoggerProps logprops;
	logprops.logLevel = boost::log::trivial::severity_level::info;
	Logger::initLogger(logprops);
	WebCamSourceProps webCamSourceprops(-1, 1920, 1080, 30);
	auto source = boost::shared_ptr<WebCamSource>(new WebCamSource(webCamSourceprops));
	auto fileWriter = boost::shared_ptr<Module>(new FileWriterModule(FileWriterModuleProps("./data/testOutput/wcs/wcs_????.raw")));
	source->setNext(fileWriter);

	PipeLine p("test");
	p.appendModule(source);
	BOOST_TEST(p.init());

	p.run_all_threaded();

	boost::this_thread::sleep_for(boost::chrono::seconds(10));
	Logger::setLogLevel(boost::log::trivial::severity_level::error);

	p.stop();
	p.term();
}
BOOST_AUTO_TEST_CASE(basicStatSink, *boost::unit_test::disabled())
{
	LoggerProps logprops;
	logprops.logLevel = boost::log::trivial::severity_level::info;
	Logger::initLogger(logprops);
	WebCamSourceProps webCamSourceprops(-1, 352, 288);
	auto source = boost::shared_ptr<WebCamSource>(new WebCamSource(webCamSourceprops));

	StatSinkProps sinkProps;
	sinkProps.logHealth = true;
	sinkProps.logHealthFrequency = 100;
	auto sink = boost::shared_ptr<Module>(new StatSink(sinkProps));
	source->setNext(sink);

	PipeLine p("test");
	p.appendModule(source);
	BOOST_TEST(p.init());

	p.run_all_threaded();

	boost::this_thread::sleep_for(boost::chrono::seconds(600));
	Logger::setLogLevel(boost::log::trivial::severity_level::error);

	p.stop();
	p.term();
}
BOOST_AUTO_TEST_CASE(basic_small)
{

	LoggerProps logprops;
	logprops.logLevel = boost::log::trivial::severity_level::info;
	Logger::initLogger(logprops);
	WebCamSourceProps webCamSourceprops(-1, 352, 288);
	auto source = boost::shared_ptr<WebCamSource>(new WebCamSource(webCamSourceprops));

	auto sink = boost::shared_ptr<ExternalSinkModule>(new ExternalSinkModule());
	source->setNext(sink);

	BOOST_TEST(source->init());
	BOOST_TEST(sink->init());
	source->step();
	auto frames = sink->pop();
	auto outFrame = frames.begin()->second;
	BOOST_TEST(outFrame->size() == 352 * 288 * 3);
	BOOST_TEST(source->term());
	BOOST_TEST(sink->term());
}

BOOST_AUTO_TEST_SUITE_END()