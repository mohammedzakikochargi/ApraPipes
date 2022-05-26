#include <ctime>
#include <fstream>

#include "FrameMetadata.h"
#include "Mp4VideoMetadata.h"
#include "Mp4WriterSink.h"
#include "Mp4WriterSinkUtils.h"
#include "EncodedImageMetadata.h"
#include "libmp4.h"
#include "list.h"
#include "PropsChangeMetadata.h"

class Mp4WriterSink::Detail
{
public:
	Detail(Mp4WriterSinkProps &_props)
	{
		setProps(_props);
		mNextFrameFileName = "";
		mux = nullptr;
		mMetadataEnabled = false;
	};

	void setProps(Mp4WriterSinkProps &_props)
	{
		mProps.reset(new Mp4WriterSinkProps(_props.chunkTime, _props.syncTime, _props.fps, _props.baseFolder));
	}

	~Detail()
	{
	};

	void setImageMetadata(framemetadata_sp &metadata)
	{
		mInputMetadata = metadata;
		auto encodedImageMetadata = FrameMetadataFactory::downcast<EncodedImageMetadata>(metadata);
		mHeight = encodedImageMetadata->getHeight();
		mWidth = encodedImageMetadata->getWidth();
	}

	bool enableMetadata(std::string &formatVersion)
	{
		mMetadataEnabled = true;
		mSerFormatVersion = formatVersion;
		return mMetadataEnabled;
	}

	bool set_video_decoder_config(mp4_video_codec codec)
	{
		if (codec == MP4_VIDEO_CODEC_MP4V)
		{
			vdc.width = mWidth;
			vdc.height = mHeight;
			vdc.codec = codec;
		}
		else
		{
			LOG_ERROR << "The codec is not supported by Mp4Writer";
			throw AIPException(AIP_FATAL, "Codec not found.");
		}
		return true;
	}

	void initNewMp4File(std::string &filename)
	{
		if (mux)
		{
			mp4_mux_close(mux);
		}
		syncFlag = false;
		lastFrameTS = 0;

		uint32_t timescale = 30000;
		now = std::time(nullptr);

		auto ret = mp4_mux_open(filename.c_str(), timescale, now, now, &mux);

		if (mMetadataEnabled)
		{
			/* \251too -> �too */
			std::string key = "\251too";
			std::string val = mSerFormatVersion.c_str();
			mp4_mux_add_file_metadata(mux, key.c_str(), val.c_str());
		}

		// track parameters
		params.type = MP4_TRACK_TYPE_VIDEO;
		params.name = "VideoHandler";
		params.enabled = 1;
		params.in_movie = 1;
		params.in_preview = 0;
		params.timescale = timescale;
		params.creation_time = now;
		params.modification_time = now;


		// add video track
		videotrack = mp4_mux_add_track(mux, &params);

		// sets vdc - replace this with strategy based impl
		set_video_decoder_config(MP4_VIDEO_CODEC_MP4V);

		mp4_mux_track_set_video_decoder_config(mux, videotrack, &vdc);

		// METADATA stuff
		if (mMetadataEnabled)
		{
			metatrack_params = params;
			metatrack_params.type = MP4_TRACK_TYPE_METADATA;
			metatrack_params.name = "ECS_METADATA";
			metatrack = mp4_mux_add_track(mux, &metatrack_params);

			if (metatrack < 0)
			{
				LOG_ERROR << "Failed to add metadata track";
				// #Dec_24_Review - should we throw exception here ? This means that user sends metadata to this module but we don't write ?
			}

			// https://www.rfc-editor.org/rfc/rfc4337.txt
			std::string content_encoding = "base64";
			std::string mime_format = "video/mp4";

			// #Dec_24_Review - use return code from the below function call 
			mp4_mux_track_set_metadata_mime_type(
				mux,
				metatrack,
				content_encoding.c_str(),
				mime_format.c_str());

			/* Add track reference */
			if (metatrack > 0)
			{
				LOG_INFO << "metatrack <" << metatrack << "> videotrack <" << videotrack << ">";
				ret = mp4_mux_add_ref_to_track(mux, metatrack, videotrack);
				if (ret != 0)
				{
					LOG_ERROR << "Failed to add metadata track as reference";
					// #Dec_24_Review - should we throw exception here ? This means that user sends metadata to this module but we don't write ?
				}
			}
		}
	}

