#include "Mp4ReaderSource.h"
#include "FrameMetadata.h"
#include "Mp4VideoMetadata.h"
#include "Mp4ReaderSourceUtils.h"
#include "PropsChangeMetadata.h"
#include "Frame.h"
#include "Command.h"
#include "libmp4.h"

class Mp4ReaderSource::Detail
{
public:
	Detail(Mp4ReaderSourceProps &props, std::function<frame_sp(size_t size)> _makeFrame)
	{
		setProps(props);
		makeFrame = _makeFrame;
		mFSParser = boost::shared_ptr<FileStructureParser>(new FileStructureParser());
	}

	~Detail()
	{
	}

	void setProps(Mp4ReaderSourceProps &props)
	{
		mProps = props;
		mState.mVideoPath = mProps.videoPath;
	}

	std::string getSerFormatVersion()
	{
		return mState.mSerFormatVersion;
	}

	void Init()
	{
		// #Dec_27_Review - redundant - use makeBuffer from produce
		sample_buffer_size = 5 * 1024 * 1024;
		size_t frameSize = static_cast<size_t>(sample_buffer_size);
		tempSampleBuffer = makeFrame(frameSize);
		sample_buffer = static_cast<uint8_t *>(tempSampleBuffer->data());

		// #Dec_27_Review - redundant - use makeBuffer from produce
		metadata_buffer_size = 1 * 1024 * 1024;
		frameSize = static_cast<size_t>(metadata_buffer_size);
		tempMetaBuffer = makeFrame(frameSize);
		metadata_buffer = static_cast<uint8_t *>(tempMetaBuffer->data()); // MFrames  

		initNewVideo();
	}

	bool parseFS()
	{
		/*
			raise error if init fails
			return 0 if no relevant files left on disk
			relevant - files with timestamp after the mVideoPath
		*/

		bool includeStarting = (mState.mVideoCounter ? false : true) | (mState.randomSeekParseFlag);
		mState.mParsedVideoFiles.clear();
		bool flag = mFSParser->init(mState.mVideoPath, mState.mParsedVideoFiles, includeStarting, mProps.parseFS);
		if (!flag)
		{
			LOG_ERROR << "File Structure Parsing Failed. Check logs.";
			throw AIPException(AIP_FATAL, "Parsing File Structure failed");
		}
		mState.mVideoCounter = 0;
		mState.randomSeekParseFlag = false;
		mState.mParsedFilesCount = mState.mParsedVideoFiles.size();
		return mState.mParsedFilesCount == 0;
	}

	/* initNewVideo responsible for setting demux-ing files in correct order
	   Opens the mp4 file at mVideoCounter index in mParsedVideoFiles
	   If mParsedVideoFiles are exhausted - it performs a fresh fs parse for mp4 files */
	bool initNewVideo()
	{
		/*  parseFS() is called:
			only if parseFS is set AND (it is the first time OR if parse file limit is reached)
			returns false if no relevant mp4 file left on disk. */
		if (mProps.parseFS && (mState.mParsedVideoFiles.empty() || mState.mVideoCounter == mState.mParsedFilesCount))
		{
			mState.end = parseFS();
		}

		// no files left to read
		if (mState.end)
		{
			mState.mVideoPath = "";
			return false;
		}

		if (mState.mVideoCounter < mState.mParsedFilesCount) //just for safety
		{
			mState.mVideoPath = mState.mParsedVideoFiles[mState.mVideoCounter];
			++mState.mVideoCounter;
		}

		LOG_INFO << "InitNewVideo <" << mState.mVideoPath << ">";

		/* libmp4 stuff */
		// open the mp4 file here
		if (mState.demux)
		{
			mp4_demux_close(mState.demux);
			mState.videotrack = -1;
			mState.metatrack = -1;
			mState.mFrameCounter = 0;
		}

		ret = mp4_demux_open(mState.mVideoPath.c_str(), &mState.demux);
		if (ret < 0)
		{
			throw AIPException(AIP_FATAL, "Failed to open the file <" + mState.mVideoPath + ">");
		}

		unsigned int count = 0;
		char **keys = NULL;
		char **values = NULL;
		ret = mp4_demux_get_metadata_strings(mState.demux, &count, &keys, &values);
		if (ret < 0)
		{
			LOG_ERROR << "mp4_demux_get_metadata_strings <" << -ret;
		}

		if (count > 0) {
			LOG_INFO << "Reading User Metadata Key-Values\n";
			for (auto i = 0; i < count; i++) {
				if ((keys[i]) && (values[i]) && !strcmp(keys[i], "\251too"))
				{
					LOG_INFO << "key <" << keys[i] << ",<" << values[i] << ">";
					mState.mSerFormatVersion.assign(values[i]);
				}
			}
		}

		mState.ntracks = mp4_demux_get_track_count(mState.demux);
		for (auto i = 0; i < mState.ntracks; i++)
		{

			ret = mp4_demux_get_track_info(mState.demux, i, &mState.info);
			if (ret < 0) {
				LOG_ERROR << "mp4 track info fetch failed <" << i << "> ret<" << ret << ">";
				continue;
			}

			if (mState.info.type == MP4_TRACK_TYPE_VIDEO && mState.videotrack == -1)
			{
				mState.video = mState.info;
				mState.has_more_video = mState.info.sample_count > 0;
				mState.videotrack = 1;
				mState.mFramesInVideo = mState.info.sample_count;
			}
		}

		if (mState.videotrack == -1)
		{
			LOG_ERROR << "No Videotrack found in the video <" << mState.mVideoPath << " Stopping.";
			throw AIPException(AIP_FATAL, "No video track found");
		}

		return true;
	}

