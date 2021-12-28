#include <boost/test/unit_test.hpp>
#include <sw/redis++/redis++.h>
#include "FileReaderModule.h"
#include "FrameMetadata.h"
#include "Frame.h"
#include "Logger.h"
#include "AIPExceptions.h"
#include "GstOnvifRtspSink.h"
#include "PipeLine.h"
#include "H264Metadata.h"
#include "WebCamSource.h"
#include "StatSink.h"
#include "EventSource.h"
#include "RedisRepositoryController.h"
#include "RedisDBReader.h"
#include "FileReaderModule.h"
#include "FrameMetadataFactory.h"
// #include "CudaMemCopy.h"
// #include "ResizeNPPI.h"
// #include "CCNPPI.h"
// #include "JPEGEncoderNVJPEG.h"

BOOST_AUTO_TEST_SUITE(gstrtsponvifsink_tests)

BOOST_AUTO_TEST_CASE(gstrtspservertest, *boost::unit_test::disabled())
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

	GStreamerOnvifRTSPSinkProps _props;
	_props.frameFetchStrategy = ModuleProps::FrameFetchStrategy::PULL;
	_props.fps = 30;
	auto sink = boost::shared_ptr<GStreamerOnvifRTSPSink>(new GStreamerOnvifRTSPSink(_props));
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

BOOST_AUTO_TEST_CASE(gstrtspservertestrawyuv, *boost::unit_test::disabled())
{

	LoggerProps logprops;
	logprops.logLevel = boost::log::trivial::severity_level::info;
	Logger::initLogger(logprops);

	// metadata is known
	auto width = 640;
	auto height = 360;

	FileReaderModuleProps fileReaderProps("./data/Raw_YUV420_640x360/Image???_YUV420.raw");
	fileReaderProps.fps = 1000;
	fileReaderProps.readLoop = true;
	auto fileReader = boost::shared_ptr<FileReaderModule>(new FileReaderModule(fileReaderProps));
	auto metadata = framemetadata_sp(new RawImagePlanarMetadata(width, height, ImageMetadata::ImageType::YUV420, size_t(0), CV_8U, FrameMetadata::MemType::HOST));
	fileReader->addOutputPin(metadata);

	GStreamerOnvifRTSPSinkProps _props;
	_props.frameFetchStrategy = ModuleProps::FrameFetchStrategy::PULL;
	_props.fps = 30;
	_props.width = width;
	_props.height = height;
	auto sink = boost::shared_ptr<GStreamerOnvifRTSPSink>(new GStreamerOnvifRTSPSink(_props));
	fileReader->setNext(sink);

	PipeLine p("test");
	p.appendModule(fileReader);
	BOOST_TEST(p.init());

	p.run_all_threaded();
	boost::this_thread::sleep_for(boost::chrono::seconds(10000000));
	LOG_INFO << "profiling done - stopping the pipeline";
	// p.stop();
	p.term();
	p.wait_for_all();
}