	bool write(frame_sp &inImageFrame, frame_sp &inMp4MetaFrame)
	{
		std::string _nextFrameFileName = mWriterSinkUtils.getFilenameForNextFrame(inImageFrame->timestamp, mProps->baseFolder,
			mProps->chunkTime, mProps->syncTime, syncFlag);
		if (_nextFrameFileName == "")
		{
			LOG_ERROR << "Unable to get a filename for the next frame";
			return false;
		}
		if (mNextFrameFileName != _nextFrameFileName)
		{
			mNextFrameFileName = _nextFrameFileName;
			initNewMp4File(mNextFrameFileName);
		}
		if (syncFlag)
		{
			mp4_mux_sync(mux);
			syncFlag = false;
		}

		mux_sample.buffer = static_cast<uint8_t *>(inImageFrame->data());
		mux_sample.len = inImageFrame->size();
		mux_sample.sync = 0;
		int64_t diffInMsecs = 0;
		if (!lastFrameTS)
		{
			diffInMsecs = 0;
			mux_sample.dts = 0;
		}
		else
		{
			diffInMsecs = inImageFrame->timestamp - lastFrameTS;
			int64_t halfDurationInMsecs = static_cast<int64_t>(1000 / (2 * mProps->fps));
			if (!diffInMsecs)
			{
				inImageFrame->timestamp += halfDurationInMsecs;
			}
			else if (diffInMsecs < 0)
			{
				inImageFrame->timestamp = lastFrameTS + halfDurationInMsecs;
			}
			diffInMsecs = inImageFrame->timestamp - lastFrameTS;
		}
		lastFrameTS = inImageFrame->timestamp;
		mux_sample.dts = mux_sample.dts + static_cast<int64_t>((params.timescale / 1000) * diffInMsecs);

		mp4_mux_track_add_sample(mux, videotrack, &mux_sample);

		if (metatrack != -1 && mMetadataEnabled && inMp4MetaFrame.get())
		{
			mux_sample.buffer = static_cast<uint8_t *>(inMp4MetaFrame->data());
			mux_sample.len = inMp4MetaFrame->size();
			mp4_mux_track_add_sample(mux, metatrack, &mux_sample);
		}


		return true;
	}

	bool attemptFileClose()
	{
		if (mux)
		{
			mp4_mux_close(mux);
		}
		return true;
	}

	bool shouldTriggerSOS()
	{
		return !mInputMetadata.get();
	}

	boost::shared_ptr<Mp4WriterSinkProps> mProps;
	bool mMetadataEnabled = false;
private:
	// #Dec_24_Review - good practice to initialize all the variables here to default values - uninitialized values can cause unexpected errors
	int videotrack;
	int metatrack;
	int audiotrack;
	int current_track;
	uint64_t now;
	struct mp4_mux* mux;
	struct mp4_mux_track_params params, metatrack_params;
	struct mp4_video_decoder_config vdc;
	struct mp4_mux_sample mux_sample;
	struct mp4_track_sample sample;

	int mHeight;
	int mWidth;
	bool syncFlag = false;
	Mp4WriterSinkUtils mWriterSinkUtils;
	std::string mNextFrameFileName;
	std::string mSerFormatVersion;
	framemetadata_sp mInputMetadata;
	uint64_t lastFrameTS = 0;
};

bool Mp4WriterSink::validateInputOutputPins()
{
	if (getNumberOfInputsByType(FrameMetadata::ENCODED_IMAGE) != 1)
	{
		LOG_ERROR << "<" << getId() << ">::validateInputOutputPins expected 1 pin of ENCODED_IMAGE. Actual<" << getNumberOfInputPins() << ">";
		return false;
	}
	return true;
}

bool Mp4WriterSink::validateInputPins()
{
	if (getNumberOfInputPins() > 2)
	{
		LOG_ERROR << "<" << getId() << ">::validateInputPins size is expected to be 2. Actual<" << getNumberOfInputPins() << ">";
		return false;
	}

	auto inputPinIdMetadataMap = getInputMetadata();
	for (auto const &element : inputPinIdMetadataMap)
	{
		auto& metadata = element.second;
		auto mFrameType = metadata->getFrameType();
		if (mFrameType != FrameMetadata::ENCODED_IMAGE && mFrameType != FrameMetadata::MP4_VIDEO_METADATA)
		{
			LOG_ERROR << "<" << getId() << ">::validateInputPins input frameType is expected to be ENCODED_IMAGE or MP4_VIDEO_METADATA. Actual<" << mFrameType << ">";
			return false;
		}

		FrameMetadata::MemType memType = metadata->getMemType();
		if (memType != FrameMetadata::MemType::HOST)
		{
			LOG_ERROR << "<" << getId() << ">::validateInputPins input memType is expected to be HOST. Actual<" << memType << ">";
			return false;
		}
	}

	return true;
}