	bool randomSeek(uint64_t &skipTS)
	{
		/* Takes a timestamp and sets proper mVideoFile and mParsedFilesCount (in case new parse is required) and initNewVideo().
		* Also, seeks to correct frame in the mVideoFile. If seek within in the videoFile fails, moving to next available video is attempted.
		* If all ways to seek fails, the read state is reset.
		*/

		DemuxAndParserState tempState = mState;
		std::string skipVideoFile;
		uint64_t skipMsecsInFile;
		int ret = mFSParser->randomSeek(skipTS, mProps.skipDir, skipVideoFile, skipMsecsInFile);
		LOG_INFO << "Attempting seek <" << skipVideoFile << "> @skipMsecsInFile <" << skipMsecsInFile << ">";
		if (ret < 0)
		{
			LOG_ERROR << "Skip to ts <" << skipTS << "> failed. Please check skip dir <" << mProps.skipDir << ">";
			return false;
		}
		bool found = false;
		for (auto i = 0; i < mState.mParsedVideoFiles.size(); ++i)
		{
			if (mState.mParsedVideoFiles[i] == skipVideoFile)
			{
				mState.mVideoCounter = i;
				found = true;
				break;
			}
		}
		if (!found)
		{
			// do fresh fs parse
			mState.mVideoPath = skipVideoFile;
			mState.mVideoCounter = mState.mParsedFilesCount;
			mState.randomSeekParseFlag = true;
		}
		initNewVideo();
		if (skipMsecsInFile)
		{
			uint64_t time_offset_usec = skipMsecsInFile * 1000;
			int seekedToFrame = -1;
			int returnCode = mp4_demux_seek(mState.demux, time_offset_usec, mp4_seek_method::MP4_SEEK_METHOD_NEAREST_SYNC, &seekedToFrame);
			// to determine the end of video
			mState.mFrameCounter = seekedToFrame;

			if (returnCode < 0)
			{
				LOG_ERROR << "Error while skipping to ts <" << skipTS << "> failed. File <" << skipVideoFile << "> @ time <" << skipMsecsInFile << "> ms errorCode <" << returnCode << ">";
				LOG_ERROR << "Trying to seek to next available file...";
				auto nextFlag = mFSParser->getNextToVideoFileFlag();
				if (nextFlag < 0)
				{
					// reset the state of mux to before seek op
					LOG_ERROR << "No next file found <" << nextFlag << ">" << "Resetting the reader state to before seek op";
					mState = tempState;
					LOG_ERROR << "Switching back to video <" << mState.mVideoPath << ">";
					// hot fix to avoid demux close attempt
					mState.demux = nullptr;
					mState.mVideoCounter -= 1;
					initNewVideo();
					return false;
				}
				mState.mVideoPath = mFSParser->getNextVideoFile();
				mState.mVideoCounter = mState.mParsedFilesCount;
				mState.randomSeekParseFlag = true;
				initNewVideo();
			}
		}
		return true;
	}