BOOST_AUTO_TEST_CASE(gstrtspservertestrawrgbwcs, *boost::unit_test::disabled())
{

	LoggerProps logprops;
	logprops.logLevel = boost::log::trivial::severity_level::info;
	Logger::initLogger(logprops);

	// metadata is known
	auto width = 640;
	auto height = 480;

	WebCamSourceProps webCamSourceprops(-1, width, height);
	webCamSourceprops.qlen = 1;
	webCamSourceprops.quePushStrategyType = QuePushStrategy::NON_BLOCKING_ANY;
	auto source = boost::shared_ptr<WebCamSource>(new WebCamSource(webCamSourceprops));

	GStreamerOnvifRTSPSinkProps _props;
	_props.frameFetchStrategy = ModuleProps::FrameFetchStrategy::PULL;
	_props.fps = 30;
	_props.goplength = 1;
	_props.width = width;
	_props.height = height;
	_props.qlen = 1;
	_props.quePushStrategyType = QuePushStrategy::NON_BLOCKING_ANY;
	auto sink = boost::shared_ptr<GStreamerOnvifRTSPSink>(new GStreamerOnvifRTSPSink(_props));
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

// BOOST_AUTO_TEST_CASE(gstrtspservertestrawyuvwcs, *boost::unit_test::disabled())
// {

// 	LoggerProps logprops;
// 	logprops.logLevel = boost::log::trivial::severity_level::info;
// 	Logger::initLogger(logprops);

// 	// metadata is known
// 	auto width = 1920;
// 	auto height = 1080;

// 	WebCamSourceProps webCamSourceprops(-1, width, height);
// 	auto source = boost::shared_ptr<WebCamSource>(new WebCamSource(webCamSourceprops));

// 	auto stream = cudastream_sp(new ApraCudaStream);
// 	auto copy1 = boost::shared_ptr<Module>(new CudaMemCopy(CudaMemCopyProps(cudaMemcpyHostToDevice, stream)));
// 	source->setNext(copy1);

// 	auto m2 = boost::shared_ptr<Module>(new CCNPPI(CCNPPIProps(ImageMetadata::YUV420, stream)));
// 	copy1->setNext(m2);
// 	auto copy2 = boost::shared_ptr<Module>(new CudaMemCopy(CudaMemCopyProps(cudaMemcpyDeviceToHost, stream)));
// 	m2->setNext(copy2);
// 	auto outputPinId = copy2->getAllOutputPinsByType(FrameMetadata::RAW_IMAGE_PLANAR)[0];

// 	GStreamerOnvifRTSPSinkProps _props;
// 	_props.frameFetchStrategy = ModuleProps::FrameFetchStrategy::PULL;
// 	_props.fps = 30;
// 	_props.width = width;
// 	_props.height = height;
// 	auto sink = boost::shared_ptr<GStreamerOnvifRTSPSink>(new GStreamerOnvifRTSPSink(_props));
// 	copy2->setNext(sink);

// 	PipeLine p("test");
// 	p.appendModule(source);
// 	BOOST_TEST(p.init());

// 	p.run_all_threaded();
// 	boost::this_thread::sleep_for(boost::chrono::seconds(10000000));
// 	LOG_INFO << "profiling done - stopping the pipeline";
// 	p.stop();
// 	p.term();
// 	p.wait_for_all();
// }

BOOST_AUTO_TEST_CASE(gstrtspservertestwithuserupdates, *boost::unit_test::disabled())
{

	LoggerProps logprops;
	logprops.logLevel = boost::log::trivial::severity_level::info;
	Logger::initLogger(logprops);

	// metadata is known
	auto width = 640;
	auto height = 360;

	FileReaderModuleProps fileReaderProps("./data/h264_frames/Raw_YUV420_640x360_????.h264");
	fileReaderProps.fps = 1000;
	fileReaderProps.readLoop = true;
	auto fileReader = boost::shared_ptr<FileReaderModule>(new FileReaderModule(fileReaderProps));
	//create a class of h264Metadata
	auto metadata = framemetadata_sp(new H264Metadata(width, height));
	fileReader->addOutputPin(metadata);
	sw::redis::ConnectionOptions connection_options;
	connection_options.type = sw::redis::ConnectionType::UNIX;
	connection_options.path = "/run/redis/redis.sock";
	connection_options.db = 1;
	connection_options.socket_timeout = std::chrono::milliseconds(1000);
	sw::redis::Redis redis(connection_options);

	auto redisReader = boost::shared_ptr<RedisDBReader>(new RedisDBReader());
	std::string port = redisReader->getValueByKeyName(redis, "onvif.device.network.Ports.RTSPPort");
	GStreamerOnvifRTSPSinkProps _props;
	_props.frameFetchStrategy = ModuleProps::FrameFetchStrategy::PULL;
	_props.fps = 30;
	if (!port.empty())
	{
		LOG_INFO << "Port fetched from redis is " << port;
		_props.port = port;
	}
	auto sink = boost::shared_ptr<GStreamerOnvifRTSPSink>(new GStreamerOnvifRTSPSink(_props));
	fileReader->setNext(sink);

	// auto redisInstance = boost::shared_ptr<RedisRepositoryController>(new RedisRepositoryController());
	//initially read from DB and set props
	auto eventSource = boost::shared_ptr<EventSource>(new EventSource());

	eventSource->listenKey("__keyspace@1__:onvif.users.User*", [&]() -> void
						   {
							   auto userList = redisReader->getUsersList(redis);
							   _props.userList = userList;
							   sink->setProps(_props);
							   LOG_INFO << "userList Fetched on callback";
						   });

	eventSource->listenKey("__keyspace@1__:onvif.media.VideoSourceConfiguration*", [&]() -> void
						   { LOG_INFO << "Hello"; });

	PipeLine p("test");
	p.appendModule(fileReader);
	BOOST_TEST(p.init());
	std::thread t1 = std::thread(&EventSource::callbackWatcher, eventSource.get(), std::ref(redis));
	p.run_all_threaded();
	boost::this_thread::sleep_for(boost::chrono::seconds(10000));
	LOG_INFO << "profiling done - stopping the pipeline";
	p.stop();
	p.term();
	t1.join();
	p.wait_for_all();
}

BOOST_AUTO_TEST_CASE(propsSerialzationTest)
{
	GStreamerOnvifRTSPSinkProps props, propsResult;
	props.height = 320;
	props.width = 120;
	GStreamerOnvifRTSPSinkProps::User user;
	user.username = "Hello world";
	user.password = "Hello world";
	for (int i = 0; i < 5000000; i++)
	{
		props.userList.push_back(user);
	}

	size_t size = props.getSerializeSize();

	void *buffer = malloc(size);

	Utils::serialize(props, buffer, size);
	Utils::deSerialize(propsResult, buffer, size);

	BOOST_TEST(props.height == propsResult.height);
	BOOST_TEST(props.width == propsResult.width);
	for (int i = 0; i < 5000000; i++)
	{
		BOOST_TEST(props.userList[i].username.compare(propsResult.userList[i].username) == 0);
		BOOST_TEST(props.userList[i].password.compare(propsResult.userList[i].password) == 0);
	}
}

BOOST_AUTO_TEST_SUITE_END()