Mp4WriterSink::Mp4WriterSink(Mp4WriterSinkProps _props)
	: Module(SINK, "Mp4WriterSink", _props)
{
	mDetail.reset(new Detail(_props));
}

Mp4WriterSink::~Mp4WriterSink() {}

bool Mp4WriterSink::init()
{

	if (!Module::init())
	{
		return false;
	}

	return true;
}

bool Mp4WriterSink::setMetadata(framemetadata_sp &inputMetadata)
{
	// #Dec_24_Review - this function seems to do nothing
	mDetail->setImageMetadata(inputMetadata);
	return true;
}

bool Mp4WriterSink::processSOS(frame_sp &frame)
{
	auto inputMetadata = frame->getMetadata();
	auto mFrameType = inputMetadata->getFrameType();
	if (mFrameType == FrameMetadata::FrameType::ENCODED_IMAGE)
	{
		setMetadata(inputMetadata);
	}
	if (mFrameType == FrameMetadata::FrameType::MP4_VIDEO_METADATA)
	{
		auto mp4VideoMetadata = FrameMetadataFactory::downcast<Mp4VideoMetadata>(inputMetadata);
		std::string formatVersion = mp4VideoMetadata->getVersion();
		if (formatVersion.empty())
		{
			LOG_ERROR << "Serialization Format Information missing from the metadata. Metadata writing will be disabled";
			return true;
		}
		mDetail->enableMetadata(formatVersion);
	}
	return true;
}

bool Mp4WriterSink::shouldTriggerSOS()
{
	return mDetail->shouldTriggerSOS();
}

bool Mp4WriterSink::term()
{
	mDetail->attemptFileClose();
	return true;
}

bool Mp4WriterSink::process(frame_container& frames)
{
	auto imageFrame = getFrameByType(frames, FrameMetadata::FrameType::ENCODED_IMAGE);
	auto mp4metaFrame = getFrameByType(frames, FrameMetadata::FrameType::MP4_VIDEO_METADATA);

	if (isFrameEmpty(imageFrame))
	{
		LOG_ERROR << "Image Frame is empty. Unable to write.";
		// #Dec_24_Review - return false may do some bad things - it calls onStepFail - return true instead
		// #Dec_24_Review - returning here means that metadata information is dropped
		return true;
	}

	if (isFrameEmpty(mp4metaFrame) && mDetail->mMetadataEnabled)
	{
		// #Dec_24_Review - why to log error level ? can skip logging ?
		LOG_INFO << "empty MP4_VIDEO_METADATA frame fIndex<" << imageFrame->fIndex << ">";
	}

	try
	{
		if (!mDetail->write(imageFrame, mp4metaFrame))
		{
			LOG_FATAL << "Error occured while writing mp4 file<>" << imageFrame->fIndex;
			// #Dec_24_Review - return false may do some bad things - it calls onStepFail - do you want to throw exception instead ?
			return true;
		}
	}
	catch (const std::exception& e)
	{
		LOG_ERROR << e.what();
		// close any open file
		mDetail->attemptFileClose();
	}
	return true;
}

bool Mp4WriterSink::processEOS(string &pinId)
{
	// #Dec_24_Review - generally you do opposite of what you do on SOS, so that after EOS, SOS is triggered
	// in current state after EOS, SOS is not triggered - is it by design ? 
	// Example EOS can be triggered if there is some resolution change in upstream module
	// so you want to do mDetail->mInputMetadata.reset() - so that SOS gets triggered
	return true;
}

void Mp4WriterSink::setProps(Mp4WriterSinkProps &props)
{
	Module::setProps(props, PropsChangeMetadata::ModuleName::Mp4WriterSink);
}

bool Mp4WriterSink::handlePropsChange(frame_sp &frame)
{
	Mp4WriterSinkProps props;
	bool ret = Module::handlePropsChange(frame, props);
	mDetail->setProps(props);
	return ret;
}