	void readNextFrame(uint8_t * sampleFrames, uint8_t * sampleMetadata, size_t * image_Frame_Size, size_t * metadata_Frame_Size)//, uint8_t* BFrames, uint8_t* MFrames
	{

		// all frames of the open video are already read and end has not reached
		if (mState.mFrameCounter == mState.mFramesInVideo && !mState.end)
		{
			// if parseFS is unset, it is the end
			LOG_ERROR << "frames number" << mState.mFrameCounter;
			mState.end = !mProps.parseFS;
			initNewVideo();
		}

		if (mState.end) // no files left to be parsed
		{
			actualImageSize = NULL;
			return;
		}

		if (mState.has_more_video)
		{
			ret = mp4_demux_get_track_sample(mState.demux,
				mState.video.id,
				1,
				sampleFrames,
				sample_buffer_size,
				sampleMetadata,
				metadata_buffer_size,
				&mState.sample);
			actualImageSize = reinterpret_cast<uint32_t>(image_Frame_Size);
			actualMetadataSize = reinterpret_cast<uint32_t>(metadata_Frame_Size);
			actualImageSize = mState.sample.size;
			actualMetadataSize = mState.sample.metadata_size;
			/* To get only info about the frames
			ret = mp4_demux_get_track_sample(
				demux, id, 1, NULL, 0, NULL, 0, &sample);
			*/

			if (ret != 0 || mState.sample.size == 0)
			{
				LOG_INFO << "<" << ret << "," << mState.sample.size << "," << mState.sample.metadata_size << ">";
				mState.has_more_video = 0;
				sampleFrames = nullptr;
				return;
			}

			++mState.mFrameCounter;
			// #Dec_27_Review - don't use string

			/*frameSize = static_cast<size_t>(mState.sample.size);
			metadataBuffer.assign(reinterpret_cast<char *>(metadata_buffer), mState.sample.metadata_size);
			return sample_buffer;*/

		}
		return;
	}


	Mp4ReaderSourceProps mProps;
private:
	frame_sp tempSampleBuffer;
	frame_sp tempMetaBuffer;

	struct DemuxAndParserState
	{
		mp4_demux *demux = nullptr;
		mp4_track_info info, video;
		mp4_track_sample sample;
		std::string mSerFormatVersion = "";
		int has_more_video;
		int videotrack = -1;
		int metatrack = -1;
		int ntracks = -1;
		uint32_t mParsedFilesCount = 0;
		uint32_t mVideoCounter = 0;
		uint32_t mFrameCounter = 0;
		uint32_t mFramesInVideo = 0;
		std::vector<std::string> mParsedVideoFiles;
		std::string mVideoPath = "";
		bool randomSeekParseFlag = false;
		bool end = false;
		Mp4ReaderSourceProps props;
	} mState;

	/*
		mState.end = true is possible only in two cases:
		- if parseFS found no more relevant files on the disk
		- parseFS is disabled and intial video has finished playing
	*/
public:
	size_t actualImageSize;
	size_t actualMetadataSize;
	uint8_t* sample_buffer;
	uint32_t sample_buffer_size = 0;
	uint8_t* metadata_buffer;
	uint32_t  metadata_buffer_size = 0;
	int ret;

	std::function<frame_sp(size_t size)> makeFrame;
	boost::shared_ptr<FileStructureParser> mFSParser;
};

Mp4ReaderSource::Mp4ReaderSource(Mp4ReaderSourceProps _props)
	:Module(SOURCE, "Mp4ReaderSource", _props)
{
	mDetail.reset(new Detail(_props, [&](size_t size) {
		return makeFrame(size);
	}));
}

Mp4ReaderSource::~Mp4ReaderSource() {}

bool Mp4ReaderSource::init()
{
	if (!Module::init())
	{
		return false;
	}

	mDetail->Init();
	encodedImageMetadata = getOutputMetadataByType(outImageFrameType);
	mp4FrameMetadata = getOutputMetadataByType(FrameMetadata::FrameType::MP4_VIDEO_METADATA);

	// set proto version in mp4videometadata
	auto serFormatVersion = mDetail->getSerFormatVersion();
	auto mp4VideoMetadata = FrameMetadataFactory::downcast<Mp4VideoMetadata>(mp4FrameMetadata);
	mp4VideoMetadata->setData(serFormatVersion);

	return true;
}

bool Mp4ReaderSource::term()
{
	auto moduleRet = Module::term();

	return moduleRet;
}

string Mp4ReaderSource::addOutputPin(framemetadata_sp& metadata)
{
	auto outFrameType = metadata->getFrameType();

	if (outFrameType == FrameMetadata::FrameType::ENCODED_IMAGE)
	{
		encodedImagePinId = Module::addOutputPin(metadata);
		outImageFrameType = outFrameType;
		return encodedImagePinId;
	}
	else
	{
		mp4FramePinId = Module::addOutputPin(metadata);
		return mp4FramePinId;
	}
}

bool Mp4ReaderSource::produce()
{
	frame_sp imgFrame;
	frame_sp metadataFrame;
	imgFrame = makeFrame(mDetail->mProps.biggerFrameSize, getOutputPinIdByType(FrameMetadata::ENCODED_IMAGE));
	metadataFrame = makeFrame(mDetail->mProps.biggerMetadataSize, getOutputPinIdByType(FrameMetadata::MP4_VIDEO_METADATA));
	uint8_t * sampleFrames = reinterpret_cast<uint8_t *>(imgFrame->data());
	uint8_t * sampleMetadata = reinterpret_cast<uint8_t *>(metadataFrame->data());

	// #Dec_27_Review - why can't we do makeBuffer and send the pointer here?
	// #Dec_27_Review - why do we have to use string, can't we use makeBuffer here
	size_t imageActualSize;
	size_t metadataActualSize;
	mDetail->readNextFrame(sampleFrames, sampleMetadata, &imageActualSize, &metadataActualSize);

	if (!mDetail->actualImageSize)
	{
		// #Dec_27_Review - return false will call onStepFail
		// #Dec_27_Review - onStepFail will not do anything currently - so produce is called again and again
		return false;
	}

	// #Dec_27_Review - why can't we use makeFrame here ?
	// makeBuffer is used - when you don't know how much buffer is required - in this case we exactly know how much we want
	// #Dec_27_Review - redundant use makeBuffer to readNextFrame
	auto frame1 = makeFrame(imgFrame, mDetail->actualImageSize, encodedImagePinId);
	frame_container frames;

	frames.insert(make_pair(encodedImagePinId, frame1));

	auto metadataFrameSize = mDetail->mProps.biggerMetadataSize;
	if (metadataFrameSize)
	{
		// #Dec_27_Review - redundant use makeBuffer to readNextFrame
		auto metadataSizeFrame = makeFrame(metadataFrame, metadataFrameSize, getOutputPinIdByType(FrameMetadata::MP4_VIDEO_METADATA));

		frames.insert(make_pair(mp4FramePinId, metadataSizeFrame));
	}

	send(frames);

	return true;
}

bool Mp4ReaderSource::validateOutputPins()
{
	if (getNumberOfOutputPins() > 2)
	{
		return false;
	}

	auto outputMetadataByPin = getOutputMetadata();
	for (auto const &itr : outputMetadataByPin)
	{
		auto &metadata = itr.second;
		FrameMetadata::FrameType frameType = metadata->getFrameType();

		if (frameType != FrameMetadata::MP4_VIDEO_METADATA && frameType != FrameMetadata::ENCODED_IMAGE)
		{
			LOG_ERROR << "<" << getId() << ">::validateOutputPins input frameType is expected to be MP4_VIDEO_METADATA or ENCODED_IMAGE. Actual<" << frameType << ">";
			return false;
		}

		auto memType = metadata->getMemType();
		if (memType != FrameMetadata::MemType::HOST)
		{
			LOG_ERROR << "<" << getId() << ">::validateOutputPins input memType is expected to be HOST. Actual<" << memType << ">";
			return false;
		}
	}

	return true;
}

Mp4ReaderSourceProps Mp4ReaderSource::getProps()
{
	return mDetail->mProps;
}

void Mp4ReaderSource::setProps(Mp4ReaderSourceProps &props)
{
	Module::setProps(props, PropsChangeMetadata::ModuleName::Mp4ReaderSource);
}

bool Mp4ReaderSource::handlePropsChange(frame_sp &frame)
{
	Mp4ReaderSourceProps props(mDetail->mProps.videoPath, mDetail->mProps.parseFS);
	bool ret = Module::handlePropsChange(frame, props);
	mDetail->setProps(props);
	return ret;
}

bool Mp4ReaderSource::handleCommand(Command::CommandType type, frame_sp& frame)
{
	if (type == Command::CommandType::Mp4Seek)
	{
		Mp4SeekCommand seekCmd;
		getCommand(seekCmd, frame);
		return mDetail->randomSeek(seekCmd.skipTS);
	}
	else
	{
		return Module::handleCommand(type, frame);
	}
}

bool Mp4ReaderSource::randomSeek(uint64_t skipTS)
{
	Mp4SeekCommand cmd(skipTS);
	return queueCommand(cmd);
}
